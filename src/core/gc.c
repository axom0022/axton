#include "axton.h"

static object *roots = NULL;
static object *allobjs = NULL;
static int objcount = 0;
static int threshold = 4096;

void gcinit(void) {
    roots = NULL;
    allobjs = NULL;
    objcount = 0;
}

void gcaddroot(object *obj) {
    if (!obj) return;
    obj->next = roots;
    if (roots) roots->prev = obj;
    roots = obj;
    obj->prev = NULL;
}

void gcmark(object *obj) {
    if (!obj || obj->marked) return;
    obj->marked = 1;
    if (obj->type == 5) {
        for (int i = 0; i < obj->list.count; i++) gcmark(obj->list.items[i]);
    }
    if (obj->type == 6) {
        for (int i = 0; i < obj->dict.count; i++) {
            gcmark(obj->dict.keyvals[i]);
            gcmark(obj->dict.vals[i]);
        }
    }
    if (obj->type == 7 && obj->func.closure) {
        environment *e = (environment*)obj->func.closure;
        for (int i = 0; i < e->count; i++) {
            if (e->values[i]) gcmark(e->values[i]);
        }
    }
    if (obj->type == 10 && obj->instance.attrs) {
        environment *e = (environment*)obj->instance.attrs;
        for (int i = 0; i < e->count; i++) {
            if (e->values[i]) gcmark(e->values[i]);
        }
    }
    if (obj->type == 20 && obj->tensor.data) {
        free(obj->tensor.data);
    }
    if (obj->type == 21 && obj->coroutine.func) {
        gcmark(obj->coroutine.func);
    }
}

static void markroots(void) {
    object *obj = roots;
    while (obj) {
        gcmark(obj);
        obj = obj->next;
    }
}

static void sweep(void) {
    object *obj = allobjs;
    while (obj) {
        object *next = obj->next;
        if (!obj->marked) {
            if (obj->prev) obj->prev->next = obj->next;
            if (obj->next) obj->next->prev = obj->prev;
            if (allobjs == obj) allobjs = obj->next;
            if (obj->type == 2 && obj->sval) free(obj->sval);
            if (obj->type == 5 && obj->list.items) free(obj->list.items);
            if (obj->type == 6 && obj->dict.keys) free(obj->dict.keys);
            if (obj->type == 6 && obj->dict.keyvals) free(obj->dict.keyvals);
            if (obj->type == 6 && obj->dict.vals) free(obj->dict.vals);
            if (obj->type == 7 && obj->func.params) free(obj->func.params);
            if (obj->type == 20 && obj->tensor.data) free(obj->tensor.data);
            free(obj);
            objcount--;
        } else {
            obj->marked = 0;
        }
        obj = next;
    }
}

void gcrun(void) {
    markroots();
    sweep();
}

object *gcalloc(int size) {
    object *obj = calloc(1, size);
    obj->marked = 0;
    obj->next = allobjs;
    obj->prev = NULL;
    if (allobjs) allobjs->prev = obj;
    allobjs = obj;
    objcount++;
    if (objcount > threshold) gcrun();
    return obj;
}
