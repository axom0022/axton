#include "../core/axton.h"
#include <sys/stat.h>

static void copyfile(const char *src, const char *dst) {
    FILE *fs = fopen(src, "rb");
    if(!fs) throwexception("source not found");
    FILE *fd = fopen(dst, "wb");
    if(!fd){ fclose(fs); throwexception("cannot create destination"); }
    char buf[8192];
    size_t n;
    while((n=fread(buf,1,sizeof(buf),fs))>0) fwrite(buf,1,n,fd);
    fclose(fs); fclose(fd);
}

object *shutil_copy(object **args, int argc, void *env) {
    if(argc<2) throwexception("copy needs src and dst");
    copyfile(args[0]->sval, args[1]->sval);
    return makestring(args[1]->sval);
}

object *shutil_copytree(object **args, int argc, void *env) {
    if(argc<2) throwexception("copytree needs src and dst");
    char mkcmd[512];
    snprintf(mkcmd, sizeof(mkcmd), "mkdir -p %s", args[1]->sval);
    system(mkcmd);
    char cpcmd[512];
    snprintf(cpcmd, sizeof(cpcmd), "cp -r %s/* %s/", args[0]->sval, args[1]->sval);
    system(cpcmd);
    return makestring(args[1]->sval);
}

object *shutil_rmtree(object **args, int argc, void *env) {
    if(argc<1) throwexception("rmtree needs path");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", args[0]->sval);
    system(cmd);
    return makenone();
}

object *shutil_move(object **args, int argc, void *env) {
    if(argc<2) throwexception("move needs src and dst");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mv %s %s", args[0]->sval, args[1]->sval);
    system(cmd);
    return makestring(args[1]->sval);
}

void registershutillib(environment *env) {
    object *mod = makemodule("shutil", NULL);
    envset(mod->module.exports, "copy", makebuiltin(shutil_copy), 0);
    envset(mod->module.exports, "copytree", makebuiltin(shutil_copytree), 0);
    envset(mod->module.exports, "rmtree", makebuiltin(shutil_rmtree), 0);
    envset(mod->module.exports, "move", makebuiltin(shutil_move), 0);
    envset(env, "shutil", mod, 0);
}
