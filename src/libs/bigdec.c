#include "../core/axton.h"

object *bigdeccreate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("create needs string scale");
    char *str = args[0]->sval;
    int scale = args[1]->ival;
    void *bd = platformbigdeccreate(str, scale);
    if (!bd) throwexception("bigdec create failed");
    return makebigdec(bd);
}

object *bigdecadd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs two bigdec");
    object *a = args[0];
    object *b = args[1];
    void *result = platformbigdecadd(a->bigdec.bigdec, b->bigdec.bigdec);
    return makebigdec(result);
}

object *bigdecmul(object **args, int argc, void *env) {
    if (argc < 2) throwexception("mul needs two bigdec");
    object *a = args[0];
    object *b = args[1];
    void *result = platformbigdecmul(a->bigdec.bigdec, b->bigdec.bigdec);
    return makebigdec(result);
}

void registerbigdecmodule(environment *env) {
    object *mod = makemodule("bigdec", NULL);
    envset(mod->module.exports, "create", makebuiltin(bigdeccreate), 0);
    envset(mod->module.exports, "add", makebuiltin(bigdecadd), 0);
    envset(mod->module.exports, "mul", makebuiltin(bigdecmul), 0);
    envset(env, "bigdec", mod, 0);
}
