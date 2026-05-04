#include "axton.h"
#include "bytecode.h"

static bytecode *currentbc = NULL;

static void compileexpr(expr *e);
static void compilestmt(stmt *s);

void compileprogram(stmt *prog, bytecode *bc) {
    currentbc = bc;
    stmtlist *list = (stmtlist*)prog;
    for (int i = 0; i < list->count; i++) compilestmt(list->items[i]);
    bytecodeemit(currentbc, OPRETURN, 0);
}

static void compileexpr(expr *e) {
    if (((binaryexpr*)e)->op) {
        binaryexpr *b = (binaryexpr*)e;
        compileexpr(b->left);
        compileexpr(b->right);
        switch (b->op) {
            case TOKPLUS: bytecodeemit(currentbc, OPADD, e->line); break;
            case TOKMINUS: bytecodeemit(currentbc, OPSUB, e->line); break;
            case TOKSTAR: bytecodeemit(currentbc, OPMUL, e->line); break;
            case TOKSLASH: bytecodeemit(currentbc, OPDIV, e->line); break;
            default: throwexception("unsupported op in compiler");
        }
        return;
    }
    if (((identexpr*)e)->name) {
        identexpr *i = (identexpr*)e;
        int idx = bytecodeemitname(currentbc, i->name, e->line);
        bytecodeemit(currentbc, OPLOADVAR, e->line);
        bytecodeemitint(currentbc, idx, e->line);
        return;
    }
    if (((numberexpr*)e)->value) {
        numberexpr *n = (numberexpr*)e;
        double val = n->value;
        if (val == (double)(long)val) {
            bytecodeemitconst(currentbc, makeint((long)val), e->line);
        } else {
            bytecodeemitconst(currentbc, makefloat(val), e->line);
        }
        return;
    }
    if (((stringexpr*)e)->value) {
        stringexpr *s = (stringexpr*)e;
        bytecodeemitconst(currentbc, makestring(s->value), e->line);
        return;
    }
    
    bytecodeemitconst(currentbc, makenone(), e->line);
}

static void compilestmt(stmt *s) {
    if (((letexpr*)s)->name) {
        letexpr *l = (letexpr*)s;
        compileexpr(l->value);
        int idx = bytecodeemitname(currentbc, l->name, s->line);
        bytecodeemit(currentbc, OPSTOREVAR, s->line);
        bytecodeemitint(currentbc, idx, s->line);
        return;
    }
    if (((returnexpr*)s)->value != NULL || ((returnexpr*)s)->value == NULL) {
        returnexpr *r = (returnexpr*)s;
        if (r->value) compileexpr(r->value);
        else bytecodeemitconst(currentbc, makenone(), s->line);
        bytecodeemit(currentbc, OPRETURN, s->line);
        return;
    }
    
    if (((exprstmt*)s)->expression) {
        exprstmt *e = (exprstmt*)s;
        compileexpr(e->expression);
        bytecodeemit(currentbc, OPPOP, s->line);
        return;
    }
}
