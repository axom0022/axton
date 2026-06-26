#include "../core/axton.h"
#include <GL/gl.h>

typedef struct gpuparticle {
    float pos[3];
    float vel[3];
    float life;
    float maxlife;
    float size;
    float color[3];
} gpuparticle;

typedef struct gpuparticles {
    gpuparticle *particles;
    int count;
    int maxcount;
    int emitcount;
    float emitterspeed;
    float emitterpos[3];
    unsigned int vbo;
    unsigned int vao;
} gpuparticles;

static gpuparticles *curgp = NULL;

object *gpuparticlescreate(object **args, int argc, void *env) {
    int max = argc > 0 && args[0]->type == 0 ? args[0]->ival : 10000;
    gpuparticles *g = malloc(sizeof(gpuparticles));
    g->maxcount = max;
    g->count = 0;
    g->particles = calloc(max, sizeof(gpuparticle));
    g->emitcount = 100;
    g->emitterspeed = 1;
    g->emitterpos[0]=0; g->emitterpos[1]=0; g->emitterpos[2]=0;
    glGenBuffers(1, &g->vbo);
    glGenVertexArrays(1, &g->vao);
    curgp = g;
    return makenative(g, NULL);
}

object *gpuparticlesemit(object **args, int argc, void *env) {
    if (argc < 4 || !curgp) throwexception("emit needs pos speed count");
    float x=args[0]->fval, y=args[1]->fval, z=args[2]->fval;
    float speed=args[3]->fval;
    int count = argc > 4 ? args[4]->ival : 100;
    for (int i = 0; i < count && curgp->count < curgp->maxcount; i++) {
        gpuparticle *p = &curgp->particles[curgp->count++];
        p->pos[0] = x + ((float)rand()/RAND_MAX - 0.5) * 0.1;
        p->pos[1] = y + ((float)rand()/RAND_MAX - 0.5) * 0.1;
        p->pos[2] = z + ((float)rand()/RAND_MAX - 0.5) * 0.1;
        p->vel[0] = ((float)rand()/RAND_MAX - 0.5) * speed;
        p->vel[1] = ((float)rand()/RAND_MAX) * speed;
        p->vel[2] = ((float)rand()/RAND_MAX - 0.5) * speed;
        p->life = (float)rand()/RAND_MAX * 2 + 1;
        p->maxlife = p->life;
        p->size = 0.1 + (float)rand()/RAND_MAX * 0.2;
        p->color[0] = 1; p->color[1] = 0.5; p->color[2] = 0;
    }
    return makenone();
}

object *gpuparticlesupdate(object **args, int argc, void *env) {
    if (argc < 1 || !curgp) throwexception("update needs dt");
    float dt = args[0]->fval;
    for (int i = 0; i < curgp->count; i++) {
        gpuparticle *p = &curgp->particles[i];
        p->pos[0] += p->vel[0] * dt;
        p->pos[1] += p->vel[1] * dt;
        p->pos[2] += p->vel[2] * dt;
        p->vel[1] -= 9.81 * dt;
        p->life -= dt;
        if (p->life <= 0) {
            curgp->particles[i] = curgp->particles[curgp->count - 1];
            curgp->count--;
            i--;
        }
    }
    return makenone();
}

object *gpuparticlesrender(object **args, int argc, void *env) {
    if (!curgp) return makenone();
    glBindVertexArray(curgp->vao);
    glDrawArrays(GL_POINTS, 0, curgp->count);
    return makenone();
}

void registergpuparticleslib(environment *env) {
    object *mod = makemodule("gpuparticles", NULL);
    envset(mod->module.exports, "create", makebuiltin(gpuparticlescreate), 0);
    envset(mod->module.exports, "emit", makebuiltin(gpuparticlesemit), 0);
    envset(mod->module.exports, "update", makebuiltin(gpuparticlesupdate), 0);
    envset(mod->module.exports, "render", makebuiltin(gpuparticlesrender), 0);
    envset(env, "gpuparticles", mod, 0);
}
