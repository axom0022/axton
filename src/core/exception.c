#include "axton.h"

static jmp_buf exceptionjump;
static object *exceptionvalue = NULL;

void throwexception(char *msg) {
    exceptionvalue = makestring(msg);
    longjmp(exceptionjump, 1);
}

object *catchexception(void) {
    object *val = exceptionvalue;
    exceptionvalue = NULL;
    return val;
}

void initexceptions(environment *env) {
    envset(env, "exception", makeclass("exception", envnew(NULL), NULL), 0);
}
