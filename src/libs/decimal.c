#include "../core/axton.h"

object *decimaldecimal(object **a, int c, void *e) {
    if(c<1) throwexception("Decimal needs value");
    char buf[256];
    if(a[0]->type==0) snprintf(buf,256,"%ld",a[0]->ival);
    else if(a[0]->type==1) snprintf(buf,256,"%g",a[0]->fval);
    else return makestring(a[0]->sval);
    return makestring(buf);
}
object *decimalgetcontext(object **a, int c, void *e) { return makedict(); }
void registerdecimallib(environment *env) {
    object *mod = makemodule("decimal", NULL);
    envset(mod->module.exports, "Decimal", makebuiltin(decimaldecimal), 0);
    envset(mod->module.exports, "getcontext", makebuiltin(decimalgetcontext), 0);
    envset(env, "decimal", mod, 0);
}
