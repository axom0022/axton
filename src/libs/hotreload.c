#include "../core/axton.h"
#include <sys/stat.h>
#include <unistd.h>

typedef struct hotreload {
    char *path;
    char *backup;
    long lastmtime;
    object *onchange;
    int running;
    int interval;
} hotreload;

static hotreload *hr = NULL;

object *hotreloadcreate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("hotreload needs path");
    hotreload *h = malloc(sizeof(hotreload));
    h->path = strdup(args[0]->sval);
    h->backup = NULL;
    struct stat st;
    h->lastmtime = stat(h->path, &st) == 0 ? st.st_mtime : 0;
    h->onchange = NULL;
    h->running = 1;
    h->interval = 100;
    hr = h;
    return makenative(h, NULL);
}

object *hotreloadonchange(object **args, int argc, void *env) {
    if (argc < 1 || !hr) throwexception("onchange needs handler");
    hr->onchange = args[0];
    return makenone();
}

object *hotreloadupdate(object **args, int argc, void *env) {
    if (!hr || !hr->running) return makenone();
    struct stat st;
    if (stat(hr->path, &st) == 0 && st.st_mtime != hr->lastmtime) {
        hr->lastmtime = st.st_mtime;
        char *content = platformreadfile(hr->path);
        if (content && hr->onchange) {
            object *args[1] = {makestring(content)};
            callfunc(hr->onchange, args, 1, env);
            free(content);
        }
    }
    return makenone();
}

object *hotreloadstop(object **args, int argc, void *env) {
    if (hr) hr->running = 0;
    return makenone();
}

void registerhotreloadlib(environment *env) {
    object *mod = makemodule("hotreload", NULL);
    envset(mod->module.exports, "create", makebuiltin(hotreloadcreate), 0);
    envset(mod->module.exports, "onchange", makebuiltin(hotreloadonchange), 0);
    envset(mod->module.exports, "update", makebuiltin(hotreloadupdate), 0);
    envset(mod->module.exports, "stop", makebuiltin(hotreloadstop), 0);
    envset(env, "hotreload", mod, 0);
}
