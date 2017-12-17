Daniel Beyer


Getting started (Unzipping and compiling chatclient.c):
	- Unzip project2.zip folder containing the 3 program files (ftserver.c, ftclient.py, makefile)

	- Make sure all 3 files (ftserver.c, ftclient.py, makefile) are all in the same directory after being extracted

	- On any flip server (flip1, flip2, or flip3), go that directory and on the command prompt type "make", this will compile ftserver.c 
	and create the program file called "ftserver".

	- Do not run the ftserver program yet!

Running the server and client programs:
	- Open an instance of flip (flip1, flip2, or flip3) and go the directory containing the 4 program files (ftserver.c, ftclient.py,
	ftserver, makefile)

	- Run the server by typing "ftserver {port number}", where port number is a random port of your choosing

	- Example command (running on flip2): ftserver 20300

	- After the server starts, you should see a message "Server open on {port number}"

	- Open a new instance of flip, different than the one you used to run the server (for example, if you ran the server on flip2, 
	open an instance of flip3).

	- Go to the same directory as before containing the program files, and run the  client by typing:
	"python3 ftclient.py [SERVERHOST] [SERVERPORT] [COMMAND] [FILENAME] [DATAPORT]" where SERVERHOST is the host name of the host running the server (in our example above 
	this would be flip2) and SERVERPORT is the port you chose when you started the server.  COMMAND is "-l" for directory listing or "-g" for file request.  
	FILENAME is the name of file requesting.  DATAPORT is the port you wish to use for data transfer.

	- Example command to run client and request directory list (running on flip3 and trying to connect to our server example above):
	python ftclient.py flip2 20300 -l 20301

	- Example command to request a file:
	python ftclient.py flip2 20300 -g shortfile.txt 20301 

Tested using these commands (and including a file called "shortfile.txt" in the same directory):
	- ftserver 20000
	- python ftclient.py flip2 20000 -l 20001
	- python ftclient.py flip2 20000 -g shortfile.txt 20001