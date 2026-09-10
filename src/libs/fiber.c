#include "../core/axton.h"

object *fibercreate(object **args, int argc, void *env) {
    void *f = platformfibercreate();
    if (!f) throwexception("fiber create failed");
    return makefiber(f);
}

object *fiberswitch(object **args, int argc, void *env) {
    if (argc < 1) throwexception("switch needs fiber");
    object *fiber = args[0];
    platformfiberswitch(fiber->fiber.fiber);
    return makenone();
}

void registerfibermodule(environment *env) {
    object *mod = makemodule("fiber", NULL);
    envset(mod->module.exports, "create", makebuiltin(fibercreate), 0);
    envset(mod->module.exports, "switch", makebuiltin(fiberswitch), 0);
    envset(env, "fiber", mod, 0);
}
