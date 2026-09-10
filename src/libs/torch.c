#include "../core/axton.h"

object *torchcreate(object **args, int argc, void *env) {
    void *tc = platformtorchcreate();
    if (!tc) throwexception("torch create failed");
    return maketorchobj(tc);
}

object *torchtrain(object **args, int argc, void *env) {
    if (argc < 4) throwexception("train needs x y epochs");
    object *tc = args[0];
    object *x = args[1];
    object *y = args[2];
    int epochs = args[3]->ival;
    platformtorchtrain(tc->torchobj.torch, x, y, epochs);
    return makenone();
}

object *torchpredict(object **args, int argc, void *env) {
    if (argc < 2) throwexception("predict needs x");
    object *tc = args[0];
    object *x = args[1];
    void *result = platformtorchpredict(tc->torchobj.torch, x);
    return makenative(result, NULL);
}

void registertorchmodule(environment *env) {
    object *mod = makemodule("torch", NULL);
    envset(mod->module.exports, "create", makebuiltin(torchcreate), 0);
    envset(mod->module.exports, "train", makebuiltin(torchtrain), 0);
    envset(mod->module.exports, "predict", makebuiltin(torchpredict), 0);
    envset(env, "torch", mod, 0);
}
