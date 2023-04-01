#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../mysocket.h"

#define PORT 8085

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
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[9000] = {0};
    char *hello = "Hello from server";

    // Create socket file descriptor
    if ((server_fd = my_socket(AF_INET, SOCK_MyTCP, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (my_bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (my_listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = my_accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // // Communication loop
    // for (int i = 0; i < 9; i++) {
    //     // Receive message from client
    //     valread = my_recv(new_socket, buffer, 200, 0);
    //     printf("%d:: %s\n", i+1, buffer);

    //     // Send message to client
    //     strcpy(buffer, generate_random_string(50+i));
    //     my_send(new_socket, buffer, strlen(buffer)+1, 0);
    //     printf("%d:: %s\n",i+1, buffer);

    //     memset(buffer, 0, sizeof(buffer));
    // }

    // strcpy(buffer, generate_random_string(6000));
    // my_send(new_socket, buffer, strlen(buffer)+1, 0);

    // printf("%d:: %s\n",40, buffer);

    // strcpy(buffer, "hello there");
    // printf("extra %ld\n", send(new_socket, buffer, strlen(buffer)+1, 0));


    strcpy(buffer, "Gaurav");
    my_send(new_socket, buffer, strlen(buffer)+1, 0);


    strcpy(buffer, "Atulya");
    my_send(new_socket, buffer, strlen(buffer)+1, 0);
    my_close(new_socket);
    my_close(server_fd);

    return 0;
}
