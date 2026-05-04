#include "../core/axton.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct widget widget;
typedef struct window window;

struct widget {
    int type;
    int x,y,w,h;
    char* text;
    int value;
    object* callback;
    widget* children;
    widget* next;
    widget* parent;
    void* native;
};

struct window {
    Display* dpy;
    Window win;
    GC gc;
    XftDraw* xftdraw;
    XftFont* font;
    widget* root;
    int running;
    int focused;
};

static window* currentwin = NULL;
static XColor black, white;

static void drawtext(window* win, int x, int y, char* text) {
    XftColor xftcol;
    XftColorAllocName(win->dpy, DefaultVisual(win->dpy, DefaultScreen(win->dpy)),
                      DefaultColormap(win->dpy, DefaultScreen(win->dpy)),
                      "black", &xftcol);
    XftDrawStringUtf8(win->xftdraw, &xftcol, win->font, x, y, (XftChar8*)text, strlen(text));
}

static void drawrect(window* win, int x, int y, int w, int h, int fill, unsigned long color) {
    XSetForeground(win->dpy, win->gc, color);
    if(fill) XFillRectangle(win->dpy, win->win, win->gc, x, y, w, h);
    else XDrawRectangle(win->dpy, win->win, win->gc, x, y, w, h);
}

static void drawbutton(window* win, widget* w) {
    drawrect(win, w->x, w->y, w->w, w->h, 1, 0xC0C0C0);
    drawrect(win, w->x, w->y, w->w, w->h, 0, 0x000000);
    int tw = strlen(w->text) * 8;
    drawtext(win, w->x + (w->w - tw)/2, w->y + w->h - 8, w->text);
}

static void drawlabel(window* win, widget* w) {
    drawtext(win, w->x, w->y + 12, w->text);
}

static void drawtextbox(window* win, widget* w) {
    drawrect(win, w->x, w->y, w->w, w->h, 1, 0xFFFFFF);
    drawrect(win, w->x, w->y, w->w, w->h, 0, 0x000000);
    drawtext(win, w->x + 4, w->y + 14, w->text ? w->text : "");
}

static void drawcheckbox(window* win, widget* w) {
    drawrect(win, w->x, w->y, 16, 16, 0, 0x000000);
    if(w->value) {
        XDrawLine(win->dpy, win->win, win->gc, w->x+3, w->y+8, w->x+13, w->y+8);
        XDrawLine(win->dpy, win->win, win->gc, w->x+8, w->y+3, w->x+8, w->y+13);
    }
    drawtext(win, w->x+20, w->y+12, w->text);
}

static void drawslider(window* win, widget* w) {
    drawrect(win, w->x, w->y, w->w, 16, 1, 0xD0D0D0);
    drawrect(win, w->x, w->y, w->w, 16, 0, 0x000000);
    int knob = (w->value - 0) * (w->w - 20) / (100 - 0);
    drawrect(win, w->x + knob, w->y, 20, 16, 1, 0x808080);
}

static void handleclick(window* win, widget* w, int mx, int my) {
    if(mx < w->x || mx > w->x + w->w || my < w->y || my > w->y + w->h) return;
    if(w->type == 1 && w->callback) callfunc(w->callback, NULL, 0, globalenv);
    if(w->type == 4) { w->value = !w->value; if(w->callback) callfunc(w->callback, NULL, 0, globalenv); }
    if(w->type == 8) { int val = (mx - w->x) * 100 / (w->w - 20); if(val<0)val=0; if(val>100)val=100; w->value=val; if(w->callback) callfunc(w->callback, NULL, 0, globalenv); }
}

static void handlekey(window* win, widget* w, char key) {
    if(w->type != 3) return;
    if(key == 8 && strlen(w->text)>0) w->text[strlen(w->text)-1]=0;
    else if(key >= 32 && key <= 126) {
        int len = strlen(w->text);
        w->text = realloc(w->text, len+2);
        w->text[len]=key; w->text[len+1]=0;
    }
    if(w->callback) callfunc(w->callback, NULL, 0, globalenv);
}

static void drawwidgets(window* win, widget* w) {
    while(w) {
        switch(w->type) {
            case 1: drawbutton(win, w); break;
            case 2: drawlabel(win, w); break;
            case 3: drawtextbox(win, w); break;
            case 4: drawcheckbox(win, w); break;
            case 8: drawslider(win, w); break;
        }
        drawwidgets(win, w->children);
        w = w->next;
    }
}

static widget* findwidgetat(window* win, widget* w, int x, int y) {
    while(w) {
        if(x >= w->x && x <= w->x+w->w && y >= w->y && y <= w->y+w->h) return w;
        widget* found = findwidgetat(win, w->children, x, y);
        if(found) return found;
        w = w->next;
    }
    return NULL;
}

object* guicreatewindow(object** args, int argc, void* env) {
    char* title = (argc>0 && args[0]->type==2) ? args[0]->sval : "Axton";
    int w = (argc>1 && args[1]->type==0) ? args[1]->ival : 800;
    int h = (argc>2 && args[2]->type==0) ? args[2]->ival : 600;
    window* win = malloc(sizeof(window));
    win->dpy = XOpenDisplay(NULL);
    win->running = 1;
    int screen = DefaultScreen(win->dpy);
    win->win = XCreateSimpleWindow(win->dpy, RootWindow(win->dpy, screen), 0, 0, w, h, 1, 0, 0xFFFFFF);
    XStoreName(win->dpy, win->win, title);
    XSelectInput(win->dpy, win->win, ExposureMask | ButtonPressMask | KeyPressMask);
    XMapWindow(win->dpy, win->win);
    win->gc = XCreateGC(win->dpy, win->win, 0, NULL);
    win->xftdraw = XftDrawCreate(win->dpy, win->win, DefaultVisual(win->dpy, screen), DefaultColormap(win->dpy, screen));
    win->font = XftFontOpenName(win->dpy, screen, "Sans-12");
    win->root = NULL;
    win->focused = NULL;
    currentwin = win;
    return makenative(win, NULL);
}

object* guibutton(object** args, int argc, void* env) {
    if(!currentwin) throwexception("no window");
    widget* w = calloc(1, sizeof(widget));
    w->type = 1;
    w->text = (argc>0 && args[0]->type==2) ? strdup(args[0]->sval) : strdup("");
    w->x = (argc>1 && args[1]->type==0) ? args[1]->ival : 10;
    w->y = (argc>2 && args[2]->type==0) ? args[2]->ival : 10;
    w->w = (argc>3 && args[3]->type==0) ? args[3]->ival : 80;
    w->h = (argc>4 && args[4]->type==0) ? args[4]->ival : 30;
    w->callback = (argc>5) ? args[5] : NULL;
    w->next = currentwin->root;
    currentwin->root = w;
    return makenative(w, NULL);
}

object* guilabel(object** args, int argc, void* env) {
    if(!currentwin) throwexception("no window");
    widget* w = calloc(1, sizeof(widget));
    w->type = 2;
    w->text = (argc>0 && args[0]->type==2) ? strdup(args[0]->sval) : strdup("");
    w->x = (argc>1 && args[1]->type==0) ? args[1]->ival : 10;
    w->y = (argc>2 && args[2]->type==0) ? args[2]->ival : 10;
    w->next = currentwin->root;
    currentwin->root = w;
    return makenative(w, NULL);
}

object* guitextbox(object** args, int argc, void* env) {
    if(!currentwin) throwexception("no window");
    widget* w = calloc(1, sizeof(widget));
    w->type = 3;
    w->text = strdup("");
    w->x = (argc>0 && args[0]->type==0) ? args[0]->ival : 10;
    w->y = (argc>1 && args[1]->type==0) ? args[1]->ival : 10;
    w->w = (argc>2 && args[2]->type==0) ? args[2]->ival : 200;
    w->h = 24;
    w->callback = (argc>3) ? args[3] : NULL;
    w->next = currentwin->root;
    currentwin->root = w;
    return makenative(w, NULL);
}

object* guicheckbox(object** args, int argc, void* env) {
    if(!currentwin) throwexception("no window");
    widget* w = calloc(1, sizeof(widget));
    w->type = 4;
    w->text = (argc>0 && args[0]->type==2) ? strdup(args[0]->sval) : strdup("");
    w->x = (argc>1 && args[1]->type==0) ? args[1]->ival : 10;
    w->y = (argc>2 && args[2]->type==0) ? args[2]->ival : 10;
    w->value = 0;
    w->callback = (argc>3) ? args[3] : NULL;
    w->next = currentwin->root;
    currentwin->root = w;
    return makenative(w, NULL);
}

object* guislider(object** args, int argc, void* env) {
    if(!currentwin) throwexception("no window");
    widget* w = calloc(1, sizeof(widget));
    w->type = 8;
    w->x = (argc>0 && args[0]->type==0) ? args[0]->ival : 10;
    w->y = (argc>1 && args[1]->type==0) ? args[1]->ival : 10;
    w->w = (argc>2 && args[2]->type==0) ? args[2]->ival : 200;
    w->h = 16;
    w->value = 50;
    w->callback = (argc>3) ? args[3] : NULL;
    w->next = currentwin->root;
    currentwin->root = w;
    return makenative(w, NULL);
}

object* guirun(object** args, int argc, void* env) {
    if(!currentwin) throwexception("no window");
    XEvent ev;
    while(currentwin->running) {
        while(XPending(currentwin->dpy)) {
            XNextEvent(currentwin->dpy, &ev);
            if(ev.type == Expose) {
                XClearWindow(currentwin->dpy, currentwin->win);
                drawwidgets(currentwin, currentwin->root);
            }
            if(ev.type == ButtonPress) {
                int x = ev.xbutton.x, y = ev.xbutton.y;
                widget* w = findwidgetat(currentwin, currentwin->root, x, y);
                if(w) handleclick(currentwin, w, x, y);
            }
            if(ev.type == KeyPress) {
                char buf[2] = {0};
                XLookupString(&ev.xkey, buf, 1, NULL, NULL);
                if(currentwin->focused) handlekey(currentwin, currentwin->focused, buf[0]);
            }
        }
        usleep(16666);
    }
    return makenone();
}

void registerguireallib(environment* env) {
    object* mod = makemodule("gui", NULL);
    envset(mod->module.exports, "createwindow", makebuiltin(guicreatewindow), 0);
    envset(mod->module.exports, "button", makebuiltin(guibutton), 0);
    envset(mod->module.exports, "label", makebuiltin(guilabel), 0);
    envset(mod->module.exports, "textbox", makebuiltin(guitextbox), 0);
    envset(mod->module.exports, "checkbox", makebuiltin(guicheckbox), 0);
    envset(mod->module.exports, "slider", makebuiltin(guislider), 0);
    envset(mod->module.exports, "run", makebuiltin(guirun), 0);
    envset(env, "gui", mod, 0);
}
