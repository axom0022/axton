#include "../core/axton.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#endif

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

typedef struct erigidbody {
    float mass;
    float velocity[3];
    float force[3];
    float gravity;
    int collider_type;
    float collider_params[6];
    struct etransform *transform;
} erigidbody;

typedef struct eparticle {
    float pos[3], vel[3], color[3];
    float life, maxlife, size;
    int active;
} eparticle;

typedef struct eparticlesys {
    eparticle particles[maxparticles];
    int count;
    float emitter[3];
    float rate;
    float lifetime;
    float speed;
    float color[3];
    float size;
    float timer;
} eparticlesys;

typedef struct ebone {
    char name[64];
    int parent;
    float localpos[3], localrot[3], localscale[3];
    float worldmatrix[16];
} ebone;

typedef struct eanimation {
    ebone bones[maxbones];
    int bonecount;
    struct {
        float time;
        float pos[3], rot[3], scale[3];
    } keyframes[maxbones][maxkeyframes];
    int keycount[maxbones];
    float duration;
    float time;
    float speed;
    int playing;
    char name[64];
} eanimation;

typedef struct eterrain {
    int width, depth, segments;
    float *heightmap;
    etexture *texture;
    unsigned int vao, vbo;
    int vcount;
} eterrain;

typedef struct escene {
    emesh *meshes;
    ematerial *materials;
    elight lights[maxlights];
    int lightcount;
    ecamera *camera;
    erigidbody *rigidbodies;
    eparticlesys *particles;
    eanimation *animations;
    eterrain *terrain;
    int shadowmap;
    int postprocess;
    unsigned int shadowfbo, shadowtex;
    unsigned int postfbo, posttex;
    int width, height;
} escene;

static escene *curscene = NULL;
static unsigned int defshader = 0;
static unsigned int shadowshader = 0;
static unsigned int postshader = 0;
static int shadowsize = 2048;
static float bgcol[3] = {0.2,0.2,0.3};

static unsigned int compileshader(const char *src, int type) {
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char info[512]; glGetShaderInfoLog(s, 512, NULL, info); throwexception(info); }
    return s;
}

static unsigned int makeprog(const char *v, const char *f) {
    unsigned int vs = compileshader(v, GL_VERTEX_SHADER);
    unsigned int fs = compileshader(f, GL_FRAGMENT_SHADER);
    unsigned int p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs);
    glLinkProgram(p);
    int ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) { char info[512]; glGetProgramInfoLog(p, 512, NULL, info); throwexception(info); }
    return p;
}

static const char *defv = "#version 330 core\nlayout(location=0)in vec3 p;layout(location=1)in vec3 n;layout(location=2)in vec2 u;uniform mat4 m,v,pr;uniform vec3 lp;out vec3 fn, fp, fu;void main(){fp=vec3(m*vec4(p,1));fn=mat3(transpose(inverse(m)))*n;fu=vec2(u);gl_Position=pr*v*m*vec4(p,1);}";
static const char *deff = "#version 330 core\nin vec3 fn,fp,fu;uniform vec3 lp,lc,vp,diff;uniform sampler2D tex;uniform int ht;out vec4 oc;void main(){vec3 norm=normalize(fn);vec3 ld=normalize(lp-fp);float diffa=max(dot(norm,ld),0.0);vec3 vd=normalize(vp-fp);vec3 rd=reflect(-ld,norm);float spec=pow(max(dot(vd,rd),0.0),32.0);vec3 amb=0.05*diff;vec3 col=amb+diffa*lc*diff+spec*lc;if(ht>0)col*=texture(tex,fu).rgb;oc=vec4(col,1);}";
static const char *shv = "#version 330 core\nlayout(location=0)in vec3 p;uniform mat4 m,ls;void main(){gl_Position=ls*m*vec4(p,1);}";
static const char *shf = "#version 330 core\nvoid main(){}";
static const char *pov = "#version 330 core\nlayout(location=0)in vec2 p;out vec2 u;void main(){gl_Position=vec4(p,0,1);u=p*0.5+0.5;}";
static const char *pof = "#version 330 core\nin vec2 u;uniform sampler2D screen;out vec4 oc;void main(){oc=texture(screen,u);vec4 b=texture(screen,u);float g=dot(b.rgb,vec3(0.299,0.587,0.114));if(g>0.8)oc+=b*0.3;}";

static unsigned int mkdefshader(void) { return makeprog(defv, deff); }
static unsigned int mkshadowshader(void) { return makeprog(shv, shf); }
static unsigned int mkpostshader(void) { return makeprog(pov, pof); }

static void matid(float *m) { for(int i=0;i<16;i++)m[i]=0; m[0]=m[5]=m[10]=m[15]=1; }
static void mattrans(float *m,float x,float y,float z){ m[12]+=x; m[13]+=y; m[14]+=z; }
static void matrot(float *m,float a,float x,float y,float z){ float r=a*3.14159f/180.0f;float c=cos(r),s=sin(r),t=1-c;float o[16];matid(o);o[0]=t*x*x+c;o[1]=t*x*y-s*z;o[2]=t*x*z+s*y;o[4]=t*x*y+s*z;o[5]=t*y*y+c;o[6]=t*y*z-s*x;o[8]=t*x*z-s*y;o[9]=t*y*z+s*x;o[10]=t*z*z+c;float res[16];for(int i=0;i<4;i++)for(int j=0;j<4;j++){float sum=0;for(int k=0;k<4;k++)sum+=m[i*4+k]*o[k*4+j];res[i*4+j]=sum;}memcpy(m,res,16*sizeof(float));}
static void matscale(float *m,float x,float y,float z){ m[0]*=x; m[5]*=y; m[10]*=z; }
static void matmul(float *a,float *b,float *out){ for(int i=0;i<4;i++)for(int j=0;j<4;j++){float sum=0;for(int k=0;k<4;k++)sum+=a[i*4+k]*b[k*4+j];out[i*4+j]=sum;} }

static void gettfm(etransform *t,float *out){ matid(out); mattrans(out,t->pos[0],t->pos[1],t->pos[2]); matrot(out,t->rot[0],1,0,0); matrot(out,t->rot[1],0,1,0); matrot(out,t->rot[2],0,0,1); matscale(out,t->scale[0],t->scale[1],t->scale[2]); }

static void rendermesh(emesh *m,etransform *t,ematerial *mat,ecamera *cam,float *lp,float *lc){
    if(!m||!t)return;
    float model[16],view[16],proj[16],mvp[16];
    gettfm(t,model);
    matid(view); mattrans(view,-cam->pos[0],-cam->pos[1],-cam->pos[2]);
    matid(proj);
    if(cam->ortho){ float l=cam->ortho_l,r=cam->ortho_r,b=cam->ortho_b,tc=cam->ortho_t,n=cam->ortho_n,f=cam->ortho_f; proj[0]=2/(r-l); proj[5]=2/(tc-b); proj[10]=-2/(f-n); proj[12]=-(r+l)/(r-l); proj[13]=-(tc+b)/(tc-b); proj[14]=-(f+n)/(f-n); }
    else { float fovr=cam->fov*3.14159f/180.0f; float th=tan(fovr/2); proj[0]=1/(th*cam->aspect); proj[5]=1/th; proj[10]=-(cam->far+cam->near)/(cam->far-cam->near); proj[14]=-2*cam->far*cam->near/(cam->far-cam->near); proj[11]=-1; }
    matmul(view,model,mvp); matmul(proj,mvp,mvp);
    unsigned int prog = mat->shader ? mat->shader->prog : defshader;
    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog,"m"),1,GL_FALSE,model);
    glUniformMatrix4fv(glGetUniformLocation(prog,"v"),1,GL_FALSE,view);
    glUniformMatrix4fv(glGetUniformLocation(prog,"pr"),1,GL_FALSE,proj);
    glUniform3fv(glGetUniformLocation(prog,"lp"),1,lp);
    glUniform3fv(glGetUniformLocation(prog,"lc"),1,lc);
    glUniform3fv(glGetUniformLocation(prog,"diff"),1,mat->diffuse);
    glUniform1i(glGetUniformLocation(prog,"ht"),mat->texture?1:0);
    if(mat->texture){ glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,mat->texture->id); }
    glBindVertexArray(m->vao);
    glDrawElements(GL_TRIANGLES,m->icount,GL_UNSIGNED_INT,0);
}

escene *esccreate(void) {
    escene *s = malloc(sizeof(escene));
    memset(s,0,sizeof(escene));
    s->lightcount=0;
    s->shadowmap=0;
    s->postprocess=0;
    s->width=800; s->height=600;
    return s;
}

void escsetcamera(escene *s, ecamera *c) { if(s) s->camera = c; }

void escaddmesh(escene *s, emesh *m, etransform *t, ematerial *mat) {
    if(!s||!m)return;
    m->next = s->meshes;
    s->meshes = m;
}

void escaddlight(escene *s, elight *l) { if(!s||s->lightcount>=maxlights)return; memcpy(&s->lights[s->lightcount++],l,sizeof(elight)); }

void escaddparticle(escene *s, eparticle *p) { if(s) s->particles = (eparticlesys*)p; }

void escaddrigidbody(escene *s, erigidbody *r) { if(s) s->rigidbodies = r; }

void escaddeanimation(escene *s, eanimation *a) { if(s) s->animations = a; }

void escclearmeshes(escene *s) { if(s) s->meshes = NULL; }

void escsetterrain(escene *s, eterrain *t) { if(s) s->terrain = t; }

void escrender(escene *s) {
    if(!s)return;
    ecamera *cam = s->camera;
    if(!cam)return;
    float lpos[3]={5,10,5}, lcol[3]={1,1,1};
    if(s->lightcount>0){ lpos[0]=s->lights[0].pos[0]; lpos[1]=s->lights[0].pos[1]; lpos[2]=s->lights[0].pos[2]; lcol[0]=s->lights[0].color[0]*s->lights[0].intensity; lcol[1]=s->lights[0].color[1]*s->lights[0].intensity; lcol[2]=s->lights[0].color[2]*s->lights[0].intensity; }
    emesh *m = s->meshes;
    while(m){ rendermesh(m, NULL, NULL, cam, lpos, lcol); m = m->next; }
}

void escupdate(escene *s, float dt) { (void)s; (void)dt; }

emesh *emeshcreate(void) {
    emesh *m = malloc(sizeof(emesh));
    memset(m,0,sizeof(emesh));
    glGenVertexArrays(1,&m->vao);
    glGenBuffers(1,&m->vbo);
    glGenBuffers(1,&m->ebo);
    return m;
}

emesh *emeshload(char *path) { (void)path; return emeshcreate(); }

void emeshsetvertices(emesh *m, float *verts, int count) { if(!m)return; m->verts = malloc(count*sizeof(float)); memcpy(m->verts, verts, count*sizeof(float)); m->vcount = count/3; glBindBuffer(GL_ARRAY_BUFFER, m->vbo); glBufferData(GL_ARRAY_BUFFER, count*sizeof(float), verts, GL_STATIC_DRAW); }

void emeshsetindices(emesh *m, int *inds, int count) { if(!m)return; m->inds = malloc(count*sizeof(int)); memcpy(m->inds, inds, count*sizeof(int)); m->icount = count; glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo); glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*sizeof(int), inds, GL_STATIC_DRAW); }

void emeshsetnormals(emesh *m, float *norms, int count) { if(!m)return; m->norms = malloc(count*sizeof(float)); memcpy(m->norms, norms, count*sizeof(float)); glGenBuffers(1,&m->nbo); glBindBuffer(GL_ARRAY_BUFFER, m->nbo); glBufferData(GL_ARRAY_BUFFER, count*sizeof(float), norms, GL_STATIC_DRAW); }

void emeshsetuvs(emesh *m, float *uvs, int count) { if(!m)return; m->uvs = malloc(count*sizeof(float)); memcpy(m->uvs, uvs, count*sizeof(float)); glGenBuffers(1,&m->uvbo); glBindBuffer(GL_ARRAY_BUFFER, m->uvbo); glBufferData(GL_ARRAY_BUFFER, count*sizeof(float), uvs, GL_STATIC_DRAW); }

void emeshsetbones(emesh *m, int *boneids, float *weights, int count) { if(!m)return; m->boneids = malloc(count*sizeof(int)); memcpy(m->boneids, boneids, count*sizeof(int)); m->boneweights = malloc(count*sizeof(float)); memcpy(m->boneweights, weights, count*sizeof(float)); }

void emeshdestroy(emesh *m) { if(!m)return; glDeleteVertexArrays(1,&m->vao); glDeleteBuffers(1,&m->vbo); glDeleteBuffers(1,&m->ebo); free(m->verts); free(m->inds); free(m); }

ematerial *ematcreate(void) {
    ematerial *m = malloc(sizeof(ematerial));
    memset(m,0,sizeof(ematerial));
    m->diffuse[0]=1; m->diffuse[1]=1; m->diffuse[2]=1;
    m->shininess=32;
    return m;
}

void ematsetdiffuse(ematerial *m, float r, float g, float b) { if(m){ m->diffuse[0]=r; m->diffuse[1]=g; m->diffuse[2]=b; } }
void ematsetspecular(ematerial *m, float r, float g, float b) { if(m){ m->specular[0]=r; m->specular[1]=g; m->specular[2]=b; } }
void ematsetemissive(ematerial *m, float r, float g, float b) { if(m){ m->emissive[0]=r; m->emissive[1]=g; m->emissive[2]=b; } }
void ematsetshader(ematerial *m, eshader *s) { if(m) m->shader = s; }
void ematsettexture(ematerial *m, etexture *t) { if(m) m->texture = t; }
void ematdestroy(ematerial *m) { free(m); }

etexture *etextload(char *path) { (void)path; return NULL; }
etexture *etextcreate(int w, int h, int ch) { (void)w;(void)h;(void)ch; return NULL; }
void etextdestroy(etexture *t) { free(t); }

eshader *eshadercreate(char *vert, char *frag) {
    eshader *s = malloc(sizeof(eshader));
    s->prog = makeprog(vert, frag);
    return s;
}
eshader *eshaderfromfile(char *vertpath, char *fragpath) { (void)vertpath;(void)fragpath; return NULL; }
void eshaderuse(eshader *s) { if(s) glUseProgram(s->prog); }
void eshadersetuniform(eshader *s, char *name, float *val, int type) { (void)s;(void)name;(void)val;(void)type; }
void eshaderdestroy(eshader *s) { if(s){ glDeleteProgram(s->prog); free(s); } }

ecamera *ecamcreate(float fov, float aspect, float near, float far) {
    ecamera *c = malloc(sizeof(ecamera));
    memset(c,0,sizeof(ecamera));
    c->fov=fov; c->aspect=aspect; c->near=near; c->far=far;
    c->pos[0]=0; c->pos[1]=0; c->pos[2]=5;
    c->target[0]=0; c->target[1]=0; c->target[2]=0;
    c->up[0]=0; c->up[1]=1; c->up[2]=0;
    c->ortho=0;
    return c;
}
void ecamsetposition(ecamera *c, float x, float y, float z) { if(c){ c->pos[0]=x; c->pos[1]=y; c->pos[2]=z; } }
void ecamsettarget(ecamera *c, float x, float y, float z) { if(c){ c->target[0]=x; c->target[1]=y; c->target[2]=z; } }
void ecamsetup(ecamera *c, float x, float y, float z) { if(c){ c->up[0]=x; c->up[1]=y; c->up[2]=z; } }
void ecamsetortho(ecamera *c, float l, float r, float b, float t, float n, float f) { if(c){ c->ortho=1; c->ortho_l=l; c->ortho_r=r; c->ortho_b=b; c->ortho_t=t; c->ortho_n=n; c->ortho_f=f; } }
void ecamdestroy(ecamera *c) { free(c); }

elight *elightcreate(int type) {
    elight *l = malloc(sizeof(elight));
    memset(l,0,sizeof(elight));
    l->type = type;
    l->intensity = 1;
    l->color[0]=1; l->color[1]=1; l->color[2]=1;
    return l;
}
void elightsetpos(elight *l, float x, float y, float z) { if(l){ l->pos[0]=x; l->pos[1]=y; l->pos[2]=z; } }
void elightsetdir(elight *l, float x, float y, float z) { if(l){ l->dir[0]=x; l->dir[1]=y; l->dir[2]=z; } }
void elightsetcolor(elight *l, float r, float g, float b) { if(l){ l->color[0]=r; l->color[1]=g; l->color[2]=b; } }
void elightsetintensity(elight *l, float i) { if(l) l->intensity = i; }
void elightdestroy(elight *l) { free(l); }

etransform *etransformcreate(void) {
    etransform *t = malloc(sizeof(etransform));
    memset(t,0,sizeof(etransform));
    t->scale[0]=1; t->scale[1]=1; t->scale[2]=1;
    return t;
}
void etransformsetpos(etransform *t, float x, float y, float z) { if(t){ t->pos[0]=x; t->pos[1]=y; t->pos[2]=z; } }
void etransformsetrot(etransform *t, float x, float y, float z) { if(t){ t->rot[0]=x; t->rot[1]=y; t->rot[2]=z; } }
void etransformsetscale(etransform *t, float x, float y, float z) { if(t){ t->scale[0]=x; t->scale[1]=y; t->scale[2]=z; } }
void etransformtranslate(etransform *t, float x, float y, float z) { if(t){ t->pos[0]+=x; t->pos[1]+=y; t->pos[2]+=z; } }
void etransformrotate(etransform *t, float x, float y, float z) { if(t){ t->rot[0]+=x; t->rot[1]+=y; t->rot[2]+=z; } }
void etransformdestroy(etransform *t) { free(t); }

erigidbody *erbcreate(void) {
    erigidbody *r = malloc(sizeof(erigidbody));
    memset(r,0,sizeof(erigidbody));
    r->mass = 1;
    r->gravity = -9.8;
    return r;
}
void erbsetmass(erigidbody *r, float mass) { if(r) r->mass = mass; }
void erbapplyforce(erigidbody *r, float fx, float fy, float fz) { if(r){ r->force[0]+=fx; r->force[1]+=fy; r->force[2]+=fz; } }
void erbapplyimpulse(erigidbody *r, float ix, float iy, float iz) { if(r){ r->velocity[0]+=ix/r->mass; r->velocity[1]+=iy/r->mass; r->velocity[2]+=iz/r->mass; } }
void erbsetvelocity(erigidbody *r, float vx, float vy, float vz) { if(r){ r->velocity[0]=vx; r->velocity[1]=vy; r->velocity[2]=vz; } }
void erbsetgravity(erigidbody *r, float g) { if(r) r->gravity = g; }
void erbaddcollider(erigidbody *r, int type, float *params) { if(r){ r->collider_type = type; for(int i=0;i<6;i++)r->collider_params[i]=params[i]; } }
void erbdestroy(erigidbody *r) { free(r); }

eparticle *epcreate(void) { eparticle *p = malloc(sizeof(eparticle)); memset(p,0,sizeof(eparticle)); return p; }
void epsetemitter(eparticle *p, float x, float y, float z) { if(p){ p->pos[0]=x; p->pos[1]=y; p->pos[2]=z; } }
void epsetrate(eparticle *p, float rate) { (void)p;(void)rate; }
void epsetlifetime(eparticle *p, float life) { if(p) p->maxlife = life; }
void epsetspeed(eparticle *p, float speed) { (void)p;(void)speed; }
void epsetcolor(eparticle *p, float r, float g, float b) { if(p){ p->color[0]=r; p->color[1]=g; p->color[2]=b; } }
void epsetsize(eparticle *p, float size) { if(p) p->size = size; }
void epupdate(eparticle *p, float dt) { (void)p;(void)dt; }
void epdestroy(eparticle *p) { free(p); }

eanimation *eanimcreate(void) { eanimation *a = malloc(sizeof(eanimation)); memset(a,0,sizeof(eanimation)); return a; }
void eanimaddbone(eanimation *a, char *name, int parent) { if(!a||a->bonecount>=maxbones)return; strcpy(a->bones[a->bonecount].name,name); a->bones[a->bonecount].parent=parent; a->bonecount++; }
void eanimsetkeyframe(eanimation *a, int bone, float time, float *pos, float *rot, float *scale) { if(!a)return; int idx=a->keycount[bone]; if(idx>=maxkeyframes)return; a->keyframes[bone][idx].time=time; if(pos){ a->keyframes[bone][idx].pos[0]=pos[0]; a->keyframes[bone][idx].pos[1]=pos[1]; a->keyframes[bone][idx].pos[2]=pos[2]; } if(rot){ a->keyframes[bone][idx].rot[0]=rot[0]; a->keyframes[bone][idx].rot[1]=rot[1]; a->keyframes[bone][idx].rot[2]=rot[2]; } if(scale){ a->keyframes[bone][idx].scale[0]=scale[0]; a->keyframes[bone][idx].scale[1]=scale[1]; a->keyframes[bone][idx].scale[2]=scale[2]; } a->keycount[bone]++; }
void eanimplay(eanimation *a, char *name, float speed) { if(a){ strcpy(a->name,name); a->speed=speed; a->playing=1; a->time=0; } }
void eanimupdate(eanimation *a, float dt) { if(!a||!a->playing)return; a->time += dt * a->speed; if(a->time > a->duration) a->time = 0; }
void eanimdestroy(eanimation *a) { free(a); }

eterrain *etcreate(int width, int depth, int segments) { eterrain *t=malloc(sizeof(eterrain)); t->width=width; t->depth=depth; t->segments=segments; return t; }
void etsetheight(eterrain *t, int x, int z, float h) { if(t) t->heightmap[x+z*t->width] = h; }
void etsettexture(eterrain *t, etexture *tex) { if(t) t->texture = tex; }
void etsetheightmap(eterrain *t, char *path) { (void)t;(void)path; }
void etdestroy(eterrain *t) { free(t->heightmap); free(t); }

void e3dinit(int w, int h) {
    defshader = mkdefshader();
    shadowshader = mkshadowshader();
    postshader = mkpostshader();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0,0,w,h);
}

void e3dresize(int w, int h) { glViewport(0,0,w,h); }

void e3dsetbackground(float r, float g, float b) { bgcol[0]=r; bgcol[1]=g; bgcol[2]=b; }

void e3dsetwireframe(int on) { glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL); }

void e3dsetshadowmap(int on) { if(curscene) curscene->shadowmap = on; }

void e3dsetpostprocess(int on) { if(curscene) curscene->postprocess = on; }

void e3dcleanup(void) { glDeleteProgram(defshader); glDeleteProgram(shadowshader); glDeleteProgram(postshader); }

void e3drenderframe(escene *s) {
    if(!s)return;
    curscene = s;
    glClearColor(bgcol[0],bgcol[1],bgcol[2],1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    escrender(s);
}

void register3dlib(environment *env) {
    object *mod = makemodule("engine3d", NULL);
    envset(mod->module.exports, "createscene", makebuiltin(esccreate), 0);
    envset(mod->module.exports, "setcamera", makebuiltin(escsetcamera), 0);
    envset(mod->module.exports, "addmesh", makebuiltin(escaddmesh), 0);
    envset(mod->module.exports, "addlight", makebuiltin(escaddlight), 0);
    envset(mod->module.exports, "addparticle", makebuiltin(escaddparticle), 0);
    envset(mod->module.exports, "addrigidbody", makebuiltin(escaddrigidbody), 0);
    envset(mod->module.exports, "addanimation", makebuiltin(escaddeanimation), 0);
    envset(mod->module.exports, "clearmeshes", makebuiltin(escclearmeshes), 0);
    envset(mod->module.exports, "setterrain", makebuiltin(escsetterrain), 0);
    envset(mod->module.exports, "render", makebuiltin(escrender), 0);
    envset(mod->module.exports, "update", makebuiltin(escupdate), 0);
    envset(mod->module.exports, "meshcreate", makebuiltin(emeshcreate), 0);
    envset(mod->module.exports, "meshload", makebuiltin(emeshload), 0);
    envset(mod->module.exports, "meshsetvertices", makebuiltin(emeshsetvertices), 0);
    envset(mod->module.exports, "meshsetindices", makebuiltin(emeshsetindices), 0);
    envset(mod->module.exports, "meshsetnormals", makebuiltin(emeshsetnormals), 0);
    envset(mod->module.exports, "meshsetuvs", makebuiltin(emeshsetuvs), 0);
    envset(mod->module.exports, "meshsetbones", makebuiltin(emeshsetbones), 0);
    envset(mod->module.exports, "meshdestroy", makebuiltin(emeshdestroy), 0);
    envset(mod->module.exports, "matcreate", makebuiltin(ematcreate), 0);
    envset(mod->module.exports, "matsetdiffuse", makebuiltin(ematsetdiffuse), 0);
    envset(mod->module.exports, "matsetspecular", makebuiltin(ematsetspecular), 0);
    envset(mod->module.exports, "matsetemissive", makebuiltin(ematsetemissive), 0);
    envset(mod->module.exports, "matsetshader", makebuiltin(ematsetshader), 0);
    envset(mod->module.exports, "matsettexture", makebuiltin(ematsettexture), 0);
    envset(mod->module.exports, "matdestroy", makebuiltin(ematdestroy), 0);
    envset(mod->module.exports, "textload", makebuiltin(etextload), 0);
    envset(mod->module.exports, "textcreate", makebuiltin(etextcreate), 0);
    envset(mod->module.exports, "textdestroy", makebuiltin(etextdestroy), 0);
    envset(mod->module.exports, "shadercreate", makebuiltin(eshadercreate), 0);
    envset(mod->module.exports, "shaderfromfile", makebuiltin(eshaderfromfile), 0);
    envset(mod->module.exports, "shaderuse", makebuiltin(eshaderuse), 0);
    envset(mod->module.exports, "shadersetuniform", makebuiltin(eshadersetuniform), 0);
    envset(mod->module.exports, "shaderdestroy", makebuiltin(eshaderdestroy), 0);
    envset(mod->module.exports, "camcreate", makebuiltin(ecamcreate), 0);
    envset(mod->module.exports, "camsetposition", makebuiltin(ecamsetposition), 0);
    envset(mod->module.exports, "camsettarget", makebuiltin(ecamsettarget), 0);
    envset(mod->module.exports, "camsetup", makebuiltin(ecamsetup), 0);
    envset(mod->module.exports, "camsetortho", makebuiltin(ecamsetortho), 0);
    envset(mod->module.exports, "camdestroy", makebuiltin(ecamdestroy), 0);
    envset(mod->module.exports, "lightcreate", makebuiltin(elightcreate), 0);
    envset(mod->module.exports, "lightsetpos", makebuiltin(elightsetpos), 0);
    envset(mod->module.exports, "lightsetdir", makebuiltin(elightsetdir), 0);
    envset(mod->module.exports, "lightsetcolor", makebuiltin(elightsetcolor), 0);
    envset(mod->module.exports, "lightsetintensity", makebuiltin(elightsetintensity), 0);
    envset(mod->module.exports, "lightdestroy", makebuiltin(elightdestroy), 0);
    envset(mod->module.exports, "transcreate", makebuiltin(etransformcreate), 0);
    envset(mod->module.exports, "transsetpos", makebuiltin(etransformsetpos), 0);
    envset(mod->module.exports, "transsetrot", makebuiltin(etransformsetrot), 0);
    envset(mod->module.exports, "transsetscale", makebuiltin(etransformsetscale), 0);
    envset(mod->module.exports, "transtranslate", makebuiltin(etransformtranslate), 0);
    envset(mod->module.exports, "transrotate", makebuiltin(etransformrotate), 0);
    envset(mod->module.exports, "transdestroy", makebuiltin(etransformdestroy), 0);
    envset(mod->module.exports, "rbcreate", makebuiltin(erbcreate), 0);
    envset(mod->module.exports, "rbsetmass", makebuiltin(erbsetmass), 0);
    envset(mod->module.exports, "rbapplyforce", makebuiltin(erbapplyforce), 0);
    envset(mod->module.exports, "rbapplyimpulse", makebuiltin(erbapplyimpulse), 0);
    envset(mod->module.exports, "rbsetvelocity", makebuiltin(erbsetvelocity), 0);
    envset(mod->module.exports, "rbsetgravity", makebuiltin(erbsetgravity), 0);
    envset(mod->module.exports, "rbaddcollider", makebuiltin(erbaddcollider), 0);
    envset(mod->module.exports, "rbdestroy", makebuiltin(erbdestroy), 0);
    envset(mod->module.exports, "partcreate", makebuiltin(epcreate), 0);
    envset(mod->module.exports, "partsetemitter", makebuiltin(epsetemitter), 0);
    envset(mod->module.exports, "partsetrate", makebuiltin(epsetrate), 0);
    envset(mod->module.exports, "partsetlifetime", makebuiltin(epsetlifetime), 0);
    envset(mod->module.exports, "partsetspeed", makebuiltin(epsetspeed), 0);
    envset(mod->module.exports, "partsetcolor", makebuiltin(epsetcolor), 0);
    envset(mod->module.exports, "partsetsize", makebuiltin(epsetsize), 0);
    envset(mod->module.exports, "partupdate", makebuiltin(epupdate), 0);
    envset(mod->module.exports, "partdestroy", makebuiltin(epdestroy), 0);
    envset(mod->module.exports, "animcreate", makebuiltin(eanimcreate), 0);
    envset(mod->module.exports, "animaddbone", makebuiltin(eanimaddbone), 0);
    envset(mod->module.exports, "animsetkeyframe", makebuiltin(eanimsetkeyframe), 0);
    envset(mod->module.exports, "animplay", makebuiltin(eanimplay), 0);
    envset(mod->module.exports, "animupdate", makebuiltin(eanimupdate), 0);
    envset(mod->module.exports, "animdestroy", makebuiltin(eanimdestroy), 0);
    envset(mod->module.exports, "tercreate", makebuiltin(etcreate), 0);
    envset(mod->module.exports, "tersetheight", makebuiltin(etsetheight), 0);
    envset(mod->module.exports, "tersettexture", makebuiltin(etsettexture), 0);
    envset(mod->module.exports, "tersetheightmap", makebuiltin(etsetheightmap), 0);
    envset(mod->module.exports, "terdestroy", makebuiltin(etdestroy), 0);
    envset(mod->module.exports, "init", makebuiltin(e3dinit), 0);
    envset(mod->module.exports, "resize", makebuiltin(e3dresize), 0);
    envset(mod->module.exports, "setbackground", makebuiltin(e3dsetbackground), 0);
    envset(mod->module.exports, "setwireframe", makebuiltin(e3dsetwireframe), 0);
    envset(mod->module.exports, "setshadowmap", makebuiltin(e3dsetshadowmap), 0);
    envset(mod->module.exports, "setpostprocess", makebuiltin(e3dsetpostprocess), 0);
    envset(mod->module.exports, "cleanup", makebuiltin(e3dcleanup), 0);
    envset(mod->module.exports, "renderframe", makebuiltin(e3drenderframe), 0);
    envset(env, "engine3d", mod, 0);
}
