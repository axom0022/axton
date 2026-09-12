#include "axton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    token *tokens;
    int pos;
    int count;
} parser;

typedef struct {
    expr **items;
    int count;
} exprlist;

typedef struct {
    stmt **items;
    int count;
} stmtlist;

typedef struct identexpr {
    expr base;
    char *name;
} identexpr;

typedef struct numberexpr {
    expr base;
    double value;
} numberexpr;

typedef struct stringexpr {
    expr base;
    char *value;
} stringexpr;

typedef struct boolexpr {
    expr base;
    int value;
} boolexpr;

typedef struct noneexpr {
    expr base;
} noneexpr;

typedef struct binaryexpr {
    expr base;
    expr *left;
    toktype op;
    expr *right;
} binaryexpr;

typedef struct unaryexpr {
    expr base;
    toktype op;
    expr *operand;
} unaryexpr;

typedef struct callexpr {
    expr base;
    expr *callee;
    exprlist *args;
} callexpr;

typedef struct indexexpr {
    expr base;
    expr *target;
    expr *index;
} indexexpr;

typedef struct attribexpr {
    expr base;
    expr *target;
    char *attr;
} attribexpr;

typedef struct listexpr {
    expr base;
    expr **items;
    int count;
} listexpr;

typedef struct dictexpr {
    expr base;
    expr **keys;
    expr **values;
    int count;
} dictexpr;

typedef struct letexpr {
    stmt base;
    char *name;
    expr *value;
    int isconst;
} letexpr;

typedef struct returnexpr {
    stmt base;
    expr *value;
} returnexpr;

typedef struct ifexpr {
    stmt base;
    expr *cond;
    stmtlist *body;
    struct { expr *cond; stmtlist *body; } *elifs;
    int elifcount;
    stmtlist *elsebody;
} ifexpr;

typedef struct whileexpr {
    stmt base;
    expr *cond;
    stmtlist *body;
} whileexpr;

typedef struct forexp {
    stmt base;
    char *var;
    expr *iter;
    stmtlist *body;
} forexp;

typedef struct breakexp {
    stmt base;
} breakexp;

typedef struct nextexp {
    stmt base;
} nextexp;

typedef struct fnexp {
    stmt base;
    char *name;
    char **params;
    int pcount;
    stmtlist *body;
} fnexp;

typedef struct classexp {
    stmt base;
    char *name;
    stmtlist *body;
} classexp;

typedef struct exprstmt {
    stmt base;
    expr *expression;
} exprstmt;

typedef struct tryexpr {
    stmt base;
    stmtlist *body;
    char *catchvar;
    stmtlist *catchbody;
    stmtlist *finallybody;
} tryexpr;

typedef struct throwexpr {
    stmt base;
    expr *value;
} throwexpr;

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

static expr *parseprimary(parser *p) {
    token t = peektoken(p);
    if (t.type == TOKIDENT) {
        nexttoken(p);
        identexpr *e = malloc(sizeof(identexpr));
        e->base.type = EXPR_IDENT;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->name = strdup(t.text);
        if (peektoken(p).type == TOKLPAREN) {
            nexttoken(p);
            callexpr *call = malloc(sizeof(callexpr));
            call->base.type = EXPR_CALL;
            call->base.eval = NULL;
            call->base.node.line = t.line;
            call->callee = (expr*)e;
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
            return (expr*)call;
        }
        if (peektoken(p).type == TOKDOT) {
            nexttoken(p);
            attribexpr *attr = malloc(sizeof(attribexpr));
            attr->base.type = EXPR_ATTR;
            attr->base.eval = NULL;
            attr->base.node.line = t.line;
            attr->target = (expr*)e;
            token name = peektoken(p);
            expect(p, TOKIDENT);
            attr->attr = strdup(name.text);
            return (expr*)attr;
        }
        if (peektoken(p).type == TOKLBRACKET) {
            nexttoken(p);
            indexexpr *idx = malloc(sizeof(indexexpr));
            idx->base.type = EXPR_INDEX;
            idx->base.eval = NULL;
            idx->base.node.line = t.line;
            idx->target = (expr*)e;
            idx->index = parseexpr(p);
            expect(p, TOKRBRACKET);
            return (expr*)idx;
        }
        return (expr*)e;
    }
    if (t.type == TOKNUMBER) {
        nexttoken(p);
        numberexpr *e = malloc(sizeof(numberexpr));
        e->base.type = EXPR_NUMBER;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = atof(t.text);
        return (expr*)e;
    }
    if (t.type == TOKSTRING) {
        nexttoken(p);
        stringexpr *e = malloc(sizeof(stringexpr));
        e->base.type = EXPR_STRING;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = strdup(t.text);
        return (expr*)e;
    }
    if (t.type == TOKTRUE) {
        nexttoken(p);
        boolexpr *e = malloc(sizeof(boolexpr));
        e->base.type = EXPR_BOOL;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 1;
        return (expr*)e;
    }
    if (t.type == TOKFALSE) {
        nexttoken(p);
        boolexpr *e = malloc(sizeof(boolexpr));
        e->base.type = EXPR_BOOL;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 0;
        return (expr*)e;
    }
    if (t.type == TOKNONE) {
        nexttoken(p);
        noneexpr *e = malloc(sizeof(noneexpr));
        e->base.type = EXPR_NONE;
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
        e->base.type = EXPR_LIST;
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
        e->base.type = EXPR_DICT;
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
        e->base.type = EXPR_UNARY;
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->op = t.type;
        e->operand = parseunary(p);
        return (expr*)e;
    }
    return parseprimary(p);
}

static expr *parsemul(parser *p) {
    expr *left = parseunary(p);
    while (1) {
        token t = peektoken(p);
        if (t.type == TOKSTAR || t.type == TOKSLASH || t.type == TOKPERCENT) {
            nexttoken(p);
            binaryexpr *e = malloc(sizeof(binaryexpr));
            e->base.type = EXPR_BINARY;
            e->base.eval = NULL;
            e->base.node.line = t.line;
            e->left = left;
            e->op = t.type;
            e->right = parseunary(p);
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
            e->base.type = EXPR_BINARY;
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
            e->base.type = EXPR_BINARY;
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
        e->base.type = EXPR_BINARY;
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
        e->base.type = EXPR_BINARY;
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

static stmtlist *parseblock(parser *p) {
    expect(p, TOKINDENT);
    stmtlist *block = malloc(sizeof(stmtlist));
    block->items = NULL;
    block->count = 0;
    while (peektoken(p).type != TOKDEDENT && peektoken(p).type != TOKEOF) {
        block->count++;
        block->items = realloc(block->items, sizeof(stmt*) * block->count);
        block->items[block->count - 1] = parsestmt(p);
    }
    expect(p, TOKDEDENT);
    return block;
}

static stmt *parselet(parser *p) {
    nexttoken(p);
    letexpr *s = malloc(sizeof(letexpr));
    s->base.type = STMT_LET;
    token name = peektoken(p);
    expect(p, TOKIDENT);
    s->name = strdup(name.text);
    expect(p, TOKEQ);
    s->value = parseexpr(p);
    s->isconst = 0;
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parseconst(parser *p) {
    nexttoken(p);
    letexpr *s = malloc(sizeof(letexpr));
    s->base.type = STMT_LET;
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
    s->base.type = STMT_RETURN;
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
    s->base.type = STMT_IF;
    s->cond = parseexpr(p);
    expect(p, TOKCOLON);
    expect(p, TOKNEWLINE);
    s->body = parseblock(p);
    s->elifs = NULL;
    s->elifcount = 0;
    s->elsebody = NULL;
    while (peektoken(p).type == TOKELIF) {
        nexttoken(p);
        s->elifcount++;
        s->elifs = realloc(s->elifs, sizeof(s->elifs[0]) * s->elifcount);
        s->elifs[s->elifcount - 1].cond = parseexpr(p);
        expect(p, TOKCOLON);
        expect(p, TOKNEWLINE);
        s->elifs[s->elifcount - 1].body = parseblock(p);
    }
    if (peektoken(p).type == TOKELSE) {
        nexttoken(p);
        expect(p, TOKCOLON);
        expect(p, TOKNEWLINE);
        s->elsebody = parseblock(p);
    }
    s->base.exec = NULL;
    s->base.node.line = s->cond->node.line;
    return (stmt*)s;
}

static stmt *parsewhile(parser *p) {
    nexttoken(p);
    whileexpr *s = malloc(sizeof(whileexpr));
    s->base.type = STMT_WHILE;
    s->cond = parseexpr(p);
    expect(p, TOKCOLON);
    expect(p, TOKNEWLINE);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = s->cond->node.line;
    return (stmt*)s;
}

static stmt *parsefor(parser *p) {
    nexttoken(p);
    forexp *s = malloc(sizeof(forexp));
    s->base.type = STMT_FOR;
    token var = peektoken(p);
    expect(p, TOKIDENT);
    s->var = strdup(var.text);
    expect(p, TOKIN);
    s->iter = parseexpr(p);
    expect(p, TOKCOLON);
    expect(p, TOKNEWLINE);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = var.line;
    return (stmt*)s;
}

static stmt *parsebreak(parser *p) {
    nexttoken(p);
    breakexp *s = malloc(sizeof(breakexp));
    s->base.type = STMT_BREAK;
    s->base.exec = NULL;
    s->base.node.line = peektoken(p).line;
    return (stmt*)s;
}

static stmt *parsenext(parser *p) {
    nexttoken(p);
    nextexp *s = malloc(sizeof(nextexp));
    s->base.type = STMT_NEXT;
    s->base.exec = NULL;
    s->base.node.line = peektoken(p).line;
    return (stmt*)s;
}

static stmt *parsefn(parser *p) {
    nexttoken(p);
    fnexp *s = malloc(sizeof(fnexp));
    s->base.type = STMT_FN;
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
    expect(p, TOKCOLON);
    expect(p, TOKNEWLINE);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parseclass(parser *p) {
    nexttoken(p);
    classexp *s = malloc(sizeof(classexp));
    s->base.type = STMT_CLASS;
    token name = peektoken(p);
    expect(p, TOKIDENT);
    s->name = strdup(name.text);
    if (peektoken(p).type == TOKLPAREN) {
        nexttoken(p);
        if (peektoken(p).type == TOKIDENT) nexttoken(p);
        expect(p, TOKRPAREN);
    }
    expect(p, TOKCOLON);
    expect(p, TOKNEWLINE);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parsethrow(parser *p) {
    nexttoken(p);
    throwexpr *s = malloc(sizeof(throwexpr));
    s->base.type = STMT_THROW;
    s->base.node.line = peektoken(p).line;
    s->value = parseexpr(p);
    s->base.exec = NULL;
    return (stmt*)s;
}

static stmt *parsetry(parser *p) {
    nexttoken(p);
    tryexpr *s = malloc(sizeof(tryexpr));
    s->base.type = STMT_TRY;
    s->base.node.line = peektoken(p).line;
    expect(p, TOKCOLON);
    expect(p, TOKNEWLINE);
    s->body = parseblock(p);
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
        expect(p, TOKCOLON);
        expect(p, TOKNEWLINE);
        s->catchbody = parseblock(p);
    }
    if (peektoken(p).type == TOKFINALLY) {
        nexttoken(p);
        expect(p, TOKCOLON);
        expect(p, TOKNEWLINE);
        s->finallybody = parseblock(p);
    }
    s->base.exec = NULL;
    return (stmt*)s;
}

static stmt *parseexprstmt(parser *p) {
    exprstmt *s = malloc(sizeof(exprstmt));
    s->base.type = STMT_EXPR;
    s->expression = parseexpr(p);
    s->base.exec = NULL;
    s->base.node.line = s->expression->node.line;
    return (stmt*)s;
}

static stmt *parsestmt(parser *p) {
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
        list->count++;
        list->items = realloc(list->items, sizeof(stmt*) * list->count);
        list->items[list->count - 1] = parsestmt(p);
    }
    return (stmt*)list;
}
