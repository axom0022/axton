#include "bytecode.h"
#include <stdlib.h>
#include <string.h>

bytecode *axprogrambc = NULL;

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
    bc->gcache = malloc(bc->cap * sizeof(int));
    memset(bc->gcache, 0xFF, bc->cap * sizeof(int));
    bc->auxstmts = NULL;
    bc->auxcount = 0;
    bc->auxfuncs = NULL;
    bc->auxfcount = 0;
    bc->slotnames = NULL;
    bc->slotconst = NULL;
    bc->slotcount = 0;
    bc->slotbase = 0;
    bc->istop = 0;
    bc->hascapture = 0;
    bc->usesthis = 0;
    bc->refs = 0;
    return bc;
}

void bytecodeemit(bytecode *bc, unsigned char op, int line) {
    if (bc->size >= bc->cap) {
        bc->cap *= 2;
        bc->code = realloc(bc->code, bc->cap);
        bc->lines = realloc(bc->lines, bc->cap * sizeof(int));
        bc->gcache = realloc(bc->gcache, bc->cap * sizeof(int));
        memset(bc->gcache + bc->size, 0xFF, (bc->cap - bc->size) * sizeof(int));
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

int bytecodeaddconst(bytecode *bc, object *obj) {
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
    return idx;
}

void bytecodeemitconst(bytecode *bc, object *obj, int line) {
    bytecodeemit(bc, oploadconst, line);
    bytecodeemitint(bc, bytecodeaddconst(bc, obj), line);
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
    free(bc->gcache);
    free(bc->auxstmts);
    free(bc->auxfuncs);
    for (int i = 0; i < bc->slotcount; i++) free(bc->slotnames[i]);
    free(bc->slotnames);
    free(bc->slotconst);
    free(bc);
}

void bytetrace(bytecode *bc) {
    if (!bc) return;
    for (int i = 0; i < bc->constcount; i++) {
        gcmark(bc->constants[i]);
    }
}

static bytecode **retiredprogs = NULL;
static int retiredcount = 0;
static int retiredcap = 0;

void axprogretire(bytecode *bc) {
    if (!bc) return;
    if (retiredcount >= retiredcap) {
        retiredcap = retiredcap ? retiredcap * 2 : 8;
        retiredprogs = realloc(retiredprogs, retiredcap * sizeof(bytecode*));
    }
    retiredprogs[retiredcount++] = bc;
}

void axprogprune(void) {
    int w = 0;
    for (int i = 0; i < retiredcount; i++) {
        if (retiredprogs[i]->refs == 0) {
            bytecodefree(retiredprogs[i]);
        } else {
            retiredprogs[w++] = retiredprogs[i];
        }
    }
    retiredcount = w;
}

void axprogtrace(void) {
    if (axprogrambc) bytetrace(axprogrambc);
    for (int i = 0; i < retiredcount; i++) {
        bytetrace(retiredprogs[i]);
    }
}
