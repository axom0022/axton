#include "axton.h"

static environment *allenvs = NULL;

static unsigned int strhash(const char *s) {
    unsigned int h = 2166136261u;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h;
}

environment *envnew(environment *parent) {
    environment *env = calloc(1, sizeof(environment));
    env->cap = 16;
    env->slots = calloc(env->cap, sizeof(envslot));
    env->parent = parent;
    env->next = allenvs;
    allenvs = env;
    return env;
}

static void envgrow(environment *env) {
    int newcap = env->cap * 2;
    envslot *newslots = calloc(newcap, sizeof(envslot));
    for (int i = 0; i < env->cap; i++) {
        if (env->slots[i].used) {
            unsigned int idx = env->slots[i].hash & (newcap - 1);
            while (newslots[idx].used) idx = (idx + 1) & (newcap - 1);
            newslots[idx] = env->slots[i];
        }
    }
    free(env->slots);
    env->slots = newslots;
    env->cap = newcap;
}

void envset(environment *env, char *name, object *val, int cnst) {
    if (env->count * 4 >= env->cap * 3) {
        envgrow(env);
    }
    unsigned int h = strhash(name);
    unsigned int idx = h & (env->cap - 1);
    while (env->slots[idx].used) {
        if (env->slots[idx].hash == h && strcmp(env->slots[idx].name, name) == 0) {
            if (env->slots[idx].isconst) throwexception("cannot reassign constant");
            env->slots[idx].value = val;
            return;
        }
        idx = (idx + 1) & (env->cap - 1);
    }
    env->slots[idx].used = 1;
    env->slots[idx].name = strdup(name);
    env->slots[idx].value = val;
    env->slots[idx].isconst = cnst;
    env->slots[idx].hash = h;
    env->count++;
}

object *envget(environment *env, char *name) {
    unsigned int h = strhash(name);
    while (env) {
        unsigned int mask = env->cap - 1;
        unsigned int idx = h & mask;
        unsigned int start = idx;
        while (env->slots[idx].used) {
            if (env->slots[idx].hash == h && strcmp(env->slots[idx].name, name) == 0) {
                return env->slots[idx].value;
            }
            idx = (idx + 1) & mask;
            if (idx == start) break;
        }
        env = env->parent;
    }
    return NULL;
}

environment *envfirst(void) {
    return allenvs;
}
