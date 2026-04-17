#include "server.h"

Server* pserver = NULL;
int running = 1;
int* GameScore = NULL;

// debug file
FILE* logFile = NULL;

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

/**
 * Sends the current position of the ball in a player's court
 * after a pass is initiated
 */
static void PassBall(int ballData[4], int socket_fd) {
    char passMsg[124];
    snprintf(passMsg, sizeof(passMsg), "BALL %d %d %d %d\n",
        ballData[0],
        ballData[1],
        ballData[2],
        ballData[3]
    );
    write(socket_fd, passMsg, strlen(passMsg));

    // file write
    if (socket_fd == pserver->clients[0]) {
        fprintf(logFile, "> %s", passMsg);
    }
}

/**
 * Sends the current score the server is tracking to
 * a player socket
 */
static void SendScore(int socket_fd) {
    char scoreMsg[124];
    snprintf(scoreMsg, sizeof(scoreMsg), "SCORE %d %d\n",
        GameScore[0],
        GameScore[1]
    );
    write(socket_fd, scoreMsg, strlen(scoreMsg));

    // file write
    if (socket_fd == pserver->clients[0]) {
        fprintf(logFile, "> %s", scoreMsg);
    }
}

/**
 * Send a signal to a player to serve the ball after
 * a score update
 */
static void SendServe(int socket_fd) {
    char reserveMsg[124];
    snprintf(reserveMsg, sizeof(reserveMsg), "SERVE \n");
    write(socket_fd, reserveMsg, strlen(reserveMsg));

    // file write
    if (socket_fd == pserver->clients[0]) {
        fprintf(logFile, "> %s", reserveMsg);
    }
}

static void SendGameOver(int socket_fd) {
    char msg[124];
    snprintf(msg, sizeof(msg), "GAMEOVER \n");
    write(socket_fd, msg, strlen(msg));

    // file write
    if (socket_fd == pserver->clients[0]) {
        fprintf(logFile, "> %s", msg);
    }
}

static void SendWinHeader(int socket_fd) {
    char msg[124];
    snprintf(msg, sizeof(msg), "WINNER %d\n", ( GameScore[0] == GAMEOVER ) ? 0 : 1 );
    write(socket_fd, msg, strlen(msg));

    // file write
    if (socket_fd == pserver->clients[0]) {
        fprintf(logFile, "> %s", msg);
    }
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
            if (res.stringCount != 2) continue;

            int victorId = atoi(res.strs[1]); // 0 or 1
            ++GameScore[victorId];

            SendScore(pserver->clients[0]);
            SendScore(pserver->clients[1]);

            if ( GameScore[0] >= GAMEOVER || GameScore[1] >= GAMEOVER ) {
                SendWinHeader(pserver->clients[1]);
                SendWinHeader(pserver->clients[0]);

                sleep(3); // give player time to see end-game message

                SendGameOver(pserver->clients[1]);
                SendGameOver(pserver->clients[0]);
                continue;
            }

            // round victor re-serves the ball
            SendServe(pserver->clients[victorId]);
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

    // create log file for debugging
    logFile = fopen("server.log", "w");
    setvbuf(logFile, NULL, _IONBF, 0);

    // create a patch of memory the parent and child procs
    // share similar to the producer-consumer problem
    GameScore = mmap(NULL, 2 * sizeof(int),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1, 0);
    GameScore[0] = GameScore[1] = 0;

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