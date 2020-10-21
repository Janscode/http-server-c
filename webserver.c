#include "webserver.h"

int serving_requests = 1; //global variable for if the program should continue running
static char* error_string = "HTTP/1.1 500 Internal Server Error\r\n";
static char* success_string = " 200 OK";
static char* newline_string = "\r\n";
static char* type_string = "Content-Type: ";
static char* len_string = "Content-Length: ";


static char* html_type = "text/html";
static char* txt_type = "text/plain";
static char* png_type = "image/png";
static char* gif_type =  "image/gif";
static char* jpg_type = "image/jpg";
static char* css_type = "text/css";
static char* js_type = "application/javascript";


void * serve_single_request(void * connection_pointer){
    char request_buffer[1024];
    char response_buffer[1024];
    char * http_method;
    char * reasource;
    char * http_version;
    char * http_type;
    char * delimit = " \t\r\n\v\f";
    char content_len[10]; //this will break if there are realllllly long files todo: malloc all memory that needs it
    int connection_fd, did_error, file_size, bytes_copied;
    FILE * fd;

    did_error = 0;

    connection_fd = *(int *)connection_pointer;
    free(connection_pointer);

    read(connection_fd, request_buffer, 1024);

    http_method = strtok(request_buffer, delimit);
    reasource = strtok(NULL, delimit);
    http_version = strtok(NULL, delimit);

    //need to use http_method here

    if (strlen(reasource) == 1){
        fd = fopen("www/index.html" , "r");
        if (fd == NULL) 
            fd = fopen("www/index.htm", "r");
        if (fd == NULL){
            perror("Error on fopen");
            did_error = 1;
        }
        http_type = html_type;
    }
    else {
        //this part overwrites http method
        strncpy(request_buffer + 1, "www", 3); //todo: support post request
        reasource = request_buffer + 1;
        fd = fopen(reasource, "r");
        if (fd == NULL){
            perror("Error on fopen");
            did_error = 1;
        } 
        else{
            strtok(reasource, ".");
            http_type = strtok(NULL, ".");
            printf("%s\n", http_type);

            if (strlen(http_type) == 4){
                http_type = html_type;
            }
            else if (strlen(http_type) == 3){
                if (!strncmp(http_type, "gif", 3))
                    http_type = gif_type;
                if (!strncmp(http_type, "jpg", 3))
                    http_type = jpg_type;
                if (!strncmp(http_type, "png", 3))
                    http_type = png_type;
                if (!strncmp(http_type, "txt", 3))
                    http_type =  txt_type;
                if (!strncmp(http_type, "css", 3))
                    http_type = css_type;
            }
            else if (strlen(http_type) == 2){
                http_type = js_type;
            }
        }
    }

    //consulted https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c how to tell size of file
    fseek(fd, 0L, SEEK_END);
    file_size = ftell(fd);
    rewind(fd);
    sprintf(content_len, "%d", file_size);

    send(connection_fd, http_version, strlen(http_version), 0);
    send(connection_fd, success_string, strlen(success_string), 0);
    send(connection_fd, newline_string, strlen(newline_string), 0); 
    send(connection_fd, len_string, strlen(len_string), 0);
    send(connection_fd, content_len, strlen(content_len), 0);
    send(connection_fd, newline_string, strlen(newline_string), 0);
    send(connection_fd, type_string, strlen(type_string), 0);
    send(connection_fd, http_type, strlen(http_type), 0);
    send(connection_fd, newline_string, strlen(newline_string), 0);
    send(connection_fd, newline_string, strlen(newline_string), 0);

    while((bytes_copied = fread(response_buffer, sizeof(char), 1028, fd))  > 0){
        send(connection_fd, response_buffer, bytes_copied, 0);
    }

    // send(connection_fd, error_string, strlen(error_string),0);
    fclose(fd);
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
            }  
    }
    close(socket_fd);
    return(0);
}

//todo: handle post requests somehow
