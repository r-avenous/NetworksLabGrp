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
#define MAXLINE 1000
#define MAXCONNECTIONS 5
#define DURATION 5000

void get(char *url);                                           // Implements GET {url}
void put(char *url, char *filename);                           // Implements PUT {url} <filename>
void get_to_request(char *url, char *request);                 // converts get command to HTTP request
void put_to_request(char *url, char *filename, char *request); // converts put command to HTTP request
void set_content_type(const char *accept, char *content_type); // sets conent type according to accept header
void download_file(char* filename, int sockfd, int size, char* contentStart, int contstartlen);   // writes content to file
void upload_file(char* filename, int sockfd);                  // uploads file to server
enum fileType {HTML, PDF, JPG, OTHER};   // file type enum
int curfileType;                        // current file type
char* curfilename;                      // current filename

int main()
{

    // assumed url max size
    char cmd[4], url[200], filename[100];
    while (1)
    {
        printf("MyOwnBrowser> ");

        scanf("%s", cmd);
        if (strcmp(cmd, "QUIT") == 0)
        {
            printf("Bye....\n");
            break;
        }

        scanf("%s", url);
        if (strcmp(cmd, "GET") == 0)
        {
            get(url);
        }
        else if (strcmp(cmd, "PUT") == 0)
        {
            scanf("%s", filename);
            put(url, filename);
        }
        else
        {
            printf("Invalid command\n");
        }
    }
    return 0;
}

// Implements GET {url}
void get(char *url)
{
    char http_request[10000], server_ip[16];
    get_to_request(url, http_request);

    // strcpy(http_request, "GET / HTTP/1.1\r\n\
    //                       Host: localhost:8080\r\n\
    //                       Date: Sat, 11 Feb 2023 16:51:02 GMT\r\n\
    //                       Accept: */*\r\n\
    //                       If-Modified-Since: Sat, 09 Feb 2023 16:47:56 GMT\r\n\r\n");

    printf("\n\nRequest Sent=\n%s\n\n", http_request);

    strcpy(server_ip, url + 7); // GET http://127.0.0.1:8000/

    for (int i = 0; i < 16; i++)
    {
        if (server_ip[i] != '.' && (server_ip[i] < '0' || server_ip[i] > '9'))
        {
            server_ip[i] = '\0';
            break;
        }
    }
    printf("SERVER IP = %s\n\n\n", server_ip);

    int sockfd;
    struct sockaddr_in serv_addr;

    // Opening a new socket is exactly similar to the server process
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton(server_ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(8000);

    int status = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    send(sockfd, http_request, strlen(http_request) + 1, 0);

    // download function complete
    char response[MAXLINE];
    int r = recv(sockfd, response, MAXLINE, MSG_WAITALL);
    printf("%s\n", response);
    char* pt = strstr(response, "Content-Length: ");
    int size = atoi(pt + 16);
    pt = strstr(response, "\r\n\r\n");
    pt += 4;
    size -= r - (pt - response);
    download_file("untitled", sockfd, size, pt, r - (pt - response));
    // -------
    close(sockfd);

    return;
}

// Implements PUT {url} <filename>
void put(char *url, char *filename)
{
    char http_request[10000], server_ip[16];
    put_to_request(url, filename, http_request);
    
    printf("\n\nRequest Sent=\n%s\n\n", http_request);

    strcpy(server_ip, url + 7);
    for (int i = 0; i < 16; i++)
    {
        if (server_ip[i] != '.' && (server_ip[i] < '0' || server_ip[i] > '9'))
        {
            server_ip[i] = '\0';
            break;
        }
    }
    printf("SERVER IP = %s\n\n\n", server_ip);

    int sockfd;
    struct sockaddr_in serv_addr;

    // Opening a new socket is exactly similar to the server process
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Unable to create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton(server_ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(8000);

    int status = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    send(sockfd, http_request, strlen(http_request) + 1, 0);
    // download function complete
    upload_file(filename, sockfd);
    // -------
    close(sockfd);
}

void get_to_request(char *url, char *request)
{

    char *host, *path, *file_extension;
    time_t now;
    struct tm *tm;
    char date[100];
    char accept[100];
    char accept_language[100];
    char if_modified_since[100];

    // get components of url
    strsep(&url, "/");
    strsep(&url, "/");
    host = strsep(&url, "/"); // get host
    if (url)
        path = url; // get file path
    else
        path = "/";

    // get file extension from path
    file_extension = path;
    while (*file_extension != '.' && *file_extension != '\0')
    {
        file_extension++;
    }

    // Get the current date and time
    time(&now);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", tm);

    // Set the Accept header based on the file extension
    if (strcmp(file_extension, ".html") == 0)
    {
        strcpy(accept, "Accept: text/html\r\n");
        curfileType = HTML;
    }
    else if (strcmp(file_extension, ".pdf") == 0)
    {
        strcpy(accept, "Accept: application/pdf\r\n");
        curfileType = PDF;
    }
    else if (strcmp(file_extension, ".jpg") == 0)
    {
        strcpy(accept, "Accept: image/jpeg\r\n");
        curfileType = JPG;
    }
    else
    {
        strcpy(accept, "Accept: */*\r\n");
        curfileType = OTHER;
    }

    // Set the Accept-Language header
    strcpy(accept_language, "Accept-Language: en-us\r\n");

    // Set the If-Modified-Since header
    tm->tm_mday -= 2;
    strftime(if_modified_since, sizeof(if_modified_since), "If-Modified-Since: %a, %d %b %Y %H:%M:%S %Z\r\n", tm);

    // Construct the request message
    sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s\r\nDate: %s\r\n%s%s%sConnection: close\r\n\r\n",
            path, host, date, accept, accept_language, if_modified_since);
}

void put_to_request(char *url, char *filename, char *request)
{
    char *host, *path, *file_extension;
    time_t now;
    struct tm *tm;
    char date[100];
    char accept[100];
    char accept_language[100];
    char last_modified[100];
    char content_length[100];

    // get components of url
    strsep(&url, "/");
    strsep(&url, "/");
    host = strsep(&url, "/"); // get host
    if (url)
        path = url; // get file path
    else
        path = "/";

    // get file extension from path
    file_extension = path;
    while (*file_extension != '.' && *file_extension != '\0')
    {
        file_extension++;
    }

    // Get the current date and time
    time(&now);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", tm);

    // Set the Accept header based on the file extension
    if (strcmp(file_extension, ".html") == 0)
    {
        strcpy(accept, "Content-type: text/html\r\n");
        curfileType = HTML;
    }
    else if (strcmp(file_extension, ".pdf") == 0)
    {
        strcpy(accept, "Content-type: application/pdf\r\n");
        curfileType = PDF;
    }
    else if (strcmp(file_extension, ".jpg") == 0)
    {
        strcpy(accept, "Content-type: image/jpeg\r\n");
        curfileType = JPG;
    }
    else
    {
        strcpy(accept, "Content-type: */*\r\n");
        curfileType = OTHER;
    }

    // Set the Accept-Language header
    strcpy(accept_language, "Content-language: en-us\r\n");

    // Set the Last-Modified header
    strftime(last_modified, sizeof(last_modified), "Last-Modified: %a, %d %b %Y %H:%M:%S %Z\r\n", tm);

    // Set the Content-Length header
    FILE *fp;
    fp = fopen(filename, "rb");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    sprintf(content_length, "Content-Length: %d\r\n", size);

    // Construct the request message
    sprintf(request, "PUT /%s HTTP/1.1\r\nHost: %s\r\nDate: %s\r\n%s%s%s%s\r\n",
            path, host, date, accept, accept_language, last_modified, content_length);
}

/*
GET http://127.0.0.1:8000/Hello.txt
GET http://127.0.0.1:8000/queries.sql
GET http://127.0.0.1:8000/Assgn-3.pdf
*/

void download_file(char* filename, int sockfd, int size, char* content, int contstartlen)
{
    FILE *fp;
    fp = fopen(filename, "wb");
    fwrite(content, contstartlen, 1, fp);
    int r = 0;
    while (r < size)
    {
        char buffer[MAXLINE+1];
        int n = recv(sockfd, buffer, MAXLINE, MSG_WAITALL);
        buffer[n] = '\0';
        fwrite(buffer, n, 1, fp);
        r += n;
    }
    fclose(fp);

    if(!fork())
    {
        if(curfileType == OTHER)
        {
            execvp("gedit", (char*[]){"gedit", filename, NULL});
        }
        else if(curfileType == HTML)
        {
            execvp("firefox", (char*[]){"firefox", filename, NULL});
        }
        else if(curfileType == PDF)
        {
            execvp("evince", (char*[]){"evince", filename, NULL});
        }
        else if(curfileType == JPG)
        {
            execvp("eog", (char*[]){"eog", filename, NULL});
        }
        exit(0);
    }
}

void upload_file(char *filename, int sockfd)
{
    FILE *fp;
    fp = fopen(filename, "rb");
    fseek(fp, 0, SEEK_END);
    char buffer[MAXLINE];
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    int r = 0;
    while (r < size)
    {
        int n = fread(buffer, 1, MAXLINE, fp);
        send(sockfd, buffer, n, 0);
        r += n;
    }
    fclose(fp);
}