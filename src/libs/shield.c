#include "../core/axton.h"

object *shieldcreate(object **args, int argc, void *env) {
    void *cs = platformshieldcreate();
    if (!cs) throwexception("csrf create failed");
    return makeshieldobj(cs);
}

object *shieldtoken(object **args, int argc, void *env) {
    if (argc < 1) throwexception("token needs shield");
    object *cs = args[0];
    char *token = platformshieldtoken(cs->shieldobj.shield);
    return makestring(token);
}

object *shieldvalidate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("validate needs token");
    object *cs = args[0];
    char *token = args[1]->sval;
    int res = platformshieldvalidate(cs->shieldobj.shield, token);
    return makebool(res);
}

void registershieldmodule(environment *env) {
    object *mod = makemodule("shield", NULL);
    envset(mod->module.exports, "create", makebuiltin(shieldcreate), 0);
    envset(mod->module.exports, "token", makebuiltin(shieldtoken), 0);
    envset(mod->module.exports, "validate", makebuiltin(shieldvalidate), 0);
    envset(env, "shield", mod, 0);
}
