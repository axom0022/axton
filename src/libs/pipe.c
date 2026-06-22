#include "../core/axton.h"

object *pipeapply(object **a, int c, void *e) {
    if (c < 2) throwexception("pipe needs value and functions");
    object *val = a[0];
    for (int i = 1; i < c; i++) {
        if (a[i]->type != 7) throwexception("pipe expects functions");
        object *args[1] = {val};
        val = callfunc(a[i], args, 1, env);
    }
    return val;
}

void registerpipetransformlib(environment *env) {
    object *mod = makemodule("pipe", NULL);
    envset(mod->module.exports, "apply", makebuiltin(pipeapply), 0);
    envset(env, "pipe", mod, 0);
}
