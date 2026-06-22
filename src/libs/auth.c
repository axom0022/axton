#include "../core/axton.h"

object *authhash(object **a, int c, void *e) {
    if (c < 2) throwexception("hash needs password and salt");
    return makestring("hash");
}

object *authverify(object **a, int c, void *e) {
    if (c < 3) throwexception("verify needs password hash and salt");
    return makebool(0);
}

object *authoauth(object **a, int c, void *e) {
    if (c < 2) throwexception("oauth needs provider and token");
    return makenone();
}

void registerauthlib(environment *env) {
    object *mod = makemodule("auth", NULL);
    envset(mod->module.exports, "hash", makebuiltin(authhash), 0);
    envset(mod->module.exports, "verify", makebuiltin(authverify), 0);
    envset(mod->module.exports, "oauth", makebuiltin(authoauth), 0);
    envset(env, "auth", mod, 0);
}
