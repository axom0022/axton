#include "../core/axton.h"

object *enumclass(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("enum needs name and values");
    char *name = args[0]->sval;
    object *values = args[1];
    if (values->type != 6 && values->type != 5) throwexception("enum needs dict or list");
    object *cls = makeclass(name, envnew(NULL), NULL);
    if (values->type == 6) {
        for (int i = 0; i < values->dict.count; i++) {
            char *key = values->dict.keyvals[i]->sval;
            object *val = values->dict.vals[i];
            envset(cls->klass.attrs, key, val, 0);
        }
    } else {
        for (int i = 0; i < values->list.count; i++) {
            char *key = values->list.items[i]->sval;
            envset(cls->klass.attrs, key, makeint(i), 0);
        }
    }
    return cls;
}

void registerenumlib(environment *env) {
    envset(env, "Enum", makebuiltin(enumclass), 0);
}
