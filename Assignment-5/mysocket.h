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
void my_close();
int my_bind(int sockfd, (struct sockaddr *)addr_ptr, size_t addr_size);

#endif