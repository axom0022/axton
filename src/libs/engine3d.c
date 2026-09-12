#include "../core/axton.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>

#define maxshaders 32
#define maxtextures 64
#define maxmeshes 128
#define maxlights 8
#define maxparticles 4096
#define maxbones 64
#define maxkeyframes 256

typedef struct etexture {
    unsigned int id;
    int w, h, ch;
} etexture;

typedef struct eshader {
    unsigned int prog;
} eshader;

typedef struct emesh {
    unsigned int vao, vbo, ebo, nbo, uvbo, bbo;
    int vcount, icount;
    float *verts, *norms, *uvs;
    int *inds;
    int *boneids;
    float *boneweights;
    struct etransform *transform;
    struct ematerial *material;
    struct emesh *next;
} emesh;

typedef struct ematerial {
    float diffuse[3], specular[3], emissive[3];
    float shininess;
    eshader *shader;
    etexture *texture;
    struct ematerial *next;
} ematerial;

typedef struct ecamera {
    float pos[3], target[3], up[3];
    float fov, aspect, near, far;
    int ortho;
    float ortho_l, ortho_r, ortho_b, ortho_t, ortho_n, ortho_f;
} ecamera;

typedef struct elight {
    int type;
    float pos[3], dir[3], color[3];
    float intensity;
} elight;

typedef struct etransform {
    float pos[3], rot[3], scale[3];
    float matrix[16];
} etransform;

typedef struct escene {
    emesh *meshes;
    ematerial *materials;
    elight lights[maxlights];
    int lightcount;
    ecamera *camera;
    int width, height;
} escene;

static escene *curscene = NULL;
static unsigned int defshader = 0;
static float bgcol[3] = {0.2, 0.2, 0.3};

static unsigned int compileshader(const char *src, int type) {
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char info[512];
        glGetShaderInfoLog(s, 512, NULL, info);
        throwexception(info);
    }
    return s;
}

static unsigned int makeprog(const char *v, const char *f) {
    unsigned int vs = compileshader(v, GL_VERTEX_SHADER);
    unsigned int fs = compileshader(f, GL_FRAGMENT_SHADER);
    unsigned int p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    int ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char info[512];
        glGetProgramInfoLog(p, 512, NULL, info);
        throwexception(info);
    }
    return p;
}

static const char *defv =
    "#version 330 core\n"
    "layout(location=0) in vec3 p;\n"
    "layout(location=1) in vec3 n;\n"
    "layout(location=2) in vec2 u;\n"
    "uniform mat4 m, v, pr;\n"
    "out vec3 fn, fp, fu;\n"
    "void main(){\n"
    "  fp = vec3(m * vec4(p, 1.0));\n"
    "  fn = mat3(transpose(inverse(m))) * n;\n"
    "  fu = u;\n"
    "  gl_Position = pr * v * m * vec4(p, 1.0);\n"
    "}\n";

static const char *deff =
    "#version 330 core\n"
    "in vec3 fn, fp, fu;\n"
    "uniform vec3 lp, lc, vp, diff;\n"
    "uniform sampler2D tex;\n"
    "uniform int ht;\n"
    "out vec4 oc;\n"
    "void main(){\n"
    "  vec3 norm = normalize(fn);\n"
    "  vec3 ld = normalize(lp - fp);\n"
    "  float diffa = max(dot(norm, ld), 0.0);\n"
    "  vec3 vd = normalize(vp - fp);\n"
    "  vec3 rd = reflect(-ld, norm);\n"
    "  float spec = pow(max(dot(vd, rd), 0.0), 32.0);\n"
    "  vec3 amb = 0.15 * diff;\n"
    "  vec3 col = amb + diffa * lc * diff + spec * lc;\n"
    "  if (ht > 0) col *= texture(tex, fu).rgb;\n"
    "  oc = vec4(col, 1.0);\n"
    "}\n";

static void matid(float *m) {
    for (int i = 0; i < 16; i++) m[i] = 0;
    m[0] = m[5] = m[10] = m[15] = 1;
}

static void matmul(float *a, float *b, float *out) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0;
            for (int k = 0; k < 4; k++) sum += a[i * 4 + k] * b[k * 4 + j];
            out[i * 4 + j] = sum;
        }
    }
}

static void gettfm(etransform *t, float *out) {
    matid(out);
    float cx = cosf(t->rot[0]), sx = sinf(t->rot[0]);
    float cy = cosf(t->rot[1]), sy = sinf(t->rot[1]);
    float cz = cosf(t->rot[2]), sz = sinf(t->rot[2]);
    float rx[16] = {1,0,0,0, 0,cx,-sx,0, 0,sx,cx,0, 0,0,0,1};
    float ry[16] = {cy,0,sy,0, 0,1,0,0, -sy,0,cy,0, 0,0,0,1};
    float rz[16] = {cz,-sz,0,0, sz,cz,0,0, 0,0,1,0, 0,0,0,1};
    float tmp[16], tmp2[16];
    matmul(rz, ry, tmp);
    matmul(tmp, rx, tmp2);
    tmp2[12] = t->pos[0];
    tmp2[13] = t->pos[1];
    tmp2[14] = t->pos[2];
    tmp2[0] *= t->scale[0];
    tmp2[5] *= t->scale[1];
    tmp2[10] *= t->scale[2];
    memcpy(out, tmp2, 16 * sizeof(float));
}

escene *esccreate(void) {
    escene *s = calloc(1, sizeof(escene));
    s->lightcount = 0;
    s->width = 800;
    s->height = 600;
    return s;
}

void escsetcamera(escene *s, ecamera *c) {
    if (s) s->camera = c;
}

void escaddmesh(escene *s, emesh *m, etransform *t, ematerial *mat) {
    if (!s || !m) return;
    m->transform = t;
    m->material = mat;
    m->next = s->meshes;
    s->meshes = m;
}

void escaddlight(escene *s, elight *l) {
    if (!s || s->lightcount >= maxlights) return;
    memcpy(&s->lights[s->lightcount++], l, sizeof(elight));
}

void escclearmeshes(escene *s) {
    if (s) s->meshes = NULL;
}

static void rendermesh(emesh *m, etransform *t, ematerial *mat, ecamera *cam, float *lp, float *lc) {
    if (!m || !cam) return;
    float model[16], view[16], proj[16];
    if (t) gettfm(t, model);
    else matid(model);

    float fx = cam->pos[0] - cam->target[0];
    float fy = cam->pos[1] - cam->target[1];
    float fz = cam->pos[2] - cam->target[2];
    float flen = sqrtf(fx * fx + fy * fy + fz * fz);
    if (flen < 0.0001f) flen = 1.0f;
    fx /= flen; fy /= flen; fz /= flen;

    float sx = fy * cam->up[2] - fz * cam->up[1];
    float sy = fz * cam->up[0] - fx * cam->up[2];
    float sz = fx * cam->up[1] - fy * cam->up[0];
    float slen = sqrtf(sx * sx + sy * sy + sz * sz);
    if (slen < 0.0001f) slen = 1.0f;
    sx /= slen; sy /= slen; sz /= slen;

    float tx = sy * fz - sz * fy;
    float ty = sz * fx - sx * fz;
    float tz = sx * fy - sy * fx;

    memset(view, 0, sizeof(view));
    view[0] = sx; view[4] = sy; view[8]  = sz;
    view[1] = tx; view[5] = ty; view[9]  = tz;
    view[2] = fx; view[6] = fy; view[10] = fz;
    view[12] = -(sx * cam->pos[0] + sy * cam->pos[1] + sz * cam->pos[2]);
    view[13] = -(tx * cam->pos[0] + ty * cam->pos[1] + tz * cam->pos[2]);
    view[14] = (fx * cam->pos[0] + fy * cam->pos[1] + fz * cam->pos[2]);
    view[15] = 1;

    memset(proj, 0, sizeof(proj));
    float fovr = cam->fov * 3.14159265f / 180.0f;
    float th = tanf(fovr * 0.5f);
    proj[0] = 1.0f / (th * cam->aspect);
    proj[5] = 1.0f / th;
    proj[10] = -(cam->far + cam->near) / (cam->far - cam->near);
    proj[11] = -1.0f;
    proj[14] = -2.0f * cam->far * cam->near / (cam->far - cam->near);

    unsigned int prog = (mat && mat->shader) ? mat->shader->prog : defshader;
    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "m"), 1, GL_FALSE, model);
    glUniformMatrix4fv(glGetUniformLocation(prog, "v"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(prog, "pr"), 1, GL_FALSE, proj);
    glUniform3fv(glGetUniformLocation(prog, "lp"), 1, lp);
    glUniform3fv(glGetUniformLocation(prog, "lc"), 1, lc);
    glUniform3fv(glGetUniformLocation(prog, "vp"), 1, cam->pos);
    if (mat) {
        glUniform3fv(glGetUniformLocation(prog, "diff"), 1, mat->diffuse);
        glUniform1i(glGetUniformLocation(prog, "ht"), mat->texture ? 1 : 0);
        if (mat->texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat->texture->id);
        }
    } else {
        float white[3] = {1, 1, 1};
        glUniform3fv(glGetUniformLocation(prog, "diff"), 1, white);
        glUniform1i(glGetUniformLocation(prog, "ht"), 0);
    }

    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    if (m->nbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m->nbo);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    } else {
        glDisableVertexAttribArray(1);
    }
    if (m->uvbo) {
        glBindBuffer(GL_ARRAY_BUFFER, m->uvbo);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    } else {
        glDisableVertexAttribArray(2);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glDrawElements(GL_TRIANGLES, m->icount, GL_UNSIGNED_INT, 0);
}

void escrender(escene *s) {
    if (!s) return;
    ecamera *cam = s->camera;
    if (!cam) return;
    float lpos[3] = {5, 10, 5};
    float lcol[3] = {1, 1, 1};
    if (s->lightcount > 0) {
        lpos[0] = s->lights[0].pos[0];
        lpos[1] = s->lights[0].pos[1];
        lpos[2] = s->lights[0].pos[2];
        lcol[0] = s->lights[0].color[0] * s->lights[0].intensity;
        lcol[1] = s->lights[0].color[1] * s->lights[0].intensity;
        lcol[2] = s->lights[0].color[2] * s->lights[0].intensity;
    }
    emesh *m = s->meshes;
    while (m) {
        rendermesh(m, m->transform, m->material, cam, lpos, lcol);
        m = m->next;
    }
}

void escupdate(escene *s, float dt) {
    (void)s;
    (void)dt;
}

emesh *emeshcreate(void) {
    emesh *m = calloc(1, sizeof(emesh));
    glGenVertexArrays(1, &m->vao);
    glGenBuffers(1, &m->vbo);
    glGenBuffers(1, &m->ebo);
    return m;
}

emesh *emeshload(char *path) {
    (void)path;
    return emeshcreate();
}

void emeshsetvertices(emesh *m, float *verts, int count) {
    if (!m) return;
    m->verts = malloc(count * sizeof(float));
    memcpy(m->verts, verts, count * sizeof(float));
    m->vcount = count / 3;
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void emeshsetindices(emesh *m, int *inds, int count) {
    if (!m) return;
    m->inds = malloc(count * sizeof(int));
    memcpy(m->inds, inds, count * sizeof(int));
    m->icount = count;
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(int), inds, GL_STATIC_DRAW);
}

void emeshsetnormals(emesh *m, float *norms, int count) {
    if (!m) return;
    m->norms = malloc(count * sizeof(float));
    memcpy(m->norms, norms, count * sizeof(float));
    glGenBuffers(1, &m->nbo);
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->nbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), norms, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void emeshsetuvs(emesh *m, float *uvs, int count) {
    if (!m) return;
    m->uvs = malloc(count * sizeof(float));
    memcpy(m->uvs, uvs, count * sizeof(float));
    glGenBuffers(1, &m->uvbo);
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->uvbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void emeshsetbones(emesh *m, int *boneids, float *weights, int count) {
    if (!m) return;
    m->boneids = malloc(count * sizeof(int));
    memcpy(m->boneids, boneids, count * sizeof(int));
    m->boneweights = malloc(count * sizeof(float));
    memcpy(m->boneweights, weights, count * sizeof(float));
}

void emeshdestroy(emesh *m) {
    if (!m) return;
    glDeleteVertexArrays(1, &m->vao);
    glDeleteBuffers(1, &m->vbo);
    glDeleteBuffers(1, &m->ebo);
    free(m->verts);
    free(m->inds);
    free(m);
}

ematerial *ematcreate(void) {
    ematerial *m = calloc(1, sizeof(ematerial));
    m->diffuse[0] = 1;
    m->diffuse[1] = 1;
    m->diffuse[2] = 1;
    m->shininess = 32;
    return m;
}

void ematsetdiffuse(ematerial *m, float r, float g, float b) {
    if (m) {
        m->diffuse[0] = r;
        m->diffuse[1] = g;
        m->diffuse[2] = b;
    }
}

void ematsetspecular(ematerial *m, float r, float g, float b) {
    if (m) {
        m->specular[0] = r;
        m->specular[1] = g;
        m->specular[2] = b;
    }
}

void ematsetemissive(ematerial *m, float r, float g, float b) {
    if (m) {
        m->emissive[0] = r;
        m->emissive[1] = g;
        m->emissive[2] = b;
    }
}

void ematsetshader(ematerial *m, eshader *s) {
    if (m) m->shader = s;
}

void ematsettexture(ematerial *m, etexture *t) {
    if (m) m->texture = t;
}

void ematdestroy(ematerial *m) {
    free(m);
}

etexture *etextload(char *path) {
    (void)path;
    return NULL;
}

etexture *etextcreate(int w, int h, int ch) {
    etexture *t = calloc(1, sizeof(etexture));
    t->w = w;
    t->h = h;
    t->ch = ch;
    glGenTextures(1, &t->id);
    return t;
}

void etextdestroy(etexture *t) {
    if (!t) return;
    glDeleteTextures(1, &t->id);
    free(t);
}

eshader *eshadercreate(char *vert, char *frag) {
    eshader *s = malloc(sizeof(eshader));
    s->prog = makeprog(vert, frag);
    return s;
}

eshader *eshaderfromfile(char *vertpath, char *fragpath) {
    (void)vertpath;
    (void)fragpath;
    return NULL;
}

void eshaderuse(eshader *s) {
    if (s) glUseProgram(s->prog);
}

void eshadersetuniform(eshader *s, char *name, float *val, int type) {
    if (!s) return;
    int loc = glGetUniformLocation(s->prog, name);
    if (loc < 0) return;
    if (type == 0) glUniform1f(loc, val[0]);
    else if (type == 1) glUniform2fv(loc, 1, val);
    else if (type == 2) glUniform3fv(loc, 1, val);
    else if (type == 3) glUniform4fv(loc, 1, val);
}

void eshaderdestroy(eshader *s) {
    if (s) {
        glDeleteProgram(s->prog);
        free(s);
    }
}

ecamera *ecamcreate(float fov, float aspect, float near, float far) {
    ecamera *c = calloc(1, sizeof(ecamera));
    c->fov = fov;
    c->aspect = aspect;
    c->near = near;
    c->far = far;
    c->pos[0] = 0; c->pos[1] = 0; c->pos[2] = 5;
    c->target[0] = 0; c->target[1] = 0; c->target[2] = 0;
    c->up[0] = 0; c->up[1] = 1; c->up[2] = 0;
    return c;
}

void ecamsetposition(ecamera *c, float x, float y, float z) {
    if (c) { c->pos[0] = x; c->pos[1] = y; c->pos[2] = z; }
}

void ecamsettarget(ecamera *c, float x, float y, float z) {
    if (c) { c->target[0] = x; c->target[1] = y; c->target[2] = z; }
}

void ecamsetup(ecamera *c, float x, float y, float z) {
    if (c) { c->up[0] = x; c->up[1] = y; c->up[2] = z; }
}

void ecamsetortho(ecamera *c, float l, float r, float b, float t, float n, float f) {
    if (c) {
        c->ortho = 1;
        c->ortho_l = l; c->ortho_r = r;
        c->ortho_b = b; c->ortho_t = t;
        c->ortho_n = n; c->ortho_f = f;
    }
}

void ecamdestroy(ecamera *c) {
    free(c);
}

elight *elightcreate(int type) {
    elight *l = calloc(1, sizeof(elight));
    l->type = type;
    l->intensity = 1;
    l->color[0] = 1;
    l->color[1] = 1;
    l->color[2] = 1;
    return l;
}

void elightsetpos(elight *l, float x, float y, float z) {
    if (l) { l->pos[0] = x; l->pos[1] = y; l->pos[2] = z; }
}

void elightsetdir(elight *l, float x, float y, float z) {
    if (l) { l->dir[0] = x; l->dir[1] = y; l->dir[2] = z; }
}

void elightsetcolor(elight *l, float r, float g, float b) {
    if (l) { l->color[0] = r; l->color[1] = g; l->color[2] = b; }
}

void elightsetintensity(elight *l, float i) {
    if (l) l->intensity = i;
}

void elightdestroy(elight *l) {
    free(l);
}

etransform *etransformcreate(void) {
    etransform *t = calloc(1, sizeof(etransform));
    t->scale[0] = 1;
    t->scale[1] = 1;
    t->scale[2] = 1;
    return t;
}

void etransformsetpos(etransform *t, float x, float y, float z) {
    if (t) { t->pos[0] = x; t->pos[1] = y; t->pos[2] = z; }
}

void etransformsetrot(etransform *t, float x, float y, float z) {
    if (t) { t->rot[0] = x; t->rot[1] = y; t->rot[2] = z; }
}

void etransformsetscale(etransform *t, float x, float y, float z) {
    if (t) { t->scale[0] = x; t->scale[1] = y; t->scale[2] = z; }
}

void etransformtranslate(etransform *t, float x, float y, float z) {
    if (t) { t->pos[0] += x; t->pos[1] += y; t->pos[2] += z; }
}

void etransformrotate(etransform *t, float x, float y, float z) {
    if (t) { t->rot[0] += x; t->rot[1] += y; t->rot[2] += z; }
}

void etransformdestroy(etransform *t) {
    free(t);
}

void e3dinit(int w, int h) {
    defshader = makeprog(defv, deff);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, w, h);
}

void e3dresize(int w, int h) {
    glViewport(0, 0, w, h);
}

void e3dsetbackground(float r, float g, float b) {
    bgcol[0] = r;
    bgcol[1] = g;
    bgcol[2] = b;
}

void e3dsetwireframe(int on) {
    glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
}

void e3dcleanup(void) {
    glDeleteProgram(defshader);
}

void e3drenderframe(escene *s) {
    if (!s) return;
    curscene = s;
    glClearColor(bgcol[0], bgcol[1], bgcol[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    escrender(s);
}

object *esccreate_wrap(object **args, int argc, void *env) {
    (void)args; (void)argc; (void)env;
    return makenative(esccreate(), NULL);
}

object *escsetcamera_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 2) throwexception("setcamera needs scene and camera");
    escsetcamera((escene*)args[0]->native.data, (ecamera*)args[1]->native.data);
    return makenone();
}

object *escaddmesh_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("addmesh needs scene mesh transform material");
    escaddmesh((escene*)args[0]->native.data,
               (emesh*)args[1]->native.data,
               (etransform*)args[2]->native.data,
               (ematerial*)args[3]->native.data);
    return makenone();
}

object *escaddlight_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 2) throwexception("addlight needs scene light");
    escaddlight((escene*)args[0]->native.data, (elight*)args[1]->native.data);
    return makenone();
}

object *escclearmeshes_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 1) throwexception("clearmeshes needs scene");
    escclearmeshes((escene*)args[0]->native.data);
    return makenone();
}

object *escrender_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 1) throwexception("render needs scene");
    escrender((escene*)args[0]->native.data);
    return makenone();
}

object *escupdate_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 2) throwexception("update needs scene dt");
    escupdate((escene*)args[0]->native.data, args[1]->fval);
    return makenone();
}

object *emeshcreate_wrap(object **args, int argc, void *env) {
    (void)args; (void)argc; (void)env;
    return makenative(emeshcreate(), NULL);
}

object *emeshsetvertices_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 2) throwexception("meshsetvertices needs mesh and list");
    emesh *m = (emesh*)args[0]->native.data;
    object *lst = args[1];
    if (lst->type != 5) throwexception("vertices must be list");
    int count = lst->list.count;
    float *buf = malloc(count * sizeof(float));
    for (int i = 0; i < count; i++) {
        object *v = lst->list.items[i];
        buf[i] = v->type == 0 ? (float)v->ival : (float)v->fval;
    }
    emeshsetvertices(m, buf, count);
    free(buf);
    return makenone();
}

object *emeshsetindices_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 2) throwexception("meshsetindices needs mesh and list");
    emesh *m = (emesh*)args[0]->native.data;
    object *lst = args[1];
    if (lst->type != 5) throwexception("indices must be list");
    int count = lst->list.count;
    int *buf = malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) buf[i] = (int)lst->list.items[i]->ival;
    emeshsetindices(m, buf, count);
    free(buf);
    return makenone();
}

object *ematcreate_wrap(object **args, int argc, void *env) {
    (void)args; (void)argc; (void)env;
    return makenative(ematcreate(), NULL);
}

object *ematsetdiffuse_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("matsetdiffuse needs mat r g b");
    ematsetdiffuse((ematerial*)args[0]->native.data,
                   args[1]->fval, args[2]->fval, args[3]->fval);
    return makenone();
}

object *ecamcreate_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("camcreate needs fov aspect near far");
    return makenative(ecamcreate(args[0]->fval, args[1]->fval, args[2]->fval, args[3]->fval), NULL);
}

object *ecamsetposition_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("camsetposition needs cam x y z");
    ecamsetposition((ecamera*)args[0]->native.data, args[1]->fval, args[2]->fval, args[3]->fval);
    return makenone();
}

object *ecamsettarget_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("camsettarget needs cam x y z");
    ecamsettarget((ecamera*)args[0]->native.data, args[1]->fval, args[2]->fval, args[3]->fval);
    return makenone();
}

object *elightcreate_wrap(object **args, int argc, void *env) {
    (void)env;
    int type = (argc > 0) ? (int)args[0]->ival : 0;
    return makenative(elightcreate(type), NULL);
}

object *elightsetpos_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("lightsetpos needs light x y z");
    elightsetpos((elight*)args[0]->native.data, args[1]->fval, args[2]->fval, args[3]->fval);
    return makenone();
}

object *etransformcreate_wrap(object **args, int argc, void *env) {
    (void)args; (void)argc; (void)env;
    return makenative(etransformcreate(), NULL);
}

object *etransformsetpos_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("transsetpos needs trans x y z");
    etransformsetpos((etransform*)args[0]->native.data, args[1]->fval, args[2]->fval, args[3]->fval);
    return makenone();
}

object *etransformsetrot_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 4) throwexception("transsetrot needs trans x y z");
    etransformsetrot((etransform*)args[0]->native.data, args[1]->fval, args[2]->fval, args[3]->fval);
    return makenone();
}

object *e3dinit_wrap(object **args, int argc, void *env) {
    (void)env;
    int w = (argc > 0) ? (int)args[0]->ival : 800;
    int h = (argc > 1) ? (int)args[1]->ival : 600;
    e3dinit(w, h);
    return makenone();
}

object *e3dsetbackground_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 3) throwexception("setbackground needs r g b");
    e3dsetbackground(args[0]->fval, args[1]->fval, args[2]->fval);
    return makenone();
}

object *e3drenderframe_wrap(object **args, int argc, void *env) {
    (void)env;
    if (argc < 1) throwexception("renderframe needs scene");
    e3drenderframe((escene*)args[0]->native.data);
    return makenone();
}

void register3dlib(environment *env) {
    object *mod = makemodule("engine3d", NULL);
    environment *m = (environment*)mod->module.exports;
    envset(m, "createscene", makebuiltin(esccreate_wrap), 0);
    envset(m, "setcamera", makebuiltin(escsetcamera_wrap), 0);
    envset(m, "addmesh", makebuiltin(escaddmesh_wrap), 0);
    envset(m, "addlight", makebuiltin(escaddlight_wrap), 0);
    envset(m, "clearmeshes", makebuiltin(escclearmeshes_wrap), 0);
    envset(m, "render", makebuiltin(escrender_wrap), 0);
    envset(m, "update", makebuiltin(escupdate_wrap), 0);
    envset(m, "meshcreate", makebuiltin(emeshcreate_wrap), 0);
    envset(m, "meshsetvertices", makebuiltin(emeshsetvertices_wrap), 0);
    envset(m, "meshsetindices", makebuiltin(emeshsetindices_wrap), 0);
    envset(m, "matcreate", makebuiltin(ematcreate_wrap), 0);
    envset(m, "matsetdiffuse", makebuiltin(ematsetdiffuse_wrap), 0);
    envset(m, "camcreate", makebuiltin(ecamcreate_wrap), 0);
    envset(m, "camsetposition", makebuiltin(ecamsetposition_wrap), 0);
    envset(m, "camsettarget", makebuiltin(ecamsettarget_wrap), 0);
    envset(m, "lightcreate", makebuiltin(elightcreate_wrap), 0);
    envset(m, "lightsetpos", makebuiltin(elightsetpos_wrap), 0);
    envset(m, "transcreate", makebuiltin(etransformcreate_wrap), 0);
    envset(m, "transsetpos", makebuiltin(etransformsetpos_wrap), 0);
    envset(m, "transsetrot", makebuiltin(etransformsetrot_wrap), 0);
    envset(m, "init", makebuiltin(e3dinit_wrap), 0);
    envset(m, "setbackground", makebuiltin(e3dsetbackground_wrap), 0);
    envset(m, "renderframe", makebuiltin(e3drenderframe_wrap), 0);
    envset(env, "engine3d", mod, 0);
}
