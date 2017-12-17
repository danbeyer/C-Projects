CS327 Chat client/server


Getting started (Unzipping and compiling chatclient.c):
	- Unzip project1.zip folder containing the 3 program files (chatclient.c, chatserve.py, makefile)

	- Make sure all 3 files (chatclient.c, chatserve.py, makefile) are all in the same directory after being extracted

	- On any flip server (flip1, flip2, or flip3), go that directory and on the command prompt type "make", this will compile chatclient.c 
	and create the program file called "chatclient".

	- Do not run the chatclient program yet!

Running the server and client programs:
	- Open an instance of flip (flip1, flip2, or flip3) and go the directory containing the 4 program files (chatclient.c, chatserve.py,
	chatclient, makefile)

	- Run the server by typing "python chatserve.py {port number}", where port number is a random port of your choosing

	- Example command (running on flip2): python chatserve.py 2030

	- After the server starts, you should see a message "Waiting for incoming connections"

	- Open a new instance of flip, different than the one you used to run the server (for example, if you ran the server on flip2, 
	open an instance of flip3).

	- Go to the same directory as before containing the program files, and run the chat client by typing:
	"chatclient {host name} {port number}" where host name is the host name of the host running the server (in our example above 
	this would be flip2) and port number is the port you chose when you started the server.

	- Example command to run client (running on flip3 and trying to connect to our server example above):
	chatclient flip2 2030

Using the server and client programs:
	- After connecting with the client, the client gets to type the first message.  After this, client and server take turns writing messages.

	- Either client or server can type "\quit" to quit the program.

	- If the client types "\quit", the client program quits but the server goes back to waiting for connections