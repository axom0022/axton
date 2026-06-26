#include "../core/axton.h"
#include <btBulletDynamicsCommon.h>
#include <btSoftBody.h>
#include <btSoftBodyHelpers.h>
#include <btSoftRigidDynamicsWorld.h>

typedef struct softbody {
    btSoftBody *body;
    btSoftBodyWorldInfo *info;
} softbody;

typedef struct vehicle {
    btRaycastVehicle *vehicle;
    btVehicleTuning tuning;
    btRigidBody *chassis;
} vehicle;

typedef struct joint {
    btTypedConstraint *constraint;
    int type;
} joint;

static btSoftBodyWorldInfo *softworldinfo = NULL;
static btSoftRigidDynamicsWorld *softworld = NULL;

object *physsoftcreate(object **args, int argc, void *env) {
    if (!softworld) {
        softworld = new btSoftRigidDynamicsWorld(NULL, NULL, NULL, NULL);
        softworldinfo = new btSoftBodyWorldInfo();
        softworldinfo->m_gravity.setValue(0,-9.81,0);
    }
    return makenone();
}

object *physaddsoftbody(object **args, int argc, void *env) {
    if (!softworld) throwexception("soft physics not created");
    return makenone();
}

object *physaddvehicle(object **args, int argc, void *env) {
    if (argc<4) throwexception("addvehicle needs chassis mass wheelradius");
    float mass=args[0]->fval, radius=args[1]->fval;
    return makenone();
}

object *physaddjoint(object **args, int argc, void *env) {
    if (argc<3) throwexception("addjoint needs type bodya bodyb");
    int type=args[0]->ival;
    return makenone();
}

object *physapplyforceatpoint(object **args, int argc, void *env) {
    if (argc<4) throwexception("applyforceatpoint needs body fx fy fz");
    btRigidBody *body = (btRigidBody*)args[0]->native.data;
    btVector3 force(args[1]->fval, args[2]->fval, args[3]->fval);
    body->applyCentralForce(force);
    return makenone();
}

object *physsetfriction(object **args, int argc, void *env) {
    if (argc<2) throwexception("setfriction needs body friction");
    btRigidBody *body = (btRigidBody*)args[0]->native.data;
    body->setFriction(args[1]->fval);
    return makenone();
}

object *physsetrestitution(object **args, int argc, void *env) {
    if (argc<2) throwexception("setrestitution needs body restitution");
    btRigidBody *body = (btRigidBody*)args[0]->native.data;
    body->setRestitution(args[1]->fval);
    return makenone();
}

void registerphysicsextralib(environment *env) {
    object *mod = makemodule("physicsextra", NULL);
    envset(mod->module.exports, "softcreate", makebuiltin(physsoftcreate), 0);
    envset(mod->module.exports, "addsoftbody", makebuiltin(physaddsoftbody), 0);
    envset(mod->module.exports, "addvehicle", makebuiltin(physaddvehicle), 0);
    envset(mod->module.exports, "addjoint", makebuiltin(physaddjoint), 0);
    envset(mod->module.exports, "applyforceatpoint", makebuiltin(physapplyforceatpoint), 0);
    envset(mod->module.exports, "setfriction", makebuiltin(physsetfriction), 0);
    envset(mod->module.exports, "setrestitution", makebuiltin(physsetrestitution), 0);
    envset(env, "physicsextra", mod, 0);
}
