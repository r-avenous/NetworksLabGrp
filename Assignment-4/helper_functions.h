int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}

int startServer(int port_no){
	int sockfd, newsockfd; // socket descriptors
	struct sockaddr_in serv_addr;



	char buf[BUF_SIZE]; // used to communicate with client

	// Opens a socket (returns -1 incase an error occurs )
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family = AF_INET;			// the internet family
	serv_addr.sin_addr.s_addr = INADDR_ANY; // set to INADDR_ANY for machines having a single IP address
	serv_addr.sin_port = htons(port_no);	// specifies the port number of the server

	// bind() function to provide local address to the socket
	int status = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (status < 0){
		perror("Unable to bind local address\n");
		exit(0);
	}

	// Specifies that upto {MAXCONNECTIONS} active clients can wait for a connection
	listen(sockfd, MAXCONNECTIONS);
	printf("Server is listening on port %d....\n", port_no);
	fflush(stdout);

	return sockfd;
}





void receive_headers(int sockfd, char *buf, int size)
{
	int bytes_received = 0;
	buf[0] = '\0';
	int line_break = 0, end_of_req = 0;
	while (bytes_received < size)
	{
		int bytes = recv(sockfd, buf + bytes_received, min(size - bytes_received, MAXLINE), 0);
		if (bytes == -1)
		{
			perror("Error in receiving data");
			exit(0);
		}
		if (bytes == 0)
		{
			break;
		}

		for (int i = 0; i < bytes; ++i)
		{
			char ch = buf[bytes_received + i];
			if (ch == ' ' || ch == '\t' || ch == '\r')
				continue;

			if (ch == '\n')
			{
				line_break++;
				if (line_break == 2)
				{
					strcpy(extra_data, buf + bytes_received + i + 1);
					extra_data_size = bytes - i - 1;
					end_of_req = 1;
					break;
				}
			}
			else
			{
				line_break = 0;
			}
		}

		bytes_received += bytes;
		if (end_of_req)
		{
			buf[bytes_received] = '\0';
			break;
		}

		if (buf[bytes_received - 1] == '\0')
		{
			break;
		}
	}
}


void parse_headers(char *request)
{
	char *version; int header_count; char **headers;
	method = strsep(&request, " ");
	path = strsep(&request, " ");
	version = strsep(&request, "\r\n ");

	printf("Method: %s\n", method);
	printf("Path: %s\n", path);
	printf("Version: %s\n", version);

	if(strcmp(method, "GET")==0){	///////////
		header_count = header_count_get;
		headers = headers_get;
	}
	else if(strcmp(method, "PUT")==0){	///////////
		header_count = header_count_put;
		headers = headers_put;
	}


	for(int i=0; i<header_count; ++i)
		values[i] = NULL;

	while(1){
		++request; // to remove the \n	
		char *line = strsep(&request, "\r");

		if(strlen(line) == 0) 		// if the line is empty
			break;

		char *header = strsep(&line, " ");	char *value = line;

		for(int i=0; i<header_count; ++i){
			if(strcasecmp(header, headers[i])==0){
				values[i] = value;
				break;
			}
		}	
	}


	for(int i=0; i<header_count; ++i){
		printf("%s %s\n", headers[i], values[i]);
	}

	return;
}

char *get_content_type(char *path){
	char *ext = strrchr(path, '.');
	if (ext == NULL) return "Content-Type: text/*\r\n";
	if (strcmp(ext, ".html")==0) return "Content-Type: text/html\r\n";
	if(strcmp(ext, ".pdf")==0) return "Content-Type: application/pdf\r\n";
	if(strcmp(ext, ".jpg")==0) return "Content-Type: image/jpeg\r\n";

	return "Content-Type: text/*\r\n";
}

void send_file(FILE *fp, char *filename, int newsockfd){

	// Send the content type
	char *content_type = get_content_type(filename);
	send(newsockfd, content_type, strlen(content_type), 0);	// send the content type


	// Send the content length
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);


	char content_len[30]; 
	sprintf(content_len, "Content-Length: %d", size);
	strcat(content_len, "\r\n\r\n");
	send(newsockfd, content_len, strlen(content_len), 0);	// send the content length


	char content[101];
	int sent_size=0, read_size, total_read_size=0;
	while((read_size = fread(content,1,100,fp))!= 0){
		total_read_size += read_size;
		int count = send(newsockfd, content, read_size, 0);
		sent_size += count;
	}

	printf("\n\nFile sent\n");
	printf("Sent size: %d\n", sent_size); 
	printf("Total read size: %d\n\n", total_read_size);
	fflush(stdout);
}

void send_general_response(int status_code, int newsockfd){
	char date[100];
	time_t now;
	struct tm *tm;
	char *first_line;

	if(status_code==200){
		first_line = "HTTP/1.1 200 OK\r\n";  
	}
	else if(status_code==404){
		first_line = "HTTP/1.1 404 Not Found\r\n";
	}

	// send the first line
	send(newsockfd, first_line, strlen(first_line), 0);	


	// Send the current date and time
    time(&now);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", tm); 
	send(newsockfd, date, strlen(date), 0);	// send the date


	// Send the server name
	char *server_name = "Server: MyBrowser/100.29.12\r\n";
	send(newsockfd, server_name, strlen(server_name), 0);	// send the server name
}

void implement_error_404(int newsockfd){
	send_general_response(404, newsockfd);
	FILE *fp = fopen("404.html", "rb");
	send_file(fp, "404.html", newsockfd);
}


void implement_GET(char *path, char **values, int newsockfd){
	// putting a '.' before the path
	char *modified_path = (char *)malloc(sizeof(char)*(sizeof(path)+1));
	strcpy(modified_path, "."); strcat(modified_path, path);

	printf("\n\nSending: %s\n\n", modified_path); fflush(stdout);

	FILE *fp = fopen(modified_path, "rb");

	// File Not Found
	if (fp == NULL) {
		perror("Could not open file\n");
		implement_error_404(newsockfd);
		return;
	}

	// File Found
	send_general_response(200, newsockfd);

	// Send the file type, length and content
	send_file(fp, path, newsockfd);


	// Close the file
	fclose(fp);

}

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

	FILE *fp = fopen(modified_path, "wb");
	if (fp == NULL) {
		//File could not be opened(Probably not found)
		implement_error_404(newsockfd);
		perror("Could not open file\n");
		return;
	}

	// receive the file content
	int content_length = atoi(values[3]);	// "content-length:"  is at 3rd index
	receive_file_content(fp, content_length, newsockfd);
	fclose(fp);



	// File Received
	send_general_response(200, newsockfd);
}
