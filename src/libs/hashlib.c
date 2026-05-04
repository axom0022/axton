#include "../core/axton.h"
#include <openssl/md5.h>
#include <openssl/sha.h>

static char *tohex(unsigned char *d, int len) {
    char *hex = malloc(len * 2 + 1);
    for (int i = 0; i < len; i++) snprintf(hex + i*2, 3, "%02x", d[i]);
    return hex;
}

object *hashmd5(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 2) throwexception("md5 needs string");
    unsigned char d[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)a[0]->sval, strlen(a[0]->sval), d);
    char *hex = tohex(d, MD5_DIGEST_LENGTH);
    object *res = makestring(hex);
    free(hex);
    return res;
}

object *hashsha1(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 2) throwexception("sha1 needs string");
    unsigned char d[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)a[0]->sval, strlen(a[0]->sval), d);
    char *hex = tohex(d, SHA_DIGEST_LENGTH);
    object *res = makestring(hex);
    free(hex);
    return res;
}

object *hashsha256(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 2) throwexception("sha256 needs string");
    unsigned char d[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)a[0]->sval, strlen(a[0]->sval), d);
    char *hex = tohex(d, SHA256_DIGEST_LENGTH);
    object *res = makestring(hex);
    free(hex);
    return res;
}

object *hashsha512(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 2) throwexception("sha512 needs string");
    unsigned char d[SHA512_DIGEST_LENGTH];
    SHA512((unsigned char*)a[0]->sval, strlen(a[0]->sval), d);
    char *hex = tohex(d, SHA512_DIGEST_LENGTH);
    object *res = makestring(hex);
    free(hex);
    return res;
}

void registerhashliblib(environment *env) {
    object *mod = makemodule("hashlib", NULL);
    envset(mod->module.exports, "md5", makebuiltin(hashmd5), 0);
    envset(mod->module.exports, "sha1", makebuiltin(hashsha1), 0);
    envset(mod->module.exports, "sha256", makebuiltin(hashsha256), 0);
    envset(mod->module.exports, "sha512", makebuiltin(hashsha512), 0);
    envset(env, "hashlib", mod, 0);
}
