# http-server-c
A basic http server coded in c

Implementation Details:

	The server does not support pipelined requests or post request.

	The server uses select() to check for new connections, and accepts connections before creating a new thread which the server passes the connection fd to using a dynamically allocated variable.
	
	Threads are immedietly detatched in the main routine and clean up all their own reasources, including freeing the memory for the dynamically allocated variable and closing any file descriptors.

Running the server:
	
	To compile, type "make" into the terminal.

	To run, type "./webserver.o <port number>" into the terminal.
