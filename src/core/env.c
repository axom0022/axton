#include "axton.h"

environment *envnew(environment *parent) {
    environment *env = malloc(sizeof(environment));
    env->cap = 8;
    env->count = 0;
    env->names = malloc(sizeof(char*) * env->cap);
    env->values = malloc(sizeof(object*) * env->cap);
    env->isconst = malloc(sizeof(int) * env->cap);
    env->parent = parent;
    env->globals = NULL;
    return env;
}

void envset(environment *env, char *name, object *val, int cnst) {
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            if (env->isconst[i]) {
                throwexception("cannot reassign constant");
            }
            env->values[i] = val;
            return;
        }
    }
    if (env->count >= env->cap) {
        env->cap *= 2;
        env->names = realloc(env->names, sizeof(char*) * env->cap);
        env->values = realloc(env->values, sizeof(object*) * env->cap);
        env->isconst = realloc(env->isconst, sizeof(int) * env->cap);
    }
    env->names[env->count] = strdup(name);
    env->values[env->count] = val;
    env->isconst[env->count] = cnst;
    env->count++;
}

object *envget(environment *env, char *name) {
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) return env->values[i];
    }
    if (env->parent) return envget(env->parent, name);
    return NULL;
}
