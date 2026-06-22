#include "../core/axton.h"
#include <wchar.h>
#include <locale.h>

object *unicodewidth(object **a, int c, void *e) {
    if (c < 1) throwexception("width needs char");
    wchar_t wc;
    mbtowc(&wc, a[0]->sval, strlen(a[0]->sval));
    return makeint(wcwidth(wc));
}

object *unicodenormalize(object **a, int c, void *e) {
    if (c < 2) throwexception("normalize needs form and string");
    return makestring(a[1]->sval);
}

void registerunicodelib(environment *env) {
    object *mod = makemodule("unicode", NULL);
    envset(mod->module.exports, "width", makebuiltin(unicodewidth), 0);
    envset(mod->module.exports, "normalize", makebuiltin(unicodenormalize), 0);
    envset(env, "unicode", mod, 0);
}
