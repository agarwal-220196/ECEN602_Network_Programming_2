//system header
#include <iostream>
#include <string>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>

//network header
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>


using namespace std;

#define str_size  256

int main(int argc, char* argv[]){
    //1. Make select run. --Done!
    //2. Input Ip, port # and number of maximum clients from command line--Done!
    //3. Forward receive messages from clients and forward them.--Done!
    //4. Only accept clint with JOIN SBCP msg and unique username.
    //5. Clean client's resources if left.
    
    //Server's address info
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));
    //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //server_addr.sin_port = htons(12345);
    socklen_t server_addr_size = sizeof(server_addr);
    
    int maxClients=atoi(argv[3]);
    //int maxClients = 10;
    
    char received_str[str_size]; //wolaila nihaoshaya么么哒 嘿嘿哈是呀
    
    fd_set master;// Create a master set of file discriptor.
    fd_set read_fds;  // temp file descriptor list for select()
    FD_ZERO (&master);// Clear all entries in set.
    FD_ZERO (&read_fds);// Clear all entries in set.
    
    int newclient = 0; //newly accepted socket descriptor
    struct sockaddr_in client_addr; // client address
    int bytes = 0; // Number of bytes received.
    
    //==============initialize socket==================//
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        
    if(server_socket < 0 ){ 
        cout << "Could not establish connection" << endl;
        exit(0);
    }
    
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0){
        cout << "setsockopt(SO_REUSEADDR) failed" << endl;
    }
    
    cout << "Server socket established." << endl;
    
    //============bind socket to ip and port===============//
    if( bind(server_socket, (struct sockaddr* ) &server_addr, server_addr_size) < 0){
        cout << "Unable to bind socket." << endl;
        cout << "Error number " << (int) errno << endl;
        exit(0);
    }
    cout << "Socket binded." << endl;
    //=============listen to the client============//
    if(listen(server_socket, 10)< 0){
        cout << "Unable to find client." << endl;
        cout << "Error number " << (int) errno << endl;
        exit(0);
    }
    cout << "Listening to the cilent..." << endl;

    //====Now for this assignment, we use select to wait for event=======//
    
    FD_SET(server_socket, &master); // Add server_socket to master set.
    int fdmax = server_socket; //numfds should be highest file discriptor + 1
    
    while(true){
        read_fds = master; // Copy master.
        
        int socketnum = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        
        if(socketnum == -1){
            cout << "Error occurs when selecting." << endl;
            cout << "Error number " << (int) errno << endl;
            exit(0);
        }
        
        for(int i = 0; i <= fdmax; i++){
            
            if(FD_ISSET(i, &read_fds)){//Get one 
            
                if(i = server_socket){
                    //Accept a new connection
                    socklen_t client_addr_size = sizeof(client_addr);
                    newclient = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
                    
                    if(newclient == -1){
                        cout<< "Error occurs when accepting new clients" <<endl;
                        cout << "Error number " << (int) errno << endl;
                    }else {
                        //Add new connection to the list of connected clients
                        FD_SET(newclient, &master);
                    }
                    
                    //Send welcome message to the connected client
                    string welcomeMSG = "Welcome to the Chat Server";
                    send(newclient, welcomeMSG.c_str(), welcomeMSG.size() + 1, 0);
                } else{
                    char buf[4096];
                    memset(buf, 0, 4096);
                    //Receive message from ith descriptor in master
                    bytes = recv(newclient, buf, 4096,0);
                    if(bytes == 0){
                        //The client quit the chat server, close the socket
                        close(newclient);
                        FD_CLR(i, &master); //Remove from master set
                    } else{
                        //We got some data and send messages to other clients
                        for(int j = 0; j < fdmax; j++){
                            if(FD_ISSET(j, &master)){
                                //Except the server and ourselves
                                if(j != server_socket && j != i){
                                    if(send(j, buf, bytes, 0) == -1){
                                        cout << "Error occurs when broadcasting data!" << endl;
                                        perror("send");
                                    }//End of sending data
                                }//End of broadcasting
                            }
                        }//End of loop through file descriptor
                    }
                }//End dealing with data from client
            }//End got new connection
        }//End loop through file descriptors
    }//End of while loop

    return 0;
    
    
}