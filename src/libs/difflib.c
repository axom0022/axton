#include "../core/axton.h"

object *difflibndiff(object **a, int c, void *e) {
    if(c<2) throwexception("ndiff needs two strings");
    char *a1 = a[0]->sval, *a2 = a[1]->sval;
    object *res = makelist();
    int i=0,j=0;
    while(a1[i]||a2[j]){
        if(a1[i]==a2[j]){
            char buf[256];
            snprintf(buf,256,"  %c",a1[i]);
            listappend(res, makestring(buf));
            i++; j++;
        } else if(a1[i] && (!a2[j] || a1[i]<a2[j])){
            char buf[256];
            snprintf(buf,256,"- %c",a1[i]);
            listappend(res, makestring(buf));
            i++;
        } else {
            char buf[256];
            snprintf(buf,256,"+ %c",a2[j]);
            listappend(res, makestring(buf));
            j++;
        }
    }
    return res;
}

void registerdiffliblib(environment *env) {
    object *mod = makemodule("difflib", NULL);
    envset(mod->module.exports, "ndiff", makebuiltin(difflibndiff), 0);
    envset(env, "difflib", mod, 0);
}
