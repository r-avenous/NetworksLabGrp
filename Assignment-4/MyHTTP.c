#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>

#define NOTFOUND 404
#define OK 200
#define BADREQUEST 400
#define FORBIDDEN 403

#define LOCALHOST "127.0.0.1"
#define MAXLINE 10
#define MAXCONNECTIONS 5
#define DURATION 5000
#define LOGGILE "AccessLog.txt"
#define BUF_SIZE 10000

int min( int a, int b){
    if(a<b) return a;
    return b;
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

        // for(int i=0; i<bytes; ++i) printf("%d ", *(buf+bytes_received+i));
        // printf("\n"); fflush(stdout);
        printf("%s", buf+bytes_received);

        

        bytes_received += bytes;
        if(buf[bytes_received-1] == '\0'){
            break;
        }


        
    }
}


int main(){
	int	sockfd, newsockfd;		// socket descriptors 
	int	clilen;
	struct sockaddr_in	cli_addr, serv_addr;
	

	int i, status;				// status variable for bind() function
	char buf[BUF_SIZE];		// used to communicate with client 		

	// Opens a socket (returns -1 incase an error occurs )
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;			// the internet family
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// set to INADDR_ANY for machines having a single IP address
	serv_addr.sin_port = htons(8050);		// specifies the port number of the server


	// bind() function to provide local address to the socket
	status = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (status< 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	// Specifies that upto 5 active clients can wait for a connection
	listen(sockfd, 5); 
    printf("Server is listening on port 8080....\n");

	while(1) {
		// accept() function waits until a client connects to the server and then sending/receiving pf data occurs
		// newsockfd is used for communicating with client
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); 

		// if erroe in accept() then -1 is returned
		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

        // receive the request from the client
        receive_in_packets(newsockfd, buf, BUF_SIZE);

        printf("Request received:\n %s\n", buf);
        fflush(stdout);
		
		
		send(newsockfd, "REQUEST RECEIVED", 17, 0);	// send the current time
		close(newsockfd);

	}
	return 0;
}



