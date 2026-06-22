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
static int rununinstall(int argc, char **argv);
static int runlist(void);
static int runsearch(int argc, char **argv);
static int runupdate(int argc, char **argv);
static int runupload(int argc, char **argv);

object *builtinhelp(object **args, int argc, environment *env) {
    platformlog("axton language\n");
    platformlog("  print len str int float input\n");
    platformlog("  range type exit sleep time\n");
    platformlog("  readfile writefile help\n");
    platformlog("  import <module>      load module\n");
    platformlog("  --compile file.ax -> bytecode\n");
    platformlog("\n");
    platformlog("package manager:\n");
    platformlog("  axton install <pkg>   install package\n");
    platformlog("  axton uninstall <pkg> uninstall package\n");
    platformlog("  axton list            list installed packages\n");
    platformlog("  axton search <query>  search registry\n");
    platformlog("  axton update <pkg>    update package\n");
    platformlog("  axton update --all    update all packages\n");
    platformlog("  axton upload <file>   upload package to registry\n");
    platformlog("  axton registry        start registry server\n");
    platformlog("\n");
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
    if (strcmp(argv[1], "uninstall") == 0) {
        return rununinstall(argc, argv);
    }
    if (strcmp(argv[1], "list") == 0) {
        return runlist();
    }
    if (strcmp(argv[1], "search") == 0) {
        return runsearch(argc, argv);
    }
    if (strcmp(argv[1], "update") == 0) {
        return runupdate(argc, argv);
    }
    if (strcmp(argv[1], "upload") == 0) {
        return runupload(argc, argv);
    }
    if (strcmp(argv[1], "registry") == 0) {
        runfile("registry.ax");
        return 0;
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
        platformlog("usage: axton install <package>\n");
        return 1;
    }
    char *name = argv[2];
    char *version = (argc > 3) ? argv[3] : NULL;
    char code[1024];
    if (version) {
        snprintf(code, sizeof(code), "let p=require('package');p.install('%s','%s')", name, version);
    } else {
        snprintf(code, sizeof(code), "let p=require('package');p.install('%s')", name);
    }
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
    platformlog("installed ");
    platformlog(name);
    platformlog("\n");
    return 0;
}

static int rununinstall(int argc, char **argv) {
    if (argc < 3) {
        platformlog("usage: axton uninstall <package>\n");
        return 1;
    }
    char *name = argv[2];
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -f packages/%s.ax", name);
    system(cmd);
    platformlog("uninstalled ");
    platformlog(name);
    platformlog("\n");
    return 0;
}

static int runlist(void) {
    system("ls -1 packages/*.ax 2>/dev/null | sed 's/packages\\///' | sed 's/.ax$//' || echo 'no packages installed'");
    return 0;
}

static int runsearch(int argc, char **argv) {
    if (argc < 3) {
        platformlog("usage: axton search <query>\n");
        return 1;
    }
    char code[512];
    snprintf(code, sizeof(code), "let p=require('package');let r=p.search('%s');for i in r{print(i)}", argv[2]);
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

static int runupdate(int argc, char **argv) {
    if (argc < 3) {
        platformlog("usage: axton update <package> or axton update --all\n");
        return 1;
    }
    if (strcmp(argv[2], "--all") == 0) {
        system("for pkg in packages/*.ax; do name=$(basename $pkg .ax); axton update $name; done 2>/dev/null");
        platformlog("all packages updated\n");
    } else {
        char *name = argv[2];
        char code[512];
        snprintf(code, sizeof(code), "let p=require('package');p.update('%s')", name);
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
        platformlog("updated ");
        platformlog(name);
        platformlog("\n");
    }
    return 0;
}

static int runupload(int argc, char **argv) {
    if (argc < 5) {
        platformlog("usage: axton upload <file.ax> <name> <version> <author>\n");
        return 1;
    }
    char *file = argv[2];
    char *name = argv[3];
    char *version = argv[4];
    char *author = (argc > 5) ? argv[5] : "anonymous";
    char code[1024];
    snprintf(code, sizeof(code), "let p=require('package');p.upload('%s','%s','%s')", name, version, author);
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
    platformlog("uploaded ");
    platformlog(name);
    platformlog(" to registry\n");
    return 0;
}
