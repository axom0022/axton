#include "../core/axton.h"

object *scicreate(object **args, int argc, void *env) {
    void *sci = platformscicreate();
    if (!sci) throwexception("sci create failed");
    return makesciobj(sci);
}

object *scistats(object **args, int argc, void *env) {
    if (argc < 2) throwexception("stats needs name");
    object *sci = args[0];
    char *name = args[1]->sval;
    double result = platformscistats(sci->sciobj.sci, name);
    return makefloat(result);
}

void registerscimodule(environment *env) {
    object *mod = makemodule("sci", NULL);
    envset(mod->module.exports, "create", makebuiltin(scicreate), 0);
    envset(mod->module.exports, "stats", makebuiltin(scistats), 0);
    envset(env, "sci", mod, 0);
}
