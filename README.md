# Net-Pong
## About
This is a simple C project to demonstrate knowledge and understanding of
Systems Programming in a Unix environment. This net-pong program allows
two players to play a simple game of pong across a network.

Player 1 will create the game-server Player 2 will connect to, this server
is used to signal players of game updates such as score change or when
the ball is moving across the court divide.

## Build
```console
# compile the pong program
make build

# display the help screen
./pong -h
```

After compiling the program
```console
# start the game as player 1
./pong

# start server on desired port
./pong -p 1234

# connect to player 1's game as player 2
./pong -s 127.0.0.1

# connect to server on specific port
./pong -s 127.0.0.1 -p 1234
```
