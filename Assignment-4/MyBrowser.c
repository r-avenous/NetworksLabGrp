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

void get(char *url);                    // implements GET {url}
void put(char *url, char *filename);    // Implements PUT {url} <filename>

int main()
{
    // assumed url max size
    char cmd[4], url[100], filename[100];
    while(1){
        printf("MyOwnBrowser> ");
        scanf("%s, %s", cmd, url);
        if(strcmp(cmd, "GET")==0){
            get(url);
        }
        else if(strcmp(cmd, "PUT")==0){
            scanf("%s", filename);
            put(url, filename);
        }
        else if(strcmp(cmd, "QUIT")==0){
            printf("Bye\n");
            break;
        }
        else{
            printf("Invalid command\n");
        }
    }
    return 0;
}