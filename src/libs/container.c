#include "../core/axton.h"

object *containerbuild(object **a, int c, void *e) {
    if (c < 2) throwexception("build needs name and path");
    return makenone();
}

object *containerrun(object **a, int c, void *e) {
    if (c < 1) throwexception("run needs image");
    return makenone();
}

void registercontainerlib(environment *env) {
    object *mod = makemodule("container", NULL);
    envset(mod->module.exports, "build", makebuiltin(containerbuild), 0);
    envset(mod->module.exports, "run", makebuiltin(containerrun), 0);
    envset(env, "container", mod, 0);
}
