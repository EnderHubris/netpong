/**
 * Author: Noah Al-lahabi
 * Date: 4/2/2026
 */
#include "cli.h" // custom c import i developed -> https://github.com/EnderHubris/CCLI

#include "server/server.h"
#include "server/client.h"

#include "game/game.h"

static int isUp(int ch) {
    return ch == (int)'j' || ch == (int)'w';
}

static int isDown(int ch) {
    return ch == (int)'k' || ch == (int)'s';
}

// run in current process
static void InitGame(int playerId) {
    setup(playerId);
    // if (playerId == 0) serve();
    
    // game main loop
    int ch;
    while ( gameRunning(&ch) ) {
        if (isUp(ch)) {
            moveUp();
        } else if (isDown(ch)) {
            moveDown();
        }
    
        paddleHandle();
    }
    cleanCurses();
}

/**
 * Net-Pong game initiator will create a game server
 * that both players connect to
 */
static pid_t HostGame(int write_fd, int serverPort) {
    pid_t cpid = fork();

    if (cpid < 0) {
        perror("fork");
        exit(1);
    }

    if (cpid == 0) {
        printf("[!] Pong-Server is Starting\n");
        RunServer(write_fd, serverPort);
        exit(0);
    }

    printf(" |___ Pong-Server is Active!\n");
    return cpid;
}

/**
 * Net-Pong creators create a socket connect to the
 * game server
 */
static pid_t ConnectToGame(char* serverHost, int serverPort) {
    pid_t cpid = fork();

    if (cpid < 0) {
        perror("fork");
        exit(1);
    }

    if (cpid == 0) {
        RunClient(serverHost, serverPort);
        exit(0);
    }

    printf("[!] Pong-Client is Active!\n");
    return cpid;
}

int main(int argc, char** argv) {
    int serverPort = 6254;
    char* serverHost = NULL;

    App app = createApp("Net-Pong");

    app.AddOption("-s", "--server", "IP of Server", O_STRING, &serverHost, &app);
    app.AddOption("-p", "--port", "Port of Server [Default 6254]", O_INTEGER, &serverPort, &app);

    parseCLI(&app, argc, argv);

    if (!serverHost) {
        // create pipe used for IPC
        int pipefd[2];
        // pipefd[0] = parent reads, pipefd[1] = child writes
        pipe(pipefd);

        // this is Player 1
        pid_t server_p = HostGame(pipefd[1], serverPort);

        // allow the server time to start up before connecting
        sleep(1);

        // player 1 socket
        pid_t sock_p = ConnectToGame(serverHost, serverPort);

        // wait for player 2 to connect
        // when player 2 connects the pipe will recv data
        close(pipefd[1]);
        char buf[100] = {0};
        while (1) {
            // wait for child to send via the pipe
            ssize_t n = read(pipefd[0], buf, 99);
            if (n == 0) continue;
            buf[n] = '\0';
            if (strcmp(buf, "START") == 0) break;
        }

        printf("[!] Net-Pong Starting\n");
        InitGame(0);
        printf(" |___ Pong Game Exited\n");

        // send kill signal to PIDs
        kill(sock_p, SIGTERM);
        kill(server_p, SIGTERM);

        // wait for pids to die
        waitpid(sock_p, NULL, 0);
        waitpid(server_p, NULL, 0);

        printf("[!] Net-Pong Exiting\n");
    } else {
        // this is Player 2
        printf("[+] Connecting -> %s %d\n", serverHost, serverPort);
        
        // player 2 socket
        pid_t sock_p = ConnectToGame(serverHost, serverPort);
        
        InitGame(1);
        printf(" |___ Pong Game Exited\n");

        kill(sock_p, SIGTERM);
        
        waitpid(sock_p, NULL, 0);
        printf("[!] Net-Pong Exiting\n");
    }

    return 0;
}