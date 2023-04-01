#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PROBES 5
#define DEFAULT_INTERVAL 1

struct ip_packet
{
    struct icmphdr icmp_header;
    struct timeval timestamp;
    char data[BUFFER_SIZE - sizeof(struct icmphdr) - sizeof(struct timeval)];
};

unsigned short calculate_checksum(void *buffer, int length);
int resolve_address(const char *address, struct in_addr *addr);
void print_address(const struct in_addr *addr);
double time_diff(const struct timeval *start, const struct timeval *end);
int send_packet(int sockfd, const struct sockaddr_in *dest_addr, const char *buffer, int size);
int receive_packet(int sockfd, struct sockaddr_in *src_addr, char *buffer, int buffer_size, struct timeval *timestamp);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <hostname or IP address> [number of probes] [interval]\n", argv[0]);
        return 1;
    }

    const char *hostname_or_ip = argv[1];
    int num_probes = DEFAULT_PROBES;
    int interval = DEFAULT_INTERVAL;
    if (argc > 2)
    {
        num_probes = atoi(argv[2]);
    }
    if (argc > 3)
    {
        interval = atoi(argv[3]);
    }

    struct in_addr address;
    if (!resolve_address(hostname_or_ip, &address))
    {
        fprintf(stderr, "Could not resolve address\n");
        return 1;
    }

    printf("Probing %s [%s]\n", hostname_or_ip, inet_ntoa(address));

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = 0,
        .sin_addr = address};

    char buffer[BUFFER_SIZE];

    for (int ttl = 1; ttl <= 64; ttl++)
    {
        printf("%2d ", ttl);

        struct timeval start_time;
        gettimeofday(&start_time, NULL);

        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        int done = 0;
        int probes_received = 0;
        double rtt_sum = 0.0;
        double rtt_min = INFINITY;
        double rtt_max = 0.0;
        int num_success = 0;

        for (int i = 0; i < num_probes && !done; i++)
        {
            struct timeval timestamp;
            int packet_size = sizeof(struct ip_packet) + i * 8;

            memset(buffer, 0, sizeof(buffer));

            struct ip_packet *packet = (struct ip_packet *)buffer;

            packet->icmp_header.type = ICMP_ECHO;
            packet->icmp_header.code = 0;
            packet->icmp_header.un.echo.id = getpid() & 0xFFFF;

            for (int j = 0; j < packet_size - sizeof(struct icmphdr) - sizeof(struct timeval); j++)
            {
                packet->data[j] = j;
            }

            gettimeofday(&timestamp, NULL);
            packet->icmp_header.checksum = calculate_checksum(packet, packet_size);

            if (send_packet(sockfd, &dest_addr, buffer, packet_size) < 0)
            {
                printf("* ");
                continue;
            }

            struct sockaddr_in src_addr;
            struct timeval receive_time;

            int bytes_received = receive_packet(sockfd, &src_addr, buffer, BUFFER_SIZE, &receive_time);
            if (bytes_received < 0)
            {
                printf("* ");
                continue;
            }

            double rtt = time_diff(&timestamp, &receive_time) * 1000;
            if (rtt < rtt_min)
            {
                rtt_min = rtt;
            }
            if (rtt > rtt_max)
            {
                rtt_max = rtt;
            }
            rtt_sum += rtt;
            probes_received++;
            num_success++;

            printf("%.1fms ", rtt);

            if (src_addr.sin_addr.s_addr == address.s_addr)
            {
                done = 1;
            }

            sleep(interval);
        }

        if (probes_received == 0)
        {
            printf(" * * *");
        }
        else
        {
            double rtt_avg = rtt_sum / probes_received;
            printf("%.1fms %.1fms %.1fms", rtt_min, rtt_avg, rtt_max);
            if (num_success < num_probes)
            {
                printf(" %d/%d packets received", num_success, num_probes);
            }
        }

        printf("\n");

        if (done)
        {
            break;
        }
    }

    close(sockfd);

    return 0;
}

unsigned short calculate_checksum(void *buffer, int length)
{
    unsigned short *buf = (unsigned short *)buffer;
    unsigned int sum = 0;
    unsigned short result;
    for (int i = 0; i < length / 2; i++)
    {
        sum += buf[i];
    }

    if (length % 2)
    {
        sum += ((unsigned char *)buffer)[length - 1];
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

// int resolve_address(const char *address, struct in_addr *addr)
// {
//     struct addrinfo *info;
//     int ret;
//     if ((ret = getaddrinfo(address, NULL, NULL, &info)) != 0)
//     {
//         return 0;
//     }

//     *addr = ((struct sockaddr_in *)info->ai_addr)->sin_addr;

//     freeaddrinfo(info);

//     return 1;
// }

int resolve_address(const char *address, struct in_addr *addr)
{
    struct hostent *he;
    if ((he = gethostbyname(address)) == NULL) {
        return 0;
    }
    // memcpy(addr, he->h_addr, he->h_length);
    memcpy(addr, he->h_addr_list[0], he->h_length);

    return 1;
}


void print_address(const struct in_addr *addr)
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, addr, buf, sizeof(buf));
    printf("%s", buf);
}

double time_diff(const struct timeval *start, const struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + (end->tv_usec - start->tv_usec) / 1000000.0;
}

int send_packet(int sockfd, const struct sockaddr_in *dest_addr, const char *buffer, int size)
{
    int bytes_sent = sendto(sockfd, buffer, size, 0, (const struct sockaddr *)dest_addr, sizeof(struct sockaddr_in));
    if (bytes_sent < 0)
    {
        perror("sendto");
        return -1;
    }

    return bytes_sent;
}

int receive_packet(int sockfd, struct sockaddr_in *src_addr, char *buffer, int buffer_size, struct timeval *timestamp)
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);

    struct timeval timeout = {
        .tv_sec = 1,
        .tv_usec = 0};

    int max_fd = sockfd + 1;

    int ready = select(max_fd, &read_fds, NULL, NULL, &timeout);
    if (ready == -1)
    {
        perror("select");
        return -1;
    }
    else if (ready == 0)
    {
        return 0;
    }

    ssize_t num_bytes = recvfrom(sockfd, buffer, buffer_size, 0, (struct sockaddr *)src_addr, &(socklen_t){sizeof(*src_addr)});
    if (num_bytes < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
        {
            return 0;
        }
        perror("recvfrom");
        return -1;
    }

    if (timestamp != NULL)
    {
        gettimeofday(timestamp, NULL);
    }

    return num_bytes;
}