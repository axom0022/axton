#include "../core/axton.h"

object *scenecreate(object **args, int argc, void *env) {
    void *scene = platformscenecreate();
    if (!scene) throwexception("scene create failed");
    return makesceneobj(scene);
}

object *sceneaddnode(object **args, int argc, void *env) {
    if (argc < 2) throwexception("addnode needs node");
    object *scene = args[0];
    object *node = args[1];
    platformsceneaddnode(scene->sceneobj.scene, node);
    return makenone();
}

object *scenerender(object **args, int argc, void *env) {
    if (argc < 1) throwexception("render needs scene");
    object *scene = args[0];
    platformscenerender(scene->sceneobj.scene);
    return makenone();
}

void registerscenemodule(environment *env) {
    object *mod = makemodule("scene", NULL);
    envset(mod->module.exports, "create", makebuiltin(scenecreate), 0);
    envset(mod->module.exports, "addnode", makebuiltin(sceneaddnode), 0);
    envset(mod->module.exports, "render", makebuiltin(scenerender), 0);
    envset(env, "scene", mod, 0);
}
