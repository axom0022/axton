#include "../core/axton.h"
#include <time.h>

static int loglevel = 20;
static const char *lvlname[] = {"DEBUG","INFO","WARNING","ERROR","CRITICAL"};

object *loggingbasicconfig(object **args, int argc, void *env) {
    if(argc>0 && args[0]->type==0) loglevel = args[0]->ival;
    return makenone();
}

static void logmsg(int level, char *msg) {
    if(level<loglevel) return;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    fprintf(stderr, "%s - %s - %s\n", buf, lvlname[level/10-1], msg);
}

object *loggingdebug(object **args, int argc, void *env) {
    if(argc<1) return makenone();
    logmsg(10, args[0]->sval);
    return makenone();
}

object *logginginfo(object **args, int argc, void *env) {
    if(argc<1) return makenone();
    logmsg(20, args[0]->sval);
    return makenone();
}

object *loggingwarning(object **args, int argc, void *env) {
    if(argc<1) return makenone();
    logmsg(30, args[0]->sval);
    return makenone();
}

object *loggingerror(object **args, int argc, void *env) {
    if(argc<1) return makenone();
    logmsg(40, args[0]->sval);
    return makenone();
}

object *loggingcritical(object **args, int argc, void *env) {
    if(argc<1) return makenone();
    logmsg(50, args[0]->sval);
    return makenone();
}

void registerlogginglib(environment *env) {
    object *mod = makemodule("logging", NULL);
    envset(mod->module.exports, "basicConfig", makebuiltin(loggingbasicconfig), 0);
    envset(mod->module.exports, "debug", makebuiltin(loggingdebug), 0);
    envset(mod->module.exports, "info", makebuiltin(logginginfo), 0);
    envset(mod->module.exports, "warning", makebuiltin(loggingwarning), 0);
    envset(mod->module.exports, "error", makebuiltin(loggingerror), 0);
    envset(mod->module.exports, "critical", makebuiltin(loggingcritical), 0);
    envset(env, "logging", mod, 0);
}
