#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../core/axton.h"

typedef struct app app;
typedef struct window window;
typedef struct widget widget;
typedef struct canvas canvas;
typedef struct engine3d engine3d;

app* createapp(void);
void runapp(app* a);
void stopapp(app* a);

window* newwindow(app* a, char* title, int w, int h);
widget* newbutton(window* w, char* text, int x, int y, int wd, int ht, object* onclick);
widget* newlabel(window* w, char* text, int x, int y);
widget* newtextbox(window* w, int x, int y, int wd, object* onchange);
widget* newcheckbox(window* w, char* text, int x, int y, object* oncheck);
widget* newradio(window* w, char* text, int x, int y, object* onselect);
widget* newlistbox(window* w, int x, int y, int wd, int ht, object* onselect);
widget* newdropdown(window* w, object* items, int x, int y, int wd, object* onselect);
widget* newslider(window* w, int x, int y, int wd, int min, int max, object* onchange);
widget* newprogress(window* w, int x, int y, int wd, int ht);
widget* newimage(window* w, char* path, int x, int y, int wd, int ht);
widget* newwebview(window* w, char* url, int x, int y, int wd, int ht);
widget* newtable(window* w, object* cols, object* rows, int x, int y, int wd, int ht);
widget* newchart(window* w, object* data, int x, int y, int wd, int ht);
widget* newcanvas(window* w, int wd, int ht, object* ondraw);
widget* newmenubar(window* w);
widget* newmenu(widget* bar, char* name);
widget* newmenuitem(widget* menu, char* name, object* onclick);
widget* newtabs(window* w, int x, int y, int wd, int ht);
widget* newtabpage(widget* tabs, char* title);
widget* newpanel(window* w, int x, int y, int wd, int ht);
widget* newspinner(window* w, int x, int y, int wd, object* onchange);
widget* newdatepicker(window* w, int x, int y, int wd, object* onchange);
widget* newcolorpicker(window* w, int x, int y, object* onchange);
widget* newfilepicker(window* w, int x, int y, int wd, object* onselect);
widget* newtoolbar(window* w);
widget* newtoolbutton(widget* bar, char* icon, object* onclick);
widget* newstatusbar(window* w);
widget* newsplitter(window* w, int x, int y, int wd, int ht, int vertical);
widget* newtree(window* w, object* nodes, int x, int y, int wd, int ht, object* onselect);
widget* newhtmlview(window* w, char* html, int x, int y, int wd, int ht);
widget* newmarkdown(window* w, char* text, int x, int y, int wd, int ht);
widget* newvideo(window* w, char* path, int x, int y, int wd, int ht);
widget* newaudio(window* w, char* path, int x, int y, int wd, int ht);
widget* newmap(window* w, double lat, double lon, int zoom, int x, int y, int wd, int ht);
widget* newpdfviewer(window* w, char* path, int x, int y, int wd, int ht);
widget* newcodeeditor(window* w, char* code, int x, int y, int wd, int ht, object* onchange);
widget* newterminal(window* w, int x, int y, int wd, int ht);
widget* newnotebook(window* w, int x, int y, int wd, int ht);
widget* newdebugger(window* w, int x, int y, int wd, int ht);
widget* newlogger(window* w, int x, int y, int wd, int ht);
widget* newtimer(window* w, int ms, object* ontimeout);

void settext(widget* w, char* text);
char* gettext(widget* w);
void setvalue(widget* w, double val);
double getvalue(widget* w);
void appenditem(widget* w, char* item);
void clearitems(widget* w);
void show(widget* w);
void hide(widget* w);
void enable(widget* w);
void disable(widget* w);
void focus(widget* w);
void destroy(widget* w);

void msgbox(char* title, char* msg);
char* opendialog(char* title, char* filter);
char* savedialog(char* title, char* filter);
void alert(char* msg);
int confirm(char* msg);
void notify(char* title, char* msg, int ms);

engine3d* newengine3d(int w, int h);
void render3d(engine3d* e);
void rotate3d(engine3d* e, float x, float y, float z);
void translate3d(engine3d* e, float x, float y, float z);
void scale3d(engine3d* e, float x, float y, float z);
void loadmodel(engine3d* e, char* path);
void loadtexture(engine3d* e, char* path);
void addlight(engine3d* e, float x, float y, float z, float r, float g, float b);
void setcamera(engine3d* e, float x, float y, float z, float tx, float ty, float tz);
void setbackground(engine3d* e, float r, float g, float b);
void wireframe(engine3d* e, int on);
void setshader(engine3d* e, char* vert, char* frag);
void billboard(engine3d* e, char* text, float x, float y, float z);
void particles(engine3d* e, int count, float x, float y, float z);
void animate3d(engine3d* e, float speed);
void raycast(engine3d* e, int x, int y, object* callback);
void enablecollision(engine3d* e, int on);
void setgravity(engine3d* e, float force);
void enablephysics(engine3d* e, int on);
void setjoystick(engine3d* e, int idx, object* callback);
void enablevr(engine3d* e, int on);

#endif
