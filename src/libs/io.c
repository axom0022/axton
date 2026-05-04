#include "../core/axton.h"

object *iobytesio(object **a, int c, void *e) {
    object *obj = makemodule("BytesIO", NULL);
    envset(obj->module.exports, "data", (c>0 && a[0]->type==2)?a[0]:makestring(""), 0);
    return obj;
}
void registeriolib(environment *env) {
    object *mod = makemodule("io", NULL);
    envset(mod->module.exports, "BytesIO", makebuiltin(iobytesio), 0);
    envset(env, "io", mod, 0);
}
