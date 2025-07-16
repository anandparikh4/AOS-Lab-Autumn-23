#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

			/* THE SERVER PROCESS */

int sockfd;
struct sockaddr_in lb_addr;


void *thread_func(void *arg){
    int clilen;
    struct sockaddr_in	cli_addr;
    while(1){
        clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
                    &clilen) ;

        if (newsockfd < 0) {
            printf("Accept error\n");
            exit(0);
        }
        int slp_time;
        if(recv(newsockfd, slp_time, sizeof(int), 0)<0){
            perror("receive error : correct message not received\n");
            exit(0);
        }
        close(newsockfd);
        printf("Received %d . Now sleeping for %d seconds\n", slp_time, slp_time);
        sleep(slp_time);
        printf("Sleep over, sending packet to load balancer\n");
        int temp_sockfd;
        if ((temp_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Cannot create socket\n");
            exit(0);
        }
        if(connect(temp_sockfd, (struct sockaddr *)& lb_addr, sizeof(lb_addr))<0){       // connecting to the server having serv_addr address
            perror("connect error : unable to connect to server\n");
            exit(0);
        }
        if(send(temp_sockfd, slp_time, sizeof(int), 0)!=sizeof(int)){                // sending a sleep time to the server
            perror("send error : sent bytes mismatch\n");
            exit(0);
        }
        else{
            printf("sent message to load balancer\n");
        }
        close(temp_sockfd);
    }
    pthread_exit(NULL);
}

int main()
{
	// int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);

    lb_addr.sin_family = AF_INET;
    inet_aton("172.17.0.1", &lb_addr.sin_addr); //CHANGE IT
    lb_addr.sin_port = htons(20000);



	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); 

    pthread_t thread_id[5];

    for(int i=0;i<5;i++){
        pthread_create(&thread_id[i], NULL, thread_func, NULL);
    }
    for(int i=0;i<5;i++){
        pthread_join(thread_id[i], NULL);
    }
	
    close(sockfd);
	return 0;
}
			

