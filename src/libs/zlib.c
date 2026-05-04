#include "../core/axton.h"
#include <zlib.h>

object *zlibcompress(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=2) throwexception("compress needs string");
    uLong len = compressBound(strlen(a[0]->sval));
    Bytef *out = malloc(len);
    uLong outlen = len;
    if(compress(out, &outlen, (Bytef*)a[0]->sval, strlen(a[0]->sval)) != Z_OK){
        free(out);
        throwexception("compress failed");
    }
    char *result = malloc(outlen+1);
    memcpy(result, out, outlen);
    result[outlen]=0;
    free(out);
    return makestring(result);
}

object *zlibdecompress(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=2) throwexception("decompress needs string");
    uLong len = strlen(a[0]->sval) * 4;
    Bytef *out = malloc(len);
    uLong outlen = len;
    int ret = uncompress(out, &outlen, (Bytef*)a[0]->sval, strlen(a[0]->sval));
    if(ret == Z_BUF_ERROR){
        len *= 2;
        out = realloc(out, len);
        outlen = len;
        ret = uncompress(out, &outlen, (Bytef*)a[0]->sval, strlen(a[0]->sval));
    }
    if(ret != Z_OK){
        free(out);
        throwexception("decompress failed");
    }
    char *result = malloc(outlen+1);
    memcpy(result, out, outlen);
    result[outlen]=0;
    free(out);
    return makestring(result);
}

void registerzliblib(environment *env) {
    object *mod = makemodule("zlib", NULL);
    envset(mod->module.exports, "compress", makebuiltin(zlibcompress), 0);
    envset(mod->module.exports, "decompress", makebuiltin(zlibdecompress), 0);
    envset(env, "zlib", mod, 0);
}
