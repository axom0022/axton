#include "axton.h"

static jmp_buf exception_jump;
static object *exception_value = NULL;

void throw_exception(char *msg) {
    exception_value = make_string(msg);
    longjmp(exception_jump, 1);
}

object *catch_exception(void) {
    object *val = exception_value;
    exception_value = NULL;
    return val;
}

void init_exceptions(environment *env) {
    env_set(env, "exception", make_class("exception", env_new(NULL), NULL), 0);
}
