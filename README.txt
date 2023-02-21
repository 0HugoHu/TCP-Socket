TCP example by Brian Rogers, updated by Rabih Younes. Duke University.
-----------------------------------------------------------------------

Build using make. To test:

In one terminal, run the server.
In a second terminal, run the client with the hostname of the server as a parameter.

To run multiple instances of a terminal on a single machine, consider using a program such as screen or tmux.

If both server and client are the same machine, use localhost - i.e. 127.0.0.1 or 0.0.0.0 for the client.

Single machine example:
  On terminal 1:
    $ ./server
  On terminal 2:
    $ ./client 0.0.0.0

Multiple machine example:
  On a machine called 'host1':
    $ ./server
  On another machine:
    $ ./client host1
