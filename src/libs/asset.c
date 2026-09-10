#include "../core/axton.h"

object *assetcreate(object **args, int argc, void *env) {
    void *asset = platformassetcreate();
    if (!asset) throwexception("asset create failed");
    return makeassetobj(asset);
}

object *assetload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("load needs path");
    object *asset = args[0];
    char *path = args[1]->sval;
    platformassetload(asset->assetobj.asset, path);
    return makenone();
}

object *assetget(object **args, int argc, void *env) {
    if (argc < 2) throwexception("get needs name");
    object *asset = args[0];
    char *name = args[1]->sval;
    void *data = platformassetget(asset->assetobj.asset, name);
    return makenative(data, NULL);
}

void registerassetmodule(environment *env) {
    object *mod = makemodule("asset", NULL);
    envset(mod->module.exports, "create", makebuiltin(assetcreate), 0);
    envset(mod->module.exports, "load", makebuiltin(assetload), 0);
    envset(mod->module.exports, "get", makebuiltin(assetget), 0);
    envset(env, "asset", mod, 0);
}
