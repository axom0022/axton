#include "../core/axton.h"

object *oauthcreate(object **args, int argc, void *env) {
    void *oauth = platformoauthcreate();
    if (!oauth) throwexception("oauth create failed");
    return makeoauth(oauth);
}

object *oauthauthurl(object **args, int argc, void *env) {
    if (argc < 1) throwexception("authurl needs oauth");
    object *oauth = args[0];
    char *url = platformoauthauthurl(oauth->oauth.client);
    return makestring(url);
}

object *oauthgettoken(object **args, int argc, void *env) {
    if (argc < 2) throwexception("gettoken needs code");
    object *oauth = args[0];
    char *code = args[1]->sval;
    char *token = platformoauthgettoken(oauth->oauth.client, code);
    return makestring(token);
}

void registeroauthmodule(environment *env) {
    object *mod = makemodule("oauth", NULL);
    envset(mod->module.exports, "create", makebuiltin(oauthcreate), 0);
    envset(mod->module.exports, "authurl", makebuiltin(oauthauthurl), 0);
    envset(mod->module.exports, "gettoken", makebuiltin(oauthgettoken), 0);
    envset(env, "oauth", mod, 0);
}
