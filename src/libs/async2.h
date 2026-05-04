#ifndef async2_h
#define async2_h

#include "../core/axton.h"

typedef struct asynctask {
    object *coro;
    int state;
    double timeout;
    int fd;
    int events;
    struct asynctask *next;
} asynctask;

typedef struct asyncsocket {
    int fd;
    object *onconnect;
    object *ondata;
    object *onclose;
    object *onerror;
    void *data;
} asyncsocket;

object *asynceventloop(void);
object *asynccreate(object *func);
object *asyncawait(object *coro);
object *asyncsleep(double seconds);
object *asyncgather(object **tasks, int count);
object *asyncwaitfor(object *task, double timeout);
object *asyncsocketconnect(char *host, int port, object *onconnect);
object *asyncsocketlisten(int port, object *onconnect);
object *asyncsocketwrite(asyncsocket *sock, char *data);
object *asyncsocketclose(asyncsocket *sock);
object *asynctcpclient(char *host, int port);
object *asynctcpserver(int port, object *handler);
object *asyncexec(object *func);
object *asyncqueue(void);
object *asyncqueueput(object *queue, object *item);
object *asyncqueueget(object *queue);
object *asynclock(void);
void asynclockacquire(object *lock);
void asynclockrelease(object *lock);
void asyncrun(void);
void asyncstop(void);

#endif
