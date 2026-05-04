#include "../core/axton.h"
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

static int setnonblock(int fd) {
#ifdef _WIN32
    unsigned long mode = 1;
    return ioctlsocket(fd, FIONBIO, &mode);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}

object *socket_create(object **args, int argc, void *env) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throwexception("socket failed");
    return makeint(fd);
}

object *socket_connect(object **args, int argc, void *env) {
    if (argc < 3) throwexception("connect needs fd, host, port");
    int fd = args[0]->ival;
    char *host = args[1]->sval;
    int port = args[2]->ival;
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

object *socket_send(object **args, int argc, void *env) {
    if (argc < 2) throwexception("send needs fd and data");
    int fd = args[0]->ival;
    char *data = args[1]->sval;
    int sent = send(fd, data, strlen(data), 0);
    return makeint(sent);
}

object *socket_recv(object **args, int argc, void *env) {
    if (argc < 2) throwexception("recv needs fd and size");
    int fd = args[0]->ival;
    int size = args[1]->ival;
    char *buf = malloc(size + 1);
    int n = recv(fd, buf, size, 0);
    if (n <= 0) { free(buf); return makestring(""); }
    buf[n] = 0;
    object *res = makestring(buf);
    free(buf);
    return res;
}

object *socket_close(object **args, int argc, void *env) {
    if (argc < 1) throwexception("close needs fd");
#ifdef _WIN32
    closesocket(args[0]->ival);
#else
    close(args[0]->ival);
#endif
    return makenone();
}

object *socket_bind(object **args, int argc, void *env) {
    if (argc < 3) throwexception("bind needs fd, port");
    int fd = args[0]->ival;
    int port = args[1]->ival;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throwexception("bind failed");
    return makenone();
}

object *socket_listen(object **args, int argc, void *env) {
    if (argc < 1) throwexception("listen needs fd");
    int fd = args[0]->ival;
    if (listen(fd, 10) < 0) throwexception("listen failed");
    return makenone();
}

object *socket_accept(object **args, int argc, void *env) {
    if (argc < 1) throwexception("accept needs fd");
    int fd = args[0]->ival;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int clientfd = accept(fd, (struct sockaddr*)&client, &len);
    if (clientfd < 0) return makeint(-1);
    setnonblock(clientfd);
    return makeint(clientfd);
}

void registersocketlib(environment *env) {
    object *mod = makemodule("socket", NULL);
    envset(mod->module.exports, "socket", makebuiltin(socket_create), 0);
    envset(mod->module.exports, "connect", makebuiltin(socket_connect), 0);
    envset(mod->module.exports, "send", makebuiltin(socket_send), 0);
    envset(mod->module.exports, "recv", makebuiltin(socket_recv), 0);
    envset(mod->module.exports, "close", makebuiltin(socket_close), 0);
    envset(mod->module.exports, "bind", makebuiltin(socket_bind), 0);
    envset(mod->module.exports, "listen", makebuiltin(socket_listen), 0);
    envset(mod->module.exports, "accept", makebuiltin(socket_accept), 0);
    envset(env, "socket", mod, 0);
}
