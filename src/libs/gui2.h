#ifndef gui2_h
#define gui2_h

#include "../core/axton.h"

typedef struct guiwindow {
    void *handle;
    int width;
    int height;
    char *title;
    object *children;
    object *events;
    void *data;
} guiwindow;

typedef struct guibutton {
    void *handle;
    char *text;
    int x;
    int y;
    int width;
    int height;
    object *onclick;
    void *parent;
} guibutton;

typedef struct guilabel {
    void *handle;
    char *text;
    int x;
    int y;
    object *parent;
} guilabel;

typedef struct guitextbox {
    void *handle;
    char *text;
    int x;
    int y;
    int width;
    object *onchange;
    void *parent;
} guitextbox;

typedef struct guicheckbox {
    void *handle;
    char *text;
    int x;
    int y;
    int checked;
    object *oncheck;
    void *parent;
} guicheckbox;

typedef struct guilistbox {
    void *handle;
    object *items;
    int x;
    int y;
    int width;
    int height;
    int selected;
    object *onselect;
    void *parent;
} guilistbox;

typedef struct guimenu {
    void *handle;
    char *name;
    object *items;
    void *parent;
} guimenu;

typedef struct guidialog {
    void *handle;
    char *title;
    char *message;
    object *result;
} guidialog;

object *guiwindow(char *title, int width, int height);
object *guibutton(object *parent, char *text, int x, int y, int width, int height, object *onclick);
object *guilabel(object *parent, char *text, int x, int y);
object *guitextbox(object *parent, int x, int y, int width, object *onchange);
object *guicheckbox(object *parent, char *text, int x, int y, object *oncheck);
object *guilistbox(object *parent, object *items, int x, int y, int width, int height, object *onselect);
object *guimenu(char *name, object *items);
void guiappendmenu(object *window, object *menu);
void guisettext(object *widget, char *text);
char *guigettext(object *widget);
void guisetchecked(object *widget, int checked);
int guigetchecked(object *widget);
object *guigetselected(object *widget);
void guiappenditem(object *listbox, char *item);
void guiclearitems(object *listbox);
object *guiopendialog(char *title);
object *guisavedialog(char *title);
void guimessagebox(char *title, char *message);
void guirun(void);
void guiexit(void);

#endif
