#define NOTFOUND 404
#define OK 200
#define BADREQUEST 400
#define FORBIDDEN 403

#define LOCALHOST "127.0.0.1"
#define MAXLINE 10
#define MAXCONNECTIONS 5
#define DURATION 5000
#define LOGGILE "AccessLog.txt"
#define BUF_SIZE 10000

int header_count_get = 6;
char *headers_get[] = {"Host:", "Date:", "Accept:", "Accept-Language:", "If-Modified-Since:", "Connection:" };


char *values[10];
char *method;
char *path;

