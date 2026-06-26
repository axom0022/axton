#include "../core/axton.h"
#include <math.h>

typedef struct wind {
    float direction[3];
    float speed;
    float gustiness;
    float turbulence;
} wind;

static wind *curwind = NULL;

object *windcreate(object **args, int argc, void *env) {
    wind *w = malloc(sizeof(wind));
    w->direction[0]=1; w->direction[1]=0; w->direction[2]=0;
    w->speed=5;
    w->gustiness=0.5;
    w->turbulence=0.1;
    curwind=w;
    return makenative(w, NULL);
}

object *windsetdirection(object **args, int argc, void *env) {
    if (argc < 3 || !curwind) throwexception("setdirection needs x y z");
    curwind->direction[0]=args[0]->fval;
    curwind->direction[1]=args[1]->fval;
    curwind->direction[2]=args[2]->fval;
    return makenone();
}

object *windsetspeed(object **args, int argc, void *env) {
    if (argc < 1 || !curwind) throwexception("setspeed needs speed");
    curwind->speed=args[0]->fval;
    return makenone();
}

object *windgetspeed(object **args, int argc, void *env) {
    if (!curwind) return makefloat(0);
    return makefloat(curwind->speed);
}

object *windgetdirection(object **args, int argc, void *env) {
    if (!curwind) return makenone();
    object *r = makelist();
    listappend(r, makefloat(curwind->direction[0]));
    listappend(r, makefloat(curwind->direction[1]));
    listappend(r, makefloat(curwind->direction[2]));
    return r;
}

void registerwindlib(environment *env) {
    object *mod = makemodule("wind", NULL);
    envset(mod->module.exports, "create", makebuiltin(windcreate), 0);
    envset(mod->module.exports, "setdirection", makebuiltin(windsetdirection), 0);
    envset(mod->module.exports, "setspeed", makebuiltin(windsetspeed), 0);
    envset(mod->module.exports, "getspeed", makebuiltin(windgetspeed), 0);
    envset(mod->module.exports, "getdirection", makebuiltin(windgetdirection), 0);
    envset(env, "wind", mod, 0);
}
