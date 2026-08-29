#include "../core/axton.h"
#include <renderdoc_app.h>

object *renderdoccreate(object **args, int argc, void *env) {
    int res = platformrenderdocstart();
    object *rd = makerenderdoc(NULL);
    rd->renderdoc.enabled = res == 0;
    return rd;
}

object *renderdoccapture(object **args, int argc, void *env) {
    if (argc < 1) throwexception("capture needs renderdoc");
    object *rd = args[0];
    if (!rd->renderdoc.enabled) throwexception("renderdoc not enabled");
    platformrenderdocend();
    return makenone();
}

object *renderdocenable(object **args, int argc, void *env) {
    if (argc < 2) throwexception("enable needs renderdoc on");
    object *rd = args[0];
    rd->renderdoc.enabled = args[1]->bval;
    return makenone();
}

void registerrenderdoclib(environment *env) {
    object *mod = makemodule("renderdoc", NULL);
    envset(mod->module.exports, "create", makebuiltin(renderdoccreate), 0);
    envset(mod->module.exports, "capture", makebuiltin(renderdoccapture), 0);
    envset(mod->module.exports, "enable", makebuiltin(renderdocenable), 0);
    envset(env, "renderdoc", mod, 0);
}
