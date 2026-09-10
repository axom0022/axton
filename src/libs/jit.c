#include "../core/axton.h"

object *jitcreate(object **args, int argc, void *env) {
    void *jit = platformjitcreate();
    if (!jit) throwexception("jit create failed");
    return makejitobj(jit);
}

object *jitcompile(object **args, int argc, void *env) {
    if (argc < 2) throwexception("compile needs src");
    object *jit = args[0];
    char *src = args[1]->sval;
    void *func = platformjitcompile(jit->jitobj.jit, src);
    return makenative(func, NULL);
}

object *jitexec(object **args, int argc, void *env) {
    if (argc < 2) throwexception("exec needs func");
    object *jit = args[0];
    void *func = args[1];
    void *result = platformjitexec(jit->jitobj.jit, func);
    return makenative(result, NULL);
}

void registerjitmodule(environment *env) {
    object *mod = makemodule("jit", NULL);
    envset(mod->module.exports, "create", makebuiltin(jitcreate), 0);
    envset(mod->module.exports, "compile", makebuiltin(jitcompile), 0);
    envset(mod->module.exports, "exec", makebuiltin(jitexec), 0);
    envset(env, "jit", mod, 0);
}
