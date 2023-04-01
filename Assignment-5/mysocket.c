#include "mysocket.h"

#define TIMEOUT 0.1

int min(int a, int b) {return (a<b)?a:b;}
char *Send_Message[10], *Recieved_Message[10];
int Send_Message_Size[10], Recieved_Message_Size[10];
pthread_t R, S;
pthread_mutex_t R_Mutex, S_Mutex;
pthread_cond_t send_cond, recv_cond_empt, recv_cond_full;
int sr_socket, send_counter, recv_counter, send_in, send_out, recv_in, recv_out;

int my_socket(int domain, int type, int protocol)
{
    if(type != SOCK_MyTCP) return -1;
    send_counter = 0; recv_counter = 0;
    send_in = 0; send_out = 0;
    recv_in = 0; recv_out = 0;
    sr_socket = -1;

    for(int i=0; i<10; i++)
    {
        Send_Message[i] = (char*)malloc(MAXMSGSIZE);
        Recieved_Message[i] = (char*)malloc(MAXMSGSIZE);
    }

    pthread_create(&R, NULL, RThread, NULL);
    pthread_create(&S, NULL, SThread, NULL);
    pthread_mutex_init(&R_Mutex, NULL);
    pthread_mutex_init(&S_Mutex, NULL);
    pthread_cond_init(&recv_cond_empt, NULL);
    pthread_cond_init(&send_cond, NULL);
    pthread_cond_init(&recv_cond_full, NULL);
    return socket(domain, SOCK_STREAM, protocol);
}

int my_bind(int sockfd, struct sockaddr *addr_ptr, size_t addr_size)
{
    int status = bind(sockfd, addr_ptr, addr_size);
    return status;
}

int my_listen(int sockfd, int client_count)
{
    return listen(sockfd, client_count);
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

void connection_close_check(int c)
{
    if(c) return;
    sr_socket = -1;
    printf("Connection closed\n");
    pthread_mutex_unlock(&R_Mutex);
    pthread_cond_signal(&recv_cond_empt);
    pthread_cond_signal(&send_cond);
    pthread_cond_signal(&recv_cond_full);
  
    pthread_cancel(S);
    pthread_exit(NULL);
}
void* RThread(void* arg)
{   
    while(sr_socket==-1);
    while(1)
    {
        int count=0;
        unsigned char *msg_len = (unsigned char*)malloc(2);
        count = recv(sr_socket, msg_len, 2, 0);
        connection_close_check(count);
        if(count == -1)
        {
            perror("Error in receiving message length: -1");
            exit(1);
        }
        if(count == 1)
        {
            int count2 = recv(sr_socket, msg_len+1, 1, 0);
            connection_close_check(count2);
            count += count2;
        }
        // printf("%d, %s\n", count, msg_len);
        if(count != 2)
        {
            perror("Error in receiving message length");
            exit(1);
        }

        int len = 256*(int)msg_len[0] + (int)msg_len[1];
        free(msg_len);

        pthread_mutex_lock(&R_Mutex);
        while(recv_counter == 10) 
            pthread_cond_wait(&recv_cond_full, &R_Mutex);
        
        if(recv_counter < 10)
        {
            int rec_len = 0;
            while(rec_len<len)
            {
                count = recv(sr_socket, Recieved_Message[recv_in]+rec_len, min(len-rec_len, MAXLINE), 0);
                connection_close_check(count);
                if(count == -1)
                {
                    perror("Error in receiving message");
                    exit(1);
                }
                rec_len += count;
            }
            Recieved_Message_Size[recv_in] = len;
            recv_in = (recv_in+1)%10;
            recv_counter++;
        }

        if(recv_counter == 1) 
            pthread_cond_signal(&recv_cond_empt);
        pthread_mutex_unlock(&R_Mutex);
    }
    return NULL;
}


void* SThread(void* arg)
{
    while(sr_socket==-1);
    while(1)
    { 
        sleep(TIMEOUT);
        pthread_mutex_lock(&S_Mutex);
        if(send_counter==0)
        {
            pthread_mutex_unlock(&S_Mutex);
            continue;
        }

        char *msg = Send_Message[send_out];
        int len = Send_Message_Size[send_out];

        unsigned char *msg_len = (unsigned char*)malloc(2);
        msg_len[0] = len/256;
        msg_len[1] = len%256;
        
        int count;
        count = send(sr_socket, msg_len, 2, 0);
        if(count != 2)
        {
            perror("Error in sending message length");
            exit(1);
        }
        while(len)
        {
            count = send(sr_socket, msg, min(len, MAXLINE), 0);
            if(count == -1){
                perror("Error in sending message");
                exit(1);
            }
            len -= count;
            msg += count;
        }
        free(msg_len);
        send_out = (send_out+1)%10;
        send_counter--;
        pthread_cond_signal(&send_cond);
        pthread_mutex_unlock(&S_Mutex);
        

    }
    return NULL;
}

int my_send(int sockfd, char* msg, int len, int flag)
{
    if(sockfd != sr_socket) return 0;
    int l = 0;
    pthread_mutex_lock(&S_Mutex);
    while(send_counter == 10 && sr_socket!=-1) 
        pthread_cond_wait(&send_cond, &S_Mutex);
    if(send_counter < 10)
    {
        l = min(len, MAXMSGSIZE);

        memcpy(Send_Message[send_in], msg, l);
        Send_Message_Size[send_in] = l;
        send_counter++;
        send_in = (send_in + 1) % 10;
    }
    pthread_mutex_unlock(&S_Mutex);
    return l;
}

int my_recv(int sockfd, char* buf, int len, int flag)
{
    if(sockfd != sr_socket) return 0;
    int l = 0;
    pthread_mutex_lock(&R_Mutex);
    while(recv_counter == 0 && sr_socket!=-1) 
        pthread_cond_wait(&recv_cond_empt, &R_Mutex);
    if(recv_counter)
    {
        l = min(len, MAXMSGSIZE);
        memcpy(buf, Recieved_Message[recv_out], l);
        recv_out = (recv_out+1)%10;
        recv_counter--;
    }
    
    if(recv_counter == 9) 
        pthread_cond_signal(&recv_cond_full);
    pthread_mutex_unlock(&R_Mutex);
    return l;
}

void my_close(int sockfd)
{
    if(sockfd != sr_socket) return;
    sleep(5);
    pthread_cancel(R);
    pthread_cancel(S);
    // for(int i=0; i<10; i++) free(Send_Message[i]);
    pthread_mutex_destroy(&R_Mutex);
    pthread_mutex_destroy(&S_Mutex);
    pthread_cond_destroy(&send_cond);
    pthread_cond_destroy(&recv_cond_empt);
    pthread_cond_destroy(&recv_cond_full);

    for(int i=0; i<10; i++) 
    {
        free(Send_Message[i]);
        free(Recieved_Message[i]);
    }
    close(sockfd);
    
}