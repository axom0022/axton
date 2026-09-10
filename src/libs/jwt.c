#include "../core/axton.h"

object *jwtcreate(object **args, int argc, void *env) {
    void *jwt = platformjwtcreate();
    if (!jwt) throwexception("jwt create failed");
    return makejwt(jwt);
}

object *jwtencode(object **args, int argc, void *env) {
    if (argc < 2) throwexception("encode needs secret");
    object *jwt = args[0];
    char *secret = args[1]->sval;
    char *token = platformjwtencode(jwt->jwttok.token, secret);
    return makestring(token);
}

object *jwtdecode(object **args, int argc, void *env) {
    if (argc < 2) throwexception("decode needs token");
    object *jwt = args[0];
    char *token = args[1]->sval;
    void *decoded = platformjwtdecode(jwt->jwttok.token, token);
    return makenative(decoded, NULL);
}

void registerjwtmodule(environment *env) {
    object *mod = makemodule("jwt", NULL);
    envset(mod->module.exports, "create", makebuiltin(jwtcreate), 0);
    envset(mod->module.exports, "encode", makebuiltin(jwtencode), 0);
    envset(mod->module.exports, "decode", makebuiltin(jwtdecode), 0);
    envset(env, "jwt", mod, 0);
}
