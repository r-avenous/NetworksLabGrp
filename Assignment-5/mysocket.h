#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAXLINE 1000
#define MAXMSGSIZE 5000
#define SOCK_MyTCP 100

int my_socket(int domain, int type, int protocol);
void my_close(int sockfd);
int my_bind(int sockfd, struct sockaddr *addr_ptr, size_t addr_size);
int my_listen(int sockfd, int client_count);
int my_accept(int sockfd, struct sockaddr* cliaddr, int* clilen);
int my_connect(int sockfd, struct sockaddr* servaddr, int servlen);
int my_send(int sockfd, char* msg, int len, int flag);
int my_recv(int sockfd, char* buf, int len, int flag);
void* SThread(void* arg);
void* RThread(void* arg);
#endif