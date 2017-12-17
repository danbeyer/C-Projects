import socket
import sys
import os

#Function: createSocket()
#Description: Handles creation of initial client socket
#Parameters: Int server host, int server port 
def createSocket(serverHost, serverPort):
	controlsckt = socket.socket()
	if (controlsckt == -1):
		return -1

	if ((controlsckt.connect((serverHost, serverPort))) == -1):
		return -1

	return controlsckt

#Function: requestHandler()
#Description: Handles sending requests for directory or files to server and making those responses usable by dataHandler()
#Parameters: int socket descriptor, int command, int data port, string file name

def requestHandler(controlsckt, command, dPort, fname):
	resp = 0
	#if command is for directory listing
	if (command == 0): 
		if (controlsckt.send(str(command)) == -1):
			print 'ERROR: Error sending command to server\n'
			return -1

		resp = controlsckt.recv(4)		
		dPort = dPort + '\0' 
		if (controlsckt.send(dPort) == -1):
			print 'ERROR: Error sending data Port to server\n'
			return -1
		
		resp = controlsckt.recv(4)
		host = socket.gethostname()

		if(controlsckt.send(host) == -1):
			print 'ERROR: Error sending host name to server\n'
			return -1
		
		resp = controlsckt.recv(4)
	#if command is for file request
	elif (command == 1): 
		if (controlsckt.send(str(command)) == -1):
			print "ERROR: Error sending command to server\n"
			return -1

		resp = controlsckt.recv(4)
		dPort = dPort + '\0' 
		if (controlsckt.send(dPort) == -1):
			print 'ERROR: error sending data port to server\n'
			return -1

		resp = controlsckt.recv(4)
		host = socket.gethostname()

		if(controlsckt.send(host) == -1):
			print 'ERROR: Error sending hosname to server\n'
			return -1

		resp = controlsckt.recv(4)		
		fname = fname + '\0'
		
		#send filename
		if (controlsckt.send(fname) == -1):
			print 'ERROR: error sending file name to server\n'
			return -1
				
		resp = controlsckt.recv(4)
	else:
		
		print 'ERROR: Invalid request sent/n'
		return -1

	return 0

#Function: receiveFile()
#Description: Handles incoming data from server.  Contains functionality to recieve directory listing and files
#Parameters: Int socket descriptor, int command, string file name  
def dataHandler(newdatasckt, command, fname):
	if (command == 0): 		
		data = 1
		print '\nDirectory Contents:'
		while(data): 			
			data = newdatasckt.recv(40)			
			#if directory retrieval returned error 
			if (data == '-1'):
				print 'ERROR: Failed to retrieve directory list\n'
				return -1
			#if directory end	
			elif (data == 0): 
				return 0
			#Continue to recieve directory data
			else:				
				print '  ' + data				
				res = 'res'
				newdatasckt.send(res)
	elif (command == 1): 
		flen = newdatasckt.recv(15)
		res = 'res'
		newdatasckt.send(res)

		file_length = int(flen.strip('\0'))
		
		if (file_length == -1):
			print 'ERROR: Error opening file\n'
			return -1

		#Source: http://www.tutorialspoint.com/python/os_listdir.htm
		#Find current directory contents
		curdir = os.listdir('.')
		
		#Below function handles renaming file if already exists, uses file pid
		for file in curdir:
			if (file == fname): 
				pid = os.getpid() 
				pid = str(pid) 
				fname = pid + fname
		
		file = open(fname, "a")	
		flen = 0 #set initial file length to 0
		#continue to recv data until reached file length
		while (flen < file_length): 			
			data = newdatasckt.recv(1024)
			if (data == -1):				
				print 'ERROR: Unable to receive file\n'
				return -1
			elif (data == 0):				
				return 0
			else:				
				file.write(data.strip('\0'))
				flen += 1024		
		res = 'res'
		newdatasckt.send(res)
	
		print 'File saved as: ' + fname
	return 0



def main(argv):

	if (len(sys.argv) < 5 or len(sys.argv) > 6):
		print 'ERROR: Use format: python ftclient.py [host] [port] [command] [filename(if needed)] [data port]\n'
		exit(1)
	
	sHost = argv[0]
	sPort = int(argv[1])

	controlsckt = createSocket(sHost, sPort)

	if(controlsckt == -1):
		print 'ERROR: Could not connect to server\n'
		exit(1)
	
	dPort = 0
	command = -1
	fname = -1
	#if command is for directory list
	if (argv[2] == '-l'): 
		if (len(sys.argv) != 5):			
			print 'ERROR: Use format: python ftclient.py [host] [port] [command] [data port]\n'
			exit(1)
			
		command = 0;
		
		dPort = argv[3]

		#Run request for directory listing
		requestHandler(controlsckt, command, dPort, -1)
	#If command is for file request
	elif (argv[2] == '-g'): 
		if (len(sys.argv) != 6):
			print 'ERROR: Use format: python ftclient.py [host] [port] [command] [filename] [data port]\n'
			exit(1)		

		command = 1
		fname = argv[3]
		dPort = argv[4]

		#Run request for file
		requestHandler(controlsckt, command, dPort, fname)
	else: 
		
		print 'ERROR: Use format: -g or -l\n'
		exit(1)

	#Set up data socket 
	datasckt = socket.socket()
	#error check
	if (datasckt == -1):
		print 'ERROR: Error creating data socket\n'
		exit(1)

	#convert data port to int
	dPort = int(dPort)

	#bind
	x = datasckt.bind(('0.0.0.0', dPort))
	#error check
	if(x == -1):
		print 'ERROR: Binding data socket failed\n'
		exit(1)

	#listen
	x = datasckt.listen(5)
	#error check
	if (x == -1):
		print 'ERROR: Data socket listening failed\n'
		exit(1)

	#accept
	newdatasckt, addr = datasckt.accept()
	#error check
	if(newdatasckt == -1):
		print 'ERROR: Data socket accept failed\n'

	#Retrieve file from server
	dataHandler(newdatasckt, command, fname)

	newdatasckt.close

	datasckt.close

	controlsckt.close

	return 0

if __name__ == "__main__":
	main(sys.argv[1:])