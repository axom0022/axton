#include "../core/axton.h"

object *ssecreate(object **args, int argc, void *env) {
    void *sse = platformssecreate();
    if (!sse) throwexception("sse create failed");
    return makesse(sse);
}

object *sseadd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs id");
    object *sse = args[0];
    char *id = args[1]->sval;
    platformsseadd(sse->sse.sse, id);
    return makenone();
}

object *ssesend(object **args, int argc, void *env) {
    if (argc < 2) throwexception("send needs data");
    object *sse = args[0];
    char *data = args[1]->sval;
    platformssesend(sse->sse.sse, data);
    return makenone();
}

void registerssemodule(environment *env) {
    object *mod = makemodule("sse", NULL);
    envset(mod->module.exports, "create", makebuiltin(ssecreate), 0);
    envset(mod->module.exports, "add", makebuiltin(sseadd), 0);
    envset(mod->module.exports, "send", makebuiltin(ssesend), 0);
    envset(env, "sse", mod, 0);
}
