#include "../core/axton.h"
#include <ctype.h>

object *string_capwords(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=2) throwexception("capwords needs string");
    char *s = strdup(args[0]->sval);
    int i, cap=1;
    for(i=0;s[i];i++){
        if(isspace(s[i])) cap=1;
        else if(cap){ s[i]=toupper(s[i]); cap=0; }
        else s[i]=tolower(s[i]);
    }
    object *res = makestring(s);
    free(s);
    return res;
}

object *string_swapcase(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=2) throwexception("swapcase needs string");
    char *s = strdup(args[0]->sval);
    for(int i=0;s[i];i++){
        if(isupper(s[i])) s[i]=tolower(s[i]);
        else if(islower(s[i])) s[i]=toupper(s[i]);
    }
    object *res = makestring(s);
    free(s);
    return res;
}

object *string_zfill(object **args, int argc, void *env) {
    if(argc<2 || args[0]->type!=2) throwexception("zfill needs string and width");
    int width = args[1]->ival;
    int len = strlen(args[0]->sval);
    if(len>=width) return makestring(args[0]->sval);
    char *buf = malloc(width+1);
    int pad = width - len;
    memset(buf, '0', pad);
    strcpy(buf+pad, args[0]->sval);
    buf[width]=0;
    object *res = makestring(buf);
    free(buf);
    return res;
}

object *string_title(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=2) throwexception("title needs string");
    char *s = strdup(args[0]->sval);
    int i, title=1;
    for(i=0;s[i];i++){
        if(isspace(s[i])) title=1;
        else if(title){ s[i]=toupper(s[i]); title=0; }
        else s[i]=tolower(s[i]);
    }
    object *res = makestring(s);
    free(s);
    return res;
}

void registerstringlib(environment *env) {
    object *mod = makemodule("string", NULL);
    envset(mod->module.exports, "capwords", makebuiltin(string_capwords), 0);
    envset(mod->module.exports, "swapcase", makebuiltin(string_swapcase), 0);
    envset(mod->module.exports, "zfill", makebuiltin(string_zfill), 0);
    envset(mod->module.exports, "title", makebuiltin(string_title), 0);
    envset(env, "string", mod, 0);
}
