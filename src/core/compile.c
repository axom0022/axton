#include "axton.h"
#include "bytecode.h"
#include <stdlib.h>
#include <string.h>

typedef struct cscope {
    char **names;
    int *slots;
    unsigned char *cnsts;
    int count;
    int cap;
} cscope;

typedef struct cs {
    bytecode *bc;
    int istop;
    cscope *scopes;
    int scopec;
    int scopemax;
    int nextslot;
    int line;
    int usesthis;
} cs;

static void compilestmt(cs *c, stmt *s);
static void compileexpr(cs *c, expr *e);

static void scopepush(cs *c) {
    if (c->scopec >= c->scopemax) {
        c->scopemax = c->scopemax ? c->scopemax * 2 : 8;
        c->scopes = realloc(c->scopes, c->scopemax * sizeof(cscope));
    }
    cscope *s = &c->scopes[c->scopec++];
    s->names = NULL;
    s->slots = NULL;
    s->cnsts = NULL;
    s->count = 0;
    s->cap = 0;
}

static void scopepop(cs *c) {
    cscope *s = &c->scopes[--c->scopec];
    free(s->names);
    free(s->slots);
    free(s->cnsts);
}

static int allocslot(cs *c, char *name, int isconst) {
    int slot = c->nextslot++;
    bytecode *bc = c->bc;
    bc->slotnames = realloc(bc->slotnames, c->nextslot * sizeof(char*));
    bc->slotnames[slot] = strdup(name);
    bc->slotconst = realloc(bc->slotconst, c->nextslot);
    bc->slotconst[slot] = (unsigned char)isconst;
    return slot;
}

static void declarelocal(cs *c, char *name, int slot, int isconst) {
    cscope *s = &c->scopes[c->scopec - 1];
    if (s->count >= s->cap) {
        s->cap = s->cap ? s->cap * 2 : 8;
        s->names = realloc(s->names, s->cap * sizeof(char*));
        s->slots = realloc(s->slots, s->cap * sizeof(int));
        s->cnsts = realloc(s->cnsts, s->cap);
    }
    s->names[s->count] = name;
    s->slots[s->count] = slot;
    s->cnsts[s->count] = (unsigned char)isconst;
    s->count++;
}

static int findlocal(cs *c, char *name, int *slot, int *isconst) {
    for (int i = c->scopec - 1; i >= 0; i--) {
        cscope *s = &c->scopes[i];
        for (int j = 0; j < s->count; j++) {
            if (strcmp(s->names[j], name) == 0) {
                *slot = s->slots[j];
                *isconst = s->cnsts[j];
                return 1;
            }
        }
    }
    return 0;
}

static int auxaddstmt(cs *c, stmt *s) {
    bytecode *bc = c->bc;
    bc->auxstmts = realloc(bc->auxstmts, (bc->auxcount + 1) * sizeof(stmt*));
    bc->auxstmts[bc->auxcount] = s;
    return bc->auxcount++;
}

static int auxaddfunc(cs *c, fnexp *f) {
    bytecode *bc = c->bc;
    bc->auxfuncs = realloc(bc->auxfuncs, (bc->auxfcount + 1) * sizeof(fnexp*));
    bc->auxfuncs[bc->auxfcount] = f;
    return bc->auxfcount++;
}

static void emit(cs *c, int op) {
    bytecodeemit(c->bc, (unsigned char)op, c->line);
}

static void emiti(cs *c, int op, int v) {
    bytecodeemit(c->bc, (unsigned char)op, c->line);
    bytecodeemitint(c->bc, v, c->line);
}

static int emitj(cs *c, int op) {
    bytecodeemit(c->bc, (unsigned char)op, c->line);
    int pos = c->bc->size;
    bytecodeemitint(c->bc, 0, c->line);
    return pos;
}

static void patchj(cs *c, int pos) {
    int off = c->bc->size - (pos + 4);
    unsigned char *p = c->bc->code + pos;
    p[0] = (unsigned char)((off >> 24) & 0xFF);
    p[1] = (unsigned char)((off >> 16) & 0xFF);
    p[2] = (unsigned char)((off >> 8) & 0xFF);
    p[3] = (unsigned char)(off & 0xFF);
}

static void emitjto(cs *c, int op, int target) {
    bytecodeemit(c->bc, (unsigned char)op, c->line);
    int pos = c->bc->size;
    bytecodeemitint(c->bc, 0, c->line);
    int off = target - (pos + 4);
    unsigned char *p = c->bc->code + pos;
    p[0] = (unsigned char)((off >> 24) & 0xFF);
    p[1] = (unsigned char)((off >> 16) & 0xFF);
    p[2] = (unsigned char)((off >> 8) & 0xFF);
    p[3] = (unsigned char)(off & 0xFF);
}

static void compilestmts(cs *c, stmtlist *list) {
    if (!list) return;
    for (int i = 0; i < list->count; i++) {
        compilestmt(c, list->items[i]);
    }
}

static void compileident(cs *c, char *name) {
    int slot, isconst;
    if (strcmp(name, "this") == 0) c->usesthis = 1;
    if (findlocal(c, name, &slot, &isconst)) {
        emiti(c, oploadlocal, slot);
    } else {
        emiti(c, oploadupvalue, bytecodeemitname(c->bc, name, c->line));
    }
}

static int localident(cs *c, expr *e, int *slot) {
    if (e->type != exprident) return 0;
    int isconst;
    if (!findlocal(c, ((identexpr*)e)->name, slot, &isconst)) return 0;
    return 1;
}

static int intlit(expr *e, int *v) {
    if (e->type != exprnumber) return 0;
    double d = ((numberexpr*)e)->value;
    if (d != (double)(long)d) return 0;
    if (d < -2147483648.0 || d > 2147483647.0) return 0;
    *v = (int)(long)d;
    return 1;
}

static int scancapexpr(expr *e);

static int scancapcallargs(exprlist *args) {
    if (!args) return 0;
    for (int i = 0; i < args->count; i++) {
        if (scancapexpr(args->items[i])) return 1;
    }
    return 0;
}

static int scancapexpr(expr *e) {
    if (!e) return 0;
    switch (e->type) {
        case exprbinary: {
            binaryexpr *b = (binaryexpr*)e;
            return scancapexpr(b->left) || scancapexpr(b->right);
        }
        case exprunary: return scancapexpr(((unaryexpr*)e)->operand);
        case exprcall: {
            callexpr *cl = (callexpr*)e;
            if (cl->callee->type == exprident) {
                char *n = ((identexpr*)cl->callee)->name;
                if (strcmp(n, "eval") == 0 || strcmp(n, "exec") == 0) return 1;
            }
            if (scancapexpr(cl->callee)) return 1;
            return scancapcallargs(cl->args);
        }
        case exprindex: {
            indexexpr *ix = (indexexpr*)e;
            return scancapexpr(ix->target) || scancapexpr(ix->index);
        }
        case exprattr: return scancapexpr(((attribexpr*)e)->target);
        case exprlistlit: {
            listexpr *le = (listexpr*)e;
            for (int i = 0; i < le->count; i++) if (scancapexpr(le->items[i])) return 1;
            return 0;
        }
        case exprdict: {
            dictexpr *de = (dictexpr*)e;
            for (int i = 0; i < de->count; i++) {
                if (scancapexpr(de->keys[i]) || scancapexpr(de->values[i])) return 1;
            }
            return 0;
        }
        case exprfn: return 1;
        case exprclass: return 1;
        case exprassign: {
            assignexpr *a = (assignexpr*)e;
            return scancapexpr(a->target) || scancapexpr(a->value);
        }
        default: return 0;
    }
}

static int scancapstmts(stmtlist *list);

static int scancapstmt(stmt *s) {
    if (!s) return 0;
    switch (s->type) {
        case stmtlet: return scancapexpr(((letexpr*)s)->value);
        case stmtreturn: return scancapexpr(((returnexpr*)s)->value);
        case stmtif: {
            ifexpr *f = (ifexpr*)s;
            if (scancapexpr(f->cond) || scancapstmts(f->body)) return 1;
            for (int i = 0; i < f->elifcount; i++) {
                if (scancapexpr(f->elifs[i].cond) || scancapstmts(f->elifs[i].body)) return 1;
            }
            if (f->elsebody && scancapstmts(f->elsebody)) return 1;
            return 0;
        }
        case stmtwhile: {
            whileexpr *w = (whileexpr*)s;
            return scancapexpr(w->cond) || scancapstmts(w->body);
        }
        case stmtfor: {
            forexp *f = (forexp*)s;
            return scancapexpr(f->iter) || scancapstmts(f->body);
        }
        case stmtfn: return 1;
        case stmtclass: return 1;
        case stmtexpr: return scancapexpr(((exprstmt*)s)->expression);
        case stmttry: return 1;
        case stmtthrow: return scancapexpr(((throwexpr*)s)->value);
        default: return 0;
    }
}

static int scancapstmts(stmtlist *list) {
    if (!list) return 0;
    for (int i = 0; i < list->count; i++) {
        if (scancapstmt(list->items[i])) return 1;
    }
    return 0;
}

static void compilebinop(cs *c, expr *e) {
    binaryexpr *b = (binaryexpr*)e;
    c->line = b->base.node.line;
    int s1, s2, iv;
    if (b->op == TOKPLUS && localident(c, b->left, &s1) && localident(c, b->right, &s2)) {
        bytecodeemit(c->bc, (unsigned char)opaddloc, c->line);
        bytecodeemitint(c->bc, s1, c->line);
        bytecodeemitint(c->bc, s2, c->line);
        return;
    }
    if (b->op == TOKMINUS && localident(c, b->left, &s1) && intlit(b->right, &iv)) {
        emiti(c, opsublocint, s1);
        bytecodeemitint(c->bc, iv, c->line);
        return;
    }
    if (b->op == TOKLT && localident(c, b->left, &s1) && intlit(b->right, &iv)) {
        emiti(c, oplltlocint, s1);
        bytecodeemitint(c->bc, iv, c->line);
        return;
    }
    int op = opnop;
    switch (b->op) {
        case TOKPLUS: op = opadd; break;
        case TOKMINUS: op = opsub; break;
        case TOKSTAR: op = opmul; break;
        case TOKSLASH: op = opdiv; break;
        case TOKPERCENT: op = opmod; break;
        case TOKPOWER: op = oppow; break;
        case TOKEQEQ: op = opeq; break;
        case TOKNE: op = opne; break;
        case TOKLT: op = oplt; break;
        case TOKGT: op = opgt; break;
        case TOKLE: op = ople; break;
        case TOKGE: op = opge; break;
        default:
            compileexpr(c, b->left);
            compileexpr(c, b->right);
            emiti(c, oploadconst, bytecodeaddconst(c->bc, makestring("unknown binary operator")));
            emit(c, opthrow);
            return;
    }
    compileexpr(c, b->left);
    compileexpr(c, b->right);
    emit(c, op);
}

static void compileassign(cs *c, assignexpr *a) {
    expr *t = a->target;
    if (t->type == exprident) {
        char *name = ((identexpr*)t)->name;
        int slot, isconst;
        int dslot = -1;
        if (strcmp(name, "this") == 0) c->usesthis = 1;
        int s1, s2;
        if (findlocal(c, name, &slot, &isconst)) dslot = slot;
        binaryexpr *bv = NULL;
        if (a->value->type == exprbinary) bv = (binaryexpr*)a->value;
        if (dslot >= 0 && bv && bv->op == TOKPLUS &&
            localident(c, bv->left, &s1) && localident(c, bv->right, &s2)) {
            bytecodeemit(c->bc, (unsigned char)opadd3, c->line);
            bytecodeemitint(c->bc, dslot, c->line);
            bytecodeemitint(c->bc, s1, c->line);
            bytecodeemitint(c->bc, s2, c->line);
            return;
        }
        compileexpr(c, a->value);
        emit(c, opdup);
        if (dslot >= 0) {
            emiti(c, isconst ? opstorelocalc : opstorelocal, dslot);
        } else {
            emiti(c, opstoreupvalue, bytecodeemitname(c->bc, name, c->line));
        }
        return;
    }
    if (t->type == exprattr) {
        attribexpr *at = (attribexpr*)t;
        compileexpr(c, a->value);
        compileexpr(c, at->target);
        emiti(c, opsetattr, bytecodeemitname(c->bc, at->attr, c->line));
        return;
    }
    if (t->type == exprindex) {
        indexexpr *ix = (indexexpr*)t;
        compileexpr(c, a->value);
        compileexpr(c, ix->target);
        compileexpr(c, ix->index);
        emit(c, opsetindex);
        return;
    }
    compileexpr(c, a->value);
    emiti(c, oploadconst, bytecodeaddconst(c->bc, makestring("invalid assignment target")));
    emit(c, opthrow);
}

static void compileexpr(cs *c, expr *e) {
    c->line = e->node.line;
    switch (e->type) {
        case exprbinary: {
            binaryexpr *b = (binaryexpr*)e;
            if (b->op == TOKAND) {
                compileexpr(c, b->left);
                emit(c, opdup);
                int j = emitj(c, opjumpiffalse);
                compileexpr(c, b->right);
                emit(c, oppop);
                patchj(c, j);
                return;
            }
            if (b->op == TOKOR) {
                compileexpr(c, b->left);
                emit(c, opdup);
                int j = emitj(c, opjumpiftrue);
                compileexpr(c, b->right);
                emit(c, oppop);
                patchj(c, j);
                return;
            }
            compilebinop(c, e);
            return;
        }
        case exprunary: {
            unaryexpr *u = (unaryexpr*)e;
            compileexpr(c, u->operand);
            if (u->op == TOKNOT) emit(c, opnot);
            else emit(c, opneg);
            return;
        }
        case exprident: compileident(c, ((identexpr*)e)->name); return;
        case exprnumber: {
            numberexpr *n = (numberexpr*)e;
            double v = n->value;
            if (v == (double)(long)v && v >= -2147483648.0 && v <= 2147483647.0) {
                emiti(c, oploadint, (int)(long)v);
            } else {
                emiti(c, oploadconst, bytecodeaddconst(c->bc, makefloat(v)));
            }
            return;
        }
        case exprstring:
            emiti(c, oploadconst, bytecodeaddconst(c->bc, makestring(((stringexpr*)e)->value)));
            return;
        case exprbool: emiti(c, oploadbool, ((boolexpr*)e)->value); return;
        case exprnone: emit(c, oploadnone); return;
        case exprcall: {
            callexpr *cl = (callexpr*)e;
            if (cl->callee->type == exprattr) {
                attribexpr *at = (attribexpr*)cl->callee;
                compileexpr(c, at->target);
                for (int i = 0; i < cl->args->count; i++) compileexpr(c, cl->args->items[i]);
                bytecodeemit(c->bc, (unsigned char)opcallattr, c->line);
                bytecodeemitint(c->bc, bytecodeemitname(c->bc, at->attr, c->line), c->line);
                bytecodeemitint(c->bc, cl->args->count, c->line);
                return;
            }
            compileexpr(c, cl->callee);
            for (int i = 0; i < cl->args->count; i++) compileexpr(c, cl->args->items[i]);
            emiti(c, opcall, cl->args->count);
            return;
        }
        case exprindex: {
            indexexpr *ix = (indexexpr*)e;
            compileexpr(c, ix->target);
            compileexpr(c, ix->index);
            emit(c, opindex);
            return;
        }
        case exprattr:
            compileexpr(c, ((attribexpr*)e)->target);
            emiti(c, opgetattr, bytecodeemitname(c->bc, ((attribexpr*)e)->attr, c->line));
            return;
        case exprlistlit: {
            listexpr *le = (listexpr*)e;
            for (int i = 0; i < le->count; i++) compileexpr(c, le->items[i]);
            emiti(c, opmakelist, le->count);
            return;
        }
        case exprdict: {
            dictexpr *de = (dictexpr*)e;
            for (int i = 0; i < de->count; i++) {
                compileexpr(c, de->keys[i]);
                compileexpr(c, de->values[i]);
            }
            emiti(c, opmakedict, de->count);
            return;
        }
        case exprfn:
            emiti(c, opmakefunc, auxaddfunc(c, ((fnextexpr*)e)->fn));
            return;
        case exprclass:
            emiti(c, opclassexpr, auxaddstmt(c, (stmt*)((classexpr*)e)->cls));
            return;
        case exprassign: compileassign(c, (assignexpr*)e); return;
    }
    emiti(c, oploadconst, bytecodeaddconst(c->bc, makestring("unknown expression")));
    emit(c, opthrow);
}

static void compilestmt(cs *c, stmt *s) {
    c->line = s->node.line;
    switch (s->type) {
        case stmtlet: {
            letexpr *l = (letexpr*)s;
            compileexpr(c, l->value);
            int slot = allocslot(c, l->name, l->isconst);
            declarelocal(c, l->name, slot, l->isconst);
            emiti(c, opstorelocal, slot);
            return;
        }
        case stmtreturn: {
            returnexpr *r = (returnexpr*)s;
            if (r->value) compileexpr(c, r->value);
            else emit(c, oploadnone);
            emit(c, opreturn);
            return;
        }
        case stmtif: {
            ifexpr *f = (ifexpr*)s;
            int ends[64];
            int endn = 0;
            compileexpr(c, f->cond);
            int j = emitj(c, opjumpiffalse);
            scopepush(c);
            compilestmts(c, f->body);
            scopepop(c);
            if (endn < 64) ends[endn++] = emitj(c, opjump);
            for (int i = 0; i < f->elifcount; i++) {
                patchj(c, j);
                compileexpr(c, f->elifs[i].cond);
                j = emitj(c, opjumpiffalse);
                scopepush(c);
                compilestmts(c, f->elifs[i].body);
                scopepop(c);
                if (endn < 64) ends[endn++] = emitj(c, opjump);
            }
            patchj(c, j);
            if (f->elsebody) {
                scopepush(c);
                compilestmts(c, f->elsebody);
                scopepop(c);
            }
            for (int i = 0; i < endn; i++) patchj(c, ends[i]);
            return;
        }
        case stmtwhile: {
            whileexpr *w = (whileexpr*)s;
            int l = c->bc->size;
            compileexpr(c, w->cond);
            int j = emitj(c, opjumpiffalse);
            scopepush(c);
            compilestmts(c, w->body);
            scopepop(c);
            emitjto(c, opjump, l);
            patchj(c, j);
            return;
        }
        case stmtfor: {
            forexp *f = (forexp*)s;
            int rangeargs = -1;
            if (f->iter->type == exprcall) {
                callexpr *rc = (callexpr*)f->iter;
                if (rc->callee->type == exprident &&
                    strcmp(((identexpr*)rc->callee)->name, "range") == 0 &&
                    rc->args->count >= 1 && rc->args->count <= 3) {
                    int rs, rcst;
                    if (!findlocal(c, "range", &rs, &rcst)) rangeargs = rc->args->count;
                }
            }
            if (rangeargs > 0) {
                callexpr *rc = (callexpr*)f->iter;
                if (rangeargs == 1) {
                    emiti(c, oploadint, 0);
                    compileexpr(c, rc->args->items[0]);
                    emiti(c, oploadint, 1);
                } else if (rangeargs == 2) {
                    compileexpr(c, rc->args->items[0]);
                    compileexpr(c, rc->args->items[1]);
                    emiti(c, oploadint, 1);
                } else {
                    compileexpr(c, rc->args->items[0]);
                    compileexpr(c, rc->args->items[1]);
                    compileexpr(c, rc->args->items[2]);
                }
                emit(c, opforrangebegin);
                scopepush(c);
                int slot = allocslot(c, f->var, 0);
                declarelocal(c, f->var, slot, 0);
                int l = c->bc->size;
                bytecodeemit(c->bc, (unsigned char)opforrangenext, c->line);
                bytecodeemitint(c->bc, slot, c->line);
                int p2 = c->bc->size;
                bytecodeemitint(c->bc, 0, c->line);
                compilestmts(c, f->body);
                emitjto(c, opjump, l);
                patchj(c, p2);
                emiti(c, oppopn, 3);
                scopepop(c);
                return;
            }
            compileexpr(c, f->iter);
            emit(c, opforbegin);
            scopepush(c);
            int slot = allocslot(c, f->var, 0);
            declarelocal(c, f->var, slot, 0);
            int l = c->bc->size;
            bytecodeemit(c->bc, (unsigned char)opfornext, c->line);
            bytecodeemitint(c->bc, slot, c->line);
            int p2 = c->bc->size;
            bytecodeemitint(c->bc, 0, c->line);
            compilestmts(c, f->body);
            emitjto(c, opjump, l);
            patchj(c, p2);
            emiti(c, oppopn, 2);
            scopepop(c);
            return;
        }
        case stmtbreak: return;
        case stmtnext: return;
        case stmtfn: {
            fnexp *f = (fnexp*)s;
            int slot = allocslot(c, f->name, 0);
            declarelocal(c, f->name, slot, 0);
            emiti(c, opmakefunc, auxaddfunc(c, f));
            emiti(c, opstorelocal, slot);
            return;
        }
        case stmtclass: {
            classexp *cl = (classexp*)s;
            int slot = allocslot(c, cl->name, 0);
            declarelocal(c, cl->name, slot, 0);
            emiti(c, opclassstmt, auxaddstmt(c, s));
            return;
        }
        case stmtexpr:
            compileexpr(c, ((exprstmt*)s)->expression);
            emit(c, oppop);
            return;
        case stmttry:
            emiti(c, optrystmt, auxaddstmt(c, s));
            return;
        case stmtthrow:
            compileexpr(c, ((throwexpr*)s)->value);
            emit(c, opthrow);
            return;
    }
    emiti(c, oploadconst, bytecodeaddconst(c->bc, makestring("unknown statement")));
    emit(c, opthrow);
}

static bytecode *compileinit(int istop, char **params, int pcount, stmt **body, int bcount) {
    bytecode *bc = bytecodenew();
    cs c;
    memset(&c, 0, sizeof(c));
    c.bc = bc;
    c.istop = istop;
    scopepush(&c);
    int slot = allocslot(&c, "this", 0);
    declarelocal(&c, "this", slot, 0);
    for (int i = 0; i < pcount; i++) {
        int s2 = allocslot(&c, params[i], 0);
        declarelocal(&c, params[i], s2, 0);
    }
    for (int i = 0; i < bcount; i++) {
        compilestmt(&c, body[i]);
    }
    emit(&c, oploadnone);
    emit(&c, opreturn);
    bc->slotcount = c.nextslot;
    bc->usesthis = c.usesthis;
    bc->hascapture = 0;
    for (int i = 0; i < bcount; i++) {
        if (scancapstmt(body[i])) {
            bc->hascapture = 1;
            break;
        }
    }
    bc->istop = istop;
    for (int i = 0; i < c.scopec; i++) {
        free(c.scopes[i].names);
        free(c.scopes[i].slots);
        free(c.scopes[i].cnsts);
    }
    free(c.scopes);
    return bc;
}

bytecode *axcompilefn(char *name, char **params, int pcount, stmt **body, int bcount) {
    (void)name;
    return compileinit(0, params, pcount, body, bcount);
}

bytecode *axcompileprog(stmt **body, int count) {
    return compileinit(1, NULL, 0, body, count);
}
