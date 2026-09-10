#include "../core/axton.h"

object *vrcreate(object **args, int argc, void *env) {
    void *headset = platformvrcreate();
    if (!headset) throwexception("vr create failed");
    return makevr(headset);
}

object *vrconnect(object **args, int argc, void *env) {
    if (argc < 1) throwexception("connect needs vr");
    object *vr = args[0];
    int res = platformvrconnect(vr->vr.headset);
    if (!res) throwexception("vr connect failed");
    return makebool(1);
}

object *vrpoll(object **args, int argc, void *env) {
    if (argc < 1) throwexception("poll needs vr");
    object *vr = args[0];
    platformvrpoll(vr->vr.headset);
    return makenone();
}

object *vrrender(object **args, int argc, void *env) {
    if (argc < 1) throwexception("render needs vr");
    object *vr = args[0];
    platformvrrender(vr->vr.headset);
    return makenone();
}

void registervrmodule(environment *env) {
    object *mod = makemodule("vr", NULL);
    envset(mod->module.exports, "create", makebuiltin(vrcreate), 0);
    envset(mod->module.exports, "connect", makebuiltin(vrconnect), 0);
    envset(mod->module.exports, "poll", makebuiltin(vrpoll), 0);
    envset(mod->module.exports, "render", makebuiltin(vrrender), 0);
    envset(env, "vr", mod, 0);
}
