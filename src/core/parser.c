#include "axton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    token *tokens;
    int pos;
    int count;
} parser;

static token peektoken(parser *p) {
    if (p->pos >= p->count) {
        token t = {TOKEOF, NULL, 0, 0};
        return t;
    }
    return p->tokens[p->pos];
}

static token nexttoken(parser *p) {
    if (p->pos >= p->count) {
        token t = {TOKEOF, NULL, 0, 0};
        return t;
    }
    return p->tokens[p->pos++];
}

static void expect(parser *p, toktype type) {
    token t = nexttoken(p);
    if (t.type != type) {
        char msg[256];
        snprintf(msg, sizeof(msg), "line %d expected token %d got %d", t.line, type, t.type);
        throwexception(msg);
    }
}

static expr *parseexpr(parser *p);
static stmt *parsestmt(parser *p);
static stmtlist *parseblock(parser *p);
static int parseblockstart(parser *p);
static void parseblockend(parser *p, int braced);

static expr *parseprimary(parser *p) {
    token t = peektoken(p);
    if (t.type == TOKIDENT) {
        nexttoken(p);
        identexpr *e = malloc(sizeof(identexpr));
        e->base.type = exprident;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->name = strdup(t.text);
        expr *cur = (expr*)e;
        
        while (peektoken(p).type == TOKLPAREN || peektoken(p).type == TOKDOT || peektoken(p).type == TOKLBRACKET) {
            if (peektoken(p).type == TOKLPAREN) {
                nexttoken(p);
                callexpr *call = malloc(sizeof(callexpr));
                call->base.type = exprcall;
                call->base.eval = NULL;
                call->base.node.line = t.line;
                call->callee = cur;
                call->args = malloc(sizeof(exprlist));
                call->args->items = NULL;
                call->args->count = 0;
                if (peektoken(p).type != TOKRPAREN) {
                    while (1) {
                        call->args->count++;
                        call->args->items = realloc(call->args->items, sizeof(expr*) * call->args->count);
                        call->args->items[call->args->count - 1] = parseexpr(p);
                        if (peektoken(p).type == TOKCOMMA) nexttoken(p);
                        else break;
                    }
                }
                expect(p, TOKRPAREN);
                cur = (expr*)call;
            } else if (peektoken(p).type == TOKDOT) {
                nexttoken(p);
                attribexpr *attr = malloc(sizeof(attribexpr));
                attr->base.type = exprattr;
                attr->base.eval = NULL;
                attr->base.node.line = t.line;
                attr->target = cur;
                token name = peektoken(p);
                expect(p, TOKIDENT);
                attr->attr = strdup(name.text);
                cur = (expr*)attr;
            } else {
                nexttoken(p);
                indexexpr *idx = malloc(sizeof(indexexpr));
                idx->base.type = exprindex;
                idx->base.eval = NULL;
                idx->base.node.line = t.line;
                idx->target = cur;
                idx->index = parseexpr(p);
                expect(p, TOKRBRACKET);
                cur = (expr*)idx;
            }
        }
        return cur;
    }
    if (t.type == TOKNUMBER) {
        nexttoken(p);
        numberexpr *e = malloc(sizeof(numberexpr));
        e->base.type = exprnumber;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = atof(t.text);
        return (expr*)e;
    }
    if (t.type == TOKSTRING) {
        nexttoken(p);
        stringexpr *e = malloc(sizeof(stringexpr));
        e->base.type = exprstring;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = strdup(t.text);
        return (expr*)e;
    }
    if (t.type == TOKTRUE) {
        nexttoken(p);
        boolexpr *e = malloc(sizeof(boolexpr));
        e->base.type = exprbool;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 1;
        return (expr*)e;
    }
    if (t.type == TOKFALSE) {
        nexttoken(p);
        boolexpr *e = malloc(sizeof(boolexpr));
        e->base.type = exprbool;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 0;
        return (expr*)e;
    }
    if (t.type == TOKNONE) {
        nexttoken(p);
        noneexpr *e = malloc(sizeof(noneexpr));
        e->base.type = exprnone;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        return (expr*)e;
    }
    if (t.type == TOKLPAREN) {
        nexttoken(p);
        expr *e = parseexpr(p);
        expect(p, TOKRPAREN);
        return e;
    }
    if (t.type == TOKLBRACKET) {
        nexttoken(p);
        listexpr *e = malloc(sizeof(listexpr));
        e->base.type = exprlistlit;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->items = NULL;
        e->count = 0;
        if (peektoken(p).type != TOKRBRACKET) {
            while (1) {
                e->count++;
                e->items = realloc(e->items, sizeof(expr*) * e->count);
                e->items[e->count - 1] = parseexpr(p);
                if (peektoken(p).type == TOKCOMMA) nexttoken(p);
                else break;
            }
        }
        expect(p, TOKRBRACKET);
        return (expr*)e;
    }
    if (t.type == TOKLBRACE) {
        nexttoken(p);
        dictexpr *e = malloc(sizeof(dictexpr));
        e->base.type = exprdict;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->keys = NULL;
        e->values = NULL;
        e->count = 0;
        if (peektoken(p).type != TOKRBRACE) {
            while (1) {
                e->count++;
                e->keys = realloc(e->keys, sizeof(expr*) * e->count);
                e->values = realloc(e->values, sizeof(expr*) * e->count);
                e->keys[e->count - 1] = parseexpr(p);
                expect(p, TOKCOLON);
                e->values[e->count - 1] = parseexpr(p);
                if (peektoken(p).type == TOKCOMMA) nexttoken(p);
                else break;
            }
        }
        expect(p, TOKRBRACE);
        return (expr*)e;
    }
    if (t.type == TOKFN) {
        nexttoken(p);
        fnexp *fn = malloc(sizeof(fnexp));
        fn->base.type = stmtfn;
        fn->name = NULL;
        fn->params = NULL;
        fn->pcount = 0;
        expect(p, TOKLPAREN);
        if (peektoken(p).type != TOKRPAREN) {
            while (1) {
                fn->pcount++;
                fn->params = realloc(fn->params, sizeof(char*) * fn->pcount);
                token param = peektoken(p);
                expect(p, TOKIDENT);
                fn->params[fn->pcount - 1] = strdup(param.text);
                if (peektoken(p).type == TOKCOMMA) nexttoken(p);
                else break;
            }
        }
        expect(p, TOKRPAREN);
        int braced = parseblockstart(p);
        fn->body = parseblock(p);
        parseblockend(p, braced);
        fn->base.exec = NULL;
        fn->base.node.line = t.line;
        fnextexpr *e = malloc(sizeof(fnextexpr));
        e->base.type = exprfn;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->fn = fn;
        return (expr*)e;
    }
    if (t.type == TOKCLASS) {
        nexttoken(p);
        classexp *cls = malloc(sizeof(classexp));
        cls->base.type = stmtclass;
        cls->name = NULL;
        if (peektoken(p).type == TOKIDENT) {
            token nm = peektoken(p);
            nexttoken(p);
            cls->name = strdup(nm.text);
        }
        int braced = parseblockstart(p);
        cls->body = parseblock(p);
        parseblockend(p, braced);
        cls->base.exec = NULL;
        cls->base.node.line = t.line;
        classexpr *e = malloc(sizeof(classexpr));
        e->base.type = exprclass;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->cls = cls;
        return (expr*)e;
    }
    char msg[256];
    snprintf(msg, sizeof(msg), "unexpected token at line %d", t.line);
    throwexception(msg);
    return NULL;
}

static expr *parseunary(parser *p) {
    token t = peektoken(p);
    if (t.type == TOKMINUS || t.type == TOKNOT) {
        nexttoken(p);
        unaryexpr *e = malloc(sizeof(unaryexpr));
        e->base.type = exprunary;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->op = t.type;
        e->operand = parseunary(p);
        return (expr*)e;
    }
    return parseprimary(p);
}

static expr *parsepower(parser *p) {
    expr *left = parseunary(p);
    while (peektoken(p).type == TOKPOWER) {
        token t = peektoken(p);
        nexttoken(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.type = exprbinary;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->left = left;
        e->op = t.type;
        e->right = parseunary(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parsemul(parser *p) {
    expr *left = parsepower(p);
    while (1) {
        token t = peektoken(p);
        if (t.type == TOKSTAR || t.type == TOKSLASH || t.type == TOKPERCENT) {
            nexttoken(p);
            binaryexpr *e = malloc(sizeof(binaryexpr));
            e->base.type = exprbinary;
            e->base.eval = NULL;
            e->base.node.line = t.line;
            e->left = left;
            e->op = t.type;
            e->right = parsepower(p);
            left = (expr*)e;
        } else break;
    }
    return left;
}

static expr *parseadd(parser *p) {
    expr *left = parsemul(p);
    while (1) {
        token t = peektoken(p);
        if (t.type == TOKPLUS || t.type == TOKMINUS) {
            nexttoken(p);
            binaryexpr *e = malloc(sizeof(binaryexpr));
            e->base.type = exprbinary;
            e->base.eval = NULL;
            e->base.node.line = t.line;
            e->left = left;
            e->op = t.type;
            e->right = parsemul(p);
            left = (expr*)e;
        } else break;
    }
    return left;
}

static expr *parsecompare(parser *p) {
    expr *left = parseadd(p);
    while (1) {
        token t = peektoken(p);
        if (t.type == TOKEQEQ || t.type == TOKNE || t.type == TOKLT ||
            t.type == TOKGT || t.type == TOKLE || t.type == TOKGE) {
            nexttoken(p);
            binaryexpr *e = malloc(sizeof(binaryexpr));
            e->base.type = exprbinary;
            e->base.eval = NULL;
            e->base.node.line = t.line;
            e->left = left;
            e->op = t.type;
            e->right = parseadd(p);
            left = (expr*)e;
        } else break;
    }
    return left;
}

static expr *parseand(parser *p) {
    expr *left = parsecompare(p);
    while (peektoken(p).type == TOKAND) {
        nexttoken(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.type = exprbinary;
        e->base.eval = NULL;
        e->base.node.line = left->node.line;
        e->left = left;
        e->op = TOKAND;
        e->right = parsecompare(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parseor(parser *p) {
    expr *left = parseand(p);
    while (peektoken(p).type == TOKOR) {
        nexttoken(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.type = exprbinary;
        e->base.eval = NULL;
        e->base.node.line = left->node.line;
        e->left = left;
        e->op = TOKOR;
        e->right = parseand(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parseexpr(parser *p) {
    return parseor(p);
}



static int parseblockstart(parser *p) {
    if (peektoken(p).type == TOKLBRACE) {
        nexttoken(p);
        return 1;
    }
    expect(p, TOKCOLON);
    return 0;
}

static void parseblockend(parser *p, int braced) {
    if (braced) expect(p, TOKRBRACE);
}

static stmtlist *parseblock(parser *p) {
    stmtlist *block = malloc(sizeof(stmtlist));
    block->items = NULL;
    block->count = 0;
    
    while (peektoken(p).type == TOKINDENT) nexttoken(p);
    while (peektoken(p).type != TOKDEDENT && peektoken(p).type != TOKRBRACE && peektoken(p).type != TOKEOF) {
        block->count++;
        block->items = realloc(block->items, sizeof(stmt*) * block->count);
        block->items[block->count - 1] = parsestmt(p);
    }
    if (peektoken(p).type == TOKDEDENT) nexttoken(p);
    return block;
}

static stmt *parselet(parser *p) {
    nexttoken(p);
    token name = peektoken(p);
    expect(p, TOKIDENT);
    expr *target = NULL;
    

    if (peektoken(p).type != TOKEQ) {
        identexpr *e = malloc(sizeof(identexpr));
        e->base.type = exprident;
        e->base.eval = NULL;
        e->base.node.line = name.line;
        e->name = strdup(name.text);
        target = (expr*)e;
        while (peektoken(p).type == TOKDOT || peektoken(p).type == TOKLBRACKET) {
            if (peektoken(p).type == TOKDOT) {
                nexttoken(p);
                attribexpr *attr = malloc(sizeof(attribexpr));
                attr->base.type = exprattr;
                attr->base.eval = NULL;
                attr->base.node.line = name.line;
                attr->target = target;
                token an = peektoken(p);
                expect(p, TOKIDENT);
                attr->attr = strdup(an.text);
                target = (expr*)attr;
            } else {
                nexttoken(p);
                indexexpr *idx = malloc(sizeof(indexexpr));
                idx->base.type = exprindex;
                idx->base.eval = NULL;
                idx->base.node.line = name.line;
                idx->target = target;
                idx->index = parseexpr(p);
                expect(p, TOKRBRACKET);
                target = (expr*)idx;
            }
        }
    }
    expect(p, TOKEQ);
    expr *value = parseexpr(p);
    if (!target) {
        letexpr *s = malloc(sizeof(letexpr));
        s->base.type = stmtlet;
        s->name = strdup(name.text);
        s->value = value;
        s->isconst = 0;
        s->base.exec = NULL;
        s->base.node.line = name.line;
        return (stmt*)s;
    }
    assignexpr *a = malloc(sizeof(assignexpr));
    a->base.type = exprassign;
    a->base.eval = NULL;
    a->base.node.line = name.line;
    a->target = target;
    a->value = value;
    exprstmt *s = malloc(sizeof(exprstmt));
    s->base.type = stmtexpr;
    s->expression = (expr*)a;
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parseconst(parser *p) {
    nexttoken(p);
    letexpr *s = malloc(sizeof(letexpr));
    s->base.type = stmtlet;
    token name = peektoken(p);
    expect(p, TOKIDENT);
    s->name = strdup(name.text);
    expect(p, TOKEQ);
    s->value = parseexpr(p);
    s->isconst = 1;
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parsereturn(parser *p) {
    nexttoken(p);
    returnexpr *s = malloc(sizeof(returnexpr));
    s->base.type = stmtreturn;
    s->base.node.line = peektoken(p).line;
    s->value = NULL;
    if (peektoken(p).type != TOKNEWLINE && peektoken(p).type != TOKDEDENT && peektoken(p).type != TOKEOF) {
        s->value = parseexpr(p);
    }
    s->base.exec = NULL;
    return (stmt*)s;
}

static stmt *parseif(parser *p) {
    nexttoken(p);
    ifexpr *s = malloc(sizeof(ifexpr));
    s->base.type = stmtif;
    s->cond = parseexpr(p);
    int braced = parseblockstart(p);
    s->body = parseblock(p);
    parseblockend(p, braced);
    s->elifs = NULL;
    s->elifcount = 0;
    s->elsebody = NULL;
    while (peektoken(p).type == TOKELIF) {
        nexttoken(p);
        s->elifcount++;
        s->elifs = realloc(s->elifs, sizeof(s->elifs[0]) * s->elifcount);
        s->elifs[s->elifcount - 1].cond = parseexpr(p);
        braced = parseblockstart(p);
        s->elifs[s->elifcount - 1].body = parseblock(p);
        parseblockend(p, braced);
    }
    if (peektoken(p).type == TOKELSE) {
        nexttoken(p);
        braced = parseblockstart(p);
        s->elsebody = parseblock(p);
        parseblockend(p, braced);
    }
    s->base.exec = NULL;
    s->base.node.line = s->cond->node.line;
    return (stmt*)s;
}

static stmt *parsewhile(parser *p) {
    nexttoken(p);
    whileexpr *s = malloc(sizeof(whileexpr));
    s->base.type = stmtwhile;
    s->cond = parseexpr(p);
    int braced = parseblockstart(p);
    s->body = parseblock(p);
    parseblockend(p, braced);
    s->base.exec = NULL;
    s->base.node.line = s->cond->node.line;
    return (stmt*)s;
}

static stmt *parsefor(parser *p) {
    nexttoken(p);
    forexp *s = malloc(sizeof(forexp));
    s->base.type = stmtfor;
    token var = peektoken(p);
    expect(p, TOKIDENT);
    s->var = strdup(var.text);
    expect(p, TOKIN);
    s->iter = parseexpr(p);
    int braced = parseblockstart(p);
    s->body = parseblock(p);
    parseblockend(p, braced);
    s->base.exec = NULL;
    s->base.node.line = var.line;
    return (stmt*)s;
}

static stmt *parsebreak(parser *p) {
    nexttoken(p);
    breakexp *s = malloc(sizeof(breakexp));
    s->base.type = stmtbreak;
    s->base.exec = NULL;
    s->base.node.line = peektoken(p).line;
    return (stmt*)s;
}

static stmt *parsenext(parser *p) {
    nexttoken(p);
    nextexp *s = malloc(sizeof(nextexp));
    s->base.type = stmtnext;
    s->base.exec = NULL;
    s->base.node.line = peektoken(p).line;
    return (stmt*)s;
}

static stmt *parsefn(parser *p) {
    nexttoken(p);
    fnexp *s = malloc(sizeof(fnexp));
    s->base.type = stmtfn;
    token name = peektoken(p);
    expect(p, TOKIDENT);
    s->name = strdup(name.text);
    expect(p, TOKLPAREN);
    s->params = NULL;
    s->pcount = 0;
    if (peektoken(p).type != TOKRPAREN) {
        while (1) {
            s->pcount++;
            s->params = realloc(s->params, sizeof(char*) * s->pcount);
            token param = peektoken(p);
            expect(p, TOKIDENT);
            s->params[s->pcount - 1] = strdup(param.text);
            if (peektoken(p).type == TOKCOMMA) nexttoken(p);
            else break;
        }
    }
    expect(p, TOKRPAREN);
    int braced = parseblockstart(p);
    s->body = parseblock(p);
    parseblockend(p, braced);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parseclass(parser *p) {
    nexttoken(p);
    classexp *s = malloc(sizeof(classexp));
    s->base.type = stmtclass;
    token name = peektoken(p);
    expect(p, TOKIDENT);
    s->name = strdup(name.text);
    if (peektoken(p).type == TOKLPAREN) {
        nexttoken(p);
        if (peektoken(p).type == TOKIDENT) nexttoken(p);
        expect(p, TOKRPAREN);
    }
    int braced = parseblockstart(p);
    s->body = parseblock(p);
    parseblockend(p, braced);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parsethrow(parser *p) {
    nexttoken(p);
    throwexpr *s = malloc(sizeof(throwexpr));
    s->base.type = stmtthrow;
    s->base.node.line = peektoken(p).line;
    s->value = parseexpr(p);
    s->base.exec = NULL;
    return (stmt*)s;
}

static stmt *parsetry(parser *p) {
    nexttoken(p);
    tryexpr *s = malloc(sizeof(tryexpr));
    s->base.type = stmttry;
    s->base.node.line = peektoken(p).line;
    int braced = parseblockstart(p);
    s->body = parseblock(p);
    parseblockend(p, braced);
    s->catchvar = NULL;
    s->catchbody = NULL;
    s->finallybody = NULL;
    if (peektoken(p).type == TOKCATCH) {
        nexttoken(p);
        token var = peektoken(p);
        if (var.type == TOKIDENT) {
            nexttoken(p);
            s->catchvar = strdup(var.text);
        }
        braced = parseblockstart(p);
        s->catchbody = parseblock(p);
        parseblockend(p, braced);
    }
    if (peektoken(p).type == TOKFINALLY) {
        nexttoken(p);
        braced = parseblockstart(p);
        s->finallybody = parseblock(p);
        parseblockend(p, braced);
    }
    s->base.exec = NULL;
    return (stmt*)s;
}

static stmt *parseexprstmt(parser *p) {
    expr *e = parseexpr(p);
    if (peektoken(p).type == TOKEQ) {
        nexttoken(p);
        assignexpr *a = malloc(sizeof(assignexpr));
        a->base.type = exprassign;
        a->base.eval = NULL;
        a->base.node.line = e->node.line;
        a->target = e;
        a->value = parseexpr(p);
        exprstmt *s = malloc(sizeof(exprstmt));
        s->base.type = stmtexpr;
        s->expression = (expr*)a;
        s->base.exec = NULL;
        s->base.node.line = e->node.line;
        return (stmt*)s;
    }
    exprstmt *s = malloc(sizeof(exprstmt));
    s->base.type = stmtexpr;
    s->expression = e;
    s->base.exec = NULL;
    s->base.node.line = e->node.line;
    return (stmt*)s;
}

static stmt *parsestmt(parser *p) {
    
    while (peektoken(p).type == TOKINDENT || peektoken(p).type == TOKDEDENT) nexttoken(p);
    token t = peektoken(p);
    switch (t.type) {
        case TOKLET: return parselet(p);
        case TOKCONST: return parseconst(p);
        case TOKFN: return parsefn(p);
        case TOKCLASS: return parseclass(p);
        case TOKIF: return parseif(p);
        case TOKWHILE: return parsewhile(p);
        case TOKFOR: return parsefor(p);
        case TOKBREAK: return parsebreak(p);
        case TOKNEXT: return parsenext(p);
        case TOKRETURN: return parsereturn(p);
        case TOKTHROW: return parsethrow(p);
        case TOKTRY: return parsetry(p);
        default: return parseexprstmt(p);
    }
}

stmt *parsetokens(token *tokens, int count) {
    parser *p = malloc(sizeof(parser));
    p->tokens = tokens;
    p->pos = 0;
    p->count = count;
    stmtlist *list = malloc(sizeof(stmtlist));
    list->items = NULL;
    list->count = 0;
    while (peektoken(p).type != TOKEOF) {
        if (peektoken(p).type == TOKINDENT || peektoken(p).type == TOKDEDENT) {
            nexttoken(p);
            continue;
        }
        list->count++;
        list->items = realloc(list->items, sizeof(stmt*) * list->count);
        list->items[list->count - 1] = parsestmt(p);
    }
    return (stmt*)list;
}
