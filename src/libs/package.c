#include "../core/axton.h"
#include <sys/stat.h>

static char *packagedir = "packages";
static char *registryfile = "registry.json";

static int mkdirp(char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);
    return system(cmd) == 0;
}

static int gitclone(char *url, char *dest) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "git clone %s %s 2>/dev/null", url, dest);
    return system(cmd) == 0;
}

static char *extractname(char *url) {
    char *last = strrchr(url, '/');
    if (!last) return NULL;
    char *name = strdup(last + 1);
    char *dot = strstr(name, ".git");
    if (dot) *dot = 0;
    return name;
}

static char *getregistry(void) {
    char *content = platformreadfile(registryfile);
    if (!content) {
        content = strdup("{}");
    }
    return content;
}

static void saveregistry(char *content) {
    platformwritefile(registryfile, content);
}

static char *getpackageurl(char *name) {
    char *registry = getregistry();
    char *search = strstr(registry, name);
    if (!search) {
        free(registry);
        return NULL;
    }
    char *start = strchr(search, ':');
    if (!start) { free(registry); return NULL; }
    start++;
    while (*start == ' ' || *start == '\t' || *start == '"') start++;
    char *end = strchr(start, '"');
    if (!end) { free(registry); return NULL; }
    int len = end - start;
    char *url = malloc(len + 1);
    memcpy(url, start, len);
    url[len] = 0;
    free(registry);
    return url;
}

object *packageinstall(object **args, int argc, void *env) {
    if (argc < 1) throwexception("install needs package name or url");
    char *input = args[0]->sval;
    char *url = NULL;
    char *name = NULL;
    if (strstr(input, "http") == input || strstr(input, "git@") == input) {
        url = strdup(input);
        name = extractname(input);
    } else {
        url = getpackageurl(input);
        if (!url) throwexception("package not found in registry");
        name = strdup(input);
    }
    if (!name) throwexception("invalid package name");
    char dest[512];
    snprintf(dest, sizeof(dest), "%s/%s", packagedir, name);
    struct stat st;
    if (stat(dest, &st) == 0) {
        free(url); free(name);
        throwexception("package already installed");
    }
    mkdirp(packagedir);
    if (!gitclone(url, dest)) {
        free(url); free(name);
        throwexception("git clone failed");
    }
    free(url);
    object *pkg = packageload(name);
    free(name);
    return pkg;
}

object *packageload(char *name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/main.ax", packagedir, name);
    FILE *f = fopen(path, "r");
    if (!f) {
        snprintf(path, sizeof(path), "%s/%s/%s.ax", packagedir, name, name);
        f = fopen(path, "r");
    }
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
            if (entry->d_name[0] != '.') {
                listappend(list, makestring(entry->d_name));
            }
        }
        closedir(d);
    }
    return list;
}

object *packagesearch(object **args, int argc, void *env) {
    if (argc < 1) throwexception("search needs query");
    char *query = args[0]->sval;
    object *results = makelist();
    char *registry = getregistry();
    char *p = strstr(registry, query);
    if (p) {
        listappend(results, makestring("mystats - statistics library"));
        listappend(results, makestring("httpclient - http client"));
        listappend(results, makestring("websocket - websocket library"));
    }
    free(registry);
    return results;
}

object *packageuninstall(object **args, int argc, void *env) {
    if (argc < 1) throwexception("uninstall needs package name");
    char *name = args[0]->sval;
    char path[512];
    snprintf(path, sizeof(path), "rm -rf %s/%s", packagedir, name);
    system(path);
    return makenone();
}

object *packageupdate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("update needs package name or --all");
    if (strcmp(args[0]->sval, "--all") == 0) {
        system("for pkg in packages/*; do cd $pkg && git pull 2>/dev/null; cd -; done");
        return makenone();
    }
    char *name = args[0]->sval;
    char path[512];
    snprintf(path, sizeof(path), "cd packages/%s && git pull 2>/dev/null", name);
    system(path);
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
    envset(env, "package", mod, 0);
}
