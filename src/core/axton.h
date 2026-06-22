#ifndef axton_h
#define axton_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define mkdir(x,y) _mkdir(x)
#define snprintf _snprintf
#define PATHSEP '\\'
#else
#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <signal.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netdb.h>
#define PATHSEP '/'
#endif

#ifdef __linux__
#include <pcap.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#include <jpeglib.h>
#include <png.h>
#include <tesseract/capi.h>

typedef enum {
    TOKEOF, TOKIDENT, TOKNUMBER, TOKSTRING, TOKINDENT, TOKDEDENT, TOKNEWLINE,
    TOKLET, TOKCONST, TOKFN, TOKIF, TOKELSE, TOKELIF,
    TOKFOR, TOKIN, TOKWHILE, TOKBREAK, TOKNEXT, TOKRETURN,
    TOKNONE, TOKTRUE, TOKFALSE, TOKAND, TOKOR, TOKNOT,
    TOKPLUS, TOKMINUS, TOKSTAR, TOKSLASH, TOKPERCENT, TOKPOWER,
    TOKEQ, TOKEQEQ, TOKNE, TOKLT, TOKGT, TOKLE, TOKGE,
    TOKLPAREN, TOKRPAREN, TOKLBRACKET, TOKRBRACKET,
    TOKLBRACE, TOKRBRACE, TOKCOMMA, TOKDOT, TOKCOLON,
    TOKCOLONEQ, TOKCOLONCOLON, TOKARROW, TOKUNDERSCORE,
    TOKTRY, TOKCATCH, TOKFINALLY, TOKTHROW, TOKCLASS, TOKIMPORT,
    TOKASYNC, TOKAWAIT, TOKYIELD, TOKWITH, TOKAS, TOKGLOBAL,
    TOKNONLOCAL, TOKASSERT, TOKDECORATOR, TOKMATCH, TOKCASE,
    TOKPIPE, TOKTYPEHINT, TOKENUM, TOKDATACLASS,
    TOKUNION, TOKOPTIONAL, TOKASYNCIO
} toktype;

typedef struct token {
    toktype type;
    char *text;
    int line;
    int col;
} token;

typedef struct node {
    int line;
} node;

typedef struct expr {
    struct node node;
    void *(*eval)(struct expr*, void*);
    char *typehint;
} expr;

typedef struct stmt {
    struct node node;
    void *(*exec)(struct stmt*, void*);
} stmt;

typedef struct object {
    struct object *next;
    struct object *prev;
    int marked;
    int refcount;
    int type;
    union {
        long ival;
        double fval;
        char *sval;
        int bval;
        struct {
            struct object **items;
            int count;
            int cap;
        } list;
        struct {
            char **keys;
            struct object **keyvals;
            struct object **vals;
            int count;
        } dict;
        struct {
            char **params;
            char **typehints;
            int pcount;
            struct stmt **body;
            int bcount;
            void *closure;
            char *name;
            int isasync;
            int isgenerator;
            struct object *decorators;
        } func;
        struct {
            void *(*fn)(struct object**, int, void*);
        } builtin;
        struct {
            char *name;
            void *attrs;
            struct object *bases;
        } klass;
        struct {
            struct object *klass;
            void *attrs;
        } instance;
        struct {
            long start;
            long stop;
            long step;
        } range;
        struct {
            unsigned char *code;
            int size;
            void *entry;
        } bytecode;
        struct {
            char *name;
            char *version;
            void *exports;
        } module;
        struct {
            void *handle;
            void *data;
        } native;
        struct {
            float *data;
            int rows;
            int cols;
            int *shape;
            int ndim;
        } tensor;
        struct {
            struct object *func;
            int state;
            void *frame;
            struct object *value;
        } generator;
        struct {
            struct object *func;
            int state;
            void *frame;
        } coroutine;
        struct {
            void *display;
            void *window;
            void *gc;
            int w,h;
        } guiwin;
        struct {
            void *display;
            void *window;
            void *glc;
        } glwin;
        struct {
            void *data;
            int w,h;
        } canvas;
        struct {
            char *name;
            struct object *fields;
            int issimple;
        } dataclass;
        struct {
            char *name;
            int value;
        } enumval;
        struct {
            struct object *left;
            struct object *right;
        } uniontype;
        struct {
            struct object *type;
        } optionaltype;
        struct {
            void *ctx;
            int fd;
        } sslctx;
        struct {
            void *handle;
            int type;
        } cloud;
        struct {
            ffi_cif cif;
            ffi_type **argtypes;
            void *function;
            int argcount;
            int rettype;
            int *argtypes_code;
        } ffiwrap;
        struct {
            void *handle;
        } libhandle;
        struct {
            void *ptr;
        } pointer;
        struct {
            void *data;
            int size;
        } structobj;
        struct {
            object *fn;
        } callback;
        struct {
            unsigned int program;
            unsigned int vao;
            unsigned int vbo;
            unsigned int ebo;
            int vertexcount;
            int indexcount;
        } mesh;
        struct {
            unsigned int program;
            struct object *textures;
        } material;
        struct {
            float pos[3];
            float rot[3];
            float scale[3];
        } transform;
        struct {
            object *mesh;
            object *material;
            object *transform;
        } renderable;
        struct {
            unsigned int framebuffer;
            unsigned int depthbuffer;
            unsigned int texture;
            int w,h;
        } rendertarget;
        struct {
            void *memory;
            int size;
            int used;
        } memblock;
        struct {
            int (*check)(void*);
        } typeguard;
        struct {
            object *rules;
        } ratelimiter;
        struct {
            object *patterns;
        } validator;
        struct {
            unsigned char *data;
            int size;
        } hashcrack;
        struct {
            object *targets;
            int state;
        } passwordbrute;
        struct {
            int fd;
            object *clients;
        } mitmproxy;
        struct {
            int fd;
            char *interface;
        } arpspoof;
        struct {
            int fd;
            char *host;
            int port;
            object *credentials;
        } sshbrute;
        struct {
            object *session;
        } scraper;
        struct {
            object *sources;
        } osint;
        struct {
            void *engine;
        } vision;
        struct {
            void *tess;
        } ocr;
    };
} object;

typedef struct environment {
    char **names;
    object **values;
    int *isconst;
    char **typehints;
    int count;
    int cap;
    struct environment *parent;
    struct environment *globals;
} environment;

typedef struct {
    jmp_buf jump;
    object *result;
    struct frame *prev;
    struct environment *env;
    struct object *generator;
} frame;

typedef struct {
    void (*log)(const char*);
    double (*time)(void);
    void (*sleep)(double);
    char *(*readfile)(const char*);
    int (*writefile)(const char*, const char*);
    void *(*loadlib)(const char*);
    int (*socket)(void);
    int (*bind)(int, int);
    int (*listen)(int, int);
    int (*accept)(int);
    int (*send)(int, const char*, int);
    int (*recv)(int, char*, int);
    void *(*createwindow)(int, int, const char*);
    void (*destroywindow)(void*);
    void (*mainloop)(void);
    void (*postquit)(void);
    char *(*getenv)(const char*);
    int (*setenv)(const char*, const char*);
    int (*kill)(int, int);
    int (*getpid)(void);
    void *(*opendir)(const char*);
    char *(*readdir)(void*);
    void (*closedir)(void*);
    int (*chmod)(const char*, int);
    int (*chown)(const char*, int, int);
    void *(*allocate)(int);
    void (*deallocate)(void*);
    void *(*reallocate)(void*, int);
    int (*getpagesize)(void);
    int (*pcapopen)(const char*, char*);
    int (*pcapnext)(int, unsigned char**, int*);
    int (*pcapinject)(int, unsigned char*, int);
    void (*pcapclose)(int);
    int (*bluetoothopen)(void);
    int (*bluetoothscan)(int, object*);
    void (*bluetoothclose)(int);
} platformapi;

extern environment *globalenv;
extern frame *currentframe;
extern int tcount;
extern platformapi platform;

void gcinit(void);
void gcaddroot(object *obj);
void gcrun(void);
object *gcalloc(int size);

object *makeint(long v);
object *makefloat(double v);
object *makestring(char *v);
object *makebool(int v);
object *makenone(void);
object *makelist(void);
object *makedict(void);
object *makerange(long start, long stop, long step);
object *makefunc(char **params, char **typehints, int pcount, stmt **body, int bcount, environment *closure, char *name, int isasync, int isgenerator);
object *makebuiltin(void *(*fn)(object**, int, environment*));
object *makeclass(char *name, environment *attrs, object *bases);
object *makeinstance(object *klass, object **args, int argc);
object *makemodule(char *name, void *handle);
object *makenative(void *handle, void *data);
object *makegenerator(object *func, environment *env);
object *makecoroutine(object *func, environment *env);
object *makeguiwin(void *dpy, void *win, int w, int h);
object *makeglwin(void *dpy, void *win, void *glc);
object *makedataclass(char *name, object *fields);
object *makeenum(char *name, object *values);
object *makeunion(object *left, object *right);
object *makeoptional(object *type);
object *makemesh(unsigned int prog, unsigned int vao, unsigned int vbo, unsigned int ebo, int vc, int ic);
object *makematerial(unsigned int prog, object *textures);
object *maketransform(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
object *makerenderable(object *mesh, object *mat, object *trans);
object *makerendertarget(int w, int h);
object *makememoryblock(int size);
object *maketypeguard(int (*check)(void*));
object *makeratelimiter(object *rules);
object *makevalidator(object *patterns);
object *makehashcrack(unsigned char *data, int size);
object *makepasswordbrute(object *targets);
object *makemitmproxy(int fd);
object *makearpspoof(int fd, char *iface);
object *makesshbrute(int fd, char *host, int port, object *creds);
object *makescraper(object *session);
object *makeosint(object *sources);
object *makevision(void *engine);
object *makeocr(void *tess);

void listappend(object *list, object *item);
object *listpop(object *list, int index);
void listinsert(object *list, int index, object *item);
void dictset(object *dict, object *key, object *val);
object *dictget(object *dict, object *key);
int dicthas(object *dict, object *key);
object *dictkeys(object *dict);
object *dictvalues(object *dict);
object *dictitems(object *dict);

int istruthy(object *v);
int valuesequal(object *a, object *b);
object *addvalues(object *a, object *b);
object *subvalues(object *a, object *b);
object *mulvalues(object *a, object *b);
object *divvalues(object *a, object *b);
int lessthan(object *a, object *b);
int greaterthan(object *a, object *b);

void throwexception(char *msg);
void throwexceptiontype(char *type, char *msg);
object *catchexception(void);
void initexceptions(environment *env);

token *tokenize(char *input);
stmt *parsetokens(token *tokens, int count);
object *evalprogram(stmt *program, environment *env);
object *callfunc(object *fn, object **args, int argc, environment *env);
void registerbuiltins(environment *env);
void registerstdlib(environment *env);
void registeralllibs(environment *env);

void replstart(void);
void platforminit(void);
void platformlog(const char *msg);
void platformsleep(double seconds);
double platformtime(void);
char *platformreadfile(const char *path);
int platformwritefile(const char *path, const char *content);
void *platformloadlib(const char *path);
int platformsocket(void);
int platformbind(int fd, int port);
int platformlisten(int fd, int backlog);
int platformaccept(int fd);
int platformsend(int fd, const char *data, int len);
int platformrecv(int fd, char *buf, int len);
char *platformgetenv(const char *name);
int platformsetenv(const char *name, const char *value);
int platformkill(int pid, int sig);
int platformgetpid(void);
void *platformopendir(const char *path);
char *platformreaddir(void *dir);
void platformclosedir(void *dir);
int platformchmod(const char *path, int mode);
int platformchown(const char *path, int uid, int gid);
void *platformallocate(int size);
void platformdeallocate(void *ptr);
void *platformreallocate(void *ptr, int size);
int platformgetpagesize(void);
int platformpcapopen(const char *iface, char *err);
int platformpcapnext(int handle, unsigned char **data, int *len);
int platformpcapinject(int handle, unsigned char *packet, int len);
void platformpcapclose(int handle);
int platformbluetoothopen(void);
int platformbluetoothscan(int handle, object *devices);
void platformbluetoothclose(int handle);

void registerhttplib(environment *env);
void registerwebsocketlib(environment *env);
void registertunnellib(environment *env);
void registerguilib(environment *env);
void registerwebviewlib(environment *env);
void registerdblib(environment *env);
void registerpathlib(environment *env);
void registerasynclib(environment *env);
void registermetaprogramlib(environment *env);
void registergeneratorlib(environment *env);
void registercontextlib(environment *env);
void registerpipetransformlib(environment *env);
void registerpatternlib(environment *env);
void registerjwtlib(environment *env);
void registerauthlib(environment *env);
void registerencryptionlib(environment *env);
void registerenvlib(environment *env);
void registersecretlib(environment *env);
void registerauditlib(environment *env);
void registercolorlib(environment *env);
void registerdebuglib(environment *env);
void registerobfuscatelib(environment *env);
void registerailib(environment *env);
void registerextensionlib(environment *env);
void registerwebrtclib(environment *env);
void registerwebgpulib(environment *env);
void registercloudlib(environment *env);
void registercontainerlib(environment *env);
void registerenumlib(environment *env);
void registerdataclasslib(environment *env);
void registerunionlib(environment *env);
void registeroptionallib(environment *env);
void registergraphqllib(environment *env);
void registerunicodelib(environment *env);
void register3dlib(environment *env);
void registermemorylib(environment *env);
void registerweblib(environment *env);
void registerpackagelib(environment *env);
void registersecuritylib(environment *env);
void registerscriptlib(environment *env);
void registerosintlib(environment *env);
void registervisionlib(environment *env);
void registerocrlib(environment *env);

#endif
