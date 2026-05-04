#include "engine3d.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

struct mesh {
    int vcount;
    int icount;
    float* vertices;
    int* indices;
    char* name;
};

struct material {
    float r,g,b;
    float shininess;
};

struct camera {
    float fov, aspect, near, far;
    float eyex, eyey, eyez;
    float centerx, centery, centerz;
    float upx, upy, upz;
};

struct light {
    int type;
    float posx, posy, posz;
    float dirx, diry, dirz;
    float r,g,b;
    float intensity;
};

struct transform {
    float tx,ty,tz;
    float rx,ry,rz;
    float sx,sy,sz;
};

struct item {
    mesh* m;
    transform* t;
    material* mat;
    struct item* next;
};

struct lightitem {
    light* l;
    struct lightitem* next;
};

struct scene {
    struct item* meshes;
    struct lightitem* lights;
    camera* cam;
    int w, h;
    int running;
};

static scene* currentscene = NULL;
static int winid = 0;

void init3d(int w, int h) {
    int argc = 1;
    char* argv[1] = {"axton"};
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    winid = glutCreateWindow("Axton 3D");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.2, 0.2, 0.2, 1.0);
}

scene* createscene(void) {
    scene* s = malloc(sizeof(scene));
    s->meshes = NULL;
    s->lights = NULL;
    s->cam = NULL;
    s->w = 800;
    s->h = 600;
    s->running = 1;
    return s;
}

void destroyscene(scene* s) {
    struct item* it = s->meshes;
    while(it) { struct item* next = it->next; free(it); it = next; }
    struct lightitem* lit = s->lights;
    while(lit) { struct lightitem* next = lit->next; free(lit); lit = next; }
    free(s);
}

mesh* loadmesh(char* path) {
    FILE* f = fopen(path, "r");
    if(!f) return NULL;
    mesh* m = malloc(sizeof(mesh));
    m->vcount = 0; m->icount = 0;
    m->vertices = NULL;
    m->indices = NULL;
    m->name = strdup(path);
    char line[256];
    while(fgets(line, sizeof(line), f)) {
        if(line[0] == 'v' && line[1] == ' ') {
            float x,y,z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            m->vcount++;
            m->vertices = realloc(m->vertices, m->vcount * 3 * sizeof(float));
            m->vertices[(m->vcount-1)*3+0] = x;
            m->vertices[(m->vcount-1)*3+1] = y;
            m->vertices[(m->vcount-1)*3+2] = z;
        }
        else if(line[0] == 'f' && line[1] == ' ') {
            int a,b,c;
            sscanf(line, "f %d %d %d", &a, &b, &c);
            m->icount++;
            m->indices = realloc(m->indices, m->icount * 3 * sizeof(int));
            m->indices[(m->icount-1)*3+0] = a-1;
            m->indices[(m->icount-1)*3+1] = b-1;
            m->indices[(m->icount-1)*3+2] = c-1;
        }
    }
    fclose(f);
    return m;
}

void freemesh(mesh* m) {
    free(m->vertices);
    free(m->indices);
    free(m->name);
    free(m);
}

material* creatematerial(float r, float g, float b, float shininess) {
    material* mat = malloc(sizeof(material));
    mat->r = r; mat->g = g; mat->b = b;
    mat->shininess = shininess;
    return mat;
}

camera* createcamera(float fov, float aspect, float near, float far) {
    camera* cam = malloc(sizeof(camera));
    cam->fov = fov; cam->aspect = aspect; cam->near = near; cam->far = far;
    cam->eyex = 0; cam->eyey = 0; cam->eyez = 5;
    cam->centerx = 0; cam->centery = 0; cam->centerz = 0;
    cam->upx = 0; cam->upy = 1; cam->upz = 0;
    return cam;
}

light* createlight(int type, float x, float y, float z, float r, float g, float b, float intensity) {
    light* l = malloc(sizeof(light));
    l->type = type;
    l->posx = x; l->posy = y; l->posz = z;
    l->dirx = 0; l->diry = -1; l->dirz = 0;
    l->r = r; l->g = g; l->b = b;
    l->intensity = intensity;
    return l;
}

transform* createtransform(void) {
    transform* t = malloc(sizeof(transform));
    t->tx = t->ty = t->tz = 0;
    t->rx = t->ry = t->rz = 0;
    t->sx = t->sy = t->sz = 1;
    return t;
}

void setposition(transform* t, float x, float y, float z) {
    t->tx = x; t->ty = y; t->tz = z;
}

void setrotation(transform* t, float x, float y, float z) {
    t->rx = x; t->ry = y; t->rz = z;
}

void setscale(transform* t, float x, float y, float z) {
    t->sx = x; t->sy = y; t->sz = z;
}

void addmesh(scene* s, mesh* m, transform* t, material* mat) {
    struct item* it = malloc(sizeof(struct item));
    it->m = m;
    it->t = t;
    it->mat = mat;
    it->next = s->meshes;
    s->meshes = it;
}

void addlight(scene* s, light* l) {
    struct lightitem* it = malloc(sizeof(struct lightitem));
    it->l = l;
    it->next = s->lights;
    s->lights = it;
}

void setcamera(scene* s, camera* cam) {
    s->cam = cam;
}

static void drawmesh(mesh* m, transform* t, material* mat) {
    glPushMatrix();
    glTranslatef(t->tx, t->ty, t->tz);
    glRotatef(t->rx, 1,0,0);
    glRotatef(t->ry, 0,1,0);
    glRotatef(t->rz, 0,0,1);
    glScalef(t->sx, t->sy, t->sz);
    if(mat) {
        GLfloat diffuse[] = {mat->r, mat->g, mat->b, 1.0};
        GLfloat specular[] = {0.5,0.5,0.5,1.0};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT, GL_SHININESS, mat->shininess);
    }
    if(m->indices) {
        glBegin(GL_TRIANGLES);
        for(int i=0; i<m->icount; i++) {
            int i0 = m->indices[i*3+0];
            int i1 = m->indices[i*3+1];
            int i2 = m->indices[i*3+2];
            glVertex3f(m->vertices[i0*3+0], m->vertices[i0*3+1], m->vertices[i0*3+2]);
            glVertex3f(m->vertices[i1*3+0], m->vertices[i1*3+1], m->vertices[i1*3+2]);
            glVertex3f(m->vertices[i2*3+0], m->vertices[i2*3+1], m->vertices[i2*3+2]);
        }
        glEnd();
    }
    glPopMatrix();
}

static void setlight(light* l, int idx) {
    GLfloat ambient[] = {0.2,0.2,0.2,1.0};
    GLfloat diffuse[] = {l->r*l->intensity, l->g*l->intensity, l->b*l->intensity,1.0};
    GLfloat specular[] = {0.5,0.5,0.5,1.0};
    GLfloat position[] = {l->posx, l->posy, l->posz, l->type==0?1.0:0.0};
    glLightfv(GL_LIGHT0 + idx, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0 + idx, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0 + idx, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0 + idx, GL_POSITION, position);
    glEnable(GL_LIGHT0 + idx);
}

static void renderScene(void) {
    if(!currentscene) return;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    camera* cam = currentscene->cam;
    if(cam) {
        gluPerspective(cam->fov, cam->aspect, cam->near, cam->far);
        gluLookAt(cam->eyex, cam->eyey, cam->eyez,
                  cam->centerx, cam->centery, cam->centerz,
                  cam->upx, cam->upy, cam->upz);
    } else {
        gluPerspective(60, (double)currentscene->w/currentscene->h, 0.1, 100);
        gluLookAt(0,0,5, 0,0,0, 0,1,0);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    int li = 0;
    struct lightitem* lit = currentscene->lights;
    while(lit && li<8) {
        setlight(lit->l, li);
        li++; lit = lit->next;
    }
    struct item* it = currentscene->meshes;
    while(it) {
        drawmesh(it->m, it->t, it->mat);
        it = it->next;
    }
    glutSwapBuffers();
}

static void reshape(int w, int h) {
    if(!currentscene) return;
    currentscene->w = w; currentscene->h = h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(currentscene->cam) {
        currentscene->cam->aspect = (double)w/h;
        gluPerspective(currentscene->cam->fov, currentscene->cam->aspect,
                       currentscene->cam->near, currentscene->cam->far);
    }
    glMatrixMode(GL_MODELVIEW);
}

static void idle(void) {
    if(currentscene && currentscene->running) {
        glutPostRedisplay();
    }
}

static void keyboard(unsigned char key, int x, int y) {
    if(key == 27) { // ESC
        if(currentscene) currentscene->running = 0;
        glutDestroyWindow(winid);
        exit(0);
    }
}

void run3d(scene* s, void (*update)(void*), void (*render)(void*), void* data) {
    currentscene = s;
    glutReshapeFunc(reshape);
    glutDisplayFunc(renderScene);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    glutMainLoop();
}

void registerengine3dlib(environment* env) {
    object* mod = makemodule("engine3d", NULL);
    envset(mod->module.exports, "createscene", makebuiltin(createscene), 0);
    envset(mod->module.exports, "destroyscene", makebuiltin(destroyscene), 0);
    envset(mod->module.exports, "loadmesh", makebuiltin(loadmesh), 0);
    envset(mod->module.exports, "freemesh", makebuiltin(freemesh), 0);
    envset(mod->module.exports, "creatematerial", makebuiltin(creatematerial), 0);
    envset(mod->module.exports, "createcamera", makebuiltin(createcamera), 0);
    envset(mod->module.exports, "createlight", makebuiltin(createlight), 0);
    envset(mod->module.exports, "createtransform", makebuiltin(createtransform), 0);
    envset(mod->module.exports, "setposition", makebuiltin(setposition), 0);
    envset(mod->module.exports, "setrotation", makebuiltin(setrotation), 0);
    envset(mod->module.exports, "setscale", makebuiltin(setscale), 0);
    envset(mod->module.exports, "addmesh", makebuiltin(addmesh), 0);
    envset(mod->module.exports, "addlight", makebuiltin(addlight), 0);
    envset(mod->module.exports, "setcamera", makebuiltin(setcamera), 0);
    envset(mod->module.exports, "init3d", makebuiltin(init3d), 0);
    envset(mod->module.exports, "run3d", makebuiltin(run3d), 0);
    envset(env, "engine3d", mod, 0);
}
