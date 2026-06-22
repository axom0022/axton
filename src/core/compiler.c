#include "compiler.h"
#include <string.h>

static bytecode *curbc = NULL;

static void compileexpr(expr *e);
static void compilestmt(stmt *s);

void compileprogram(stmt *prog, bytecode *bc) {
    curbc = bc;
    stmtlist *list = (stmtlist*)prog;
    for (int i = 0; i < list->count; i++) compilestmt(list->items[i]);
    bytecodeemit(curbc, opreturn, 0);
}

static void compileexpr(expr *e) {
    if (((binaryexpr*)e)->op) {
        binaryexpr *b = (binaryexpr*)e;
        compileexpr(b->left);
        compileexpr(b->right);
        switch (b->op) {
            case TOKPLUS: bytecodeemit(curbc, opadd, e->line); break;
            case TOKMINUS: bytecodeemit(curbc, opsub, e->line); break;
            case TOKSTAR: bytecodeemit(curbc, opmul, e->line); break;
            case TOKSLASH: bytecodeemit(curbc, opdiv, e->line); break;
            default: throwexception("unsupported op");
        }
        return;
    }
    if (((identexpr*)e)->name) {
        identexpr *i = (identexpr*)e;
        int idx = bytecodeemitname(curbc, i->name, e->line);
        bytecodeemit(curbc, oploadvar, e->line);
        bytecodeemitint(curbc, idx, e->line);
        return;
    }
    if (((numberexpr*)e)->value) {
        numberexpr *n = (numberexpr*)e;
        double val = n->value;
        if (val == (double)(long)val) {
            bytecodeemitconst(curbc, makeint((long)val), e->line);
        } else {
            bytecodeemitconst(curbc, makefloat(val), e->line);
        }
        return;
    }
    if (((stringexpr*)e)->value) {
        stringexpr *s = (stringexpr*)e;
        bytecodeemitconst(curbc, makestring(s->value), e->line);
        return;
    }
    bytecodeemitconst(curbc, makenone(), e->line);
}

static void compilestmt(stmt *s) {
    if (((letexpr*)s)->name) {
        letexpr *l = (letexpr*)s;
        compileexpr(l->value);
        int idx = bytecodeemitname(curbc, l->name, s->line);
        bytecodeemit(curbc, opstorevar, s->line);
        bytecodeemitint(curbc, idx, s->line);
        return;
    }
    if (((returnexpr*)s)->value != NULL || ((returnexpr*)s)->value == NULL) {
        returnexpr *r = (returnexpr*)s;
        if (r->value) compileexpr(r->value);
        else bytecodeemitconst(curbc, makenone(), s->line);
        bytecodeemit(curbc, opreturn, s->line);
        return;
    }
    if (((exprstmt*)s)->expression) {
        exprstmt *e = (exprstmt*)s;
        compileexpr(e->expression);
        bytecodeemit(curbc, oppop, s->line);
        return;
    }
    bytecodeemit(curbc, oppop, s->line);
}
