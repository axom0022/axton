#include "axton.h"
#include "bytecode.h"
#include <stdlib.h>
#include <string.h>

static object *allobjects = NULL;
static size_t objcount = 0;
static size_t envcount = 0;
static size_t gcnext = 4096;
static object **roots = NULL;
static size_t rootcount = 0;
static size_t rootcap = 0;
static environment **envroots = NULL;
static size_t envrootcount = 0;
static size_t envrootcap = 0;

extern environment *envfirst(void);
extern environment *globalenv;

static void freeobject(object *obj);

void gcinit(void) {
    allobjects = NULL;
    objcount = 0;
    envcount = 0;
    gcnext = 4096;
    roots = NULL;
    rootcount = 0;
    rootcap = 0;
    envroots = NULL;
    envrootcount = 0;
    envrootcap = 0;
}

void gcaddroot(object *obj) {
    if (!obj) return;
    if (rootcount >= rootcap) {
        rootcap = rootcap ? rootcap * 2 : 1024;
        roots = realloc(roots, rootcap * sizeof(object*));
    }
    roots[rootcount++] = obj;
}

void gcremoveroot(object *obj) {
    if (!obj || rootcount == 0) return;
    if (roots[rootcount - 1] == obj) {
        rootcount--;
        return;
    }
    for (size_t i = 0; i < rootcount; i++) {
        if (roots[i] == obj) {
            roots[i] = roots[--rootcount];
            return;
        }
    }
}

void gcaddrootenv(environment *env) {
    if (!env) return;
    if (envrootcount >= envrootcap) {
        envrootcap = envrootcap ? envrootcap * 2 : 64;
        envroots = realloc(envroots, envrootcap * sizeof(environment*));
    }
    envroots[envrootcount++] = env;
}

size_t gcrootscount(void) {
    return rootcount;
}

size_t gcrootenvscount(void) {
    return envrootcount;
}

void gctruncroots(size_t depth) {
    if (rootcount > depth) rootcount = depth;
}

void gctruncrootenvs(size_t depth) {
    if (envrootcount > depth) envrootcount = depth;
}

void gcremoverootenv(environment *env) {
    if (!env || envrootcount == 0) return;
    if (envroots[envrootcount - 1] == env) {
        envrootcount--;
        return;
    }
    for (size_t i = 0; i < envrootcount; i++) {
        if (envroots[i] == env) {
            envroots[i] = envroots[--envrootcount];
            return;
        }
    }
}

void gcenvalloc(void) {
    envcount++;
    if (envcount > gcnext) gcrun();
}

object *gcalloc(int size) {
    if (objcount > gcnext) gcrun();
    object *obj = calloc(1, size);
    obj->next = allobjects;
    allobjects = obj;
    objcount++;
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
    for (int i = 0; i < env->lcount; i++) {
        if (env->lvals[i]) gcmark(env->lvals[i]);
    }
    gcmarkenv(env->parent);
}

void gcmark(object *obj) {
    if (!obj || isintval(obj) || isboolval(obj) || isnoneval(obj) || obj->marked) return;
    obj->marked = 1;
    switch (otype(obj)) {
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
            gcmark(obj->func.decorators);
            if (obj->func.code) bytetrace(obj->func.code);
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

static size_t gcsweepenvs(void) {
    size_t alive = 0;
    environment **link = envlisthead();
    environment *env = *link;
    environment *lastkept = NULL;
    while (env) {
        environment *next = env->next;
        if (env->marked) {
            env->marked = 0;
            env->prev = lastkept;
            lastkept = env;
            alive++;
            link = &env->next;
            env = next;
        } else {
            for (int j = 0; j < env->cap; j++) {
                if (env->slots[j].used) free(env->slots[j].name);
            }
            memset(env->slots, 0, (size_t)env->cap * sizeof(envslot));
            if (env->lownnames && env->lnames) {
                for (int j = 0; j < env->lcount; j++) {
                    if (env->lnames[j]) free(env->lnames[j]);
                }
                free(env->lnames);
                free(env->lconst);
            }
            if (env->lvals && env->lvcap > 0) {
                axlvpush(env->lvals, env->lvcap);
            }
            *link = next;
            if (next) next->prev = lastkept;
            env->lvals = NULL;
            env->lnames = NULL;
            env->lconst = NULL;
            env->lcount = 0;
            envrecycle(env);
            env = next;
        }
    }
    return alive;
}

static void freeobject(object *dead) {
    switch (otype(dead)) {
        case 2: free(dead->sval); break;
        case 5: free(dead->list.items); break;
        case 6:
            free(dead->dict.keys);
            free(dead->dict.keyvals);
            free(dead->dict.vals);
            free(dead->dict.khash);
            free(dead->dict.kidx);
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
    for (size_t i = 0; i < envrootcount; i++) gcmarkenv(envroots[i]);
    if (globalenv) gcmarkenv(globalenv);
    axprogtrace();
    for (int i = 0; i < axvmsp; i++) {
        if (axvmstack[i]) gcmark(axvmstack[i]);
    }
    object *cur = allobjects;
    while (cur) {
        if (otype(cur) == 7 || otype(cur) == 9 || otype(cur) == 10 || otype(cur) == 12 || otype(cur) == 21 || otype(cur) == 22) {
            gcmark(cur);
        }
        cur = cur->next;
    }
    envcount = gcsweepenvs();
    gcsweepobjects();
    gcnext = objcount * 2 + 4096;
    if (gcnext < 4096) gcnext = 4096;
}
