# Project 1: Socket Programming

> Xavier Bonavita
> 11/15/2024

## How to run this program

1) Compile the program

- Use the provided makefile using the `make` command
- If manual compilation is preferred:
  - To compile http_client: `gcc -Wall -o http_client http_client.c`
  - To compile http_server: `gcc -Wall -o http_server http_server.c`

2) Execution of client
`./http_client [-p] <server_url> <port_number>`

- To run the http_client binary compiled in step 1, you will need to specify two arguments:
  - `server_url` is the url of the HTTP server you wish to connect to.
  - `port_number` is the port of said HTTP server (typically 80).
- There is also an optional argument:
  - `-p` when specified, will print out the RTT of the HTTP request.
- An example of a full request would be as follows:
  - `./http_client -p www.google.com 80`

3) Execution of server
`./http_server <port_number>`

- To run the http_server binary compiled in step 1, only one argument is necessary:
  - `port_number` is the port the server will use to communicate with the client. It will default to `8888` if no port is specified.
  - It is possible that you may get an error saying "Address already in use". If this occurs, change the port to a different number and try again.
- To have the client connect to this server:
  - use `localhost` for the `server_url`
  - use whatever port your server is listening to for `port_number`
  - ex: `./http_client [-p] localhost 3621`
