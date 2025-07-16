#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DELAY 1

int main(){
    struct sockaddr_in serv_addr;
    int buff_int;
    int sockfd;
    int i;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){       //creates a client side socket
        perror("socket error : socket not created\n");     
        exit(0);
    }

    serv_addr.sin_family = AF_INET;                         // filling the feilds of server address
    inet_aton("172.17.0.1", &serv_addr.sin_addr); //CHANGE IT
    serv_addr.sin_port = htons(20000);

    if(connect(sockfd, (struct sockaddr *)& serv_addr, sizeof(serv_addr))<0){       // connecting to the server having serv_addr address
        perror("connect error : unable to connect to server\n");
        exit(0);
    }

    while(1){
        buff_int = DELAY;
        if(send(sockfd, buff_int, sizeof(int), 0)!=sizeof(int)){                // sending a sleep time to the server
            perror("send error : sent bytes mismatch\n");
            exit(0);
        }
        else{
            printf("sent %d\n", buff_int);
        }
        sleep(1);
    }

    close(sockfd);      // closing the socket
    
    return 0;
}