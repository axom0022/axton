#include "../core/axton.h"

object *secretstore(object **a, int c, void *e) {
    if (c < 2) throwexception("store needs key and value");
    return makenone();
}

object *secretretrieve(object **a, int c, void *e) {
    if (c < 1) throwexception("retrieve needs key");
    return makenone();
}

void registersecretlib(environment *env) {
    object *mod = makemodule("secret", NULL);
    envset(mod->module.exports, "store", makebuiltin(secretstore), 0);
    envset(mod->module.exports, "retrieve", makebuiltin(secretretrieve), 0);
    envset(env, "secret", mod, 0);
}
