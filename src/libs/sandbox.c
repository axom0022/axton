#include "../core/axton.h"

object *sandboxcreate(object **args, int argc, void *env) {
    void *sb = platformsandboxcreate();
    if (!sb) throwexception("sandbox create failed");
    return makesandbox(sb);
}

object *sandboxsetperm(object **args, int argc, void *env) {
    if (argc < 3) throwexception("setperm needs path perm");
    object *sb = args[0];
    char *path = args[1]->sval;
    int perm = args[2]->ival;
    platformsandboxsetperm(sb->sandbox.sandbox, path, perm);
    return makenone();
}

object *sandboxrun(object **args, int argc, void *env) {
    if (argc < 2) throwexception("run needs fn");
    object *sb = args[0];
    object *fn = args[1];
    void *result = platformsandboxrun(sb->sandbox.sandbox, fn);
    return makenative(result, NULL);
}

void registersandboxmodule(environment *env) {
    object *mod = makemodule("sandbox", NULL);
    envset(mod->module.exports, "create", makebuiltin(sandboxcreate), 0);
    envset(mod->module.exports, "setperm", makebuiltin(sandboxsetperm), 0);
    envset(mod->module.exports, "run", makebuiltin(sandboxrun), 0);
    envset(env, "sandbox", mod, 0);
}
