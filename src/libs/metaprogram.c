#include "../core/axton.h"

object *metaeval(object **a, int c, void *e) {
    if (c < 1) throwexception("eval needs string");
    char *code = a[0]->sval;
    token *toks = tokenize(code);
    stmt *prog = parsetokens(toks, tcount);
    return evalprogram(prog, env);
}

object *metacompile(object **a, int c, void *e) {
    if (c < 1) throwexception("compile needs string");
    return makenone();
}

object *metaexec(object **a, int c, void *e) {
    if (c < 1) throwexception("exec needs code");
    token *toks = tokenize(a[0]->sval);
    stmt *prog = parsetokens(toks, tcount);
    evalprogram(prog, env);
    return makenone();
}

void registermetaprogramlib(environment *env) {
    object *mod = makemodule("meta", NULL);
    envset(mod->module.exports, "eval", makebuiltin(metaeval), 0);
    envset(mod->module.exports, "compile", makebuiltin(metacompile), 0);
    envset(mod->module.exports, "exec", makebuiltin(metaexec), 0);
    envset(env, "meta", mod, 0);
}
