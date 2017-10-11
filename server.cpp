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

int main(){
    
    
    
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
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(12345);
    socklen_t server_addr_size = sizeof(server_addr);
    
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

    char received_str[str_size]; 
    
    //====Now for this assignment, we use select to wait for event=======//
    fd_set master;
    FD_ZERO (&master);
    FD_SET(server_socket, &master);
    
    while(true){
        fd_set copy = master;
        
        int socketnum = select(0, &copy, nullptr, nullptr, nullptr);
        
        for(int i = 0; i < socketnum; i++){
            
            int sock = copy.fd_array[i];
            
            if(sock = server_socket){
                //Accept a new connection
                int client = accept(server_addr, nullptr, nullptr);
                
                //Add new connection to the list of connected clients
                FD_SET(client, &master);
                
                //Send welcome message to the connected client
                string welcomeMSG = "Welcome to the Chat Server";
                send(client, welcomeMSG.c_str(), welcomeMSG.size() + 1, 0);
            } else{
                char buf[4096];
                memset(buf, 0, 4096);
                //Receive message
                int bytes = recv(sock, buf, 4096,0);
                if(bytes == 0){
                    //Close the socket
                    close(sock);
                    FD_CLR(sock, &master);
                }else{
                    //Send messages to other clients
                    for(int i = 0; i < master.fd_count; i++){
                        int outSock = master.fd_array[i];
                        if(outSock != server_socket && outSock != sock){
                            send = (outSock, buf, bytes, 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
    
    
}