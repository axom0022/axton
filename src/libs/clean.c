#include "../core/axton.h"

object *cleancreate(object **args, int argc, void *env) {
    void *cl = platformcleancreate();
    if (!cl) throwexception("xss clean create failed");
    return makecleanobj(cl);
}

object *cleansanitize(object **args, int argc, void *env) {
    if (argc < 2) throwexception("sanitize needs input");
    object *cl = args[0];
    char *input = args[1]->sval;
    char *result = platformcleansanitize(cl->cleanobj.clean, input);
    return makestring(result);
}

void registercleanmodule(environment *env) {
    object *mod = makemodule("clean", NULL);
    envset(mod->module.exports, "create", makebuiltin(cleancreate), 0);
    envset(mod->module.exports, "sanitize", makebuiltin(cleansanitize), 0);
    envset(env, "clean", mod, 0);
}
