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
char *chunk;

// clock_t start_time, end_time;
struct timeval start_time, end_time;
struct timespec start, end;   


unsigned short in_cksum(unsigned short *ptr, int nbytes);
long receive_packet(int sockfd, char *add);
void send_packet(int sockfd, char *data, struct sockaddr_in dest_addr, int ttl);


int main(int argc, char *argv[]) 
{
    chunk = (char *)malloc(1000);
    memset(chunk, 'a', 1000);
    chunk[999] = '\0';

    int T = 1, n = 5;
    if (argc < 2) 
    {
        printf("Usage: %s <ip_address>\n", argv[0]);
        return 1;
    }
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) 
    {
        perror("socket");
        return 1;
    }

    int optval = 1;
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) < 0) 
    {
        perror("setsockopt() error");
        exit(EXIT_FAILURE);
    }

    struct hostent *he = gethostbyname(argv[1]);
    if (he == NULL) 
    {
        perror("gethostbyname");
    }
    printf("Target IP: %s\n\n", inet_ntoa(*((struct in_addr *)he->h_addr)));

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = *((struct in_addr *)he->h_addr);

    char add[100][100], destAdd[100];
    long noDataRTT[100], withDataRTT[100];
    int hop = 0;
    sprintf(destAdd, "%s", inet_ntoa(dest_addr.sin_addr));
    
    for(int i = 0; i < 64; i++)
    {
        send_packet(sockfd, NULL, dest_addr, i+1);
        receive_packet(sockfd, add[hop++]);
        printf("Hop %d: %s\n", i+1, add[hop-1]);
        if(strcmp(add[hop-1], destAdd) == 0) break;
    }
    for(int i=0; i<hop; i++)
    {
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = inet_addr(add[i]);
        send_packet(sockfd, NULL, dest_addr, 64);
        char a[100];
        noDataRTT[i] = receive_packet(sockfd, a);
        send_packet(sockfd, chunk, dest_addr, 64);
        withDataRTT[i] = receive_packet(sockfd, a);
        // printf("a: %s\n", a);
        printf("%ld\n", withDataRTT[i] - noDataRTT[i]);
    }
}

void setICMP(char *buf, char *data, int size) 
{
    struct icmphdr *icmp_hdr = (struct icmphdr *)buf;
    icmp_hdr->type = ICMP_ECHO;
    icmp_hdr->code = 0;
    icmp_hdr->un.echo.id = getpid();
    icmp_hdr->un.echo.sequence = 1;
    icmp_hdr->checksum = 0;
    if(data != NULL) memcpy(buf+sizeof(struct icmphdr), data, strlen(data)+1);
    icmp_hdr->checksum = in_cksum((unsigned short *)icmp_hdr, size);    // 20 is ip header size
}

void setIP(struct iphdr *ip_hdr, struct sockaddr_in *dest_addr, int ttl) 
{
    ip_hdr->ihl = 5;
    ip_hdr->version = 4;
    ip_hdr->tos = 0;
    ip_hdr->tot_len = htons(MAX_PACKET_SIZE);
    ip_hdr->id = htons(0);
    ip_hdr->frag_off = htons(0);
    ip_hdr->ttl = ttl;
    ip_hdr->protocol = IPPROTO_ICMP;
    ip_hdr->check = 0;
    ip_hdr->saddr = INADDR_ANY;
    ip_hdr->daddr = dest_addr->sin_addr.s_addr;
    ip_hdr->check = in_cksum((unsigned short *)ip_hdr, 4*ip_hdr->ihl);
}

void send_packet(int sockfd, char *data, struct sockaddr_in dest_addr, int ttl) 
{
    if(data != NULL) MAX_PACKET_SIZE = 28 + strlen(data) + 1;
    else MAX_PACKET_SIZE = 28;

    char packet[MAX_PACKET_SIZE];
    struct iphdr *ip_hdr = (struct iphdr *)packet;
    setIP(ip_hdr, &dest_addr, ttl);

    

    setICMP(packet+4*ip_hdr->ihl, data, MAX_PACKET_SIZE-4*ip_hdr->ihl);

    //start_time = clock();
    if (sendto(sockfd, packet, MAX_PACKET_SIZE, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto"); return;
    }
    // record start time
    // gettimeofday(&start_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, &start);

    // printf("Destination IP: %s\n", inet_ntoa(dest_addr.sin_addr));
}

long receive_packet(int sockfd, char *add) 
{
    char buf[MAX_PACKET_SIZE];
    struct sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);

    
    int bytes = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&src_addr, &src_addr_len);
    if (bytes < 0) 
    {
        perror("recvfrom error");
        return -1;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    long time_taken = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec)/1000;

    // printf("Time taken by the function: %ld microseconds\n", time_taken);
    
    struct iphdr *ip_hdr = (struct iphdr *)buf;
    struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + (ip_hdr->ihl * 4));

    char *data = buf + (ip_hdr->ihl * 4) + sizeof(struct icmphdr);
    // printf("%d\n", ip_hdr->ihl * 4 + sizeof(struct icmphdr));
    // printf("Received Message: %s\n", data);
    // print sender 
    // printf("Sender IP: %s\n", inet_ntoa(src_addr.sin_addr));
    sprintf(add, "%s", inet_ntoa(src_addr.sin_addr));
    return time_taken;
}

unsigned short in_cksum(unsigned short *ptr, int nbytes) 
{
    unsigned long sum;
    unsigned short oddbyte;
    unsigned short answer;

    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = (unsigned short)~sum;

    return answer;
}

