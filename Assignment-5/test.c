#include "mysocket.h"
#include <stdio.h>
#define PORT 20001
int main()
{
    if(fork() == 0)
    {
        sleep(1);
        int			sockfd ;
        struct sockaddr_in	serv_addr;

        int i;
        char buf[100];

        if ((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0)) < 0) {
            perror("Unable to create socket\n");
            exit(0);
        }
        serv_addr.sin_family	= AF_INET;
        inet_aton("127.0.0.1", &serv_addr.sin_addr);
        serv_addr.sin_port	= htons(20001);
        if ((my_connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
            perror("Unable to connect to server\n");
            exit(0);
        }
        for(i=0; i < 100; i++) buf[i] = '\0';
        my_recv(sockfd, buf, 100, 0);
        printf("%s\n", buf);
            
        my_close(sockfd);
        return 0;
    }
    int sockfd = my_socket(AF_INET, SOCK_MyTCP, 0);
    struct sockaddr_in serv_addr, cli_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20001);
    my_bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(sockfd, 1);
    int clilen = sizeof(cli_addr);
    int newsockfd = my_accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    char buf[100];
    strcpy(buf,"Message from server");
	my_send(newsockfd, buf, strlen(buf) + 1, 0);
    my_close(newsockfd);
    return 0;
}