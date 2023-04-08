
int create_socket()
{
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
    return sockfd;
}

struct sockaddr_in getDestAddr(char *arg){
    // get sedtination ip address
    struct hostent *he = gethostbyname(arg);
    if (he == NULL) {
        perror("gethostbyname");
    }
    char destIP[20];
    strcpy(destIP, inet_ntoa(*((struct in_addr *)he->h_addr)));
    printf("Target IP: %s\n\n", destIP);

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr = *((struct in_addr *)he->h_addr);

    return dest_addr;
}

void print_ICMP_type(int type){
    if(type==ICMP_ECHO) printf("ICMP_ECHO");
    else if(type==ICMP_ECHOREPLY) printf("ICMP_ECHOREPLY");
    else if(type==ICMP_DEST_UNREACH) printf("ICMP_DEST_UNREACH");
    else if(type==ICMP_REDIRECT) printf("ICMP_REDIRECT");
    else if(type==ICMP_TIME_EXCEEDED) printf("ICMP_TIME_EXCEEDED");
    else if(type==ICMP_INFO_REQUEST) printf("ICMP_INFO_REQUEST");
    else if(type==ICMP_INFO_REPLY) printf("ICMP_INFO_REPLY");
    else if(type==ICMP_ADDRESS) printf("ICMP_ADDRESS");
    else if(type==ICMP_ADDRESSREPLY) printf("ICMP_ADDRESSREPLY");
    else printf("UNKNOWN");
}

const char *tos_str(uint8_t tos) {
    static char str[64];
    snprintf(str, sizeof(str), "%s%s%s%s%s%s%s",
        (tos & IPTOS_LOWDELAY) ? "Low-Delay " : "",
        (tos & IPTOS_THROUGHPUT) ? "High-Throughput " : "",
        (tos & IPTOS_RELIABILITY) ? "High-Reliability " : "",
        (tos & IPTOS_MINCOST) ? "Low-Cost " : "",
        (tos & IPTOS_ECN_CE) ? "ECN-Capable Transport " : "",
        (tos & IPTOS_ECN_MASK) == IPTOS_ECN_ECT0 ? "ECT(0) " :
        (tos & IPTOS_ECN_MASK) == IPTOS_ECN_ECT1 ? "ECT(1) " :
        (tos & IPTOS_ECN_MASK) == IPTOS_ECN_CE ? "CE " : "",
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS0 ? "CS0 " :
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS1 ? "CS1 " :
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS2 ? "CS2 " :
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS3 ? "CS3 " :
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS4 ? "CS4 " :
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS5 ? "CS5 " :
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS6 ? "CS6 " :
        (tos & IPTOS_CLASS_MASK) == IPTOS_CLASS_CS7 ? "CS7 " : "");
    return str;
}

const char *protocol_str(uint8_t protocol) {
    switch (protocol) {
        case IPPROTO_ICMP:
            return "ICMP";
        case IPPROTO_TCP:
            return "TCP";
        case IPPROTO_UDP:
            return "UDP";
        default:
            return "Unknown Protocol";
    }
}

void print_ip_header(struct iphdr *ip) {
    printf("IP Header:\n");
    printf("  Version: %d\n", ip->version);
    printf("  Header Length: %d bytes\n", ip->ihl * 4);
    printf("  Type of Service: %s\n", tos_str(ip->tos));
    printf("  Total Length: %d bytes\n", ntohs(ip->tot_len));
    printf("  Identification: 0x%04x\n", ntohs(ip->id));
    printf("  Flags:\n");
    printf("    Reserved: %d\t", (ntohs(ip->frag_off) & 0x8000) >> 15);
    printf("    Don't Fragment: %d\t", (ntohs(ip->frag_off) & 0x4000) >> 14);
    printf("    More Fragments: %d\t", (ntohs(ip->frag_off) & 0x2000) >> 13);
    printf("  Fragment Offset: %d\n", ntohs(ip->frag_off) & 0x1FFF);
    printf("  Time to Live: %d\n", ip->ttl);
    printf("  Protocol: %s\n", protocol_str(ip->protocol));
    printf("  Header Checksum: 0x%04x\n", ntohs(ip->check));
    printf("  Source Address: %s\n", inet_ntoa(*(struct in_addr *)&ip->saddr));
    printf("  Destination Address: %s\n", inet_ntoa(*(struct in_addr *)&ip->daddr));
    printf("\n");
}

void print_icmp_header(struct icmphdr *icmp) {
    printf("ICMP Header:\n");
    printf("  Type: "); print_ICMP_type(icmp->type); printf("\n");
    printf("  Code: %d\n", icmp->code);
    printf("  Checksum: %d\n", ntohs(icmp->checksum));
    printf("  Identifier: %d\n", ntohs(icmp->un.echo.id));
    printf("  Sequence Number: %d\n", ntohs(icmp->un.echo.sequence));
    printf("\n\n");
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




void setIP(struct iphdr *ip_hdr, struct sockaddr_in *dest_addr, int packet_size, int ttl) 
{
    ip_hdr->ihl = 5;
    ip_hdr->version = 4;
    ip_hdr->tos = IPTOS_LOWDELAY;
    ip_hdr->tot_len = htons(packet_size);
    ip_hdr->id = htons(0);
    ip_hdr->frag_off = htons(0);
    ip_hdr->ttl = ttl;
    ip_hdr->protocol = IPPROTO_ICMP;
    ip_hdr->check = 0;
    ip_hdr->saddr = INADDR_ANY;
    ip_hdr->daddr = dest_addr->sin_addr.s_addr;
    ip_hdr->check = in_cksum((unsigned short *)ip_hdr, 4*ip_hdr->ihl);
}

int send_packet(int sockfd, char *data, struct sockaddr_in dest_addr, int ttl) 
{
    int packet_size;
    if(data != NULL) packet_size = 28 + strlen(data) + 1;
    else packet_size = 28;

    char packet[packet_size];
    struct iphdr *ip_hdr = (struct iphdr *)packet;
    setIP(ip_hdr, &dest_addr, packet_size, ttl);

    

    setICMP(packet+4*ip_hdr->ihl, data, packet_size-4*ip_hdr->ihl);

    printf("\n\n******* Sent Packet Headers ********\n");
    print_ip_header(ip_hdr);
    print_icmp_header((struct icmphdr *)packet+4*ip_hdr->ihl);

    //start_time = clock();
    clock_gettime(CLOCK_MONOTONIC, &start);
    // gettimeofday(&start_time, NULL);
    if (sendto(sockfd, packet, packet_size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto"); 
        return -1;
    }
    return 0;
}

long receive_packet(int sockfd, char *add, int *icmp_reply) 
{
    char buf[MAX_PACKET_SIZE];
    struct sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);

    // poll for 5 seconds
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    int ret = poll(fds, 1, 5000);
    if (ret == 0) {
        // printf("Timeout\n");
        return -1;
    }

    int bytes = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&src_addr, &src_addr_len);
    // gettimeofday(&end_time, NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (bytes <= 0) 
    {
        perror("recvfrom error");
        return -1;
    }
    
    long time_taken = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec)/1000;
    
    struct iphdr *ip_hdr = (struct iphdr *)buf;

    printf("\n\n******* Received Packet Headers ********\n");
    print_ip_header(ip_hdr);

    if(ip_hdr->protocol == IPPROTO_ICMP) {
        struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + (ip_hdr->ihl * 4));
        print_icmp_header(icmp_hdr);
        *icmp_reply = icmp_hdr->type;
    }


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
        sum += *(unsigned char *)ptr; //oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum = (sum >> 16) + (sum & 0xFFFF);
    answer = (unsigned short)~sum;

    return answer;
}
