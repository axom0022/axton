#include "../core/axton.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/wglew.h>
#endif

typedef struct overlay {
    void *display;
    void *window;
    void *context;
    int w, h;
    int x, y;
    int running;
} overlay;

#ifdef __linux__
static Display *xdpy;
static Window xwin;
static GLXContext xctx;

static int attrlist[] = {
    GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24,
    GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
    None
};

overlay *overlaycreate(int w, int h, int x, int y) {
    overlay *ov = malloc(sizeof(overlay));
    ov->w = w; ov->h = h; ov->x = x; ov->y = y;
    ov->display = XOpenDisplay(NULL);
    if (!ov->display) { free(ov); return NULL; }
    xdpy = ov->display;
    int screen = DefaultScreen(xdpy);
    Window root = RootWindow(xdpy, screen);
    XVisualInfo *vi = glXChooseVisual(xdpy, screen, attrlist);
    if (!vi) { XCloseDisplay(xdpy); free(ov); return NULL; }
    Colormap cmap = XCreateColormap(xdpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
    swa.override_redirect = True;
    swa.background_pixmap = None;
    swa.background_pixel = 0;
    unsigned long mask = CWColormap | CWEventMask | CWOverrideRedirect | CWBackPixmap | CWBackPixel;
    xwin = XCreateWindow(xdpy, root, x, y, w, h, 0, vi->depth, InputOutput, vi->visual, mask, &swa);
    ov->window = (void*)(long)xwin;
    Atom wm_delete = XInternAtom(xdpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(xdpy, xwin, &wm_delete, 1);
    Atom state = XInternAtom(xdpy, "_NET_WM_STATE", False);
    Atom above = XInternAtom(xdpy, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(xdpy, xwin, state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&above, 1);
    ov->context = glXCreateContext(xdpy, vi, NULL, GL_TRUE);
    xctx = ov->context;
    glXMakeCurrent(xdpy, xwin, xctx);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
    glOrtho(0, w, h, 0, -1, 1);
    glViewport(0, 0, w, h);
    ov->running = 1;
    XMapWindow(xdpy, xwin);
    return ov;
}

void overlaydestroy(overlay *ov) {
    if (!ov) return;
    glXMakeCurrent(xdpy, None, NULL);
    glXDestroyContext(xdpy, xctx);
    XDestroyWindow(xdpy, xwin);
    XCloseDisplay(xdpy);
    free(ov);
}

void overlaybegin(overlay *ov) {
    if (!ov) return;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glOrtho(0, ov->w, ov->h, 0, -1, 1);
}

void overlayend(overlay *ov) {
    if (!ov) return;
    glXSwapBuffers(xdpy, xwin);
}

void overlaydrawtext(overlay *ov, int x, int y, char *text, float r, float g, float b, float size) {
    if (!ov) return;
    glColor4f(r, g, b, 1);
    glRasterPos2i(x, y);
    for (char *p = text; *p; p++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
}

void overlaydrawrect(overlay *ov, int x, int y, int w, int h, float r, float g, float b, int fill) {
    if (!ov) return;
    glColor4f(r, g, b, 1);
    if (fill) {
        glBegin(GL_QUADS);
        glVertex2i(x, y); glVertex2i(x+w, y); glVertex2i(x+w, y+h); glVertex2i(x, y+h);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        glVertex2i(x, y); glVertex2i(x+w, y); glVertex2i(x+w, y+h); glVertex2i(x, y+h);
        glEnd();
    }
}

void overlaydrawline(overlay *ov, int x1, int y1, int x2, int y2, float r, float g, float b) {
    if (!ov) return;
    glColor4f(r, g, b, 1);
    glBegin(GL_LINES);
    glVertex2i(x1, y1); glVertex2i(x2, y2);
    glEnd();
}

void overlaydrawcircle(overlay *ov, int cx, int cy, int radius, float r, float g, float b, int fill) {
    if (!ov) return;
    glColor4f(r, g, b, 1);
    float step = 2 * 3.14159 / 36;
    glBegin(fill ? GL_POLYGON : GL_LINE_LOOP);
    for (float angle = 0; angle < 2 * 3.14159; angle += step) {
        glVertex2f(cx + radius * cos(angle), cy + radius * sin(angle));
    }
    glEnd();
}

void overlaydrawimage(overlay *ov, int x, int y, char *path) {
}

void overlaysettitle(overlay *ov, char *title) {
    if (!ov) return;
    XStoreName(xdpy, xwin, title);
}

void overlaysetposition(overlay *ov, int x, int y) {
    if (!ov) return;
    ov->x = x; ov->y = y;
    XMoveWindow(xdpy, xwin, x, y);
}

void overlaysetsize(overlay *ov, int w, int h) {
    if (!ov) return;
    ov->w = w; ov->h = h;
    XResizeWindow(xdpy, xwin, w, h);
    glViewport(0, 0, w, h);
}

void overlayfocus(overlay *ov) {
    if (!ov) return;
    XRaiseWindow(xdpy, xwin);
    XSetInputFocus(xdpy, xwin, RevertToParent, CurrentTime);
}

void overlayhide(overlay *ov) {
    if (!ov) return;
    XUnmapWindow(xdpy, xwin);
}

void overlayshow(overlay *ov) {
    if (!ov) return;
    XMapWindow(xdpy, xwin);
}

#elif _WIN32

static HGLRC wglctx;
static HDC wgldc;
static HWND wwin;

overlay *overlaycreate(int w, int h, int x, int y) {
    overlay *ov = malloc(sizeof(overlay));
    ov->w = w; ov->h = h; ov->x = x; ov->y = y;
    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "OverlayClass";
    wc.style = CS_OWNDC;
    RegisterClass(&wc);
    wwin = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
                          "OverlayClass", "Overlay",
                          WS_POPUP | WS_VISIBLE,
                          x, y, w, h, NULL, NULL, hInst, NULL);
    if (!wwin) { free(ov); return NULL; }
    SetLayeredWindowAttributes(wwin, 0, 0, LWA_ALPHA);
    wgldc = GetDC(wwin);
    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };
    int format = ChoosePixelFormat(wgldc, &pfd);
    SetPixelFormat(wgldc, format, &pfd);
    wglctx = wglCreateContext(wgldc);
    wglMakeCurrent(wgldc, wglctx);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0,0,0,0);
    glOrtho(0, w, h, 0, -1, 1);
    glViewport(0,0,w,h);
    ov->window = wwin;
    ov->context = wglctx;
    ov->running = 1;
    ShowWindow(wwin, SW_SHOW);
    return ov;
}

void overlaydestroy(overlay *ov) {
    if (!ov) return;
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(wglctx);
    ReleaseDC(wwin, wgldc);
    DestroyWindow(wwin);
    free(ov);
}

void overlaybegin(overlay *ov) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glOrtho(0, ov->w, ov->h, 0, -1, 1);
}

void overlayend(overlay *ov) {
    SwapBuffers(wgldc);
}

void overlaydrawtext(overlay *ov, int x, int y, char *text, float r, float g, float b, float size) {
    HDC hdc = GetDC(wwin);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(r*255, g*255, b*255));
    SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
    TextOut(hdc, x, y, text, strlen(text));
    ReleaseDC(wwin, hdc);
}

void overlaydrawrect(overlay *ov, int x, int y, int w, int h, float r, float g, float b, int fill) {
    glColor4f(r,g,b,1);
    if (fill) {
        glBegin(GL_QUADS);
        glVertex2i(x,y); glVertex2i(x+w,y); glVertex2i(x+w,y+h); glVertex2i(x,y+h);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        glVertex2i(x,y); glVertex2i(x+w,y); glVertex2i(x+w,y+h); glVertex2i(x,y+h);
        glEnd();
    }
}

void overlaydrawline(overlay *ov, int x1, int y1, int x2, int y2, float r, float g, float b) {
    glColor4f(r,g,b,1);
    glBegin(GL_LINES);
    glVertex2i(x1,y1); glVertex2i(x2,y2);
    glEnd();
}

void overlaydrawcircle(overlay *ov, int cx, int cy, int radius, float r, float g, float b, int fill) {
    glColor4f(r,g,b,1);
    float step = 2*3.14159/36;
    glBegin(fill?GL_POLYGON:GL_LINE_LOOP);
    for (float a=0; a<2*3.14159; a+=step) {
        glVertex2f(cx+radius*cos(a), cy+radius*sin(a));
    }
    glEnd();
}

void overlaydrawimage(overlay *ov, int x, int y, char *path) {}
void overlaysettitle(overlay *ov, char *title) { SetWindowText(wwin, title); }
void overlaysetposition(overlay *ov, int x, int y) { SetWindowPos(wwin, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER); }
void overlaysetsize(overlay *ov, int w, int h) { ov->w=w; ov->h=h; SetWindowPos(wwin, NULL, 0,0,w,h, SWP_NOMOVE | SWP_NOZORDER); glViewport(0,0,w,h); }
void overlayfocus(overlay *ov) { SetForegroundWindow(wwin); }
void overlayhide(overlay *ov) { ShowWindow(wwin, SW_HIDE); }
void overlayshow(overlay *ov) { ShowWindow(wwin, SW_SHOW); }

#else
overlay *overlaycreate(int w, int h, int x, int y) { return NULL; }
void overlaydestroy(overlay *ov) {}
void overlaybegin(overlay *ov) {}
void overlayend(overlay *ov) {}
void overlaydrawtext(overlay *ov, int x, int y, char *text, float r, float g, float b, float size) {}
void overlaydrawrect(overlay *ov, int x, int y, int w, int h, float r, float g, float b, int fill) {}
void overlaydrawline(overlay *ov, int x1, int y1, int x2, int y2, float r, float g, float b) {}
void overlaydrawcircle(overlay *ov, int cx, int cy, int radius, float r, float g, float b, int fill) {}
void overlaydrawimage(overlay *ov, int x, int y, char *path) {}
void overlaysettitle(overlay *ov, char *title) {}
void overlaysetposition(overlay *ov, int x, int y) {}
void overlaysetsize(overlay *ov, int w, int h) {}
void overlayfocus(overlay *ov) {}
void overlayhide(overlay *ov) {}
void overlayshow(overlay *ov) {}
#endif

void registeroverlaylib(environment *env) {
    object *mod = makemodule("overlay", NULL);
    envset(mod->module.exports, "create", makebuiltin(overlaycreate), 0);
    envset(mod->module.exports, "destroy", makebuiltin(overlaydestroy), 0);
    envset(mod->module.exports, "begin", makebuiltin(overlaybegin), 0);
    envset(mod->module.exports, "end", makebuiltin(overlayend), 0);
    envset(mod->module.exports, "drawtext", makebuiltin(overlaydrawtext), 0);
    envset(mod->module.exports, "drawrect", makebuiltin(overlaydrawrect), 0);
    envset(mod->module.exports, "drawline", makebuiltin(overlaydrawline), 0);
    envset(mod->module.exports, "drawcircle", makebuiltin(overlaydrawcircle), 0);
    envset(mod->module.exports, "drawimage", makebuiltin(overlaydrawimage), 0);
    envset(mod->module.exports, "settitle", makebuiltin(overlaysettitle), 0);
    envset(mod->module.exports, "setposition", makebuiltin(overlaysetposition), 0);
    envset(mod->module.exports, "setsize", makebuiltin(overlaysetsize), 0);
    envset(mod->module.exports, "focus", makebuiltin(overlayfocus), 0);
    envset(mod->module.exports, "hide", makebuiltin(overlayhide), 0);
    envset(mod->module.exports, "show", makebuiltin(overlayshow), 0);
    envset(env, "overlay", mod, 0);
}
