#include "../core/axton.h"
#include <sys/stat.h>

static char *packagedir = "packages";

static int mkdirp(char *path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);
    return system(cmd) == 0;
}

static int downloadfile(char *url, char *dest) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "curl -sL %s -o %s", url, dest);
    int ret = system(cmd);
    return ret == 0;
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
    if (strlen(name) == 0) {
        free(name);
        return NULL;
    }
    return name;
}

static char *getfilename(char *url) {
    char *last = strrchr(url, '/');
    if (!last) return NULL;
    char *name = strdup(last + 1);
    return name;
}

object *packageinstall(object **args, int argc, void *env) {
    if (argc < 1) throwexception("install needs url");
    char *url = args[0]->sval;
    mkdirp(packagedir);
    int isgit = (strstr(url, ".git") != NULL || strstr(url, "git@") != NULL);
    int ishttp = (strstr(url, "http://") == url || strstr(url, "https://") == url);
    char *name = NULL;
    if (isgit) {
        name = extractname(url);
        if (!name) throwexception("invalid git url");
        char dest[512];
        snprintf(dest, sizeof(dest), "%s/%s", packagedir, name);
        struct stat st;
        if (stat(dest, &st) == 0) {
            free(name);
            throwexception("package already installed");
        }
        if (!gitclone(url, dest)) {
            free(name);
            throwexception("git clone failed");
        }
    } else if (ishttp) {
        char *filename = getfilename(url);
        if (!filename) throwexception("invalid url");
        name = strdup(filename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = 0;
        char dest[512];
        snprintf(dest, sizeof(dest), "%s/%s.ax", packagedir, name);
        if (!downloadfile(url, dest)) {
            free(name);
            free(filename);
            throwexception("download failed");
        }
        free(filename);
    } else {
        char *filename = getfilename(url);
        if (!filename) throwexception("invalid file path");
        name = strdup(filename);
        char *dot = strrchr(name, '.');
        if (dot) *dot = 0;
        char dest[512];
        snprintf(dest, sizeof(dest), "%s/%s.ax", packagedir, name);
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "cp %s %s", url, dest);
        if (system(cmd) != 0) {
            free(name);
            free(filename);
            throwexception("copy failed");
        }
        free(filename);
    }
    object *pkg = packageload(name);
    free(name);
    return pkg;
}

object *packageload(char *name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.ax", packagedir, name);
    FILE *f = fopen(path, "r");
    if (!f) {
        snprintf(path, sizeof(path), "%s/%s/main.ax", packagedir, name);
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
            char *name = entry->d_name;
            if (name[0] == '.') continue;
            int len = strlen(name);
            if (len > 3 && strcmp(name + len - 3, ".ax") == 0) {
                name[len - 3] = 0;
                listappend(list, makestring(name));
            } else {
                struct stat st;
                char path[512];
                snprintf(path, sizeof(path), "%s/%s", packagedir, name);
                if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                    listappend(list, makestring(name));
                }
            }
        }
        closedir(d);
    }
    return list;
}

object *packageuninstall(object **args, int argc, void *env) {
    if (argc < 1) throwexception("uninstall needs package name");
    char *name = args[0]->sval;
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.ax", packagedir, name);
    if (remove(path) == 0) {
        return makenone();
    }
    snprintf(path, sizeof(path), "%s/%s", packagedir, name);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    if (system(cmd) == 0) {
        return makenone();
    }
    throwexception("package not found");
    return NULL;
}

void registerpackagelib(environment *env) {
    object *mod = makemodule("package", NULL);
    envset(mod->module.exports, "install", makebuiltin(packageinstall), 0);
    envset(mod->module.exports, "load", makebuiltin(packageload), 0);
    envset(mod->module.exports, "list", makebuiltin(packagelist), 0);
    envset(mod->module.exports, "uninstall", makebuiltin(packageuninstall), 0);
    envset(env, "package", mod, 0);
}
