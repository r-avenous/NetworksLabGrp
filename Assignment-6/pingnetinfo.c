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

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("PingNetInfo takes 3 arguments: \n1. a site to probe (can be either a name like cse.iitkgp.ac.in or an IP address like 10.3.45.6), \n2. the number of times a probe will be sent per link, and \n3. the time difference between any two probes\n");
        return 1;
    }
    int num_probes = atoi(argv[2]), interval = atoi(argv[3]);
    struct in_addr targetAddress;
    // check if the input is an IP address or a hostname
    if (inet_aton(argv[1], &targetAddress) == 1)
    {
        printf("The input is an IP address\n");
    }
    else
    {
        // if the input is not an IP address, it must be a hostname or a domain name
        // use gethostbyname() to resolve the hostname
        struct hostent *host = gethostbyname(argv[1]);
        if (host == NULL)
        {
            printf("Could not resolve hostname\n");
            return 0;
        }
        // get the IP address from the hostent struct
        targetAddress = *(struct in_addr *)host->h_addr_list[0];
        // print the IP address
        printf("The IP address is %s \n", inet_ntoa(targetAddress));
    }
    // create raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    return 0;
}