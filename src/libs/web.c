#include "../core/axton.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

object *webinit(object **args, int argc, void *env) {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(platform.mainloop, 0, 1);
#endif
    return makenone();
}

object *webcanvas(object **args, int argc, void *env) {
    if (argc < 1) throwexception("canvas needs id");
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_RESULT r = emscripten_set_canvas_size(800, 600);
    if (r != EMSCRIPTEN_RESULT_SUCCESS) throwexception("canvas setup failed");
#endif
    return makenone();
}

object *webdom(object **args, int argc, void *env) {
    if (argc < 2) throwexception("dom needs element and html");
#ifdef __EMSCRIPTEN__
    char code[1024];
    snprintf(code, sizeof(code), "document.getElementById('%s').innerHTML = '%s';", args[0]->sval, args[1]->sval);
    emscripten_run_script(code);
#endif
    return makenone();
}

object *webjs(object **args, int argc, void *env) {
    if (argc < 1) throwexception("js needs code");
#ifdef __EMSCRIPTEN__
    emscripten_run_script(args[0]->sval);
#endif
    return makenone();
}

void registerweblib(environment *env) {
    object *mod = makemodule("web", NULL);
    envset(mod->module.exports, "init", makebuiltin(webinit), 0);
    envset(mod->module.exports, "canvas", makebuiltin(webcanvas), 0);
    envset(mod->module.exports, "dom", makebuiltin(webdom), 0);
    envset(mod->module.exports, "js", makebuiltin(webjs), 0);
    envset(env, "web", mod, 0);
}
