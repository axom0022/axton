#include "../core/axton.h"
#include <math.h>

typedef struct fluidparticle {
    float x, y, z;
    float vx, vy, vz;
    float density;
    float pressure;
} fluidparticle;

typedef struct fluid {
    fluidparticle *particles;
    int count;
    int capacity;
    float viscosity;
    float restdensity;
    float gasconstant;
    float h;
    float dt;
} fluid;

object *fluidcreate(object **args, int argc, void *env) {
    int count = argc > 0 && args[0]->type == 0 ? args[0]->ival : 1000;
    fluid *f = malloc(sizeof(fluid));
    f->count = count;
    f->capacity = count;
    f->particles = calloc(count, sizeof(fluidparticle));
    f->viscosity = 0.01;
    f->restdensity = 1000.0;
    f->gasconstant = 1.0;
    f->h = 0.1;
    f->dt = 0.001;
    for (int i = 0; i < count; i++) {
        f->particles[i].x = (float)rand() / RAND_MAX;
        f->particles[i].y = (float)rand() / RAND_MAX;
        f->particles[i].z = (float)rand() / RAND_MAX;
    }
    return makenative(f, NULL);
}

object *fluidstep(object **args, int argc, void *env) {
    if (argc < 1) throwexception("step needs fluid");
    fluid *f = (fluid*)args[0]->native.data;
    for (int i = 0; i < f->count; i++) {
        f->particles[i].x += f->particles[i].vx * f->dt;
        f->particles[i].y += f->particles[i].vy * f->dt;
        f->particles[i].z += f->particles[i].vz * f->dt;
        f->particles[i].vy -= 9.81 * f->dt;
    }
    return makenone();
}

object *fluidgetpos(object **args, int argc, void *env) {
    if (argc < 2) throwexception("getpos needs fluid index");
    fluid *f = (fluid*)args[0]->native.data;
    int idx = args[1]->ival;
    if (idx < 0 || idx >= f->count) throwexception("index out of range");
    object *r = makelist();
    listappend(r, makefloat(f->particles[idx].x));
    listappend(r, makefloat(f->particles[idx].y));
    listappend(r, makefloat(f->particles[idx].z));
    return r;
}

void registerfluidlib(environment *env) {
    object *mod = makemodule("fluid", NULL);
    envset(mod->module.exports, "create", makebuiltin(fluidcreate), 0);
    envset(mod->module.exports, "step", makebuiltin(fluidstep), 0);
    envset(mod->module.exports, "getpos", makebuiltin(fluidgetpos), 0);
    envset(env, "fluid", mod, 0);
}
