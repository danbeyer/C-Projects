#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<dirent.h>
#include<netdb.h>

#define BACKLOG 5
//Source: https://beej.us/guide/bgnet/output/html/multipage/index.html
/*Function: createSocket()
* Description: Iniates creation of initial server socket used to listen for incoming connections.
* Parameters: Int
*/
int createSocket (int sPort) {
	int controlsckt;

	if((controlsckt = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "ERROR: server socket creation\n");
		return -1;
	}
	
	//Source: https://www.freebsd.org/doc/en_US.ISO8859-1/books/developers-handbook/sockets-essential-functions.html
	struct sockaddr_in sa;

	sa.sin_family = AF_INET;
	sa.sin_port = htons(sPort);
	sa.sin_addr.s_addr = INADDR_ANY;

	if(bind(controlsckt, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
		fprintf(stderr, "ERROR: Binding\n");
		return -1;
	}

	if (listen(controlsckt, BACKLOG) == -1) { 
		fprintf(stderr, "ERROR: Listen\n");
		return -1;
	}

	return controlsckt;
}

/*Function: createDataSocket()
* Description: Initiates creation of socket used for data transfer between client and server
* Parameters: int, struct hostent
*/
int createDataSocket(int dataPortInt, struct hostent* host){
	int newdatasckt;
	if((newdatasckt = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //create
		fprintf(stderr, "ERROR: Unable to create data socket\n");
		return -1;
	}

	struct sockaddr_in sa_client;
	memset((char *)&sa_client, 0, sizeof(sa_client));
	
	//set values of addr struct
	sa_client.sin_family = AF_INET;
	sa_client.sin_port = htons(dataPortInt);
	memcpy(&sa_client.sin_addr, host->h_addr, host->h_length);

	if(connect(newdatasckt, (struct sockaddr*) &sa_client, sizeof(sa_client)) == -1) {
		fprintf(stderr, "ERROR: Connecting to data port\n");
		return -1;
	}
	
	return newdatasckt;
}
/*Function: connectionLoop()
* Description: Controls communication between server and client.  Receives requests and initiates transfer of
* either file or directory listing.
* Parameters: Int
*/
int connectionLoop(int newsockfd) {
	
	int commandInt;
	int dataPortInt;
	int newdatasckt;
	char dPort[10];
	char hostName[255];
	struct hostent * host;
	char fname[255];
	char *res = "res";
	char recvres[4];
	char command[2];
	
	//Much of the below back and forth between client server taken from Beej: https://beej.us/guide/bgnet/output/html/multipage/index.html
	//Client sends command type
	if((recv(newsockfd, &command, sizeof(command), 0)) == -1) {
		fprintf(stderr, "ERROR: Receiving command form client\n", command);
		return -1;
	}
	
	//response
	if ((send(newsockfd, res, sizeof(res), 0)) == -1){
		fprintf(stderr, "ERROR: sending response to command/n");
	}

	//Client sends data port 
	if((recv(newsockfd, &dPort, sizeof(dPort), 0)) == -1) {
		fprintf(stderr, "ERROR: Receiving port from client\n");
		return -1;
	}
	
	//response
	if ((send(newsockfd, res, sizeof(res), 0)) == -1){
		fprintf(stderr, "ERROR: error acking data port");
	}

	//Client sends hostname
	if((recv(newsockfd, &hostName, sizeof(hostName), 0)) == -1) {
		fprintf(stderr, "ERROR: Receiving host name\n");
		return -1;
	}
	
	//response
	if ((send(newsockfd, res, sizeof(res), 0)) == -1){
		fprintf(stderr, "ERROR: sending response to port/n");
	}

	//get host from hostname
	host = gethostbyname(hostName);
	commandInt = atoi(command);
	dataPortInt = atoi(dPort);
	
	if (commandInt == 1) { 
		if((recv(newsockfd, &fname, sizeof(fname), 0)) == -1) {
			fprintf(stderr, "ERROR: Receiving file name\n");
			return -1;
		}
		
		//response
		if ((send(newsockfd, res, sizeof(res), 0)) == -1){
			fprintf(stderr, "ERROR: sending response to file name\n");
		}
	}
	
	//Create new data socket for data transfer
	newdatasckt = createDataSocket(dataPortInt, host);
	
	if(commandInt == 1) {
		//Per project instructions, display file and port 
		printf("File '%s' requested on port %d.\n", fname, dataPortInt);

		//Source: Beej's guide for file I/O: https://beej.us/guide/bgc/output/html/multipage/stdio.html
		int file;
		if((file = open(fname, O_RDONLY)) == -1) {
			fprintf(stderr, "ERROR: Unable to open file\n");			
			char error[3];
			strncpy(error, "-1", 3);
			send(newdatasckt, &error, 2, 0);
			return -1;
		}

		int file_length = lseek(file, 0, SEEK_END);
		char string_length[15];
		memset((char *)&string_length, '\0', sizeof(string_length));
		//convert length to string
		sprintf(string_length, "%d", file_length);

		//send file text size
		if (send(newdatasckt, &string_length, sizeof(string_length), 0) == -1) { 
			fprintf(stderr, "ERROR: File length error\n");
			return -1;
		}

		recv(newdatasckt, &recvres, sizeof(recvres), 0);

		//Create string to hold file
		//Source: http://beej.us/298C/notes/notes1.html
		char *fileString = malloc(sizeof(char) * file_length);
		lseek(file, 0, SEEK_SET);

		//Read file text into string
		if(read(file, fileString, file_length) == -1) { 
			fprintf(stderr, "ERROR: Error reading file to string\n");
			return -1;
		}
		
		printf("Sending \"%s\" to %s: %d\n", fname, hostName, dataPortInt);  
		//Sending file, set initial length to 0
		int total = 0; 
		while (total < file_length) { 								   
			char outFile[1024]; 
			strncpy(outFile, &fileString[total], 1024);

			if (send(newdatasckt, outFile, 1024, 0) == -1) {
				fprintf(stderr, "ERROR: error sending file\n\n");
				return -1;
			}
			total += 1024; 
		}
			
		recv(newdatasckt, &recvres, sizeof(recvres), 0);
		printf("%s file successfully sent\n", fname);

		free(fileString);
	}
	else if(commandInt == 0) {		
		printf("List directory requested on port %d\n", dataPortInt);
		printf("Sending directory contents to %s: %d\n", hostName, dataPortInt);
		
		//Source: https://stackoverflow.com/questions/3554120/open-directory-using-c
		DIR *dir;
		if((dir = opendir(".")) == NULL) {			
			fprintf(stderr, "ERROR: error opening directory\n\n");			
			send(newdatasckt, "-1", 2, 0);
			return -1;
		}

		//Source:https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
		struct dirent *d;
		while ((d = readdir(dir)) != NULL) { 											
			char fdata[40];
			memset((char *)&fdata, '\0', sizeof(fdata));			
			strncpy(fdata, d->d_name, strlen(d->d_name));
			
			if (strncmp(fdata, ".", sizeof(fdata)) == 0 || strncmp(fdata, "..", sizeof(fdata)) == 0){				
			}
			else { 				
				if (send(newdatasckt, fdata, sizeof(fdata), 0) == -1) {					
					fprintf(stderr, "ERROR: error sending directory name\n\n");
					return -1;
				}
				recv(newdatasckt, &recvres, sizeof(recvres), 0);
			}
		}
		closedir(dir);
	}
	close(newdatasckt);
	return 0;
}


int main(int argc, char ** argv) {
	int sPort;
	int controlsckt;
	int newsockfd;
	//Command line validation
	if (argc != 2) {		
		fprintf(stderr, "ERROR: Use format: ftserver [port]");
		exit(1);
	}
	else {
		sPort = atoi(argv[1]);
	}
	//Creating initial socket
	if ((controlsckt = createSocket(sPort)) == -1) {
		exit(1);
	}

	printf("Server open on %d\n", sPort);
	
	while (1) { 		
		newsockfd = accept(controlsckt, NULL, NULL);
		if (newsockfd == -1) {			
			fprintf(stderr, "ERROR: Unable to accept incoming communication\n");
			exit(1);
		}

		int conn;
		conn = connectionLoop(newsockfd);

		close(newsockfd);
	}

	close(controlsckt);

	return 0;
}