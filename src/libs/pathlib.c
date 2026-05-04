#include "../core/axton.h"
#include <sys/stat.h>

static char *joinpath(const char *a, const char *b) {
    int len = strlen(a)+strlen(b)+2;
    char *res = malloc(len);
    snprintf(res, len, "%s%c%s", a, PATHSEP, b);
    return res;
}

object *pathlib_path(object **args, int argc, void *env) {
    if(argc<1) return makestring(".");
    char *path = strdup(args[0]->sval);
    for(int i=1;i<argc;i++){
        char *newpath = joinpath(path, args[i]->sval);
        free(path);
        path = newpath;
    }
    object *p = makedict();
    dictset(p, makestring("_path"), makestring(path), 0);
    free(path);
    return p;
}

object *pathlib_join(object **args, int argc, void *env) {
    if(argc<2) throwexception("join needs path and *paths");
    char *base = args[0]->sval;
    for(int i=1;i<argc;i++){
        char *newbase = joinpath(base, args[i]->sval);
        base = newbase;
    }
    return makestring(base);
}

object *pathlib_exists(object **args, int argc, void *env) {
    if(argc<1) throwexception("exists needs path");
    struct stat st;
    return makebool(stat(args[0]->sval, &st)==0);
}

object *pathlib_isdir(object **args, int argc, void *env) {
    if(argc<1) throwexception("isdir needs path");
    struct stat st;
    if(stat(args[0]->sval, &st)!=0) return makebool(0);
    return makebool(S_ISDIR(st.st_mode));
}

object *pathlib_isfile(object **args, int argc, void *env) {
    if(argc<1) throwexception("isfile needs path");
    struct stat st;
    if(stat(args[0]->sval, &st)!=0) return makebool(0);
    return makebool(S_ISREG(st.st_mode));
}

void registerpathliblib(environment *env) {
    object *mod = makemodule("pathlib", NULL);
    envset(mod->module.exports, "Path", makebuiltin(pathlib_path), 0);
    envset(mod->module.exports, "join", makebuiltin(pathlib_join), 0);
    envset(mod->module.exports, "exists", makebuiltin(pathlib_exists), 0);
    envset(mod->module.exports, "isdir", makebuiltin(pathlib_isdir), 0);
    envset(mod->module.exports, "isfile", makebuiltin(pathlib_isfile), 0);
    envset(env, "pathlib", mod, 0);
}
