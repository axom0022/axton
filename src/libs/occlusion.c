#include "../core/axton.h"
#include <GL/gl.h>

typedef struct occlusion {
    unsigned int query;
    int available;
    int visible;
    float pos[3];
    float size;
} occlusion;

object *occlusioncreate(object **args, int argc, void *env) {
    occlusion *o = malloc(sizeof(occlusion));
    glGenQueries(1, &o->query);
    o->available = 0;
    o->visible = 0;
    o->pos[0]=0; o->pos[1]=0; o->pos[2]=0;
    o->size=1;
    return makenative(o, NULL);
}

object *occlusionbegin(object **args, int argc, void *env) {
    if (argc < 1) throwexception("begin needs occlusion");
    occlusion *o = (occlusion*)args[0]->native.data;
    glBeginQuery(GL_SAMPLES_PASSED, o->query);
    return makenone();
}

object *occlusionend(object **args, int argc, void *env) {
    if (argc < 1) throwexception("end needs occlusion");
    occlusion *o = (occlusion*)args[0]->native.data;
    glEndQuery(GL_SAMPLES_PASSED);
    o->available = 0;
    return makenone();
}

object *occlusionresult(object **args, int argc, void *env) {
    if (argc < 1) throwexception("result needs occlusion");
    occlusion *o = (occlusion*)args[0]->native.data;
    if (!o->available) {
        GLuint result;
        glGetQueryObjectuiv(o->query, GL_QUERY_RESULT_AVAILABLE, &result);
        if (result) {
            glGetQueryObjectuiv(o->query, GL_QUERY_RESULT, &result);
            o->visible = result > 0;
            o->available = 1;
        }
    }
    return makebool(o->visible);
}

void registerocclusionlib(environment *env) {
    object *mod = makemodule("occlusion", NULL);
    envset(mod->module.exports, "create", makebuiltin(occlusioncreate), 0);
    envset(mod->module.exports, "begin", makebuiltin(occlusionbegin), 0);
    envset(mod->module.exports, "end", makebuiltin(occlusionend), 0);
    envset(mod->module.exports, "result", makebuiltin(occlusionresult), 0);
    envset(env, "occlusion", mod, 0);
}
