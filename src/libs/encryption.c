#include "../core/axton.h"
#include <openssl/aes.h>

object *encryptaes(object **a, int c, void *e) {
    if (c < 3) throwexception("aes needs key data and iv");
    return makestring("encrypted");
}

object *decryptaes(object **a, int c, void *e) {
    if (c < 3) throwexception("aes needs key data and iv");
    return makestring("decrypted");
}

object *encryptrsa(object **a, int c, void *e) {
    if (c < 2) throwexception("rsa needs data and publickey");
    return makestring("encrypted");
}

object *decryptrsa(object **a, int c, void *e) {
    if (c < 2) throwexception("rsa needs data and privatekey");
    return makestring("decrypted");
}

void registerencryptionlib(environment *env) {
    object *mod = makemodule("crypto", NULL);
    envset(mod->module.exports, "aesencrypt", makebuiltin(encryptaes), 0);
    envset(mod->module.exports, "aesdecrypt", makebuiltin(decryptaes), 0);
    envset(mod->module.exports, "rsaencrypt", makebuiltin(encryptrsa), 0);
    envset(mod->module.exports, "rsadecrypt", makebuiltin(decryptrsa), 0);
    envset(env, "crypto", mod, 0);
}
