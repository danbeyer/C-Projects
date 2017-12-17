#!/bin/python

# Author: Daniel Beyer
# CS372 - Project 1: Chat server/client
# 10/24/17



import sys
from socket import *

serverHandle = "MrServer"	#server name

#Function where chat loop happens
def chatLoop(conn):
    
    while 1:                	#Loop runs continuously
        rec_data = conn.recv(513)[0:-1]  		#Using 512 here to hold enough space for message + ">" + client name      
        if rec_data == "":                  
            print "Connection closed on client end"
            print "Waiting for new connection"
            break
        
        print rec_data		#print received message      
        send_data = ""
        while len(send_data) > 500 or len(send_data) == 0:
            send_data = raw_input("{}> ".format(serverHandle))
        
		#quit command
        if send_data == "\quit":              
            print "Exiting..."
            exit(1)
        conn.send("{}> {}\0".format(serverHandle, send_data))	#Combine message with server name, ">", and \0 to send to C-based client


if __name__ == "__main__":    
    if len(sys.argv) != 2:              		#input validation for port
        print "Error: Use this format: python chatServer.py [port]"
        exit(1)
   
	#Source: https://docs.python.org/3.3/howto/sockets.html
    portNum = sys.argv[1]
    sckt = socket(AF_INET, SOCK_STREAM)         
    
    sckt.bind(('', int(portNum)))           
   
    sckt.listen(1)         
    


    print "Waiting for incoming connections"
    while 1:
        conn, address = sckt.accept()                         
        print "Connected on address {}".format(address)
        
		#Begin chat function
        chatLoop(conn)
        
        conn.close()