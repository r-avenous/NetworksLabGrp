#define NOTFOUND 404
#define OK 200
#define BADREQUEST 400
#define FORBIDDEN 403

#define LOCALHOST "127.0.0.1"
#define MAXLINE 128
#define MAXCONNECTIONS 5
#define DURATION 5000
#define LOGGILE "AccessLog.txt"
#define BUF_SIZE 10000

int header_count_get = 6, header_count_put = 5;
char *headers_get[] = {"Host:", "Date:", "Accept:", "Accept-Language:", "If-Modified-Since:", "Connection:"};
char *headers_put[] = {"Host:", "Date:", "Content-Type:", "Content-Length:", "Connection:"};


char *values[10];
char *method;
char *path;

// stores the content received along with the headers
char extra_data[MAXLINE];
int extra_data_size = 0;

