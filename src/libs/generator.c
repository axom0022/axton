#include "../core/axton.h"

object *gencreate(object **a, int c, void *e) {
    if (c < 1) throwexception("create needs function");
    return makegenerator(a[0], env);
}

object *gennext(object **a, int c, void *e) {
    if (c < 1) throwexception("next needs generator");
    object *gen = a[0];
    if (gen->type != 21) throwexception("not a generator");
    return gen->generator.value;
}

object *genyield(object **a, int c, void *e) {
    if (c < 1) throwexception("yield needs value");
    return makenone();
}

void registergeneratorlib(environment *env) {
    object *mod = makemodule("gen", NULL);
    envset(mod->module.exports, "create", makebuiltin(gencreate), 0);
    envset(mod->module.exports, "next", makebuiltin(gennext), 0);
    envset(mod->module.exports, "yield", makebuiltin(genyield), 0);
    envset(env, "gen", mod, 0);
}
