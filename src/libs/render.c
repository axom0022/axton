#include "../core/axton.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <math.h>
#include <string.h>

typedef struct pbrmaterial {
    float basecolor[3];
    float metallic;
    float roughness;
    float emissive[3];
    float normalstrength;
    unsigned int textures[4];
} pbrmaterial;

typedef struct shadowmap {
    unsigned int fbo;
    unsigned int depthtex;
    int size;
    float bias[16];
} shadowmap;

typedef struct postprocess {
    unsigned int fbo;
    unsigned int color;
    unsigned int depth;
    unsigned int shader;
    int width;
    int height;
    float bloomstrength;
    float hdrexposure;
    float doffocus;
    float dofrange;
    int ssao;
    int ssr;
} postprocess;

static pbrmaterial *curpbr = NULL;
static shadowmap *curshadow = NULL;
static postprocess *curpost = NULL;

object *pbrcreate(object **args, int argc, void *env) {
    pbrmaterial *p = malloc(sizeof(pbrmaterial));
    p->basecolor[0]=1; p->basecolor[1]=1; p->basecolor[2]=1;
    p->metallic=0; p->roughness=0.5; p->emissive[0]=p->emissive[1]=p->emissive[2]=0;
    p->normalstrength=1; p->textures[0]=p->textures[1]=p->textures[2]=p->textures[3]=0;
    curpbr=p;
    return makenative(p, NULL);
}

object *pbrsetcolor(object **args, int argc, void *env) {
    if(argc<4 || !curpbr) throwexception("setcolor needs r g b");
    curpbr->basecolor[0]=args[0]->fval;
    curpbr->basecolor[1]=args[1]->fval;
    curpbr->basecolor[2]=args[2]->fval;
    return makenone();
}

object *pbrsetmetallic(object **args, int argc, void *env) {
    if(argc<1 || !curpbr) throwexception("setmetallic needs val");
    curpbr->metallic=args[0]->fval;
    return makenone();
}

object *pbrsetroughness(object **args, int argc, void *env) {
    if(argc<1 || !curpbr) throwexception("setroughness needs val");
    curpbr->roughness=args[0]->fval;
    return makenone();
}

object *pbrsetemissive(object **args, int argc, void *env) {
    if(argc<3 || !curpbr) throwexception("setemissive needs r g b");
    curpbr->emissive[0]=args[0]->fval;
    curpbr->emissive[1]=args[1]->fval;
    curpbr->emissive[2]=args[2]->fval;
    return makenone();
}

object *pbrsettexture(object **args, int argc, void *env) {
    if(argc<3 || !curpbr) throwexception("settexture needs type path");
    int type=args[0]->ival; char *path=args[1]->sval;
    if(type>=0&&type<4) curpbr->textures[type] = 1;
    return makenone();
}

object *shadowcreate(object **args, int argc, void *env) {
    shadowmap *s = malloc(sizeof(shadowmap));
    s->size = (argc>0 && args[0]->type==0) ? args[0]->ival : 2048;
    glGenFramebuffers(1,&s->fbo);
    glGenTextures(1,&s->depthtex);
    glBindTexture(GL_TEXTURE_2D, s->depthtex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,s->size,s->size,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER,s->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,s->depthtex,0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    curs hadow=s;
    return makenative(s, NULL);
}

object *shadowbegin(object **args, int argc, void *env) {
    if(!curshadow) throwexception("no shadow");
    glViewport(0,0,curshadow->size,curshadow->size);
    glBindFramebuffer(GL_FRAMEBUFFER,curshadow->fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    return makenone();
}

object *shadowend(object **args, int argc, void *env) {
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    return makenone();
}

object *postcreate(object **args, int argc, void *env) {
    postprocess *p = malloc(sizeof(postprocess));
    p->width = (argc>0 && args[0]->type==0) ? args[0]->ival : 1920;
    p->height = (argc>1 && args[1]->type==0) ? args[1]->ival : 1080;
    p->bloomstrength=0.1; p->hdrexposure=1; p->dofocus=0.5; p->dofrange=0.1;
    p->ssao=0; p->ssr=0;
    glGenFramebuffers(1,&p->fbo);
    glGenTextures(1,&p->color);
    glGenTextures(1,&p->depth);
    glBindTexture(GL_TEXTURE_2D,p->color);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,p->width,p->height,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,p->depth);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,p->width,p->height,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glBindFramebuffer(GL_FRAMEBUFFER,p->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,p->color,0);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,p->depth,0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    curpost=p;
    return makenative(p, NULL);
}

object *postbegin(object **args, int argc, void *env) {
    if(!curpost) throwexception("no post");
    glBindFramebuffer(GL_FRAMEBUFFER,curpost->fbo);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    return makenone();
}

object *postend(object **args, int argc, void *env) {
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    return makenone();
}

object *postsetbloom(object **args, int argc, void *env) {
    if(argc<1 || !curpost) throwexception("setbloom needs strength");
    curpost->bloomstrength=args[0]->fval;
    return makenone();
}

object *postsetexposure(object **args, int argc, void *env) {
    if(argc<1 || !curpost) throwexception("setexposure needs val");
    curpost->hdrexposure=args[0]->fval;
    return makenone();
}

object *postsetdof(object **args, int argc, void *env) {
    if(argc<2 || !curpost) throwexception("setdof needs focus range");
    curpost->dofocus=args[0]->fval;
    curpost->dofrange=args[1]->fval;
    return makenone();
}

object *postsetsao(object **args, int argc, void *env) {
    if(argc<1 || !curpost) throwexception("setsao needs on");
    curpost->ssao=args[0]->bval;
    return makenone();
}

object *postsetssr(object **args, int argc, void *env) {
    if(argc<1 || !curpost) throwexception("setssr needs on");
    curpost->ssr=args[0]->bval;
    return makenone();
}

void registerrenderlib(environment *env) {
    object *mod = makemodule("render", NULL);
    envset(mod->module.exports, "pbrcreate", makebuiltin(pbrcreate), 0);
    envset(mod->module.exports, "pbrsetcolor", makebuiltin(pbrsetcolor), 0);
    envset(mod->module.exports, "pbrsetmetallic", makebuiltin(pbrsetmetallic), 0);
    envset(mod->module.exports, "pbrsetroughness", makebuiltin(pbrsetroughness), 0);
    envset(mod->module.exports, "pbrsetemissive", makebuiltin(pbrsetemissive), 0);
    envset(mod->module.exports, "pbrsettexture", makebuiltin(pbrsettexture), 0);
    envset(mod->module.exports, "shadowcreate", makebuiltin(shadowcreate), 0);
    envset(mod->module.exports, "shadowbegin", makebuiltin(shadowbegin), 0);
    envset(mod->module.exports, "shadowend", makebuiltin(shadowend), 0);
    envset(mod->module.exports, "postcreate", makebuiltin(postcreate), 0);
    envset(mod->module.exports, "postbegin", makebuiltin(postbegin), 0);
    envset(mod->module.exports, "postend", makebuiltin(postend), 0);
    envset(mod->module.exports, "postsetbloom", makebuiltin(postsetbloom), 0);
    envset(mod->module.exports, "postsetexposure", makebuiltin(postsetexposure), 0);
    envset(mod->module.exports, "postsetdof", makebuiltin(postsetdof), 0);
    envset(mod->module.exports, "postsetsao", makebuiltin(postsetsao), 0);
    envset(mod->module.exports, "postsetssr", makebuiltin(postsetssr), 0);
    envset(env, "render", mod, 0);
}
