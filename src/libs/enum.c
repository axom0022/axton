#include "../core/axton.h"

object *enumcreate(object **a, int c, void *e) {
    if (c < 2) throwexception("enum needs name and values");
    char *name = a[0]->sval;
    object *values = a[1];
    object *cls = makeclass(name, envnew(NULL), NULL);
    if (values->type == 5) {
        for (int i = 0; i < values->list.count; i++) {
            char *key = values->list.items[i]->sval;
            envset(cls->klass.attrs, key, makeint(i), 0);
        }
    }
    return cls;
}

void registerenumlib(environment *env) {
    object *mod = makemodule("enum", NULL);
    envset(mod->module.exports, "create", makebuiltin(enumcreate), 0);
    envset(env, "enum", mod, 0);
}
