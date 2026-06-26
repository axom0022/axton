#include "../core/axton.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct ainode {
    int type;
    void *data;
    struct ainode **children;
    int childcount;
    int state;
} ainode;

typedef struct aipath {
    int *nodes;
    int count;
    int capacity;
} aipath;

typedef struct navmesh {
    float *vertices;
    int *indices;
    int vcount;
    int icount;
} navmesh;

static navmesh *curmesh = NULL;
static aipath *curpath = NULL;

object *ainavmeshcreate(object **args, int argc, void *env) {
    navmesh *m = malloc(sizeof(navmesh));
    m->vertices = NULL; m->indices = NULL; m->vcount=0; m->icount=0;
    curmesh=m;
    return makenative(m, NULL);
}

object *ainavmeshload(object **args, int argc, void *env) {
    if (argc<1 || !curmesh) throwexception("navmeshload needs path");
    return makenone();
}

object *aipathfind(object **args, int argc, void *env) {
    if (argc<2) throwexception("pathfind needs start end");
    float sx=args[0]->fval, sy=args[1]->fval, sz=args[2]->fval;
    float ex=args[3]->fval, ey=args[4]->fval, ez=args[5]->fval;
    aipath *p = malloc(sizeof(aipath));
    p->nodes = malloc(sizeof(int)*4);
    p->capacity=4; p->count=0;
    curpath=p;
    return makenative(p, NULL);
}

object *aipathnext(object **args, int argc, void *env) {
    if(!curpath) throwexception("no path");
    if(curpath->count==0) return makenone();
    int node = curpath->nodes[0];
    return makeint(node);
}

object *aibehaviortree(object **args, int argc, void *env) {
    if (argc<1) throwexception("behaviortree needs definition");
    return makenone();
}

object *aifsmcreate(object **args, int argc, void *env) {
    if (argc<1) throwexception("fsm needs states");
    return makenone();
}

object *aifsmsetstate(object **args, int argc, void *env) {
    if (argc<2) throwexception("fsmsetstate needs fsm state");
    return makenone();
}

object *aifsmupdate(object **args, int argc, void *env) {
    if (argc<1) throwexception("fsmupdate needs fsm");
    return makenone();
}

object *aiflocksim(object **args, int argc, void *env) {
    if (argc<4) throwexception("flocksim needs count position spread");
    return makenone();
}

object *aiflocksimupdate(object **args, int argc, void *env) {
    if (argc<1) throwexception("flocksimupdate needs flock");
    return makenone();
}

void registerailib(environment *env) {
    object *mod = makemodule("ai", NULL);
    envset(mod->module.exports, "navmeshcreate", makebuiltin(ainavmeshcreate), 0);
    envset(mod->module.exports, "navmeshload", makebuiltin(ainavmeshload), 0);
    envset(mod->module.exports, "pathfind", makebuiltin(aipathfind), 0);
    envset(mod->module.exports, "pathnext", makebuiltin(aipathnext), 0);
    envset(mod->module.exports, "behaviortree", makebuiltin(aibehaviortree), 0);
    envset(mod->module.exports, "fsmcreate", makebuiltin(aifsmcreate), 0);
    envset(mod->module.exports, "fsmsetstate", makebuiltin(aifsmsetstate), 0);
    envset(mod->module.exports, "fsmupdate", makebuiltin(aifsmupdate), 0);
    envset(mod->module.exports, "flocksim", makebuiltin(aiflocksim), 0);
    envset(mod->module.exports, "flocksimupdate", makebuiltin(aiflocksimupdate), 0);
    envset(env, "ai", mod, 0);
}
