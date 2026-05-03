#include "../core/axton.h"
#include <pthread.h>

static void *threadfunc(void *arg) {
    object *thread = (object*)arg;
    object *argslist[1] = {thread->threadobj.args};
    callfunc(thread->threadobj.func, argslist, 1, globalenv);
    thread->threadobj.running = 0;
    return NULL;
}

object *builtinthreadstart(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("thread.start needs function");
    object *thread = makethread(args[0], argc > 1 ? args[1] : makenone());
    pthread_create(&thread->threadobj.thread, NULL, threadfunc, thread);
    return thread;
}

object *builtinthreadjoin(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("thread.join needs thread");
    if (args[0]->type != 25) throwexception("not a thread");
    pthread_join(args[0]->threadobj.thread, NULL);
    return makenone();
}

void registerthreadlib(environment *env) {
    object *threadmod = makemodule("threading", NULL);
    envset(threadmod->module.exports, "start", makebuiltin(builtinthreadstart), 0);
    envset(threadmod->module.exports, "join", makebuiltin(builtinthreadjoin), 0);
    envset(env, "threading", threadmod, 0);
}
