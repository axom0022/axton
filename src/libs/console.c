#include "../core/axton.h"

object *consolecreate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("create needs ctx platform");
    void *ctx = args[0];
    char *platform = args[1]->sval;
    void *console = platformconsolecreate(ctx, platform);
    if (!console) throwexception("console create failed");
    return makeconsoleobj(console);
}

object *consoleconnect(object **args, int argc, void *env) {
    if (argc < 1) throwexception("connect needs console");
    object *console = args[0];
    int res = platformconsoleconnect(console->consoleobj.console);
    return makebool(res);
}

object *consoledeploy(object **args, int argc, void *env) {
    if (argc < 1) throwexception("deploy needs console");
    object *console = args[0];
    platformconsoledeploy(console->consoleobj.console);
    return makenone();
}

void registerconsolemodule(environment *env) {
    object *mod = makemodule("console", NULL);
    envset(mod->module.exports, "create", makebuiltin(consolecreate), 0);
    envset(mod->module.exports, "connect", makebuiltin(consoleconnect), 0);
    envset(mod->module.exports, "deploy", makebuiltin(consoledeploy), 0);
    envset(env, "console", mod, 0);
}
