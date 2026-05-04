#include "../core/axton.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <math.h>
#include <unistd.h>

typedef struct {
    Display *dpy;
    Window win;
    GLXContext ctx;
    int w, h;
    int running;
    object *onidle;
    object *onkey;
} appcontext;

static appcontext *app = NULL;

static void redraw(void) {
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (double)app->w/app->h, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
    static float angle = 0;
    angle += 0.02;
    glRotatef(angle, 1, 1, 0);
    glBegin(GL_QUADS);
    glColor3f(1,0,0); glVertex3f(-1,-1, 1);
    glColor3f(0,1,0); glVertex3f( 1,-1, 1);
    glColor3f(0,0,1); glVertex3f( 1, 1, 1);
    glColor3f(1,1,0); glVertex3f(-1, 1, 1);
    glEnd();
    glXSwapBuffers(app->dpy, app->win);
}

static void handleevents(void) {
    XEvent ev;
    while (XPending(app->dpy)) {
        XNextEvent(app->dpy, &ev);
        if (ev.type == KeyPress) {
            if (app->onkey) {
                char buf[2] = {0};
                XLookupString(&ev.xkey, buf, 1, NULL, NULL);
                object *args[1] = {makestring(buf)};
                callfunc(app->onkey, args, 1, globalenv);
            }
            if (ev.xkey.keycode == 9) app->running = 0;
        }
        if (ev.type == ConfigureNotify) {
            app->w = ev.xconfigure.width;
            app->h = ev.xconfigure.height;
            glViewport(0, 0, app->w, app->h);
        }
        if (ev.type == ClientMessage && ev.xclient.data.l[0] == (Atom)ev.xclient.data.l[1]) {
            app->running = 0;
        }
    }
}

static int attrs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, None };

object *graphicscreatewindow(object **args, int argc, void *env) {
    char *title = (argc>0 && args[0]->type==2) ? args[0]->sval : "Axton";
    int w = (argc>1 && args[0]->type==0) ? args[1]->ival : 800;
    int h = (argc>2 && args[0]->type==0) ? args[2]->ival : 600;
    app = malloc(sizeof(appcontext));
    app->dpy = XOpenDisplay(NULL);
    if (!app->dpy) throwexception("cannot open display");
    int screen = DefaultScreen(app->dpy);
    Window root = RootWindow(app->dpy, screen);
    XVisualInfo *vi = glXChooseVisual(app->dpy, screen, attrs);
    if (!vi) throwexception("no GL visual");
    Colormap cmap = XCreateColormap(app->dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
    app->win = XCreateWindow(app->dpy, root, 0, 0, w, h, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    XStoreName(app->dpy, app->win, title);
    XMapWindow(app->dpy, app->win);
    app->ctx = glXCreateContext(app->dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(app->dpy, app->win, app->ctx);
    glEnable(GL_DEPTH_TEST);
    app->w = w; app->h = h;
    app->running = 1;
    app->onidle = NULL;
    app->onkey = NULL;
    return makeguiwin(app->dpy, (void*)(long)app->win, w, h);
}

object *graphicsonidle(object **args, int argc, void *env) {
    if (argc<1) throwexception("onidle needs function");
    app->onidle = args[0];
    return makenone();
}

object *graphicsonkey(object **args, int argc, void *env) {
    if (argc<1) throwexception("onkey needs function");
    app->onkey = args[0];
    return makenone();
}

object *graphicsrun(object **args, int argc, void *env) {
    while (app->running) {
        handleevents();
        if (app->onidle) {
            callfunc(app->onidle, NULL, 0, globalenv);
        }
        redraw();
        usleep(16666);
    }
    glXMakeCurrent(app->dpy, None, NULL);
    glXDestroyContext(app->dpy, app->ctx);
    XDestroyWindow(app->dpy, app->win);
    XCloseDisplay(app->dpy);
    free(app);
    return makenone();
}

void registergraphicslib(environment *env) {
    object *mod = makemodule("graphics", NULL);
    envset(mod->module.exports, "createwindow", makebuiltin(graphicscreatewindow), 0);
    envset(mod->module.exports, "onidle", makebuiltin(graphicsonidle), 0);
    envset(mod->module.exports, "onkey", makebuiltin(graphicsonkey), 0);
    envset(mod->module.exports, "run", makebuiltin(graphicsrun), 0);
    envset(env, "graphics", mod, 0);
}
