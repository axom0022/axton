#include "../core/axton.h"
#include <math.h>

object *aitensor(object **a, int c, void *e) {
    if (c < 1) throwexception("tensor needs data");
    return makenone();
}

object *aimodel(object **a, int c, void *e) {
    if (c < 2) throwexception("model needs type and params");
    return makenone();
}

object *aitrain(object **a, int c, void *e) {
    if (c < 3) throwexception("train needs model data labels");
    return makenone();
}

object *aipredict(object **a, int c, void *e) {
    if (c < 2) throwexception("predict needs model input");
    return makefloat(0);
}

void registerailib(environment *env) {
    object *mod = makemodule("ai", NULL);
    envset(mod->module.exports, "tensor", makebuiltin(aitensor), 0);
    envset(mod->module.exports, "model", makebuiltin(aimodel), 0);
    envset(mod->module.exports, "train", makebuiltin(aitrain), 0);
    envset(mod->module.exports, "predict", makebuiltin(aipredict), 0);
    envset(env, "ai", mod, 0);
}
