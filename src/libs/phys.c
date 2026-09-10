#include "../core/axton.h"

object *physcreate(object **args, int argc, void *env) {
    void *world = platformphyscreate();
    if (!world) throwexception("physics create failed");
    return makephys(world);
}

object *physaddbody(object **args, int argc, void *env) {
    if (argc < 5) throwexception("addbody needs x y z mass");
    object *phys = args[0];
    float x = args[1]->fval;
    float y = args[2]->fval;
    float z = args[3]->fval;
    float m = args[4]->fval;
    platformphysaddbody(phys->phys.world, x, y, z, m);
    return makenone();
}

object *physstep(object **args, int argc, void *env) {
    if (argc < 2) throwexception("step needs dt");
    object *phys = args[0];
    float dt = args[1]->fval;
    platformphysstep(phys->phys.world, dt);
    return makenone();
}

object *physsetgravity(object **args, int argc, void *env) {
    if (argc < 4) throwexception("setgravity needs x y z");
    object *phys = args[0];
    float x = args[1]->fval;
    float y = args[2]->fval;
    float z = args[3]->fval;
    platformphyssetgravity(phys->phys.world, x, y, z);
    return makenone();
}

void registerphysmodule(environment *env) {
    object *mod = makemodule("phys", NULL);
    envset(mod->module.exports, "create", makebuiltin(physcreate), 0);
    envset(mod->module.exports, "addbody", makebuiltin(physaddbody), 0);
    envset(mod->module.exports, "step", makebuiltin(physstep), 0);
    envset(mod->module.exports, "setgravity", makebuiltin(physsetgravity), 0);
    envset(env, "phys", mod, 0);
}
