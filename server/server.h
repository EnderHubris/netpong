#ifndef PONG_SERVER
#define PONG_SERVER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../utils.h"

#define BUFF_LEN 1024

typedef struct {
    int server_fd;
    
    int clientsConnected;
    int clients[2];
    pid_t pids[2];

    struct sockaddr_in address;
} Server;
extern Server* pserver;
extern int running;

int RunServer(int port);

#endif