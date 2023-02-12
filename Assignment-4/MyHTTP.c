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


void implement_GET(char *path, char **values, int newsockfd){
	// putting a '.' before the path
	char *modified_path = (char *)malloc(sizeof(char)*(sizeof(path)+1));
	strcpy(modified_path, "."); strcat(modified_path, path);

	printf("\n\nOpening: %s\n\n", modified_path); fflush(stdout);
	FILE *fp = fopen(modified_path, "r");
	if (fp == NULL) {
		//File could not be opened
		perror("Could not open file\n");
		exit(EXIT_FAILURE);
	}



	// File Found
	char *first_line = "HTTP/1.1 200 OK\r\n"; 
	char date[100];
    // Get the current date and time
    time_t now;
    struct tm *tm;
    time(&now);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", tm); 








	char *response_headers = "HTTP/1.0 200 OK\r\n\
Server: SimpleHTTP/0.6 Python/3.10.6\r\n\
Date: Sun, 12 Feb 2023 14:33:36 GMT\r\n\
Content-type: text/plain\r\n\
Content-Length:";

	// Get size of file
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char *content_len = (char *)malloc(sizeof(char)*10);
	sprintf(content_len, "%d", size);

	char *content = (char *)malloc(sizeof(char)*size);
	fread(content, sizeof(char), size, fp);

	char *response = (char *)malloc(sizeof(char)*(strlen(response_headers)+strlen(content_len)+4+strlen(content)+1));
	strcpy(response, response_headers); strcat(response, content_len); strcat(response, "\r\n\r\n"); strcat(response, content);

	printf("\n\nResponse:\n%s\n", response); fflush(stdout);

	send(newsockfd, response, strlen(response), 0);

}

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
