#include "axton.h"

object *makeint(long v) {
    return (object*)(((intptr_t)v << 1) | 1);
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
    obj->sval = strdup(v ? v : "");
    return obj;
}

object *makebool(int v) {
    return (object*)(((intptr_t)!!v << 2) | 2);
}

object *makenone(void) {
    return (object*)4;
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
    obj->dict.kcap = 16;
    obj->dict.khash = malloc(sizeof(unsigned long) * 16);
    obj->dict.kidx = malloc(sizeof(int) * 16);
    for (int i = 0; i < 16; i++) obj->dict.kidx[i] = -1;
    obj->dict.dcap = 4;
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
    obj->func.params = malloc(sizeof(char*) * (pcount > 0 ? pcount : 1));
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

object *makebuiltin(object *(*fn)(object**, int, environment*)) {
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
    (void)args; (void)argc;
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
    return obj;
}

object *makenative(void *handle, void *data) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 13;
    obj->native.handle = handle;
    obj->native.data = data;
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

object *makefile(FILE *f, int fd, int mode) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 23;
    obj->fileobj.file = f;
    obj->fileobj.fd = fd;
    obj->fileobj.mode = mode;
    return obj;
}

object *makethread(object *func, object *args) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 25;
    obj->threadobj.func = func;
    obj->threadobj.args = args;
    obj->threadobj.running = 1;
    return obj;
}

void listappend(object *list, object *item) {
    if (otype(list) != 5) return;
    if (list->list.count >= list->list.cap) {
        list->list.cap = list->list.cap ? list->list.cap * 2 : 4;
        list->list.items = realloc(list->list.items, sizeof(object*) * list->list.cap);
    }
    list->list.items[list->list.count++] = item;
}

object *listpop(object *list, int index) {
    if (otype(list) != 5) return NULL;
    if (index < 0) index = list->list.count + index;
    if (index < 0 || index >= list->list.count) return NULL;
    object *item = list->list.items[index];
    for (int i = index; i < list->list.count - 1; i++) {
        list->list.items[i] = list->list.items[i+1];
    }
    list->list.count--;
    return item;
}

void listinsert(object *list, int index, object *item) {
    if (otype(list) != 5) return;
    if (index < 0) index = 0;
    if (index > list->list.count) index = list->list.count;
    if (list->list.count >= list->list.cap) {
        list->list.cap = list->list.cap ? list->list.cap * 2 : 4;
        list->list.items = realloc(list->list.items, sizeof(object*) * list->list.cap);
    }
    for (int i = list->list.count; i > index; i--) {
        list->list.items[i] = list->list.items[i-1];
    }
    list->list.items[index] = item;
    list->list.count++;
}

static int cmpobjects(const void *a, const void *b) {
    object *oa = *(object**)a;
    object *ob = *(object**)b;
    if (lessthan(oa, ob)) return -1;
    if (greaterthan(oa, ob)) return 1;
    return 0;
}

void listsort(object *list) {
    if (otype(list) != 5) return;
    qsort(list->list.items, list->list.count, sizeof(object*), cmpobjects);
}

static void dictgrowhash(object *dict) {
    int ncap = dict->dict.kcap * 2;
    unsigned long *nh = malloc(sizeof(unsigned long) * ncap);
    int *ni = malloc(sizeof(int) * ncap);
    for (int i = 0; i < ncap; i++) ni[i] = -1;
    int mask = ncap - 1;
    for (int e = 0; e < dict->dict.count; e++) {
        unsigned long h = valhash(dict->dict.keyvals[e]);
        int i = (int)(h & (unsigned long)mask);
        while (ni[i] >= 0) i = (i + 1) & mask;
        nh[i] = h;
        ni[i] = e;
    }
    free(dict->dict.khash);
    free(dict->dict.kidx);
    dict->dict.khash = nh;
    dict->dict.kidx = ni;
    dict->dict.kcap = ncap;
}

void dictset(object *dict, object *key, object *val) {
    if (otype(dict) != 6) return;
    unsigned long h = valhash(key);
    int mask = dict->dict.kcap - 1;
    int i = (int)(h & (unsigned long)mask);
    while (dict->dict.kidx[i] >= 0) {
        int e = dict->dict.kidx[i];
        if (dict->dict.khash[i] == h && valuesequal(dict->dict.keyvals[e], key)) {
            dict->dict.vals[e] = val;
            return;
        }
        i = (i + 1) & mask;
    }
    if (dict->dict.count >= dict->dict.dcap) {
        dict->dict.dcap = dict->dict.dcap * 2;
        dict->dict.keys = realloc(dict->dict.keys, sizeof(char*) * dict->dict.dcap);
        dict->dict.keyvals = realloc(dict->dict.keyvals, sizeof(object*) * dict->dict.dcap);
        dict->dict.vals = realloc(dict->dict.vals, sizeof(object*) * dict->dict.dcap);
    }
    dict->dict.count++;
    dict->dict.keyvals[dict->dict.count - 1] = key;
    dict->dict.vals[dict->dict.count - 1] = val;
    dict->dict.khash[i] = h;
    dict->dict.kidx[i] = dict->dict.count - 1;
    if (dict->dict.count * 4 >= dict->dict.kcap * 3) dictgrowhash(dict);
}

object *dictget(object *dict, object *key) {
    if (otype(dict) != 6) return NULL;
    unsigned long h = valhash(key);
    int mask = dict->dict.kcap - 1;
    int i = (int)(h & (unsigned long)mask);
    while (dict->dict.kidx[i] >= 0) {
        int e = dict->dict.kidx[i];
        if (dict->dict.khash[i] == h && valuesequal(dict->dict.keyvals[e], key)) {
            return dict->dict.vals[e];
        }
        i = (i + 1) & mask;
    }
    return NULL;
}

int dicthas(object *dict, object *key) {
    if (otype(dict) != 6) return 0;
    unsigned long h = valhash(key);
    int mask = dict->dict.kcap - 1;
    int i = (int)(h & (unsigned long)mask);
    while (dict->dict.kidx[i] >= 0) {
        int e = dict->dict.kidx[i];
        if (dict->dict.khash[i] == h && valuesequal(dict->dict.keyvals[e], key)) return 1;
        i = (i + 1) & mask;
    }
    return 0;
}

object *dictkeys(object *dict) {
    object *keys = makelist();
    gcaddroot(keys);
    for (int i = 0; i < dict->dict.count; i++) {
        listappend(keys, dict->dict.keyvals[i]);
    }
    gcremoveroot(keys);
    return keys;
}

object *dictvalues(object *dict) {
    object *vals = makelist();
    gcaddroot(vals);
    for (int i = 0; i < dict->dict.count; i++) {
        listappend(vals, dict->dict.vals[i]);
    }
    gcremoveroot(vals);
    return vals;
}

object *dictitems(object *dict) {
    object *items = makelist();
    gcaddroot(items);
    for (int i = 0; i < dict->dict.count; i++) {
        object *pair = makelist();
        gcaddroot(pair);
        listappend(pair, dict->dict.keyvals[i]);
        listappend(pair, dict->dict.vals[i]);
        gcremoveroot(pair);
        listappend(items, pair);
    }
    gcremoveroot(items);
    return items;
}

int hasattr(object *obj, char *name) {
    if (otype(obj) == 9 && obj->klass.attrs) {
        return envget((environment*)obj->klass.attrs, name) != NULL;
    }
    if (otype(obj) == 10) {
        if (obj->instance.attrs && envget((environment*)obj->instance.attrs, name)) return 1;
        if (obj->instance.klass && obj->instance.klass->klass.attrs) {
            return envget((environment*)obj->instance.klass->klass.attrs, name) != NULL;
        }
    }
    if (otype(obj) == 12 && obj->module.exports) {
        return envget((environment*)obj->module.exports, name) != NULL;
    }
    return 0;
}

object *getattr(object *obj, char *name) {
    if (otype(obj) == 9 && obj->klass.attrs) {
        object *v = envget((environment*)obj->klass.attrs, name);
        if (v) return v;
    }
    if (otype(obj) == 10) {
        if (obj->instance.attrs) {
            object *v = envget((environment*)obj->instance.attrs, name);
            if (v) return v;
        }
        if (obj->instance.klass && obj->instance.klass->klass.attrs) {
            object *v = envget((environment*)obj->instance.klass->klass.attrs, name);
            if (v) return v;
        }
    }
    if (otype(obj) == 12 && obj->module.exports) {
        object *v = envget((environment*)obj->module.exports, name);
        if (v) return v;
    }
    return NULL;
}

void setattr(object *obj, char *name, object *val) {
    if (otype(obj) == 10 && obj->instance.attrs) {
        envset((environment*)obj->instance.attrs, name, val, 0);
        return;
    }
    if (otype(obj) == 9 && obj->klass.attrs) {
        envset((environment*)obj->klass.attrs, name, val, 0);
        return;
    }
}
