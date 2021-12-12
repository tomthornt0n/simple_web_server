
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "string.c"
#include "work_queue.c"
#include "http_parse.c"
#include "server.c"

int
main(int arguments_count, char **arguments){
    Server server = server_make(18000, 100, 16);
    for(;;){
        server_update(&server);
    }
    server_destroy(&server); // NOTE(tbt): a bit reduntant lol
    return 0;
}
