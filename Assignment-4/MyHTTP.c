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

#include "global_variables.h"
#include "helper_functions.h"



int main(int argc, char **argv)
{
	int sockfd, newsockfd; // socket descriptors
	struct sockaddr_in cli_addr; int clilen = sizeof(cli_addr); // client address, size

	int port_no = 8000; // default
	if (argc > 1) port_no = atoi(argv[1]);

	char buf[BUF_SIZE]; // used to communicate with client

	sockfd = startServer(port_no); // start the server


	while (1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0){
			perror("Accept error\n");
			exit(0);
		}

		// receive the request from the client
		receive_headers(newsockfd, buf, BUF_SIZE);

		printf("\n\nRequest received:\n%s\n", buf); fflush(stdout);

		parse_headers(buf);

		if(strcmp(method, "GET")==0){
			implement_GET(path, values, newsockfd);
		}


		// send(newsockfd, "REQUEST RECEIVED", 17, 0);	// send the current time
		close(newsockfd);
	}
	return 0;
}
