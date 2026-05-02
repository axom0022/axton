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
#define PATH_SEP '\\'
#else
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PATH_SEP '/'
#endif

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "axton", __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__)
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define PLATFORM_IOS 1
#else
#define PLATFORM_MACOS 1
#endif
#endif

typedef enum {
    TOK_EOF, TOK_IDENT, TOK_NUMBER, TOK_STRING,
    TOK_INDENT, TOK_DEDENT, TOK_NEWLINE,
    TOK_LET, TOK_CONST, TOK_FN, TOK_IF, TOK_ELSE, TOK_ELIF,
    TOK_FOR, TOK_IN, TOK_WHILE, TOK_BREAK, TOK_NEXT, TOK_RETURN,
    TOK_NONE, TOK_TRUE, TOK_FALSE, TOK_AND, TOK_OR, TOK_NOT,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    TOK_EQ, TOK_EQEQ, TOK_NE, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACKET, TOK_RBRACKET,
    TOK_LBRACE, TOK_RBRACE, TOK_COMMA, TOK_DOT, TOK_COLON,
    TOK_TRY, TOK_CATCH, TOK_FINALLY, TOK_THROW, TOK_CLASS, TOK_IMPORT,
    TOK_ASYNC, TOK_AWAIT, TOK_YIELD
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
} expr;

typedef struct stmt {
    struct node node;
    void *(*exec)(struct stmt*, void*);
} stmt;

typedef struct object {
    struct object *next;
    struct object *prev;
    int marked;
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
            int pcount;
            struct stmt **body;
            int bcount;
            void *closure;
            char *name;
        } func;
        struct {
            void *(*fn)(struct object**, int, void*);
        } builtin;
        struct {
            char *name;
            void *attrs;
            struct object *super;
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
    };
} object;

typedef struct environment {
    char **names;
    object **values;
    int *isconst;
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
} platform_api;

extern environment *globalenv;
extern frame *currentframe;
extern int tcount;
extern platform_api platform;

void gc_init(void);
void gc_add_root(object *obj);
void gc_run(void);
object *gc_alloc(int size);

object *make_int(long v);
object *make_float(double v);
object *make_string(char *v);
object *make_bool(int v);
object *make_none(void);
object *make_list(void);
object *make_dict(void);
object *make_func(char **params, int pcount, stmt **body, int bcount, environment *closure, char *name);
object *make_builtin(void *(*fn)(object**, int, environment*));
object *make_class(char *name, environment *attrs, object *super);
object *make_instance(object *klass);
object *make_range(long start, long stop, long step);
object *make_module(char *name, void *handle);
object *make_native(void *handle, void *data);

void list_append(object *list, object *item);
object *list_get(object *list, long idx);
void dict_set(object *dict, object *key, object *val);
object *dict_get(object *dict, object *key);
int dict_has(object *dict, object *key);

environment *env_new(environment *parent);
void env_set(environment *env, char *name, object *val, int cnst);
object *env_get(environment *env, char *name);

int is_truthy(object *v);
int values_equal(object *a, object *b);
object *add_values(object *a, object *b);
object *sub_values(object *a, object *b);
object *mul_values(object *a, object *b);
object *div_values(object *a, object *b);
int less_than(object *a, object *b);
int greater_than(object *a, object *b);

void throw_exception(char *msg);
object *catch_exception(void);
void init_exceptions(environment *env);

token *tokenize(char *input);
stmt *parse_tokens(token *tokens, int count);
object *eval_program(stmt *program, environment *env);
object *call_function(object *fn, object **args, int argc, environment *env);
void register_builtins(environment *env);
void register_stdlib(environment *env);

void repl_start(void);
void debugger_break(int line);
void debugger_inspect(char *var);
void profiler_start(void);
void profiler_stop(void);
char *format_code(char *source);
char **lint_code(char *source, int *count);
char *generate_docs(char *source);

void platform_init(void);
void platform_log(const char *msg);
void platform_sleep(double seconds);
double platform_time(void);
char *platform_read_file(const char *path);
int platform_write_file(const char *path, const char *content);
void *platform_load_library(const char *path);
void *platform_get_proc(void *lib, const char *name);

#endif
