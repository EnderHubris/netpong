#ifndef PONG_SERVER
#define PONG_SERVER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    struct sockaddr_in address;
} Server;

static void ListenForClient(int client) {
    char* msg = "Hello from server!";

    char buffer[BUFF_LEN] = { 0 };
    ssize_t bytesRecv;

    printf("%s\n", buffer);
    send(client, msg, strlen(msg), 0);
    printf("Hello message sent\n");

    while (1) {
        // subtract 1 for the null-terminator at the end
        int read_amount = BUFF_LEN - 1;
        bytesRecv = read(client, buffer, read_amount);
        // remove new-line from buffer
        for (int i = 0; i < bytesRecv; ++i) {
            if (buffer[i] == '\n') {
                buffer[i] = 0;
                break;
            }
        }

        printf("[*] Recv (%lu) -> %s\n", bytesRecv, buffer);

        // split the recv buffer
        Strings res = SplitStr(buffer, ' ');
        printf(" |___ split count (%i)\n", res.stringCount);

        // the split results should have the format
        // [ TYPE DATA ]
        if (res.stringCount != 2) continue;

        if (strcmp("SCORE", res.strs[0]) == 0) {
            // expecting 1 or 2
            printf("Player %s has scored!\n", res.strs[1]);
        } else if (strcmp("PASS", res.strs[0]) == 0) {
            // expecting 1 or 2
            printf("Ball has left court %s\n", res.strs[1]);
        }
    }

    // closing the connected socket
    printf("[*] Closing Connection. . .");
    close(client);
}

int RunServer(int port) {
    Server server = {0};

    int opt = 1;
    socklen_t addrlen = sizeof(server.address);

    // Creating socket file descriptor
    if ((server.server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    int sock_set = setsockopt(
        server.server_fd,
        SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT,
        &opt,
        sizeof(opt)
    );

    // Forcefully attaching socket to the PORT
    if (sock_set) {
        perror("setsockopt");
        return EXIT_FAILURE;
    }

    // configure server address and port to bind to
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = INADDR_ANY;
    server.address.sin_port = htons(port);

    struct sockaddr* s_addr = (struct sockaddr*) &server.address;

    // Forcefully attaching socket to the PORT
    int r_bind = bind(
        server.server_fd,
        s_addr,
        sizeof(server.address)
    );

    if (r_bind < 0) {
        perror("bind failed");
        return EXIT_FAILURE;
    }

    printf("[+] Server Active -> 127.0.0.1:%i\n", port);

    if (listen(server.server_fd, 3) < 0) {
        perror("listen");
        return EXIT_FAILURE;
    }

    // accept only two incoming connections
    // (left player and right player)
    while (server.clientsConnected < 2) {
        int clientId = server.clientsConnected;

        server.clients[clientId] = accept(
            server.server_fd,
            s_addr,
            &addrlen
        );

        printf("[!] Detect Incoming Connection\n");
    
        if (server.clients[clientId] < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }

        printf(" |___ Accepted Connection!\n");

        // create another child proc for the clients
        pid_t cpid = fork();

        if (cpid < 0) {
            perror("fork");
        } else if (cpid == 0) {
            // child operation
            ListenForClient(server.clients[clientId]);
        }

        // when a client connects increment
        ++server.clientsConnected;
    }

    // wait for child connections to die
    wait(NULL);
  
    // closing the listening socket
    close(server.server_fd);

    return 0;
}

#endif