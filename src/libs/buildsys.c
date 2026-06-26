#include "../core/axton.h"
#include <sys/stat.h>
#include <dirent.h>

typedef struct buildtarget {
    char *name;
    char *source;
    char *output;
    char **deps;
    int depcount;
    struct buildtarget *next;
} buildtarget;

typedef struct buildsys {
    buildtarget *targets;
    int count;
    char *outputdir;
} buildsys;

static buildsys *bs = NULL;

object *buildsyscreate(object **args, int argc, void *env) {
    buildsys *b = malloc(sizeof(buildsys));
    b->targets = NULL;
    b->count = 0;
    b->outputdir = strdup("./bin");
    bs = b;
    return makenative(b, NULL);
}

object *buildsysaddtarget(object **args, int argc, void *env) {
    if (argc < 3) throwexception("addtarget needs name source output");
    if (!bs) throwexception("buildsys not created");
    buildtarget *t = malloc(sizeof(buildtarget));
    t->name = strdup(args[0]->sval);
    t->source = strdup(args[1]->sval);
    t->output = strdup(args[2]->sval);
    t->deps = NULL;
    t->depcount = 0;
    if (argc > 3 && args[3]->type == 5) {
        t->depcount = args[3]->list.count;
        t->deps = malloc(t->depcount * sizeof(char*));
        for (int i = 0; i < t->depcount; i++) {
            t->deps[i] = strdup(args[3]->list.items[i]->sval);
        }
    }
    t->next = bs->targets;
    bs->targets = t;
    bs->count++;
    return makenone();
}

object *buildsysbuild(object **args, int argc, void *env) {
    if (!bs) throwexception("buildsys not created");
    buildtarget *t = bs->targets;
    while (t) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gcc -o %s %s", t->output, t->source);
        if (t->depcount > 0) {
            strcat(cmd, " ");
            for (int i = 0; i < t->depcount; i++) {
                strcat(cmd, t->deps[i]);
                if (i < t->depcount - 1) strcat(cmd, " ");
            }
        }
        system(cmd);
        t = t->next;
    }
    return makenone();
}

object *buildsyssetoutputdir(object **args, int argc, void *env) {
    if (argc < 1 || !bs) throwexception("setoutputdir needs path");
    free(bs->outputdir);
    bs->outputdir = strdup(args[0]->sval);
    return makenone();
}

void registerbuildsyslib(environment *env) {
    object *mod = makemodule("buildsys", NULL);
    envset(mod->module.exports, "create", makebuiltin(buildsyscreate), 0);
    envset(mod->module.exports, "addtarget", makebuiltin(buildsysaddtarget), 0);
    envset(mod->module.exports, "build", makebuiltin(buildsysbuild), 0);
    envset(mod->module.exports, "setoutputdir", makebuiltin(buildsyssetoutputdir), 0);
    envset(env, "buildsys", mod, 0);
}
