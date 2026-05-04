#include "../core/axton.h"
#include <math.h>

static double meanlist(object *lst) {
    double sum = 0.0;
    for(int i=0;i<lst->list.count;i++){
        double v = lst->list.items[i]->type? lst->list.items[i]->fval : lst->list.items[i]->ival;
        sum += v;
    }
    return sum / lst->list.count;
}
object *statisticsmean(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=5) throwexception("mean needs list");
    return makefloat(meanlist(a[0]));
}
object *statisticsmedian(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=5) throwexception("median needs list");
    int n = a[0]->list.count;
    if(n==0) throwexception("empty list");
    double *vals = malloc(n*sizeof(double));
    for(int i=0;i<n;i++) vals[i] = a[0]->list.items[i]->type? a[0]->list.items[i]->fval : a[0]->list.items[i]->ival;
    for(int i=0;i<n-1;i++) for(int j=i+1;j<n;j++) if(vals[i]>vals[j]){ double t=vals[i]; vals[i]=vals[j]; vals[j]=t; }
    double med;
    if(n%2) med = vals[n/2];
    else med = (vals[n/2-1] + vals[n/2])/2.0;
    free(vals);
    return makefloat(med);
}
object *statisticsstdev(object **a, int c, void *e) {
    if(c<1 || a[0]->type!=5) throwexception("stdev needs list");
    double m = meanlist(a[0]);
    double sum = 0.0;
    for(int i=0;i<a[0]->list.count;i++){
        double v = a[0]->list.items[i]->type? a[0]->list.items[i]->fval : a[0]->list.items[i]->ival;
        sum += (v - m)*(v - m);
    }
    return makefloat(sqrt(sum / (a[0]->list.count - 1)));
}
void registerstatisticslib(environment *env) {
    object *mod = makemodule("statistics", NULL);
    envset(mod->module.exports, "mean", makebuiltin(statisticsmean), 0);
    envset(mod->module.exports, "median", makebuiltin(statisticsmedian), 0);
    envset(mod->module.exports, "stdev", makebuiltin(statisticsstdev), 0);
    envset(env, "statistics", mod, 0);
}
