#include "../core/axton.h"
#include <btBulletDynamicsCommon.h>

typedef struct compoundcollider {
    btCompoundShape *shape;
    int childcount;
} compoundcollider;

object *compoundcreate(object **args, int argc, void *env) {
    compoundcollider *c = malloc(sizeof(compoundcollider));
    c->shape = new btCompoundShape();
    c->childcount = 0;
    return makenative(c, NULL);
}

object *compoundaddbox(object **args, int argc, void *env) {
    if (argc < 4) throwexception("addbox needs collider x y z");
    compoundcollider *c = (compoundcollider*)args[0]->native.data;
    float x=args[1]->fval, y=args[2]->fval, z=args[3]->fval;
    btBoxShape *box = new btBoxShape(btVector3(x,y,z));
    btTransform trans; trans.setIdentity();
    trans.setOrigin(btVector3(0,0,0));
    c->shape->addChildShape(trans, box);
    c->childcount++;
    return makenone();
}

object *compoundaddsphere(object **args, int argc, void *env) {
    if (argc < 2) throwexception("addsphere needs collider radius");
    compoundcollider *c = (compoundcollider*)args[0]->native.data;
    float r = args[1]->fval;
    btSphereShape *sphere = new btSphereShape(r);
    btTransform trans; trans.setIdentity();
    c->shape->addChildShape(trans, sphere);
    c->childcount++;
    return makenone();
}

object *compoundaddcapsule(object **args, int argc, void *env) {
    return makenone();
}

object *compoundgetshape(object **args, int argc, void *env) {
    if (argc < 1) throwexception("getshape needs collider");
    compoundcollider *c = (compoundcollider*)args[0]->native.data;
    return makeint((long)c->shape);
}

void registercompoundlib(environment *env) {
    object *mod = makemodule("compound", NULL);
    envset(mod->module.exports, "create", makebuiltin(compoundcreate), 0);
    envset(mod->module.exports, "addbox", makebuiltin(compoundaddbox), 0);
    envset(mod->module.exports, "addsphere", makebuiltin(compoundaddsphere), 0);
    envset(mod->module.exports, "addcapsule", makebuiltin(compoundaddcapsule), 0);
    envset(mod->module.exports, "getshape", makebuiltin(compoundgetshape), 0);
    envset(env, "compound", mod, 0);
}
