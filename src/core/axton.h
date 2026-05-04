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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>
#define mkdir(x,y) _mkdir(x)
#define strcasecmp _stricmp
#define snprintf _snprintf
#define PATHSEP '\\'
#else
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PATHSEP '/'
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "axton", __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__)
#endif

typedef enum {
    TOKEOF, TOKIDENT, TOKNUMBER, TOKSTRING, TOKFSTRING,
    TOKINDENT, TOKDEDENT, TOKNEWLINE,
    TOKLET, TOKCONST, TOKFN, TOKIF, TOKELSE, TOKELIF,
    TOKFOR, TOKIN, TOKWHILE, TOKBREAK, TOKNEXT, TOKRETURN,
    TOKNONE, TOKTRUE, TOKFALSE, TOKAND, TOKOR, TOKNOT,
    TOKPLUS, TOKMINUS, TOKSTAR, TOKSLASH, TOKPERCENT, TOKPOWER,
    TOKEQ, TOKEQEQ, TOKNE, TOKLT, TOKGT, TOKLE, TOKGE,
    TOKPLUSEQ, TOKMINUSEQ, TOKSTAREQ, TOKSLASHEQ,
    TOKLPAREN, TOKRPAREN, TOKLBRACKET, TOKRBRACKET,
    TOKLBRACE, TOKRBRACE, TOKCOMMA, TOKDOT, TOKCOLON, TOKCOLONEQ,
    TOKCOLONCOLON, TOKARROW, TOKUNDERSCORE,
    TOKTRY, TOKCATCH, TOKFINALLY, TOKTHROW, TOKCLASS, TOKIMPORT,
    TOKASYNC, TOKAWAIT, TOKYIELD, TOKWITH, TOKAS, TOKGLOBAL,
    TOKNONLOCAL, TOKASSERT, TOKDECORATOR, TOKMATCH, TOKCASE
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
            struct object *mro;
        } klass;
        struct {
            struct object *klass;
            void *attrs;
        } instance;
        struct {
            long start;
            long stop;
            long step;
            long current;
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
            struct object *enter;
            struct object *exit;
        } context;
        struct {
            struct object *getter;
            struct object *setter;
            struct object *deleter;
        } property;
        struct {
            char *name;
            struct object *fields;
        } dataclass;
        struct {
            char *name;
            int value;
        } enumval;
        struct {
            float *data;
            int size;
        } vector;
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
    int (*open)(const char*);
    void (*close)(void);
    void (*log)(const char*);
    void (*sleep)(double);
    double (*time)(void);
    char *(*readfile)(const char*);
    int (*writefile)(const char*, const char*);
    void *(*loadlib)(const char*);
    void *(*getproc)(void*, const char*);
    int (*socket)(void);
    int (*bind)(int, int);
    int (*listen)(int, int);
    int (*accept)(int);
    int (*send)(int, const char*, int);
    int (*recv)(int, char*, int);
    char **(*listdir)(const char*, int*);
    int (*mkdir)(const char*);
    int (*remove)(const char*);
    int (*rename)(const char*, const char*);
    char *(*getenv)(const char*);
    int (*setenv)(const char*, const char*);
    void (*exit)(int);
} platformapi;

extern environment *globalenv;
extern frame *currentframe;
extern int tcount;
extern platformapi platform;

void gcinit(void);
void gcaddroot(object *obj);
void gcremove(object *obj);
void gcretain(object *obj);
void gcrelease(object *obj);
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
object *maketensor(float *data, int rows, int cols);
object *makegenerator(object *func, environment *env);
object *makecoroutine(object *func, environment *env);
object *makevector(int size);
object *makevectorfromlist(object *list);

void listappend(object *list, object *item);
object *listpop(object *list, int index);
void listinsert(object *list, int index, object *item);
object *listremove(object *list, object *item);
object *listindex(object *list, object *item);
void listsort(object *list);
void listreverse(object *list);
object *listslice(object *list, int start, int stop, int step);

void dictset(object *dict, object *key, object *val);
object *dictget(object *dict, object *key);
void dictdel(object *dict, object *key);
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
void breakpoint(int line);
void inspectvar(char *var);
void profilestart(void);
void profilestop(void);
char *formatcode(char *source);
char **lintcode(char *source, int *count);
char *generatedocs(char *source);
char *typedocs(char *source);

void platforminit(void);
void platformlog(const char *msg);
void platformsleep(double seconds);
double platformtime(void);
char *platformreadfile(const char *path);
int platformwritefile(const char *path, const char *content);
void *platformloadlib(const char *path);
void *platformgetproc(void *lib, const char *name);
int platformsocket(void);
int platformbind(int fd, int port);
int platformlisten(int fd, int backlog);
int platformaccept(int fd);
int platformsend(int fd, const char *data, int len);
int platformrecv(int fd, char *buf, int len);
char **platformlistdir(const char *path, int *count);
int platformmkdir(const char *path);
int platformremove(const char *path);
int platformrename(const char *old, const char *new);
char *platformgetenv(const char *name);
int platformsetenv(const char *name, const char *value);
void platformexit(int code);

#endif
