#include "axton.h"

object *make_int(long v) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 0;
    obj->ival = v;
    return obj;
}

object *make_float(double v) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 1;
    obj->fval = v;
    return obj;
}

object *make_string(char *v) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 2;
    obj->sval = strdup(v);
    return obj;
}

object *make_bool(int v) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 3;
    obj->bval = v;
    return obj;
}

object *make_none(void) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 4;
    return obj;
}

object *make_list(void) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 5;
    obj->list.cap = 4;
    obj->list.count = 0;
    obj->list.items = malloc(sizeof(object*) * obj->list.cap);
    return obj;
}

object *make_dict(void) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 6;
    obj->dict.count = 0;
    obj->dict.keys = malloc(sizeof(char*) * 4);
    obj->dict.keyvals = malloc(sizeof(object*) * 4);
    obj->dict.vals = malloc(sizeof(object*) * 4);
    return obj;
}

object *make_func(char **params, int pcount, stmt **body, int bcount, environment *closure, char *name) {
    object *obj = gc_alloc(sizeof(object));
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

object *make_builtin(void *(*fn)(object**, int, environment*)) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 8;
    obj->builtin.fn = fn;
    return obj;
}

object *make_class(char *name, environment *attrs, object *super) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 9;
    obj->klass.name = strdup(name);
    obj->klass.attrs = attrs;
    obj->klass.super = super;
    return obj;
}

object *make_instance(object *klass) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 10;
    obj->instance.klass = klass;
    obj->instance.attrs = env_new(NULL);
    return obj;
}

object *make_range(long start, long stop, long step) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 11;
    obj->range.start = start;
    obj->range.stop = stop;
    obj->range.step = step;
    obj->range.current = start;
    return obj;
}

object *make_module(char *name, void *handle) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 12;
    obj->module.name = name ? strdup(name) : NULL;
    obj->module.version = NULL;
    obj->module.exports = env_new(NULL);
    obj->module.handle = handle;
    return obj;
}

object *make_native(void *handle, void *data) {
    object *obj = gc_alloc(sizeof(object));
    obj->type = 13;
    obj->native.handle = handle;
    obj->native.data = data;
    return obj;
}
