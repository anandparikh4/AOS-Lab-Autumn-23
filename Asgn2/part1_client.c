#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){
    struct sockaddr_in serv_addr;
    char buff[1];
    int sockfd;
    int i;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){       //creates a client side socket
        perror("socket error : socket not created\n");     
        exit(0);
    }

    serv_addr.sin_family = AF_INET;                         // filling the feilds of server address
    inet_aton("172.17.0.1", &serv_addr.sin_addr); // CHANGE IT
    serv_addr.sin_port = htons(20000);

    if(connect(sockfd, (struct sockaddr *)& serv_addr, sizeof(serv_addr))<0){       // connecting to the server having serv_addr address
        perror("connect error : unable to connect to server\n");
        exit(0);
    }

    while(1){
        for(int i=0;i<26;i++){
            buff[0] = 'a'+i;
            if(send(sockfd, buff, 1, 0)!=1){                // sending the alphabets to the server
                perror("send error : sent bytes mismatch\n");
                exit(0);
            }
            else{
                printf("sent %c\n", buff[0]);
            }
            sleep(1);
        }
        for(int i=0;i<26;i++){
            buff[0] = 'A'+i;
            if(send(sockfd, buff, 1, 0)!=1){                // sending the alphabets to the server
                perror("send error : sent bytes mismatch\n");
                exit(0);
            }
            else{
                printf("sent %c\n", buff[0]);
            }
            sleep(1);
        }
    }

    close(sockfd);      // closing the socket
    
    return 0;
}