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

static void PassBall(int ballData[4], int socket_fd) {
    char passMsg[124];
    snprintf(passMsg, sizeof(passMsg), "BALL %d %d %d %d\n",
        ballData[0],
        ballData[1],
        ballData[2],
        ballData[3]
    );
    write(socket_fd, passMsg, strlen(passMsg));
}

// thread target for listening to incoming client msgs
static void ListenForClient(int client, int playerId) {
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
        
        // split the recv buffer
        Strings res = SplitStr(buffer, ' ');

        // the split results should have the format
        // [ TYPE DATA ]
        if (res.stringCount == 0) continue;
        if (!pserver) break;

        if (strcmp("SCORE", res.strs[0]) == 0) {
        } else if (strcmp("PASS", res.strs[0]) == 0) {
            if (res.stringCount != 5) continue;

            // extract ball data [1:4]
            int ballData[4] = {0};
            for (int i = 0; i < 4; ++i) {
                ballData[i] = atoi(res.strs[i+1]);
            }

            if (playerId == 0) {
                // alert player 2
                ballData[0] = 2; // change x position to be valid to respective player area
                PassBall(ballData, pserver->clients[1]);
            } else {
                // alert player 1
                ballData[0] = 68; // rules.h (WIDTH-2)
                PassBall(ballData, pserver->clients[0]);
            }
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

        // when a client connects increment
        ++server.clientsConnected;
    }

    // make fork after both clients connect so the forks
    // (children) can use the socket_fds defined in the parent
    for (int i = 0; i < server.clientsConnected; ++i) {
        // create another child proc for the clients
        pid_t cpid = fork();
    
        if (cpid < 0) {
            perror("fork");
            serverKill();
            exit(1);
        } else if (cpid == 0) {
            // child operation
            ListenForClient(server.clients[i], i);
            exit(0);
        }

        // track the new children pids
        server.pids[i] = cpid;
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