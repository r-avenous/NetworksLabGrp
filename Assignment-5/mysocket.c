#include "mysocket.h"

#define TIMEOUT 1

char *Send_Message[10], Recieved_Message[10][MAXMSGSIZE];
pthread_t R, S;
pthread_mutex_t R_Mutex, S_Mutex;
pthread_cond_t send_cond, recv_cond;
int sr_socket, send_counter, recv_counter, send_in, send_out, recv_in, recv_out;

int my_socket(int domain, int type, int protocol)
{
    if(type != SOCK_MyTCP) return -1;
    send_counter = 0;
    recv_counter = 0;
    for(int i=0; i<10; i++)
    {
        Send_Message[i] = NULL;
    }
    pthread_create(&R, NULL, RThread, NULL);
    pthread_create(&S, NULL, SThread, NULL);
    pthread_mutex_init(&R_Mutex, NULL);
    pthread_mutex_init(&S_Mutex, NULL);
    pthread_cond_init(&recv_cond, NULL);
    pthread_cond_init(&send_cond, NULL);
    return socket(domain, SOCK_STREAM, protocol);
}

int my_bind(int sockfd, struct sockaddr *addr_ptr, size_t addr_size){
    int status = bind(sockfd, addr_ptr, addr_size);
    return status;
}

int my_listen(int sockfd, int client_count)
{
    listen(sockfd, client_count);
}

int my_accept(int sockfd, struct sockaddr* cliaddr, int* clilen)
{
    int newsockfd = accept(sockfd, cliaddr, clilen);
    sr_socket = newsockfd;
    return newsockfd;
}

int my_connect(int sockfd, struct sockaddr* servaddr, int servlen)
{
    sr_socket = sockfd;
    return connect(sockfd, servaddr, servlen);
}

void* RThread(void* arg)
{
    return NULL;
}

void* SThread(void* arg)
{
    return NULL;
}

int my_send(int sockfd, char* msg, int len, int flag)
{
    if(sockfd != sr_socket) return -1;
    int l = 0;
    pthread_mutex_lock(&S_Mutex);
    if(send_counter == 10) pthread_cond_wait(&send_cond, &S_Mutex);
    if(send_counter < 10)
    {
        l = min(len, MAXMSGSIZE);
        Send_Message[send_in] = (char*)malloc(l);
        memcpy(Send_Message[send_in], msg, l);
        send_counter++;
        send_in = (send_in + 1) % 10;
    }
    pthread_mutex_unlock(&S_Mutex);
    return l;
}

int my_recv(int sockfd, char* buf, int len, int flag)
{
    if(sockfd != sr_socket) return -1;
    int l = 0;
    pthread_mutex_lock(&R_Mutex);
    if(recv_counter == 0) pthread_cond_wait(&recv_cond, &R_Mutex);
    if(recv_counter)
    {
        l = min(len, MAXMSGSIZE);
        memcpy(buf, Recieved_Message[recv_out], l);
        recv_out = (recv_out+1)%10;
        recv_counter--;
    }
    pthread_mutex_unlock(&R_Mutex);
    return l;
}

void my_close()
{
    pthread_cancel(R);
    pthread_cancel(S);
    for(int i=0; i<10; i++) free(Send_Message[i]);
    pthread_mutex_destroy(&R_Mutex);
    pthread_mutex_destroy(&S_Mutex);
    pthread_cond_destroy(&send_cond);
    pthread_cond_destroy(&recv_cond);
    
}