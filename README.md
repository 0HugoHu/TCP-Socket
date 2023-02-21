# TCP-Socket
Duke ECE 650 Project 3: TCP-Socket Programming. A hot potato game.

## Usage
Build using ```make```. To test:

In one terminal, run the ringmaster server.
In a second and following terminals, run the player client with the hostname of the server as a parameter.

To run multiple instances of a terminal on a single machine, consider using a program such as screen or tmux.

If both server and client are the same machine, use localhost - i.e. 127.0.0.1 or 0.0.0.0 for the client.

***Single machine example:***

On terminal 1:
```sh
$ ./ringmaster 5477 3 20
# Format: PORT, NUM_PLAYERS, NUM_HOPS
```
On terminal 2:
```sh
$ ./player localhost 5477
# Format: HOST_NAME, PORT
```

On terminal 3:
```sh
$ ./player localhost 5477
# Format: HOST_NAME, PORT
```

On terminal 4:
```sh
$ ./player localhost 5477
# Format: HOST_NAME, PORT
```

**One Possible Results:**
```sh
# Server:
Potato Ringmaster
Players = 3
Hops = 20
Player 0 is ready to play
Player 1 is ready to play
Player 2 is ready to play
Ready to start the game, sending potato to player 2
Trace of potato:
2,1,0,1,0,2,0,1,2,1,0,1,2,0,2,0,2,1,0,2

# Client 1:
Connected as player 0 out of 3 total players
Sending potato to 2
Sending potato to 1
Sending potato to 2
Sending potato to 2
Sending potato to 1
Sending potato to 1
Sending potato to 1

# Client 2:
Connected as player 1 out of 3 total players
Sending potato to 2
Sending potato to 2
Sending potato to 0
Sending potato to 2
Sending potato to 0
Sending potato to 2

# Client 3:
Connected as player 2 out of 3 total players
Sending potato to 0
Sending potato to 1
Sending potato to 0
Sending potato to 1
Sending potato to 1
Sending potato to 0
I'm it
```
