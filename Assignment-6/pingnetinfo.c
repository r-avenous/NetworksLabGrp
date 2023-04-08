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

#define MAX_PACKET_SIZE 1500
#define h_addr h_addr_list[0]

// clock_t start_time, end_time;
struct timeval start_time, end_time;
struct timespec start, end;   


unsigned short in_cksum(unsigned short *ptr, int nbytes);
long receive_packet(int sockfd, char *add, int *icmp_reply);
void send_packet(int sockfd, char *data, struct sockaddr_in dest_addr, int ttl);

#include "helper_functions.h"



void getAvgRTT(int sockfd,  char *destIP, long *empty_RTT, long *data_RTT){
    char chunk[1024];
    memset(chunk, 'a', 1023); chunk[1023] = '\0';

    char small_chunk[64];
    memset(small_chunk, 'a', 63); small_chunk[63] = '\0';

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(destIP);

    *empty_RTT = 0; *data_RTT = 0;
    char currIP[20];
    int icmp_type;
    for(int i=0; i<5; ++i){
        send_packet(sockfd, NULL, dest_addr, 64);
        *empty_RTT += receive_packet(sockfd, currIP, &icmp_type);
        
        send_packet(sockfd, chunk, dest_addr, 64);
        *data_RTT += receive_packet(sockfd, currIP, &icmp_type);
        usleep(10000);  //sleep for 10 miliseconds
    }

    *empty_RTT /= 5;
    *data_RTT /= 5;
    

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
        

        getAvgRTT(sockfd, currIP, &curr_empty_RTT, &curr_data_RTT);


        printf("noDataRTT: %ld\t", curr_empty_RTT);
        printf("withDataRTT: %ld\t", curr_data_RTT);

        link_empty_RTT = curr_empty_RTT - prev_empty_RTT;
        link_data_RTT = curr_data_RTT - prev_data_RTT;
        
        
        printf("Latency: %lf ms\t", link_empty_RTT/1000.0);
        printf("Bandwidth: %lf Mbps\t", (1024*8.0)/(link_data_RTT-link_empty_RTT));
        
        print_ICMP_type(icmp_type);
        printf("\n");

        prev_empty_RTT = curr_empty_RTT;
        prev_data_RTT = curr_data_RTT;


        
        if(icmp_type==ICMP_ECHOREPLY) break;
    }

    close(sockfd);

}


