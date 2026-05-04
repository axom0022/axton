#include "axton.h"
#include "bytecode.h"

typedef struct vmframe {
    object **stack;
    int stacksize;
    int stackcap;
    unsigned char *ip;
    environment *env;
    object **locals;
    int localcount;
    struct vmframe *prev;
} vmframe;

static vmframe *currentframe = NULL;
static bytecode *currentbc = NULL;

static void push(object *o) {
    if (currentframe->stacksize >= currentframe->stackcap) {
        currentframe->stackcap *= 2;
        currentframe->stack = realloc(currentframe->stack, currentframe->stackcap * sizeof(object*));
    }
    currentframe->stack[currentframe->stacksize++] = o;
}

static object *pop(void) {
    if (currentframe->stacksize == 0) return makenone();
    return currentframe->stack[--currentframe->stacksize];
}

static object *vmrun(bytecode *bc, environment *env) {
    currentbc = bc;
    vmframe frame;
    frame.stackcap = 256;
    frame.stacksize = 0;
    frame.stack = malloc(frame.stackcap * sizeof(object*));
    frame.ip = bc->code;
    frame.env = env;
    frame.localcount = bc->namecount;
    frame.locals = calloc(frame.localcount, sizeof(object*));
    frame.prev = NULL;
    currentframe = &frame;

    while (1) {
        unsigned char op = *frame.ip++;
        int line = 0;
        (void)line;
        int idx, a, b, c, d;
        switch (op) {
            case OPLOADCONST:
                idx = (frame.ip[0] << 24) | (frame.ip[1] << 16) | (frame.ip[2] << 8) | frame.ip[3];
                frame.ip += 4;
                push(bc->constants[idx]);
                break;
            case OPLOADVAR:
                idx = (frame.ip[0] << 24) | (frame.ip[1] << 16) | (frame.ip[2] << 8) | frame.ip[3];
                frame.ip += 4;
                push(frame.locals[idx]);
                break;
            case OPSTOREVAR:
                idx = (frame.ip[0] << 24) | (frame.ip[1] << 16) | (frame.ip[2] << 8) | frame.ip[3];
                frame.ip += 4;
                frame.locals[idx] = pop();
                break;
            case OPADD: {
                object *right = pop();
                object *left = pop();
                push(addvalues(left, right));
                break;
            }
            case OPSUB: {
                object *right = pop();
                object *left = pop();
                push(subvalues(left, right));
                break;
            }
            case OPMUL: {
                object *right = pop();
                object *left = pop();
                push(mulvalues(left, right));
                break;
            }
            case OPDIV: {
                object *right = pop();
                object *left = pop();
                push(divvalues(left, right));
                break;
            }
            case OPPOP:
                pop();
                break;
            case OPRETURN: {
                object *result = pop();
                free(frame.stack);
                free(frame.locals);
                currentframe = frame.prev;
                return result;
            }
            default:
                throwexception("unknown opcode");
                break;
        }
    }
    return makenone();
}

object *executebytecode(bytecode *bc, environment *env) {
    return vmrun(bc, env);
}
