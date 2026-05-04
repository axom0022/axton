#include "../core/axton.h"
#include <time.h>

static long epochsec(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

object *datetime_now(object **args, int argc, void *env) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    object *d = makedict();
    dictset(d, makestring("year"), makeint(tm->tm_year + 1900));
    dictset(d, makestring("month"), makeint(tm->tm_mon + 1));
    dictset(d, makestring("day"), makeint(tm->tm_mday));
    dictset(d, makestring("hour"), makeint(tm->tm_hour));
    dictset(d, makestring("minute"), makeint(tm->tm_min));
    dictset(d, makestring("second"), makeint(tm->tm_sec));
    dictset(d, makestring("weekday"), makeint(tm->tm_wday));
    dictset(d, makestring("yearday"), makeint(tm->tm_yday));
    return d;
}

object *datetime_timestamp(object **args, int argc, void *env) {
    return makefloat(epochsec());
}

object *datetime_sleep(object **args, int argc, void *env) {
    double sec = 1.0;
    if (argc > 0 && args[0]->type == 0) sec = args[0]->ival;
    else if (argc > 0 && args[0]->type == 1) sec = args[0]->fval;
    platformsleep(sec);
    return makenone();
}

object *datetime_strftime(object **args, int argc, void *env) {
    if (argc < 1) throwexception("strftime needs format");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[256];
    strftime(buf, sizeof(buf), args[0]->sval, tm);
    return makestring(buf);
}

void registerdatetimelib(environment *env) {
    object *mod = makemodule("datetime", NULL);
    envset(mod->module.exports, "now", makebuiltin(datetime_now), 0);
    envset(mod->module.exports, "timestamp", makebuiltin(datetime_timestamp), 0);
    envset(mod->module.exports, "sleep", makebuiltin(datetime_sleep), 0);
    envset(mod->module.exports, "strftime", makebuiltin(datetime_strftime), 0);
    envset(env, "datetime", mod, 0);
}
