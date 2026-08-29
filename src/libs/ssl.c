#include "../core/axton.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

object *sslcreate(object **args, int argc, void *env) {
    platformsslinit();
    SSL_CTX *ctx = platformsslctxnew();
    if (!ctx) throwexception("ssl context failed");
    SSL *ssl = platformsslnew(ctx);
    if (!ssl) { platformssldestroy(ctx); throwexception("ssl object failed"); }
    return makesslconn(ssl, ctx, -1);
}

object *sslconnect(object **args, int argc, void *env) {
    if (argc < 2) throwexception("connect needs ssl fd");
    object *sslobj = args[0];
    int fd = args[1]->ival;
    SSL *ssl = sslobj->sslconn.ssl;
    if (platformsslconnect(ssl, fd) != 1) throwexception("ssl handshake failed");
    sslobj->sslconn.fd = fd;
    return makenone();
}

object *sslwrite(object **args, int argc, void *env) {
    if (argc < 2) throwexception("write needs ssl data");
    object *sslobj = args[0];
    char *data = args[1]->sval;
    SSL *ssl = sslobj->sslconn.ssl;
    int n = platformsslwrite(ssl, data, strlen(data));
    if (n < 0) throwexception("ssl write failed");
    return makeint(n);
}

object *sslread(object **args, int argc, void *env) {
    if (argc < 2) throwexception("read needs ssl size");
    object *sslobj = args[0];
    int size = args[1]->ival;
    SSL *ssl = sslobj->sslconn.ssl;
    char *buf = malloc(size + 1);
    int n = platformsslread(ssl, buf, size);
    if (n < 0) { free(buf); throwexception("ssl read failed"); }
    buf[n] = 0;
    object *res = makestring(buf);
    free(buf);
    return res;
}

void registerssllib(environment *env) {
    object *mod = makemodule("ssl", NULL);
    envset(mod->module.exports, "create", makebuiltin(sslcreate), 0);
    envset(mod->module.exports, "connect", makebuiltin(sslconnect), 0);
    envset(mod->module.exports, "write", makebuiltin(sslwrite), 0);
    envset(mod->module.exports, "read", makebuiltin(sslread), 0);
    envset(env, "ssl", mod, 0);
}
