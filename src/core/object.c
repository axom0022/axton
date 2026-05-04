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

object *makerange(long start, long stop, long step) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 11;
    obj->range.start = start;
    obj->range.stop = stop;
    obj->range.step = step;
    return obj;
}

object *makefunc(char **params, char **typehints, int pcount, stmt **body, int bcount, environment *closure, char *name, int isasync, int isgenerator) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 7;
    obj->func.params = malloc(sizeof(char*) * pcount);
    for (int i = 0; i < pcount; i++) obj->func.params[i] = strdup(params[i]);
    obj->func.pcount = pcount;
    obj->func.body = body;
    obj->func.bcount = bcount;
    obj->func.closure = closure;
    obj->func.name = name ? strdup(name) : NULL;
    obj->func.isasync = isasync;
    obj->func.isgenerator = isgenerator;
    return obj;
}

object *makebuiltin(void *(*fn)(object**, int, environment*)) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 8;
    obj->builtin.fn = fn;
    return obj;
}

object *makeclass(char *name, environment *attrs, object *bases) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 9;
    obj->klass.name = strdup(name);
    obj->klass.attrs = attrs;
    obj->klass.bases = bases;
    return obj;
}

object *makeinstance(object *klass, object **args, int argc) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 10;
    obj->instance.klass = klass;
    obj->instance.attrs = envnew(NULL);
    return obj;
}

object *makemodule(char *name, void *handle) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 12;
    obj->module.name = name ? strdup(name) : NULL;
    obj->module.exports = envnew(NULL);
    obj->module.handle = handle;
    return obj;
}

object *makegenerator(object *func, environment *env) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 21;
    obj->generator.func = func;
    obj->generator.frame = env;
    obj->generator.state = 0;
    obj->generator.value = NULL;
    return obj;
}

object *makecoroutine(object *func, environment *env) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 22;
    obj->coroutine.func = func;
    obj->coroutine.frame = env;
    obj->coroutine.state = 0;
    return obj;
}

object *makeguiwin(void *display, void *window, int w, int h) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 40;
    obj->guiwin.display = display;
    obj->guiwin.window = window;
    obj->guiwin.gc = NULL;
    obj->guiwin.w = w;
    obj->guiwin.h = h;
    return obj;
}

object *makeglwin(void *display, void *window, void *glc) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 41;
    obj->glwin.display = display;
    obj->glwin.window = window;
    obj->glwin.glc = glc;
    return obj;
}

void listappend(object *list, object *item) {
    if (list->type != 5) return;
    if (list->list.count >= list->list.cap) {
        list->list.cap *= 2;
        list->list.items = realloc(list->list.items, sizeof(object*) * list->list.cap);
    }
    list->list.items[list->list.count++] = item;
}

object *listpop(object *list, int index) {
    if (list->type != 5) return NULL;
    if (index < 0) index = list->list.count + index;
    if (index < 0 || index >= list->list.count) return NULL;
    object *item = list->list.items[index];
    for (int i = index; i < list->list.count - 1; i++) {
        list->list.items[i] = list->list.items[i+1];
    }
    list->list.count--;
    return item;
}

void dictset(object *dict, object *key, object *val) {
    if (dict->type != 6) return;
    for (int i = 0; i < dict->dict.count; i++) {
        if (valuesequal(dict->dict.keyvals[i], key)) {
            dict->dict.vals[i] = val;
            return;
        }
    }
    dict->dict.count++;
    dict->dict.keys = realloc(dict->dict.keys, sizeof(char*) * dict->dict.count);
    dict->dict.keyvals = realloc(dict->dict.keyvals, sizeof(object*) * dict->dict.count);
    dict->dict.vals = realloc(dict->dict.vals, sizeof(object*) * dict->dict.count);
    dict->dict.keyvals[dict->dict.count - 1] = key;
    dict->dict.vals[dict->dict.count - 1] = val;
}

object *dictget(object *dict, object *key) {
    if (dict->type != 6) return NULL;
    for (int i = 0; i < dict->dict.count; i++) {
        if (valuesequal(dict->dict.keyvals[i], key)) return dict->dict.vals[i];
    }
    return NULL;
}

int dicthas(object *dict, object *key) {
    if (dict->type != 6) return 0;
    for (int i = 0; i < dict->dict.count; i++) {
        if (valuesequal(dict->dict.keyvals[i], key)) return 1;
    }
    return 0;
}

object *dictkeys(object *dict) {
    object *keys = makelist();
    for (int i = 0; i < dict->dict.count; i++) {
        listappend(keys, dict->dict.keyvals[i]);
    }
    return keys;
}

object *dictvalues(object *dict) {
    object *vals = makelist();
    for (int i = 0; i < dict->dict.count; i++) {
        listappend(vals, dict->dict.vals[i]);
    }
    return vals;
}

object *dictitems(object *dict) {
    object *items = makelist();
    for (int i = 0; i < dict->dict.count; i++) {
        object *pair = makelist();
        listappend(pair, dict->dict.keyvals[i]);
        listappend(pair, dict->dict.vals[i]);
        listappend(items, pair);
    }
    return items;
}
