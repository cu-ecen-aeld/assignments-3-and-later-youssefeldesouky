#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#define IDENT "Writer"

int main(int argc, char **argv){
    if(argc < 3){
        puts("Usage: ./writer writefile writestr");
        return 1;
    }

    char *writefile = argv[1];
    char *writestr = argv[2];

    openlog(IDENT, LOG_PID, LOG_USER);

    errno = 0;
    int fd = open(writefile, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd == -1){
        syslog(LOG_ERR, "Error: open(): %s\n", strerror(errno));
        closelog();
        return 1;
    }

    if(write(fd, writestr, strlen(writestr)) == -1){
        syslog(LOG_ERR, "Error: write(): %s\n", strerror(errno));
        if(close(fd) == -1){
            syslog(LOG_ERR, "Error: close(): %s\n", strerror(errno));
        }
        closelog();
        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

    if(close(fd) == -1){
        syslog(LOG_ERR, "Error: close(): %s\n", strerror(errno));
    }
    
    closelog();
    return 0;
}