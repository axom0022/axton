#include "../core/axton.h"

object *structpack(object **a, int c, void *e) {
    if(c<1) throwexception("pack needs format");
    return makestring("");
}
object *structunpack(object **a, int c, void *e) {
    if(c<2) throwexception("unpack needs format and data");
    return makelist();
}
void registerstructlib(environment *env) {
    object *mod = makemodule("struct", NULL);
    envset(mod->module.exports, "pack", makebuiltin(structpack), 0);
    envset(mod->module.exports, "unpack", makebuiltin(structunpack), 0);
    envset(env, "struct", mod, 0);
}
