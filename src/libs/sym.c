#include "../core/axton.h"

object *symcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("create needs expr");
    char *expr = args[0]->sval;
    void *sym = platformsymcreate(expr);
    if (!sym) throwexception("sym create failed");
    return makesym(sym);
}

object *symdiff(object **args, int argc, void *env) {
    if (argc < 2) throwexception("diff needs var");
    object *sym = args[0];
    char *var = args[1]->sval;
    void *result = platformsymdiff(sym->sym.expr, var);
    return makesym(result);
}

object *symintegrate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("integrate needs var");
    object *sym = args[0];
    char *var = args[1]->sval;
    void *result = platformsymintegrate(sym->sym.expr, var);
    return makesym(result);
}

void registersymmodule(environment *env) {
    object *mod = makemodule("sym", NULL);
    envset(mod->module.exports, "create", makebuiltin(symcreate), 0);
    envset(mod->module.exports, "diff", makebuiltin(symdiff), 0);
    envset(mod->module.exports, "integrate", makebuiltin(symintegrate), 0);
    envset(env, "sym", mod, 0);
}
