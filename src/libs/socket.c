#include "../core/axton.h"
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

static void setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

object *socketcreate(object **a, int c, void *e) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throwexception("socket failed");
    return makeint(fd);
}

object *socketconnect(object **a, int c, void *e) {
    if (c < 3 || a[0]->type != 0 || a[1]->type != 2 || a[2]->type != 0)
        throwexception("connect needs fd host port");
    int fd = a[0]->ival;
    char *host = a[1]->sval;
    int port = a[2]->ival;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    struct hostent *he = gethostbyname(host);
    if (!he) throwexception("host not found");
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throwexception("connect failed");
    return makeint(0);
}

object *socketsend(object **a, int c, void *e) {
    if (c < 2 || a[0]->type != 0 || a[1]->type != 2)
        throwexception("send needs fd and data");
    int fd = a[0]->ival;
    char *data = a[1]->sval;
    int sent = send(fd, data, strlen(data), 0);
    return makeint(sent);
}

object *socketrecv(object **a, int c, void *e) {
    if (c < 2 || a[0]->type != 0 || a[1]->type != 0)
        throwexception("recv needs fd and size");
    int fd = a[0]->ival;
    int size = a[1]->ival;
    char *buf = malloc(size + 1);
    int n = recv(fd, buf, size, 0);
    if (n <= 0) { free(buf); return makestring(""); }
    buf[n] = 0;
    object *res = makestring(buf);
    free(buf);
    return res;
}

object *socketclose(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 0) throwexception("close needs fd");
    close(a[0]->ival);
    return makenone();
}

object *socketbind(object **a, int c, void *e) {
    if (c < 2 || a[0]->type != 0 || a[1]->type != 0)
        throwexception("bind needs fd and port");
    int fd = a[0]->ival;
    int port = a[1]->ival;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throwexception("bind failed");
    return makenone();
}

object *socketlisten(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 0) throwexception("listen needs fd");
    if (listen(a[0]->ival, 10) < 0) throwexception("listen failed");
    return makenone();
}

object *socketaccept(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 0) throwexception("accept needs fd");
    int fd = a[0]->ival;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (struct sockaddr*)&client, &len);
    if (clientfd < 0) return makeint(-1);
    setnonblock(clientfd);
    return makeint(clientfd);
}

void registersocketlib(environment *env) {
    object *mod = makemodule("socket", NULL);
    envset(mod->module.exports, "socket", makebuiltin(socketcreate), 0);
    envset(mod->module.exports, "connect", makebuiltin(socketconnect), 0);
    envset(mod->module.exports, "send", makebuiltin(socketsend), 0);
    envset(mod->module.exports, "recv", makebuiltin(socketrecv), 0);
    envset(mod->module.exports, "close", makebuiltin(socketclose), 0);
    envset(mod->module.exports, "bind", makebuiltin(socketbind), 0);
    envset(mod->module.exports, "listen", makebuiltin(socketlisten), 0);
    envset(mod->module.exports, "accept", makebuiltin(socketaccept), 0);
    envset(env, "socket", mod, 0);
}
