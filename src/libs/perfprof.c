#include "../core/axton.h"
#include <time.h>

typedef struct perfentry {
    char *name;
    long calls;
    double totaltime;
    double mintime;
    double maxtime;
    struct perfentry *next;
} perfentry;

typedef struct perfprof {
    int enabled;
    perfentry *entries;
    int count;
    double starttime;
    char *currentname;
    double currentstart;
} perfprof;

static perfprof *prof = NULL;

object *perfprofcreate(object **args, int argc, void *env) {
    perfprof *p = malloc(sizeof(perfprof));
    p->enabled = 1;
    p->entries = NULL;
    p->count = 0;
    p->starttime = platformtime();
    p->currentname = NULL;
    p->currentstart = 0;
    prof = p;
    return makenative(p, NULL);
}

object *perfprofenable(object **args, int argc, void *env) {
    if (argc < 1 || !prof) throwexception("enable needs on");
    prof->enabled = args[0]->bval;
    return makenone();
}

object *perfprofsection(object **args, int argc, void *env) {
    if (!prof || !prof->enabled) return makenone();
    if (argc < 1) throwexception("section needs name");
    if (prof->currentname) {
        double elapsed = platformtime() - prof->currentstart;
        perfentry *e = prof->entries;
        while (e) {
            if (strcmp(e->name, prof->currentname) == 0) break;
            e = e->next;
        }
        if (!e) {
            e = malloc(sizeof(perfentry));
            e->name = strdup(prof->currentname);
            e->calls = 0;
            e->totaltime = 0;
            e->mintime = 0;
            e->maxtime = 0;
            e->next = prof->entries;
            prof->entries = e;
            prof->count++;
        }
        e->calls++;
        e->totaltime += elapsed;
        if (e->mintime == 0 || elapsed < e->mintime) e->mintime = elapsed;
        if (elapsed > e->maxtime) e->maxtime = elapsed;
    }
    prof->currentname = strdup(args[0]->sval);
    prof->currentstart = platformtime();
    return makenone();
}

object *perfprofend(object **args, int argc, void *env) {
    if (!prof || !prof->enabled) return makenone();
    if (prof->currentname) {
        double elapsed = platformtime() - prof->currentstart;
        perfentry *e = prof->entries;
        while (e) {
            if (strcmp(e->name, prof->currentname) == 0) break;
            e = e->next;
        }
        if (e) {
            e->calls++;
            e->totaltime += elapsed;
            if (e->mintime == 0 || elapsed < e->mintime) e->mintime = elapsed;
            if (elapsed > e->maxtime) e->maxtime = elapsed;
        }
        free(prof->currentname);
        prof->currentname = NULL;
    }
    return makenone();
}

object *perfprofreport(object **args, int argc, void *env) {
    if (!prof) return makenone();
    platformlog("perf report\n");
    platformlog("name                 calls    total    avg     min     max\n");
    platformlog("--------------------------------------------------------\n");
    perfentry *e = prof->entries;
    while (e) {
        char buf[256];
        double avg = e->calls > 0 ? e->totaltime / e->calls : 0;
        snprintf(buf, sizeof(buf), "%-20s %8ld %8.3f %8.3f %8.3f %8.3f\n",
                 e->name, e->calls, e->totaltime, avg, e->mintime, e->maxtime);
        platformlog(buf);
        e = e->next;
    }
    return makenone();
}

void registerperfproflib(environment *env) {
    object *mod = makemodule("perfprof", NULL);
    envset(mod->module.exports, "create", makebuiltin(perfprofcreate), 0);
    envset(mod->module.exports, "enable", makebuiltin(perfprofenable), 0);
    envset(mod->module.exports, "section", makebuiltin(perfprofsection), 0);
    envset(mod->module.exports, "end", makebuiltin(perfprofend), 0);
    envset(mod->module.exports, "report", makebuiltin(perfprofreport), 0);
    envset(env, "perfprof", mod, 0);
}
