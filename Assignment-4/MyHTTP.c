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

void receive_file_content(FILE *fp, int content_length, int newsockfd)
{
	// if there is any extra data in the buffer received along with the headers
	fwrite(extra_data, 1, extra_data_size, fp);
	content_length -= extra_data_size;
	extra_data_size = 0;

	char buf[BUF_SIZE];
	int n;
	printf("conntent_length: %d\n", content_length); fflush(stdout);
	while(content_length>0){
		n = recv(newsockfd, buf, MAXLINE, 0);
		printf("\n***Received: %s\n", buf); fflush(stdout);
		content_length -= n;
		printf("conntent_length: %d\n", content_length); fflush(stdout);
		fwrite(buf, 1, n, fp);
	}

}
void implement_PUT(char *path, char **values, int newsockfd)
{
	// putting a '.' before the path
	char *modified_path = (char *)malloc(sizeof(char)*(sizeof(path)+1));
	strcpy(modified_path, "."); strcat(modified_path, path);

	printf("\n\nReceiving : %s\n\n", modified_path); fflush(stdout);

	FILE *fp = fopen(modified_path, "w");
	if (fp == NULL) {
		//File could not be opened(Probably not found)
		perror("Could not open file\n");
		// send_general_response(404, newsockfd);
		// fp = fopen("404.html", "r");
		// send_file(fp, "404.html", newsockfd);
		return;
	}

	// receive the file content
	int content_length = atoi(values[3]);	// "content-length:"  is at 3rd index
	receive_file_content(fp, content_length, newsockfd);
	fclose(fp);



	// File Received
	send_general_response(200, newsockfd);
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
		else if(strcmp(method, "PUT")==0){
			implement_PUT(path, values, newsockfd);
		}



		// send(newsockfd, "REQUEST RECEIVED", 17, 0);	// send the current time
		close(newsockfd);
	}
	return 0;
}
