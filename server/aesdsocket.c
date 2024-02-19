#include <unistd.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <syslog.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOG_PATH "/var/tmp/aesdsocketdata"
#define BUF_SIZE 1024

int sockfd = 0, clientfd = 0;
int tmpfd = 0;
char *buf = NULL;

void sighandle(int signum){
    close(clientfd);
    close(sockfd);
    close(tmpfd);
    unlink(LOG_PATH);
    free(buf);
    syslog(LOG_NOTICE, "Caught signal, exiting");
    closelog();
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
    int retval;
    struct addrinfo hints, *res;
    struct sockaddr client_addr;
    socklen_t client_addr_len;
    struct sigaction sa;
    bool daemon = false;

    if(argc > 1){
        if(!strcmp(argv[1], "-d")){
            daemon = true;
            puts("ags");
        }
    }

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = sighandle;
    
    // Set the hints structure 
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    retval = getaddrinfo(NULL, "9000", &hints, &res);
    if(retval != 0){
        fprintf(stderr, "Error: getaddrinfo: %s\n", gai_strerror(retval));
        goto RET;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        fprintf(stderr, "Error: socket: %s\n", strerror(errno));
        goto FRE;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    retval = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if(retval == -1){
        fprintf(stderr, "Error: bind: %s\n", strerror(errno));
        goto CLO;
    }

    //daemonize here according to desc


    retval = listen(sockfd, 1);
    if(retval == -1){
        fprintf(stderr, "Error: listen: %s\n", strerror(errno));
        goto CLO;
    }

    openlog(NULL, LOG_PID, LOG_USER);

    if(sigaction(SIGINT, &sa, NULL) || sigaction(SIGTERM, &sa, NULL)){
        fprintf(stderr, "Error: sigaction: %s\n", strerror(errno));
        goto CLO;
    }

    
    buf = malloc(BUF_SIZE * sizeof(*buf));
    if(!buf){
        fprintf(stderr, "Error: malloc: %s\n", strerror(errno));
        goto CLO;
    }

    freeaddrinfo(res);
    //better daemon spot
    if(daemon){
        int pid = fork();
        if(pid == -1){
            fprintf(stderr, "Error: fork: %s\n", strerror(errno));
            goto CLO;
        }else if(pid != 0){
            //parent
            exit(EXIT_SUCCESS);
        }
        //child
        setsid(); //change session id to detach from shell session id
        pid = fork(); //fork again to ensure that it isn't session leader and can't own a shell
        if(pid == -1){
            fprintf(stderr, "Error: fork: %s\n", strerror(errno));
            goto CLO;
        }else if(pid != 0){
            //parent
            exit(EXIT_SUCCESS);
        }
        umask(0); //change umask
        chdir("/"); //change directory to root
        //close all std files
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    while(1){
        ssize_t bytes = 0;
        
        //accept connection
        client_addr_len = sizeof(struct sockaddr);
        clientfd = accept(sockfd, &client_addr, &client_addr_len);
        if(clientfd == -1){
            syslog(LOG_ERR, "Couldn't connect to client, trying again...");
            continue;
        }

        struct in_addr sin_addr = ((struct sockaddr_in *)&client_addr)->sin_addr;
        char ip_addr[INET_ADDRSTRLEN];
        (void)inet_ntop(AF_INET, (void *)&sin_addr, ip_addr, INET_ADDRSTRLEN);

        syslog(LOG_INFO, "Accepted connection from %s", ip_addr);

        tmpfd = open(LOG_PATH, O_RDWR | O_CREAT | O_APPEND, 0755);
        if(tmpfd == -1){
            syslog(LOG_ERR, "Couldn't open log file, %s...", strerror(errno));
            close(clientfd);
            continue;
        }

        while((bytes = recv(clientfd, buf, BUF_SIZE, 0)) > 0){
            write(tmpfd, buf, bytes);
            if(buf[bytes - 1] == '\n'){
                break;
            }
            
        }
        sendfile(clientfd, tmpfd, &(off_t){0}, lseek(tmpfd, 0, SEEK_END));
        close(clientfd);
        syslog(LOG_INFO, "Closed connection from %s", ip_addr);
        close(tmpfd);
    }
    
CLO:
    close(sockfd);
FRE:
    freeaddrinfo(res);
RET:
    return -1;
}