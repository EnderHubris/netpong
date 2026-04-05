#include "server.h"

Server* pserver = NULL;
int running = 1;

static void serverKill() {
    running = 0;
    if (!pserver) return;

    // closing the server interupts blocking accept
    close(pserver->server_fd);

    // close conenctions
    for (int i = 0; i < pserver->clientsConnected; ++i) {
        // prevent read/write from client_fd (helps for graceful shutdown)
        shutdown(pserver->clients[i], SHUT_RDWR);
        // close client_fd
        close(pserver->clients[i]);
    }

    // ask children procs to die
    for (int i = 0; i < pserver->clientsConnected; ++i) {
        kill(pserver->pids[i], SIGTERM);
    }

    // potentially interupted or kill requested
    exit(2);
}

// thread target for listening to incoming client msgs
static void ListenForClient(int client) {
    char* msg = "Hello from server!";

    char buffer[BUFF_LEN] = { 0 };
    ssize_t bytesRecv;

    send(client, msg, strlen(msg), 0);

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

        //printf("[*] Recv (%lu) -> %s\n", bytesRecv, buffer);
        
        // split the recv buffer
        Strings res = SplitStr(buffer, ' ');

        // the split results should have the format
        // [ TYPE DATA ]
        if (res.stringCount != 2) continue;

        if (strcmp("SCORE", res.strs[0]) == 0) {
            // expecting 1 or 2
            //printf("Player %s has scored!\n", res.strs[1]);
        } else if (strcmp("PASS", res.strs[0]) == 0) {
            // expecting 1 or 2
            //printf("Ball has left court %s\n", res.strs[1]);
        }
    }

    // closing the connected socket
    close(client);
}

int RunServer(int port) {
    Server server = {0};
    pserver = &server;

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

    if (listen(server.server_fd, 3) < 0) {
        perror("listen");
        return EXIT_FAILURE;
    }

    signal(SIGINT, serverKill);  // ctrl+c
    signal(SIGTERM, serverKill);

    // accept only two incoming connections
    // (left player and right player)
    while (server.clientsConnected < 2) {
        int clientId = server.clientsConnected;

        server.clients[clientId] = accept(
            server.server_fd,
            s_addr,
            &addrlen
        );

        if (server.clients[clientId] < 0) {
            if (!running) break; // kill server early
            continue;
        }
    
        if (server.clients[clientId] < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }

        // printf("[!] Player has Connected!\n");

        // create another child proc for the clients
        pid_t cpid = fork();

        if (cpid < 0) {
            perror("fork");
            serverKill();
            exit(1);
        } else if (cpid == 0) {
            // child operation
            ListenForClient(server.clients[clientId]);
            exit(0);
        }

        // track the new children pids
        server.pids[clientId] = cpid;

        // when a client connects increment
        ++server.clientsConnected;
    }

    // inform player 1 that both players have joined
    kill(getppid(), SIGUSR1);

    // wait for child connections to die
    for (int i = 0; i < server.clientsConnected; i++) {
        waitpid(server.pids[i], NULL, 0);
    }
  
    // closing the listening socket
    close(server.server_fd);

    return 0;
}