#include "webserver.h"



//todo: add all needed header files

int serving_requests = 1; //global variable for if the program should continue running

// void end_thread(struct threadsafe_data * shared_data){
//     /* safely decrement the thread count and then unsafely exit thread*/
//     sem_wait(shared_data->thread_count_mutex);
//         shared_data->thread_count--;
//     sem_post(shared_data->thread_count_mutex);
//     pthread_exit(NULL);
// }

// void start_thread(struct threadsafe_data * shared_data){
//     /* safely increment the thread count, generate a new thread, and detach the thread */
//     pthread_t tid;
//     sem_wait(shared_data->thread_count_mutex);
//         shared_data->thread_count++;
//         pthread_create(&tid, NULL, &serve_single_request, (void *)shared_data);
//         pthread_detach(tid);
//     sem_post(shared_data->thread_count_mutex);
    
// }

// int pop_connection(struct threadsafe_data * shared_data){ 
//     int connection_fd;
//     sem_wait(shared_data->queue_mutex);
//         /* safely retreive a connection from the queue 
//             warning: this function assumes that it is not called too frequently
//         */
//         //get socket file descriptor for the connection from the queue
//         connection_fd = shared_data->connection_queue[shared_data->queue_start];

//         //move index for start of queue
//         shared_data->queue_start++;
//         if (shared_data->queue_start == MAX_CONNECTION_QUEUE)
//             shared_data->queue_start = 0;

//     sem_post(shared_data->queue_mutex);
//     return connection_fd;
// }

// void push_connection(int connection_fd, struct threadsafe_data * shared_data){
//     /* safley add a connection to the queue
//         warning: this function assumes the queue is not full
//     */
//     printf("Pushing connection \n");
//     sem_wait(shared_data->queue_mutex);
//         //add the socket file descriptor for the connection to the queue 
//         shared_data->connection_queue[shared_data->queue_end] = connection_fd;

//         //move index for end of queue
//         shared_data->queue_end++;
//         if (shared_data->queue_end == MAX_CONNECTION_QUEUE)
//             shared_data->queue_end = 0;
        
//     sem_post(shared_data->queue_mutex);
// }

void * serve_single_request(void * connection_pointer){
    char buffer[1024];
    struct threadsafe_data * shared_data;
    int connection_fd = *(int *)connection_pointer;
    free(connection_pointer);

    read(connection_fd, buffer, 1024);
    printf("%s\n", buffer);
    
    send(connection_fd, SERVER_ERROR, strlen(SERVER_ERROR),0);

    //todo: parse http request for method, parameters, etc
    //todo: search file system for requested reasource
    //todo: serve http request response
    //todo: release socket connection
    close(connection_fd);
    return(NULL);
}

void serve_pipleined_requests(void * data){ //todo figure out time correctly
    //todo: last, serve pipelined requests
}

void handle_exit(){
    serving_requests = 0;
}

int main(int argc, char ** argv){
    int socket_fd, portno, optval, socket_ready, server_addr_len;
    int *  connection_pointer;
    fd_set socket_set;
    pthread_t tid;
    struct timeval timeout;
    struct sockaddr_in server_addr;
    // struct threadsafe_data * shared_data;
    
    

    // shared_data = malloc(sizeof(struct threadsafe_data));
    
    // shared_data->thread_count_mutex = sem_open("/thread_count_mutex", O_CREAT, 0777, 1);
    // shared_data->queue_mutex = sem_open("/queue_mutex", O_CREAT, 0777, 1);

    // shared_data->queue_end = 0;
    // shared_data->queue_start = 0;
    // shared_data->thread_count = 0;

    server_addr_len = sizeof(server_addr);
    
    //todo: make sure input is well formatted

    signal(SIGINT, handle_exit); //register function to handle shutdown
    

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd  < 0) 
        perror("ERROR opening socket");
    
    optval = 1;
    if (setsockopt(socket_fd , SOL_SOCKET, SO_REUSEADDR, 
        (const void *)&optval , sizeof(int)) < 0)
        perror("ERROR on setsockopt");

    portno = atoi(argv[1]);
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)portno);

    if (bind(socket_fd, (struct sockaddr *) &server_addr, //bind socket to port
        server_addr_len) < 0) 
        perror("ERROR on binding");
    
    if(listen(socket_fd, 
        MAX_REQUEST_QUEUE) < 0) 
        perror("ERROR on listening");

    while(serving_requests){
            printf("Here\n");

            FD_ZERO(&socket_set);
            FD_SET(socket_fd, &socket_set);

            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

            socket_ready = select(socket_fd + 1, &socket_set, NULL, NULL, &timeout);

            if (socket_ready < 0){
                perror("Error on select");
            }
            else if (socket_ready)
            {
                printf("Accepting Connections\n");
                
                
                connection_pointer = malloc(sizeof(int));
                *connection_pointer = accept(socket_fd, (struct sockaddr *)&server_addr, (socklen_t*)&server_addr_len);
                if (*connection_pointer < 0)
                    perror("ERROR on accept");

                pthread_create(&tid, NULL, serve_single_request, (void *)connection_pointer);
                pthread_detach(tid);
                // push_connection(connection_fd, shared_data);
                // printf("Added connection to list\n");
                // start_thread(shared_data);
                // printf("Started thread\n");
            }  
    }
    close(socket_fd);
    //loop to check that all threads have terminated
    return(0);
}

//todo: handle post requests somehow
