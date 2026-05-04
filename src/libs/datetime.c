#include "../core/axton.h"
#include <time.h>

object *datetimenow(object **a, int c, void *e) {
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

object *datetimetimestamp(object **a, int c, void *e) {
    return makefloat(time(NULL));
}

object *datetimesleep(object **a, int c, void *e) {
    double sec = 1.0;
    if (c > 0 && a[0]->type == 0) sec = a[0]->ival;
    else if (c > 0 && a[0]->type == 1) sec = a[0]->fval;
    platformsleep(sec);
    return makenone();
}

object *datetimestrftime(object **a, int c, void *e) {
    if (c < 1 || a[0]->type != 2) throwexception("strftime needs format");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[256];
    strftime(buf, sizeof(buf), a[0]->sval, tm);
    return makestring(buf);
}

void registerdatetimelib(environment *env) {
    object *mod = makemodule("datetime", NULL);
    envset(mod->module.exports, "now", makebuiltin(datetimenow), 0);
    envset(mod->module.exports, "timestamp", makebuiltin(datetimetimestamp), 0);
    envset(mod->module.exports, "sleep", makebuiltin(datetimesleep), 0);
    envset(mod->module.exports, "strftime", makebuiltin(datetimestrftime), 0);
    envset(env, "datetime", mod, 0);
}
