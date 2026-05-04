#include "../core/axton.h"

static const char b64abc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

object *base64encode(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=2) throwexception("encode needs string");
    unsigned char *in = (unsigned char*)args[0]->sval;
    int len = strlen(args[0]->sval);
    int outlen = ((len+2)/3)*4;
    char *out = malloc(outlen+1);
    int i,j;
    for(i=0,j=0;i<len;i+=3,j+=4){
        int b1=i<len?in[i]:0, b2=i+1<len?in[i+1]:0, b3=i+2<len?in[i+2]:0;
        unsigned int triple = (b1<<16)|(b2<<8)|b3;
        out[j] = b64abc[(triple>>18)&0x3F];
        out[j+1] = b64abc[(triple>>12)&0x3F];
        out[j+2] = i+1<len?b64abc[(triple>>6)&0x3F]:'=';
        out[j+3] = i+2<len?b64abc[triple&0x3F]:'=';
    }
    out[outlen]=0;
    object *res = makestring(out);
    free(out);
    return res;
}

static int b64val(char c) {
    if(c>='A'&&c<='Z') return c-'A';
    if(c>='a'&&c<='z') return c-'a'+26;
    if(c>='0'&&c<='9') return c-'0'+52;
    if(c=='+') return 62;
    if(c=='/') return 63;
    return -1;
}

object *base64decode(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=2) throwexception("decode needs string");
    char *in = args[0]->sval;
    int len = strlen(in);
    if(len%4!=0) throwexception("invalid length");
    int outlen = len/4*3;
    unsigned char *out = malloc(outlen+1);
    int i,j;
    for(i=0,j=0;i<len;i+=4,j+=3){
        int a=b64val(in[i]), b=b64val(in[i+1]), c=b64val(in[i+2]), d=b64val(in[i+3]);
        if(a<0||b<0) throwexception("invalid char");
        unsigned int triple = (a<<18)|(b<<12)|((c>=0?c:0)<<6)|((d>=0?d:0));
        out[j] = (triple>>16)&0xFF;
        if(in[i+2]!='=') out[j+1] = (triple>>8)&0xFF;
        if(in[i+3]!='=') out[j+2] = triple&0xFF;
    }
    out[outlen]=0;
    object *res = makestring((char*)out);
    free(out);
    return res;
}

void registerbase64lib(environment *env) {
    object *mod = makemodule("base64", NULL);
    envset(mod->module.exports, "b64encode", makebuiltin(base64encode), 0);
    envset(mod->module.exports, "b64decode", makebuiltin(base64decode), 0);
    envset(env, "base64", mod, 0);
}
