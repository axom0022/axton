#include "axton.h"
#include "bytecode.h"
#include <pthread.h>
#include <string.h>

typedef struct vmthread {
    unsigned char *ip;
    object **stack;
    int stacksize;
    int stackcap;
    environment *env;
    object **locals;
    int localcount;
    struct vmthread *next;
} vmthread;

static vmthread *threads = NULL;
static int threadcount = 0;
static pthread_mutex_t vmlock = PTHREAD_MUTEX_INITIALIZER;

static void push(vmthread *t, object *o) {
    if (t->stacksize >= t->stackcap) {
        t->stackcap *= 2;
        t->stack = realloc(t->stack, t->stackcap * sizeof(object*));
    }
    t->stack[t->stacksize++] = o;
}

static object *pop(vmthread *t) {
    if (t->stacksize == 0) return makenone();
    return t->stack[--t->stacksize];
}

static object *executethread(vmthread *t) {
    bytecode *bc = (bytecode*)t->ip;
    while (1) {
        unsigned char op = *t->ip++;
        int idx;
        switch (op) {
            case oploadconst:
                idx = (t->ip[0] << 24) | (t->ip[1] << 16) | (t->ip[2] << 8) | t->ip[3];
                t->ip += 4;
                push(t, bc->constants[idx]);
                break;
            case oploadvar:
                idx = (t->ip[0] << 24) | (t->ip[1] << 16) | (t->ip[2] << 8) | t->ip[3];
                t->ip += 4;
                push(t, t->locals[idx]);
                break;
            case opstorevar:
                idx = (t->ip[0] << 24) | (t->ip[1] << 16) | (t->ip[2] << 8) | t->ip[3];
                t->ip += 4;
                t->locals[idx] = pop(t);
                break;
            case opadd: {
                object *right = pop(t);
                object *left = pop(t);
                push(t, addvalues(left, right));
                break;
            }
            case opsub: {
                object *right = pop(t);
                object *left = pop(t);
                push(t, subvalues(left, right));
                break;
            }
            case opmul: {
                object *right = pop(t);
                object *left = pop(t);
                push(t, mulvalues(left, right));
                break;
            }
            case opdiv: {
                object *right = pop(t);
                object *left = pop(t);
                push(t, divvalues(left, right));
                break;
            }
            case oppop:
                pop(t);
                break;
            case opreturn: {
                object *result = pop(t);
                free(t->stack);
                free(t->locals);
                pthread_mutex_lock(&vmlock);
                vmthread *prev = NULL, *cur = threads;
                while (cur && cur != t) { prev = cur; cur = cur->next; }
                if (cur) { if (prev) prev->next = cur->next; else threads = cur->next; }
                threadcount--;
                pthread_mutex_unlock(&vmlock);
                free(t);
                return result;
            }
            case opjump: {
                int offset = (t->ip[0] << 24) | (t->ip[1] << 16) | (t->ip[2] << 8) | t->ip[3];
                t->ip += 4;
                t->ip += offset;
                break;
            }
            case opjumpiffalse: {
                object *cond = pop(t);
                int offset = (t->ip[0] << 24) | (t->ip[1] << 16) | (t->ip[2] << 8) | t->ip[3];
                t->ip += 4;
                if (!istruthy(cond)) t->ip += offset;
                break;
            }
            default:
                throwexception("unknown opcode");
                break;
        }
    }
    return makenone();
}

static void *threadrunner(void *arg) {
    vmthread *t = (vmthread*)arg;
    object *result = executethread(t);
    return result;
}

object *executebytecode(bytecode *bc, environment *env) {
    vmthread *t = malloc(sizeof(vmthread));
    t->ip = bc->code;
    t->stackcap = 256;
    t->stacksize = 0;
    t->stack = malloc(t->stackcap * sizeof(object*));
    t->env = env;
    t->localcount = bc->namecount;
    t->locals = calloc(t->localcount, sizeof(object*));
    t->next = NULL;
    pthread_mutex_lock(&vmlock);
    t->next = threads;
    threads = t;
    threadcount++;
    pthread_mutex_unlock(&vmlock);
    pthread_t tid;
    pthread_create(&tid, NULL, threadrunner, t);
    void *result;
    pthread_join(tid, &result);
    return (object*)result;
}
