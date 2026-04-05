LD_FLAGS = -lncurses
CC_FLAGS = -Wall -Wextra

all: build clean_o
	./pong -h

# build rules focusing on PONG game
rules_o:
	gcc -c $(CC_FLAGS) game/rules.c -o rules.o
paddle_o:
	gcc -c $(CC_FLAGS) game/paddle.c -o paddle.o
ball_o:
	gcc -c $(CC_FLAGS) game/ball.c -o ball.o
game_o:
	gcc -c $(CC_FLAGS) game/game.c -o game.o

# build rules focusing on SERVER

# generic rules
build: rules_o paddle_o ball_o game_o
	# download cli.h used in main.c
	curl -kLO 'https://github.com/EnderHubris/CCLI/raw/refs/heads/main/cli.h'
	# compile project
	gcc $(CC_FLAGS) main.c rules.o game.o paddle.o ball.o -o pong $(LD_FLAGS)

clean_o:
	rm -f *.o
clean: clean_o
	rm -f pong
