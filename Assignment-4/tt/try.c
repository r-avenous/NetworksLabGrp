#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <file_name>\n", argv[0]);
        return 1;
    }

    struct stat file_stat;
    if (stat(argv[1], &file_stat) == -1)
    {
        perror("stat");
        return 1;
    }

    struct tm *timeinfo;
    timeinfo = gmtime(&file_stat.st_mtime);
    char buffer[80];
    strftime(buffer, 80, "%a, %d %b %Y %T GMT", timeinfo);
    printf("Last modified (HTTP format): %s\n", buffer);
    return 0;
}
