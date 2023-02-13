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
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>

#include "global_variables.h"
#include "helper_functions.h"



int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	int sockfd, newsockfd; // socket descriptors
	struct sockaddr_in cli_addr; int clilen = sizeof(cli_addr); // client address, size

	int port_no = 8000; // default
	if (argc > 1) port_no = atoi(argv[1]);

	char buf[BUF_SIZE]; // used to communicate with client

	sockfd = startServer(port_no); // start the server


	while (1)
	{	
		printf("\n\n<#> Waiting for a new client...\n\n"); fflush(stdout);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0){
			perror("Accept error\n");
			exit(0);
		}
		// receive the request from the client
		receive_headers(newsockfd, buf, BUF_SIZE);
		printf("\n\nRequest received:\n%s\n", buf); fflush(stdout);
		parse_headers(buf, newsockfd);

		if(strcmp(method, "GET")==0){
			implement_GET(path, values, newsockfd);
		}
		else if(strcmp(method, "PUT")==0){
			implement_PUT(path, values, newsockfd);
		}
		else{
			if(values[CONTENT_LENGTH_INDEX] != NULL){
				FILE* fp = fopen("temp.txt", "w");
				receive_file_content(fp, atoi(values[0]), newsockfd);
				fclose(fp);
			}
			implement_error(BADREQUEST, newsockfd);
			remove("temp.txt");
		}
		close(newsockfd);
	}
	return 0;
}
