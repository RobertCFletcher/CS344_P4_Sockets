/***********************************************************************************************************
**Name: Robert Fletcher
**Email: fletcrob@OregonState.edu
**Assignment: Program 4 - OTP
**Due Date: Dec 6th, 2019
**Description:  This is the client portion for decoding ciphertext, runs once when called to talk to server
** and gets a decoded message back from the server
***********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

//read in the message from the file indicated by the user
void readMessageFromFile(char* fileName, char* entireMessage) {
	FILE* myFile = fopen(fileName, "r");
	//if there is an error opening the file, print error message and exit
	if (!myFile) {
		fprintf(stderr, "error opening file\n");
		exit(1);
	}

	//read in message contents as a single line (it shouldn't accept newline characters)
	fgets(entireMessage, 70000, myFile);
	fclose(myFile);
}

//remove any trailing newline characters from the message
void checkForTrailingNewline(char* entireMessage) {
	int message_size = 0;
	int i = 0;
	//loop until the last character is no longer a newline character
	for(i = 0; i < strlen(entireMessage); i++){
		if(entireMessage[i] == '\n'){
			entireMessage[i] = '\0';
		}
	}
}

void lengthAndCharacterCheck(char* entireMessage, char* key) {
	//check to ensure key and message are the same size
	if (strlen(entireMessage) > strlen(key)) {
		fprintf(stderr, "Error: key is too short\n");
		exit(1);
	}

	//check to ensure that there are no invalid characters in the message or key
	int i = 0;
	for (i = 0; i < strlen(entireMessage); i++) {
		//if the ASCII value falls outside of the range allowed, or if the character is not a space character
		if ((entireMessage[i] < 65 || entireMessage[i] > 90) && entireMessage[i] != 32) {
			fprintf(stderr, "invalid characters are present in the message\n");
			fprintf(stderr, "invalid character is: %c\n", entireMessage[i]);
			exit(1);
		}
		//if the ASCII value falls outside of the range allowed, or if the character is not a space character
		if ((key[i] < 65 || key[i] > 90) && key[i] != 32) {
			fprintf(stderr, "invalid characters are present in the key\n");
			exit(1);
		}
	}
}


void clientCheck(int socketFD) {
	char connectionBuffer[8];
	memset(connectionBuffer, '\0', sizeof(connectionBuffer));
	int charsWritten = 0;
	int charsRead = 0;
	
	//send otp_enc to server
	charsWritten = send(socketFD, "otp_dec", 7, 0);
	//check if there is an error sending
	if (charsWritten < 0) {
		error("CLIENT: ERROR writing to socket");
	}

	//receive message back from server
	charsRead = recv(socketFD, connectionBuffer, 7, 0);
	//check if there is an error receiving
	if (charsRead < 0) {
		error("CLIENT: ERROR reading from socket");
	}

	//if the server doesn't send back "success" print error message and exit 2
	if (strcmp("success", connectionBuffer) != 0) {
		fprintf(stderr, "rejected by server, could not connect to port%d\n", socketFD);
		exit(2);
	}
}



//sends the size of the message to the server
void sendMessageSize(char* entireMessage, int socketFD) {
	int charsWritten = 0;
	int charsRead = 0;
	char messageSizeString[6];
	char buffer[8];
	int messageSize = strlen(entireMessage);
	memset(buffer, '\0', sizeof(buffer));
	memset(messageSizeString, '\0', sizeof(messageSizeString));
	//convert size of entire message into a string
	//itoa(messageSize, messageSizeString, 10);
	snprintf(messageSizeString, 6, "%d", strlen(entireMessage));

	//send the string to the server
	charsWritten = send(socketFD, messageSizeString, 5, 0);
	if (charsWritten < 0) {
		error("CLIENT: ERROR writing to socket");
	}

	//receive message back from server
	charsRead = recv(socketFD, buffer, 7, 0);
	//check if there is an error receiving
	if (charsRead < 0) {
		error("CLIENT: ERROR reading from socket");
	}
}


//sends an entire message to the server
int sendMessage(char* entireMessage, int socketFD) {
	int charsWritten = 0;
	int charsRead = 0;
	char buffer[8];
	int messageSize = strlen(entireMessage);
	//send message to server
	charsWritten = send(socketFD, entireMessage, messageSize, 0);
	//check if there was an error sending
	if (charsWritten < 0) {
		error("CLIENT: ERROR writing to socket");
	}
	//receive message back from server
	charsRead = recv(socketFD, buffer, 7, 0);
	if (charsRead < 0) {
		error("CLIENT: ERROR reading from socket");
	}

}

//gets the encoded message back from the server and returns size of new message
int getMessage(char* entireMessage, int socketFD) {
	int sizeOfMessage = strlen(entireMessage);
	int charsRead = 0;
	char buffer[256];
	memset(entireMessage, '\0', 70000);  //reset all values in message
	int i = 0;

	//get message from the server in pieces
	while (i < sizeOfMessage) {
		//reset buffer
		memset(buffer, '\0', 256);
		//read the message
		charsRead = recv(socketFD, buffer, 255, 0); // Read 255 characters from the socket
		if (charsRead < 0) {
			error("CLIENT: ERROR reading from socket");
		}
		else {
			i += charsRead;
			strcat(entireMessage, buffer);
		}
	}

	return strlen(entireMessage);
}

int main(int argc, char *argv[])
{
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
	char entireMessage[70000];
	char key[70000];
	memset(entireMessage, '\0', 70000);
	int message_size = 0;

	if (argc < 3) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	//open message file for reading and get message before setting up connection in case there's an issue with the file
	readMessageFromFile(argv[1], entireMessage);

	//open key file for reading and get key before setting up connection in case there's an issue with the file
	readMessageFromFile(argv[2], key);

	//remove any trailing newline character at the end of the message
	checkForTrailingNewline(entireMessage);

	//remove any trailing newline character at the end of the key
	checkForTrailingNewline(key);

	//perform length and character checks on key nad message
	lengthAndCharacterCheck(entireMessage, key);

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address, using "localhost"
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
		error("CLIENT: ERROR connecting");
		exit(2);
	}

	//perform client check
	clientCheck(socketFD);
	
	//send message size to server
	sendMessageSize(entireMessage, socketFD);

	//send the message to the server
	sendMessage(entireMessage, socketFD);

	//send the key to the server
	sendMessage(key, socketFD);

	//get encoded message back from the server
	getMessage(entireMessage, socketFD);

	//print out the encoded message
	printf("%s\n", entireMessage);
	fflush(stdout);

	close(socketFD); // Close the socket
	return 0;
}
