#include "../core/axton.h"
#include <sys/socket.h>
#include <netdb.h>

object *wsclientcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("create needs url");
    char *url = args[0]->sval;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throwexception("socket failed");
    struct hostent *h = gethostbyname("localhost");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    memcpy(&addr.sin_addr, h->h_addr, h->h_length);
    connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    return makewsclient(fd, url);
}

object *wsclientconnect(object **args, int argc, void *env) {
    if (argc < 1) throwexception("connect needs wsclient");
    object *ws = args[0];
    char *key = "dGhlIHNhbXBsZSBub25jZQ==";
    char handshake[512];
    snprintf(handshake, sizeof(handshake),
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n", key);
    send(ws->wsclient.fd, handshake, strlen(handshake), 0);
    ws->wsclient.connected = 1;
    return makenone();
}

object *wsclientsend(object **args, int argc, void *env) {
    if (argc < 2) throwexception("send needs wsclient data");
    object *ws = args[0];
    char *data = args[1]->sval;
    if (!ws->wsclient.connected) throwexception("not connected");
    send(ws->wsclient.fd, data, strlen(data), 0);
    return makenone();
}

object *wsclientclose(object **args, int argc, void *env) {
    if (argc < 1) throwexception("close needs wsclient");
    object *ws = args[0];
    close(ws->wsclient.fd);
    ws->wsclient.connected = 0;
    return makenone();
}

void registerwsclientlib(environment *env) {
    object *mod = makemodule("wsclient", NULL);
    envset(mod->module.exports, "create", makebuiltin(wsclientcreate), 0);
    envset(mod->module.exports, "connect", makebuiltin(wsclientconnect), 0);
    envset(mod->module.exports, "send", makebuiltin(wsclientsend), 0);
    envset(mod->module.exports, "close", makebuiltin(wsclientclose), 0);
    envset(env, "wsclient", mod, 0);
}
