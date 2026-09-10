#include "../core/axton.h"

object *http3create(object **args, int argc, void *env) {
    void *stream = platformhttp3create();
    if (!stream) throwexception("http3 create failed");
    return makehttp3(stream);
}

object *http3listen(object **args, int argc, void *env) {
    if (argc < 2) throwexception("listen needs port");
    object *http3 = args[0];
    int port = args[1]->ival;
    int res = platformhttp3listen(http3->http3.stream, port);
    if (!res) throwexception("http3 listen failed");
    return makebool(1);
}

object *http3serve(object **args, int argc, void *env) {
    if (argc < 1) throwexception("serve needs http3");
    object *http3 = args[0];
    platformhttp3serve(http3->http3.stream);
    return makenone();
}

void registerhttp3module(environment *env) {
    object *mod = makemodule("http3", NULL);
    envset(mod->module.exports, "create", makebuiltin(http3create), 0);
    envset(mod->module.exports, "listen", makebuiltin(http3listen), 0);
    envset(mod->module.exports, "serve", makebuiltin(http3serve), 0);
    envset(env, "http3", mod, 0);
}
