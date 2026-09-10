#include "../core/axton.h"

object *ciphercreate(object **args, int argc, void *env) {
    void *cipher = platformciphercreate();
    if (!cipher) throwexception("cipher create failed");
    return makecipher(cipher);
}

object *cipherencrypt(object **args, int argc, void *env) {
    if (argc < 2) throwexception("encrypt needs data");
    object *cipher = args[0];
    char *data = args[1]->sval;
    platformcipherencrypt(cipher->cipher.cipher, data, strlen(data));
    return makenone();
}

object *cipherdecrypt(object **args, int argc, void *env) {
    if (argc < 2) throwexception("decrypt needs data");
    object *cipher = args[0];
    char *data = args[1]->sval;
    platformcipherdecrypt(cipher->cipher.cipher, data, strlen(data));
    return makenone();
}

void registerciphermodule(environment *env) {
    object *mod = makemodule("cipher", NULL);
    envset(mod->module.exports, "create", makebuiltin(ciphercreate), 0);
    envset(mod->module.exports, "encrypt", makebuiltin(cipherencrypt), 0);
    envset(mod->module.exports, "decrypt", makebuiltin(cipherdecrypt), 0);
    envset(env, "cipher", mod, 0);
}
