#include "../core/axton.h"

object *poolcreate(object **args, int argc, void *env) {
    int size = argc > 0 ? args[0]->ival : 4;
    void *pool = platformpoolcreate(size);
    if (!pool) throwexception("thread pool create failed");
    return makethreadpool(pool);
}

object *poolsubmit(object **args, int argc, void *env) {
    if (argc < 2) throwexception("submit needs task");
    object *pool = args[0];
    object *task = args[1];
    platformpoolsubmit(pool->threadpool.pool, task);
    return makenone();
}

object *poolwait(object **args, int argc, void *env) {
    if (argc < 1) throwexception("wait needs pool");
    object *pool = args[0];
    platformpoolwait(pool->threadpool.pool);
    return makenone();
}

void registerpoolmodule(environment *env) {
    object *mod = makemodule("pool", NULL);
    envset(mod->module.exports, "create", makebuiltin(poolcreate), 0);
    envset(mod->module.exports, "submit", makebuiltin(poolsubmit), 0);
    envset(mod->module.exports, "wait", makebuiltin(poolwait), 0);
    envset(env, "pool", mod, 0);
}
