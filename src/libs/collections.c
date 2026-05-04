#include "../core/axton.h"

object *builtincollectionsdefaultdict(object **args, int argc, environment *env) {
    object *defaultdict = makeclass("defaultdict", envnew(NULL), NULL);
    if (argc > 0) {
        envset(defaultdict->klass.attrs, "default_factory", args[0], 0);
    }
    return defaultdict;
}

object *builtincollectionsdeque(object **args, int argc, environment *env) {
    object *deque = makelist();
    if (argc > 0 && args[0]->type == 5) {
        for (int i = 0; i < args[0]->list.count; i++) {
            listappend(deque, args[0]->list.items[i]);
        }
    }
    object *dequeclass = makeclass("deque", envnew(NULL), NULL);
    envset(dequeclass->klass.attrs, "_data", deque, 0);
    return dequeclass;
}

object *builtincollectionsnamedtuple(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("namedtuple needs name and fields");
    char *name = args[0]->sval;
    object *fields = args[1];
    if (fields->type != 5) throwexception("fields must be list");
    object *cls = makeclass(name, envnew(NULL), NULL);
    for (int i = 0; i < fields->list.count; i++) {
        if (fields->list.items[i]->type == 2) {
            envset(cls->klass.attrs, fields->list.items[i]->sval, makenone(), 0);
        }
    }
    return cls;
}

object *builtincollectionscounter(object **args, int argc, environment *env) {
    object *counter = makedict();
    if (argc > 0 && args[0]->type == 5) {
        for (int i = 0; i < args[0]->list.count; i++) {
            object *key = args[0]->list.items[i];
            object *count = dictget(counter, key);
            if (!count) count = makeint(0);
            dictset(counter, key, makeint(count->ival + 1));
        }
    }
    return counter;
}

void registercollectionslib(environment *env) {
    object *colmod = makemodule("collections", NULL);
    envset(colmod->module.exports, "defaultdict", makebuiltin(builtincollectionsdefaultdict), 0);
    envset(colmod->module.exports, "deque", makebuiltin(builtincollectionsdeque), 0);
    envset(colmod->module.exports, "namedtuple", makebuiltin(builtincollectionsnamedtuple), 0);
    envset(colmod->module.exports, "Counter", makebuiltin(builtincollectionscounter), 0);
    envset(env, "collections", colmod, 0);
}
