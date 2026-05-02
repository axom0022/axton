#include "../core/axton.h"

typedef struct asynctask {
    object *coro;
    struct asynctask *next;
} asynctask;

static asynctask *taskqueue = NULL;
static int asynclooprunning = 0;

object *asynccreate(object *func) {
    object *coro = gcalloc(sizeof(object));
    coro->type = 22;
    coro->coroutine.func = func;
    coro->coroutine.state = 0;
    return coro;
}

void asyncrunloop(void) {
    asynclooprunning = 1;
    while (asynclooprunning && taskqueue) {
        asynctask *task = taskqueue;
        taskqueue = task->next;
        if (task->coro->coroutine.state == 0) {
            callfunc(task->coro->coroutine.func, NULL, 0, globalenv);
            task->coro->coroutine.state = 1;
        }
        free(task);
    }
    asynclooprunning = 0;
}

object *asyncawait(object *coro) {
    if (coro->type != 22) throwexception("not a coroutine");
    callfunc(coro->coroutine.func, NULL, 0, globalenv);
    return makenone();
}

object *builtinasync(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("async needs function");
    return asynccreate(args[0]);
}

object *builtinawait(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("await needs coroutine");
    return asyncawait(args[0]);
}

object *builtinrunloop(object **args, int argc, environment *env) {
    asyncrunloop();
    return makenone();
}

void registerasynclib(environment *env) {
    object *asyncmod = makemodule("async", NULL);
    envset(asyncmod->module.exports, "create", makebuiltin(builtinasync), 0);
    envset(asyncmod->module.exports, "await", makebuiltin(builtinawait), 0);
    envset(asyncmod->module.exports, "run", makebuiltin(builtinrunloop), 0);
    envset(env, "async", asyncmod, 0);
}
