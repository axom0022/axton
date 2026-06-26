#include "../core/axton.h"
#include <math.h>

typedef struct buoyancy {
    float waterlevel;
    float density;
    float viscosity;
    float *positions;
    int count;
} buoyancy;

object *buoyancycreate(object **args, int argc, void *env) {
    buoyancy *b = malloc(sizeof(buoyancy));
    b->waterlevel = 0;
    b->density = 1000;
    b->viscosity = 0.001;
    b->positions = NULL;
    b->count = 0;
    return makenative(b, NULL);
}

object *buoyancysetlevel(object **args, int argc, void *env) {
    if (argc < 2) throwexception("setlevel needs buoyancy level");
    buoyancy *b = (buoyancy*)args[0]->native.data;
    b->waterlevel = args[1]->fval;
    return makenone();
}

object *buoyancyaddbody(object **args, int argc, void *env) {
    if (argc < 2) throwexception("addbody needs buoyancy body");
    buoyancy *b = (buoyancy*)args[0]->native.data;
    return makenone();
}

object *buoyancyupdate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("update needs buoyancy dt");
    buoyancy *b = (buoyancy*)args[0]->native.data;
    float dt = args[1]->fval;
    return makenone();
}

void registerbuoyancylib(environment *env) {
    object *mod = makemodule("buoyancy", NULL);
    envset(mod->module.exports, "create", makebuiltin(buoyancycreate), 0);
    envset(mod->module.exports, "setlevel", makebuiltin(buoyancysetlevel), 0);
    envset(mod->module.exports, "addbody", makebuiltin(buoyancyaddbody), 0);
    envset(mod->module.exports, "update", makebuiltin(buoyancyupdate), 0);
    envset(env, "buoyancy", mod, 0);
}
