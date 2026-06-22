#include "../core/axton.h"

object *webgpuinit(object **a, int c, void *e) {
    return makenone();
}

object *webgpurender(object **a, int c, void *e) {
    return makenone();
}

void registerwebgpulib(environment *env) {
    object *mod = makemodule("webgpu", NULL);
    envset(mod->module.exports, "init", makebuiltin(webgpuinit), 0);
    envset(mod->module.exports, "render", makebuiltin(webgpurender), 0);
    envset(env, "webgpu", mod, 0);
}
