#include "../core/axton.h"

#ifdef _WIN32
#include <windows.h>

static HWND mainwindow = NULL;

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

object *guiwindow(char *title, int width, int height) {
    HINSTANCE hinst = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = wndproc;
    wc.hInstance = hinst;
    wc.lpszClassName = "axtonwin";
    RegisterClass(&wc);
    mainwindow = CreateWindow("axtonwin", title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hinst, NULL);
    ShowWindow(mainwindow, SW_SHOW);
    return makestring("window created");
}

object *guirun(void) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return makenone();
}

#elif defined(__linux__) && !defined(__ANDROID__)
#include <X11/Xlib.h>

static Display *xdisplay;
static Window xwindow;

object *guiwindow(char *title, int width, int height) {
    xdisplay = XOpenDisplay(NULL);
    int screen = DefaultScreen(xdisplay);
    xwindow = XCreateSimpleWindow(xdisplay, RootWindow(xdisplay, screen),
        0, 0, width, height, 1, BlackPixel(xdisplay, screen), WhitePixel(xdisplay, screen));
    XStoreName(xdisplay, xwindow, title);
    XSelectInput(xdisplay, xwindow, ExposureMask | KeyPressMask);
    XMapWindow(xdisplay, xwindow);
    return makestring("window created");
}

object *guirun(void) {
    XEvent event;
    while (1) {
        XNextEvent(xdisplay, &event);
        if (event.type == KeyPress) break;
    }
    return makenone();
}

#else

object *guiwindow(char *title, int width, int height) {
    platformlog("gui not supported on this platform\n");
    return makenone();
}

object *guirun(void) {
    return makenone();
}

#endif

object *builtinwindow(object **args, int argc, environment *env) {
    char *title = argc > 0 && args[0]->type == 2 ? args[0]->sval : "axton";
    int width = argc > 1 && args[1]->type == 0 ? args[1]->ival : 800;
    int height = argc > 2 && args[2]->type == 0 ? args[2]->ival : 600;
    return guiwindow(title, width, height);
}

object *builtinguirun(object **args, int argc, environment *env) {
    return guirun();
}

void registerguilib(environment *env) {
    object *guimod = makemodule("gui", NULL);
    envset(guimod->module.exports, "window", makebuiltin(builtinwindow), 0);
    envset(guimod->module.exports, "run", makebuiltin(builtinguirun), 0);
    envset(env, "gui", guimod, 0);
}
