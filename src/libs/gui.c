#include "../core/axton.h"

object *guiwindow(object **a, int c, void *e) {
    if (c < 3) throwexception("window needs title w h");
    return makeguiwin(NULL, NULL, a[1]->ival, a[2]->ival);
}

object *guibutton(object **a, int c, void *e) {
    if (c < 6) throwexception("button needs parent text x y w h onclick");
    return makenone();
}

object *guilabel(object **a, int c, void *e) {
    if (c < 4) throwexception("label needs parent text x y");
    return makenone();
}

object *guirun(object **a, int c, void *e) {
    platform.mainloop();
    return makenone();
}

void registerguilib(environment *env) {
    object *mod = makemodule("gui", NULL);
    envset(mod->module.exports, "window", makebuiltin(guiwindow), 0);
    envset(mod->module.exports, "button", makebuiltin(guibutton), 0);
    envset(mod->module.exports, "label", makebuiltin(guilabel), 0);
    envset(mod->module.exports, "run", makebuiltin(guirun), 0);
    envset(env, "gui", mod, 0);
}
