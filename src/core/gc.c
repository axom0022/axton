#include "axton.h"
#include <stdlib.h>
#include <string.h>

static object *allobjects = NULL;
static size_t objcount = 0;
static size_t gcnext = 4096;
static object **roots = NULL;
static size_t rootcount = 0;
static size_t rootcap = 0;

extern environment *envfirst(void);
extern environment *globalenv;

static void freeobject(object *obj);

void gcinit(void) {
    allobjects = NULL;
    objcount = 0;
    gcnext = 4096;
    roots = NULL;
    rootcount = 0;
    rootcap = 0;
}

void gcaddroot(object *obj) {
    if (!obj) return;
    for (size_t i = 0; i < rootcount; i++) {
        if (roots[i] == obj) return;
    }
    if (rootcount >= rootcap) {
        rootcap = rootcap ? rootcap * 2 : 1024;
        roots = realloc(roots, rootcap * sizeof(object*));
    }
    roots[rootcount++] = obj;
}

void gcremoveroot(object *obj) {
    for (size_t i = 0; i < rootcount; i++) {
        if (roots[i] == obj) {
            roots[i] = roots[--rootcount];
            return;
        }
    }
}

object *gcalloc(int size) {
    object *obj = calloc(1, size);
    obj->next = allobjects;
    allobjects = obj;
    objcount++;
    if (objcount > gcnext) gcrun();
    return obj;
}

void gcmarkenv(environment *env) {
    if (!env || env->marked) return;
    env->marked = 1;
    for (int i = 0; i < env->cap; i++) {
        if (env->slots[i].used) {
            gcmark(env->slots[i].value);
        }
    }
    gcmarkenv(env->parent);
}

void gcmark(object *obj) {
    if (!obj || obj->marked) return;
    obj->marked = 1;
    switch (obj->type) {
        case 5:
            for (int i = 0; i < obj->list.count; i++) gcmark(obj->list.items[i]);
            break;
        case 6:
            for (int i = 0; i < obj->dict.count; i++) {
                gcmark(obj->dict.keyvals[i]);
                gcmark(obj->dict.vals[i]);
            }
            break;
        case 7:
            gcmarkenv((environment*)obj->func.closure);
            for (int i = 0; i < obj->func.bcount; i++) {
                gcmark((object*)obj->func.body[i]);
            }
            gcmark(obj->func.decorators);
            break;
        case 9:
            if (obj->klass.attrs) gcmarkenv((environment*)obj->klass.attrs);
            gcmark(obj->klass.bases);
            break;
        case 10:
            gcmark(obj->instance.klass);
            if (obj->instance.attrs) gcmarkenv((environment*)obj->instance.attrs);
            break;
        case 12:
            if (obj->module.exports) gcmarkenv((environment*)obj->module.exports);
            break;
        case 21:
            gcmark(obj->generator.func);
            gcmark(obj->generator.value);
            if (obj->generator.frame) gcmarkenv((environment*)obj->generator.frame);
            break;
        case 22:
            gcmark(obj->coroutine.func);
            if (obj->coroutine.frame) gcmarkenv((environment*)obj->coroutine.frame);
            break;
        default:
            break;
    }
}

static void gcsweepenvs(void) {
    environment *prev = NULL;
    environment *env = envfirst();
    while (env) {
        environment *next = env->next;
        if (env->marked) {
            env->marked = 0;
            prev = env;
            env = next;
        } else {
            if (prev) prev->next = next;
            for (int j = 0; j < env->cap; j++) {
                if (env->slots[j].used) free(env->slots[j].name);
            }
            free(env->slots);
            free(env);
            env = next;
        }
    }
}

static void freeobject(object *dead) {
    switch (dead->type) {
        case 2: free(dead->sval); break;
        case 5: free(dead->list.items); break;
        case 6:
            free(dead->dict.keys);
            free(dead->dict.keyvals);
            free(dead->dict.vals);
            break;
        case 7:
            for (int i = 0; i < dead->func.pcount; i++) free(dead->func.params[i]);
            free(dead->func.params);
            free(dead->func.name);
            break;
        case 9: free(dead->klass.name); break;
        case 12: free(dead->module.name); break;
        case 20: free(dead->tensor.data); free(dead->tensor.shape); break;
        default: break;
    }
    free(dead);
}

static void gcsweepobjects(void) {
    object **prev = &allobjects;
    object *obj = allobjects;
    while (obj) {
        if (obj->marked) {
            obj->marked = 0;
            prev = &obj->next;
            obj = obj->next;
        } else {
            *prev = obj->next;
            object *dead = obj;
            obj = obj->next;
            objcount--;
            freeobject(dead);
        }
    }
}

void gcrun(void) {
    for (size_t i = 0; i < rootcount; i++) gcmark(roots[i]);
    if (globalenv) gcmarkenv(globalenv);
    object *cur = allobjects;
    while (cur) {
        if (cur->type == 7 || cur->type == 9 || cur->type == 10 || cur->type == 12 || cur->type == 21 || cur->type == 22) {
            gcmark(cur);
        }
        cur = cur->next;
    }
    gcsweepenvs();
    gcsweepobjects();
    gcnext = objcount * 2 + 4096;
    if (gcnext < 4096) gcnext = 4096;
}
