#ifndef engine3d_h
#define engine3d_h

typedef struct escene escene;
typedef struct emesh emesh;
typedef struct ematerial ematerial;
typedef struct etexture etexture;
typedef struct eshader eshader;
typedef struct ecamera ecamera;
typedef struct elight elight;
typedef struct etransform etransform;
typedef struct erigidbody erigidbody;
typedef struct eparticle eparticle;
typedef struct eanimation eanimation;
typedef struct eterrain eterrain;

escene *esccreate(void);
void escsetcamera(escene *s, ecamera *c);
void escaddmesh(escene *s, emesh *m, etransform *t, ematerial *mat);
void escaddlight(escene *s, elight *l);
void escaddparticle(escene *s, eparticle *p);
void escaddrigidbody(escene *s, erigidbody *r);
void escaddeanimation(escene *s, eanimation *a);
void escclearmeshes(escene *s);
void escsetterrain(escene *s, eterrain *t);
void escrender(escene *s);
void escupdate(escene *s, float dt);

emesh *emeshcreate(void);
emesh *emeshload(char *path);
void emeshsetvertices(emesh *m, float *verts, int count);
void emeshsetindices(emesh *m, int *inds, int count);
void emeshsetnormals(emesh *m, float *norms, int count);
void emeshsetuvs(emesh *m, float *uvs, int count);
void emeshsetbones(emesh *m, int *boneids, float *weights, int count);
void emeshdestroy(emesh *m);

ematerial *ematcreate(void);
void ematsetdiffuse(ematerial *m, float r, float g, float b);
void ematsetspecular(ematerial *m, float r, float g, float b);
void ematsetemissive(ematerial *m, float r, float g, float b);
void ematsetshader(ematerial *m, eshader *s);
void ematsettexture(ematerial *m, etexture *t);
void ematdestroy(ematerial *m);

etexture *etextload(char *path);
etexture *etextcreate(int w, int h, int channels);
void etextdestroy(etexture *t);

eshader *eshadercreate(char *vert, char *frag);
eshader *eshaderfromfile(char *vertpath, char *fragpath);
void eshaderuse(eshader *s);
void eshadersetuniform(eshader *s, char *name, float *val, int type);
void eshaderdestroy(eshader *s);

ecamera *ecamcreate(float fov, float aspect, float near, float far);
void ecamsetposition(ecamera *c, float x, float y, float z);
void ecamsettarget(ecamera *c, float x, float y, float z);
void ecamsetup(ecamera *c, float x, float y, float z);
void ecamsetortho(ecamera *c, float l, float r, float b, float t, float n, float f);
void ecamdestroy(ecamera *c);

elight *elightcreate(int type);
void elightsetpos(elight *l, float x, float y, float z);
void elightsetdir(elight *l, float x, float y, float z);
void elightsetcolor(elight *l, float r, float g, float b);
void elightsetintensity(elight *l, float i);
void elightdestroy(elight *l);

etransform *etransformcreate(void);
void etransformsetpos(etransform *t, float x, float y, float z);
void etransformsetrot(etransform *t, float x, float y, float z);
void etransformsetscale(etransform *t, float x, float y, float z);
void etransformtranslate(etransform *t, float x, float y, float z);
void etransformrotate(etransform *t, float x, float y, float z);
void etransformdestroy(etransform *t);

erigidbody *erbcreate(void);
void erbsetmass(erigidbody *r, float mass);
void erbapplyforce(erigidbody *r, float fx, float fy, float fz);
void erbapplyimpulse(erigidbody *r, float ix, float iy, float iz);
void erbsetvelocity(erigidbody *r, float vx, float vy, float vz);
void erbsetgravity(erigidbody *r, float g);
void erbaddcollider(erigidbody *r, int type, float *params);
void erbdestroy(erigidbody *r);

eparticle *epcreate(void);
void epsetemitter(eparticle *p, float x, float y, float z);
void epsetrate(eparticle *p, float rate);
void epsetlifetime(eparticle *p, float life);
void epsetspeed(eparticle *p, float speed);
void epsetcolor(eparticle *p, float r, float g, float b);
void epsetsize(eparticle *p, float size);
void epupdate(eparticle *p, float dt);
void epdestroy(eparticle *p);

eanimation *eanimcreate(void);
void eanimaddbone(eanimation *a, char *name, int parent);
void eanimsetkeyframe(eanimation *a, int bone, float time, float *pos, float *rot, float *scale);
void eanimplay(eanimation *a, char *name, float speed);
void eanimupdate(eanimation *a, float dt);
void eanimdestroy(eanimation *a);

eterrain *etcreate(int width, int depth, int segments);
void etsetheight(eterrain *t, int x, int z, float h);
void etsettexture(eterrain *t, etexture *tex);
void etsetheightmap(eterrain *t, char *path);
void etdestroy(eterrain *t);

void e3dinit(int w, int h);
void e3dresize(int w, int h);
void e3dsetbackground(float r, float g, float b);
void e3dsetwireframe(int on);
void e3dsetshadowmap(int on);
void e3dsetpostprocess(int on);
void e3dcleanup(void);
void e3drenderframe(escene *s);

#endif
