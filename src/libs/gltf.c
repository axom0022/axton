#include "../core/axton.h"

object *gltfload(object **args, int argc, void *env) {
    if (argc < 1) throwexception("load needs path");
    char *path = args[0]->sval;
    void *data = platformgltfload(path);
    if (!data) throwexception("gltf load failed");
    float *verts = NULL;
    int *inds = NULL;
    int vc = 0, ic = 0;
    return makegltf(data, vc, ic, verts, inds);
}

object *gltfgetvertices(object **args, int argc, void *env) {
    if (argc < 1) throwexception("getvertices needs gltf");
    object *gltf = args[0];
    object *list = makelist();
    for (int i=0;i<gltf->gltf.vertexcount*3;i++) {
        listappend(list, makefloat(gltf->gltf.vertices[i]));
    }
    return list;
}

object *gltfgetindices(object **args, int argc, void *env) {
    if (argc < 1) throwexception("getindices needs gltf");
    object *gltf = args[0];
    object *list = makelist();
    for (int i=0;i<gltf->gltf.indexcount;i++) {
        listappend(list, makeint(gltf->gltf.indices[i]));
    }
    return list;
}

void registergltflib(environment *env) {
    object *mod = makemodule("gltf", NULL);
    envset(mod->module.exports, "load", makebuiltin(gltfload), 0);
    envset(mod->module.exports, "getvertices", makebuiltin(gltfgetvertices), 0);
    envset(mod->module.exports, "getindices", makebuiltin(gltfgetindices), 0);
    envset(env, "gltf", mod, 0);
}
