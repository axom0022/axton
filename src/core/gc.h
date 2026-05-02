#include "axton.h"

static object *gc_roots = NULL;
static object *gc_all = NULL;
static int gc_count = 0;
static int gc_threshold = 4096;

void gc_init(void) {
    gc_roots = NULL;
    gc_all = NULL;
    gc_count = 0;
}

void gc_add_root(object *obj) {
    if (!obj) return;
    obj->next = gc_roots;
    if (gc_roots) gc_roots->prev = obj;
    gc_roots = obj;
    obj->prev = NULL;
}

void gc_mark(object *obj) {
    if (!obj || obj->marked) return;
    obj->marked = 1;
    if (obj->type == 5) {
        for (int i = 0; i < obj->list.count; i++) gc_mark(obj->list.items[i]);
    }
    if (obj->type == 6) {
        for (int i = 0; i < obj->dict.count; i++) {
            gc_mark(obj->dict.keyvals[i]);
            gc_mark(obj->dict.vals[i]);
        }
    }
    if (obj->type == 7 && obj->func.closure) {
        environment *e = (environment*)obj->func.closure;
        for (int i = 0; i < e->count; i++) {
            if (e->values[i]) gc_mark(e->values[i]);
        }
    }
    if (obj->type == 10 && obj->instance.attrs) {
        environment *e = (environment*)obj->instance.attrs;
        for (int i = 0; i < e->count; i++) {
            if (e->values[i]) gc_mark(e->values[i]);
        }
    }
}

static void mark_roots(void) {
    object *obj = gc_roots;
    while (obj) {
        gc_mark(obj);
        obj = obj->next;
    }
}

static void sweep(void) {
    object *obj = gc_all;
    while (obj) {
        object *next = obj->next;
        if (!obj->marked) {
            if (obj->prev) obj->prev->next = obj->next;
            if (obj->next) obj->next->prev = obj->prev;
            if (gc_all == obj) gc_all = obj->next;
            free(obj);
            gc_count--;
        } else {
            obj->marked = 0;
        }
        obj = next;
    }
}

void gc_run(void) {
    mark_roots();
    sweep();
}

object *gc_alloc(int size) {
    object *obj = calloc(1, size);
    obj->marked = 0;
    obj->next = gc_all;
    obj->prev = NULL;
    if (gc_all) gc_all->prev = obj;
    gc_all = obj;
    gc_count++;
    if (gc_count > gc_threshold) gc_run();
    return obj;
}
