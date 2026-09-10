#include "../core/axton.h"

object *raycreate(object **args, int argc, void *env) {
    void *rt = platformraycreate();
    if (!rt) throwexception("ray create failed");
    return makeray(rt);
}

object *rayrender(object **args, int argc, void *env) {
    if (argc < 3) throwexception("render needs w h");
    object *ray = args[0];
    int w = args[1]->ival;
    int h = args[2]->ival;
    platformrayrender(ray->ray.rt, w, h);
    return makenone();
}

object *raysetlight(object **args, int argc, void *env) {
    if (argc < 5) throwexception("setlight needs x y z intensity");
    object *ray = args[0];
    float x = args[1]->fval;
    float y = args[2]->fval;
    float z = args[3]->fval;
    float i = args[4]->fval;
    platformraysetlight(ray->ray.rt, x, y, z, i);
    return makenone();
}

object *raysetmaterial(object **args, int argc, void *env) {
    if (argc < 5) throwexception("setmaterial needs r g b shininess");
    object *ray = args[0];
    float r = args[1]->fval;
    float g = args[2]->fval;
    float b = args[3]->fval;
    float s = args[4]->fval;
    platformraysetmaterial(ray->ray.rt, r, g, b, s);
    return makenone();
}

void registerraymodule(environment *env) {
    object *mod = makemodule("ray", NULL);
    envset(mod->module.exports, "create", makebuiltin(raycreate), 0);
    envset(mod->module.exports, "render", makebuiltin(rayrender), 0);
    envset(mod->module.exports, "setlight", makebuiltin(raysetlight), 0);
    envset(mod->module.exports, "setmaterial", makebuiltin(raysetmaterial), 0);
    envset(env, "ray", mod, 0);
}
