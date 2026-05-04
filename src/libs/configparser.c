#include "../core/axton.h"

object *configparserparser(object **a, int c, void *e) {
    return makedict();
}

object *configparserread(object **a, int c, void *e) {
    if(c<2 || a[0]->type!=6 || a[1]->type!=2) throwexception("read needs parser and filename");
    object *parser = a[0];
    char *fn = a[1]->sval;
    char *content = platformreadfile(fn);
    if(!content) return makenone();
    char *section = "DEFAULT";
    char *line = strtok(content, "\n");
    while(line){
        if(line[0]=='[' && strchr(line,']')){
            section = strdup(line+1);
            char *end = strchr(section,']');
            if(end) *end=0;
        } else if(strchr(line,'=')){
            char *key = line;
            char *val = strchr(line,'=');
            *val = 0;
            val++;
            while(*key==' ') key++;
            while(*val==' ') val++;
            char *end = key+strlen(key)-1;
            while(end>key && *end==' ') end--;
            end[1]=0;
            object *sec = dictget(parser, makestring(section));
            if(!sec){
                sec = makedict();
                dictset(parser, makestring(section), sec);
            }
            dictset(sec, makestring(key), makestring(val));
        }
        line = strtok(NULL, "\n");
    }
    free(content);
    return parser;
}

object *configparserget(object **a, int c, void *e) {
    if(c<3) throwexception("get needs parser, section, key");
    object *sec = dictget(a[0], makestring(a[1]->sval));
    if(!sec) return makenone();
    object *val = dictget(sec, makestring(a[2]->sval));
    return val ? val : makenone();
}

void registerconfigparserlib(environment *env) {
    object *mod = makemodule("configparser", NULL);
    envset(mod->module.exports, "ConfigParser", makebuiltin(configparserparser), 0);
    envset(mod->module.exports, "read", makebuiltin(configparserread), 0);
    envset(mod->module.exports, "get", makebuiltin(configparserget), 0);
    envset(env, "configparser", mod, 0);
}
