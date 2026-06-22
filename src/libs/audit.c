#include "../core/axton.h"
#include <time.h>

object *auditlog(object **a, int c, void *e) {
    if (c < 2) throwexception("log needs action and data");
    time_t t = time(NULL);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    platformlog("audit: ");
    platformlog(buf);
    platformlog(" ");
    platformlog(a[0]->sval);
    platformlog(" ");
    platformlog(a[1]->sval);
    platformlog("\n");
    return makenone();
}

void registerauditlib(environment *env) {
    object *mod = makemodule("audit", NULL);
    envset(mod->module.exports, "log", makebuiltin(auditlog), 0);
    envset(env, "audit", mod, 0);
}
