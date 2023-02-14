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
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>

#define NOTFOUND 404
#define OK 200
#define BADREQUEST 400
#define FORBIDDEN 403

#define LOCALHOST "127.0.0.1"
#define MAXLINE 1500
#define MAXCONNECTIONS 5
#define DURATION 3000

void get(char *url);                                                                            // Implements GET {url}
void put(char *url, char *filename);                                                            // Implements PUT {url} <filename>
void get_to_request(char *url, char *request);                                                  // converts get command to HTTP request
void put_to_request(char *url, char *filename, char *request);                                  // converts put command to HTTP request
void set_content_type(const char *accept, char *content_type);                                  // sets conent type according to accept header
void download_file(char *filename, int sockfd, int size, char *contentStart, int contstartlen); // writes content to file
void upload_file(char *filename, int sockfd);                                                   // uploads file to server
char *stristr(const char *str1, const char *str2);                                              // case insensitive strstr
enum fileType
{
    HTML,
    PDF,
    JPG,
    OTHER
};                 // file type enum
int curfileType;   // current file type
char *curfilename; // current filename

int port_no = 80; // default port number
int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        port_no = atoi(argv[1]);
    }

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

int min(int a, int b)
{
    return a < b ? a : b;
}
void receive_in_packets(int sockfd, char *buf, int size)
{
    int bytes_received = 0;
    buf[0] = '\0';
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
        bytes_received += bytes;
        if (buf[bytes_received - 1] == '\0')
        {
            break;
        }
    }
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
    // get port number
    char *port = strstr(url+7, ":");
    if (port != NULL)
    {
        port_no = atoi(port + 1);
    }

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
    serv_addr.sin_port = htons(port_no);

    int status = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    send(sockfd, http_request, strlen(http_request), 0);

    // download function complete
    char response[MAXLINE];
    // timeout of 3 seconds with poll
    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, DURATION);
    if (ret == 0)
    {
        printf("Timeout\n");
        return;
    }
    int r = recv(sockfd, response, MAXLINE, MSG_WAITALL);
    // get status code
    int status_code = atoi(response + 9);
    if (status_code == NOTFOUND)
    {
        printf("404 Not Found\n");
    }
    else if (status_code == BADREQUEST)
    {
        printf("400 Bad Request\n");
    }
    else if (status_code == FORBIDDEN)
    {
        printf("403 Forbidden\n");
    }
    else if (status_code != OK)
    {
        printf("%d Unknown Error\n", status_code);
    }
    char *pt2 = stristr(response, "Content-Type: ");
    if (pt2 == NULL)
    {
        return;
    }
    pt2 += 14;
    char *pt3 = stristr(pt2, "text/html");
    if (pt3 != NULL && pt3 < pt2 + 10)
    {
        curfileType = HTML;
    }
    else
    {
        pt3 = stristr(pt2, "application/pdf");
        if (pt3 != NULL && pt3 < pt2 + 15)
        {
            curfileType = PDF;
        }
        else
        {
            pt3 = stristr(pt2, "image/jpeg");
            if (pt3 != NULL && pt3 < pt2 + 10)
            {
                curfileType = JPG;
            }
            else
            {
                curfileType = OTHER;
            }
        }
    }

    printf("%s\n", response);
    char *pt = stristr(response, "Content-Length: ");
    // if content length is not present
    if (pt == NULL)
    {
        return;
    }
    int size = atoi(pt + 16);
    pt = stristr(response, "\r\n\r\n");
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
    // get port number
    char *port = strstr(url+7, ":");
    if (port != NULL)
    {
        port_no = atoi(port + 1);
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
    serv_addr.sin_port = htons(port_no);

    int status = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        perror("Unable to connect to server\n");
        exit(0);
    }

    send(sockfd, http_request, strlen(http_request), 0);
    // upload function complete
    upload_file(filename, sockfd);
    // -------
    char response[MAXLINE];
    // timeout of 3 seconds with poll
    struct pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, DURATION);
    if (ret == 0)
    {
        printf("Timeout\n");
        return;
    }
    int r = recv(sockfd, response, MAXLINE, MSG_WAITALL);
    printf("%s\n", response);
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
    if (strlen(path) == 0 || path[strlen(path) - 1] == '/')
        sprintf(request, "PUT /%s%s HTTP/1.1\r\nHost: %s\r\nDate: %s\r\n%s%s%s%s\r\n",
                path, filename, host, date, accept, accept_language, last_modified, content_length);
    else
        sprintf(request, "PUT /%s/%s HTTP/1.1\r\nHost: %s\r\nDate: %s\r\n%s%s%s%s\r\n",
                path, filename, host, date, accept, accept_language, last_modified, content_length);
}

/*
GET http://127.0.0.1:8000/Hello.txt
GET http://127.0.0.1:8000/queries.sql
GET http://127.0.0.1:8000/Assgn-3.pdf
*/

void download_file(char *filename, int sockfd, int size, char *content, int contstartlen)
{
    FILE *fp;
    fp = fopen(filename, "wb");
    fwrite(content, contstartlen, 1, fp);
    int r = 0;
    while (r < size)
    {
        char buffer[MAXLINE];
        // timeout of 3 seconds with poll
        struct pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = POLLIN;
        int ret = poll(&pfd, 1, DURATION);
        if (ret == 0)
        {
            printf("Timeout\n");
            return;
        }
        int n = recv(sockfd, buffer, MAXLINE, 0);
        fwrite(buffer, n, 1, fp);
        r += n;
    }
    fclose(fp);

    if (!fork())
    {
        if (curfileType == OTHER)
        {
            execvp("gedit", (char *[]){"gedit", filename, NULL});
        }
        else if (curfileType == HTML)
        {
            execvp("firefox", (char *[]){"firefox", filename, NULL});
        }
        else if (curfileType == PDF)
        {
            execvp("evince", (char *[]){"evince", filename, NULL});
        }
        else if (curfileType == JPG)
        {
            execvp("eog", (char *[]){"eog", filename, NULL});
        }
        exit(0);
    }
    wait(NULL);
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

char *stristr(const char *str1, const char *str2)
{
    const char *p1 = str1;
    const char *p2 = str2;
    const char *r = *p2 == 0 ? str1 : 0;

    while (*p1 != 0 && *p2 != 0)
    {
        if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
        {
            if (r == 0)
            {
                r = p1;
            }

            p2++;
        }
        else
        {
            p2 = str2;
            if (r != 0)
            {
                p1 = r + 1;
            }

            if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
            {
                r = p1;
                p2++;
            }
            else
            {
                r = 0;
            }
        }

        p1++;
    }

    return *p2 == 0 ? (char *)r : 0;
}