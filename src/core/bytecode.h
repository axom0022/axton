#ifndef bytecode_h
#define bytecode_h

#include "axton.h"

typedef enum {
    oploadconst, oploadvar, opstorevar, opadd, opsub, opmul, opdiv,
    opcall, opreturn, opjump, opjumpiffalse, oppop, opcompare,
    opmakelist, opmakedict, opgetattr, opsetattr, opindex, opsetindex,
    opmakefunc, opmakeclass, opmakeinstance, opgetglobal, opsetglobal
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
