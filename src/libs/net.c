#include "../core/axton.h"

object *netcreate(object **args, int argc, void *env) {
    void *net = platformnetcreate();
    if (!net) throwexception("net create failed");
    return makenetobj(net);
}

object *netlisten(object **args, int argc, void *env) {
    if (argc < 2) throwexception("listen needs port");
    object *net = args[0];
    int port = args[1]->ival;
    platformnetlisten(net->netobj.net, port);
    return makenone();
}

object *netsend(object **args, int argc, void *env) {
    if (argc < 3) throwexception("send needs data len");
    object *net = args[0];
    char *data = args[1]->sval;
    int len = args[2]->ival;
    platformnetsend(net->netobj.net, data, len);
    return makenone();
}

object *netrecv(object **args, int argc, void *env) {
    if (argc < 1) throwexception("recv needs net");
    object *net = args[0];
    void *data = platformnetrecv(net->netobj.net);
    return makenative(data, NULL);
}

void registernetmodule(environment *env) {
    object *mod = makemodule("net", NULL);
    envset(mod->module.exports, "create", makebuiltin(netcreate), 0);
    envset(mod->module.exports, "listen", makebuiltin(netlisten), 0);
    envset(mod->module.exports, "send", makebuiltin(netsend), 0);
    envset(mod->module.exports, "recv", makebuiltin(netrecv), 0);
    envset(env, "net", mod, 0);
}
