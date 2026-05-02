#include "../core/axton.h"
#include <math.h>

object *builtinmathpi(object **args, int argc, environment *env) {
    return makefloat(3.141592653589793);
}

object *builtinmathe(object **args, int argc, environment *env) {
    return makefloat(2.718281828459045);
}

object *builtinmathsqrt(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("sqrt needs number");
    double val = args[0]->type == 0 ? args[0]->ival : args[0]->fval;
    return makefloat(sqrt(val));
}

object *builtinmathsin(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("sin needs number");
    double val = args[0]->type == 0 ? args[0]->ival : args[0]->fval;
    return makefloat(sin(val));
}

object *builtinmathcos(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("cos needs number");
    double val = args[0]->type == 0 ? args[0]->ival : args[0]->fval;
    return makefloat(cos(val));
}

object *builtinmathtan(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("tan needs number");
    double val = args[0]->type == 0 ? args[0]->ival : args[0]->fval;
    return makefloat(tan(val));
}

object *builtinmathpow(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("pow needs base and exponent");
    double base = args[0]->type == 0 ? args[0]->ival : args[0]->fval;
    double exp = args[1]->type == 0 ? args[1]->ival : args[1]->fval;
    return makefloat(pow(base, exp));
}

void registermathlib(environment *env) {
    object *mathmod = makemodule("math", NULL);
    envset(mathmod->module.exports, "pi", makebuiltin(builtinmathpi), 0);
    envset(mathmod->module.exports, "e", makebuiltin(builtinmathe), 0);
    envset(mathmod->module.exports, "sqrt", makebuiltin(builtinmathsqrt), 0);
    envset(mathmod->module.exports, "sin", makebuiltin(builtinmathsin), 0);
    envset(mathmod->module.exports, "cos", makebuiltin(builtinmathcos), 0);
    envset(mathmod->module.exports, "tan", makebuiltin(builtinmathtan), 0);
    envset(mathmod->module.exports, "pow", makebuiltin(builtinmathpow), 0);
    envset(env, "math", mathmod, 0);
}
