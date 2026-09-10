#include "../core/axton.h"

object *frontendcreate(object **args, int argc, void *env) {
    void *fe = platformfrontendcreate();
    if (!fe) throwexception("frontend create failed");
    return makefrontendobj(fe);
}

object *frontendadd(object **args, int argc, void *env) {
    if (argc < 3) throwexception("add needs name comp");
    object *fe = args[0];
    char *name = args[1]->sval;
    object *comp = args[2];
    platformfrontendadd(fe->frontendobj.frontend, name, comp);
    return makenone();
}

object *frontendrender(object **args, int argc, void *env) {
    if (argc < 1) throwexception("render needs frontend");
    object *fe = args[0];
    platformfrontendrender(fe->frontendobj.frontend);
    return makenone();
}

void registerfrontendmodule(environment *env) {
    object *mod = makemodule("frontend", NULL);
    envset(mod->module.exports, "create", makebuiltin(frontendcreate), 0);
    envset(mod->module.exports, "add", makebuiltin(frontendadd), 0);
    envset(mod->module.exports, "render", makebuiltin(frontendrender), 0);
    envset(env, "frontend", mod, 0);
}
