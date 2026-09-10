#include "../core/axton.h"

object *sessioncreate(object **args, int argc, void *env) {
    void *session = platformsessioncreate();
    if (!session) throwexception("session create failed");
    return makesessionobj(session);
}

object *sessionset(object **args, int argc, void *env) {
    if (argc < 3) throwexception("set needs key val");
    object *session = args[0];
    char *key = args[1]->sval;
    object *val = args[2];
    platformsessionset(session->sessionobj.session, key, val);
    return makenone();
}

object *sessionget(object **args, int argc, void *env) {
    if (argc < 2) throwexception("get needs key");
    object *session = args[0];
    char *key = args[1]->sval;
    void *val = platformsessionget(session->sessionobj.session, key);
    return (object*)val;
}

void registersessionmodule(environment *env) {
    object *mod = makemodule("session", NULL);
    envset(mod->module.exports, "create", makebuiltin(sessioncreate), 0);
    envset(mod->module.exports, "set", makebuiltin(sessionset), 0);
    envset(mod->module.exports, "get", makebuiltin(sessionget), 0);
    envset(env, "session", mod, 0);
}
