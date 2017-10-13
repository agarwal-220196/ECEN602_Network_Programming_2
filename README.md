#### This is 2nd Network Programming Assignment of ECEN 602
- This project implements a TCP Simple Broadcast Chat Server and Client
- This project is written in C/C++.
- The IDE is cloud 9.
- Ying Wang developed client side, built test cases, and wrote report.
- Hong Zhuang implemented server side code.

* How to run:
    * Run make to generate executable files for the project, client and server. Run make clean to remove all the object code from the directory.
    * To start the server, type the following command: ./server IP(IPv4 or IPv6) PORTNUMBER MaxUserNumber
    * To start the client, type the following command: ./client IP(IPv4 or IPv6) PORTNUMBER Username
    * In this project, Port number is 12345, and IP address is 127.0.0.1

* Server side:
    * Server side mainly consists of socket(), bind(), listen(), accept(), select() functions.
    * First create socket, and then bind the socket to the assigned IP address and port, and then listen to the port. Loop through 
the file descriptors array and make appropriate response to different file descriptors.

* Client side:
    * Client side mainly consists of socket(), connect(), select() functions.
    * First create socket, then connect the client to server, then select from standard input and received messages from server.


* Errata:
Server side: socket create error, bind error, litsen error, accept client error, receive and send message error.
Client side: connect error, send and receive message error