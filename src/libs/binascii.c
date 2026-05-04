#include "../core/axton.h"

object *binasciihexlify(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=2) throwexception("hexlify needs string");
    char *in = a[0]->sval;
    int len = strlen(in);
    char *out = malloc(len*2+1);
    for(int i=0;i<len;i++) snprintf(out+i*2,3,"%02x",(unsigned char)in[i]);
    object *res = makestring(out);
    free(out);
    return res;
}

object *binasciiunhexlify(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=2) throwexception("unhexlify needs hex string");
    char *in = a[0]->sval;
    int len = strlen(in);
    if(len%2) throwexception("odd length hex");
    char *out = malloc(len/2+1);
    for(int i=0;i<len;i+=2){
        char hex[3] = {in[i], in[i+1], 0};
        out[i/2] = strtol(hex, NULL, 16);
    }
    out[len/2]=0;
    object *res = makestring(out);
    free(out);
    return res;
}

void registerbinasciilib(environment *env) {
    object *mod = makemodule("binascii", NULL);
    envset(mod->module.exports, "hexlify", makebuiltin(binasciihexlify), 0);
    envset(mod->module.exports, "unhexlify", makebuiltin(binasciiunhexlify), 0);
    envset(env, "binascii", mod, 0);
}
