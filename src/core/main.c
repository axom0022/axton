#include "axton.h"
#include "bytecode.h"
#include "compiler.h"
#include "vm.h"
#include <string.h>

environment *globalenv = NULL;
frame *currentframe = NULL;
int tcount = 0;
platformapi platform;

static int runinstall(int argc, char **argv);
static int runlist(void);
static int rununinstall(int argc, char **argv);

object *builtinhelp(object **args, int argc, environment *env) {
    platformlog("axton language\n");
    platformlog("  print len str int float input\n");
    platformlog("  range type exit sleep time\n");
    platformlog("  readfile writefile help\n");
    platformlog("  import <module>      load module\n");
    platformlog("  --compile file.ax -> bytecode\n");
    platformlog("\n");
    platformlog("package manager:\n");
    platformlog("  axton install <url>   install from git, http, or file\n");
    platformlog("  axton list            list installed packages\n");
    platformlog("  axton uninstall <pkg> uninstall package\n");
    return makenone();
}

void registerbuiltins(environment *env) {
    envset(env, "help", makebuiltin(builtinhelp), 0);
}

int main(int argc, char **argv) {
    platforminit();
    gcinit();
    srand(time(NULL));
    globalenv = envnew(NULL);
    globalenv->globals = globalenv;
    registerbuiltins(globalenv);
    registerstdlib(globalenv);
    registeralllibs(globalenv);
    initexceptions(globalenv);

    if (argc < 2) {
        replstart();
        return 0;
    }

    if (strcmp(argv[1], "install") == 0) {
        return runinstall(argc, argv);
    }
    if (strcmp(argv[1], "list") == 0) {
        return runlist();
    }
    if (strcmp(argv[1], "uninstall") == 0) {
        return rununinstall(argc, argv);
    }
    if (strcmp(argv[1], "--compile") == 0 && argc > 2) {
        runfile(argv[2]);
        return 0;
    }

    runfile(argv[1]);
    return 0;
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

static int runinstall(int argc, char **argv) {
    if (argc < 3) {
        platformlog("usage: axton install <url>\n");
        platformlog("  git: https://github.com/user/repo.git\n");
        platformlog("  http: https://example.com/lib.ax\n");
        platformlog("  file: /path/to/lib.ax\n");
        return 1;
    }
    char code[512];
    snprintf(code, sizeof(code), "let p=require('package');p.install('%s')", argv[2]);
    token *toks = tokenize(code);
    stmt *prog = parsetokens(toks, tcount);
    frame frm;
    currentframe = &frm;
    if (setjmp(frm.jump) == 0) {
        evalprogram(prog, globalenv);
    } else {
        object *ex = catchexception();
        if (ex && ex->type == 2) { platformlog("error: "); platformlog(ex->sval); platformlog("\n"); return 1; }
    }
    return 0;
}

static int runlist(void) {
    system("ls -1 packages/ 2>/dev/null || echo 'no packages installed'");
    return 0;
}

static int rununinstall(int argc, char **argv) {
    if (argc < 3) {
        platformlog("usage: axton uninstall <package>\n");
        return 1;
    }
    char code[512];
    snprintf(code, sizeof(code), "let p=require('package');p.uninstall('%s')", argv[2]);
    token *toks = tokenize(code);
    stmt *prog = parsetokens(toks, tcount);
    frame frm;
    currentframe = &frm;
    if (setjmp(frm.jump) == 0) {
        evalprogram(prog, globalenv);
    } else {
        object *ex = catchexception();
        if (ex && ex->type == 2) { platformlog("error: "); platformlog(ex->sval); platformlog("\n"); return 1; }
    }
    return 0;
}
