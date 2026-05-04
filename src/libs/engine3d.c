#include "../core/axton.h"
#include <math.h>

typedef struct {
    float x,y,z;
} vec3;

typedef struct {
    vec3 pos;
    vec3 rot;
    vec3 scale;
} transform;

typedef struct {
    int vcount;
    float *verts;
    int icount;
    int *inds;
} mesh;

static mesh *cube = NULL;

static mesh *createcube(void) {
    mesh *m = malloc(sizeof(mesh));
    m->vcount = 8;
    m->verts = malloc(8*3*sizeof(float));
    float vs[] = {-1,-1,1, 1,-1,1, 1,1,1, -1,1,1, -1,-1,-1, 1,-1,-1, 1,1,-1, -1,1,-1};
    memcpy(m->verts, vs, sizeof(vs));
    m->icount = 12;
    m->inds = malloc(12*3*sizeof(int));
    int is[] = {0,1,2, 0,2,3, 1,5,6, 1,6,2, 5,4,7, 5,7,6, 4,0,3, 4,3,7, 3,2,6, 3,6,7, 4,5,1, 4,1,0};
    memcpy(m->inds, is, sizeof(is));
    return m;
}

static void drawmesh(mesh *m, transform *t) {
    glPushMatrix();
    glTranslatef(t->pos.x, t->pos.y, t->pos.z);
    glRotatef(t->rot.x, 1,0,0);
    glRotatef(t->rot.y, 0,1,0);
    glRotatef(t->rot.z, 0,0,1);
    glScalef(t->scale.x, t->scale.y, t->scale.z);
    glBegin(GL_TRIANGLES);
    for (int i=0; i<m->icount; i++) {
        int i0 = m->inds[i*3];
        int i1 = m->inds[i*3+1];
        int i2 = m->inds[i*3+2];
        glVertex3f(m->verts[i0*3], m->verts[i0*3+1], m->verts[i0*3+2]);
        glVertex3f(m->verts[i1*3], m->verts[i1*3+1], m->verts[i1*3+2]);
        glVertex3f(m->verts[i2*3], m->verts[i2*3+1], m->verts[i2*3+2]);
    }
    glEnd();
    glPopMatrix();
}

object *engine3dinit(object **args, int argc, void *env) {
    if (!cube) cube = createcube();
    return makenone();
}

object *engine3dcube(object **args, int argc, void *env) {
    transform t = {{0,0,0},{0,0,0},{1,1,1}};
    if (argc>0 && args[0]->type==6) {
        object *pos = dictget(args[0], makestring("pos"));
        if (pos && pos->type==5 && pos->list.count>=3) {
            t.pos.x = pos->list.items[0]->fval;
            t.pos.y = pos->list.items[1]->fval;
            t.pos.z = pos->list.items[2]->fval;
        }
        object *rot = dictget(args[0], makestring("rot"));
        if (rot && rot->type==5 && rot->list.count>=3) {
            t.rot.x = rot->list.items[0]->fval;
            t.rot.y = rot->list.items[1]->fval;
            t.rot.z = rot->list.items[2]->fval;
        }
        object *scl = dictget(args[0], makestring("scale"));
        if (scl && scl->type==5 && scl->list.count>=3) {
            t.scale.x = scl->list.items[0]->fval;
            t.scale.y = scl->list.items[1]->fval;
            t.scale.z = scl->list.items[2]->fval;
        }
    }
    drawmesh(cube, &t);
    return makenone();
}

void registerengine3dlib(environment *env) {
    object *mod = makemodule("engine3d", NULL);
    envset(mod->module.exports, "init", makebuiltin(engine3dinit), 0);
    envset(mod->module.exports, "cube", makebuiltin(engine3dcube), 0);
    envset(env, "engine3d", mod, 0);
}
