#include "../core/axton.h"

object *vision2create(object **args, int argc, void *env) {
    void *cv = platformvision2create();
    if (!cv) throwexception("vision create failed");
    return makevision2obj(cv);
}

object *vision2load(object **args, int argc, void *env) {
    if (argc < 2) throwexception("load needs path");
    object *cv = args[0];
    char *path = args[1]->sval;
    platformvision2load(cv->vision2obj.vision, path);
    return makenone();
}

object *vision2detect(object **args, int argc, void *env) {
    if (argc < 1) throwexception("detect needs vision");
    object *cv = args[0];
    void *result = platformvision2detect(cv->vision2obj.vision);
    return makenative(result, NULL);
}

object *vision2classify(object **args, int argc, void *env) {
    if (argc < 1) throwexception("classify needs vision");
    object *cv = args[0];
    void *result = platformvision2classify(cv->vision2obj.vision);
    return makenative(result, NULL);
}

void registervision2module(environment *env) {
    object *mod = makemodule("vision2", NULL);
    envset(mod->module.exports, "create", makebuiltin(vision2create), 0);
    envset(mod->module.exports, "load", makebuiltin(vision2load), 0);
    envset(mod->module.exports, "detect", makebuiltin(vision2detect), 0);
    envset(mod->module.exports, "classify", makebuiltin(vision2classify), 0);
    envset(env, "vision2", mod, 0);
}
