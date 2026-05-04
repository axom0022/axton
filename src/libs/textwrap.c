#include "../core/axton.h"

object *textwrapfill(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=2) throwexception("fill needs text");
    char *text = a[0]->sval;
    int width = 70;
    if(c>1 && a[1]->type==0) width = a[1]->ival;
    int len = strlen(text);
    char *out = malloc(len*2+1);
    int outpos = 0;
    int linelen = 0;
    for(int i=0;i<len;i++){
        if(text[i]==' ' && linelen>=width){
            out[outpos++]='\n';
            linelen=0;
        } else {
            out[outpos++]=text[i];
            linelen++;
        }
    }
    out[outpos]=0;
    object *res = makestring(out);
    free(out);
    return res;
}

void registertextwraplib(environment *env) {
    object *mod = makemodule("textwrap", NULL);
    envset(mod->module.exports, "fill", makebuiltin(textwrapfill), 0);
    envset(env, "textwrap", mod, 0);
}
