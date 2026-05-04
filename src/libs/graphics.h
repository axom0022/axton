#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../core/axton.h"

typedef struct matrix4 {
    float m[16];
} matrix4;

typedef struct vector3 {
    float x,y,z;
} vector3;

typedef struct vector2 {
    float x,y;
} vector2;

typedef struct color {
    float r,g,b,a;
} color;

typedef struct mesh {
    float *vertices;
    int *indices;
    int vcount;
    int icount;
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
} mesh;

typedef struct texture {
    unsigned int id;
    int width;
    int height;
} texture;

typedef struct material {
    color diffuse;
    color specular;
    float shininess;
    texture *diffusetex;
} material;

typedef struct model {
    mesh *meshes;
    material *materials;
    int meshcount;
    matrix4 transform;
} model;

typedef struct camera {
    vector3 pos;
    vector3 target;
    vector3 up;
    float fov;
    float aspect;
    float near;
    float far;
    matrix4 view;
    matrix4 proj;
} camera;

typedef struct light {
    vector3 pos;
    color diffuse;
    color specular;
} light;

typedef struct renderer {
    unsigned int program;
    camera *cam;
    light *lights;
    int lightcount;
} renderer;

object *graphicsinit(int width, int height, char *title);
object *graphicsclear(color c);
object *graphicsbegin(void);
object *graphicsend(void);
object *graphicsloadmesh(char *path);
object *graphicsloadtexture(char *path);
object *graphicsdrawmodel(object *model, matrix4 transform);
object *graphicscamera(vector3 pos, vector3 target);
object *graphicssetlight(vector3 pos, color diff, color spec);
object *graphicsrun(void);
object *graphicsclose(void);

// widgets
object *widgetcreatewindow(object *parent, int x, int y, int w, int h, char *title);
object *widgetcreatebutton(object *parent, int x, int y, int w, int h, char *text, object *onclick);
object *widgetcreatelabel(object *parent, int x, int y, char *text);
object *widgetcreatetextbox(object *parent, int x, int y, int w, char *text, object *onchange);
object *widgetcreatecheckbox(object *parent, int x, int y, char *text, object *oncheck);
object *widgetcreateradiobutton(object *parent, int x, int y, char *text, object *onselect);
object *widgetcreatecombobox(object *parent, int x, int y, int w, object *items, object *onselect);
object *widgetcreatelistbox(object *parent, int x, int y, int w, int h, object *items, object *onselect);
object *widgetcreatetable(object *parent, int x, int y, int w, int h, int rows, int cols, object *ondata);
object *widgetcreatetree(object *parent, int x, int y, int w, int h, object *onnode);
object *widgetcreatetabcontrol(object *parent, int x, int y, int w, int h, object *tabs);
object *widgetcreateprogressbar(object *parent, int x, int y, int w, int h, int min, int max, int value);
object *widgetcreateslider(object *parent, int x, int y, int w, int h, int min, int max, int value, object *onchange);
object *widgetcreatespinbox(object *parent, int x, int y, int w, int min, int max, int value, object *onchange);
object *widgetcreatedatepicker(object *parent, int x, int y, int w, object *ondate);
object *widgetcreatecolorpicker(object *parent, int x, int y, int w, color col, object *oncolor);
object *widgetcreateimage(object *parent, int x, int y, int w, int h, char *path);
object *widgetcreatecanvas(object *parent, int x, int y, int w, int h, object *ondraw);
object *widgetcreatewebview(object *parent, int x, int y, int w, int h, char *url);
object *widgetcreatechart(object *parent, int x, int y, int w, int h, object *data);
object *widgetcreategauge(object *parent, int x, int y, int w, int h, int min, int max, int value);
object *widgetcreatecalendar(object *parent, int x, int y, int w, int h, object *onchange);
object *widgetcreatetoolbar(object *parent, int x, int y, int w, int h);
object *widgetcreatemenu(object *parent, object *items);
object *widgetcreatestatusbar(object *parent, char *text);
object *widgetshownotification(char *title, char *message);
object *widgetfiledialog(int open, char *title);
object *widgetmessagebox(char *title, char *message, int buttons);
object *widgetinputdialog(char *title, char *prompt);
object *widgetcolordialog(color *col);
object *widgetfontdialog(char **fontname, int *size);
object *widgetprintdialog(void);
object *widgetwizardcreate(object *parent, char *title, object *pages);
object *widgetdockpanelcreate(object *parent, int x, int y, int w, int h);
object *widgetsplitcontainer(object *parent, int x, int y, int w, int h, int orientation);
object *widgetpanel(object *parent, int x, int y, int w, int h);
object *widgetgroupbox(object *parent, int x, int y, int w, int h, char *title);
object *widgetscrollview(object *parent, int x, int y, int w, int h);
object *widgettreeview(object *parent, int x, int y, int w, int h);
object *widgetlistviewadvanced(object *parent, int x, int y, int w, int h);
object *widgetgridview(object *parent, int x, int y, int w, int h);
object *widgettileview(object *parent, int x, int y, int w, int h);
object *widgetcarousel(object *parent, int x, int y, int w, int h, object *pages);
object *widgetaccordion(object *parent, int x, int y, int w, int h, object *panels);
object *widgetratingbar(object *parent, int x, int y, int max);
object *widgetswitch(object *parent, int x, int y, int checked, object *onchange);
object *widgettogglebutton(object *parent, int x, int y, int w, int h, char *text, int toggled, object *ontoggle);
object *widgetsegmentedcontrol(object *parent, int x, int y, int w, int h, object *segments, object *onselect);
object *widgetstepper(object *parent, int x, int y, int w, int h, int min, int max, int step, object *onchange);
object *widgetknob(object *parent, int x, int y, int size, int min, int max, int value, object *onchange);
object *widgetterminal(object *parent, int x, int y, int w, int h, object *oncommand);
object *widgetcodeeditor(object *parent, int x, int y, int w, int h, char *language, char *code, object *onchange);
object *widgetmarkdownview(object *parent, int x, int y, int w, int h, char *markdown);
object *widgetvideoplayer(object *parent, int x, int y, int w, int h, char *path);
object *widgetaudioplayer(object *parent, char *path);
object *widgetwebbrowser(object *parent, int x, int y, int w, int h, char *url);
object *widgetmapview(object *parent, int x, int y, int w, int h, double lat, double lon);
object *widget3dview(object *parent, int x, int y, int w, int h);
object *widgetparticlesystem(object *parent, int x, int y, int w, int h);
object *widgetanimation(object *target, char *property, int duration, object *values);
object *widgettransition(object *from, object *to, int duration, object *oncomplete);
object *widgetgesturerecognizer(object *target, char *type, object *onrecognize);
object *widgetdragdrop(object *source, object *target, object *ondrop);
object *widgetclipboard(char *text);
object *widgetundoredo(object *manager, object *command);
object *widgettheme(char *name, object *colors);
object *widgetstylesheet(char *css);
object *widgetlayoutgrid(object *container, int cols, int rows);
object *widgetlayoutflex(object *container, char *direction);
object *widgetlayoutabsolute(object *container);
object *widgetdatabinding(object *source, char *sourceprop, object *target, char *targetprop);
object *widgetvalidation(object *input, char *rule, char *errormsg);
object *widgetcommandbinding(object *button, char *command, object *execute);
object *widgetmultiwindowcreate(char *title, int w, int h);
object *widgettrayicon(char *iconpath, object *onclick);
object *widgetglobalshortcut(char *key, object *onpress);
object *widgetpowermanagementprevent(void);
object *widgetscreencapture(int x, int y, int w, int h);
object *widgetocr(char *imagepath);
object *widgetspeechrecognize(int duration);
object *widgetspeechspeak(char *text);
object *widgetcameracapture(int device, int width, int height);
object *widgetmicrophonerecord(int seconds);
object *widgetserialportopen(char *port, int baud);
object *widgetserialportwrite(char *data);
object *widgetserialportread(void);
object *widgetbluetoothscan(void);
object *widgetbluetoothconnect(char *addr);
object *widgetwifiscan(void);
object *widgetwifisend(char *data);
object *widgetsystemnotification(char *title, char *body);
object *widgetprintingprint(char *html);
object *widgetpdfgenerate(char *html, char *path);
object *widgetexportexcel(object *data, char *path);

#endif
