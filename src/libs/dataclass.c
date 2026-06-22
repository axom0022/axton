#include "../core/axton.h"

object *dataclasscreate(object **a, int c, void *e) {
    if (c < 2) throwexception("dataclass needs name and fields");
    char *name = a[0]->sval;
    object *fields = a[1];
    object *cls = makeclass(name, envnew(NULL), NULL);
    envset(cls->klass.attrs, "__fields__", fields, 0);
    return cls;
}

void registerdataclasslib(environment *env) {
    object *mod = makemodule("dataclass", NULL);
    envset(mod->module.exports, "create", makebuiltin(dataclasscreate), 0);
    envset(env, "dataclass", mod, 0);
}
