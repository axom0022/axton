#include "../core/axton.h"
#include <sys/stat.h>
#include <dirent.h>

typedef struct assetnode {
    char *name;
    char *path;
    char *type;
    int size;
    struct assetnode *next;
} assetnode;

typedef struct assetpipe {
    assetnode *assets;
    int count;
    char *inputdir;
    char *outputdir;
} assetpipe;

static assetpipe *ap = NULL;

object *assetpipecreate(object **args, int argc, void *env) {
    assetpipe *p = malloc(sizeof(assetpipe));
    p->assets = NULL;
    p->count = 0;
    p->inputdir = strdup("./assets");
    p->outputdir = strdup("./assets_out");
    ap = p;
    return makenative(p, NULL);
}

object *assetpipescan(object **args, int argc, void *env) {
    if (!ap) throwexception("assetpipe not created");
    DIR *d = opendir(ap->inputdir);
    if (!d) throwexception("cannot open inputdir");
    struct dirent *entry;
    while ((entry = readdir(d))) {
        if (entry->d_name[0] == '.') continue;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", ap->inputdir, entry->d_name);
        struct stat st;
        if (stat(path, &st) != 0) continue;
        assetnode *n = malloc(sizeof(assetnode));
        n->name = strdup(entry->d_name);
        n->path = strdup(path);
        char *ext = strrchr(entry->d_name, '.');
        n->type = ext ? strdup(ext + 1) : strdup("unknown");
        n->size = st.st_size;
        n->next = ap->assets;
        ap->assets = n;
        ap->count++;
    }
    closedir(d);
    return makenone();
}

object *assetpipeprocess(object **args, int argc, void *env) {
    if (!ap) throwexception("assetpipe not created");
    mkdir(ap->outputdir, 0755);
    assetnode *n = ap->assets;
    while (n) {
        char outpath[512];
        snprintf(outpath, sizeof(outpath), "%s/%s", ap->outputdir, n->name);
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "cp %s %s", n->path, outpath);
        system(cmd);
        n = n->next;
    }
    return makenone();
}

object *assetpipetexture(object **args, int argc, void *env) {
    if (argc < 1 || !ap) throwexception("texture needs path");
    return makenone();
}

object *assetpipemodel(object **args, int argc, void *env) {
    if (argc < 1 || !ap) throwexception("model needs path");
    return makenone();
}

void registerassetpip elib(environment *env) {
    object *mod = makemodule("assetpipe", NULL);
    envset(mod->module.exports, "create", makebuiltin(assetpipecreate), 0);
    envset(mod->module.exports, "scan", makebuiltin(assetpipescan), 0);
    envset(mod->module.exports, "process", makebuiltin(assetpipeprocess), 0);
    envset(mod->module.exports, "texture", makebuiltin(assetpipetexture), 0);
    envset(mod->module.exports, "model", makebuiltin(assetpipemodel), 0);
    envset(env, "assetpipe", mod, 0);
}
