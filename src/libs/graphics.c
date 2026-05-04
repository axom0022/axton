#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct app {
    int running;
    void* data;
};

struct window {
    char* title;
    int w,h;
    void* handle;
};

struct widget {
    int type;
    char* text;
    int x,y,w,h;
    object* callback;
    void* native;
};

struct canvas {
    int w,h;
    unsigned int* pixels;
};

struct engine3d {
    int w,h;
    float rx,ry,rz;
    float tx,ty,tz;
    float sx,sy,sz;
    void* gl;
};

app* createapp(void) {
    app* a = malloc(sizeof(app));
    a->running = 0;
    a->data = NULL;
    return a;
}

void runapp(app* a) {
    a->running = 1;
    while(a->running) {
        printf("app loop\n");
        break;
    }
}

void stopapp(app* a) {
    a->running = 0;
}

window* newwindow(app* a, char* title, int w, int h) {
    window* win = malloc(sizeof(window));
    win->title = strdup(title);
    win->w = w; win->h = h;
    win->handle = NULL;
    printf("window %s %dx%d\n", title, w, h);
    return win;
}

widget* newbutton(window* w, char* text, int x, int y, int wd, int ht, object* onclick) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 1;
    wg->text = strdup(text);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    wg->callback = onclick;
    return wg;
}

widget* newlabel(window* w, char* text, int x, int y) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 2;
    wg->text = strdup(text);
    wg->x = x; wg->y = y;
    return wg;
}

widget* newtextbox(window* w, int x, int y, int wd, object* onchange) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 3;
    wg->x = x; wg->y = y; wg->w = wd;
    wg->callback = onchange;
    return wg;
}

widget* newcheckbox(window* w, char* text, int x, int y, object* oncheck) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 4;
    wg->text = strdup(text);
    wg->x = x; wg->y = y;
    wg->callback = oncheck;
    return wg;
}

widget* newradio(window* w, char* text, int x, int y, object* onselect) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 5;
    wg->text = strdup(text);
    wg->x = x; wg->y = y;
    wg->callback = onselect;
    return wg;
}

widget* newlistbox(window* w, int x, int y, int wd, int ht, object* onselect) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 6;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    wg->callback = onselect;
    return wg;
}

widget* newdropdown(window* w, object* items, int x, int y, int wd, object* onselect) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 7;
    wg->x = x; wg->y = y; wg->w = wd;
    wg->callback = onselect;
    return wg;
}

widget* newslider(window* w, int x, int y, int wd, int min, int max, object* onchange) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 8;
    wg->x = x; wg->y = y; wg->w = wd;
    wg->callback = onchange;
    return wg;
}

widget* newprogress(window* w, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 9;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newimage(window* w, char* path, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 10;
    wg->text = strdup(path);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newwebview(window* w, char* url, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 11;
    wg->text = strdup(url);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newtable(window* w, object* cols, object* rows, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 12;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newchart(window* w, object* data, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 13;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newcanvas(window* w, int wd, int ht, object* ondraw) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 14;
    wg->w = wd; wg->h = ht;
    wg->callback = ondraw;
    return wg;
}

widget* newmenubar(window* w) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 15;
    return wg;
}

widget* newmenu(widget* bar, char* name) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 16;
    wg->text = strdup(name);
    return wg;
}

widget* newmenuitem(widget* menu, char* name, object* onclick) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 17;
    wg->text = strdup(name);
    wg->callback = onclick;
    return wg;
}

widget* newtabs(window* w, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 18;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newtabpage(widget* tabs, char* title) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 19;
    wg->text = strdup(title);
    return wg;
}

widget* newpanel(window* w, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 20;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newspinner(window* w, int x, int y, int wd, object* onchange) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 21;
    wg->x = x; wg->y = y; wg->w = wd;
    wg->callback = onchange;
    return wg;
}

widget* newdatepicker(window* w, int x, int y, int wd, object* onchange) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 22;
    wg->x = x; wg->y = y; wg->w = wd;
    wg->callback = onchange;
    return wg;
}

widget* newcolorpicker(window* w, int x, int y, object* onchange) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 23;
    wg->x = x; wg->y = y;
    wg->callback = onchange;
    return wg;
}

widget* newfilepicker(window* w, int x, int y, int wd, object* onselect) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 24;
    wg->x = x; wg->y = y; wg->w = wd;
    wg->callback = onselect;
    return wg;
}

widget* newtoolbar(window* w) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 25;
    return wg;
}

widget* newtoolbutton(widget* bar, char* icon, object* onclick) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 26;
    wg->text = strdup(icon);
    wg->callback = onclick;
    return wg;
}

widget* newstatusbar(window* w) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 27;
    return wg;
}

widget* newsplitter(window* w, int x, int y, int wd, int ht, int vertical) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 28;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newtree(window* w, object* nodes, int x, int y, int wd, int ht, object* onselect) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 29;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    wg->callback = onselect;
    return wg;
}

widget* newhtmlview(window* w, char* html, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 30;
    wg->text = strdup(html);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newmarkdown(window* w, char* text, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 31;
    wg->text = strdup(text);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newvideo(window* w, char* path, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 32;
    wg->text = strdup(path);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newaudio(window* w, char* path, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 33;
    wg->text = strdup(path);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newmap(window* w, double lat, double lon, int zoom, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 34;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newpdfviewer(window* w, char* path, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 35;
    wg->text = strdup(path);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newcodeeditor(window* w, char* code, int x, int y, int wd, int ht, object* onchange) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 36;
    wg->text = strdup(code);
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    wg->callback = onchange;
    return wg;
}

widget* newterminal(window* w, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 37;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newnotebook(window* w, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 38;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newdebugger(window* w, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 39;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newlogger(window* w, int x, int y, int wd, int ht) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 40;
    wg->x = x; wg->y = y; wg->w = wd; wg->h = ht;
    return wg;
}

widget* newtimer(window* w, int ms, object* ontimeout) {
    widget* wg = calloc(1, sizeof(widget));
    wg->type = 41;
    wg->callback = ontimeout;
    return wg;
}

void settext(widget* w, char* text) {
    if(w->text) free(w->text);
    w->text = strdup(text);
}

char* gettext(widget* w) {
    return w->text ? w->text : "";
}

void setvalue(widget* w, double val) {
    (void)val;
}

double getvalue(widget* w) {
    return 0.0;
}

void appenditem(widget* w, char* item) {
    (void)item;
}

void clearitems(widget* w) {
}

void show(widget* w) {
}

void hide(widget* w) {
}

void enable(widget* w) {
}

void disable(widget* w) {
}

void focus(widget* w) {
}

void destroy(widget* w) {
    free(w->text);
    free(w);
}

void msgbox(char* title, char* msg) {
    printf("msgbox %s %s\n", title, msg);
}

char* opendialog(char* title, char* filter) {
    return "";
}

char* savedialog(char* title, char* filter) {
    return "";
}

void alert(char* msg) {
    printf("alert %s\n", msg);
}

int confirm(char* msg) {
    return 0;
}

void notify(char* title, char* msg, int ms) {
    printf("notify %s %s\n", title, msg);
}

engine3d* newengine3d(int w, int h) {
    engine3d* e = malloc(sizeof(engine3d));
    e->w = w; e->h = h;
    e->rx = e->ry = e->rz = 0;
    e->tx = e->ty = e->tz = 0;
    e->sx = e->sy = e->sz = 1;
    e->gl = NULL;
    return e;
}

void render3d(engine3d* e) {
    printf("render3d\n");
}

void rotate3d(engine3d* e, float x, float y, float z) {
    e->rx += x; e->ry += y; e->rz += z;
}

void translate3d(engine3d* e, float x, float y, float z) {
    e->tx += x; e->ty += y; e->tz += z;
}

void scale3d(engine3d* e, float x, float y, float z) {
    e->sx *= x; e->sy *= y; e->sz *= z;
}

void loadmodel(engine3d* e, char* path) {
    printf("loadmodel %s\n", path);
}

void loadtexture(engine3d* e, char* path) {
    printf("loadtexture %s\n", path);
}

void addlight(engine3d* e, float x, float y, float z, float r, float g, float b) {
    printf("light at %f,%f,%f color %f,%f,%f\n", x, y, z, r, g, b);
}

void setcamera(engine3d* e, float x, float y, float z, float tx, float ty, float tz) {
    printf("camera pos %f,%f,%f target %f,%f,%f\n", x, y, z, tx, ty, tz);
}

void setbackground(engine3d* e, float r, float g, float b) {
    printf("background %f,%f,%f\n", r, g, b);
}

void wireframe(engine3d* e, int on) {
    printf("wireframe %d\n", on);
}

void setshader(engine3d* e, char* vert, char* frag) {
    printf("shader %s %s\n", vert, frag);
}

void billboard(engine3d* e, char* text, float x, float y, float z) {
    printf("billboard %s at %f,%f,%f\n", text, x, y, z);
}

void particles(engine3d* e, int count, float x, float y, float z) {
    printf("particles %d at %f,%f,%f\n", count, x, y, z);
}

void animate3d(engine3d* e, float speed) {
    printf("animate speed %f\n", speed);
}

void raycast(engine3d* e, int x, int y, object* callback) {
    printf("raycast %d,%d\n", x, y);
}

void enablecollision(engine3d* e, int on) {
    printf("collision %d\n", on);
}

void setgravity(engine3d* e, float force) {
    printf("gravity %f\n", force);
}

void enablephysics(engine3d* e, int on) {
    printf("physics %d\n", on);
}

void setjoystick(engine3d* e, int idx, object* callback) {
    printf("joystick %d\n", idx);
}

void enablevr(engine3d* e, int on) {
    printf("vr %d\n", on);
}

void registergraphicslib(environment* env) {
    object* mod = makemodule("graphics", NULL);
    envset(mod->module.exports, "createapp", makebuiltin(createapp), 0);
    envset(mod->module.exports, "runapp", makebuiltin(runapp), 0);
    envset(mod->module.exports, "stopapp", makebuiltin(stopapp), 0);
    envset(mod->module.exports, "newwindow", makebuiltin(newwindow), 0);
    envset(mod->module.exports, "newbutton", makebuiltin(newbutton), 0);
    envset(mod->module.exports, "newlabel", makebuiltin(newlabel), 0);
    envset(mod->module.exports, "newtextbox", makebuiltin(newtextbox), 0);
    envset(mod->module.exports, "newcheckbox", makebuiltin(newcheckbox), 0);
    envset(mod->module.exports, "newradio", makebuiltin(newradio), 0);
    envset(mod->module.exports, "newlistbox", makebuiltin(newlistbox), 0);
    envset(mod->module.exports, "newdropdown", makebuiltin(newdropdown), 0);
    envset(mod->module.exports, "newslider", makebuiltin(newslider), 0);
    envset(mod->module.exports, "newprogress", makebuiltin(newprogress), 0);
    envset(mod->module.exports, "newimage", makebuiltin(newimage), 0);
    envset(mod->module.exports, "newwebview", makebuiltin(newwebview), 0);
    envset(mod->module.exports, "newtable", makebuiltin(newtable), 0);
    envset(mod->module.exports, "newchart", makebuiltin(newchart), 0);
    envset(mod->module.exports, "newcanvas", makebuiltin(newcanvas), 0);
    envset(mod->module.exports, "newmenubar", makebuiltin(newmenubar), 0);
    envset(mod->module.exports, "newmenu", makebuiltin(newmenu), 0);
    envset(mod->module.exports, "newmenuitem", makebuiltin(newmenuitem), 0);
    envset(mod->module.exports, "newtabs", makebuiltin(newtabs), 0);
    envset(mod->module.exports, "newtabpage", makebuiltin(newtabpage), 0);
    envset(mod->module.exports, "newpanel", makebuiltin(newpanel), 0);
    envset(mod->module.exports, "newspinner", makebuiltin(newspinner), 0);
    envset(mod->module.exports, "newdatepicker", makebuiltin(newdatepicker), 0);
    envset(mod->module.exports, "newcolorpicker", makebuiltin(newcolorpicker), 0);
    envset(mod->module.exports, "newfilepicker", makebuiltin(newfilepicker), 0);
    envset(mod->module.exports, "newtoolbar", makebuiltin(newtoolbar), 0);
    envset(mod->module.exports, "newtoolbutton", makebuiltin(newtoolbutton), 0);
    envset(mod->module.exports, "newstatusbar", makebuiltin(newstatusbar), 0);
    envset(mod->module.exports, "newsplitter", makebuiltin(newsplitter), 0);
    envset(mod->module.exports, "newtree", makebuiltin(newtree), 0);
    envset(mod->module.exports, "newhtmlview", makebuiltin(newhtmlview), 0);
    envset(mod->module.exports, "newmarkdown", makebuiltin(newmarkdown), 0);
    envset(mod->module.exports, "newvideo", makebuiltin(newvideo), 0);
    envset(mod->module.exports, "newaudio", makebuiltin(newaudio), 0);
    envset(mod->module.exports, "newmap", makebuiltin(newmap), 0);
    envset(mod->module.exports, "newpdfviewer", makebuiltin(newpdfviewer), 0);
    envset(mod->module.exports, "newcodeeditor", makebuiltin(newcodeeditor), 0);
    envset(mod->module.exports, "newterminal", makebuiltin(newterminal), 0);
    envset(mod->module.exports, "newnotebook", makebuiltin(newnotebook), 0);
    envset(mod->module.exports, "newdebugger", makebuiltin(newdebugger), 0);
    envset(mod->module.exports, "newlogger", makebuiltin(newlogger), 0);
    envset(mod->module.exports, "newtimer", makebuiltin(newtimer), 0);
    envset(mod->module.exports, "settext", makebuiltin(settext), 0);
    envset(mod->module.exports, "gettext", makebuiltin(gettext), 0);
    envset(mod->module.exports, "setvalue", makebuiltin(setvalue), 0);
    envset(mod->module.exports, "getvalue", makebuiltin(getvalue), 0);
    envset(mod->module.exports, "appenditem", makebuiltin(appenditem), 0);
    envset(mod->module.exports, "clearitems", makebuiltin(clearitems), 0);
    envset(mod->module.exports, "show", makebuiltin(show), 0);
    envset(mod->module.exports, "hide", makebuiltin(hide), 0);
    envset(mod->module.exports, "enable", makebuiltin(enable), 0);
    envset(mod->module.exports, "disable", makebuiltin(disable), 0);
    envset(mod->module.exports, "focus", makebuiltin(focus), 0);
    envset(mod->module.exports, "destroy", makebuiltin(destroy), 0);
    envset(mod->module.exports, "msgbox", makebuiltin(msgbox), 0);
    envset(mod->module.exports, "opendialog", makebuiltin(opendialog), 0);
    envset(mod->module.exports, "savedialog", makebuiltin(savedialog), 0);
    envset(mod->module.exports, "alert", makebuiltin(alert), 0);
    envset(mod->module.exports, "confirm", makebuiltin(confirm), 0);
    envset(mod->module.exports, "notify", makebuiltin(notify), 0);
    envset(mod->module.exports, "newengine3d", makebuiltin(newengine3d), 0);
    envset(mod->module.exports, "render3d", makebuiltin(render3d), 0);
    envset(mod->module.exports, "rotate3d", makebuiltin(rotate3d), 0);
    envset(mod->module.exports, "translate3d", makebuiltin(translate3d), 0);
    envset(mod->module.exports, "scale3d", makebuiltin(scale3d), 0);
    envset(mod->module.exports, "loadmodel", makebuiltin(loadmodel), 0);
    envset(mod->module.exports, "loadtexture", makebuiltin(loadtexture), 0);
    envset(mod->module.exports, "addlight", makebuiltin(addlight), 0);
    envset(mod->module.exports, "setcamera", makebuiltin(setcamera), 0);
    envset(mod->module.exports, "setbackground", makebuiltin(setbackground), 0);
    envset(mod->module.exports, "wireframe", makebuiltin(wireframe), 0);
    envset(mod->module.exports, "setshader", makebuiltin(setshader), 0);
    envset(mod->module.exports, "billboard", makebuiltin(billboard), 0);
    envset(mod->module.exports, "particles", makebuiltin(particles), 0);
    envset(mod->module.exports, "animate3d", makebuiltin(animate3d), 0);
    envset(mod->module.exports, "raycast", makebuiltin(raycast), 0);
    envset(mod->module.exports, "enablecollision", makebuiltin(enablecollision), 0);
    envset(mod->module.exports, "setgravity", makebuiltin(setgravity), 0);
    envset(mod->module.exports, "enablephysics", makebuiltin(enablephysics), 0);
    envset(mod->module.exports, "setjoystick", makebuiltin(setjoystick), 0);
    envset(mod->module.exports, "enablevr", makebuiltin(enablevr), 0);
    envset(env, "graphics", mod, 0);
}
