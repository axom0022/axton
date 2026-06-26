#include "../core/axton.h"
#include <math.h>

typedef struct explosion {
    float pos[3];
    float radius;
    float force;
    float lifetime;
    float time;
} explosion;

object *explosioncreate(object **args, int argc, void *env) {
    if (argc < 4) throwexception("create needs pos radius force lifetime");
    explosion *e = malloc(sizeof(explosion));
    e->pos[0]=args[0]->fval; e->pos[1]=args[1]->fval; e->pos[2]=args[2]->fval;
    e->radius=args[3]->fval; e->force=args[4]->fval; e->lifetime=args[5]->fval;
    e->time=0;
    return makenative(e, NULL);
}

object *explosionupdate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("update needs explosion dt");
    explosion *e = (explosion*)args[0]->native.data;
    e->time += args[1]->fval;
    if (e->time >= e->lifetime) return makebool(0);
    return makebool(1);
}

object *explosionapply(object **args, int argc, void *env) {
    if (argc < 2) throwexception("apply needs explosion body");
    explosion *e = (explosion*)args[0]->native.data;
    return makenone();
}

object *explosiongetforce(object **args, int argc, void *env) {
    if (argc < 2) throwexception("getforce needs explosion distance");
    explosion *e = (explosion*)args[0]->native.data;
    float dist = args[1]->fval;
    float strength = e->force * (1 - dist/e->radius);
    if (strength < 0) strength = 0;
    return makefloat(strength);
}

void registerexplosionlib(environment *env) {
    object *mod = makemodule("explosion", NULL);
    envset(mod->module.exports, "create", makebuiltin(explosioncreate), 0);
    envset(mod->module.exports, "update", makebuiltin(explosionupdate), 0);
    envset(mod->module.exports, "apply", makebuiltin(explosionapply), 0);
    envset(mod->module.exports, "getforce", makebuiltin(explosiongetforce), 0);
    envset(env, "explosion", mod, 0);
}
