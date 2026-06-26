#include "../core/axton.h"
#include <GL/gl.h>

typedef struct volfog {
    int enabled;
    float density;
    float heightfalloff;
    float color[3];
    float anisotropy;
} volfog;

static volfog *curfog = NULL;

object *volfogcreate(object **args, int argc, void *env) {
    volfog *f = malloc(sizeof(volfog));
    f->enabled = 1;
    f->density = 0.01;
    f->heightfalloff = 0.1;
    f->color[0] = 0.5; f->color[1] = 0.5; f->color[2] = 0.6;
    f->anisotropy = 0.5;
    curfog = f;
    return makenative(f, NULL);
}

object *volfogsetdensity(object **args, int argc, void *env) {
    if (argc < 1 || !curfog) throwexception("setdensity needs val");
    curfog->density = args[0]->fval;
    return makenone();
}

object *volfogsetcolor(object **args, int argc, void *env) {
    if (argc < 3 || !curfog) throwexception("setcolor needs r g b");
    curfog->color[0] = args[0]->fval;
    curfog->color[1] = args[1]->fval;
    curfog->color[2] = args[2]->fval;
    return makenone();
}

object *volfogsetfalloff(object **args, int argc, void *env) {
    if (argc < 1 || !curfog) throwexception("setfalloff needs val");
    curfog->heightfalloff = args[0]->fval;
    return makenone();
}

object *volfogenable(object **args, int argc, void *env) {
    if (argc < 1 || !curfog) throwexception("enable needs on");
    curfog->enabled = args[0]->bval;
    return makenone();
}

void registervolfoglib(environment *env) {
    object *mod = makemodule("volfog", NULL);
    envset(mod->module.exports, "create", makebuiltin(volfogcreate), 0);
    envset(mod->module.exports, "setdensity", makebuiltin(volfogsetdensity), 0);
    envset(mod->module.exports, "setcolor", makebuiltin(volfogsetcolor), 0);
    envset(mod->module.exports, "setfalloff", makebuiltin(volfogsetfalloff), 0);
    envset(mod->module.exports, "enable", makebuiltin(volfogenable), 0);
    envset(env, "volfog", mod, 0);
}
