#include "axton.h"
#include <pthread.h>

static object *young = NULL;
static object *old = NULL;
static int youngcount = 0;
static int oldcount = 0;
static int youngthreshold = 1024;
static int oldthreshold = 4096;
static pthread_mutex_t gclock = PTHREAD_MUTEX_INITIALIZER;

void gcinit(void) {
    young = NULL;
    old = NULL;
    youngcount = 0;
    oldcount = 0;
}

void gcaddroot(object *obj) {
    if (!obj) return;
    pthread_mutex_lock(&gclock);
    obj->next = young;
    if (young) young->prev = obj;
    young = obj;
    obj->prev = NULL;
    youngcount++;
    if (youngcount > youngthreshold) {
        gcrun();
    }
    pthread_mutex_unlock(&gclock);
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
}

static void markroots(void) {
    object *obj = young;
    while (obj) {
        gcmark(obj);
        obj = obj->next;
    }
    obj = old;
    while (obj) {
        gcmark(obj);
        obj = obj->next;
    }
}

static void sweep(object **list, int *count) {
    object *obj = *list;
    while (obj) {
        object *next = obj->next;
        if (!obj->marked) {
            if (obj->prev) obj->prev->next = obj->next;
            if (obj->next) obj->next->prev = obj->prev;
            if (*list == obj) *list = obj->next;
            free(obj);
            (*count)--;
        } else {
            obj->marked = 0;
        }
        obj = next;
    }
}

void gcrun(void) {
    pthread_mutex_lock(&gclock);
    markroots();
    sweep(&young, &youngcount);
    object *obj = young;
    while (obj) {
        object *next = obj->next;
        if (obj->marked) {
            if (obj->prev) obj->prev->next = obj->next;
            if (obj->next) obj->next->prev = obj->prev;
            if (young == obj) young = obj->next;
            youngcount--;
            obj->next = old;
            if (old) old->prev = obj;
            old = obj;
            obj->prev = NULL;
            oldcount++;
        }
        obj = next;
    }
    if (oldcount > oldthreshold) {
        markroots();
        sweep(&old, &oldcount);
    }
    pthread_mutex_unlock(&gclock);
}

object *gcalloc(int size) {
    object *obj = calloc(1, size);
    obj->marked = 0;
    obj->refcount = 0;
    obj->file = NULL;
    obj->line = 0;
    obj->next = young;
    if (young) young->prev = obj;
    young = obj;
    obj->prev = NULL;
    youngcount++;
    if (youngcount > youngthreshold) {
        gcrun();
    }
    return obj;
}
