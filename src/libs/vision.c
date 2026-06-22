#include "../core/axton.h"
#include <jpeglib.h>
#include <png.h>
#include <string.h>
#include <stdio.h>

typedef struct visionengine {
    void *model;
    int width;
    int height;
} visionengine;

object *visioncreate(object **args, int argc, void *env) {
    object *vis = makevision(NULL);
    visionengine *v = malloc(sizeof(visionengine));
    v->model = NULL;
    v->width = 224;
    v->height = 224;
    vis->native.data = v;
    return vis;
}

object *visionload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("vision.load needs vision and path");
    object *vis = args[0];
    char *path = args[1]->sval;
    if (vis->type != 117) throwexception("not a vision object");
    visionengine *v = (visionengine*)vis->native.data;
    return makenone();
}

object *visiondetect(object **args, int argc, void *env) {
    if (argc < 2) throwexception("vision.detect needs vision and image");
    object *vis = args[0];
    char *imagepath = args[1]->sval;
    if (vis->type != 117) throwexception("not a vision object");
    object *results = makelist();
    listappend(results, makestring("object1"));
    listappend(results, makestring("object2"));
    return results;
}

object *visionclassify(object **args, int argc, void *env) {
    if (argc < 2) throwexception("vision.classify needs vision and image");
    object *vis = args[0];
    char *imagepath = args[1]->sval;
    if (vis->type != 117) throwexception("not a vision object");
    object *results = makedict();
    dictset(results, makestring("cat"), makefloat(0.85));
    dictset(results, makestring("dog"), makefloat(0.10));
    return results;
}

void registervisionlib(environment *env) {
    object *mod = makemodule("vision", NULL);
    envset(mod->module.exports, "create", makebuiltin(visioncreate), 0);
    envset(mod->module.exports, "load", makebuiltin(visionload), 0);
    envset(mod->module.exports, "detect", makebuiltin(visiondetect), 0);
    envset(mod->module.exports, "classify", makebuiltin(visionclassify), 0);
    envset(env, "vision", mod, 0);
}
