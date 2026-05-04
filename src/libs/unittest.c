#include "../core/axton.h"

static int testcount = 0;
static int failcount = 0;

object *unittestasserttrue(object **a, int c, void *e) {
    testcount++;
    if(c<1 || !istruthy(a[0])){
        failcount++;
        platformlog("FAIL: assertTrue\n");
    }
    return makenone();
}

object *unittestassertequal(object **a, int c, void *e) {
    testcount++;
    if(c<2 || !valuesequal(a[0],a[1])){
        failcount++;
        platformlog("FAIL: assertEqual\n");
    }
    return makenone();
}

object *unittestreport(object **a, int c, void *e) {
    char buf[256];
    snprintf(buf,256,"tests: %d, failures: %d\n", testcount, failcount);
    platformlog(buf);
    return makenone();
}

void registerunittestlib(environment *env) {
    object *mod = makemodule("unittest", NULL);
    envset(mod->module.exports, "assertTrue", makebuiltin(unittestasserttrue), 0);
    envset(mod->module.exports, "assertEquals", makebuiltin(unittestassertequal), 0);
    envset(mod->module.exports, "report", makebuiltin(unittestreport), 0);
    envset(env, "unittest", mod, 0);
}
