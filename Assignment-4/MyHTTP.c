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


void free_all()
{
	if(method) free(method);
	if(path) free(path);
	if(version) free(version);
	method=NULL; path=NULL; version=NULL;
	for(int i=0; i<MAX_USEFUL_HEADERS; ++i){
		if(values[i])
			free(values[i]);
		values[i]=NULL;
	}
}



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

		printf("Method: %s\n", method);
		printf("Path: %s\n", path);
		

		if(strcmp(method, "GET")==0){
			for(int i=0; i<header_count_get; ++i){
				printf("%s: %s\n", headers_get[i], values[i]);
			}
			printf("\n\n");
			implement_GET(path, values, newsockfd);
			FILE* fp = fopen(LOGGILE, "a");
			// <Date(ddmmyy)>:<Time(hhmmss)>:<Client IP>:<Client Port>:<GET/PUT>:<URL>
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			fprintf(fp, "%02d-%02d-%02d:%02d-%02d-%02d:%s:%d:%s:%s\n", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), method, path);
			fclose(fp);
			free_all();
		}
		else if(strcmp(method, "PUT")==0){
			implement_PUT(path, values, newsockfd);
			FILE* fp = fopen(LOGGILE, "a");
			// <Date(ddmmyy)>:<Time(hhmmss)>:<Client IP>:<Client Port>:<GET/PUT>:<URL>
			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			fprintf(fp, "%02d-%02d-%02d:%02d-%02d-%02d:%s:%d:%s:%s\n", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), method, path);
			fclose(fp);
			free_all();
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
