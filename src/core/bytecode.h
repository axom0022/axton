#ifndef bytecode_h
#define bytecode_h

#include "axton.h"

typedef enum {
    opnop,
    oploadconst, oploadint, oploadnone, oploadbool,
    oploadlocal, opstorelocal, opstorelocalc,
    oploadupvalue, opstoreupvalue,
    oploadglobal, opstoreglobal,
    oppop, opdup, oppopn,
    opadd, opsub, opmul, opdiv, opmod, oppow, opneg, opnot,
    opeq, opne, oplt, opgt, ople, opge,
    opjump, opjumpiffalse, opjumpiftrue,
    opcall, opcallattr, opreturn,
    opmakelist, opmakedict, opindex, opsetindex, opgetattr, opsetattr,
    opmakefunc, optrystmt, opclassstmt, opclassexpr, opthrow,
    opforbegin, opfornext, opforrangebegin, opforrangenext,
    opaddloc, opadd3, opsublocint, oplltlocint,
    opcount
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
    int *gcache;
    stmt **auxstmts;
    int auxcount;
    fnexp **auxfuncs;
    int auxfcount;
    char **slotnames;
    unsigned char *slotconst;
    int slotcount;
    int slotbase;
    int istop;
    int hascapture;
    int usesthis;
    int refs;
} bytecode;

extern bytecode *axprogrambc;

bytecode *bytecodenew(void);
void bytecodeemit(bytecode *bc, unsigned char op, int line);
void bytecodeemitint(bytecode *bc, int val, int line);
void bytecodeemitconst(bytecode *bc, object *obj, int line);
int bytecodeaddconst(bytecode *bc, object *obj);
int bytecodeemitname(bytecode *bc, char *name, int line);
void bytecodefree(bytecode *bc);
void bytetrace(bytecode *bc);
void axprogretire(bytecode *bc);
void axprogprune(void);
void axprogtrace(void);

#endif
