#include "../core/axton.h"

object *webviewcreate(object **a, int c, void *e) {
    if (c < 2) throwexception("webview needs url and parent");
    return makenone();
}

object *webviewload(object **a, int c, void *e) {
    if (c < 2) throwexception("load needs webview and url");
    return makenone();
}

void registerwebviewlib(environment *env) {
    object *mod = makemodule("webview", NULL);
    envset(mod->module.exports, "create", makebuiltin(webviewcreate), 0);
    envset(mod->module.exports, "load", makebuiltin(webviewload), 0);
    envset(env, "webview", mod, 0);
}
