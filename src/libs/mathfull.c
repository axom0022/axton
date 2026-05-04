#include "../core/axton.h"
#include <math.h>

object *mathpi(object **a, int c, void *e) { return makefloat(3.141592653589793); }
object *mathe(object **a, int c, void *e) { return makefloat(2.718281828459045); }
object *mathtau(object **a, int c, void *e) { return makefloat(6.283185307179586); }
object *mathinf(object **a, int c, void *e) { return makefloat(INFINITY); }
object *mathnan(object **a, int c, void *e) { return makefloat(NAN); }

object *mathsqrt(object **a, int c, void *e) {
    if(c<1) throwexception("sqrt needs arg");
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(sqrt(x));
}
object *mathexp(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(exp(x));
}
object *mathlog(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(log(x));
}
object *mathlog10(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(log10(x));
}
object *mathlog2(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(log2(x));
}
object *mathsin(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(sin(x));
}
object *mathcos(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(cos(x));
}
object *mathtan(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(tan(x));
}
object *mathasin(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(asin(x));
}
object *mathacos(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(acos(x));
}
object *mathatan(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(atan(x));
}
object *mathatan2(object **a, int c, void *e) {
    double y = a[0]->type? a[0]->fval : a[0]->ival;
    double x = a[1]->type? a[1]->fval : a[1]->ival;
    return makefloat(atan2(y, x));
}
object *mathsinh(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(sinh(x));
}
object *mathcosh(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(cosh(x));
}
object *mathtanh(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(tanh(x));
}
object *mathceil(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(ceil(x));
}
object *mathfloor(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(floor(x));
}
object *mathtrunc(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(trunc(x));
}
object *mathfabs(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(fabs(x));
}
object *mathgamma(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(tgamma(x));
}
object *mathlgamma(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(lgamma(x));
}
object *matherf(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(erf(x));
}
object *matherefc(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(erfc(x));
}
object *mathhypot(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    double y = a[1]->type? a[1]->fval : a[1]->ival;
    return makefloat(hypot(x, y));
}
object *mathpow(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    double y = a[1]->type? a[1]->fval : a[1]->ival;
    return makefloat(pow(x, y));
}
object *mathremainder(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    double y = a[1]->type? a[1]->fval : a[1]->ival;
    return makefloat(remainder(x, y));
}
object *mathmodf(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    double intpart;
    double frac = modf(x, &intpart);
    object *res = makelist();
    listappend(res, makefloat(frac));
    listappend(res, makefloat(intpart));
    return res;
}
object *mathfrexp(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    int exp;
    double mant = frexp(x, &exp);
    object *res = makelist();
    listappend(res, makefloat(mant));
    listappend(res, makeint(exp));
    return res;
}
object *mathldexp(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    int exp = a[1]->ival;
    return makefloat(ldexp(x, exp));
}
object *mathcopysign(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    double y = a[1]->type? a[1]->fval : a[1]->ival;
    return makefloat(copysign(x, y));
}
object *mathnextafter(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    double y = a[1]->type? a[1]->fval : a[1]->ival;
    return makefloat(nextafter(x, y));
}
object *mathulp(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    return makefloat(nextafter(x, INFINITY) - x);
}
object *mathisclose(object **a, int c, void *e) {
    double x = a[0]->type? a[0]->fval : a[0]->ival;
    double y = a[1]->type? a[1]->fval : a[1]->ival;
    double rel = 1e-9;
    double abs = 0.0;
    if(c>2 && a[2]->type==1) rel = a[2]->fval;
    if(c>3 && a[3]->type==1) abs = a[3]->fval;
    double diff = fabs(x - y);
    if(diff <= abs) return makebool(1);
    if(diff <= rel * fmax(fabs(x), fabs(y))) return makebool(1);
    return makebool(0);
}

void registermathfulllib(environment *env) {
    object *mod = makemodule("math", NULL);
    envset(mod->module.exports, "pi", makebuiltin(mathpi), 0);
    envset(mod->module.exports, "e", makebuiltin(mathe), 0);
    envset(mod->module.exports, "tau", makebuiltin(mathtau), 0);
    envset(mod->module.exports, "inf", makebuiltin(mathinf), 0);
    envset(mod->module.exports, "nan", makebuiltin(mathnan), 0);
    envset(mod->module.exports, "sqrt", makebuiltin(mathsqrt), 0);
    envset(mod->module.exports, "exp", makebuiltin(mathexp), 0);
    envset(mod->module.exports, "log", makebuiltin(mathlog), 0);
    envset(mod->module.exports, "log10", makebuiltin(mathlog10), 0);
    envset(mod->module.exports, "log2", makebuiltin(mathlog2), 0);
    envset(mod->module.exports, "sin", makebuiltin(mathsin), 0);
    envset(mod->module.exports, "cos", makebuiltin(mathcos), 0);
    envset(mod->module.exports, "tan", makebuiltin(mathtan), 0);
    envset(mod->module.exports, "asin", makebuiltin(mathasin), 0);
    envset(mod->module.exports, "acos", makebuiltin(mathacos), 0);
    envset(mod->module.exports, "atan", makebuiltin(mathatan), 0);
    envset(mod->module.exports, "atan2", makebuiltin(mathatan2), 0);
    envset(mod->module.exports, "sinh", makebuiltin(mathsinh), 0);
    envset(mod->module.exports, "cosh", makebuiltin(mathcosh), 0);
    envset(mod->module.exports, "tanh", makebuiltin(mathtanh), 0);
    envset(mod->module.exports, "ceil", makebuiltin(mathceil), 0);
    envset(mod->module.exports, "floor", makebuiltin(mathfloor), 0);
    envset(mod->module.exports, "trunc", makebuiltin(mathtrunc), 0);
    envset(mod->module.exports, "fabs", makebuiltin(mathfabs), 0);
    envset(mod->module.exports, "gamma", makebuiltin(mathgamma), 0);
    envset(mod->module.exports, "lgamma", makebuiltin(mathlgamma), 0);
    envset(mod->module.exports, "erf", makebuiltin(matherf), 0);
    envset(mod->module.exports, "erfc", makebuiltin(matherefc), 0);
    envset(mod->module.exports, "hypot", makebuiltin(mathhypot), 0);
    envset(mod->module.exports, "pow", makebuiltin(mathpow), 0);
    envset(mod->module.exports, "remainder", makebuiltin(mathremainder), 0);
    envset(mod->module.exports, "modf", makebuiltin(mathmodf), 0);
    envset(mod->module.exports, "frexp", makebuiltin(mathfrexp), 0);
    envset(mod->module.exports, "ldexp", makebuiltin(mathldexp), 0);
    envset(mod->module.exports, "copysign", makebuiltin(mathcopysign), 0);
    envset(mod->module.exports, "nextafter", makebuiltin(mathnextafter), 0);
    envset(mod->module.exports, "ulp", makebuiltin(mathulp), 0);
    envset(mod->module.exports, "isclose", makebuiltin(mathisclose), 0);
    envset(env, "math", mod, 0);
}
