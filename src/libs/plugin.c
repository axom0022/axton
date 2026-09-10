#include "../core/axton.h"

object *plugincreate(object **args, int argc, void *env) {
    void *pl = platformplugincreate();
    if (!pl) throwexception("plugin create failed");
    return makeplugin(pl);
}

object *pluginload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("load needs path");
    object *pl = args[0];
    char *path = args[1]->sval;
    int res = platformpluginload(pl->plugin.plugin, path);
    if (!res) throwexception("plugin load failed");
    return makebool(1);
}

object *plugincall(object **args, int argc, void *env) {
    if (argc < 3) throwexception("call needs name args");
    object *pl = args[0];
    char *name = args[1]->sval;
    object *argslist = args[2];
    void *result = platformplugincall(pl->plugin.plugin, name, argslist);
    return makenative(result, NULL);
}

void registerpluginmodule(environment *env) {
    object *mod = makemodule("plugin", NULL);
    envset(mod->module.exports, "create", makebuiltin(plugincreate), 0);
    envset(mod->module.exports, "load", makebuiltin(pluginload), 0);
    envset(mod->module.exports, "call", makebuiltin(plugincall), 0);
    envset(env, "plugin", mod, 0);
}
