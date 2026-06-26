#include "../core/axton.h"
#include <math.h>

typedef struct terrainphys {
    float *heightmap;
    int width;
    int depth;
    float scale;
} terrainphys;

object *terrainphyscreate(object **args, int argc, void *env) {
    if (argc < 3) throwexception("terrainphys needs width depth scale");
    int w=args[0]->ival, d=args[1]->ival, s=args[2]->fval;
    terrainphys *t = malloc(sizeof(terrainphys));
    t->width = w;
    t->depth = d;
    t->scale = s;
    t->heightmap = calloc(w * d, sizeof(float));
    return makenative(t, NULL);
}

object *terrainphyssetheight(object **args, int argc, void *env) {
    if (argc < 4) throwexception("setheight needs terrain x z height");
    terrainphys *t = (terrainphys*)args[0]->native.data;
    int x=args[1]->ival, z=args[2]->ival;
    if (x < 0 || x >= t->width || z < 0 || z >= t->depth) throwexception("out of bounds");
    t->heightmap[x + z * t->width] = args[3]->fval;
    return makenone();
}

object *terrainphysgetheight(object **args, int argc, void *env) {
    if (argc < 3) throwexception("getheight needs terrain x z");
    terrainphys *t = (terrainphys*)args[0]->native.data;
    int x=args[1]->ival, z=args[2]->ival;
    if (x < 0 || x >= t->width || z < 0 || z >= t->depth) return makefloat(0);
    return makefloat(t->heightmap[x + z * t->width]);
}

object *terrainphyssample(object **args, int argc, void *env) {
    if (argc < 3) throwexception("sample needs terrain x z");
    terrainphys *t = (terrainphys*)args[0]->native.data;
    float x=args[1]->fval, z=args[2]->fval;
    return makefloat(0);
}

void registerterrainphyslib(environment *env) {
    object *mod = makemodule("terrainphys", NULL);
    envset(mod->module.exports, "create", makebuiltin(terrainphyscreate), 0);
    envset(mod->module.exports, "setheight", makebuiltin(terrainphyssetheight), 0);
    envset(mod->module.exports, "getheight", makebuiltin(terrainphysgetheight), 0);
    envset(mod->module.exports, "sample", makebuiltin(terrainphyssample), 0);
    envset(env, "terrainphys", mod, 0);
}
