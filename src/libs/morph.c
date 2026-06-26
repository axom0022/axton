#include "../core/axton.h"

typedef struct morphtarget {
    float *vertices;
    int count;
    char *name;
} morphtarget;

typedef struct morph {
    morphtarget *targets;
    int targetcount;
    float *weights;
    float *current;
    int vertexcount;
} morph;

object *morphcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("morphcreate needs vertexcount");
    int vc = args[0]->ival;
    morph *m = malloc(sizeof(morph));
    m->vertexcount = vc;
    m->targets = NULL;
    m->targetcount = 0;
    m->weights = NULL;
    m->current = calloc(vc * 3, sizeof(float));
    return makenative(m, NULL);
}

object *morphaddtarget(object **args, int argc, void *env) {
    if (argc < 2) throwexception("addtarget needs morph and vertices");
    morph *m = (morph*)args[0]->native.data;
    object *verts = args[1];
    if (verts->type != 5) throwexception("needs list");
    m->targetcount++;
    m->targets = realloc(m->targets, m->targetcount * sizeof(morphtarget));
    morphtarget *t = &m->targets[m->targetcount - 1];
    t->count = verts->list.count;
    t->vertices = malloc(t->count * sizeof(float));
    for (int i = 0; i < t->count; i++) {
        t->vertices[i] = verts->list.items[i]->fval;
    }
    t->name = NULL;
    m->weights = realloc(m->weights, m->targetcount * sizeof(float));
    m->weights[m->targetcount - 1] = 0;
    return makenone();
}

object *morphsetweight(object **args, int argc, void *env) {
    if (argc < 3) throwexception("setweight needs morph target weight");
    morph *m = (morph*)args[0]->native.data;
    int target = args[1]->ival;
    float weight = args[2]->fval;
    if (target < 0 || target >= m->targetcount) throwexception("invalid target");
    m->weights[target] = weight;
    return makenone();
}

object *morphapply(object **args, int argc, void *env) {
    if (argc < 1) throwexception("apply needs morph");
    morph *m = (morph*)args[0]->native.data;
    for (int i = 0; i < m->vertexcount * 3; i++) m->current[i] = 0;
    for (int t = 0; t < m->targetcount; t++) {
        float w = m->weights[t];
        for (int i = 0; i < m->vertexcount * 3 && i < m->targets[t].count; i++) {
            m->current[i] += m->targets[t].vertices[i] * w;
        }
    }
    return makenone();
}

object *morphgetcurrent(object **args, int argc, void *env) {
    if (argc < 1) throwexception("getcurrent needs morph");
    morph *m = (morph*)args[0]->native.data;
    object *r = makelist();
    for (int i = 0; i < m->vertexcount * 3; i++) {
        listappend(r, makefloat(m->current[i]));
    }
    return r;
}

void registermorphlib(environment *env) {
    object *mod = makemodule("morph", NULL);
    envset(mod->module.exports, "create", makebuiltin(morphcreate), 0);
    envset(mod->module.exports, "addtarget", makebuiltin(morphaddtarget), 0);
    envset(mod->module.exports, "setweight", makebuiltin(morphsetweight), 0);
    envset(mod->module.exports, "apply", makebuiltin(morphapply), 0);
    envset(mod->module.exports, "getcurrent", makebuiltin(morphgetcurrent), 0);
    envset(env, "morph", mod, 0);
}
