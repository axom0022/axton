#include "axton.h"
#include "../platform/platform.h"

environment *globalenv = NULL;
frame *currentframe = NULL;
int tcount = 0;
platformapi platform;

object *builtinhelp(object **args, int argc, environment *env) {
    platformlog("\n");
    platformlog("axton language\n");
    platformlog("  print(x)        print value\n");
    platformlog("  len(x)          get length\n");
    platformlog("  str(x)          to string\n");
    platformlog("  int(x)          to integer\n");
    platformlog("  float(x)        to float\n");
    platformlog("  input(prompt)   read input\n");
    platformlog("  range(stop)     create range\n");
    platformlog("  type(x)         get type\n");
    platformlog("  exit()          exit\n");
    platformlog("  help()          this help\n");
    platformlog("\n");
    return makenone();
}

void registerbuiltins(environment *env) {
    envset(env, "help", makebuiltin(builtinhelp), 0);
}

static void runfile(const char *path) {
    char *source = platformreadfile(path);
    if (!source) {
        platformlog("cannot open file\n");
        exit(1);
    }
    token *toks = tokenize(source);
    if (!toks) {
        platformlog("lexical error\n");
        free(source);
        exit(1);
    }
    stmt *prog = parsetokens(toks, tcount);
    if (!prog) {
        platformlog("parse error\n");
        free(source);
        exit(1);
    }
    frame frm;
    currentframe = &frm;
    if (setjmp(frm.jump) == 0) {
        evalprogram(prog, globalenv);
    } else {
        object *ex = catchexception();
        if (ex && ex->type == 2) {
            platformlog("error: ");
            platformlog(ex->sval);
            platformlog("\n");
        }
    }
    free(source);
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
        runfile(argv[1]);
    }
    return 0;
}
