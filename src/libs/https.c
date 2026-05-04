#include "../core/axton.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

static SSL_CTX *sslctx = NULL;

object *httpscreateserver(int port) {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    sslctx = SSL_CTX_new(TLS_server_method());
    if (!sslctx) {
        throwexception("failed to create SSL context");
        return NULL;
    }
    SSL_CTX_set_ecdh_auto(sslctx, 1);
    char certpath[256];
    char keypath[256];
    snprintf(certpath, sizeof(certpath), "cert.pem");
    snprintf(keypath, sizeof(keypath), "key.pem");
    if (SSL_CTX_use_certificate_file(sslctx, certpath, SSL_FILETYPE_PEM) <= 0) {
        throwexception("failed to load certificate");
        return NULL;
    }
    if (SSL_CTX_use_PrivateKey_file(sslctx, keypath, SSL_FILETYPE_PEM) <= 0) {
        throwexception("failed to load private key");
        return NULL;
    }
    return httpcreateserver(port);
}

object *httpsstart(void) {
    return httpstart();
}

void registerhttpslib(environment *env) {
    object *httpsmod = makemodule("https", NULL);
    envset(httpsmod->module.exports, "createserver", makebuiltin(httpscreateserver), 0);
    envset(httpsmod->module.exports, "start", makebuiltin(httpsstart), 0);
    envset(env, "https", httpsmod, 0);
}
