#include "../core/axton.h"

static char *packagedir = "packages";

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

object *packageload(char *name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/main.ax", packagedir, name);
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

object *packageinstall(char *giturl) {
    char *name = extractname(giturl);
    if (!name) {
        throwexception("invalid git url");
        return NULL;
    }
    char dest[512];
    snprintf(dest, sizeof(dest), "%s/%s", packagedir, name);
    struct stat st;
    if (stat(dest, &st) == 0) {
        free(name);
        throwexception("package already installed");
        return NULL;
    }
    mkdir(packagedir, 0755);
    if (!gitclone(giturl, dest)) {
        free(name);
        throwexception("git clone failed");
        return NULL;
    }
    object *pkg = packageload(name);
    free(name);
    return pkg;
}

object *packagelist(void) {
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

object *builtinpackageload(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("package.load needs name");
    return packageload(args[0]->sval);
}

object *builtinpackageinstall(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("package.install needs git url");
    return packageinstall(args[0]->sval);
}

object *builtinpackagelist(object **args, int argc, environment *env) {
    return packagelist();
}

void registerpackagelib(environment *env) {
    object *pkgmod = makemodule("package", NULL);
    envset(pkgmod->module.exports, "load", makebuiltin(builtinpackageload), 0);
    envset(pkgmod->module.exports, "install", makebuiltin(builtinpackageinstall), 0);
    envset(pkgmod->module.exports, "list", makebuiltin(builtinpackagelist), 0);
    envset(env, "package", pkgmod, 0);
}
