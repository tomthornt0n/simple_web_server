
typedef struct{
    WQ_State *work_queue;
    int listen_fd;
}Server;

static Server
server_make(int port, int backlog, int threads_count){
    Server result;
    result.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(result.listen_fd < 0){
        fprintf(stderr, "error making socket: %s\n", strerror(errno));
    }
    
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };
    int rc;
    rc = bind(result.listen_fd, (struct sockaddr *)&server_address, sizeof(server_address));
    if(rc < 0){
        fprintf(stderr, "error binding socket: %s\n", strerror(errno));
    }
    rc = listen(result.listen_fd, backlog);
    if(rc < 0){
        fprintf(stderr, "error listening on socket: %s\n", strerror(errno));
    }
    
    result.work_queue = wq_make_and_spawn_thread_pool(threads_count);
    
    return result;
}

typedef struct{
    Server *server;
    int fd;
}ServerConnection;

static String8
server_response_content_from_request(HTTP_Request *request){
    String8 result = string8_literal
    (
     "<html>"
     "  <head>"
     "    <meta charset=\"UTF-8\">"
     "    <title>terrible web page</title>"
     "  </head>"
     "  <body>"
     "    <h3>this is a header</h3>"
     "    <p>this is a paragraph</p>"
     "  </body>"
     "</html>"
     );
    
    fprintf(stderr, "received request:\n\tmethod\t:\t%.*s\n\turi\t:\t%.*s\n\tver\t:\t%.*s\n\n",
            fmt_string8(request->method),
            fmt_string8(request->uri),
            fmt_string8(request->version));
    
    // TODO(tbt): implement me properly
    return result;
}

static void
server_handle_connection(void *arg){
    ServerConnection connection = *((ServerConnection *)arg);
    free(arg);
    
    uint8_t request_buffer[4096] = {0};
    String8 request = { .len = 0, .buffer = request_buffer };
    int n;
    do{
        size_t size_to_read = sizeof(request_buffer) - request.len - 1;
        char *cursor = request.buffer + request.len;
        int n = read(connection.fd, cursor, size_to_read);
        request.len += n;
    }while(n > 0);
    if(n < 0){
        fprintf(stderr, "error while reading request: %s\n", strerror(errno));
    }
    
    HTTP_Request parsed_request = http_parse_request(request);
    String8 response_content = server_response_content_from_request(&parsed_request);
    String8 response = string8_from_fmt("HTTP/1.0 200 OK\r\n\r\n%.*s", fmt_string8(response_content));
    write(connection.fd, response.buffer, response.len);
    close(connection.fd);
    free(response.buffer);
    http_free_request(&parsed_request);
}

static void
server_update(Server *server){
    int connection_fd = accept(server->listen_fd, NULL, NULL);
    if(connection_fd >= 0){
        ServerConnection *connection = malloc(sizeof(*connection));
        connection->server = server;
        connection->fd = connection_fd;
        wq_push(server->work_queue, server_handle_connection, connection);
    }
}

static void
server_destroy(Server *server){
    close(server->listen_fd);
}
