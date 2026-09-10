#include "../core/axton.h"

object *semcreate(object **args, int argc, void *env) {
    int count = argc > 0 ? args[0]->ival : 1;
    void *sem = platformsemcreate(count);
    if (!sem) throwexception("semaphore create failed");
    return makesemaphore(sem);
}

object *semwait(object **args, int argc, void *env) {
    if (argc < 1) throwexception("wait needs semaphore");
    object *sem = args[0];
    platformsemwait(sem->semaphore.sem);
    return makenone();
}

object *sempost(object **args, int argc, void *env) {
    if (argc < 1) throwexception("post needs semaphore");
    object *sem = args[0];
    platformsempost(sem->semaphore.sem);
    return makenone();
}

void registersemmodule(environment *env) {
    object *mod = makemodule("sem", NULL);
    envset(mod->module.exports, "create", makebuiltin(semcreate), 0);
    envset(mod->module.exports, "wait", makebuiltin(semwait), 0);
    envset(mod->module.exports, "post", makebuiltin(sempost), 0);
    envset(env, "sem", mod, 0);
}
