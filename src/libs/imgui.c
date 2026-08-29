#include "../core/axton.h"
#include <imgui.h>

object *imguiinit(object **args, int argc, void *env) {
    if (argc < 1) throwexception("init needs window");
    void *win = args[0]->guiwin.window;
    platformimguiinit(win);
    object *imgui = makeimgui(NULL, win);
    imgui->imgui.active = 1;
    return imgui;
}

object *imguiupdate(object **args, int argc, void *env) {
    if (argc < 1) throwexception("update needs imgui");
    object *imgui = args[0];
    if (!imgui->imgui.active) throwexception("imgui not active");
    platformimguiupdate(imgui->imgui.window);
    return makenone();
}

object *imguiend(object **args, int argc, void *env) {
    if (argc < 1) throwexception("end needs imgui");
    object *imgui = args[0];
    platformimguirender(imgui->imgui.window);
    return makenone();
}

void registerimguilib(environment *env) {
    object *mod = makemodule("imgui", NULL);
    envset(mod->module.exports, "init", makebuiltin(imguiinit), 0);
    envset(mod->module.exports, "update", makebuiltin(imguiupdate), 0);
    envset(mod->module.exports, "end", makebuiltin(imguiend), 0);
    envset(env, "imgui", mod, 0);
}
