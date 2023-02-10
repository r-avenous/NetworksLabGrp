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

void get(char *url);                    // Implements GET {url}
void put(char *url, char *filename);    // Implements PUT {url} <filename>
void get_to_request(const char *url, char *request); // converts get command to HTTP request
void set_content_type(const char *accept, char *content_type); // sets conent type according to accept header

int main()
{
    // assumed url max size
    char cmd[4], url[100], filename[100];
    while(1){
        printf("MyOwnBrowser> ");

        scanf("%s", cmd); 
        if(strcmp(cmd, "QUIT")==0){
            printf("Bye....\n");
            break;
        }

        scanf("%s", url);
        if(strcmp(cmd, "GET")==0){
            get(url);
        }
        else if(strcmp(cmd, "PUT")==0){
            scanf("%s", filename);
            put(url, filename);
        }
        else{
            printf("Invalid command\n");
        }
    }
    return 0;
}

// Implements GET {url}
void get(char *url){
    char request[1000];
    get_to_request(url, request);
    printf("%s\n", request);
}  

// Implements PUT {url} <filename>
void put(char *url, char *filename){
    return;
}



void get_to_request(const char *url, char *request) {
  char *file_extension;
  char host[100];
  time_t now;
  struct tm *tm;
  char date[100];
  char accept[100];
  char accept_language[100];
  char if_modified_since[100];
  char content_language[100];
  char content_length[100];
  char content_type[100];
  
  // Get the host from the URL
  sscanf(url, "http://%s", host);
  
  // Get the file extension from the URL
  file_extension = strrchr(url, '.');
  
  // Get the current date and time
  time(&now);
  tm = gmtime(&now);
  strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", tm);
  
  // Set the Accept header based on the file extension
  if (strcmp(file_extension, ".html") == 0) {
    strcpy(accept, "Accept: text/html\r\n");
  } else if (strcmp(file_extension, ".pdf") == 0) {
    strcpy(accept, "Accept: application/pdf\r\n");
  } else if (strcmp(file_extension, ".jpg") == 0) {
    strcpy(accept, "Accept: image/jpeg\r\n");
  } else {
    strcpy(accept, "Accept: text/*\r\n");
  }
  
  // Set the Accept-Language header
  strcpy(accept_language, "Accept-Language: en-us,en;q=0.9\r\n");
  
  // Set the If-Modified-Since header
  tm->tm_mday -= 2;
  strftime(if_modified_since, sizeof(if_modified_since), "If-Modified-Since: %a, %d %b %Y %H:%M:%S %Z\r\n", tm);
  
  // Set the Content-Language header
  strcpy(content_language, "Content-Language: en-us\r\n");
  
  // Set the Content-Length header
  strcpy(content_length, "Content-Length: 0\r\n");

  // Set the Content-Type header
  set_content_type(accept, content_type);
  
  // Construct the request message
  sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nDate: %s\r\n%s%s%s%s%s%s\r\n",
          url, host, date, accept, accept_language, if_modified_since, content_language, content_length, content_type);
}


void set_content_type(const char *accept, char *content_type) {
  if (strstr(accept, "text/html")) {
    strcpy(content_type, "Content-Type: text/html; charset=ISO-8859-4\r\n");
  } else if (strstr(accept, "application/pdf")) {
    strcpy(content_type, "Content-Type: application/pdf\r\n");
  } else if (strstr(accept, "image/jpeg")) {
    strcpy(content_type, "Content-Type: image/jpeg\r\n");
  } else if (strstr(accept, "text/*")) {
    strcpy(content_type, "Content-Type: text/plain; charset=ISO-8859-4\r\n");
  } else {
    strcpy(content_type, "Content-Type: \r\n");
  }
}