#include "../core/axton.h"

object *debugbreak(object **a, int c, void *e) {
    if (c < 1) throwexception("break needs line");
    return makenone();
}

object *debugstep(object **a, int c, void *e) {
    return makenone();
}

object *debugcontinue(object **a, int c, void *e) {
    return makenone();
}

object *debuginspect(object **a, int c, void *e) {
    if (c < 1) throwexception("inspect needs variable");
    return makenone();
}

void registerdebuglib(environment *env) {
    object *mod = makemodule("debug", NULL);
    envset(mod->module.exports, "break", makebuiltin(debugbreak), 0);
    envset(mod->module.exports, "step", makebuiltin(debugstep), 0);
    envset(mod->module.exports, "continue", makebuiltin(debugcontinue), 0);
    envset(mod->module.exports, "inspect", makebuiltin(debuginspect), 0);
    envset(env, "debug", mod, 0);
}
