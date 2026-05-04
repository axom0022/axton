#include "../core/axton.h"

object *withenter(object **a, int c, void *e) {
    if (c < 1) throwexception("enter needs context manager");
    object *cm = a[0];
    object *enter = envget(cm->klass.attrs, "__enter__");
    if (!enter || enter->type != 7) throwexception("no __enter__ method");
    return callfunc(enter, NULL, 0, env);
}

object *withoutexit(object **a, int c, void *e) {
    if (c < 2) throwexception("exit needs context manager and exception");
    object *cm = a[0];
    object *exit = envget(cm->klass.attrs, "__exit__");
    if (!exit || exit->type != 7) throwexception("no __exit__ method");
    object *args[3] = {a[1], makenone(), makenone()};
    return callfunc(exit, args, 3, env);
}

void registerwithlib(environment *env) {
    object *mod = makemodule("withlib", NULL);
    envset(mod->module.exports, "__enter__", makebuiltin(withenter), 0);
    envset(mod->module.exports, "__exit__", makebuiltin(withoutexit), 0);
    envset(env, "withlib", mod, 0);
}
