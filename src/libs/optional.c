#include "../core/axton.h"

object *optionalcreate(object **a, int c, void *e) {
    if (c < 1) throwexception("optional needs type");
    return a[0];
}

void registeroptionallib(environment *env) {
    object *mod = makemodule("optional", NULL);
    envset(mod->module.exports, "create", makebuiltin(optionalcreate), 0);
    envset(env, "optional", mod, 0);
}
