#include "../core/axton.h"

object *webbrowseropen(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=2) throwexception("open needs url");
    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "start %s", a[0]->sval);
#elif __APPLE__
    snprintf(cmd, sizeof(cmd), "open %s", a[0]->sval);
#else
    snprintf(cmd, sizeof(cmd), "xdg-open %s", a[0]->sval);
#endif
    system(cmd);
    return makenone();
}

void registerwebbrowserlib(environment *env) {
    object *mod = makemodule("webbrowser", NULL);
    envset(mod->module.exports, "open", makebuiltin(webbrowseropen), 0);
    envset(env, "webbrowser", mod, 0);
}
