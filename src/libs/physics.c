#include "../core/axton.h"
#include <btBulletDynamicsCommon.h>

typedef struct physicsworld {
    btDiscreteDynamicsWorld *world;
    btBroadphaseInterface *broadphase;
    btCollisionConfiguration *config;
    btCollisionDispatcher *dispatcher;
    btConstraintSolver *solver;
    btAlignedObjectArray<btCollisionShape*> shapes;
} physicsworld;

static physicsworld *pworld = NULL;

object *physcreate(object **args, int argc, void *env) {
    physicsworld *pw = malloc(sizeof(physicsworld));
    pw->config = new btDefaultCollisionConfiguration();
    pw->dispatcher = new btCollisionDispatcher(pw->config);
    pw->broadphase = new btDbvtBroadphase();
    pw->solver = new btSequentialImpulseConstraintSolver();
    pw->world = new btDiscreteDynamicsWorld(pw->dispatcher, pw->broadphase, pw->solver, pw->config);
    pw->world->setGravity(btVector3(0, -9.81, 0));
    pworld = pw;
    return makenative(pw, NULL);
}

object *physsetgravity(object **args, int argc, void *env) {
    if (argc<3 || !pworld) throwexception("setgravity needs x y z");
    float x=args[0]->fval, y=args[1]->fval, z=args[2]->fval;
    pworld->world->setGravity(btVector3(x,y,z));
    return makenone();
}

object *physaddbox(object **args, int argc, void *env) {
    if (argc<4 || !pworld) throwexception("addbox needs x y z mass");
    float x=args[0]->fval, y=args[1]->fval, z=args[2]->fval;
    float mass=args[3]->fval;
    btBoxShape *shape = new btBoxShape(btVector3(x,y,z));
    pworld->shapes.push_back(shape);
    btTransform trans; trans.setIdentity();
    trans.setOrigin(btVector3(0,0,0));
    btVector3 inertia(0,0,0);
    if (mass != 0) shape->calculateLocalInertia(mass, inertia);
    btDefaultMotionState *state = new btDefaultMotionState(trans);
    btRigidBody *body = new btRigidBody(mass, state, shape, inertia);
    pworld->world->addRigidBody(body);
    return makenative(body, NULL);
}

object *physaddsphere(object **args, int argc, void *env) {
    if (argc<2) throwexception("addsphere needs radius mass");
    float r=args[0]->fval, mass=args[1]->fval;
    btSphereShape *shape = new btSphereShape(r);
    pworld->shapes.push_back(shape);
    btTransform trans; trans.setIdentity();
    btVector3 inertia(0,0,0);
    if (mass !=0) shape->calculateLocalInertia(mass, inertia);
    btDefaultMotionState *state = new btDefaultMotionState(trans);
    btRigidBody *body = new btRigidBody(mass, state, shape, inertia);
    pworld->world->addRigidBody(body);
    return makenative(body, NULL);
}

object *physaddcapsule(object **args, int argc, void *env) { return makenone(); }
object *physaddmesh(object **args, int argc, void *env) { return makenone(); }

object *physsetpos(object **args, int argc, void *env) {
    if (argc<4) throwexception("setpos needs body x y z");
    btRigidBody *body = (btRigidBody*)args[0]->native.data;
    btTransform t = body->getWorldTransform();
    t.setOrigin(btVector3(args[1]->fval, args[2]->fval, args[3]->fval));
    body->setWorldTransform(t);
    return makenone();
}

object *physgetpos(object **args, int argc, void *env) {
    if (argc<1) throwexception("getpos needs body");
    btRigidBody *body = (btRigidBody*)args[0]->native.data;
    btVector3 pos = body->getWorldTransform().getOrigin();
    object *r = makelist();
    listappend(r, makefloat(pos.x()));
    listappend(r, makefloat(pos.y()));
    listappend(r, makefloat(pos.z()));
    return r;
}

object *physapplyforce(object **args, int argc, void *env) {
    if (argc<4) throwexception("applyforce needs body fx fy fz");
    btRigidBody *body = (btRigidBody*)args[0]->native.data;
    body->applyCentralForce(btVector3(args[1]->fval, args[2]->fval, args[3]->fval));
    return makenone();
}

object *physapplyimpulse(object **args, int argc, void *env) {
    if (argc<4) throwexception("applyimpulse needs body ix iy iz");
    btRigidBody *body = (btRigidBody*)args[0]->native.data;
    body->applyCentralImpulse(btVector3(args[1]->fval, args[2]->fval, args[3]->fval));
    return makenone();
}

object *physstep(object **args, int argc, void *env) {
    if (!pworld) throwexception("physics not created");
    float dt = (argc>0 && args[0]->type==1) ? args[0]->fval : 0.016f;
    pworld->world->stepSimulation(dt);
    return makenone();
}

object *physdestroy(object **args, int argc, void *env) {
    if (pworld) {
        delete pworld->world; delete pworld->solver; delete pworld->broadphase;
        delete pworld->dispatcher; delete pworld->config;
        free(pworld); pworld=NULL;
    }
    return makenone();
}

void registerphysicslib(environment *env) {
    object *mod = makemodule("physics", NULL);
    envset(mod->module.exports, "create", makebuiltin(physcreate), 0);
    envset(mod->module.exports, "setgravity", makebuiltin(physsetgravity), 0);
    envset(mod->module.exports, "addbox", makebuiltin(physaddbox), 0);
    envset(mod->module.exports, "addsphere", makebuiltin(physaddsphere), 0);
    envset(mod->module.exports, "addcapsule", makebuiltin(physaddcapsule), 0);
    envset(mod->module.exports, "addmesh", makebuiltin(physaddmesh), 0);
    envset(mod->module.exports, "setpos", makebuiltin(physsetpos), 0);
    envset(mod->module.exports, "getpos", makebuiltin(physgetpos), 0);
    envset(mod->module.exports, "applyforce", makebuiltin(physapplyforce), 0);
    envset(mod->module.exports, "applyimpulse", makebuiltin(physapplyimpulse), 0);
    envset(mod->module.exports, "step", makebuiltin(physstep), 0);
    envset(mod->module.exports, "destroy", makebuiltin(physdestroy), 0);
    envset(env, "physics", mod, 0);
}
