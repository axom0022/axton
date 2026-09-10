#include "../core/axton.h"

object *xlearncreate(object **args, int argc, void *env) {
    void *xl = platformxlearncreate();
    if (!xl) throwexception("xlearn create failed");
    return makexlearnobj(xl);
}

object *xlearntrain(object **args, int argc, void *env) {
    if (argc < 3) throwexception("train needs x y");
    object *xl = args[0];
    object *x = args[1];
    object *y = args[2];
    platformxlearntrain(xl->xlearnobj.xlearn, x, y);
    return makenone();
}

object *xlearnpredict(object **args, int argc, void *env) {
    if (argc < 2) throwexception("predict needs x");
    object *xl = args[0];
    object *x = args[1];
    void *result = platformxlearnpredict(xl->xlearnobj.xlearn, x);
    return makenative(result, NULL);
}

void registerxlearnmodule(environment *env) {
    object *mod = makemodule("xlearn", NULL);
    envset(mod->module.exports, "create", makebuiltin(xlearncreate), 0);
    envset(mod->module.exports, "train", makebuiltin(xlearntrain), 0);
    envset(mod->module.exports, "predict", makebuiltin(xlearnpredict), 0);
    envset(env, "xlearn", mod, 0);
}
