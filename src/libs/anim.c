#include "../core/axton.h"

object *animcreate(object **args, int argc, void *env) {
    void *anim = platformanimcreate();
    if (!anim) throwexception("anim create failed");
    return makeanimobj(anim);
}

object *animaddbone(object **args, int argc, void *env) {
    if (argc < 3) throwexception("addbone needs name parent");
    object *anim = args[0];
    char *name = args[1]->sval;
    int parent = args[2]->ival;
    platformanimaddbone(anim->animobj.anim, name, parent);
    return makenone();
}

object *animaddkeyframe(object **args, int argc, void *env) {
    if (argc < 6) throwexception("addkeyframe needs bone time pos rot scale");
    object *anim = args[0];
    int bone = args[1]->ival;
    float time = args[2]->fval;
    float pos[3] = {args[3]->fval, args[4]->fval, args[5]->fval};
    float rot[3] = {args[6]->fval, args[7]->fval, args[8]->fval};
    float scale[3] = {args[9]->fval, args[10]->fval, args[11]->fval};
    platformanimaddkeyframe(anim->animobj.anim, bone, time, pos, rot, scale);
    return makenone();
}

object *animplay(object **args, int argc, void *env) {
    if (argc < 3) throwexception("play needs name speed");
    object *anim = args[0];
    char *name = args[1]->sval;
    float speed = args[2]->fval;
    platformanimplay(anim->animobj.anim, name, speed);
    return makenone();
}

object *animupdate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("update needs dt");
    object *anim = args[0];
    float dt = args[1]->fval;
    platformanimupdate(anim->animobj.anim, dt);
    return makenone();
}

void registeranimmodule(environment *env) {
    object *mod = makemodule("anim", NULL);
    envset(mod->module.exports, "create", makebuiltin(animcreate), 0);
    envset(mod->module.exports, "addbone", makebuiltin(animaddbone), 0);
    envset(mod->module.exports, "addkeyframe", makebuiltin(animaddkeyframe), 0);
    envset(mod->module.exports, "play", makebuiltin(animplay), 0);
    envset(mod->module.exports, "update", makebuiltin(animupdate), 0);
    envset(env, "anim", mod, 0);
}
