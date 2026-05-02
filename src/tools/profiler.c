#include "../core/axton.h"
#include "../platform/platform.h"

static int profactive = 0;
static double profstart = 0;
static struct {
    char *name;
    double total;
    int calls;
} profdata[1000];
static int profcount = 0;

static int findprof(const char *name) {
    for (int i = 0; i < profcount; i++) {
        if (strcmp(profdata[i].name, name) == 0) return i;
    }
    return -1;
}

void profilestart(void) {
    profactive = 1;
    profstart = platformtime();
}

void profilestop(void) {
    profactive = 0;
}

void profilerecord(const char *name, double elapsed) {
    if (!profactive) return;
    int idx = findprof(name);
    if (idx < 0) {
        idx = profcount++;
        profdata[idx].name = strdup(name);
        profdata[idx].total = 0;
        profdata[idx].calls = 0;
    }
    profdata[idx].total += elapsed;
    profdata[idx].calls++;
}

void profilereport(void) {
    platformlog("\nprofiling report\n");
    platformlog("name                 calls    total ms\n");
    platformlog("-------------------------------------\n");
    for (int i = 0; i < profcount; i++) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%-20s %8d %10.3f\n",
                 profdata[i].name, profdata[i].calls, profdata[i].total * 1000);
        platformlog(buf);
    }
}

object *builtinprofilestart(object **args, int argc, environment *env) {
    profilestart();
    return makenone();
}

object *builtinprofilestop(object **args, int argc, environment *env) {
    profilestop();
    return makenone();
}

object *builtinprofilereport(object **args, int argc, environment *env) {
    profilereport();
    return makenone();
}
