#include "../core/axton.h"

object *builtinsysargv(object **args, int argc, environment *env) {
    object *argv = makelist();
    listappend(argv, makestring("axton"));
    return argv;
}

object *builtinsysversion(object **args, int argc, environment *env) {
    return makestring("axton 2.0.0");
}

object *builtinsysplatform(object **args, int argc, environment *env) {
#ifdef _WIN32
    return makestring("windows");
#elif __linux__
    return makestring("linux");
#elif __APPLE__
    return makestring("darwin");
#elif __ANDROID__
    return makestring("android");
#else
    return makestring("unknown");
#endif
}

object *builtinsysexit(object **args, int argc, environment *env) {
    int code = 0;
    if (argc > 0 && args[0]->type == 0) code = args[0]->ival;
    exit(code);
    return makenone();
}

object *builtinsysstdout(object **args, int argc, environment *env) {
    return makefile(stdout, 1, 1);
}

object *builtinsysstdin(object **args, int argc, environment *env) {
    return makefile(stdin, 0, 0);
}

object *builtinsysstderr(object **args, int argc, environment *env) {
    return makefile(stderr, 2, 1);
}

object *builtinsyspath(object **args, int argc, environment *env) {
    object *paths = makelist();
    listappend(paths, makestring("."));
    listappend(paths, makestring("./lib"));
    return paths;
}

void registersyslib(environment *env) {
    object *sysmod = makemodule("sys", NULL);
    envset(sysmod->module.exports, "argv", makebuiltin(builtinsysargv), 0);
    envset(sysmod->module.exports, "version", makebuiltin(builtinsysversion), 0);
    envset(sysmod->module.exports, "platform", makebuiltin(builtinsysplatform), 0);
    envset(sysmod->module.exports, "exit", makebuiltin(builtinsysexit), 0);
    envset(sysmod->module.exports, "stdout", makebuiltin(builtinsysstdout), 0);
    envset(sysmod->module.exports, "stdin", makebuiltin(builtinsysstdin), 0);
    envset(sysmod->module.exports, "stderr", makebuiltin(builtinsysstderr), 0);
    envset(sysmod->module.exports, "path", makebuiltin(builtinsyspath), 0);
    envset(env, "sys", sysmod, 0);
}
