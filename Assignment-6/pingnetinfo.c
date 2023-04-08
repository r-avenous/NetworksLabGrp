#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>

int MAX_PACKET_SIZE = 28;
#define h_addr h_addr_list[0]

// clock_t start_time, end_time;
// struct timeval start_time, end_time;
struct timespec start, end;   


unsigned short in_cksum(unsigned short *ptr, int nbytes);
long receive_packet(int sockfd, char *add, int *icmp_reply);
void send_packet(int sockfd, char *data, struct sockaddr_in dest_addr, int ttl);

#include "helper_functions.h"



long getAvgRTT(int sockfd, int data_len, char *destIP){
    char *data = NULL;
    if(data_len>0){
        data = (char *)malloc(data_len*sizeof(char));
        memset(data, 'a', data_len);
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(destIP);

    long RTTsum=0, currRTT=0;
    char currIP[20];
    int icmp_type;
    for(int i=0; i<5; ++i){
        send_packet(sockfd, data, dest_addr, 64);
        currRTT = receive_packet(sockfd, currIP, &icmp_type);
        RTTsum += currRTT;
        usleep(10000);  //sleep for 10 miliseconds
    }
    return RTTsum/5;
    

}

int T = 1, n = 5;
int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        printf("Usage: %s <ip_address>\n", argv[0]);
        return 1;
    }


    int sockfd = create_socket();
    struct sockaddr_in dest_addr = getDestAddr(argv[1]);

    long prev_empty_RTT=0, prev_data_RTT=0;
    long curr_empty_RTT, curr_data_RTT, link_empty_RTT, link_data_RTT;
    char currIP[20];
    int icmp_type;

    for(int ttl = 1; ttl <= 64; ttl++)
    {
        send_packet(sockfd, NULL, dest_addr, ttl);
        receive_packet(sockfd, currIP, &icmp_type);
        printf("Hop %d: %s\t", ttl, currIP);
        

        curr_empty_RTT = getAvgRTT(sockfd, 0, currIP);
        curr_data_RTT = getAvgRTT(sockfd, 1024, currIP);

        printf("noDataRTT: %ld\t", curr_empty_RTT);
        printf("withDataRTT: %ld\t", curr_data_RTT);

        link_empty_RTT = curr_empty_RTT - prev_empty_RTT;
        link_data_RTT = curr_data_RTT - prev_data_RTT;
        
        
        printf("Latency: %lf\t", link_empty_RTT/1000.0);
        printf("Bandwidth: %lf\t", (1024*8*1000.0)/(link_data_RTT-link_empty_RTT));
        
        print_ICMP_type(icmp_type);
        printf("\n");

        prev_empty_RTT = curr_empty_RTT;
        prev_data_RTT = curr_data_RTT;


        
        if(icmp_type==ICMP_ECHOREPLY) break;
    }

    close(sockfd);

}


