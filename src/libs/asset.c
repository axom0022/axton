#include "../core/axton.h"
#include <string.h>
#include <sys/stat.h>

typedef struct assetbundle {
    char *name;
    unsigned char *data;
    int size;
    int refcount;
    struct assetbundle *next;
} assetbundle;

typedef struct assetcache {
    assetbundle *bundles;
    int count;
} assetcache;

static assetcache *cache = NULL;

object *assetinit(object **args, int argc, void *env) {
    cache = malloc(sizeof(assetcache));
    cache->bundles = NULL;
    cache->count = 0;
    return makenone();
}

object *assetload(object **args, int argc, void *env) {
    if (argc<1) throwexception("load needs path");
    char *path = args[0]->sval;
    struct stat st;
    if (stat(path, &st) != 0) throwexception("file not found");
    FILE *f = fopen(path, "rb");
    if (!f) throwexception("cannot open");
    unsigned char *data = malloc(st.st_size);
    fread(data, 1, st.st_size, f);
    fclose(f);
    assetbundle *b = malloc(sizeof(assetbundle));
    b->name = strdup(path);
    b->data = data;
    b->size = st.st_size;
    b->refcount = 1;
    b->next = cache->bundles;
    cache->bundles = b;
    cache->count++;
    return makenative(b, NULL);
}

object *assetgetdata(object **args, int argc, void *env) {
    if (argc<1) throwexception("getdata needs asset");
    assetbundle *b = (assetbundle*)args[0]->native.data;
    return makestring((char*)b->data);
}

object *assetgetsize(object **args, int argc, void *env) {
    if (argc<1) throwexception("getsize needs asset");
    assetbundle *b = (assetbundle*)args[0]->native.data;
    return makeint(b->size);
}

object *assetunload(object **args, int argc, void *env) {
    if (argc<1) throwexception("unload needs asset");
    assetbundle *b = (assetbundle*)args[0]->native.data;
    if (--b->refcount == 0) {
        assetbundle *prev = NULL, *cur = cache->bundles;
        while (cur && cur != b) { prev = cur; cur = cur->next; }
        if (cur) {
            if (prev) prev->next = cur->next;
            else cache->bundles = cur->next;
        }
        free(b->data); free(b->name); free(b);
        cache->count--;
    }
    return makenone();
}

object *assetloadtexture(object **args, int argc, void *env) {
    if (argc<1) throwexception("loadtexture needs path");
    char *path = args[0]->sval;
    return makeint(1);
}

object *assetloadmesh(object **args, int argc, void *env) {
    if (argc<1) throwexception("loadmesh needs path");
    char *path = args[0]->sval;
    return makenone();
}

void registerassetlib(environment *env) {
    object *mod = makemodule("asset", NULL);
    envset(mod->module.exports, "init", makebuiltin(assetinit), 0);
    envset(mod->module.exports, "load", makebuiltin(assetload), 0);
    envset(mod->module.exports, "getdata", makebuiltin(assetgetdata), 0);
    envset(mod->module.exports, "getsize", makebuiltin(assetgetsize), 0);
    envset(mod->module.exports, "unload", makebuiltin(assetunload), 0);
    envset(mod->module.exports, "loadtexture", makebuiltin(assetloadtexture), 0);
    envset(mod->module.exports, "loadmesh", makebuiltin(assetloadmesh), 0);
    envset(env, "asset", mod, 0);
}
