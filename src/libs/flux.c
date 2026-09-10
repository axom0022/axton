#include "../core/axton.h"

object *fluxcreate(object **args, int argc, void *env) {
    void *fx = platformfluxcreate();
    if (!fx) throwexception("flux create failed");
    return makefluxobj(fx);
}

object *fluxtrain(object **args, int argc, void *env) {
    if (argc < 4) throwexception("train needs x y epochs");
    object *fx = args[0];
    object *x = args[1];
    object *y = args[2];
    int epochs = args[3]->ival;
    platformfluxtrain(fx->fluxobj.flux, x, y, epochs);
    return makenone();
}

object *fluxpredict(object **args, int argc, void *env) {
    if (argc < 2) throwexception("predict needs x");
    object *fx = args[0];
    object *x = args[1];
    void *result = platformfluxpredict(fx->fluxobj.flux, x);
    return makenative(result, NULL);
}

void registerfluxmodule(environment *env) {
    object *mod = makemodule("flux", NULL);
    envset(mod->module.exports, "create", makebuiltin(fluxcreate), 0);
    envset(mod->module.exports, "train", makebuiltin(fluxtrain), 0);
    envset(mod->module.exports, "predict", makebuiltin(fluxpredict), 0);
    envset(env, "flux", mod, 0);
}
