#include "../core/axton.h"

object *clouds3(object **a, int c, void *e) {
    if (c < 3) throwexception("s3 needs bucket key data");
    return makenone();
}

object *cloudlambda(object **a, int c, void *e) {
    if (c < 2) throwexception("lambda needs name handler");
    return makenone();
}

void registercloudlib(environment *env) {
    object *mod = makemodule("cloud", NULL);
    envset(mod->module.exports, "s3", makebuiltin(clouds3), 0);
    envset(mod->module.exports, "lambda", makebuiltin(cloudlambda), 0);
    envset(env, "cloud", mod, 0);
}
