#include "../core/axton.h"
#include <openssl/hmac.h>

object *hmacnew(object **a, int c, void *e) {
    if(c<2) throwexception("hmac needs key and msg");
    unsigned char *digest = HMAC(EVP_sha256(), (unsigned char*)a[0]->sval, strlen(a[0]->sval),
                                   (unsigned char*)a[1]->sval, strlen(a[1]->sval), NULL, NULL);
    char hex[65];
    for(int i=0;i<32;i++) snprintf(hex+i*2,3,"%02x",digest[i]);
    return makestring(hex);
}

void registerhmaclib(environment *env) {
    object *mod = makemodule("hmac", NULL);
    envset(mod->module.exports, "new", makebuiltin(hmacnew), 0);
    envset(env, "hmac", mod, 0);
}
