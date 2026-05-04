#include "axton.h"

static jmp_buf exceptionjump;
static object *exvalue = NULL;

void throwexception(char *msg) {
    exvalue = makestring(msg);
    longjmp(exceptionjump, 1);
}

object *catchexception(void) {
    object *val = exvalue;
    exvalue = NULL;
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
