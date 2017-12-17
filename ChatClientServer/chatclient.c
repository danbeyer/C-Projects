/*Author: Daniel Beyer
* CS372 - Project 1: Chat server/client
* 10/24/17
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

// addrinfo struct creation and socket creation and connection
// Parameters: char*, char*
// Source: https://beej.us/guide/bgnet/output/html/multipage/syscalls.html
int createSocket(char* address, char* portNum){
	struct addrinfo hints;
    struct addrinfo *res;
    int conn_status, sockfd, status;
	
	memset(&hints, 0, sizeof hints);        
	hints.ai_family = AF_INET;                  
	hints.ai_socktype = SOCK_STREAM;    

	if((status = getaddrinfo(address, portNum, &hints, &res)) != 0){   
		fprintf(stderr,
				"Error. Invalid port.\n",
				gai_strerror(status));
		exit(1);
	}
	//Socket creation with error checking
	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
		fprintf(stderr, "Error. Socket was not created.\n");
		exit(1);
	}
	//Connection creation with error checking
	if ((conn_status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
		fprintf(stderr, "Error in connection.\n");
		exit(1);
	}
	
	return sockfd;
	//frees addrinfo struct now that socket has been created
	freeaddrinfo(res);
}


//Chat function where sending and receiving happen.
//Parameters: int, char*
void chatLoop(int sockfd, char * username){  
    //Variable declarations
	int bytes = 0;      
	int status;

	//Create and set to zero buffers to hold incoming and outgoing messages
	char send_buffer[513];                   
    memset(send_buffer, 0 ,sizeof(send_buffer));
	
	char input_buffer[500];
	memset(input_buffer, 0, sizeof(input_buffer));
    
	char rec_buffer[513];
	memset(rec_buffer, 0, sizeof(rec_buffer));
    

	fgets(send_buffer, 500, stdin);       
    
	while(1){	
		//Prompt
		printf("%s> ", username);   
		fgets(input_buffer, 500, stdin);
		
		//Quit command
		if (strcmp(input_buffer, "\\quit\n") == 0){
			printf("Exiting...\n");
			exit(1);
		}
		
		//Building the send buffer by adding together username, ">", and the message
		strcpy(send_buffer, username);
		strcat(send_buffer, "> ");
		strcat(send_buffer, input_buffer);
		//Bytes as error checker
		bytes = send(sockfd, send_buffer, strlen(send_buffer), 0);  
		
		if(bytes == -1){
				fprintf(stderr, "Error in sending data.\n");
				exit(1);
		}
		
		//Status used for error checking
		status = recv(sockfd, rec_buffer, 513, 0);      
		
		if (status == -1){
			fprintf(stderr, "Error in receiving data.\n");
			exit(1);
		}
        
		//Per Beej guide
		else if (status == 0){                              
			printf("Server ended connection.\n");
			break;
		}
		else{
				//Print received message
			printf("%s\n", rec_buffer);
		}
		
		//Clear buffers
		memset(send_buffer, 0, sizeof(send_buffer)); 
		memset(input_buffer, 0, sizeof(input_buffer));		
		memset(rec_buffer, 0, sizeof(rec_buffer));
	}
	
	close(sockfd);                                  
	printf("Connection closed.\n");
}




int main(int argc, char *argv[]){ 
	//Server and User name variables
	char userName[10];     
    char serveName[10];

	//Input validation
	if(argc != 3){                                                      
		fprintf(stderr, "Error: Use this format: chatClient [server] [port]\n");
		exit(1);
	}
	
    printf("Please enter user handle (10 chars or less).");
	scanf("%s", userName);	
     
	int sockfd = createSocket(argv[1], argv[2]);           

	//Begin chat function
	chatLoop(sockfd, userName);      
	          
}
