#include "../core/axton.h"

object *matrixcreate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("create needs rows cols");
    int r = args[0]->ival;
    int c = args[1]->ival;
    void *m = platformmatrixcreate(r, c);
    if (!m) throwexception("matrix create failed");
    return makematrix(m);
}

object *matrixset(object **args, int argc, void *env) {
    if (argc < 4) throwexception("set needs row col value");
    object *m = args[0];
    int r = args[1]->ival;
    int c = args[2]->ival;
    double v = args[3]->fval;
    platformmatrixset(m->matrix.matrix, r, c, v);
    return makenone();
}

object *matrixmul(object **args, int argc, void *env) {
    if (argc < 2) throwexception("mul needs two matrices");
    object *a = args[0];
    object *b = args[1];
    void *result = platformmatrixmul(a->matrix.matrix, b->matrix.matrix);
    return makematrix(result);
}

void registermatrixmodule(environment *env) {
    object *mod = makemodule("matrix", NULL);
    envset(mod->module.exports, "create", makebuiltin(matrixcreate), 0);
    envset(mod->module.exports, "set", makebuiltin(matrixset), 0);
    envset(mod->module.exports, "mul", makebuiltin(matrixmul), 0);
    envset(env, "matrix", mod, 0);
}
