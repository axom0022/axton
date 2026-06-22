#include "../core/axton.h"
#include <sys/epoll.h>
#include <unistd.h>

static int epollfd = -1;

object *asyncloop(object **a, int c, void *e) {
    if (epollfd == -1) epollfd = epoll_create1(0);
    return makeint(epollfd);
}

object *asyncsleep(object **a, int c, void *e) {
    if (c < 1) throwexception("sleep needs seconds");
    double sec = a[0]->fval;
    platformsleep(sec);
    return makenone();
}

object *asyncaddtask(object **a, int c, void *e) {
    if (c < 1) throwexception("addtask needs coroutine");
    return makenone();
}

object *asyncawait(object **a, int c, void *e) {
    if (c < 1) throwexception("await needs task");
    return makenone();
}

void registerasynclib(environment *env) {
    object *mod = makemodule("async", NULL);
    envset(mod->module.exports, "loop", makebuiltin(asyncloop), 0);
    envset(mod->module.exports, "sleep", makebuiltin(asyncsleep), 0);
    envset(mod->module.exports, "addtask", makebuiltin(asyncaddtask), 0);
    envset(mod->module.exports, "await", makebuiltin(asyncawait), 0);
    envset(env, "async", mod, 0);
}
