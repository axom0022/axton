#include "../core/axton.h"
#include <string.h>

typedef struct animbone {
    char name[64];
    int parent;
    float pos[3];
    float rot[3];
    float scale[3];
} animbone;

typedef struct animkeyframe {
    float time;
    float pos[3];
    float rot[3];
    float scale[3];
} animkeyframe;

typedef struct animclip {
    char name[64];
    animkeyframe *frames;
    int framecount;
    float duration;
} animclip;

typedef struct animstate {
    animclip *clip;
    float time;
    float speed;
    int playing;
    int loop;
} animstate;

typedef struct animation {
    animbone *bones;
    int bonecount;
    animclip *clips;
    int clipcount;
    animstate *states;
    int statecount;
} animation;

static animation *curanim = NULL;

object *animcreate(object **args, int argc, void *env) {
    animation *a = malloc(sizeof(animation));
    a->bones = NULL;
    a->bonecount = 0;
    a->clips = NULL;
    a->clipcount = 0;
    a->states = NULL;
    a->statecount = 0;
    curanim = a;
    return makenative(a, NULL);
}

object *animaddbone(object **args, int argc, void *env) {
    if (argc<2) throwexception("addbone needs name parent");
    if (!curanim) throwexception("no animation");
    char *name = args[0]->sval;
    int parent = args[1]->ival;
    curanim->bonecount++;
    curanim->bones = realloc(curanim->bones, curanim->bonecount * sizeof(animbone));
    animbone *b = &curanim->bones[curanim->bonecount-1];
    strncpy(b->name, name, 63);
    b->parent = parent;
    b->pos[0]=b->pos[1]=b->pos[2]=0;
    b->rot[0]=b->rot[1]=b->rot[2]=0;
    b->scale[0]=b->scale[1]=b->scale[2]=1;
    return makenone();
}

object *animsetkeyframe(object **args, int argc, void *env) {
    if (argc<8) throwexception("setkeyframe needs bone time pos rot scale");
    if (!curanim) throwexception("no animation");
    int bone = args[0]->ival;
    float time = args[1]->fval;
    float pos[3] = {args[2]->fval, args[3]->fval, args[4]->fval};
    float rot[3] = {args[5]->fval, args[6]->fval, args[7]->fval};
    float scale[3] = {args[8]->fval, args[9]->fval, args[10]->fval};
    return makenone();
}

object *animcreateclip(object **args, int argc, void *env) { return makenone(); }
object *animplayclip(object **args, int argc, void *env) { return makenone(); }
object *animupdate(object **args, int argc, void *env) { return makenone(); }
object *animgetbone(object **args, int argc, void *env) { return makenone(); }

void registeranimationlib(environment *env) {
    object *mod = makemodule("animation", NULL);
    envset(mod->module.exports, "create", makebuiltin(animcreate), 0);
    envset(mod->module.exports, "addbone", makebuiltin(animaddbone), 0);
    envset(mod->module.exports, "setkeyframe", makebuiltin(animsetkeyframe), 0);
    envset(mod->module.exports, "createclip", makebuiltin(animcreateclip), 0);
    envset(mod->module.exports, "playclip", makebuiltin(animplayclip), 0);
    envset(mod->module.exports, "update", makebuiltin(animupdate), 0);
    envset(mod->module.exports, "getbone", makebuiltin(animgetbone), 0);
    envset(env, "animation", mod, 0);
}
