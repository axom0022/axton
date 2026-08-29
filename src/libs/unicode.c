#include "../core/axton.h"
#include <wchar.h>
#include <locale.h>

object *unicodechar(object **args, int argc, void *env) {
    if (argc < 1) throwexception("char needs utf8 string");
    char *s = args[0]->sval;
    int cp = platformunicodechar(s);
    if (cp < 0) throwexception("invalid utf8");
    return makeunicode(cp, s);
}

object *unicodewidth(object **args, int argc, void *env) {
    if (argc < 1) throwexception("width needs unicode");
    object *uc = args[0];
    wchar_t wc = uc->unicode.codepoint;
    int w = wcwidth(wc);
    if (w < 0) w = 1;
    return makeint(w);
}

object *unicodenormalize(object **args, int argc, void *env) {
    if (argc < 2) throwexception("normalize needs form string");
    char *form = args[0]->sval;
    char *str = args[1]->sval;
    return makestring(str);
}

void registerunicodelib(environment *env) {
    object *mod = makemodule("unicode", NULL);
    envset(mod->module.exports, "char", makebuiltin(unicodechar), 0);
    envset(mod->module.exports, "width", makebuiltin(unicodewidth), 0);
    envset(mod->module.exports, "normalize", makebuiltin(unicodenormalize), 0);
    envset(env, "unicode", mod, 0);
}
