#include "../core/axton.h"

object *ratelimitcreate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("create needs limit window");
    int limit = args[0]->ival;
    int window = args[1]->ival;
    void *rl = platformratelimitcreate(limit, window);
    if (!rl) throwexception("ratelimit create failed");
    return makeratelimitobj(rl);
}

object *ratelimitcheck(object **args, int argc, void *env) {
    if (argc < 2) throwexception("check needs key");
    object *rl = args[0];
    char *key = args[1]->sval;
    int res = platformratelimitcheck(rl->ratelimit.rate, key);
    return makebool(res);
}

void registerratelimitmodule(environment *env) {
    object *mod = makemodule("ratelimit", NULL);
    envset(mod->module.exports, "create", makebuiltin(ratelimitcreate), 0);
    envset(mod->module.exports, "check", makebuiltin(ratelimitcheck), 0);
    envset(env, "ratelimit", mod, 0);
}
