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
#include <netinet/in.h>

// SBCP STRUCT
#include "SBCP.h"

using namespace std;

//Read the message form SBCPmessage
int readMessage(int sockfd){
    struct SBCP_message serverMessage;
    int status = 0;
    int nbytes=0;
    nbytes=read(sockfd,(struct SBCP_message *) &serverMessage,sizeof(serverMessage));
    
    //read the username and message
    if(serverMessage.header.type==3){
    	if((serverMessage.attribute[0].payload!=NULL || serverMessage.attribute[0].payload!='\0') && (serverMessage.attribute[1].payload!=NULL || serverMessage.attribute[1].payload!='\0') && serverMessage.attribute[0].type==4 && serverMessage.attribute[1].type==2){     	
		    cout<<serverMessage.attribute[1].payload<<" says: " <<serverMessage.attribute[0].payload<<endl;
	    }
        status=0;
    }
    
    //when username is used, set status to be 1
    if(serverMessage.header.type==5){
    	if((serverMessage.attribute[0].payload!=NULL || serverMessage.attribute[0].payload!='\0') && serverMessage.attribute[0].type==1){
            cout<<"Try another username!" <<endl;
       }
       status=1;
    }
    
    //bonus feature
    if(serverMessage.header.type==6){
	    if((serverMessage.attribute[0].payload!=NULL || serverMessage.attribute[0].payload!='\0') && serverMessage.attribute[0].type==2){
           	cout<<serverMessage.attribute[0].payload<<" is now OFFLINE" <<endl;
        }
        status=0;
    }
    
    if(serverMessage.header.type==8){
	    if((serverMessage.attribute[0].payload!=NULL || serverMessage.attribute[0].payload!='\0') && serverMessage.attribute[0].type==2){
            cout<<serverMessage.attribute[0].payload<<" is now ONLINE" <<endl;
        }
        status=0;
    }
    
    return status;
}

// join the chatroom
void join(int sockfd, char* arg[]){
    struct SBCP_header header;
    struct SBCP_attribute attribute;
    struct SBCP_message message;
    int status = 0;

    header.vrsn = '3';
    header.type = '2';
    
    attribute.type = 2;//Username
    attribute.length = strlen(arg[3]) + 1;
    strcpy(attribute.payload,arg[3]);
    message.header = header;
    message.attribute[0] = attribute;

    write(sockfd,(void *) &message,sizeof(message));

    // Sleep to allow Server to reply
    sleep(1);
    status = readMessage(sockfd); 
    
    //if username already used, close the socket.
    if(status == 1){
        close(sockfd);
        exit(4);
        // sockfd=-1;
    }
}

//chat
void send(int socket){
    struct SBCP_header header;
    header.vrsn = '3';
    header.type = '4';//send
    
    struct SBCP_message message;
    struct SBCP_attribute Attribute;
    
    message.header = header;

    int nread = 0;
    char temp[512];
    struct timeval wait_time;
    fd_set read_fds;
    wait_time.tv_sec = 2;
    wait_time.tv_usec = 0;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
  
    select(STDIN_FILENO+1, &read_fds, NULL, NULL, &wait_time);
    if (FD_ISSET(STDIN_FILENO, &read_fds)){
	    nread = read(STDIN_FILENO, temp, sizeof(temp));
        if(nread > 0){
	        temp[nread] = '\0';
        }
    
    	Attribute.type = 4;//message;
    	strcpy(Attribute.payload,temp);
    	message.attribute[0] = Attribute;
    	write(socket,(void *) &message,sizeof(message));
    	
    }
    else{
     	cout<<"Timed out.\n"<<endl;
    }
}


int main(int argc, char*argv[]){
    if(argc==4){
        int sockfd=socket(AF_INET,SOCK_STREAM,0);
        if(sockfd==-1){
            cout<<"Error creating socket!"<<endl;
            exit(0);
        }
        else{
            cout<<"Success creating socket!"<<endl;
        }
        
        //get the server add 
        struct hostent* hret=gethostbyname(argv[1]);
        struct sockaddr_in server_address;
        bzero(&server_address,sizeof(server_address));
        server_address.sin_family=AF_INET;
        server_address.sin_port= htons(atoi(argv[2]));
        memcpy(&server_address.sin_addr.s_addr, hret->h_addr,hret->h_length);
        
        //create fd_set 
        fd_set master;
	    fd_set read_fds;
	    FD_ZERO(&read_fds);
	    FD_ZERO(&master);
	    
	    //connect to the chatroom
	    if(connect(sockfd,(struct sockaddr *)&server_address,sizeof(server_address))!=0)
	    {
	        cout<<"connection failed!" << (int) errno<<endl;
	        exit(0);
	    }
	    else
	    {
	        cout<<"connection succeeds!"<<endl;
	        join(sockfd,argv);
	        FD_SET(sockfd, &master);
	        FD_SET(STDIN_FILENO, &master);
	    }
	    while(true){
	        read_fds=master;
	       // cout<<"\n"<<endl;
	        
	        //select fail
	        if(select(sockfd+1,&read_fds,NULL,NULL,NULL)==-1){
	            perror("select");
	            exit(4);
	        }
	        //change in sockfd
	        if(FD_ISSET(sockfd,&read_fds)){
	            readMessage(sockfd);
	        }
	        //change in stdin
	        if(FD_ISSET(STDIN_FILENO,&read_fds)){
	            send(sockfd);
	            
	        }
	        
	    }
	    cout<<"chat ends"<<endl;
	    
    }
    cout<<"client close"<<endl;
    return 0;
}

	   
	        
	       