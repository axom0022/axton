#include "../core/axton.h"

object *property(object **args, int argc, environment *env) {
    object *prop = makeclass("property", envnew(NULL), NULL);
    if (argc > 0) envset(prop->klass.attrs, "fget", args[0], 0);
    if (argc > 1) envset(prop->klass.attrs, "fset", args[1], 0);
    if (argc > 2) envset(prop->klass.attrs, "fdel", args[2], 0);
    return prop;
}

void registerpropertylib(environment *env) {
    envset(env, "property", makebuiltin(property), 0);
}
