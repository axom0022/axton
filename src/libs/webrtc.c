#include "../core/axton.h"

object *webrtccreate(object **a, int c, void *e) {
    return makenone();
}

object *webrtcconnect(object **a, int c, void *e) {
    if (c < 2) throwexception("connect needs webrtc and url");
    return makenone();
}

object *webrtcsend(object **a, int c, void *e) {
    if (c < 2) throwexception("send needs webrtc and data");
    return makenone();
}

void registerwebrtclib(environment *env) {
    object *mod = makemodule("webrtc", NULL);
    envset(mod->module.exports, "create", makebuiltin(webrtccreate), 0);
    envset(mod->module.exports, "connect", makebuiltin(webrtcconnect), 0);
    envset(mod->module.exports, "send", makebuiltin(webrtcsend), 0);
    envset(env, "webrtc", mod, 0);
}
