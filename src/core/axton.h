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
#include <netdb.h>
#define PATHSEP '/'
#endif

#ifndef __ANDROID__
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "axton", __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__)
#endif

#ifndef __ANDROID__
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

#include <ffi.h>

#ifndef __ANDROID__
#include <vulkan/vulkan.h>
#include <d3d12.h>
#include <OpenCL/cl.h>
#include <cuda.h>
#include <webp/decode.h>
#include <webp/encode.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
#include <portaudio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <assimp/assimp.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btSoftBody.h>
#include <bullet/btSoftBodyHelpers.h>
#include <bullet/btSoftRigidDynamicsWorld.h>
#include <openvr/openvr.h>
#include <arpa/inet.h>
#include <nghttp2/nghttp2.h>
#include <h2o.h>
#include <graphql/graphql.h>
#include <mongoc/mongoc.h>
#include <pgsql/libpq-fe.h>
#include <mysql/mysql.h>
#include <sqlite3.h>
#endif

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
    TOKUNION, TOKOPTIONAL, TOKASYNCIO, TOKFSTRING,
    TOKWALRUS, TOKTRIPLE, TOKRAW, TOKNULLSAFE,
    TOKRESULT, TOKGENERIC, TOKMACRO, TOKJIT, TOKGPCU,
    TOKSIMD, TOKVR, TOKAR, TOKXOM
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
            object *rules;
            int count;
        } linter;
        struct {
            void *rt;
            int x;
            int y;
            int z;
            int w;
        } ray;
        struct {
            void *world;
            void *body;
            void *joint;
        } phys;
        struct {
            void *headset;
            void *controllers;
            int connected;
        } vr;
        struct {
            void *agent;
            void *env;
            void *policy;
        } rl;
        struct {
            void *model;
            void *tokenizer;
            int loaded;
        } llm;
        struct {
            void *loop;
            void *tasks;
            int running;
        } async;
        struct {
            void *fiber;
            int state;
        } fiber;
        struct {
            void *ch;
            void *sendq;
            void *recvq;
        } channel;
        struct {
            void *sem;
            int count;
        } semaphore;
        struct {
            void *pool;
            int size;
            int active;
        } threadpool;
        struct {
            void *stream;
            void *context;
            int connected;
        } http3;
        struct {
            void *sse;
            void *clients;
        } sse;
        struct {
            void *schema;
            void *resolvers;
        } graphql;
        struct {
            void *cipher;
            void *key;
            void *iv;
        } cipher;
        struct {
            void *token;
            void *header;
            void *payload;
            void *signature;
        } jwttok;
        struct {
            void *client;
            void *provider;
            void *state;
        } oauth;
        struct {
            void *bigint;
            char *digits;
            int sign;
        } bigint;
        struct {
            double real;
            double imag;
        } complex;
        struct {
            char *digits;
            int scale;
            int precision;
        } bigdec;
        struct {
            void *data;
            int rows;
            int cols;
        } matrix;
        struct {
            void *df;
            void *columns;
            void *rows;
        } dataframe;
        struct {
            void *expr;
            void *vars;
        } sym;
        struct {
            void *plot;
            void *series;
            void *axes;
        } plot;
        struct {
            void *viz;
            void *scene;
            void *camera;
        } viz3d;
        struct {
            void *gpu;
            void *kernel;
            void *buffer;
        } gpuobj;
        struct {
            void *jit;
            void *code;
            void *func;
        } jitobj;
        struct {
            void *sandbox;
            void *perms;
            int enabled;
        } sandbox;
        struct {
            void *stack;
            int depth;
            void *frames;
        } stacktrace;
        struct {
            void *plugin;
            void *handle;
            void *api;
        } plugin;
        struct {
            void *vulkan;
            void *device;
            void *swapchain;
        } vulkanobj;
        struct {
            void *d3d12;
            void *device;
            void *swapchain;
        } d3d12obj;
        struct {
            void *anim;
            void *bones;
            void *clips;
            void *state;
        } animobj;
        struct {
            void *audio;
            void *stream;
            void *effects;
        } audioobj;
        struct {
            void *net;
            void *sockets;
            void *protocol;
        } netobj;
        struct {
            void *ecs;
            void *entities;
            void *systems;
            void *components;
        } ecsobj;
        struct {
            void *scene;
            void *nodes;
            void *transforms;
            void *renderer;
        } sceneobj;
        struct {
            void *asset;
            void *loader;
            void *cache;
        } assetobj;
        struct {
            void *console;
            void *platform;
            void *sdk;
        } consoleobj;
        struct {
            void *ws;
            void *conns;
            void *events;
        } ws obj;
        struct {
            void *session;
            void *store;
            void *expiry;
        } sessionobj;
        struct {
            void *frontend;
            void *components;
            void *state;
        } frontendobj;
        struct {
            void *api;
            void *routes;
            void *middleware;
        } apiframework;
        struct {
            void *rate;
            void *limits;
            void *buckets;
        } ratelimit;
        struct {
            void *xom;
            void *data;
            void *dtype;
        } xomobj;
        struct {
            void *sci;
            void *special;
            void *stats;
        } sciobj;
        struct {
            void *xlearn;
            void *models;
            void *preprocessing;
        } xlearnobj;
        struct {
            void *torch;
            void *tensor;
            void *module;
            void *optim;
        } torchobj;
        struct {
            void *flux;
            void *graph;
            void *session;
            void *ops;
        } fluxobj;
        struct {
            void *vision2;
            void *image;
            void *video;
            void *features;
        } vision2obj;
        struct {
            void *shield;
            void *tokens;
            void *validate;
        } shieldobj;
        struct {
            void *clean;
            void *filter;
            void *sanitize;
            void *escape;
        } cleanobj;
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
    char *file;
    int line;
    struct object *stacktrace;
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
#ifndef __ANDROID__
    int (*pcapopen)(const char*, char*);
    int (*pcapnext)(int, unsigned char**, int*);
    int (*pcapinject)(int, unsigned char*, int);
    void (*pcapclose)(int);
    int (*bluetoothopen)(void);
    int (*bluetoothscan)(int, object*);
    void (*bluetoothclose)(int);
#endif
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
#ifndef __ANDROID__
    int (*renderdocstart)(void);
    void (*renderdocend)(void);
    void (*imguiinit)(void*);
    void (*imguiupdate)(void*);
    void (*imguirender)(void*);
    void *(*gltfload)(const char*);
    void (*physdebugdraw)(float*, int, int, int);
    void *(*audiomixcreate)(int);
    void (*audiomixadd)(void*, float*, int);
    void (*audiomixplay)(void*);
    void *(*videocreate)(void);
    int (*videoload)(void*, const char*);
    int (*videoplay)(void*);
    int (*videoframe)(void*, unsigned char**);
    void *(*raycreate)(void);
    void (*rayrender)(void*, int, int);
    void (*raysetlight)(void*, float, float, float, float);
    void (*raysetmaterial)(void*, float, float, float, float);
    void *(*physcreate)(void);
    void (*physaddbody)(void*, float, float, float, float);
    void (*physstep)(void*, float);
    void (*physsetgravity)(void*, float, float, float);
    void *(*vrcreate)(void);
    int (*vrconnect)(void*);
    void (*vrpoll)(void*);
    void (*vrendrer)(void*);
    void *(*rlcreate)(void);
    void (*rltrain)(void*, int, int, float);
    void *(*rlpredict)(void*, float*);
    void *(*llmcreate)(void);
    int (*llmload)(void*, const char*);
    char *(*llmgenerate)(void*, const char*);
    void *(*asynccreate)(void);
    void (*asyncadd)(void*, void*);
    int (*asyncrun)(void*);
    void *(*fibercreate)(void);
    void (*fiberswitch)(void*);
    void *(*channelcreate)(int);
    void (*channelsend)(void*, void*);
    void *(*channelrecv)(void*);
    void *(*semcreate)(int);
    void (*semwait)(void*);
    void (*sempost)(void*);
    void *(*poolcreate)(int);
    void (*poolsubmit)(void*, void*);
    void (*poolwait)(void*);
    void *(*http3create)(void);
    int (*http3listen)(void*, int);
    void (*http3serve)(void*);
    void *(*ssecreate)(void);
    void (*sseadd)(void*, const char*);
    void (*ssesend)(void*, const char*);
    void *(*graphqlcreate)(void);
    void (*graphqladdquery)(void*, const char*, void*);
    void (*graphqladdmutation)(void*, const char*, void*);
    void *(*graphqlserve)(void*, int);
    void *(*ciphercreate)(void);
    void (*cipherencrypt)(void*, char*, int);
    void (*cipherdecrypt)(void*, char*, int);
    void *(*jwtcreate)(void);
    char *(*jwtencode)(void*, char*);
    void *(*jwtdecode)(void*, char*);
    void *(*oauthcreate)(void);
    char *(*oauthauthurl)(void*);
    char *(*oauthgettoken)(void*, char*);
    void *(*bigintcreate)(char*);
    void *(*bigintadd)(void*, void*);
    void *(*bigintmul)(void*, void*);
    void *(*complexcreate)(double, double);
    void *(*complexadd)(void*, void*);
    void *(*complexmul)(void*, void*);
    void *(*bigdeccreate)(char*, int);
    void *(*bigdecadd)(void*, void*);
    void *(*bigdecmul)(void*, void*);
    void *(*matrixcreate)(int, int);
    void (*matrixset)(void*, int, int, double);
    void *(*matrixmul)(void*, void*);
    void *(*dataframecreate)(void);
    void (*dataframeload)(void*, char*);
    void *(*dataframefilter)(void*, void*);
    void *(*symcreate)(char*);
    void *(*symdiff)(void*, char*);
    void *(*symintegrate)(void*, char*);
    void *(*plotcreate)(void);
    void (*plotline)(void*, float*, float*, int);
    void (*plotscatter)(void*, float*, float*, int);
    void (*plotsave)(void*, char*);
    void *(*viz3dcreate)(void);
    void (*viz3dadd)(void*, float*, float*, float*, int);
    void (*viz3drender)(void*);
    void *(*gpucreate)(void);
    void *(*gpukernel)(void*, char*);
    void (*gpurun)(void*, void*, int);
    void *(*jitcreate)(void);
    void *(*jitcompile)(void*, char*);
    void *(*jitexec)(void*, void**);
    void *(*sandboxcreate)(void);
    void (*sandboxsetperm)(void*, char*, int);
    void *(*sandboxrun)(void*, void*);
    void *(*stacktracecreate)(void);
    void (*stacktraceadd)(void*, char*, int);
    void *(*stacktraceget)(void*);
    void *(*plugincreate)(void);
    int (*pluginload)(void*, char*);
    void *(*plugincall)(void*, char*, void*);
    void *(*vulkancreate)(void);
    void (*vulkanrender)(void*);
    void *(*d3d12create)(void);
    void (*d3d12render)(void*);
    void *(*animcreate)(void);
    void (*animaddbone)(void*, char*, int);
    void (*animaddkeyframe)(void*, int, float, float*, float*, float*);
    void (*animplay)(void*, char*, float);
    void (*animupdate)(void*, float);
    void *(*audiocreate)(void);
    void (*audioload)(void*, char*);
    void (*audioplay)(void*);
    void (*audiostop)(void*);
    void *(*netcreate)(void);
    void (*netlisten)(void*, int);
    void (*netsend)(void*, char*, int);
    void *(*netrecv)(void*);
    void *(*ecscreate)(void);
    void (*ecsaddentity)(void*, void*);
    void (*ecsaddcomponent)(void*, void*, int, void*);
    void (*ecssystem)(void*, int, void*);
    void (*ecsupdate)(void*, float);
    void *(*scenecreate)(void);
    void (*sceneaddnode)(void*, void*);
    void (*scenerender)(void*);
    void *(*assetcreate)(void);
    void (*assetload)(void*, char*);
    void *(*assetget)(void*, char*);
    void *(*consolecreate)(void*, char*);
    int (*consoleconnect)(void*);
    void (*consoledeploy)(void*);
    void *(*wscreate)(void);
    void (*wsadd)(void*, int, void*);
    void (*wssend)(void*, int, char*);
    void *(*sessioncreate)(void);
    void (*sessionset)(void*, char*, void*);
    void *(*sessionget)(void*, char*);
    void *(*frontendcreate)(void);
    void (*frontendadd)(void*, char*, void*);
    void (*frontendrender)(void*);
    void *(*apicreate)(void);
    void (*apiaddroute)(void*, char*, char*, void*);
    void (*apistart)(void*, int);
    void *(*ratelimitcreate)(int, int);
    int (*ratelimitcheck)(void*, char*);
    void *(*xomcreate)(void*, int);
    void (*xomadd)(void*, void*);
    void *(*xommul)(void*, void*);
    void *(*scicreate)(void);
    double (*scistats)(void*, char*);
    void *(*xlearncreate)(void);
    void (*xlearntrain)(void*, void*, void*);
    void *(*xlearnpredict)(void*, void*);
    void *(*torchcreate)(void);
    void (*torchtrain)(void*, void*, void*, int);
    void *(*torchpredict)(void*, void*);
    void *(*fluxcreate)(void);
    void (*fluxtrain)(void*, void*, void*, int);
    void *(*fluxpredict)(void*, void*);
    void *(*vision2create)(void);
    void (*vision2load)(void*, char*);
    void *(*vision2detect)(void*);
    void *(*vision2classify)(void*);
    void *(*shieldcreate)(void);
    char *(*shieldtoken)(void*);
    int (*shieldvalidate)(void*, char*);
    void *(*cleancreate)(void);
    char *(*cleansanitize)(void*, char*);
#endif
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
object *makeray(void *rt);
object *makephys(void *world);
object *makevr(void *headset);
object *makerl(void *agent);
object *makellm(void *model);
object *makeasync(void *loop);
object *makefiber(void *fiber);
object *makechannel(void *ch);
object *makesemaphore(void *sem);
object *makethreadpool(void *pool);
object *makehttp3(void *stream);
object *makesse(void *sse);
object *makegraphql(void *schema);
object *makecipher(void *cipher);
object *makejwt(void *jwt);
object *makeoauth(void *oauth);
object *makebigint(void *bi);
object *makecomplex(double real, double imag);
object *makebigdec(void *bd);
object *makematrix(void *m);
object *makedataframe(void *df);
object *makesym(void *sym);
object *makeplot(void *plot);
object *makeviz3d(void *viz);
object *makegpuobj(void *gpu);
object *makejitobj(void *jit);
object *makesandbox(void *sb);
object *makestacktrace(void *st);
object *makeplugin(void *pl);
object *makevulkanobj(void *vk);
object *maked3d12obj(void *d3d);
object *makeanimobj(void *anim);
object *makeaudioobj(void *audio);
object *makenetobj(void *net);
object *makeecsobj(void *ecs);
object *makesceneobj(void *scene);
object *makeassetobj(void *asset);
object *makeconsoleobj(void *console);
object *makewsobj(void *ws);
object *makesessionobj(void *session);
object *makefrontendobj(void *frontend);
object *makeapiframework(void *api);
object *makeratelimitobj(void *rate);
object *makexomobj(void *xom);
object *makesciobj(void *sci);
object *makexlearnobj(void *xlearn);
object *maketorchobj(void *torch);
object *makefluxobj(void *flux);
object *makevision2obj(void *vision2);
object *makeshieldobj(void *shield);
object *makecleanobj(void *clean);

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
#ifndef __ANDROID__
int platformpcapopen(const char *iface, char *err);
int platformpcapnext(int handle, unsigned char **data, int *len);
int platformpcapinject(int handle, unsigned char *packet, int len);
void platformpcapclose(int handle);
int platformbluetoothopen(void);
int platformbluetoothscan(int handle, object *devices);
void platformbluetoothclose(int handle);
#endif
int platformprocessopen(int pid, int flags);
int platformprocessread(int handle, long address, unsigned char *buf, int size);
int platformprocesswrite(int handle, long address, unsigned char *buf, int size);
void platformprocessclose(int handle);
int platformprocessfind(const char *name);
int platformsslinit(void);
void *platformsslctxnew(void);
void *platformsslnew(void *ctx);
int platformsslconnect(void *ssl, int fd);
int platformsslwrite(void *ssl, const char *data, int len);
int platformsslread(void *ssl, char *buf, int len);
void platformsslclose(void *ssl);
void platformssldestroy(void *ssl);
#ifndef __ANDROID__
int platformrenderdocstart(void);
void platformrenderdocend(void);
void platformimguiinit(void *window);
void platformimguiupdate(void *window);
void platformimguirender(void *window);
void *platformgltfload(const char *path);
void platformphysdebugdraw(float *pos, int count, int r, int g, int b);
void *platformaudiomixcreate(int channels);
void platformaudiomixadd(void *mix, float *samples, int count);
void platformaudiomixplay(void *mix);
void *platformvideocreate(void);
int platformvideoload(void *vid, const char *path);
int platformvideoplay(void *vid);
int platformvideoframe(void *vid, unsigned char **data);
void *platformraycreate(void);
void platformrayrender(void *ray, int w, int h);
void platformraysetlight(void *ray, float x, float y, float z, float i);
void platformraysetmaterial(void *ray, float r, float g, float b, float s);
void *platformphyscreate(void);
void platformphysaddbody(void *phys, float x, float y, float z, float m);
void platformphysstep(void *phys, float dt);
void platformphyssetgravity(void *phys, float x, float y, float z);
void *platformvrcreate(void);
int platformvrconnect(void *vr);
void platformvrpoll(void *vr);
void platformvrrender(void *vr);
void *platformrlcreate(void);
void platformrltrain(void *rl, int states, int actions, float lr);
void *platformrlpredict(void *rl, float *state);
void *platformllmcreate(void);
int platformllmload(void *llm, const char *path);
char *platformllmgenerate(void *llm, const char *prompt);
void *platformasynccreate(void);
void platformasyncadd(void *async, void *task);
int platformasyncrun(void *async);
void *platformfibercreate(void);
void platformfiberswitch(void *fiber);
void *platformchannelcreate(int size);
void platformchannelsend(void *ch, void *data);
void *platformchannelrecv(void *ch);
void *platformsemcreate(int count);
void platformsemwait(void *sem);
void platformsempost(void *sem);
void *platformpoolcreate(int size);
void platformpoolsubmit(void *pool, void *task);
void platformpoolwait(void *pool);
void *platformhttp3create(void);
int platformhttp3listen(void *http3, int port);
void platformhttp3serve(void *http3);
void *platformssecreate(void);
void platformsseadd(void *sse, const char *id);
void platformssesend(void *sse, const char *data);
void *platformgraphqlcreate(void);
void platformgraphqladdquery(void *gql, const char *name, void *fn);
void platformgraphqladdmutation(void *gql, const char *name, void *fn);
void *platformgraphqlserve(void *gql, int port);
void *platformciphercreate(void);
void platformcipherencrypt(void *cipher, char *data, int len);
void platformcipherdecrypt(void *cipher, char *data, int len);
void *platformjwtcreate(void);
char *platformjwtencode(void *jwt, char *secret);
void *platformjwtdecode(void *jwt, char *token);
void *platformoauthcreate(void);
char *platformoauthauthurl(void *oauth);
char *platformoauthgettoken(void *oauth, char *code);
void *platformbigintcreate(char *str);
void *platformbigintadd(void *a, void *b);
void *platformbigintmul(void *a, void *b);
void *platformcomplexcreate(double r, double i);
void *platformcomplexadd(void *a, void *b);
void *platformcomplexmul(void *a, void *b);
void *platformbigdeccreate(char *str, int scale);
void *platformbigdecadd(void *a, void *b);
void *platformbigdecmul(void *a, void *b);
void *platformmatrixcreate(int r, int c);
void platformmatrixset(void *m, int r, int c, double v);
void *platformmatrixmul(void *a, void *b);
void *platformdataframecreate(void);
void platformdataframeload(void *df, char *path);
void *platformdataframefilter(void *df, void *fn);
void *platformsymcreate(char *expr);
void *platformsymdiff(void *sym, char *var);
void *platformsymintegrate(void *sym, char *var);
void *platformplotcreate(void);
void platformplotline(void *plot, float *x, float *y, int n);
void platformplotscatter(void *plot, float *x, float *y, int n);
void platformplotsave(void *plot, char *path);
void *platformviz3dcreate(void);
void platformviz3dadd(void *viz, float *x, float *y, float *z, int n);
void platformviz3drender(void *viz);
void *platformgpucreate(void);
void *platformgpukernel(void *gpu, char *src);
void platformgpurun(void *gpu, void *kernel, int n);
void *platformjitcreate(void);
void *platformjitcompile(void *jit, char *src);
void *platformjitexec(void *jit, void **args);
void *platformsandboxcreate(void);
void platformsandboxsetperm(void *sb, char *path, int perm);
void *platformsandboxrun(void *sb, void *fn);
void *platformstacktracecreate(void);
void platformstacktraceadd(void *st, char *file, int line);
void *platformstacktraceget(void *st);
void *platformplugincreate(void);
int platformpluginload(void *plugin, char *path);
void *platformplugincall(void *plugin, char *name, void *args);
void *platformvulkancreate(void);
void platformvulkanrender(void *vk);
void *platformd3d12create(void);
void platformd3d12render(void *d3d);
void *platformanimcreate(void);
void platformanimaddbone(void *anim, char *name, int parent);
void platformanimaddkeyframe(void *anim, int bone, float time, float *pos, float *rot, float *scale);
void platformanimplay(void *anim, char *name, float speed);
void platformanimupdate(void *anim, float dt);
void *platformaudiocreate(void);
void platformaudioload(void *audio, char *path);
void platformaudioplay(void *audio);
void platformaudiostop(void *audio);
void *platformnetcreate(void);
void platformnetlisten(void *net, int port);
void platformnetsend(void *net, char *data, int len);
void *platformnetrecv(void *net);
void *platformecscreate(void);
void platformecsaddentity(void *ecs, void *entity);
void platformecsaddcomponent(void *ecs, void *entity, int type, void *comp);
void platformecssystem(void *ecs, int type, void *fn);
void platformecsupdate(void *ecs, float dt);
void *platformscenecreate(void);
void platformsceneaddnode(void *scene, void *node);
void platformscenerender(void *scene);
void *platformassetcreate(void);
void platformassetload(void *asset, char *path);
void *platformassetget(void *asset, char *name);
void *platformconsolecreate(void *ctx, char *platform);
int platformconsoleconnect(void *console);
void platformconsoledeploy(void *console);
void *platformwscreate(void);
void platformwsadd(void *ws, int id, void *fn);
void platformwssend(void *ws, int id, char *data);
void *platformsessioncreate(void);
void platformsessionset(void *session, char *key, void *val);
void *platformsessionget(void *session, char *key);
void *platformfrontendcreate(void);
void platformfrontendadd(void *fe, char *name, void *comp);
void platformfrontendrender(void *fe);
void *platformapicreate(void);
void platformapiaddroute(void *api, char *path, char *method, void *fn);
void platformapistart(void *api, int port);
void *platformratelimitcreate(int limit, int window);
int platformratelimitcheck(void *rl, char *key);
void *platformxomcreate(void *data, int size);
void platformxomadd(void *n1, void *n2);
void *platformxommul(void *n1, void *n2);
void *platformscicreate(void);
double platformscistats(void *sci, char *name);
void *platformxlearncreate(void);
void platformxlearntrain(void *xl, void *x, void *y);
void *platformxlearnpredict(void *xl, void *x);
void *platformtorchcreate(void);
void platformtorchtrain(void *tc, void *x, void *y, int epochs);
void *platformtorchpredict(void *tc, void *x);
void *platformfluxcreate(void);
void platformfluxtrain(void *fx, void *x, void *y, int epochs);
void *platformfluxpredict(void *fx, void *x);
void *platformvision2create(void);
void platformvision2load(void *cv, char *path);
void *platformvision2detect(void *cv);
void *platformvision2classify(void *cv);
void *platformshieldcreate(void);
char *platformshieldtoken(void *cs);
int platformshieldvalidate(void *cs, char *token);
void *platformcleancreate(void);
char *platformcleansanitize(void *cl, char *input);
#endif
int platformunicodechar(const char *utf8);
void platformlspstart(int port);
void platformlinteraddrule(void *linter, const char *name, const char *pattern);

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
void registerassetpip elib(environment *env);
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
void registerdr mlib(environment *env);
void registerobjectpoollib(environment *env);
void registereventsyslib(environment *env);
void registerserviceloclib(environment *env);
void registerdilib(environment *env);
void registercoroutinelib(environment *env);
void registertimerlib(environment *env);
void registerwebrtclib(environment *env);
void registerwsclientlib(environment *env);
void registerssllib(environment *env);
void registerrenderdoclib(environment *env);
void registerimguilib(environment *env);
void registergltflib(environment *env);
void registerphysdebuglib(environment *env);
void registeraudiomixerlib(environment *env);
void registervideolib(environment *env);
void registerunicodelib(environment *env);
void registerlsplib(environment *env);
void registerlinterlib(environment *env);
void registerraymodule(environment *env);
void registerphysmodule(environment *env);
void registervrmodule(environment *env);
void registerrlmodule(environment *env);
void registerllmmodule(environment *env);
void registerasyncmodule(environment *env);
void registerfibermodule(environment *env);
void registerchannelmodule(environment *env);
void registersemmodule(environment *env);
void registerpoolmodule(environment *env);
void registerhttp3module(environment *env);
void registerssemodule(environment *env);
void registergraphqlmodule(environment *env);
void registerciphermodule(environment *env);
void registerjwtmodule(environment *env);
void registeroauthmodule(environment *env);
void registerbigintmodule(environment *env);
void registercomplexmodule(environment *env);
void registerbigdecmodule(environment *env);
void registermatrixmodule(environment *env);
void registerdataframemodule(environment *env);
void registersymmodule(environment *env);
void registerplotmodule(environment *env);
void registerviz3dmodule(environment *env);
void registergpumodule(environment *env);
void registerjitmodule(environment *env);
void registersandboxmodule(environment *env);
void registerstackmodule(environment *env);
void registerpluginmodule(environment *env);
void registervulkanmodule(environment *env);
void registerd3d12module(environment *env);
void registeranimmodule(environment *env);
void registeraudiomodule(environment *env);
void registernetmodule(environment *env);
void registerecsmodule(environment *env);
void registerscenemodule(environment *env);
void registerassetmodule(environment *env);
void registerconsolemodule(environment *env);
void registerwsmodule(environment *env);
void registersessionmodule(environment *env);
void registerfrontendmodule(environment *env);
void registerapimodule(environment *env);
void registerratelimitmodule(environment *env);
void registerxommodule(environment *env);
void registerscimodule(environment *env);
void registerxlearnmodule(environment *env);
void registertorchmodule(environment *env);
void registerfluxmodule(environment *env);
void registervision2module(environment *env);
void registershieldmodule(environment *env);
void registercleanmodule(environment *env);

#endif
