#include "vm.h"
#include <stdlib.h>

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

static vmframe *curframe = NULL;
static bytecode *curbc = NULL;

static void push(object *o) {
    if (curframe->stacksize >= curframe->stackcap) {
        curframe->stackcap *= 2;
        curframe->stack = realloc(curframe->stack, curframe->stackcap * sizeof(object*));
    }
    curframe->stack[curframe->stacksize++] = o;
}

static object *pop(void) {
    if (curframe->stacksize == 0) return makenone();
    return curframe->stack[--curframe->stacksize];
}

object *executebytecode(bytecode *bc, environment *env) {
    curbc = bc;
    vmframe frame;
    frame.stackcap = 256;
    frame.stacksize = 0;
    frame.stack = malloc(frame.stackcap * sizeof(object*));
    frame.ip = bc->code;
    frame.env = env;
    frame.localcount = bc->namecount;
    frame.locals = calloc(frame.localcount, sizeof(object*));
    frame.prev = NULL;
    curframe = &frame;

    while (1) {
        unsigned char op = *frame.ip++;
        int idx;
        switch (op) {
            case oploadconst:
                idx = (frame.ip[0] << 24) | (frame.ip[1] << 16) | (frame.ip[2] << 8) | frame.ip[3];
                frame.ip += 4;
                push(bc->constants[idx]);
                break;
            case oploadvar:
                idx = (frame.ip[0] << 24) | (frame.ip[1] << 16) | (frame.ip[2] << 8) | frame.ip[3];
                frame.ip += 4;
                push(frame.locals[idx]);
                break;
            case opstorevar:
                idx = (frame.ip[0] << 24) | (frame.ip[1] << 16) | (frame.ip[2] << 8) | frame.ip[3];
                frame.ip += 4;
                frame.locals[idx] = pop();
                break;
            case opadd: {
                object *right = pop();
                object *left = pop();
                push(addvalues(left, right));
                break;
            }
            case opsub: {
                object *right = pop();
                object *left = pop();
                push(subvalues(left, right));
                break;
            }
            case opmul: {
                object *right = pop();
                object *left = pop();
                push(mulvalues(left, right));
                break;
            }
            case opdiv: {
                object *right = pop();
                object *left = pop();
                push(divvalues(left, right));
                break;
            }
            case oppop:
                pop();
                break;
            case opreturn: {
                object *result = pop();
                free(frame.stack);
                free(frame.locals);
                return result;
            }
            default:
                throwexception("unknown opcode");
                break;
        }
    }
    return makenone();
}
