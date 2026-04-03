/**
 * Author: Noah Al-lahabi
 * Date: 4/2/2026
 */
#include "cli.h" // custom c import i developed -> https://github.com/EnderHubris/CCLI

#include "server/server.h"
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
    // serve();
    
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

int main(int argc, char** argv) {
    int serverPort = 6254;
    char* serverHost = NULL;

    App app = createApp("Net-Pong");

    app.AddOption("-s", "--server", "IP of Server", O_STRING, &serverHost, &app);
    app.AddOption("-p", "--port", "Port of Server [Default 6254]", O_INTEGER, &serverPort, &app);

    parseCLI(&app, argc, argv);

    if (!serverHost) {
        // this is server
        pid_t cpid = fork();

        if (cpid < 0) {
            perror("fork");
            return 1;
        }

        // child runs the socket server
        if (cpid == 0) {
            exit(RunServer(serverPort));
        }

        // parent does parantal tasks
        printf("[!] Pong-Server is Active!\n");

        InitGame(0);

        printf(" |___ Pong Game Exited\n");
        wait(NULL);
        printf("[!] Net-Pong Exiting\n");
    } else {
        // this is client
        printf("[+] Connecting -> %s %d\n", serverHost, serverPort);
        InitGame(1);
    }

    return 0;
}