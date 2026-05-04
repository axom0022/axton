#include "../core/axton.h"

object *weakrefref(object **a, int c, void *e) {
    if(c<1) throwexception("ref needs object");
    return a[0];
}
void registerweakreflib(environment *env) {
    object *mod = makemodule("weakref", NULL);
    envset(mod->module.exports, "ref", makebuiltin(weakrefref), 0);
    envset(env, "weakref", mod, 0);
}
