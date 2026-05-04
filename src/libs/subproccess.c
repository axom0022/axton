#include "../core/axton.h"
#include <sys/wait.h>

object *subprocess_run(object **args, int argc, void *env) {
    if (argc < 1 || args[0]->type != 2) throwexception("run needs command");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s", args[0]->sval);
    int ret = system(cmd);
    return makeint(WEXITSTATUS(ret));
}

object *subprocess_output(object **args, int argc, void *env) {
    if (argc < 1 || args[0]->type != 2) throwexception("output needs command");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s 2>/dev/null", args[0]->sval);
    FILE *fp = popen(cmd, "r");
    if (!fp) return makestring("");
    char buf[4096];
    size_t total = 0;
    char *out = malloc(1);
    out[0] = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        size_t len = strlen(buf);
        out = realloc(out, total + len + 1);
        strcpy(out + total, buf);
        total += len;
    }
    pclose(fp);
    object *res = makestring(out);
    free(out);
    return res;
}

object *subprocess_call(object **args, int argc, void *env) {
    return subprocess_run(args, argc, env);
}

void registersubprocesslib(environment *env) {
    object *mod = makemodule("subprocess", NULL);
    envset(mod->module.exports, "run", makebuiltin(subprocess_run), 0);
    envset(mod->module.exports, "output", makebuiltin(subprocess_output), 0);
    envset(mod->module.exports, "call", makebuiltin(subprocess_call), 0);
    envset(env, "subprocess", mod, 0);
}
