#include "bytecode.h"
#include <stdlib.h>
#include <string.h>

bytecode *bytecodenew(void) {
    bytecode *bc = malloc(sizeof(bytecode));
    bc->cap = 1024;
    bc->size = 0;
    bc->code = malloc(bc->cap);
    bc->lines = malloc(bc->cap * sizeof(int));
    bc->constants = malloc(sizeof(object*) * 16);
    bc->constcount = 0;
    bc->names = malloc(sizeof(char*) * 16);
    bc->namecount = 0;
    return bc;
}

void bytecodeemit(bytecode *bc, unsigned char op, int line) {
    if (bc->size >= bc->cap) {
        bc->cap *= 2;
        bc->code = realloc(bc->code, bc->cap);
        bc->lines = realloc(bc->lines, bc->cap * sizeof(int));
    }
    bc->code[bc->size] = op;
    bc->lines[bc->size] = line;
    bc->size++;
}

void bytecodeemitint(bytecode *bc, int val, int line) {
    bytecodeemit(bc, (val >> 24) & 0xFF, line);
    bytecodeemit(bc, (val >> 16) & 0xFF, line);
    bytecodeemit(bc, (val >> 8) & 0xFF, line);
    bytecodeemit(bc, val & 0xFF, line);
}

void bytecodeemitconst(bytecode *bc, object *obj, int line) {
    int idx = -1;
    for (int i = 0; i < bc->constcount; i++) {
        if (bc->constants[i] == obj || valuesequal(bc->constants[i], obj)) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        idx = bc->constcount;
        bc->constants = realloc(bc->constants, (bc->constcount + 1) * sizeof(object*));
        bc->constants[bc->constcount++] = obj;
    }
    bytecodeemit(bc, oploadconst, line);
    bytecodeemitint(bc, idx, line);
}

int bytecodeemitname(bytecode *bc, char *name, int line) {
    int idx = -1;
    for (int i = 0; i < bc->namecount; i++) {
        if (strcmp(bc->names[i], name) == 0) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        idx = bc->namecount;
        bc->names = realloc(bc->names, (bc->namecount + 1) * sizeof(char*));
        bc->names[bc->namecount++] = strdup(name);
    }
    return idx;
}

void bytecodefree(bytecode *bc) {
    free(bc->code);
    free(bc->lines);
    free(bc->constants);
    for (int i = 0; i < bc->namecount; i++) free(bc->names[i]);
    free(bc->names);
    free(bc);
}
