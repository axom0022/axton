#include "../core/axton.h"

object *builtinosgetenv(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("os.getenv needs name");
    char *val = platformgetenv(args[0]->sval);
    return val ? makestring(val) : makenone();
}

object *builtinossetenv(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("os.setenv needs name and value");
    return makebool(platformsetenv(args[0]->sval, args[1]->sval));
}

object *builtinoslistdir(object **args, int argc, environment *env) {
    char *path = ".";
    if (argc > 0) path = args[0]->sval;
    int count = 0;
    char **entries = platformlistdir(path, &count);
    object *result = makelist();
    for (int i = 0; i < count; i++) {
        listappend(result, makestring(entries[i]));
        free(entries[i]);
    }
    free(entries);
    return result;
}

object *builtinosmkdir(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("os.mkdir needs path");
    return makebool(platformmkdir(args[0]->sval) == 0);
}

object *builtinosremove(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("os.remove needs path");
    return makebool(platformremove(args[0]->sval) == 0);
}

object *builtinosrename(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("os.rename needs old and new");
    return makebool(platformrename(args[0]->sval, args[1]->sval) == 0);
}

object *builtinosgetpid(object **args, int argc, environment *env) {
#ifdef _WIN32
    return makeint(GetCurrentProcessId());
#else
    return makeint(getpid());
#endif
}

object *builtinosgetcwd(object **args, int argc, environment *env) {
    char cwd[1024];
#ifdef _WIN32
    _getcwd(cwd, sizeof(cwd));
#else
    getcwd(cwd, sizeof(cwd));
#endif
    return makestring(cwd);
}

object *builtinoschdir(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("os.chdir needs path");
#ifdef _WIN32
    return makebool(_chdir(args[0]->sval) == 0);
#else
    return makebool(chdir(args[0]->sval) == 0);
#endif
}

object *builtinoswalk(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("os.walk needs path");
    char *path = args[0]->sval;
    object *result = makelist();
    int count = 0;
    char **entries = platformlistdir(path, &count);
    object *files = makelist();
    object *dirs = makelist();
    for (int i = 0; i < count; i++) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entries[i]);
        struct stat st;
        if (stat(fullpath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) listappend(dirs, makestring(entries[i]));
            else listappend(files, makestring(entries[i]));
        }
        free(entries[i]);
    }
    free(entries);
    object *triple = makelist();
    listappend(triple, makestring(path));
    listappend(triple, dirs);
    listappend(triple, files);
    listappend(result, triple);
    return result;
}

object *builtinossystem(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("os.system needs command");
    int ret = system(args[0]->sval);
    return makeint(ret);
}

object *builtinosname(object **args, int argc, environment *env) {
#ifdef _WIN32
    return makestring("nt");
#else
    return makestring("posix");
#endif
}

void registeroslib(environment *env) {
    object *osmod = makemodule("os", NULL);
    envset(osmod->module.exports, "getenv", makebuiltin(builtinosgetenv), 0);
    envset(osmod->module.exports, "setenv", makebuiltin(builtinossetenv), 0);
    envset(osmod->module.exports, "listdir", makebuiltin(builtinoslistdir), 0);
    envset(osmod->module.exports, "mkdir", makebuiltin(builtinosmkdir), 0);
    envset(osmod->module.exports, "remove", makebuiltin(builtinosremove), 0);
    envset(osmod->module.exports, "rename", makebuiltin(builtinosrename), 0);
    envset(osmod->module.exports, "getpid", makebuiltin(builtinosgetpid), 0);
    envset(osmod->module.exports, "getcwd", makebuiltin(builtinosgetcwd), 0);
    envset(osmod->module.exports, "chdir", makebuiltin(builtinoschdir), 0);
    envset(osmod->module.exports, "walk", makebuiltin(builtinoswalk), 0);
    envset(osmod->module.exports, "system", makebuiltin(builtinossystem), 0);
    envset(osmod->module.exports, "name", makebuiltin(builtinosname), 0);
    envset(env, "os", osmod, 0);
}
