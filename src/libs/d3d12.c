#include "../core/axton.h"

object *d3d12create(object **args, int argc, void *env) {
    void *d3d = platformd3d12create();
    if (!d3d) throwexception("d3d12 create failed");
    return maked3d12obj(d3d);
}

object *d3d12render(object **args, int argc, void *env) {
    if (argc < 1) throwexception("render needs d3d12");
    object *d3d = args[0];
    platformd3d12render(d3d->d3d12obj.d3d12);
    return makenone();
}

void registerd3d12module(environment *env) {
    object *mod = makemodule("d3d12", NULL);
    envset(mod->module.exports, "create", makebuiltin(d3d12create), 0);
    envset(mod->module.exports, "render", makebuiltin(d3d12render), 0);
    envset(env, "d3d12", mod, 0);
}
