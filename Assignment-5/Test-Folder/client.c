#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080

char* generate_random_string(int length) {
    char* str = malloc((length + 1) * sizeof(char));
    strcpy(str, "\n\nSTART:-\n\n");
    if (str == NULL) {
        printf("Error: unable to allocate memory\n");
        exit(1);
    }
    
    // Set random seed
    srand(time(NULL));
    
    for (int i = 11; i < length-12; i++) {
        // Generate a random ASCII character
        str[i] = (char) ((rand() % 127)+1);
    }
    
    // Add null terminator
    strcpy(str+length-12, "\n\nEND\n\n");
    str[length] = '\0';
    
    return str;
}

int main(int argc, char const *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[4096] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Communication loop
    int i;
    for (i = 0; i<30; ++i){
        // Send message to server
        strcpy(buffer, generate_random_string(100));
        printf("CLI SEND-%d:: %s\n",i+1, buffer);
        send(sock, buffer, strlen(buffer)+1, 0);
        
        recv(sock, buffer, 1024, 0);
        printf("CLI RECV-%d::%s\n",i+1, buffer);
    }
}
