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

* keygen.c: This program generates a string of random chars (UPPERCASE alphabet) to be used as the key
*****************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char* argv[]) {
   int i;
   int ranNum;
   int ranChar;
   int size;

   //seed rand num generator
   srand(time(NULL));

   if (argc < 2) {
	printf("Must specify a key length");
        exit(1);
   }

   //get key length from args
   sscanf(argv[1], "%d", &size);

   //create key
   for (i = 0; i < size; i++) {
	ranNum = rand() % 27;

	if(ranNum < 26) {
	   //find matching char from random number, 65+ ensures capital
	   ranChar = 65 + ranNum;
	   printf("%c", ranChar);
	}
	else {
	   //space is last possibility
	   printf(" ");
	}
   }

   //Finally, add new line
   printf("\n");

   return 0;
}
