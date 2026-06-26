#include "../core/axton.h"
#ifdef __linux__
#include <GLFW/glfw3.h>
#endif
#ifdef _WIN32
#include <GLFW/glfw3.h>
#endif
#ifdef __APPLE__
#include <GLFW/glfw3.h>
#endif

static void *glfwwin = NULL;
static int keys[512];
static int mousebuttons[8];
static int mouseposx = 0, mouseposy = 0;
static int mousedx = 0, mousedy = 0;
static int mousescroll = 0;
static int gamepadconnected[4] = {0};
static float gamepadaxes[4][16];
static int gamepadbuttons[4][32];

static void keycallback(void *win, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;
    if (action == 1) keys[key] = 1;
    else if (action == 0) keys[key] = 0;
}

static void mousebuttoncallback(void *win, int button, int action, int mods) {
    (void)win; (void)mods;
    if (action == 1) mousebuttons[button] = 1;
    else if (action == 0) mousebuttons[button] = 0;
}

static void cursorposcallback(void *win, double x, double y) {
    (void)win;
    mousedx = x - mouseposx;
    mousedy = y - mouseposy;
    mouseposx = x;
    mouseposy = y;
}

static void scrollcallback(void *win, double x, double y) {
    (void)win; (void)x;
    mousescroll = (int)y;
}

object *inputinit(object **args, int argc, void *env) {
    glfwwin = platform.createwindow(0, 0, "input");
    if (!glfwwin) throwexception("glfw init failed");
    glfwSetKeyCallback(glfwwin, keycallback);
    glfwSetMouseButtonCallback(glfwwin, mousebuttoncallback);
    glfwSetCursorPosCallback(glfwwin, cursorposcallback);
    glfwSetScrollCallback(glfwwin, scrollcallback);
    for (int i = 0; i < 4; i++) {
        if (glfwJoystickPresent(i)) {
            gamepadconnected[i] = 1;
            int count = 0;
            const float *axes = glfwGetJoystickAxes(i, &count);
            if (axes) for (int j=0;j<count && j<16;j++) gamepadaxes[i][j]=axes[j];
            const unsigned char *buttons = glfwGetJoystickButtons(i, &count);
            if (buttons) for (int j=0;j<count && j<32;j++) gamepadbuttons[i][j]=buttons[j];
        }
    }
    return makenone();
}

object *inputupdate(object **args, int argc, void *env) {
    if (glfwwin) glfwPollEvents();
    for (int i=0;i<4;i++) {
        if (glfwJoystickPresent(i)) {
            gamepadconnected[i]=1;
            int count=0;
            const float *axes=glfwGetJoystickAxes(i,&count);
            if(axes) for(int j=0;j<count&&j<16;j++) gamepadaxes[i][j]=axes[j];
            const unsigned char *buttons=glfwGetJoystickButtons(i,&count);
            if(buttons) for(int j=0;j<count&&j<32;j++) gamepadbuttons[i][j]=buttons[j];
        }
    }
    return makenone();
}

object *inputkey(object **args, int argc, void *env) {
    if (argc<1 || args[0]->type!=0) throwexception("key needs int");
    int k = args[0]->ival;
    return makebool(keys[k]);
}

object *inputmouse(object **args, int argc, void *env) {
    if (argc<1 || args[0]->type!=0) throwexception("mouse needs button");
    int b = args[0]->ival;
    return makebool(mousebuttons[b]);
}

object *inputmpos(object **args, int argc, void *env) {
    object *r = makelist();
    listappend(r, makeint(mouseposx));
    listappend(r, makeint(mouseposy));
    return r;
}

object *inputmdelta(object **args, int argc, void *env) {
    object *r = makelist();
    listappend(r, makeint(mousedx));
    listappend(r, makeint(mousedy));
    mousedx=0; mousedy=0;
    return r;
}

object *inputscroll(object **args, int argc, void *env) {
    int s = mousescroll;
    mousescroll=0;
    return makeint(s);
}

object *inputgamepadconnected(object **args, int argc, void *env) {
    if (argc<1 || args[0]->type!=0) throwexception("gamepadconnected needs index");
    int idx = args[0]->ival;
    if (idx<0||idx>=4) return makebool(0);
    return makebool(gamepadconnected[idx]);
}

object *inputgamepadaxis(object **args, int argc, void *env) {
    if (argc<2 || args[0]->type!=0 || args[1]->type!=0) throwexception("gamepadaxis needs index and axis");
    int idx=args[0]->ival, ax=args[1]->ival;
    if (idx<0||idx>=4) return makefloat(0);
    if (ax<0||ax>=16) return makefloat(0);
    return makefloat(gamepadaxes[idx][ax]);
}

object *inputgamepadbutton(object **args, int argc, void *env) {
    if (argc<2 || args[0]->type!=0 || args[1]->type!=0) throwexception("gamepadbutton needs index and button");
    int idx=args[0]->ival, btn=args[1]->ival;
    if (idx<0||idx>=4) return makebool(0);
    if (btn<0||btn>=32) return makebool(0);
    return makebool(gamepadbuttons[idx][btn]);
}

void registerinputlib(environment *env) {
    object *mod = makemodule("input", NULL);
    envset(mod->module.exports, "init", makebuiltin(inputinit), 0);
    envset(mod->module.exports, "update", makebuiltin(inputupdate), 0);
    envset(mod->module.exports, "key", makebuiltin(inputkey), 0);
    envset(mod->module.exports, "mouse", makebuiltin(inputmouse), 0);
    envset(mod->module.exports, "mpos", makebuiltin(inputmpos), 0);
    envset(mod->module.exports, "mdelta", makebuiltin(inputmdelta), 0);
    envset(mod->module.exports, "scroll", makebuiltin(inputscroll), 0);
    envset(mod->module.exports, "gamepadconnected", makebuiltin(inputgamepadconnected), 0);
    envset(mod->module.exports, "gamepadaxis", makebuiltin(inputgamepadaxis), 0);
    envset(mod->module.exports, "gamepadbutton", makebuiltin(inputgamepadbutton), 0);
    envset(env, "input", mod, 0);
}
