# Author: James Palmer
# Project 1 
# Description: Implement a client-server network
#   application using the sockets API.


from socket import socket, AF_INET, SOCK_STREAM
import sys

# check number of arguments
if len(sys.argv) != 3:
    #print out the usage
    print "Port: <serverPortNumber> ChatServe attempting to connect..."
	print "Please wait *cue elevator music*   "
    sys.exit(1)

# acquire server address
serverName = sys.argv[1]
serverPortNumber = int(sys.argv[2])

clientQuit = "\quit"
clientMessage = " "

# set up client side and TCP socket
clientSocket = socket(AF_INET, SOCK_STREAM)

# implement 3-way TCP handshake
try:
    clientSocket.connect((serverName, serverPortNumber))
except Exception as error:
    print error.strerror
    sys.exit(1)

# confirm connect to server with print
print '\nConnection established....'
print '\nWelcome to the chat!'
print 'Use \quit to end connection'

# give me a username
print 'Chatclient username must be 10 or fewer characters long.\n'
clientName = raw_input('\nEnter your chatclient username: ')

# added an if clause for instances < 10 character maximum
# if greater than 10 chars or if there is a space in the name...
while((len(clientName) > 10) or (" " in clientName)): 
    clientName = raw_input('\nError: ERROR: User names may not be longer than 10 characters long and may not have any spaces. \n')
    clientName = raw_input('\nEnter your chatclient username:')

# add header to the username
clientName = clientName + "> "
print ' '

# \quit entry loop
while(1):
    clientMessage = raw_input(clientName)

    # messages cannot be >500 per homework
    while(len(clientMessage) > 500):
        print'\nNotice: Messages must be less than 500 characters long.  Please try again\n'
        clientMessage = raw_input(clientName)

    # moved up /quit user command
    if(clientMessage == clientQuit):
        clientSocket.send("\quit")
        print '\nConnection ended by chat client'
        clientSocket.close()
        sys.exit(0)

    clientMessage = clientName + clientMessage + "\n"
    clientSocket.send(clientMessage)

    # server response!
    responseMsg = clientSocket.recv(1024)
    print(responseMsg)

    # quit program 
    if clientQuit in responseMsg:
        print 'Connection ended by server'
        
		#close connection to prevent faults
        clientSocket.close()
        sys.exit(0)