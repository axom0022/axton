#include "../core/axton.h"

object *unioncreate(object **a, int c, void *e) {
    if (c < 2) throwexception("union needs types");
    object *types = makelist();
    for (int i = 0; i < c; i++) listappend(types, a[i]);
    return types;
}

void registerunionlib(environment *env) {
    object *mod = makemodule("union", NULL);
    envset(mod->module.exports, "create", makebuiltin(unioncreate), 0);
    envset(env, "union", mod, 0);
}
