#include "../core/axton.h"
#include <complex.h>

object *cmathphase(object **a, int c, void *e) {
    double real = a[0]->type? a[0]->fval : a[0]->ival;
    double imag = (c>1 && a[1]->type==1) ? a[1]->fval : 0.0;
    return makefloat(carg(real + imag*I));
}
object *cmathpolar(object **a, int c, void *e) {
    double real = a[0]->type? a[0]->fval : a[0]->ival;
    double imag = (c>1 && a[1]->type==1) ? a[1]->fval : 0.0;
    double r = cabs(real + imag*I);
    double phi = carg(real + imag*I);
    object *res = makelist();
    listappend(res, makefloat(r));
    listappend(res, makefloat(phi));
    return res;
}
object *cmathrect(object **a, int c, void *e) {
    double r = a[0]->type? a[0]->fval : a[0]->ival;
    double phi = a[1]->type? a[1]->fval : a[1]->ival;
    object *res = makelist();
    listappend(res, makefloat(r * cos(phi)));
    listappend(res, makefloat(r * sin(phi)));
    return res;
}
void registercmathlib(environment *env) {
    object *mod = makemodule("cmath", NULL);
    envset(mod->module.exports, "phase", makebuiltin(cmathphase), 0);
    envset(mod->module.exports, "polar", makebuiltin(cmathpolar), 0);
    envset(mod->module.exports, "rect", makebuiltin(cmathrect), 0);
    envset(env, "cmath", mod, 0);
}
