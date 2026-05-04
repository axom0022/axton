#include "../core/axton.h"
#include <sys/wait.h>

object *subprocessrun(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 2) throwexception("run needs command");
    int ret = system(a[0]->sval);
    return makeint(WEXITSTATUS(ret));
}

object *subprocessoutput(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 2) throwexception("output needs command");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s 2>/dev/null", a[0]->sval);
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

object *subprocesscall(object **a, int c, void *e) {
    return subprocessrun(a, c, e);
}

void registersubprocesslib(environment *env) {
    object *mod = makemodule("subprocess", NULL);
    envset(mod->module.exports, "run", makebuiltin(subprocessrun), 0);
    envset(mod->module.exports, "output", makebuiltin(subprocessoutput), 0);
    envset(mod->module.exports, "call", makebuiltin(subprocesscall), 0);
    envset(env, "subprocess", mod, 0);
}
