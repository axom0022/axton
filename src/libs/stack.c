#include "../core/axton.h"

object *stackcreate(object **args, int argc, void *env) {
    void *st = platformstacktracecreate();
    if (!st) throwexception("stacktrace create failed");
    return makestacktrace(st);
}

object *stackadd(object **args, int argc, void *env) {
    if (argc < 3) throwexception("add needs file line");
    object *st = args[0];
    char *file = args[1]->sval;
    int line = args[2]->ival;
    platformstacktraceadd(st->stacktrace.stack, file, line);
    return makenone();
}

object *stackget(object **args, int argc, void *env) {
    if (argc < 1) throwexception("get needs stack");
    object *st = args[0];
    void *result = platformstacktraceget(st->stacktrace.stack);
    return makenative(result, NULL);
}

void registerstackmodule(environment *env) {
    object *mod = makemodule("stack", NULL);
    envset(mod->module.exports, "create", makebuiltin(stackcreate), 0);
    envset(mod->module.exports, "add", makebuiltin(stackadd), 0);
    envset(mod->module.exports, "get", makebuiltin(stackget), 0);
    envset(env, "stack", mod, 0);
}
