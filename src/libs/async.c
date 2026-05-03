#include "../core/axton.h"

#ifdef __linux__
#include <sys/epoll.h>
#include <sys/eventfd.h>

static int epollfd = -1;
static object *taskqueue = NULL;

void asyncloopadd(object *task) {
    if (!taskqueue) {
        taskqueue = makelist();
    }
    listappend(taskqueue, task);
}

void asynclooprun(void) {
    if (epollfd < 0) {
        epollfd = epoll_create1(0);
    }
    struct epoll_event events[64];
    while (taskqueue && taskqueue->list.count > 0) {
        int nfds = epoll_wait(epollfd, events, 64, 100);
        for (int i = 0; i < taskqueue->list.count; i++) {
            object *task = taskqueue->list.items[i];
            if (task->type == 21 && task->coroutine.state == 0) {
                callfunc(task->coroutine.func, NULL, 0, globalenv);
                task->coroutine.state = 1;
            }
        }
        taskqueue->list.count = 0;
    }
}
#else
void asyncloopadd(object *task) {}
void asynclooprun(void) {}
#endif

object *asynccreate(object *func) {
    return makecoroutine(func);
}

object *asyncawait(object *coro) {
    if (coro->type != 21) throwexception("not a coroutine");
    asyncloopadd(coro);
    return makenone();
}

object *builtinasync(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("async needs function");
    return asynccreate(args[0]);
}

object *builtinawait(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("await needs coroutine");
    return asyncawait(args[0]);
}

object *builtinrunloop(object **args, int argc, environment *env) {
    asynclooprun();
    return makenone();
}

void registerasynclib(environment *env) {
    object *asyncmod = makemodule("async", NULL);
    envset(asyncmod->module.exports, "create", makebuiltin(builtinasync), 0);
    envset(asyncmod->module.exports, "await", makebuiltin(builtinawait), 0);
    envset(asyncmod->module.exports, "run", makebuiltin(builtinrunloop), 0);
    envset(env, "async", asyncmod, 0);
}
