#ifndef BYTECODE_H
#define BYTECODE_H

#include "axton.h"

typedef enum {
    OPLOADCONST, OPLOADVAR, OPSTOREVAR, OPADD, OPSUB, OPMUL, OPDIV,
    OPCALL, OPRETURN, OPJUMP, OPJUMPIFFALSE, OPPOP, OPCOMPARE,
    OPMAKELIST, OPMAKEDICT, OPGETATTR, OPSETATTR, OPINDEX, OPSETINDEX,
    OPMAKEFUNC, OPMAKECLASS, OPMAKEINSTANCE, OPGETGLOBAL, OPSETGLOBAL
} opcode;

typedef struct bytecode {
    unsigned char *code;
    int *lines;
    int size;
    int cap;
    object **constants;
    int constcount;
    char **names;
    int namecount;
} bytecode;

bytecode *bytecodenew(void);
void bytecodeemit(bytecode *bc, unsigned char op, int line);
void bytecodeemitint(bytecode *bc, int val, int line);
void bytecodeemitconst(bytecode *bc, object *obj, int line);
int bytecodeemitname(bytecode *bc, char *name, int line);
void bytecodefree(bytecode *bc);

#endif
