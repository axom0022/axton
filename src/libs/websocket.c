#include "../core/axton.h"
#include <sys/socket.h>
#include <unistd.h>

object *wscreate(object **a, int c, void *e) {
    if (c < 1) throwexception("wscreate needs port");
    return makeint(0);
}

object *wsonmessage(object **a, int c, void *e) {
    if (c < 2) throwexception("onmessage needs socket and handler");
    return makenone();
}

object *wssend(object **a, int c, void *e) {
    if (c < 2) throwexception("send needs socket and data");
    return makenone();
}

object *wsbroadcast(object **a, int c, void *e) {
    if (c < 1) throwexception("broadcast needs data");
    return makenone();
}

void registerwebsocketlib(environment *env) {
    object *mod = makemodule("websocket", NULL);
    envset(mod->module.exports, "create", makebuiltin(wscreate), 0);
    envset(mod->module.exports, "onmessage", makebuiltin(wsonmessage), 0);
    envset(mod->module.exports, "send", makebuiltin(wssend), 0);
    envset(mod->module.exports, "broadcast", makebuiltin(wsbroadcast), 0);
    envset(env, "websocket", mod, 0);
}
