#ifndef PONG_CLIENT
#define PONG_CLIENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFF_LEN 1024

typedef struct {
    int client_fd;

    int status;
    int bytesRecv;

    struct sockaddr_in server_addr;
} Client;
Client* pclient = NULL;

static void killClient() {
    if (!pclient) return;
    close(pclient->client_fd);
    exit(2);
}

static Client CreateClient() {
    Client client = { 0 };

    // set server target and client_fd
    client.client_fd = socket(AF_INET, SOCK_STREAM, 0);
    client.client_fd = socket(AF_INET, SOCK_STREAM, 0);

    return client;
}

static void ListenToServer(Client* client) {
    char buffer[BUFF_LEN] = { 0 };

    while (client) {
        // subtract 1 for the null-terminator at the end
        int read_amount = BUFF_LEN - 1;
        // recv from server
        client->bytesRecv = read(client->client_fd, buffer, read_amount);
        //printf("%s\n", buffer);
    }
}

int RunClient(char* hostStr, int port) {
    Client client = CreateClient();
    pclient = &client;

    char* msg = "Hello from client";

    if (client.client_fd < 0) {
        perror("socket");
        return -1;
    }

    client.server_addr.sin_family = AF_INET;
    client.server_addr.sin_port = htons(port);

    // convert host string to binary format
    int inet_ret = inet_pton(
        AF_INET,
        hostStr ? hostStr : "127.0.0.1",
        &client.server_addr.sin_addr);
    if (inet_ret != 1) {
        perror("inet_pton");
        return -1;
    }

    // connect to server
    struct sockaddr* serv_sock_addr = (struct sockaddr*) &client.server_addr;
    client.status = connect(
        client.client_fd, 
        serv_sock_addr,
        sizeof(client.server_addr)
    );

    if (client.status < 0) {
        perror("connect");
        return -1;
    }

    signal(SIGINT, killClient);  // ctrl+c
    signal(SIGTERM, killClient); // kill

    // send message to server
    send(client.client_fd, msg, strlen(msg), 0);
    // ("Hello message sent\n");

    ListenToServer(&client);

    // close client socket
    close(client.client_fd);
    return 0;
}

#endif