/***********************************************************************************************************
**Name: Robert Fletcher
**Email: fletcrob@OregonState.edu
**Assignment: Program 4 - OTP
**Due Date: Dec 6th, 2019
**Description:  This is the server portion for decoding ciphertext, runs constantly until stopped
***********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

//gets the size of the message sent by the client and returns the size
int getMessageSize(int connection) {
	int charsRead = 0;
	char sizeOfMessage[6];
	int messageSize = 0;

	memset(sizeOfMessage, '\0', sizeof(sizeOfMessage));
	charsRead = recv(connection, sizeOfMessage, 5, 0);
	if (charsRead < 0) {
		error("ERROR reading from socket");
	}

	//convert string of size of the message to int of the size of the message
	messageSize = atoi(sizeOfMessage);

	//returns the size of the message
	return messageSize;
}


//checks to see whether request is coming from otp_enc
void check_otp_enc(char* requested, int connection) {
	int charsRead = 0;
	//if connection doesn't say it's otp_enc (doesn't match the string we've set up)
	if (strcmp("otp_dec", requested) != 0) {
		//print out an error message
		fprintf(stderr, "ERROR, connection not coming from otp_dec");
		//send back error message to client
		int charsRead = send(connection, "Failed", 6, 0);
		if (charsRead < 0) {
			error("ERROR writing to the socket");
		}
		//exit the child process with error code 1
		exit(1);
	}
	//if connection says it's otp_enc (matches the string we've set up)
	else {
		//send back success message to the client
		charsRead = send(connection, "success", 7, 0);
		//if there was an error writing to the socket
		if (charsRead < 0) {
			error("ERROR writing to the socket");
		}
	}

}


//gets the message being sent by the client
void getMessage(int message_size, char* entireMessage, int establishedConnectionFD) {
	char buffer[256];
	int charsRead = 0;
	memset(entireMessage, '\0', sizeof(entireMessage));
	//get the entire message from the client
	int i = 0;  //used as a counter to make sure we exit the loop at the appropriate time

	while (i < message_size) {
		//reset the characters in the buffer
		memset(buffer, '\0', 256);
		//receive 255 characters of the message
		charsRead = recv(establishedConnectionFD, buffer, 255, 0);
		//if there is an error reading from the buffer
		if (charsRead < 0) {
			error("ERROR reading from buffer");
		}
		else {
			//stuff the received characters into our array that will contain the entire message
			strcat(entireMessage, buffer);
		}
		i += charsRead;  //add number of characters to i
	}

}

//encrypts the message sent by the client using the client's key
void encryptMessage(int message_size, char* entireMessage, char* key) {
	//create the encrypted message via the process defined in assignment 4
	int i = 0;
	//loop through each character in the message
	for (i = 0; i < message_size; i++) {
		//if the character is the space character, set to 91 (last character after alphabet)
		if (entireMessage[i] == 32) {
			entireMessage[i] = 91;
		}
		//if the key is the space character, set to 91 (last character after alphabet)
		if (key[i] == 32) {
			key[i] = 91;
		}
		//perform the decryption of the character as per the instructions.  Subtract key from message, mod 27 (alphabet plus space)
		//need to subtract 65 from each so that it ends up being in the range of 27, then add 65 back on to get ASCII value
		//to stuff back into the message in order to return it
		entireMessage[i] = (((entireMessage[i] - 65 - (key[i] - 65)) % 27));
		//for the decryption, need to add 27 back on if it's a negative value, otherwise calculation returns incorrect character
		if(entireMessage[i] < 0){
			//add 27
			entireMessage[i] += 27;
		}
		//add 65 to get the ASCII value
		entireMessage[i] += 65;
		//if the value is at the end again, need to convert to space character (ASCII 32)
		if (entireMessage[i] == 91) {
			entireMessage[i] = 32;
		}
	}

}

int main(int argc, char *argv[])
{
	fflush(stderr);
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char connectionBuffer[8];
	struct sockaddr_in serverAddress, clientAddress;
	char entireMessage[70000];	//accepts message size up to 70,000 characters including spaces which covers test files
	char key[70000]; //should be the same size as the message

	if (argc != 2) { fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args, print error if != 2

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	//run constantly as per instructions/lecture info
	while (1) {

		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//need to use fork in order to create a separate process to handle the rest of the transaction
		pid_t processID;
		processID = fork();

		//if there was an error forking the process, print error message
		if (processID < 0) {
			perror("Failed to properly fork process");
		}

		//if this is the child process
		if (processID == 0) {
			//make sure we are communicating with otp_enc by getting a message from the client
			charsRead = recv(establishedConnectionFD, connectionBuffer, 7, 0);
			//if there was an error reading from the socket
			if (charsRead < 0) {
				error("ERROR reading from socket");
			}

			//check to see if the client is correct (otp_enc)
			check_otp_enc(connectionBuffer, establishedConnectionFD);

			//convert string of size of the message to int of the size of the message
			int message_size = getMessageSize(establishedConnectionFD);

			//reply with success message so that the client knows it was okay
			charsRead = send(establishedConnectionFD, "success", 7, 0);
			//error message if send doesn't work
			if (charsRead < 0) {
				error("ERROR writing to socket");
			}

			//get the message from the client
			getMessage(message_size, entireMessage, establishedConnectionFD);

			//let client know that the message receipt was successful
			charsRead = send(establishedConnectionFD, "success", 7, 0);
			if (charsRead < 0) {
				error("ERROR writing to socket");
			}

			//get the key from the client
			getMessage(message_size, key, establishedConnectionFD);

			//let client know that the key receipt was successful
			charsRead = send(establishedConnectionFD, "success", 7, 0);
			if (charsRead < 0) {
				error("ERROR writing to socket");
			}

			//encrypt the message sent by the user
			encryptMessage(message_size, entireMessage, key);

			//send encrypted message back to the client
			charsRead = send(establishedConnectionFD, entireMessage, message_size, 0);
			if (charsRead < 0) {
				error("ERROR writing to socket");
			}

			close(establishedConnectionFD); // Close the existing socket which is connected to the client
			close(listenSocketFD); //close the listening socket

			//exit 0 as process is now complete
			exit(0);
		}

		//if this is the parent process, waitpid
		else {
			waitpid(processID, WNOHANG);
		}
	}

	return 0;
}
