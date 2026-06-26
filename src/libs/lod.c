#include "../core/axton.h"
#include <math.h>

typedef struct lodlevel {
    float distance;
    void *mesh;
    void *material;
} lodlevel;

typedef struct lod {
    lodlevel *levels;
    int count;
    int currentlevel;
    float pos[3];
} lod;

object *lodcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("lodcreate needs count");
    int count = args[0]->ival;
    lod *l = malloc(sizeof(lod));
    l->levels = calloc(count, sizeof(lodlevel));
    l->count = count;
    l->currentlevel = 0;
    l->pos[0]=0; l->pos[1]=0; l->pos[2]=0;
    return makenative(l, NULL);
}

object *lodsetlevel(object **args, int argc, void *env) {
    if (argc < 3) throwexception("setlevel needs lod index distance mesh");
    lod *l = (lod*)args[0]->native.data;
    int idx = args[1]->ival;
    if (idx < 0 || idx >= l->count) throwexception("invalid index");
    l->levels[idx].distance = args[2]->fval;
    l->levels[idx].mesh = (void*)(long)(args[3]->ival);
    return makenone();
}

object *lodupdate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("update needs lod and camera");
    lod *l = (lod*)args[0]->native.data;
    float camx = args[1]->fval, camy = args[2]->fval, camz = args[3]->fval;
    float dist = sqrt(pow(l->pos[0]-camx,2)+pow(l->pos[1]-camy,2)+pow(l->pos[2]-camz,2));
    int newlevel = 0;
    for (int i = 0; i < l->count; i++) {
        if (dist >= l->levels[i].distance) newlevel = i;
    }
    l->currentlevel = newlevel;
    return makeint(newlevel);
}

object *lodgetmesh(object **args, int argc, void *env) {
    if (argc < 1) throwexception("getmesh needs lod");
    lod *l = (lod*)args[0]->native.data;
    return makeint((long)l->levels[l->currentlevel].mesh);
}

void registerlodlib(environment *env) {
    object *mod = makemodule("lod", NULL);
    envset(mod->module.exports, "create", makebuiltin(lodcreate), 0);
    envset(mod->module.exports, "setlevel", makebuiltin(lodsetlevel), 0);
    envset(mod->module.exports, "update", makebuiltin(lodupdate), 0);
    envset(mod->module.exports, "getmesh", makebuiltin(lodgetmesh), 0);
    envset(env, "lod", mod, 0);
}
