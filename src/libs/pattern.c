#include "../core/axton.h"

object *patternmatch(object **a, int c, void *e) {
    if (c < 2) throwexception("match needs value and cases");
    object *val = a[0];
    object *cases = a[1];
    return makenone();
}

void registerpatternlib(environment *env) {
    object *mod = makemodule("pattern", NULL);
    envset(mod->module.exports, "match", makebuiltin(patternmatch), 0);
    envset(env, "pattern", mod, 0);
}
