#include "../core/axton.h"
#include <math.h>

typedef struct ikchain {
    float *positions;
    float *angles;
    int count;
    float targetx;
    float targety;
    float targetz;
    float lengths[16];
} ikchain;

object *ikcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("ikcreate needs count");
    int count = args[0]->ival;
    ikchain *ik = malloc(sizeof(ikchain));
    ik->count = count;
    ik->positions = calloc(count * 3, sizeof(float));
    ik->angles = calloc(count, sizeof(float));
    ik->targetx = 0;
    ik->targety = 0;
    ik->targetz = 0;
    for (int i = 0; i < count; i++) ik->lengths[i] = 1.0;
    return makenative(ik, NULL);
}

object *iksettarget(object **args, int argc, void *env) {
    if (argc < 4) throwexception("settarget needs x y z");
    ikchain *ik = (ikchain*)args[0]->native.data;
    ik->targetx = args[1]->fval;
    ik->targety = args[2]->fval;
    ik->targetz = args[3]->fval;
    return makenone();
}

object *iksetlengths(object **args, int argc, void *env) {
    if (argc < 2) throwexception("setlengths needs ik and list");
    ikchain *ik = (ikchain*)args[0]->native.data;
    object *list = args[1];
    if (list->type != 5) throwexception("needs list");
    for (int i = 0; i < ik->count && i < list->list.count; i++) {
        ik->lengths[i] = list->list.items[i]->fval;
    }
    return makenone();
}

object *ikresolve(object **args, int argc, void *env) {
    if (argc < 1) throwexception("resolve needs ik");
    ikchain *ik = (ikchain*)args[0]->native.data;
    float x = 0, y = 0, z = 0;
    for (int i = 0; i < ik->count; i++) {
        float angle = ik->angles[i];
        x += ik->lengths[i] * cos(angle);
        y += ik->lengths[i] * sin(angle);
        ik->positions[i * 3] = x;
        ik->positions[i * 3 + 1] = y;
        ik->positions[i * 3 + 2] = z;
    }
    return makenone();
}

object *ikgetpos(object **args, int argc, void *env) {
    if (argc < 2) throwexception("getpos needs ik index");
    ikchain *ik = (ikchain*)args[0]->native.data;
    int idx = args[1]->ival;
    if (idx < 0 || idx >= ik->count) throwexception("index out of range");
    object *r = makelist();
    listappend(r, makefloat(ik->positions[idx * 3]));
    listappend(r, makefloat(ik->positions[idx * 3 + 1]));
    listappend(r, makefloat(ik->positions[idx * 3 + 2]));
    return r;
}

void registeriklib(environment *env) {
    object *mod = makemodule("ik", NULL);
    envset(mod->module.exports, "create", makebuiltin(ikcreate), 0);
    envset(mod->module.exports, "settarget", makebuiltin(iksettarget), 0);
    envset(mod->module.exports, "setlengths", makebuiltin(iksetlengths), 0);
    envset(mod->module.exports, "resolve", makebuiltin(ikresolve), 0);
    envset(mod->module.exports, "getpos", makebuiltin(ikgetpos), 0);
    envset(env, "ik", mod, 0);
}
