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
#define CONTENT_LENGTH_INDEX 0
#define MAX_USEFUL_HEADERS 7

int header_count_get = 6, header_count_put = 5, header_count_badreq = 1;
char *headers_get[] = {"Host:", "Date:", "Accept:", "Accept-Language:", "If-Modified-Since:", "Connection:"};
char *headers_put[] = {"Content-Length:", "Content-Type:", "Host:", "Date:", "Connection:"};
char *headers_badreq[] = {"Content-Length:"};

char *values[MAX_USEFUL_HEADERS];

char *method;
char *path;
char *version;

// stores the content received along with the headers
char extra_data[MAXLINE];
int extra_data_size = 0;

