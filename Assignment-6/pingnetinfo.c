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

#define MAX_PACKET_SIZE 64
#define h_addr h_addr_list[0]

// clock_t start_time, end_time;
struct timeval start_time, end_time;
   





unsigned short in_cksum(unsigned short *ptr, int nbytes);
void receive_packet(int sockfd);
void send_packet(int sockfd, char *ip_addr, char *data);

int main(int argc, char *argv[]) 
{
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
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(optval)) < 0) {
        perror("setsockopt() error");
        exit(EXIT_FAILURE);
    }

    char *temp = argv[1];
    char *ip_addr = (char *)malloc(strlen(temp)+1);
    strcpy(ip_addr, temp);
    char *msg = "100 Length String";
    send_packet(sockfd, ip_addr, msg);
    printf("Packet Sent!\n"); fflush(stdout);

    receive_packet(sockfd);
}

void setICMP(char *buf, char *data, int size) 
{
    struct icmphdr *icmp_hdr = (struct icmphdr *)buf;
    icmp_hdr->type = ICMP_ECHO;
    icmp_hdr->code = 0;
    icmp_hdr->un.echo.id = getpid();
    icmp_hdr->un.echo.sequence = 1;
    icmp_hdr->checksum = 0;
    memcpy(buf+sizeof(struct icmphdr), data, strlen(data)+1);

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


void send_packet(int sockfd, char *ip_addr, char *data) 
{
    struct hostent *he = gethostbyname(ip_addr);
    if (he == NULL) {
        perror("gethostbyname"); return;
    }

    // print host ip
    printf("Sending to IP: %s\n", inet_ntoa(*((struct in_addr *)he->h_addr)));

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = *((struct in_addr *)he->h_addr);

    char packet[MAX_PACKET_SIZE];
    struct iphdr *ip_hdr = (struct iphdr *)packet;
    setIP(ip_hdr, &dest_addr, 7);

    

    setICMP(packet+4*ip_hdr->ihl, data, MAX_PACKET_SIZE-4*ip_hdr->ihl);

    //start_time = clock();
    if (sendto(sockfd, packet, MAX_PACKET_SIZE, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto"); return;
    }
    // record start time
    gettimeofday(&start_time, NULL);

    printf("Destination IP: %s\n", inet_ntoa(dest_addr.sin_addr));
}

void receive_packet(int sockfd) 
{

    char buf[MAX_PACKET_SIZE];
    struct sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);

    
    int bytes = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&src_addr, &src_addr_len);
    if (bytes < 0) {
        perror("recvfrom error");
        return;
    }
    //end_time = clock();

        // record end time
    gettimeofday(&end_time, NULL);

    // calculate time taken in microseconds
    long time_taken = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);

    printf("Time taken by the function: %ld microseconds\n", time_taken);

    // int time_taken = (end_time - start_time)/(CLOCKS_PER_SEC/1000000);
    // printf("\nReceived %d bytes from %s  Time Taken = %d microseconds\n", bytes, inet_ntoa(src_addr.sin_addr), time_taken);

    struct iphdr *ip_hdr = (struct iphdr *)buf;
    struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + (ip_hdr->ihl * 4));

    char *data = buf + (ip_hdr->ihl * 4) + sizeof(struct icmphdr);
    printf("Received Message: %s\n", data);

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

