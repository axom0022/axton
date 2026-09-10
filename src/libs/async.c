#include "../core/axton.h"

object *asynccreate(object **args, int argc, void *env) {
    void *loop = platformasynccreate();
    if (!loop) throwexception("async create failed");
    return makeasync(loop);
}

object *asyncadd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs task");
    object *async = args[0];
    object *task = args[1];
    platformasyncadd(async->async.loop, task);
    return makenone();
}

object *asyncrun(object **args, int argc, void *env) {
    if (argc < 1) throwexception("run needs async");
    object *async = args[0];
    int res = platformasyncrun(async->async.loop);
    return makebool(res);
}

void registerasyncmodule(environment *env) {
    object *mod = makemodule("async", NULL);
    envset(mod->module.exports, "create", makebuiltin(asynccreate), 0);
    envset(mod->module.exports, "add", makebuiltin(asyncadd), 0);
    envset(mod->module.exports, "run", makebuiltin(asyncrun), 0);
    envset(env, "async", mod, 0);
}
