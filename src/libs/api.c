#include "../core/axton.h"

object *apicreate(object **args, int argc, void *env) {
    void *api = platformapicreate();
    if (!api) throwexception("api create failed");
    return makeapiframework(api);
}

object *apiaddroute(object **args, int argc, void *env) {
    if (argc < 4) throwexception("addroute needs path method fn");
    object *api = args[0];
    char *path = args[1]->sval;
    char *method = args[2]->sval;
    object *fn = args[3];
    platformapiaddroute(api->apiframework.api, path, method, fn);
    return makenone();
}

object *apistart(object **args, int argc, void *env) {
    if (argc < 2) throwexception("start needs port");
    object *api = args[0];
    int port = args[1]->ival;
    platformapistart(api->apiframework.api, port);
    return makenone();
}

void registerapimodule(environment *env) {
    object *mod = makemodule("api", NULL);
    envset(mod->module.exports, "create", makebuiltin(apicreate), 0);
    envset(mod->module.exports, "addroute", makebuiltin(apiaddroute), 0);
    envset(mod->module.exports, "start", makebuiltin(apistart), 0);
    envset(env, "api", mod, 0);
}
