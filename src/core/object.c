#include "axton.h"

object *makeint(long v) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 0;
    obj->ival = v;
    return obj;
}

object *makefloat(double v) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 1;
    obj->fval = v;
    return obj;
}

object *makestring(char *v) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 2;
    obj->sval = strdup(v);
    return obj;
}

object *makebool(int v) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 3;
    obj->bval = v;
    return obj;
}

object *makenone(void) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 4;
    return obj;
}

object *makelist(void) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 5;
    obj->list.cap = 4;
    obj->list.count = 0;
    obj->list.items = malloc(sizeof(object*) * obj->list.cap);
    return obj;
}

object *makedict(void) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 6;
    obj->dict.count = 0;
    obj->dict.keys = malloc(sizeof(char*) * 4);
    obj->dict.keyvals = malloc(sizeof(object*) * 4);
    obj->dict.vals = malloc(sizeof(object*) * 4);
    return obj;
}

object *makefunc(char **params, int pcount, stmt **body, int bcount, environment *closure, char *name) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 7;
    obj->func.params = malloc(sizeof(char*) * pcount);
    for (int i = 0; i < pcount; i++) obj->func.params[i] = strdup(params[i]);
    obj->func.pcount = pcount;
    obj->func.body = body;
    obj->func.bcount = bcount;
    obj->func.closure = closure;
    obj->func.name = name ? strdup(name) : NULL;
    return obj;
}

object *makebuiltin(void *(*fn)(object**, int, environment*)) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 8;
    obj->builtin.fn = fn;
    return obj;
}

object *makeclass(char *name, environment *attrs, object *super) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 9;
    obj->klass.name = strdup(name);
    obj->klass.attrs = attrs;
    obj->klass.super = super;
    return obj;
}

object *makeinstance(object *klass) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 10;
    obj->instance.klass = klass;
    obj->instance.attrs = envnew(NULL);
    return obj;
}

object *makerange(long start, long stop, long step) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 11;
    obj->range.start = start;
    obj->range.stop = stop;
    obj->range.step = step;
    obj->range.current = start;
    return obj;
}

object *makemodule(char *name, void *handle) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 12;
    obj->module.name = name ? strdup(name) : NULL;
    obj->module.version = NULL;
    obj->module.exports = envnew(NULL);
    obj->module.handle = handle;
    return obj;
}

object *makenative(void *handle, void *data) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 13;
    obj->native.handle = handle;
    obj->native.data = data;
    return obj;
}
