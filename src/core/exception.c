#include "axton.h"
#include <stdarg.h>

static object *exvalue = NULL;
static char exfile[256];
static int exline = 0;

void throwexception(char *msg) {
    char fullmsg[512];
    if (currentframe && currentframe->file) {
        snprintf(fullmsg, sizeof(fullmsg), "%s at %s:%d", msg, currentframe->file, currentframe->line);
    } else {
        snprintf(fullmsg, sizeof(fullmsg), "%s", msg);
    }
    exvalue = makestring(fullmsg);
    gcaddroot(exvalue);
    if (currentframe) longjmp(currentframe->jump, 1);
    fprintf(stderr, "uncaught exception: %s\n", fullmsg);
    exit(1);
}

void throwexceptiontype(char *type, char *msg) {
    char full[512];
    if (currentframe && currentframe->file) {
        snprintf(full, sizeof(full), "%s: %s at %s:%d", type, msg, currentframe->file, currentframe->line);
    } else {
        snprintf(full, sizeof(full), "%s: %s", type, msg);
    }
    exvalue = makestring(full);
    gcaddroot(exvalue);
    if (currentframe) longjmp(currentframe->jump, 1);
    fprintf(stderr, "uncaught exception: %s\n", full);
    exit(1);
}

object *catchexception(void) {
    object *val = exvalue;
    exvalue = NULL;
    if (val) gcremoveroot(val);
    return val;
}

void initexceptions(environment *env) {
    envset(env, "exception", makeclass("exception", envnew(NULL), NULL), 0);
    envset(env, "typeerror", makeclass("typeerror", envnew(NULL), envget(env, "exception")), 0);
    envset(env, "valueerror", makeclass("valueerror", envnew(NULL), envget(env, "exception")), 0);
    envset(env, "keyerror", makeclass("keyerror", envnew(NULL), envget(env, "exception")), 0);
    envset(env, "indexerror", makeclass("indexerror", envnew(NULL), envget(env, "exception")), 0);
    envset(env, "attributeerror", makeclass("attributeerror", envnew(NULL), envget(env, "exception")), 0);
}
