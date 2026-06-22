#include "../core/axton.h"

object *contextwith(object **a, int c, void *e) {
    if (c < 2) throwexception("with needs manager and block");
    object *mgr = a[0];
    object *enter = envget(mgr->klass.attrs, "__enter__");
    if (!enter) throwexception("no __enter__");
    object *val = callfunc(enter, NULL, 0, env);
    object *block = a[1];
    return makenone();
}

void registercontextlib(environment *env) {
    object *mod = makemodule("context", NULL);
    envset(mod->module.exports, "with", makebuiltin(contextwith), 0);
    envset(env, "context", mod, 0);
}
