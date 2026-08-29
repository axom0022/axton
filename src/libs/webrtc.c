#include "../core/axton.h"
#include <webrtc.h>

object *webrtccreate(object **args, int argc, void *env) {
    void *pc = platform.webrtccreate();
    if (!pc) throwexception("webrtc create failed");
    void *dc = platform.webrtcdatachannel(pc);
    return makewebrtc(pc, dc);
}

object *webrtcconnect(object **args, int argc, void *env) {
    if (argc < 2) throwexception("connect needs webrtc url");
    object *webrtc = args[0];
    char *url = args[1]->sval;
    int res = platform.webrtcconnect(webrtc->webrtc.peerconnection, url);
    if (res) return makebool(1);
    return makebool(0);
}

object *webrtcsend(object **args, int argc, void *env) {
    if (argc < 2) throwexception("send needs webrtc data");
    object *webrtc = args[0];
    char *data = args[1]->sval;
    platform.webrtcsend(webrtc->webrtc.datachannel, data, strlen(data));
    return makenone();
}

object *webrtconmessage(object **args, int argc, void *env) {
    if (argc < 2) throwexception("onmessage needs handler");
    object *webrtc = args[0];
    object *fn = args[1];
    platform.webrtconmessage(webrtc->webrtc.peerconnection, fn);
    return makenone();
}

void registerwebrtclib(environment *env) {
    object *mod = makemodule("webrtc", NULL);
    envset(mod->module.exports, "create", makebuiltin(webrtccreate), 0);
    envset(mod->module.exports, "connect", makebuiltin(webrtcconnect), 0);
    envset(mod->module.exports, "send", makebuiltin(webrtcsend), 0);
    envset(mod->module.exports, "onmessage", makebuiltin(webrtconmessage), 0);
    envset(env, "webrtc", mod, 0);
}
