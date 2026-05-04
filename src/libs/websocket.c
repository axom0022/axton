#include "websocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <arpa/inet.h>

static websocketserver wsserver;
static pthread_mutex_t clientlock = PTHREAD_MUTEX_INITIALIZER;

static char *compute_accept(char *key) {
    char accept[128];
    char *guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    snprintf(accept, sizeof(accept), "%s%s", key, guid);
    unsigned char hash[20];
    SHA1((unsigned char*)accept, strlen(accept), hash);
    char *result = malloc(32);
    base64_encode(hash, 20, result);
    return result;
}

static void send_frame(int fd, char *data, int len, int opcode) {
    unsigned char frame[10];
    int pos = 0;
    frame[pos++] = 0x80 | (opcode & 0x0f);
    if (len < 126) {
        frame[pos++] = len;
    } else if (len < 65536) {
        frame[pos++] = 126;
        frame[pos++] = (len >> 8) & 0xff;
        frame[pos++] = len & 0xff;
    } else {
        frame[pos++] = 127;
        for (int i = 7; i >= 0; i--) frame[pos++] = (len >> (i * 8)) & 0xff;
    }
    send(fd, frame, pos, 0);
    send(fd, data, len, 0);
}

static char *recv_frame(int fd, int *opcode) {
    unsigned char header[2];
    if (recv(fd, header, 2, 0) != 2) return NULL;
    int fin = (header[0] >> 7) & 1;
    *opcode = header[0] & 0x0f;
    int masked = (header[1] >> 7) & 1;
    int len = header[1] & 0x7f;
    if (len == 126) {
        unsigned char ext[2];
        recv(fd, ext, 2, 0);
        len = (ext[0] << 8) | ext[1];
    } else if (len == 127) {
        unsigned char ext[8];
        recv(fd, ext, 8, 0);
        len = 0;
        for (int i = 0; i < 8; i++) len = (len << 8) | ext[i];
    }
    unsigned char mask[4] = {0};
    if (masked) recv(fd, mask, 4, 0);
    unsigned char *data = malloc(len + 1);
    recv(fd, data, len, 0);
    if (masked) {
        for (int i = 0; i < len; i++) data[i] ^= mask[i % 4];
    }
    data[len] = 0;
    return (char*)data;
}

static void *wsclientthread(void *arg) {
    int fd = *(int*)arg;
    free(arg);
    char buf[4096];
    recv(fd, buf, sizeof(buf) - 1, 0);
    char key[128] = {0};
    char path[256] = {0};
    char *line = strtok(buf, "\r\n");
    sscanf(line, "GET %s", path);
    while (line) {
        if (strncmp(line, "Sec-WebSocket-Key:", 18) == 0) {
            strcpy(key, line + 19);
        }
        line = strtok(NULL, "\r\n");
    }
    char *accept = compute_accept(key);
    char response[512];
    snprintf(response, sizeof(response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n", accept);
    send(fd, response, strlen(response), 0);
    free(accept);
    websocketclient *client = malloc(sizeof(websocketclient));
    client->fd = fd;
    client->path = strdup(path);
    client->protocol = NULL;
    client->onmessage = NULL;
    client->onclose = NULL;
    client->onerror = NULL;
    pthread_mutex_lock(&clientlock);
    client->next = wsserver.clients;
    wsserver.clients = client;
    pthread_mutex_unlock(&clientlock);
    if (wsserver.onconnection) {
        object *args[1] = {makenative(client, NULL)};
        callfunc(wsserver.onconnection, args, 1, globalenv);
    }
    while (1) {
        int opcode = 0;
        char *data = recv_frame(fd, &opcode);
        if (!data) break;
        if (opcode == 0x01 && client->onmessage) {
            object *args[1] = {makestring(data)};
            callfunc(client->onmessage, args, 1, globalenv);
        }
        if (opcode == 0x08) break;
        free(data);
    }
    if (client->onclose) callfunc(client->onclose, NULL, 0, globalenv);
    close(fd);
    pthread_mutex_lock(&clientlock);
    websocketclient *prev = NULL;
    websocketclient *cur = wsserver.clients;
    while (cur && cur != client) {
        prev = cur;
        cur = cur->next;
    }
    if (cur) {
        if (prev) prev->next = cur->next;
        else wsserver.clients = cur->next;
    }
    pthread_mutex_unlock(&clientlock);
    free(client->path);
    free(client);
    return NULL;
}

static void *wsserverthread(void *arg) {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(wsserver.port);
    bind(serverfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(serverfd, 100);
    while (wsserver.running) {
        int clientfd = accept(serverfd, NULL, NULL);
        if (clientfd < 0) continue;
        int *fdptr = malloc(sizeof(int));
        *fdptr = clientfd;
        pthread_t thread;
        pthread_create(&thread, NULL, wsclientthread, fdptr);
        pthread_detach(thread);
    }
    close(serverfd);
    return NULL;
}

object *wscreateserver(int port) {
    wsserver.port = port;
    wsserver.running = 0;
    wsserver.onconnection = NULL;
    wsserver.clients = NULL;
    return makestring("websocket server created");
}

object *wsonconnection(object *handler) {
    wsserver.onconnection = handler;
    return makenone();
}

object *wssend(void *client, char *data) {
    websocketclient *c = (websocketclient*)client;
    send_frame(c->fd, data, strlen(data), 0x01);
    return makenone();
}

object *wsbroadcast(char *data) {
    pthread_mutex_lock(&clientlock);
    websocketclient *c = wsserver.clients;
    while (c) {
        send_frame(c->fd, data, strlen(data), 0x01);
        c = c->next;
    }
    pthread_mutex_unlock(&clientlock);
    return makenone();
}

object *wsclose(void *client) {
    websocketclient *c = (websocketclient*)client;
    send_frame(c->fd, NULL, 0, 0x08);
    return makenone();
}

object *wsstart(void) {
    if (wsserver.running) return makestring("already running");
    wsserver.running = 1;
    pthread_create(&wsserver.thread, NULL, wsserverthread, NULL);
    pthread_detach(wsserver.thread);
    return makestring("websocket server started");
}

void registerwebsocketlib(environment *env) {
    object *wsmod = makemodule("websocket", NULL);
    envset(wsmod->module.exports, "createserver", makebuiltin(wscreateserver), 0);
    envset(wsmod->module.exports, "onconnection", makebuiltin(wsonconnection), 0);
    envset(wsmod->module.exports, "send", makebuiltin(wssend), 0);
    envset(wsmod->module.exports, "broadcast", makebuiltin(wsbroadcast), 0);
    envset(wsmod->module.exports, "close", makebuiltin(wsclose), 0);
    envset(wsmod->module.exports, "start", makebuiltin(wsstart), 0);
    envset(env, "websocket", wsmod, 0);
}
