#include "../core/axton.h"

object *tunnelstart(object **a, int c, void *e) {
    if (c < 1) throwexception("tunnel needs port");
    return makestring("http://localhost:4040");
}

object *tunnelstop(object **a, int c, void *e) {
    return makenone();
}

void registertunnellib(environment *env) {
    object *mod = makemodule("tunnel", NULL);
    envset(mod->module.exports, "start", makebuiltin(tunnelstart), 0);
    envset(mod->module.exports, "stop", makebuiltin(tunnelstop), 0);
    envset(env, "tunnel", mod, 0);
}
