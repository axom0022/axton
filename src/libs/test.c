#include "../core/axton.h"

static int testcount = 0;
static int passcount = 0;
static int failcount = 0;
static char failures[100][256];

void testassert(int cond, char *msg) {
    testcount++;
    if (cond) {
        passcount++;
    } else {
        failcount++;
        snprintf(failures[failcount-1], sizeof(failures[0]), "FAIL: %s", msg);
    }
}

void testassertequal(object *a, object *b, char *msg) {
    testassert(valuesequal(a, b), msg);
}

void testassertnotequal(object *a, object *b, char *msg) {
    testassert(!valuesequal(a, b), msg);
}

void testasserttrue(object *val, char *msg) {
    testassert(istruthy(val), msg);
}

void testassertfalse(object *val, char *msg) {
    testassert(!istruthy(val), msg);
}

object *testrun(void) {
    printf("\n=== TEST RESULTS ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", testcount, passcount, failcount);
    for (int i = 0; i < failcount; i++) {
        printf("%s\n", failures[i]);
    }
    printf("===================\n");
    testcount = 0;
    passcount = 0;
    failcount = 0;
    return makebool(failcount == 0);
}

object *builtintestassert(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("assert needs condition");
    testassert(istruthy(args[0]), argc > 1 ? args[1]->sval : "assertion failed");
    return makenone();
}

object *builtintestassertequal(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("assertequal needs two values");
    char *msg = argc > 2 ? args[2]->sval : "values not equal";
    testassertequal(args[0], args[1], msg);
    return makenone();
}

object *builtintestrun(object **args, int argc, environment *env) {
    return testrun();
}

void registertestlib(environment *env) {
    object *testmod = makemodule("test", NULL);
    envset(testmod->module.exports, "assert", makebuiltin(builtintestassert), 0);
    envset(testmod->module.exports, "assertequal", makebuiltin(builtintestassertequal), 0);
    envset(testmod->module.exports, "run", makebuiltin(builtintestrun), 0);
    envset(env, "test", testmod, 0);
}
