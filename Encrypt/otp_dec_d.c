/***********************************************************
* Author: Daniel Beyer
* Date: 03/17/17
* CS344 Program 4 - OTP
* Description: This is an encryption program using a One-Time pad system.
* The program consists of 5 small programs.  2 function as daemons and are
* accessed using network sockets.  2 other programs will use the daemons to
* performs work, and a final program is a standalone keygen utility.
* The program encrypts and decrypts 5 plaintext files into ciphertext using a key
* based on modulo 27 operations (26 capital letters + space).
*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int MAXSIZE = 160000;

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues


int main(int argc, char *argv[])
{
    //Variable declarations
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	int pid, textLength, keyLength, i, verifyFail;
	socklen_t sizeOfClientInfo;
	char buffer[MAXSIZE];
	char readBuffer[1000];
	int ciphDec[MAXSIZE];  //array to hold text chars converted to ints
	int keyEnc[MAXSIZE]; // array to hold key chars converted to ints
	int ciphKey[MAXSIZE];    //array to hold message+key ints
	int textInts[MAXSIZE];    //array to hold modulo cipher ints
	char plainText[MAXSIZE];   //array to hold final cipher text

	struct sockaddr_in serverAddress, clientAddress;



	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

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
	printf("Listening on socket: %d\n", listenSocketFD);

	//This is a multiserver, so need a loop:
	while(1) {

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0) error("ERROR on accept");

    //Create child process
    pid = fork();

    if(pid<0) {
        perror("Error on fork");
        //exit(1);  Do we need this?
    }

    if(pid == 0) {
        //if in child process


        close(listenSocketFD); //Don't need the original listening socket anymore, so close
        //get message from client, which will contain plaintext and key
        //clear both buffers in preparation
        memset(buffer, '\0', MAXSIZE);

        while(strstr(buffer, "$$") == NULL) {
            memset(readBuffer, '\0', sizeof(readBuffer));
            charsRead = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer)-1, 0);
            //printf("Chunk recvd from client. Total size of message: %d\n", strlen(buffer));
            if(charsRead<0) {error("Server: Error reading from socket"); exit(1);}
            strcat(buffer, readBuffer);
        }

		//Evaluate first char to make sure correct client is connecting to server
		verifyFail = 0;
		if(buffer[0] != '!')
		{
            verifyFail = 1;
		}
		//else printf("Correct client verified.\n");

        //break buffer up into tokens to separate ciphertext and key
        char *ciphertext;
        char *key;
        ciphertext = strtok(buffer, ".");
        key = strtok(NULL, "$");
		textLength = strlen(ciphertext);
		keyLength = strlen(key);

		//clear array for use
		memset(ciphDec, 0, MAXSIZE);
		memset(keyEnc, 0, MAXSIZE);
		memset(ciphKey, 0, MAXSIZE);
		memset(textInts, 0, MAXSIZE);
		memset(plainText, '\0', MAXSIZE);



		//Encrypt - starting with converting text section into corresponding ints
		for(i=1; i<textLength-1; i++)
		{
			if(ciphertext[i] == ' ')
                ciphDec[i-1] = 26;
            else
            {
			ciphDec[i-1] = (ciphertext[i]-65);
            }
		}

        /*********DEBUG****
		printf("Here is the text converted to ints:\n");
        for(i = 0; i<textLength-2; i++)
		printf("%d ", ciphDec[i]);
        **********/


		//Convert key into corresponding ints
		for(i = 0; i<keyLength-1; i++)
        {
            if(key[i] == ' ')
                keyEnc[i] = 26;
            else
            {
                keyEnc[i] = (key[i]-65);
            }
        }

        /*********DEBUG****
        printf("Here is the key converted to ints:\n");
        for(i = 0; i<keyLength; i++)
		printf("%d ", keyEnc[i]);
        ******************/

        //Find difference of each position of each array
        for(i = 0; i<textLength-2; i++)
        {
            ciphKey[i] = (ciphDec[i] - keyEnc[i]);
        }

        /*********DEBUG****
        printf("Here is the text+key summed to ints:\n");
        for(i = 0; i<textLength-2; i++)
		printf("%d ", ciphKey[i]);
		******************/

        //find modulo 27 of each position
        for(i = 0; i<textLength-2; i++)
        {
            if(ciphKey[i]<0)
                textInts[i] = (ciphKey[i]+27) % 27;
            else
            textInts[i] = ciphKey[i] % 27;
        }

        /*********DEBUG****
        printf("Here is the modulo cipher\n");
        for(i = 0; i<textLength-2; i++)
		printf("%d ", textInts[i]);
		****************/

		//Convert cipher ints into corresponding letters of alphabet
		for(i = 0; i<textLength-2; i++)
        {
            if(textInts[i] == 26)
                plainText[i] = ' ';
            else
            {
                plainText[i] = textInts[i]+65;
            }
        }

        /*********DEBUG****
        printf("Here is the cipher:\n");
		printf("%s\n", plainText);
		********************/

        //Send cipher text back to client
        //printf("SERVER: I received this from the client: %s  %s\n", text, key);

        if(verifyFail == 1)
        {
            strcat(plainText, "@@$$");
        }
        else {
        strcat(plainText, "$$");
        }

        int acc = 0;
        while(acc < strlen(plainText)) {
        charsRead = send(establishedConnectionFD, plainText, strlen(plainText), 0);
        acc += charsRead;
        //printf("Actual number of chars written: %d\n", acc);
        }
        //charsRead = send(establishedConnectionFD, plainText, strlen(plainText), 0);
            //if (charsRead < 0) error("ERROR writing to socket");

        close(establishedConnectionFD);
        exit(0);
		}

    else {
        close(establishedConnectionFD); //Parent doesn't need this socket, so close.
    }

    }

	return 0;
}
