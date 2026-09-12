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

#include <ffi.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>

#include <imgui.h>

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
    char *file;
} node;

typedef enum {
    EXPR_BINARY, EXPR_UNARY, EXPR_IDENT, EXPR_NUMBER, EXPR_STRING,
    EXPR_BOOL, EXPR_NONE, EXPR_CALL, EXPR_INDEX, EXPR_ATTR,
    EXPR_LIST, EXPR_DICT
} exprtype;

typedef enum {
    STMT_LET, STMT_RETURN, STMT_IF, STMT_WHILE, STMT_FOR,
    STMT_BREAK, STMT_NEXT, STMT_FN, STMT_CLASS, STMT_EXPR,
    STMT_TRY, STMT_THROW
} stmttype;

typedef struct expr {
    struct node node;
    exprtype type;
    void *(*eval)(struct expr*, void*);
    char *typehint;
} expr;

typedef struct stmt {
    struct node node;
    stmttype type;
    void *(*exec)(struct stmt*, void*);
} stmt;

typedef struct object {
    struct object *next;
    int marked;
    int refcount;
    int type;
    char *file;
    int line;
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
            struct object *fn;
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
            struct object *mesh;
            struct object *material;
            struct object *transform;
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
            struct object *rules;
        } ratelimiter;
        struct {
            struct object *patterns;
        } validator;
        struct {
            unsigned char *data;
            int size;
        } hashcrack;
        struct {
            struct object *targets;
            int state;
        } passwordbrute;
        struct {
            int fd;
            struct object *clients;
        } mitmproxy;
        struct {
            int fd;
            char *interface;
        } arpspoof;
        struct {
            int fd;
            char *host;
            int port;
            struct object *credentials;
        } sshbrute;
        struct {
            struct object *session;
        } scraper;
        struct {
            struct object *sources;
        } osint;
        struct {
            void *engine;
        } vision;
        struct {
            void *tess;
        } ocr;
        struct {
            long handle;
            int pid;
        } processhandle;
        struct {
            void *peerconnection;
            void *datachannel;
        } webrtc;
        struct {
            int fd;
            char *url;
            int connected;
        } wsclient;
        struct {
            SSL *ssl;
            SSL_CTX *ctx;
            int fd;
        } sslconn;
        struct {
            void *capture;
            int enabled;
        } renderdoc;
        struct {
            void *context;
            void *window;
            int active;
        } imgui;
        struct {
            void *data;
            int vertexcount;
            int indexcount;
            float *vertices;
            int *indices;
        } gltf;
        struct {
            float *positions;
            int count;
            int color[3];
        } debugphys;
        struct {
            void *dev;
            void *context;
            int channels;
            float volume;
        } audiomix;
        struct {
            void *stream;
            void *decoder;
            int w,h;
            float fps;
        } video;
        struct {
            int codepoint;
            char utf8[8];
        } unicode;
        struct {
            void *server;
            int port;
            int running;
        } lsp;
        struct {
            struct object *rules;
            int count;
        } linter;
        struct {
            float *data;
            int size;
        } vector;
        struct {
            pthread_t thread;
            struct object *func;
            struct object *args;
            int running;
        } threadobj;
        struct {
            FILE *file;
            int fd;
            int mode;
        } fileobj;
    };
} object;

typedef struct envslot {
    char *name;
    object *value;
    unsigned int hash;
    int isconst;
    int used;
} envslot;

typedef struct environment {
    envslot *slots;
    int count;
    int cap;
    struct environment *parent;
    struct environment *globals;
    struct environment *next;
    int marked;
} environment;

typedef struct {
    jmp_buf jump;
    object *result;
    struct frame *prev;
    struct environment *env;
    struct object *generator;
    char *file;
    int line;
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
    int (*processopen)(int, int);
    int (*processread)(int, long, unsigned char*, int);
    int (*processwrite)(int, long, unsigned char*, int);
    void (*processclose)(int);
    int (*processfind)(const char*);
    int (*sslinit)(void);
    void *(*sslctxnew)(void);
    void *(*sslnew)(void*);
    int (*sslconnect)(void*, int);
    int (*sslwrite)(void*, const char*, int);
    int (*sslread)(void*, char*, int);
    void (*sslclose)(void*);
    void (*ssldestroy)(void*);
    int (*renderdocstart)(void);
    void (*renderdocend)(void);
    void (*imguiinit)(void*);
    void (*imguiupdate)(void*);
    void (*imguirender)(void*);
    void *(*gltfload)(const char*);
    void (*physdebugdraw)(float*, int, int, int, int);
    void *(*audiomixcreate)(int);
    void (*audiomixadd)(void*, float*, int);
    void (*audiomixplay)(void*);
    void *(*videocreate)(void);
    int (*videoload)(void*, const char*);
    int (*videoplay)(void*);
    int (*videoframe)(void*, unsigned char**);
    int (*unicodechar)(const char*);
    void (*lspstart)(int);
    void (*linteraddrule)(void*, const char*, const char*);
} platformapi;

extern environment *globalenv;
extern frame *currentframe;
extern int tcount;
extern platformapi platform;

void gcinit(void);
void gcaddroot(object *obj);
void gcremoveroot(object *obj);
void gcrun(void);
void gcmark(object *obj);
void gcmarkenv(environment *env);
object *gcalloc(int size);

environment *envnew(environment *parent);
void envset(environment *env, char *name, object *val, int cnst);
object *envget(environment *env, char *name);
environment *envfirst(void);

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
object *makeprocesshandle(long handle, int pid);
object *makewebrtc(void *pc, void *dc);
object *makewsclient(int fd, char *url);
object *makesslconn(SSL *ssl, SSL_CTX *ctx, int fd);
object *makerenderdoc(void *cap);
object *makeimgui(void *ctx, void *win);
object *makegltf(void *data, int vc, int ic, float *verts, int *inds);
object *makedebugphys(float *pos, int count, int r, int g, int b);
object *makeaudiomix(void *dev, void *ctx, int ch, float vol);
object *makevideo(void *stream, void *dec, int w, int h, float fps);
object *makeunicode(int cp, char *utf);
object *makelsp(void *srv, int port, int run);
object *makelinter(object *rules, int count);
object *makefile(FILE *f, int fd, int mode);
object *makethread(object *func, object *args);

void listappend(object *list, object *item);
object *listpop(object *list, int index);
void listinsert(object *list, int index, object *item);
void listsort(object *list);
void dictset(object *dict, object *key, object *val);
object *dictget(object *dict, object *key);
int dicthas(object *dict, object *key);
object *dictkeys(object *dict);
object *dictvalues(object *dict);
object *dictitems(object *dict);
int hasattr(object *obj, char *name);
object *getattr(object *obj, char *name);
void setattr(object *obj, char *name, object *val);

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
int platformprocessopen(int pid, int flags);
int platformprocessread(int handle, long address, unsigned char *buf, int size);
int platformprocesswrite(int handle, long address, unsigned char *buf, int size);
void platformprocessclose(int handle);
int platformprocessfind(const char *name);
void platformexit(int code);
char **platformlistdir(const char *path, int *count);
int platformmkdir(const char *path);
int platformremove(const char *path);
int platformrename(const char *oldp, const char *newp);
void *platformgetproc(void *lib, const char *name);

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
void registerprocesslib(environment *env);
void registeroverlaylib(environment *env);
void registerinputlib(environment *env);
void registerphysicslib(environment *env);
void registerecslib(environment *env);
void registeranimationlib(environment *env);
void registeraudiolib(environment *env);
void registerrenderlib(environment *env);
void registerphysicsextralib(environment *env);
void registernetworklib(environment *env);
void registerassetlib(environment *env);
void registerframedebuglib(environment *env);
void registerperfproflib(environment *env);
void registerbuildsyslib(environment *env);
void registerassetpipelinelib(environment *env);
void registerfluidlib(environment *env);
void registeriklib(environment *env);
void registermorphlib(environment *env);
void registerhotreloadlib(environment *env);
void registervolfoglib(environment *env);
void registermotionblurlib(environment *env);
void registerantialiaslib(environment *env);
void registerinstancinglib(environment *env);
void registerlodlib(environment *env);
void registerfrustumlib(environment *env);
void registerocclusionlib(environment *env);
void registergpuparticleslib(environment *env);
void registercompoundlib(environment *env);
void registerterrainphyslib(environment *env);
void registerbuoyancylib(environment *env);
void registerwindlib(environment *env);
void registerexplosionlib(environment *env);
void registersurfacedetaillib(environment *env);
void registergilibrary(environment *env);
void registeranimretargetlib(environment *env);
void registerrootmotionlib(environment *env);
void registeranimeventlib(environment *env);
void registerprocanimlib(environment *env);
void registermocaplib(environment *env);
void registeranimcomplib(environment *env);
void registeraudioocclib(environment *env);
void registeraudiostrelib(environment *env);
void registeraudiomixlib(environment *env);
void registeraudiofxlib(environment *env);
void registerspatialaudiolib(environment *env);
void registermidilib(environment *env);
void registergesturelib(environment *env);
void registervoiceinlib(environment *env);
void registermultitouchlib(environment *env);
void registervrctrllib(environment *env);
void registerhapticlib(environment *env);
void registerlagcomplib(environment *env);
void registerpredictlib(environment *env);
void registerinterplib(environment *env);
void registerreplicationlib(environment *env);
void registernatlib(environment *env);
void registermatchmakinglib(environment *env);
void registerlobbieslib(environment *env);
void registervoicechatlib(environment *env);
void registercloudsavelib(environment *env);
void registersteamlib(environment *env);
void registerepiclib(environment *env);
void registergoaplib(environment *env);
void registermctslib(environment *env);
void registerrllib(environment *env);
void registerperceptionlib(environment *env);
void registerteamailib(environment *env);
void registerfuzzylib(environment *env);
void registerutilityailib(environment *env);
void registermateditorlib(environment *env);
void registershadeditorlib(environment *env);
void registeranimateditorlib(environment *env);
void registerprefablib(environment *env);
void registerseencelib(environment *env);
void registerterraineditlib(environment *env);
void registerparteditlib(environment *env);
void registeruieditlib(environment *env);
void registermemproflib(environment *env);
void registerassetbrowserlib(environment *env);
void registerconsolelib(environment *env);
void registerscriptdebuglib(environment *env);
void registervclib(environment *env);
void registersteampublib(environment *env);
void registeritchiolib(environment *env);
void registerepicstorelib(environment *env);
void registerpslib(environment *env);
void registerxboxlib(environment *env);
void registernintendolib(environment *env);
void registermobilelib(environment *env);
void registeranalyticslib(environment *env);
void registercrashlib(environment *env);
void registerachievementslib(environment *env);
void registerleaderboardslib(environment *env);
void registeriaplib(environment *env);
void registerantipiracylib(environment *env);
void registerdrmlib(environment *env);
void registerobjectpoollib(environment *env);
void registereventsyslib(environment *env);
void registerserviceloclib(environment *env);
void registerdilib(environment *env);
void registercoroutinelib(environment *env);
void registertimerlib(environment *env);
void registerwsclientlib(environment *env);
void registerssllib(environment *env);
void registerrenderdoclib(environment *env);
void registerimguilib(environment *env);
void registergltflib(environment *env);
void registerphysdebuglib(environment *env);
void registeraudiomixerlib(environment *env);
void registervideolib(environment *env);
void registerlsplib(environment *env);
void registerlinterlib(environment *env);
void registermathlib(environment *env);
void registermathfulllib(environment *env);
void registervectorlib(environment *env);
void registergraphicslib(environment *env);
void registeroslib(environment *env);
void registersyslib(environment *env);
void registerrandomlib(environment *env);
void registerrelib(environment *env);
void registerjsonlib(environment *env);
void registerhashliblib(environment *env);
void registersubprocesslib(environment *env);
void registersocketlib(environment *env);
void registerdatetime(environment *env);
void registerdatetimelib(environment *env);
void registercsvlib(environment *env);
void registertempfilelib(environment *env);
void registerlogginglib(environment *env);
void registerargparselib(environment *env);
void registersecretslib(environment *env);
void registeritertools(environment *env);
void registercollectionslib(environment *env);
void registerthreadlib(environment *env);
void registerstructlib(environment *env);
void registershutillib(environment *env);
void registerstringlib(environment *env);
void registerstatisticslib(environment *env);
void registerio lib(environment *env);
void registerbase64lib(environment *env);
void registerbinasciilib(environment *env);
void registercmathlib(environment *env);
void registerconfigparserlib(environment *env);
void registerdecimal(environment *env);
void registerdecimallib(environment *env);
void registerdiffliblib(environment *env);
void registeremaillib(environment *env);
void registerfunctoolslib(environment *env);
void registerhmaclib(environment *env);
void registerzliblib(environment *env);
void registeruuidslib(environment *env);
void registerwebbrowserlib(environment *env);
void registerweakreflib(environment *env);
void registerwithlib(environment *env);
void registerunittestlib(environment *env);
void registertextwraplib(environment *env);
void registerarraylib(environment *env);
void registerplotlib(environment *env);
void registerpandaslib(environment *env);
void registernumpylib(environment *env);
void registertensorlib(environment *env);

#endif
