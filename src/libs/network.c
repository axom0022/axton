#include "../core/axton.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

typedef struct networkpeer {
    int fd;
    struct sockaddr_in addr;
    char *id;
    float latency;
    object *ondata;
    object *onconnect;
    object *ondisconnect;
} networkpeer;

typedef struct networkserver {
    int fd;
    int port;
    int running;
    pthread_t thread;
    networkpeer **peers;
    int peercount;
    int maxpeers;
    object *onconnect;
    object *ondisconnect;
    object *ondata;
    struct sockaddr_in addr;
} networkserver;

static networkserver *server = NULL;

object *netcreate(object **args, int argc, void *env) {
    networkserver *s = malloc(sizeof(networkserver));
    s->peers = NULL; s->peercount=0; s->maxpeers=32;
    s->onconnect=NULL; s->ondisconnect=NULL; s->ondata=NULL;
    s->running=0;
    server=s;
    return makenative(s, NULL);
}

object *netlisten(object **args, int argc, void *env) {
    if (argc<1 || !server) throwexception("listen needs port");
    server->port = args[0]->ival;
    server->fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt=1; setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    server->addr.sin_family = AF_INET;
    server->addr.sin_addr.s_addr = INADDR_ANY;
    server->addr.sin_port = htons(server->port);
    bind(server->fd, (struct sockaddr*)&server->addr, sizeof(server->addr));
    listen(server->fd, 10);
    server->running=1;
    return makenone();
}

static void *netthread(void *arg) {
    networkserver *s = (networkserver*)arg;
    fd_set readfds;
    struct timeval tv;
    while (s->running) {
        FD_ZERO(&readfds);
        FD_SET(s->fd, &readfds);
        int maxfd = s->fd;
        for (int i=0;i<s->peercount;i++) {
            if (s->peers[i] && s->peers[i]->fd > 0) {
                FD_SET(s->peers[i]->fd, &readfds);
                if (s->peers[i]->fd > maxfd) maxfd = s->peers[i]->fd;
            }
        }
        tv.tv_sec = 0; tv.tv_usec = 10000;
        select(maxfd+1, &readfds, NULL, NULL, &tv);
        if (FD_ISSET(s->fd, &readfds)) {
            int peerfd = accept(s->fd, NULL, NULL);
            if (peerfd > 0 && s->peercount < s->maxpeers) {
                networkpeer *p = malloc(sizeof(networkpeer));
                p->fd = peerfd;
                p->id = NULL;
                p->latency = 0;
                p->ondata = s->ondata;
                p->onconnect = s->onconnect;
                p->ondisconnect = s->ondisconnect;
                s->peers = realloc(s->peers, (s->peercount+1)*sizeof(networkpeer*));
                s->peers[s->peercount++] = p;
                if (s->onconnect) {
                    object *args[1] = {makeint(peerfd)};
                    callfunc(s->onconnect, args, 1, globalenv);
                }
            } else {
                close(peerfd);
            }
        }
        for (int i=0;i<s->peercount;i++) {
            networkpeer *p = s->peers[i];
            if (p && FD_ISSET(p->fd, &readfds)) {
                char buf[4096];
                int n = recv(p->fd, buf, sizeof(buf)-1, 0);
                if (n > 0) {
                    buf[n] = 0;
                    if (p->ondata) {
                        object *args[2] = {makeint(p->fd), makestring(buf)};
                        callfunc(p->ondata, args, 2, globalenv);
                    }
                } else {
                    if (p->ondisconnect) {
                        object *args[1] = {makeint(p->fd)};
                        callfunc(p->ondisconnect, args, 1, globalenv);
                    }
                    close(p->fd);
                    free(p);
                    for (int j=i;j<s->peercount-1;j++) s->peers[j]=s->peers[j+1];
                    s->peercount--;
                    i--;
                }
            }
        }
    }
    return NULL;
}

object *netstart(object **args, int argc, void *env) {
    if (!server) throwexception("server not created");
    pthread_create(&server->thread, NULL, netthread, server);
    pthread_detach(server->thread);
    return makenone();
}

object *netsend(object **args, int argc, void *env) {
    if (argc<2) throwexception("send needs peerfd data");
    int fd = args[0]->ival;
    char *data = args[1]->sval;
    if (!server) throwexception("server not started");
    for (int i=0;i<server->peercount;i++) {
        if (server->peers[i] && server->peers[i]->fd == fd) {
            send(fd, data, strlen(data), 0);
            return makenone();
        }
    }
    throwexception("peer not found");
    return makenone();
}

object *netbroadcast(object **args, int argc, void *env) {
    if (argc<1) throwexception("broadcast needs data");
    char *data = args[0]->sval;
    if (!server) throwexception("server not started");
    for (int i=0;i<server->peercount;i++) {
        if (server->peers[i]) {
            send(server->peers[i]->fd, data, strlen(data), 0);
        }
    }
    return makenone();
}

object *netclose(object **args, int argc, void *env) {
    if (!server) throwexception("server not created");
    server->running=0;
    pthread_cancel(server->thread);
    close(server->fd);
    for (int i=0;i<server->peercount;i++) if(server->peers[i]) close(server->peers[i]->fd);
    server->peercount=0;
    free(server->peers); server->peers=NULL;
    return makenone();
}

object *netonconnect(object **args, int argc, void *env) {
    if (argc<1 || !server) throwexception("onconnect needs handler");
    server->onconnect = args[0];
    return makenone();
}

object *netondisconnect(object **args, int argc, void *env) {
    if (argc<1 || !server) throwexception("ondisconnect needs handler");
    server->ondisconnect = args[0];
    return makenone();
}

object *netondata(object **args, int argc, void *env) {
    if (argc<1 || !server) throwexception("ondata needs handler");
    server->ondata = args[0];
    return makenone();
}

void registernetworklib(environment *env) {
    object *mod = makemodule("network", NULL);
    envset(mod->module.exports, "create", makebuiltin(netcreate), 0);
    envset(mod->module.exports, "listen", makebuiltin(netlisten), 0);
    envset(mod->module.exports, "start", makebuiltin(netstart), 0);
    envset(mod->module.exports, "send", makebuiltin(netsend), 0);
    envset(mod->module.exports, "broadcast", makebuiltin(netbroadcast), 0);
    envset(mod->module.exports, "close", makebuiltin(netclose), 0);
    envset(mod->module.exports, "onconnect", makebuiltin(netonconnect), 0);
    envset(mod->module.exports, "ondisconnect", makebuiltin(netondisconnect), 0);
    envset(mod->module.exports, "ondata", makebuiltin(netondata), 0);
    envset(env, "network", mod, 0);
}
