#include "../core/axton.h"

object *bigintcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("create needs string");
    char *str = args[0]->sval;
    void *bi = platformbigintcreate(str);
    if (!bi) throwexception("bigint create failed");
    return makebigint(bi);
}

object *bigintadd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs two bigints");
    object *a = args[0];
    object *b = args[1];
    void *result = platformbigintadd(a->bigint.bigint, b->bigint.bigint);
    return makebigint(result);
}

object *bigintmul(object **args, int argc, void *env) {
    if (argc < 2) throwexception("mul needs two bigints");
    object *a = args[0];
    object *b = args[1];
    void *result = platformbigintmul(a->bigint.bigint, b->bigint.bigint);
    return makebigint(result);
}

void registerbigintmodule(environment *env) {
    object *mod = makemodule("bigint", NULL);
    envset(mod->module.exports, "create", makebuiltin(bigintcreate), 0);
    envset(mod->module.exports, "add", makebuiltin(bigintadd), 0);
    envset(mod->module.exports, "mul", makebuiltin(bigintmul), 0);
    envset(env, "bigint", mod, 0);
}
