#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_REQUEST_QUEUE 128
#define MAX_CONNECTION_QUEUE 128
#define SERVER_ERROR "HTTP/1.1 500 Internal Server Error\nThese messages in the exact format as shown above should be sent back to the client if any of the above error occurs."


void * serve_single_request(void * data);

#endif