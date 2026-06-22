#include "../core/axton.h"
#include <sys/stat.h>
#include <dirent.h>

object *pathjoin(object **a, int c, void *e) {
    if (c < 1) throwexception("join needs parts");
    char *result = strdup("");
    for (int i = 0; i < c; i++) {
        if (a[i]->type != 2) throwexception("path parts must be strings");
        char *new = malloc(strlen(result) + strlen(a[i]->sval) + 2);
        if (strlen(result) > 0) sprintf(new, "%s%c%s", result, PATHSEP, a[i]->sval);
        else strcpy(new, a[i]->sval);
        free(result);
        result = new;
    }
    object *res = makestring(result);
    free(result);
    return res;
}

object *pathexists(object **a, int c, void *e) {
    if (c < 1) throwexception("exists needs path");
    struct stat st;
    return makebool(stat(a[0]->sval, &st) == 0);
}

object *pathisdir(object **a, int c, void *e) {
    if (c < 1) throwexception("isdir needs path");
    struct stat st;
    if (stat(a[0]->sval, &st) != 0) return makebool(0);
    return makebool(S_ISDIR(st.st_mode));
}

object *pathisfile(object **a, int c, void *e) {
    if (c < 1) throwexception("isfile needs path");
    struct stat st;
    if (stat(a[0]->sval, &st) != 0) return makebool(0);
    return makebool(S_ISREG(st.st_mode));
}

object *pathglob(object **a, int c, void *e) {
    if (c < 1) throwexception("glob needs pattern");
    object *list = makelist();
    return list;
}

void registerpathlib(environment *env) {
    object *mod = makemodule("path", NULL);
    envset(mod->module.exports, "join", makebuiltin(pathjoin), 0);
    envset(mod->module.exports, "exists", makebuiltin(pathexists), 0);
    envset(mod->module.exports, "isdir", makebuiltin(pathisdir), 0);
    envset(mod->module.exports, "isfile", makebuiltin(pathisfile), 0);
    envset(mod->module.exports, "glob", makebuiltin(pathglob), 0);
    envset(env, "path", mod, 0);
}
