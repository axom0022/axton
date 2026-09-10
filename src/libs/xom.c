#include "../core/axton.h"

object *xomcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("create needs data");
    object *data = args[0];
    void *xom = platformxomcreate(data, data->list.count);
    if (!xom) throwexception("xom create failed");
    return makexomobj(xom);
}

object *xomadd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs two xom");
    object *a = args[0];
    object *b = args[1];
    platformxomadd(a->xomobj.xom, b->xomobj.xom);
    return makenone();
}

object *xommul(object **args, int argc, void *env) {
    if (argc < 2) throwexception("mul needs two xom");
    object *a = args[0];
    object *b = args[1];
    void *result = platformxommul(a->xomobj.xom, b->xomobj.xom);
    return makexomobj(result);
}

void registerxommodule(environment *env) {
    object *mod = makemodule("xom", NULL);
    envset(mod->module.exports, "create", makebuiltin(xomcreate), 0);
    envset(mod->module.exports, "add", makebuiltin(xomadd), 0);
    envset(mod->module.exports, "mul", makebuiltin(xommul), 0);
    envset(env, "xom", mod, 0);
}
