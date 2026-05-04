#include "../core/axton.h"
#include <time.h>
#include <openssl/rand.h>

object *secrettokenbytes(object **args, int argc, void *env) {
    int n = (argc>0 && args[0]->type==0) ? args[0]->ival : 32;
    unsigned char *buf = malloc(n);
    RAND_bytes(buf, n);
    char *hex = malloc(n*2+1);
    for(int i=0;i<n;i++) snprintf(hex+i*2,3,"%02x",buf[i]);
    object *res = makestring(hex);
    free(buf); free(hex);
    return res;
}

object *secrettokenhex(object **args, int argc, void *env) {
    return secrettokenbytes(args, argc, env);
}

object *secretcompare(object **args, int argc, void *env) {
    if(argc<2) throwexception("compare needs two strings");
    char *a = args[0]->sval, *b = args[1]->sval;
    int diff = strlen(a)^strlen(b);
    for(int i=0;i<strlen(a) && i<strlen(b);i++) diff |= a[i]^b[i];
    return makebool(diff==0);
}

void registersecretslib(environment *env) {
    object *mod = makemodule("secrets", NULL);
    envset(mod->module.exports, "token_bytes", makebuiltin(secrettokenbytes), 0);
    envset(mod->module.exports, "token_hex", makebuiltin(secrettokenhex), 0);
    envset(mod->module.exports, "compare_digest", makebuiltin(secretcompare), 0);
    envset(env, "secrets", mod, 0);
}
