void send_file(FILE *fp, char *filename, int newsockfd);

int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}

char *deep_copy(char *str)
{
	char *copy = (char *)malloc(strlen(str) + 1);
	strcpy(copy, str);
	return copy;
}

void parse_first_line(char *request)
{
	method = deep_copy(strsep(&request, " "));
	path = deep_copy(strsep(&request, " "));
	version = deep_copy(strsep(&request, "\r"));

}

void parse_headers(char *line)
{
	int header_count; char **headers;
	if(strcmp(method, "GET")==0){	
		header_count = header_count_get;
		headers = headers_get;
	}
	else if(strcmp(method, "PUT")==0){	
		header_count = header_count_put;
		headers = headers_put;
	}
	else{
		header_count = header_count_badreq;
		headers = headers_badreq;
	}

	char *header = strsep(&line, " ");	
	char *value = strsep(&line, "\r");

	for(int i=0; i<header_count; ++i){
		if(strcasecmp(header, headers[i])==0){
			values[i] = deep_copy(value);
			break;
		}
	}	


}

void receive_headers(int sockfd, char *buf, int size)
{
	int bytes_received = 0;
	buf[0] = '\0';
	int line_break = 0, end_of_req = 0;
	int first_line = 1;
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
				buf[bytes_received + i] = '\0';
				printf("## Received: %s\n", buf); fflush(stdout);

				if(first_line){
					parse_first_line(buf);
					first_line = 0;

				}
				else{
					parse_headers(buf);
				}

				strcpy(buf, buf + bytes_received+i+1);
				line_break++;
				bytes_received = -i-1;

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





// Sends Cache-Control headers
void send_cache_ctrl(int newsockfd){
	char *cache_ctrl = "Cache-Control: no-store\r\n";
	send(newsockfd, cache_ctrl, strlen(cache_ctrl), 0);
}

char *get_content_type(char *path){
	char *ext = strrchr(path, '.');
	if (ext == NULL) return "Content-Type: text/*\r\n";
	if (strcmp(ext, ".html")==0) return "Content-Type: text/html\r\n";
	if(strcmp(ext, ".pdf")==0) return "Content-Type: application/pdf\r\n";
	if(strcmp(ext, ".jpg")==0) return "Content-Type: image/jpeg\r\n";

	return "Content-Type: text/*\r\n";
}

void send_expiry(int newsockfd){
	// set to current time + 3 days
	char expiry_date[100];
	time_t now;
	struct tm *tm;

	// Send the current date and time
	time(&now);
	tm = gmtime(&now);
	// add 3 days
	tm->tm_mday += 3;
	strftime(expiry_date, sizeof(expiry_date), "Expires: %a, %d %b %Y %H:%M:%S %Z\r\n", tm); 
	send(newsockfd, expiry_date, strlen(expiry_date), 0);	// send the expiry date
}

void send_file(FILE *fp, char *filename, int newsockfd){

	// Send the content type
	char *content_lang = "Content-Language: en-us\r\n";
	send(newsockfd, content_lang, strlen(content_lang), 0);	// send the content language
	char *content_type = get_content_type(filename);
	send(newsockfd, content_type, strlen(content_type), 0);	// send the content type


	// Send the content length
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	printf("\n\nContent-Length: %d\n\n", size);
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


void send_date_server(int newsockfd){
	char date[100];
	time_t now;
	struct tm *tm;

	// Send the current date and time
	time(&now);
	tm = gmtime(&now);
	strftime(date, sizeof(date), "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", tm); 
	send(newsockfd, date, strlen(date), 0);	// send the date
}

void implement_error(int error_code, int newsockfd){
	char *first_line, *file_name;
	if(error_code==BADREQUEST)
	{
		first_line = "HTTP/1.1 400 Bad Request\r\n";
		file_name = "400.html";
	}
	else if(error_code==FORBIDDEN)
	{
		first_line = "HTTP/1.1 403 Forbidden\r\n";
		file_name = "403.html";	
	}
	else if(error_code==NOTFOUND)
	{
		first_line = "HTTP/1.1 404 Not Found\r\n";
		file_name = "404.html";
	}

	// send the first line
	send(newsockfd, first_line, strlen(first_line), 0);	

	// Send the current date and time and server name
	send_date_server(newsockfd);

	// Send the file
	FILE *fp = fopen(file_name, "rb");
	send_file(fp, file_name, newsockfd); 
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
		implement_error(FORBIDDEN ,newsockfd);
		perror("Could not open file\n");
		return;
	}


	// receive the file content
	int content_length = atoi(values[CONTENT_LENGTH_INDEX]);	// "content-length:"  is at 3rd index
	receive_file_content(fp, content_length, newsockfd);
	fclose(fp);

	// File Received
	char *first_line = "HTTP/1.1 200 OK\r\n";
	send(newsockfd, first_line, strlen(first_line), 0);	
	send_date_server(newsockfd);
	send_cache_ctrl(newsockfd);
}

void send_last_modified(char *filename, int newsockfd){
    struct stat file_stat;

	stat(filename, &file_stat);

    struct tm *timeinfo;
    timeinfo = gmtime(&file_stat.st_mtime);
    char buffer[80];
    strftime(buffer, 80, "%a, %d %b %Y %T GMT\n", timeinfo);
    
	send(newsockfd, "Last-Modified: ", 15, 0);
	send(newsockfd, buffer, strlen(buffer), 0);
	return;
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
		implement_error(NOTFOUND, newsockfd);
		return;
	}

	// File Found
	char *first_line = "HTTP/1.1 200 OK\r\n";
	send(newsockfd, first_line, strlen(first_line), 0);
	send_date_server(newsockfd);
	send_expiry(newsockfd);	// send the expiry date
	send_cache_ctrl(newsockfd);
	send_last_modified(modified_path, newsockfd);

	// Send the file type, length and content
	send_file(fp, path, newsockfd);


	// Close the file
	fclose(fp);

}