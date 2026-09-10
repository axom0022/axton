#include "../core/axton.h"

object *ecscreate(object **args, int argc, void *env) {
    void *ecs = platformecscreate();
    if (!ecs) throwexception("ecs create failed");
    return makeecsobj(ecs);
}

object *ecsaddentity(object **args, int argc, void *env) {
    if (argc < 2) throwexception("addentity needs entity");
    object *ecs = args[0];
    object *entity = args[1];
    platformecsaddentity(ecs->ecsobj.ecs, entity);
    return makenone();
}

object *ecsaddcomponent(object **args, int argc, void *env) {
    if (argc < 4) throwexception("addcomponent needs entity type comp");
    object *ecs = args[0];
    object *entity = args[1];
    int type = args[2]->ival;
    object *comp = args[3];
    platformecsaddcomponent(ecs->ecsobj.ecs, entity, type, comp);
    return makenone();
}

object *ecssystem(object **args, int argc, void *env) {
    if (argc < 3) throwexception("system needs type fn");
    object *ecs = args[0];
    int type = args[1]->ival;
    object *fn = args[2];
    platformecssystem(ecs->ecsobj.ecs, type, fn);
    return makenone();
}

object *ecsupdate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("update needs dt");
    object *ecs = args[0];
    float dt = args[1]->fval;
    platformecsupdate(ecs->ecsobj.ecs, dt);
    return makenone();
}

void registerecsmodule(environment *env) {
    object *mod = makemodule("ecs", NULL);
    envset(mod->module.exports, "create", makebuiltin(ecscreate), 0);
    envset(mod->module.exports, "addentity", makebuiltin(ecsaddentity), 0);
    envset(mod->module.exports, "addcomponent", makebuiltin(ecsaddcomponent), 0);
    envset(mod->module.exports, "system", makebuiltin(ecssystem), 0);
    envset(mod->module.exports, "update", makebuiltin(ecsupdate), 0);
    envset(env, "ecs", mod, 0);
}
