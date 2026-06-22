#include "../core/axton.h"

object *envget(object **a, int c, void *e) {
    if (c < 1) throwexception("get needs name");
    char *val = platformgetenv(a[0]->sval);
    return val ? makestring(val) : makenone();
}

object *envset(object **a, int c, void *e) {
    if (c < 2) throwexception("set needs name and value");
    return makebool(platformsetenv(a[0]->sval, a[1]->sval));
}

void registerenvlib(environment *env) {
    object *mod = makemodule("env", NULL);
    envset(mod->module.exports, "get", makebuiltin(envget), 0);
    envset(mod->module.exports, "set", makebuiltin(envset), 0);
    envset(env, "env", mod, 0);
}
