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
* Source for much of this file: class filed "server.c"
*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

const int MAXSIZE = 150000;
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead, file_descriptor, kfile_descriptor, textLength, ktextLength;
	int ksize;
	ssize_t nread, kread;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char textBuffer[MAXSIZE];
	char keyBuffer[MAXSIZE];
	char textBuffer2[MAXSIZE];
	char testMessage[1] = "@";
	char recBuffer[1000];
	char finalCiph[MAXSIZE];
	char response[2];



	if (argc < 4) { fprintf(stderr,"USAGE: plaintext key port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert localhost name into usable form
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

    memset(textBuffer, '\0', MAXSIZE);



    //open plaintext file and get contents
    file_descriptor = open(argv[1], O_RDONLY);
    if(file_descriptor < 0)
    {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        exit(1);
    }
    nread = read(file_descriptor, textBuffer, sizeof(textBuffer));  //read in string from file
    textLength = strlen(textBuffer);

    if(textBuffer[0] == '!' || textBuffer[0] == '@' || textBuffer[0] == '#' || textBuffer[0] == '$'
       || textBuffer[0] == '%' || textBuffer[0] == '^' || textBuffer[0] == '&' || textBuffer[0] == '*'
    || textBuffer[0] == '(' || textBuffer[0] == ')') {
        fprintf(stderr, "Could not read, invalid chars, %s\n", argv[1]);
        exit(1);
    }

    //printf("%s\n", textBuffer);
    //printf("%d\n", textLength);


	//open key file and get contents
	memset(keyBuffer, '\0', MAXSIZE);
	kfile_descriptor = open(argv[2], O_RDONLY);
    if(kfile_descriptor < 0)
    {
        fprintf(stderr, "Could not open %s\n", argv[2]);
        exit(1);
    }
    kread = read(kfile_descriptor, keyBuffer, sizeof(keyBuffer));  //read in string from file
    ktextLength = strlen(keyBuffer);
    //printf("%s\n", keyBuffer);
    //printf("%d\n", ktextLength);



	//Compare plaintext and key to make sure key is big enough
	if(ktextLength < textLength)
	{
		fprintf(stderr, "Key %s is too short\n", argv[2]);
		exit(1);
	}

    //concatenate indicator, plain text, key together for sending to server, use "." as delimiter
	memset(textBuffer2, '\0', MAXSIZE);
	textBuffer2[0] = '@';
	strcat(textBuffer2, textBuffer);
    strcat(textBuffer2, ".");
    strcat(textBuffer2, keyBuffer);
    int mesLength = strlen(textBuffer2);
    textBuffer2[mesLength-1] = '\0';
    strcat(textBuffer2, "$$");
    //printf("%s\n", textBuffer2);



    //Send message
    //Source: Piazza @547
    int acc = 0;
    while(acc < strlen(textBuffer2)) {
        charsWritten = send(socketFD, textBuffer2, strlen(textBuffer2), 0);
        acc += charsWritten;
        //printf("Actual number of chars written: %d\n", acc);
    }

    //Test for verification message from server
    /*******
    int recByte;
    memset(response, '\0', sizeof(response));
    recByte = recv(socketFD, response, 1, 0);
    if(recByte < 0) error("CLIENT: error recving response from server\n");
    if(response[0] == 'n' || response[1] == 'n')
    {
        printf("Error: otp_enc not allowed to access otp_dec_d port\n");
        exit(1);
    }
    *********/
	// Send message to server
	//charsWritten = send(socketFD, textBuffer2, sizeof(textBuffer2), 0); // Write to the server
	//if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	//if (charsWritten < strlen(textBuffer2)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(finalCiph, '\0', sizeof(finalCiph));
    while(strstr(finalCiph, "$$") == NULL) {
        memset(recBuffer, '\0', sizeof(recBuffer));
        charsRead = recv(socketFD, recBuffer, sizeof(recBuffer)-1, 0);

        //printf("Chunk recvd from client. Total size of message: %d\n", strlen(buffer));
        if(charsRead<0) {error("CLIENT: Error reading from socket"); exit(1);}
        strcat(finalCiph, recBuffer);
        }

        //printf("%s\n", finalCiph);
        if(strstr(finalCiph, "@@") != NULL)
        {
            error("Error: otp_enc cannot connect to this daemon!\n");
            exit(1);
        }
        int terminalLocation = strstr(finalCiph, "$$") - finalCiph;
        finalCiph[terminalLocation] = '\0';

	//memset(textBuffer2, '\0', sizeof(textBuffer2)); // Clear out the buffer again for reuse
	//charsRead = recv(socketFD, textBuffer2, sizeof(textBuffer2) - 1, 0); // Read data from the socket, leaving \0 at end
	//if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	//printf("ciphertext recvd from server, size: %d\n", strlen(textBuffer2));
	printf("%s\n", finalCiph);    //print final cipher text

	close(socketFD); // Close the socket
	return 0;
}
