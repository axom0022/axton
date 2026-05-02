#include "axton.h"
#include "../platform/platform.h"

environment *globalenv = NULL;
frame *currentframe = NULL;
int tcount = 0;
platform_api platform;

object *builtin_help(object **args, int argc, environment *env) {
    platform_log("\n");
    platform_log("axton language\n");
    platform_log("  print(x)        print value\n");
    platform_log("  len(x)          get length\n");
    platform_log("  str(x)          to string\n");
    platform_log("  int(x)          to integer\n");
    platform_log("  float(x)        to float\n");
    platform_log("  input(prompt)   read input\n");
    platform_log("  range(stop)     create range\n");
    platform_log("  type(x)         get type\n");
    platform_log("  exit()          exit\n");
    platform_log("  help()          this help\n");
    platform_log("\n");
    return make_none();
}

void register_builtins(environment *env) {
    env_set(env, "help", make_builtin(builtin_help), 0);
}

static void run_file(const char *path) {
    char *source = platform_read_file(path);
    if (!source) {
        platform_log("cannot open file\n");
        exit(1);
    }
    token *toks = tokenize(source);
    if (!toks) {
        platform_log("lexical error\n");
        free(source);
        exit(1);
    }
    stmt *prog = parse_tokens(toks, tcount);
    if (!prog) {
        platform_log("parse error\n");
        free(source);
        exit(1);
    }
    frame frm;
    currentframe = &frm;
    if (setjmp(frm.jump) == 0) {
        eval_program(prog, globalenv);
    } else {
        object *ex = catch_exception();
        if (ex && ex->type == 2) {
            platform_log("error: ");
            platform_log(ex->sval);
            platform_log("\n");
        }
    }
    free(source);
}

int main(int argc, char **argv) {
    platform_init();
    gc_init();
    srand(time(NULL));
    globalenv = env_new(NULL);
    globalenv->globals = globalenv;
    register_builtins(globalenv);
    register_stdlib(globalenv);
    init_exceptions(globalenv);
    
    if (argc < 2) {
        repl_start();
    } else {
        run_file(argv[1]);
    }
    return 0;
}
