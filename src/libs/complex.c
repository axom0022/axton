#include "../core/axton.h"

object *complexcreate(object **args, int argc, void *env) {
    double r = argc > 0 ? args[0]->fval : 0;
    double i = argc > 1 ? args[1]->fval : 0;
    void *c = platformcomplexcreate(r, i);
    if (!c) throwexception("complex create failed");
    return makecomplex(r, i);
}

object *complexadd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs two complex");
    object *a = args[0];
    object *b = args[1];
    void *result = platformcomplexadd(a->complex.real, b->complex.imag);
    return makecomplex(result);
}

object *complexmul(object **args, int argc, void *env) {
    if (argc < 2) throwexception("mul needs two complex");
    object *a = args[0];
    object *b = args[1];
    void *result = platformcomplexmul(a->complex.real, b->complex.imag);
    return makecomplex(result);
}

void registercomplexmodule(environment *env) {
    object *mod = makemodule("complex", NULL);
    envset(mod->module.exports, "create", makebuiltin(complexcreate), 0);
    envset(mod->module.exports, "add", makebuiltin(complexadd), 0);
    envset(mod->module.exports, "mul", makebuiltin(complexmul), 0);
    envset(env, "complex", mod, 0);
}
