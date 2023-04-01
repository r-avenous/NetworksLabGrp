#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define PACKET_SIZE 64

// Calculate checksum of a buffer
unsigned short checksum(unsigned short *buf, int nwords) {
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--) {
        sum += *buf++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[]) {
    int sockfd, n;
    char packet[PACKET_SIZE];
    struct sockaddr_in addr;
    struct iphdr *ip = (struct iphdr *)packet;
    struct icmphdr *icmp = (struct icmphdr *)(packet + sizeof(struct iphdr));

    // Create raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // Set destination address
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Fill in IP header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    ip->id = htons(12345);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_ICMP;
    ip->saddr = INADDR_ANY;
    ip->daddr = addr.sin_addr.s_addr;

    // Fill in ICMP header
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = getpid() & 0xFFFF;
    icmp->un.echo.sequence = 0;
    icmp->checksum = 0;
    icmp->checksum = checksum((unsigned short *)icmp, sizeof(struct icmphdr));

    // Send packet
    n = sendto(sockfd, packet, ip->tot_len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if (n < 0) {
        perror("sendto");
        exit(1);
    }
    char buffer[1024];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    ssize_t packet_len = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&sender, &sender_len);

    // Close socket
    close(sockfd);

    return 0;
}

// 203.110.245.244
