#ifndef PONG_CLIENT
#define PONG_CLIENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../utils.h"

#define BUFF_LEN 1024

typedef struct {
    int client_fd;

    int status;
    int bytesRecv;

    struct sockaddr_in server_addr;
} Client;
extern Client* pclient;

int RunClient(char* hostStr, int port);

#endif