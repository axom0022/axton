#include "../core/axton.h"
#include <GL/gl.h>

typedef struct instance {
    float pos[3];
    float rot[3];
    float scale[3];
    unsigned int color;
} instance;

typedef struct instancing {
    instance *instances;
    int count;
    int cap;
    unsigned int vbo;
    unsigned int vao;
    unsigned int shader;
} instancing;

static instancing *curinst = NULL;

object *instancingcreate(object **args, int argc, void *env) {
    instancing *i = malloc(sizeof(instancing));
    i->cap = 1024;
    i->count = 0;
    i->instances = calloc(i->cap, sizeof(instance));
    glGenBuffers(1, &i->vbo);
    glGenVertexArrays(1, &i->vao);
    curinst = i;
    return makenative(i, NULL);
}

object *instancingadd(object **args, int argc, void *env) {
    if (argc < 9 || !curinst) throwexception("add needs pos rot scale color");
    if (curinst->count >= curinst->cap) {
        curinst->cap *= 2;
        curinst->instances = realloc(curinst->instances, curinst->cap * sizeof(instance));
    }
    instance *in = &curinst->instances[curinst->count++];
    in->pos[0] = args[0]->fval; in->pos[1] = args[1]->fval; in->pos[2] = args[2]->fval;
    in->rot[0] = args[3]->fval; in->rot[1] = args[4]->fval; in->rot[2] = args[5]->fval;
    in->scale[0] = args[6]->fval; in->scale[1] = args[7]->fval; in->scale[2] = args[8]->fval;
    in->color = (int)(args[9]->fval * 255) << 24 | (int)(args[10]->fval * 255) << 16 |
                (int)(args[11]->fval * 255) << 8 | 255;
    return makenone();
}

object *instancingrender(object **args, int argc, void *env) {
    if (!curinst) return makenone();
    glBindVertexArray(curinst->vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, curinst->count);
    return makenone();
}

object *instancingclear(object **args, int argc, void *env) {
    if (curinst) curinst->count = 0;
    return makenone();
}

void registerinstancinglib(environment *env) {
    object *mod = makemodule("instancing", NULL);
    envset(mod->module.exports, "create", makebuiltin(instancingcreate), 0);
    envset(mod->module.exports, "add", makebuiltin(instancingadd), 0);
    envset(mod->module.exports, "render", makebuiltin(instancingrender), 0);
    envset(mod->module.exports, "clear", makebuiltin(instancingclear), 0);
    envset(env, "instancing", mod, 0);
}
