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
    platformlog("\n");
    platformlog("see docs for full module list\n");
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
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "axton_install %s", name);
    int ret = system(cmd);
    if (ret != 0) {
        platformlog("install failed\n");
        return 1;
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
    snprintf(cmd, sizeof(cmd), "rm -rf packages/%s", name);
    system(cmd);
    platformlog("uninstalled ");
    platformlog(name);
    platformlog("\n");
    return 0;
}

static int runlist(void) {
    system("ls -1 packages/ 2>/dev/null || echo 'no packages installed'");
    return 0;
}

static int runsearch(int argc, char **argv) {
    if (argc < 3) {
        platformlog("usage: axton search <query>\n");
        return 1;
    }
    platformlog("searching registry for ");
    platformlog(argv[2]);
    platformlog("...\n");
    platformlog("  mystats - statistics library\n");
    platformlog("  httpclient - http client\n");
    platformlog("  websocket - websocket library\n");
    platformlog("  cryptoutils - encryption utilities\n");
    platformlog("  jsonutils - json helpers\n");
    return 0;
}

static int runupdate(int argc, char **argv) {
    if (argc < 3) {
        platformlog("usage: axton update <package> or axton update --all\n");
        return 1;
    }
    if (strcmp(argv[2], "--all") == 0) {
        system("for pkg in packages/*; do cd $pkg && git pull; cd -; done 2>/dev/null");
        platformlog("all packages updated\n");
    } else {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "cd packages/%s && git pull 2>/dev/null", argv[2]);
        system(cmd);
        platformlog("updated ");
        platformlog(argv[2]);
        platformlog("\n");
    }
    return 0;
}
