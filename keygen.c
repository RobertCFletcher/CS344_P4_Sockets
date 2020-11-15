/***********************************************************************************************************
**Name: Robert Fletcher
**Email: fletcrob@OregonState.edu
**Assignment: Program 4 - OTP
**Due Date: Dec 6th, 2019
**Description:  This is the keygen file, generates a random string to use for the key
**Usage Example: keygen 70000 > key70000 which will create a keygen file
***********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>


int main(int argc, char* argv[]){

	//prints an error and exits with error code 1 if the user doesn't use the correct syntax
	if (argc < 2) {
		fprintf(stderr, "USAGE: %s [keylength]\n", argv[0]);
		exit(1);
	}

	// http://man7.org/linux/man-pages/man3/strtol.3.html
	// used to check whether the key entered is valid

	char* myString;
	int base = (argc > 2) ? atoi(argv[2]) : 10;
	//set errno to zero to check whether strtol succeeds or fails
	errno = 0;
	long myKey = strtol(argv[1], &myString, 10);

	if((errno == ERANGE && (myKey == LONG_MAX || myKey == LONG_MIN)) || (errno != 0 && myKey == 0)) {
		fprintf(stderr, "USAGE: %s [keylength], must be a valid number\n", argv[0]);
	}

	if(myString == argv[1]) {
		fprintf(stderr, "no digits were found\n");
		exit(1);
	}

	//if we successfully parsed a number

	char keyToReturn[myKey + 1]; //add one in order to add a newline to the end
	srand(time(0));  //ensure that the number is random by using time for seed
	int i = 0;
	int myChar = 0;

	for(i = 0; i < myKey; i++){
		//random character in the alphabet
		myChar = rand() % 27 + 65;
		//if the integer is out of the range of the alphabet
		if(myChar == 91){
			//set character to space character instead
			myChar = 32;
		}
		keyToReturn[i] = myChar;		
	}
	//add newline to the end of the key to return
	keyToReturn[myKey] = '\n';

	//print out the key to return to the user
	printf("%s", keyToReturn);
	
	return 0;
}

