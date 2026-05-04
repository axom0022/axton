#include "axton.h"
#include "bytecode.h"
#include "compiler.h"
#include "vm.h"
#include <string.h>

environment *globalenv = NULL;
frame *currentframe = NULL;
int tcount = 0;
platformapi platform;

object *builtinhelp(object **args, int argc, environment *env) {
    platformlog("axton language\n");
    platformlog("  print len str int float input\n");
    platformlog("  range type exit sleep time\n");
    platformlog("  readfile writefile help\n");
    platformlog("  --compile file.ax -> bytecode\n");
    return makenone();
}

void registerbuiltins(environment *env) {
    envset(env, "help", makebuiltin(builtinhelp), 0);
}

static bytecode *currentbytecode = NULL;

static void compiletofile(bytecode *bc, char *outpath) {
    FILE *f = fopen(outpath, "wb");
    if (!f) { platformlog("cannot write bytecode\n"); return; }
    fwrite(&bc->size, sizeof(int), 1, f);
    fwrite(bc->code, 1, bc->size, f);
    fwrite(&bc->constcount, sizeof(int), 1, f);
    for (int i = 0; i < bc->constcount; i++) {
        object *c = bc->constants[i];
        int type = c->type;
        fwrite(&type, sizeof(int), 1, f);
        if (type == 0) fwrite(&c->ival, sizeof(long), 1, f);
        else if (type == 1) fwrite(&c->fval, sizeof(double), 1, f);
        else if (type == 2) { int len = strlen(c->sval); fwrite(&len, sizeof(int), 1, f); fwrite(c->sval, 1, len, f); }
        else if (type == 3) fwrite(&c->bval, sizeof(int), 1, f);
    }
    fwrite(&bc->namecount, sizeof(int), 1, f);
    for (int i = 0; i < bc->namecount; i++) { int len = strlen(bc->names[i]); fwrite(&len, sizeof(int), 1, f); fwrite(bc->names[i], 1, len, f); }
    fclose(f);
    platformlog("bytecode written to "); platformlog(outpath); platformlog("\n");
}

static bytecode *loadbytecodefromfile(char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    bytecode *bc = bytecodenew();
    fread(&bc->size, sizeof(int), 1, f);
    bc->code = realloc(bc->code, bc->size);
    fread(bc->code, 1, bc->size, f);
    fread(&bc->constcount, sizeof(int), 1, f);
    bc->constants = malloc(bc->constcount * sizeof(object*));
    for (int i = 0; i < bc->constcount; i++) {
        int type; fread(&type, sizeof(int), 1, f);
        if (type == 0) { long v; fread(&v, sizeof(long), 1, f); bc->constants[i] = makeint(v); }
        else if (type == 1) { double v; fread(&v, sizeof(double), 1, f); bc->constants[i] = makefloat(v); }
        else if (type == 2) { int len; fread(&len, sizeof(int), 1, f); char *s = malloc(len+1); fread(s, 1, len, f); s[len]=0; bc->constants[i] = makestring(s); free(s); }
        else if (type == 3) { int v; fread(&v, sizeof(int), 1, f); bc->constants[i] = makebool(v); }
        else bc->constants[i] = makenone();
    }
    fread(&bc->namecount, sizeof(int), 1, f);
    bc->names = malloc(bc->namecount * sizeof(char*));
    for (int i = 0; i < bc->namecount; i++) { int len; fread(&len, sizeof(int), 1, f); char *s = malloc(len+1); fread(s, 1, len, f); s[len]=0; bc->names[i]=s; }
    fclose(f);
    return bc;
}

static void runfile(char *path) {
    char *sourcename = path;
    int len = strlen(path);
    int isbytecode = (len > 4 && strcmp(path + len - 4, ".axc") == 0);
    bytecode *bc = NULL;
    if (isbytecode) {
        bc = loadbytecodefromfile(path);
        if (!bc) { platformlog("failed to load bytecode\n"); return; }
    } else {
        char *source = platformreadfile(path);
        if (!source) { platformlog("cannot read file\n"); return; }
        token *toks = tokenize(source);
        if (!toks) { platformlog("lexical error\n"); free(source); return; }
        stmt *prog = parsetokens(toks, tcount);
        if (!prog) { platformlog("parse error\n"); free(source); return; }
        bc = bytecodenew();
        compileprogram(prog, bc);
        free(source);
        char outpath[512];
        snprintf(outpath, sizeof(outpath), "%s.axc", sourcename);
        if (access(outpath, F_OK) == 0) unlink(outpath);
        compiletofile(bc, outpath);
    }
    frame frm;
    currentframe = &frm;
    if (setjmp(frm.jump) == 0) {
        executebytecode(bc, globalenv);
    } else {
        object *ex = catchexception();
        if (ex && ex->type == 2) { platformlog("error: "); platformlog(ex->sval); platformlog("\n"); }
    }
    bytecodefree(bc);
}

int main(int argc, char **argv) {
    platforminit();
    gcinit();
    srand(time(NULL));
    globalenv = envnew(NULL);
    globalenv->globals = globalenv;
    registerbuiltins(globalenv);
    registerstdlib(globalenv);
    initexceptions(globalenv);
    if (argc < 2) {
        replstart();
    } else {
        if (strcmp(argv[1], "--compile") == 0 && argc > 2) {
            runfile(argv[2]);
        } else {
            runfile(argv[1]);
        }
    }
    return 0;
}
