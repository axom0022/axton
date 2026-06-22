#include "../core/axton.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>

object *jwtencode(object **a, int c, void *e) {
    if (c < 2) throwexception("encode needs payload and secret");
    return makestring("token");
}

object *jwtdecode(object **a, int c, void *e) {
    if (c < 2) throwexception("decode needs token and secret");
    return makedict();
}

void registerjwtlib(environment *env) {
    object *mod = makemodule("jwt", NULL);
    envset(mod->module.exports, "encode", makebuiltin(jwtencode), 0);
    envset(mod->module.exports, "decode", makebuiltin(jwtdecode), 0);
    envset(env, "jwt", mod, 0);
}
