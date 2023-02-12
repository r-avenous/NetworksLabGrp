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
	const int PACKET_SIZE = MAXLINE;
	int bytes_received = 0;
	buf[0] = '\0';
	int line_break = 0, end_of_req = 0;
	while (bytes_received < size)
	{
		int bytes = recv(sockfd, buf + bytes_received, min(size - bytes_received, PACKET_SIZE), 0);
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


void parse_headers(char *request, char *method, char *path, char *values[])
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
		values = values_get;
	}

	for(int i = 0; i < header_count; i++){
		values[i] = NULL;
		char *header = headers[i];

		// search for the header in the request
		char *header_ptr = strstr(request, header);
		if(header_ptr == NULL) continue;

		strsep(&header_ptr, " ");
		values[i] = strsep(&header_ptr, "\r\n");

	}


	for(int i=0; i<header_count; ++i){
		printf("%s %s\n", headers[i], values[i]);
	}

	return;
}