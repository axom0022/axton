#include "../core/axton.h"
#include <openssl/md5.h>
#include <openssl/sha.h>

static char *tohex(unsigned char *digest, int len) {
    char *hex = malloc(len * 2 + 1);
    for (int i = 0; i < len; i++) snprintf(hex + i * 2, 3, "%02x", digest[i]);
    return hex;
}

object *hashlib_md5(object **args, int argc, void *env) {
    if (argc < 1 || args[0]->type != 2) throwexception("md5 needs string");
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)args[0]->sval, strlen(args[0]->sval), digest);
    char *hex = tohex(digest, MD5_DIGEST_LENGTH);
    object *res = makestring(hex);
    free(hex);
    return res;
}

object *hashlib_sha1(object **args, int argc, void *env) {
    if (argc < 1 || args[0]->type != 2) throwexception("sha1 needs string");
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)args[0]->sval, strlen(args[0]->sval), digest);
    char *hex = tohex(digest, SHA_DIGEST_LENGTH);
    object *res = makestring(hex);
    free(hex);
    return res;
}

object *hashlib_sha256(object **args, int argc, void *env) {
    if (argc < 1 || args[0]->type != 2) throwexception("sha256 needs string");
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)args[0]->sval, strlen(args[0]->sval), digest);
    char *hex = tohex(digest, SHA256_DIGEST_LENGTH);
    object *res = makestring(hex);
    free(hex);
    return res;
}

void registerhashliblib(environment *env) {
    object *mod = makemodule("hashlib", NULL);
    envset(mod->module.exports, "md5", makebuiltin(hashlib_md5), 0);
    envset(mod->module.exports, "sha1", makebuiltin(hashlib_sha1), 0);
    envset(mod->module.exports, "sha256", makebuiltin(hashlib_sha256), 0);
    envset(env, "hashlib", mod, 0);
}
