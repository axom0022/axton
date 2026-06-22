#include "../core/axton.h"
#include <sys/stat.h>

static char *packagedir = "packages";
static char *registryurl = "http://localhost:5000";
static char *registryfile = "registry.json";

static int mkdirp(char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);
    return system(cmd) == 0;
}

static char *downloadfile(char *url, char *dest) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "curl -s %s -o %s", url, dest);
    if (system(cmd) != 0) return NULL;
    return dest;
}

static char *getfromregistry(char *name, char *version) {
    char url[512];
    if (version) {
        snprintf(url, sizeof(url), "%s/download?name=%s&version=%s", registryurl, name, version);
    } else {
        snprintf(url, sizeof(url), "%s/download?name=%s", registryurl, name);
    }
    char dest[512];
    snprintf(dest, sizeof(dest), "%s/%s.ax", packagedir, name);
    if (downloadfile(url, dest)) return strdup(dest);
    return NULL;
}

object *packageinstall(object **args, int argc, void *env) {
    if (argc < 1) throwexception("install needs package name");
    char *name = args[0]->sval;
    char *version = NULL;
    if (argc > 1 && args[1]->type == 2) version = args[1]->sval;
    mkdirp(packagedir);
    char *path = getfromregistry(name, version);
    if (!path) throwexception("package not found");
    object *pkg = packageload(name);
    free(path);
    return pkg;
}

object *packageload(char *name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.ax", packagedir, name);
    FILE *f = fopen(path, "r");
    if (!f) {
        char msg[256];
        snprintf(msg, sizeof(msg), "package '%s' not installed", name);
        throwexception(msg);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *source = malloc(len + 1);
    fread(source, 1, len, f);
    source[len] = 0;
    fclose(f);
    token *toks = tokenize(source);
    stmt *prog = parsetokens(toks, tcount);
    environment *pkgenv = envnew(globalenv);
    evalprogram(prog, pkgenv);
    object *pkg = makemodule(name, NULL);
    pkg->module.exports = pkgenv;
    free(source);
    return pkg;
}

object *packagelist(object **args, int argc, void *env) {
    object *list = makelist();
    DIR *d = opendir(packagedir);
    if (d) {
        struct dirent *entry;
        while ((entry = readdir(d)) != NULL) {
            char *name = entry->d_name;
            int len = strlen(name);
            if (len > 3 && strcmp(name + len - 3, ".ax") == 0) {
                name[len - 3] = 0;
                listappend(list, makestring(name));
            }
        }
        closedir(d);
    }
    return list;
}

object *packagesearch(object **args, int argc, void *env) {
    if (argc < 1) throwexception("search needs query");
    char url[512];
    snprintf(url, sizeof(url), "%s/search?q=%s", registryurl, args[0]->sval);
    char dest[256];
    snprintf(dest, sizeof(dest), "/tmp/search_result.json");
    if (!downloadfile(url, dest)) return makelist();
    char *content = platformreadfile(dest);
    if (!content) return makelist();
    object *results = makelist();
    char *p = content;
    while ((p = strstr(p, "\"name\""))) {
        p = strchr(p, ':');
        if (!p) break;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '"') p++;
        char *end = strchr(p, '"');
        if (!end) break;
        int len = end - p;
        char *name = malloc(len + 1);
        memcpy(name, p, len);
        name[len] = 0;
        listappend(results, makestring(name));
        free(name);
        p = end;
    }
    free(content);
    return results;
}

object *packageuninstall(object **args, int argc, void *env) {
    if (argc < 1) throwexception("uninstall needs package name");
    char *name = args[0]->sval;
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.ax", packagedir, name);
    if (remove(path) == 0) {
        return makenone();
    }
    throwexception("package not found");
    return NULL;
}

object *packageupdate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("update needs package name");
    char *name = args[0]->sval;
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.ax", packagedir, name);
    if (remove(path) != 0) throwexception("package not installed");
    return packageinstall(args, argc, env);
}

object *packageupload(object **args, int argc, void *env) {
    if (argc < 3) throwexception("upload needs name version author");
    char *name = args[0]->sval;
    char *version = args[1]->sval;
    char *author = args[2]->sval;
    char path[512];
    snprintf(path, sizeof(path), "%s.ax", name);
    if (!platformreadfile(path)) throwexception("file not found");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "curl -s -X POST %s/upload -F \"name=%s\" -F \"version=%s\" -F \"author=%s\" -F \"file=@%s\"", registryurl, name, version, author, path);
    int ret = system(cmd);
    if (ret != 0) throwexception("upload failed");
    return makenone();
}

void registerpackagelib(environment *env) {
    object *mod = makemodule("package", NULL);
    envset(mod->module.exports, "install", makebuiltin(packageinstall), 0);
    envset(mod->module.exports, "load", makebuiltin(packageload), 0);
    envset(mod->module.exports, "list", makebuiltin(packagelist), 0);
    envset(mod->module.exports, "search", makebuiltin(packagesearch), 0);
    envset(mod->module.exports, "uninstall", makebuiltin(packageuninstall), 0);
    envset(mod->module.exports, "update", makebuiltin(packageupdate), 0);
    envset(mod->module.exports, "upload", makebuiltin(packageupload), 0);
    envset(env, "package", mod, 0);
}
