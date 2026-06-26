#include "../core/axton.h"
#include <GL/gl.h>

typedef struct antialias {
    int msaa;
    int fxaa;
    int taa;
    float taastrength;
} antialias;

static antialias *curaa = NULL;

object *antialiascreate(object **args, int argc, void *env) {
    antialias *a = malloc(sizeof(antialias));
    a->msaa = 4;
    a->fxaa = 1;
    a->taa = 0;
    a->taastrength = 0.1;
    curaa = a;
    return makenative(a, NULL);
}

object *antialiassetmsaa(object **args, int argc, void *env) {
    if (argc < 1 || !curaa) throwexception("setmsaa needs samples");
    curaa->msaa = args[0]->ival;
    return makenone();
}

object *antialiassetfxaa(object **args, int argc, void *env) {
    if (argc < 1 || !curaa) throwexception("setfxaa needs on");
    curaa->fxaa = args[0]->bval;
    return makenone();
}

object *antialiassettaa(object **args, int argc, void *env) {
    if (argc < 2 || !curaa) throwexception("settaa needs on strength");
    curaa->taa = args[0]->bval;
    curaa->taastrength = args[1]->fval;
    return makenone();
}

void registerantialiaslib(environment *env) {
    object *mod = makemodule("antialias", NULL);
    envset(mod->module.exports, "create", makebuiltin(antialiascreate), 0);
    envset(mod->module.exports, "setmsaa", makebuiltin(antialiassetmsaa), 0);
    envset(mod->module.exports, "setfxaa", makebuiltin(antialiassetfxaa), 0);
    envset(mod->module.exports, "settaa", makebuiltin(antialiassettaa), 0);
    envset(env, "antialias", mod, 0);
}
