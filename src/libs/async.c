#include "../core/axton.h"

object *makecoroutine(object *func) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 21;
    obj->coroutine.func = func;
    obj->coroutine.state = 0;
    return obj;
}

object *asynccreate(object *func) {
    return makecoroutine(func);
}

object *asyncawait(object *coro) {
    if (coro->type != 21) throwexception("not a coroutine");
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
    return makenone();
}

void registerasynclib(environment *env) {
    object *asyncmod = makemodule("async", NULL);
    envset(asyncmod->module.exports, "create", makebuiltin(builtinasync), 0);
    envset(asyncmod->module.exports, "await", makebuiltin(builtinawait), 0);
    envset(asyncmod->module.exports, "run", makebuiltin(builtinrunloop), 0);
    envset(env, "async", asyncmod, 0);
}
