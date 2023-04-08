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
#include <poll.h>

#define MAX_PACKET_SIZE 576
#define h_addr h_addr_list[0]

// clock_t start_time, end_time;
struct timeval start_time, end_time;
struct timespec start, end;   


unsigned short in_cksum(unsigned short *ptr, int nbytes);
long receive_packet(int sockfd, char *add, int *icmp_reply);
int send_packet(int sockfd, char *data, struct sockaddr_in dest_addr, int ttl);

#include "helper_functions.h"

int T = 1, n = 40;

void getMinRTT(int sockfd,  char *destIP, long *empty_RTT, long *data_RTT){
    char chunk[500];
    memset(chunk, 'a', 499); chunk[499] = '\0';

    char small_chunk[64];
    memset(small_chunk, 'a', 63); small_chunk[63] = '\0';

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(destIP);

    *empty_RTT = 1e9; *data_RTT = 1e9;
    char currIP[20];
    int icmp_type;
    for(int i=0; i<n; ++i){
        
        send_packet(sockfd, chunk, dest_addr, 64);
        *data_RTT = min(receive_packet(sockfd, currIP, &icmp_type), *data_RTT);
        send_packet(sockfd, NULL, dest_addr, 64);
        *empty_RTT = min(receive_packet(sockfd, currIP, &icmp_type), *empty_RTT);
        usleep(T*1000);  //sleep for T miliseconds
    }
    

}

int main(int argc, char *argv[]) 
{
    if (argc != 4) {
        printf("Usage: %s <ip_address> <n> <T>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[2]);
    T = atoi(argv[3]);

    int sockfd = create_socket();
    struct sockaddr_in dest_addr = getDestAddr(argv[1]);

    long prev_empty_RTT=0, prev_data_RTT=0;
    long curr_empty_RTT, curr_data_RTT, link_empty_RTT, link_data_RTT;
    char currIP[20], prevIP[20], prevHop[20];
    // get IP address of this host
    sprintf(prevHop, "This Host");
    int icmp_type;

    char results[100000];
    sprintf(results, "\n");

    for(int ttl = 1; ttl <= 64; ttl++)
    {
        int counter = 0, errorCounter = 0;
        while(1)
        {
            if(errorCounter == 5)
            {
                printf(" * * *\n");
                exit(0);
            }
            if(send_packet(sockfd, NULL, dest_addr, ttl) < 0)
            {
                // printf("* * *\n");
                errorCounter++;
                continue;
            }
            if(receive_packet(sockfd, currIP, &icmp_type) < 0)
            {
                // printf("* * *\n");
                errorCounter++;
                continue;
            }
            if(icmp_type == ICMP_ECHOREPLY)
            {
                break;
            }
            if(strcmp(currIP, prevIP) == 0)
            {
                counter++;
                if(counter == 5)
                {
                    break;
                }
            }
            else
            {
                counter = 1;
                strcpy(prevIP, currIP);
            }
            // usleep(1000);
            sleep(1);
        }
        sprintf(results + strlen(results), "Hop %d: %s\t\nLink:\t%s\t->\t%s\t|\t", ttl, currIP, prevHop, currIP);
        

        getMinRTT(sockfd, currIP, &curr_empty_RTT, &curr_data_RTT);


        // printf("noDataRTT: %ld\t", curr_empty_RTT);
        // printf("withDataRTT: %ld\t", curr_data_RTT);

        link_empty_RTT = curr_empty_RTT - prev_empty_RTT;
        link_data_RTT = curr_data_RTT - prev_data_RTT;
        
        
        sprintf(results + strlen(results), "Latency: %lf ms\t", (link_empty_RTT/1000.0)/2);
        sprintf(results + strlen(results), "Bandwidth: %lf Mbps\t\n", (1024*8.0*2)/(link_data_RTT-link_empty_RTT));
        
        print_ICMP_type(icmp_type);
        printf("\n");

        prev_empty_RTT = curr_empty_RTT;
        prev_data_RTT = curr_data_RTT;

        strcpy(prevHop, currIP);
        
        if(icmp_type==ICMP_ECHOREPLY) break;
    }

    printf("\n%s\n", results);

    close(sockfd);

}


