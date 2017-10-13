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
//include the header file 
#include "SBCP.h"

#define str_size  256

using namespace std;

int clientCount = 0;
struct SBCP_client_info *clients; //assign memory to store SBCP_client_info array


//check if username does not exist
bool username_not_exist(char user_name[]){
    for(int i = 0; i < clientCount; i++){
        if(strcmp(user_name,clients[i].username) == 0){
            return false; //Username has already in list, return false.
        }
    }
    return true; //If username does not exist, return true.
}

//Check if the client has already joined the chat room
bool isjoined(int client_fd){
    struct SBCP_message join_msg;
    struct SBCP_attribute join_msg_attribute;
    char user_name[16];
    read(client_fd,(struct SBCP_message *) &join_msg,sizeof(join_msg));
    join_msg_attribute = join_msg.attribute[0];//Get username
    strcpy(user_name, join_msg_attribute.payload);
    if(!username_not_exist(user_name)) //If the client with this username is in chat room
    {
        printf("\nClient already joined.");
        return true;
    }
    else //If client with this username has never joined chat room then let him join.
    {
        strcpy(clients[clientCount].username, user_name);
        clients[clientCount].fd = client_fd;
        clients[clientCount].clientCount = clientCount;
        clientCount += 1;
    }
    return false;
}


int main(int argc, char* argv[]){
    //1. Make select run. --Done!
    //2. Input Ip, port # and number of maximum clients from command line--Done!
    //3. Forward receive messages from clients and forward them.--Done!
    //4. Only accept clint with JOIN SBCP msg and unique username.
    //5. Clean client's resources if left.
    
    //SBCP message
    struct SBCP_message recvMsg, fwdMsg, join_broadcast, leave_broadcast;
    struct SBCP_attribute client_attribute;
    
    //Server's address info
    struct sockaddr_in server_addr, *clients_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));
    
    socklen_t server_addr_size = sizeof(server_addr);
    
    int maxClients=atoi(argv[3]);

    char received_str[str_size]; 
    char buf[4096];//Receive data from client

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
        perror("establishing");
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
        //cout << "Error number " << (int) errno << endl;
        perror("binding");
        exit(0);
    }
    
    cout << "Socket binded." << endl;
    
    clients = (struct SBCP_client_info *)malloc(maxClients*sizeof(struct SBCP_client_info));
    clients_addr = (struct sockaddr_in *)malloc(maxClients*sizeof(struct sockaddr_in));
    
    //=============listen to the client============//
    if(listen(server_socket, 10)< 0){
        cout << "Unable to find client." << endl;
        //cout << "Error number " << (int) errno << endl;
        perror("listening");
        exit(0);
    }
    cout << "Listening to the cilent..." << endl;

    //====Now for this assignment, we use select to wait for event=======//
    
    FD_SET(server_socket, &master); // Add server_socket to master set.
    int fdmax = server_socket; //numfds should be highest file discriptor + 1
    int temp; //Store value of fdmax temporarily.
    
    while(true){
        read_fds = master; // Copy master.
        
        if(select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1){
            cout << "Error occurs when selecting." << endl;
            perror("Select");
            exit(0);
        }
        
        for(int i = 0; i <= fdmax; i++){ //loop through file descriptor
            
            if(FD_ISSET(i, &read_fds)){//Get one from existing file descriptor
            
                if(i == server_socket){ //We get a server socket, check if there is a client want to connect or send msg.
                
                    //Accept a new connection and add it to the clients address array clients_addr
                    socklen_t client_addr_size = sizeof(clients_addr[clientCount]);
                    newclient = accept(server_socket, (struct sockaddr *)&clients_addr[clientCount], &client_addr_size);
                    
                    if(newclient == -1){
                        cout<< "Error occurs when accepting new clients" <<endl;
                        cout << "Error number " << (int) errno << endl;
                    }else {
                        temp = fdmax;
                        //Add new connection to the list of connected clients
                        FD_SET(newclient, &master);
                        if(newclient > fdmax){ //track max fd
                            fdmax = newclient;
                        }
                        if(!isjoined(newclient)){
                            //We have a new client who want to join our chat room
                            //Send an ONLINE Message to all the clients except this
                            cout << "User " << clients[clientCount-1].username << " has joined chat room" << endl;
                            join_broadcast.header.vrsn = 3;
                            join_broadcast.header.type = 8;
                            join_broadcast.attribute[0].type = 2;
                            strcpy(join_broadcast.attribute[0].payload,clients[clientCount-1].username);
                            
                            for(int j = 0; j <= fdmax; j++){ //Loop again the file descriptors and broadcast
        	            	    if (FD_ISSET(j, &master)) 
        	            	    {
        	            	        // Except the server and itself
        	            	        if (j != server_socket && j != i){
        	                    	    if ((write(j,(void *) &join_broadcast,sizeof(join_broadcast))) == -1){
        	                            	perror("broadcasting join message");
        	                            }
        	                        }
        	                    }       
        	                }
                        } else {
                            cout << "User with username "<< clients[clientCount-1].username <<" has already joined, try another username." << endl;
                            
                            
                            close(newclient);
                            fdmax = temp;
                            FD_CLR(newclient, &master);//clear newclient if username does not exist
                        }
                    }
                } else{ 
                    //Here we got a existing connection, deal with data from it.
                    bytes = read(i,(struct SBCP_message *) &recvMsg,sizeof(recvMsg));
                    
                    if(bytes <= 0){
                        if (bytes == 0){
                            for(int k = 0; k < clientCount; k++){
                                if(clients[k].fd == i){
                                    leave_broadcast.attribute[0].type = 2;
                                    strcpy(leave_broadcast.attribute[0].payload, clients[k].username);
                                    
                                    //leave_broadcast.attribute[1].type = 2;
                                    //strcpy(leave_broadcast.attribute[1].payload, clients[k].username);
                                }
                            }
                            
                            //Client closed the connection
                            cout << "User " << leave_broadcast.attribute[0].payload << " has left the chat room." << endl; 
                            
                            leave_broadcast.header.vrsn=3;
                            leave_broadcast.header.type=6;
                            
                            for(int j = 0; j <= fdmax; j++){
                                //broadcasting
                                if(FD_ISSET(j, &master)){
                                    //except the server and itself
                                    if(j != server_socket && j != i){
                                        //send leaving msg
                                        if((write(j, (void*)&leave_broadcast, sizeof(leave_broadcast))) == -1){
                                            perror("broadcasting leaving msg");
                                        }
                                    }
                                }
                            }
                            
                        } else if(bytes < 0){
                            perror("receiving msg");
                        } 
                        close(i);
                        FD_CLR(i, &master);//Remove from master set
                        for(int x = i;x < clientCount;x++){
                            
    				        clients[x]=clients[x+1];
    				        
                        }
                        clientCount--;
                    } else {
                        //bytes > 0, we received some data from client
                        client_attribute = recvMsg.attribute[0];//get message
                        fwdMsg = recvMsg;
                        fwdMsg.header.type = 3;
                        fwdMsg.attribute[1].type=2; //get username
                        
                        char uname[16];
                        strcpy(uname, recvMsg.attribute[1].payload);
                        //strcpy(uname, client_attribute.payload);
                        //uname[strlen(client_attribute.payload)]='\0';
                        
                        for(int k = 0; k < clientCount; k++){
                            
                            if(clients[k].fd == i){
                                
                                strcpy(fwdMsg.attribute[1].payload, clients[k].username);
                            }
                        }
                        
                        cout << fwdMsg.attribute[1].payload <<" says: " << fwdMsg.attribute[0].payload << endl;
                        
                        //Forward the message to all the clients except this
                        for(int j = 0; j <= fdmax; j++){
                            //send fwdMsg to everyone
                            if(FD_ISSET(j, &master)){
                                //except the server and itself
                                if(j != server_socket && j != i){
                                    if((write(j, (void *) &fwdMsg, bytes))){
                                        perror("Forwarding message");
                                    }
                                }
                            }
                        }
                    } //End fowarding messages
                }//End dealing with data from client
            }//End got new connection
        }//End loop through file descriptors
    }//End of while loop

    close(server_socket);
    return 0;
    
    
}


