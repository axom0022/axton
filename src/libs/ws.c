#include "../core/axton.h"

object *wscreate(object **args, int argc, void *env) {
    void *ws = platformwscreate();
    if (!ws) throwexception("ws create failed");
    return makewsobj(ws);
}

object *wsadd(object **args, int argc, void *env) {
    if (argc < 3) throwexception("add needs id fn");
    object *ws = args[0];
    int id = args[1]->ival;
    object *fn = args[2];
    platformwsadd(ws->wsobj.ws, id, fn);
    return makenone();
}

object *wssend(object **args, int argc, void *env) {
    if (argc < 3) throwexception("send needs id data");
    object *ws = args[0];
    int id = args[1]->ival;
    char *data = args[2]->sval;
    platformwssend(ws->wsobj.ws, id, data);
    return makenone();
}

void registerwsmodule(environment *env) {
    object *mod = makemodule("ws", NULL);
    envset(mod->module.exports, "create", makebuiltin(wscreate), 0);
    envset(mod->module.exports, "add", makebuiltin(wsadd), 0);
    envset(mod->module.exports, "send", makebuiltin(wssend), 0);
    envset(env, "ws", mod, 0);
}
