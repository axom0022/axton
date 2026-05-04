#ifndef websocket_h
#define websocket_h

#include "../core/axton.h"

typedef struct websocketclient {
    int fd;
    char *path;
    char *protocol;
    object *onmessage;
    object *onclose;
    object *onerror;
    struct websocketclient *next;
} websocketclient;

typedef struct websocketserver {
    int port;
    int running;
    object *onconnection;
    struct websocketclient *clients;
    pthread_t thread;
} websocketserver;

object *wscreateserver(int port);
object *wsonconnection(object *handler);
object *wssend(void *client, char *data);
object *wsbroadcast(char *data);
object *wsclose(void *client);
object *wsstart(void);

#endif
