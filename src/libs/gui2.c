#include "gui2.h"

#ifdef _WIN32
#include <windows.h>

static HINSTANCE hinst;
static HWND mainhwnd;
static object *windows = NULL;
static object *handlemap = NULL;

typedef struct {
    HWND hwnd;
    object *widget;
} widgetmap;

static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    object *widget = dictget(handlemap, makeint((long)hwnd));
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_COMMAND) {
        int id = LOWORD(wp);
        if (widget && widget->type == 40) {
            guibutton *btn = (guibutton*)widget->native.data;
            if (btn->onclick) callfunc(btn->onclick, NULL, 0, globalenv);
        }
    }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

static void registerclass(void) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = wndproc;
    wc.hInstance = hinst;
    wc.lpszClassName = "axtonwin";
    RegisterClass(&wc);
}

object *guiwindow(char *title, int width, int height) {
    if (!hinst) hinst = GetModuleHandle(NULL);
    registerclass();
    HWND hwnd = CreateWindow("axtonwin", title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hinst, NULL);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    object *win = makenative(NULL, NULL);
    win->type = 40;
    guiwindow *gw = malloc(sizeof(guiwindow));
    gw->handle = hwnd;
    gw->width = width;
    gw->height = height;
    gw->title = strdup(title);
    gw->children = makelist();
    gw->events = makedict();
    win->native.data = gw;
    if (!windows) windows = makelist();
    listappend(windows, win);
    if (!handlemap) handlemap = makedict();
    dictset(handlemap, makeint((long)hwnd), win);
    return win;
}

object *guibutton(object *parent, char *text, int x, int y, int width, int height, object *onclick) {
    guiwindow *gw = (guiwindow*)parent->native.data;
    HWND hwnd = (HWND)gw->handle;
    HWND btn = CreateWindow("BUTTON", text, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y, width, height, hwnd, (HMENU)1000 + (int)onclick, hinst, NULL);
    object *widget = makenative(NULL, NULL);
    widget->type = 41;
    guibutton *gb = malloc(sizeof(guibutton));
    gb->handle = btn;
    gb->text = strdup(text);
    gb->x = x;
    gb->y = y;
    gb->width = width;
    gb->height = height;
    gb->onclick = onclick;
    gb->parent = parent;
    widget->native.data = gb;
    dictset(handlemap, makeint((long)btn), widget);
    listappend(gw->children, widget);
    return widget;
}

object *guilabel(object *parent, char *text, int x, int y) {
    guiwindow *gw = (guiwindow*)parent->native.data;
    HWND hwnd = (HWND)gw->handle;
    HWND lbl = CreateWindow("STATIC", text, WS_VISIBLE | WS_CHILD,
        x, y, 100, 20, hwnd, NULL, hinst, NULL);
    object *widget = makenative(NULL, NULL);
    widget->type = 42;
    guilabel *gl = malloc(sizeof(guilabel));
    gl->handle = lbl;
    gl->text = strdup(text);
    gl->x = x;
    gl->y = y;
    gl->parent = parent;
    widget->native.data = gl;
    dictset(handlemap, makeint((long)lbl), widget);
    listappend(gw->children, widget);
    return widget;
}

object *guitextbox(object *parent, int x, int y, int width, object *onchange) {
    guiwindow *gw = (guiwindow*)parent->native.data;
    HWND hwnd = (HWND)gw->handle;
    HWND edit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        x, y, width, 20, hwnd, NULL, hinst, NULL);
    object *widget = makenative(NULL, NULL);
    widget->type = 43;
    guitextbox *gt = malloc(sizeof(guitextbox));
    gt->handle = edit;
    gt->text = strdup("");
    gt->x = x;
    gt->y = y;
    gt->width = width;
    gt->onchange = onchange;
    gt->parent = parent;
    widget->native.data = gt;
    dictset(handlemap, makeint((long)edit), widget);
    listappend(gw->children, widget);
    return widget;
}

object *guicheckbox(object *parent, char *text, int x, int y, object *oncheck) {
    guiwindow *gw = (guiwindow*)parent->native.data;
    HWND hwnd = (HWND)gw->handle;
    HWND chk = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        x, y, 100, 20, hwnd, NULL, hinst, NULL);
    object *widget = makenative(NULL, NULL);
    widget->type = 44;
    guicheckbox *gc = malloc(sizeof(guicheckbox));
    gc->handle = chk;
    gc->text = strdup(text);
    gc->x = x;
    gc->y = y;
    gc->checked = 0;
    gc->oncheck = oncheck;
    gc->parent = parent;
    widget->native.data = gc;
    dictset(handlemap, makeint((long)chk), widget);
    listappend(gw->children, widget);
    return widget;
}

object *guilistbox(object *parent, object *items, int x, int y, int width, int height, object *onselect) {
    guiwindow *gw = (guiwindow*)parent->native.data;
    HWND hwnd = (HWND)gw->handle;
    HWND lb = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
        x, y, width, height, hwnd, NULL, hinst, NULL);
    for (int i = 0; i < items->list.count; i++) {
        SendMessage(lb, LB_ADDSTRING, 0, (LPARAM)items->list.items[i]->sval);
    }
    object *widget = makenative(NULL, NULL);
    widget->type = 45;
    guilistbox *glb = malloc(sizeof(guilistbox));
    glb->handle = lb;
    glb->items = items;
    glb->x = x;
    glb->y = y;
    glb->width = width;
    glb->height = height;
    glb->selected = -1;
    glb->onselect = onselect;
    glb->parent = parent;
    widget->native.data = glb;
    dictset(handlemap, makeint((long)lb), widget);
    listappend(gw->children, widget);
    return widget;
}

object *guimenu(char *name, object *items) {
    HMENU hmenu = CreatePopupMenu();
    for (int i = 0; i < items->list.count; i++) {
        char *itemname = items->list.items[i]->sval;
        AppendMenu(hmenu, MF_STRING, 2000 + i, itemname);
    }
    object *menu = makenative(NULL, NULL);
    menu->type = 46;
    guimenu *gm = malloc(sizeof(guimenu));
    gm->handle = hmenu;
    gm->name = strdup(name);
    gm->items = items;
    menu->native.data = gm;
    return menu;
}

void guiappendmenu(object *window, object *menu) {
    guiwindow *gw = (guiwindow*)window->native.data;
    HMENU hmenu = GetMenu((HWND)gw->handle);
    if (!hmenu) {
        hmenu = CreateMenu();
        SetMenu((HWND)gw->handle, hmenu);
    }
    guimenu *gm = (guimenu*)menu->native.data;
    AppendMenu(hmenu, MF_POPUP, (UINT_PTR)gm->handle, gm->name);
    DrawMenuBar((HWND)gw->handle);
}

void guisettext(object *widget, char *text) {
    if (widget->type == 41) {
        guibutton *gb = (guibutton*)widget->native.data;
        SetWindowText((HWND)gb->handle, text);
        free(gb->text);
        gb->text = strdup(text);
    } else if (widget->type == 42) {
        guilabel *gl = (guilabel*)widget->native.data;
        SetWindowText((HWND)gl->handle, text);
        free(gl->text);
        gl->text = strdup(text);
    } else if (widget->type == 43) {
        guitextbox *gt = (guitextbox*)widget->native.data;
        SetWindowText((HWND)gt->handle, text);
        free(gt->text);
        gt->text = strdup(text);
    }
}

char *guigettext(object *widget) {
    if (widget->type == 41) {
        guibutton *gb = (guibutton*)widget->native.data;
        return gb->text;
    } else if (widget->type == 42) {
        guilabel *gl = (guilabel*)widget->native.data;
        return gl->text;
    } else if (widget->type == 43) {
        guitextbox *gt = (guitextbox*)widget->native.data;
        char buf[256];
        GetWindowText((HWND)gt->handle, buf, sizeof(buf));
        return strdup(buf);
    }
    return NULL;
}

void guisetchecked(object *widget, int checked) {
    if (widget->type == 44) {
        guicheckbox *gc = (guicheckbox*)widget->native.data;
        SendMessage((HWND)gc->handle, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
        gc->checked = checked;
    }
}

int guigetchecked(object *widget) {
    if (widget->type == 44) {
        guicheckbox *gc = (guicheckbox*)widget->native.data;
        return SendMessage((HWND)gc->handle, BM_GETCHECK, 0, 0) == BST_CHECKED;
    }
    return 0;
}

object *guigetselected(object *widget) {
    if (widget->type == 45) {
        guilistbox *glb = (guilistbox*)widget->native.data;
        int idx = SendMessage((HWND)glb->handle, LB_GETCURSEL, 0, 0);
        if (idx >= 0 && idx < glb->items->list.count) {
            return glb->items->list.items[idx];
        }
    }
    return makenone();
}

void guiappenditem(object *listbox, char *item) {
    if (listbox->type == 45) {
        guilistbox *glb = (guilistbox*)listbox->native.data;
        SendMessage((HWND)glb->handle, LB_ADDSTRING, 0, (LPARAM)item);
        listappend(glb->items, makestring(item));
    }
}

void guiclearitems(object *listbox) {
    if (listbox->type == 45) {
        guilistbox *glb = (guilistbox*)listbox->native.data;
        SendMessage((HWND)glb->handle, LB_RESETCONTENT, 0, 0);
        glb->items->list.count = 0;
    }
}

object *guiopendialog(char *title) {
    OPENFILENAME ofn = {0};
    char file[256] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = mainhwnd;
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn)) {
        return makestring(file);
    }
    return makenone();
}

object *guisavedialog(char *title) {
    OPENFILENAME ofn = {0};
    char file[256] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = mainhwnd;
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (GetSaveFileName(&ofn)) {
        return makestring(file);
    }
    return makenone();
}

void guimessagebox(char *title, char *message) {
    MessageBox(mainhwnd, message, title, MB_OK);
}

void guirun(void) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void guiexit(void) {
    PostQuitMessage(0);
}

#elif defined(__linux__) && !defined(__ANDROID__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

static Display *disp;
static int screen;
static object *windows = NULL;
static object *handlemap = NULL;

typedef struct {
    Window win;
    object *widget;
} widgetmap;

object *guiwindow(char *title, int width, int height) {
    if (!disp) {
        disp = XOpenDisplay(NULL);
        screen = DefaultScreen(disp);
    }
    Window win = XCreateSimpleWindow(disp, RootWindow(disp, screen),
        0, 0, width, height, 1, BlackPixel(disp, screen), WhitePixel(disp, screen));
    XStoreName(disp, win, title);
    XSelectInput(disp, win, ExposureMask | ButtonPressMask | KeyPressMask);
    XMapWindow(disp, win);
    object *obj = makenative(NULL, NULL);
    obj->type = 40;
    guiwindow *gw = malloc(sizeof(guiwindow));
    gw->handle = (void*)(long)win;
    gw->width = width;
    gw->height = height;
    gw->title = strdup(title);
    gw->children = makelist();
    gw->events = makedict();
    obj->native.data = gw;
    if (!windows) windows = makelist();
    listappend(windows, obj);
    if (!handlemap) handlemap = makedict();
    dictset(handlemap, makeint(win), obj);
    return obj;
}

static void handleevents(void) {
    XEvent ev;
    while (XPending(disp)) {
        XNextEvent(disp, &ev);
        if (ev.type == ButtonPress) {
            object *winobj = dictget(handlemap, makeint(ev.xbutton.window));
            if (winobj && winobj->type == 40) {
                guiwindow *gw = (guiwindow*)winobj->native.data;
                object *onclick = dictget(gw->events, makestring("click"));
                if (onclick) callfunc(onclick, NULL, 0, globalenv);
            }
        }
        if (ev.type == KeyPress) {
            object *winobj = dictget(handlemap, makeint(ev.xkey.window));
            if (winobj && winobj->type == 40) {
                guiwindow *gw = (guiwindow*)winobj->native.data;
                object *onkey = dictget(gw->events, makestring("key"));
                if (onkey) callfunc(onkey, NULL, 0, globalenv);
            }
        }
        if (ev.type == Expose) {
            object *winobj = dictget(handlemap, makeint(ev.xexpose.window));
            if (winobj && winobj->type == 40) {
                guiwindow *gw = (guiwindow*)winobj->native.data;
                object *ondraw = dictget(gw->events, makestring("draw"));
                if (ondraw) callfunc(ondraw, NULL, 0, globalenv);
            }
        }
    }
}

void guirun(void) {
    while (1) {
        handleevents();
        usleep(16666);
    }
}

void guiexit(void) {
    exit(0);
}

#else

object *guiwindow(char *title, int width, int height) {
    platformlog("GUI not supported on this platform\n");
    return makenone();
}

void guirun(void) {}
void guiexit(void) {}

#endif

void registergui2lib(environment *env) {
    object *guimod = makemodule("gui", NULL);
    envset(guimod->module.exports, "window", makebuiltin(guiwindow), 0);
    envset(guimod->module.exports, "button", makebuiltin(guibutton), 0);
    envset(guimod->module.exports, "label", makebuiltin(guilabel), 0);
    envset(guimod->module.exports, "textbox", makebuiltin(guitextbox), 0);
    envset(guimod->module.exports, "checkbox", makebuiltin(guicheckbox), 0);
    envset(guimod->module.exports, "listbox", makebuiltin(guilistbox), 0);
    envset(guimod->module.exports, "menu", makebuiltin(guimenu), 0);
    envset(guimod->module.exports, "appendmenu", makebuiltin(guiappendmenu), 0);
    envset(guimod->module.exports, "settext", makebuiltin(guisettext), 0);
    envset(guimod->module.exports, "gettext", makebuiltin(guigettext), 0);
    envset(guimod->module.exports, "setchecked", makebuiltin(guisetchecked), 0);
    envset(guimod->module.exports, "getchecked", makebuiltin(guigetchecked), 0);
    envset(guimod->module.exports, "getselected", makebuiltin(guigetselected), 0);
    envset(guimod->module.exports, "appenditem", makebuiltin(guiappenditem), 0);
    envset(guimod->module.exports, "clearitems", makebuiltin(guiclearitems), 0);
    envset(guimod->module.exports, "opendialog", makebuiltin(guiopendialog), 0);
    envset(guimod->module.exports, "savedialog", makebuiltin(guisavedialog), 0);
    envset(guimod->module.exports, "messagebox", makebuiltin(guimessagebox), 0);
    envset(guimod->module.exports, "run", makebuiltin(guirun), 0);
    envset(guimod->module.exports, "exit", makebuiltin(guiexit), 0);
    envset(env, "gui", guimod, 0);
}
