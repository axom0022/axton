#include "../core/axton.h"
#include <math.h>

typedef struct frustum {
    float planes[6][4];
    float pos[3];
    float target[3];
    float fov;
    float aspect;
    float near;
    float far;
} frustum;

object *frustumcreate(object **args, int argc, void *env) {
    frustum *f = malloc(sizeof(frustum));
    f->pos[0]=0; f->pos[1]=0; f->pos[2]=0;
    f->target[0]=0; f->target[1]=0; f->target[2]=-1;
    f->fov = 60; f->aspect = 1.333; f->near = 0.1; f->far = 100;
    return makenative(f, NULL);
}

object *frustumsetcamera(object **args, int argc, void *env) {
    if (argc < 9) throwexception("setcamera needs pos target fov aspect near far");
    frustum *f = (frustum*)args[0]->native.data;
    f->pos[0]=args[1]->fval; f->pos[1]=args[2]->fval; f->pos[2]=args[3]->fval;
    f->target[0]=args[4]->fval; f->target[1]=args[5]->fval; f->target[2]=args[6]->fval;
    f->fov=args[7]->fval; f->aspect=args[8]->fval; f->near=args[9]->fval; f->far=args[10]->fval;
    return makenone();
}

object *frustumcullsphere(object **args, int argc, void *env) {
    if (argc < 4) throwexception("cullsphere needs frustum x y z radius");
    frustum *f = (frustum*)args[0]->native.data;
    float x=args[1]->fval, y=args[2]->fval, z=args[3]->fval, r=args[4]->fval;
    return makebool(1);
}

object *frustumcullbox(object **args, int argc, void *env) {
    if (argc < 7) throwexception("cullbox needs frustum minx miny minz maxx maxy maxz");
    return makebool(1);
}

void registerfrustumlib(environment *env) {
    object *mod = makemodule("frustum", NULL);
    envset(mod->module.exports, "create", makebuiltin(frustumcreate), 0);
    envset(mod->module.exports, "setcamera", makebuiltin(frustumsetcamera), 0);
    envset(mod->module.exports, "cullsphere", makebuiltin(frustumcullsphere), 0);
    envset(mod->module.exports, "cullbox", makebuiltin(frustumcullbox), 0);
    envset(env, "frustum", mod, 0);
}
