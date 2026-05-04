#include "../core/axton.h"

object *tempfilemkstemp(object **args, int argc, void *env) {
    char template[] = "/tmp/axtonXXXXXX";
    int fd = mkstemp(template);
    if(fd<0) throwexception("cannot create temp file");
    close(fd);
    return makestring(template);
}

object *tempfilemkdtemp(object **args, int argc, void *env) {
    char template[] = "/tmp/axtonXXXXXX";
    char *dir = mkdtemp(template);
    if(!dir) throwexception("cannot create temp dir");
    return makestring(dir);
}

object *tempfilegettempdir(object **args, int argc, void *env) {
    return makestring("/tmp");
}

void registertempfilelib(environment *env) {
    object *mod = makemodule("tempfile", NULL);
    envset(mod->module.exports, "mkstemp", makebuiltin(tempfilemkstemp), 0);
    envset(mod->module.exports, "mkdtemp", makebuiltin(tempfilemkdtemp), 0);
    envset(mod->module.exports, "gettempdir", makebuiltin(tempfilegettempdir), 0);
    envset(env, "tempfile", mod, 0);
}
