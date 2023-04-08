
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

int send_packet(int sockfd, char *data, struct sockaddr_in dest_addr, int ttl) 
{
    int packet_size;
    if(data != NULL) packet_size = 28 + strlen(data) + 1;
    else packet_size = 28;

    char packet[packet_size];
    struct iphdr *ip_hdr = (struct iphdr *)packet;
    setIP(ip_hdr, &dest_addr, ttl);

    

    setICMP(packet+4*ip_hdr->ihl, data, packet_size-4*ip_hdr->ihl);

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
    // long time_taken = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);

    // printf("Time taken by the function: %ld microseconds\n", time_taken);
    
    struct iphdr *ip_hdr = (struct iphdr *)buf;
    struct icmphdr *icmp_hdr = (struct icmphdr *)(buf + (ip_hdr->ihl * 4));

    char *data = buf + (ip_hdr->ihl * 4) + sizeof(struct icmphdr);
    // printf("%d\n", ip_hdr->ihl * 4 + sizeof(struct icmphdr));
    // printf("Received Message: %s\n", data);
    // print sender 
    // printf("Sender IP: %s\n", inet_ntoa(src_addr.sin_addr));
    sprintf(add, "%s", inet_ntoa(src_addr.sin_addr));
    *icmp_reply = icmp_hdr->type;
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
