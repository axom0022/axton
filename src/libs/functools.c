#include "../core/axton.h"

object *functools_partial(object **args, int argc, void *env) {
    if(argc<1) throwexception("partial needs function");
    object *fn = args[0];
    object *partial = makeclass("partial", envnew(NULL));
    envset(partial->klass.attrs, "func", fn, 0);
    object *argslist = makelist();
    for(int i=1;i<argc;i++) listappend(argslist, args[i]);
    envset(partial->klass.attrs, "args", argslist, 0);
    return partial;
}

object *functools_reduce(object **args, int argc, void *env) {
    if(argc<2) throwexception("reduce needs function and list");
    object *fn = args[0];
    object *lst = args[1];
    if(lst->type!=5) throwexception("reduce needs list");
    if(lst->list.count==0) throwexception("empty list");
    object *res = lst->list.items[0];
    for(int i=1;i<lst->list.count;i++){
        object *params[2] = {res, lst->list.items[i]};
        res = callfunc(fn, params, 2, env);
    }
    return res;
}

object *functools_lru_cache(object **args, int argc, void *env) {
    object *deco = makeclass("lru_cache", envnew(NULL));
    if(argc>0 && args[0]->type==7) {
        object *fn = args[0];
        envset(deco->klass.attrs, "func", fn, 0);
        envset(deco->klass.attrs, "cache", makedict(), 0);
    }
    return deco;
}

void registerfunctoolslib(environment *env) {
    object *mod = makemodule("functools", NULL);
    envset(mod->module.exports, "partial", makebuiltin(functools_partial), 0);
    envset(mod->module.exports, "reduce", makebuiltin(functools_reduce), 0);
    envset(mod->module.exports, "lru_cache", makebuiltin(functools_lru_cache), 0);
    envset(env, "functools", mod, 0);
}
