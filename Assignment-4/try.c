#include <stdio.h> 
#include <string.h> 
    
int main() { 
    char filename[100] = "./Hello.txt"; 
    FILE *fp; 
    char c; 
    
    // open the file 
    fp = fopen(filename, "r"); 
    if (fp == NULL) {
        printf("Error opening file\n"); 
        return 0;
    } 
    
    // read character by character and print them 
    while ((c = fgetc(fp)) != EOF) {
        printf("%c", c); 
    }
    
    fclose(fp); 
    
    return 0; 
} 

void receive_in_packets(int sockfd, char *buf, int size){
    const int PACKET_SIZE = 4;
    int bytes_received = 0;
    buf[0] = '\0';
    while(bytes_received < size){
        int bytes = recv(sockfd, buf + bytes_received, min(size - bytes_received, PACKET_SIZE), 0);
        if(bytes == -1){
            perror("Error in receiving data");
            exit(0);
        }
        if(bytes == 0){
            break;
        }
        bytes_received += bytes;
        if(buf[bytes_received-1] == '\0'){
            break;
        }
    }
}