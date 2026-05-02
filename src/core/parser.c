#include "axton.h"

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

typedef struct {
    expr base;
    char *name;
} identexpr;

typedef struct {
    expr base;
    double value;
} numberexpr;

typedef struct {
    expr base;
    char *value;
} stringexpr;

typedef struct {
    expr base;
    int value;
} boolexpr;

typedef struct {
    expr base;
} noneexpr;

typedef struct {
    expr base;
    expr *left;
    toktype op;
    expr *right;
} binaryexpr;

typedef struct {
    expr base;
    toktype op;
    expr *operand;
} unaryexpr;

typedef struct {
    expr base;
    expr *callee;
    exprlist *args;
} callexpr;

typedef struct {
    stmt base;
    char *name;
    expr *value;
    int isconst;
} letstmt;

typedef struct {
    stmt base;
    expr *value;
} returnstmt;

typedef struct {
    stmt base;
    expr *cond;
    stmtlist *body;
    struct { expr *cond; stmtlist *body; } *elifs;
    int elifcount;
    stmtlist *elsebody;
} ifstmt;

typedef struct {
    stmt base;
    expr *cond;
    stmtlist *body;
} whilestmt;

typedef struct {
    stmt base;
    char *var;
    expr *iter;
    stmtlist *body;
} forstmt;

typedef struct {
    stmt base;
} breakstmt;

typedef struct {
    stmt base;
} nextstmt;

typedef struct {
    stmt base;
    char *name;
    char **params;
    int pcount;
    stmtlist *body;
} fnstmt;

typedef struct {
    stmt base;
    char *name;
    stmtlist *body;
} classstmt;

typedef struct {
    stmt base;
    expr *expression;
} exprstmt;

static token peek(parser *p) {
    if (p->pos >= p->count) {
        token t = {tokeof, NULL, 0, 0};
        return t;
    }
    return p->tokens[p->pos];
}

static token next(parser *p) {
    if (p->pos >= p->count) {
        token t = {tokeof, NULL, 0, 0};
        return t;
    }
    return p->tokens[p->pos++];
}

static void expect(parser *p, toktype type) {
    token t = next(p);
    if (t.type != type) {
        platformlog("parse error\n");
        exit(1);
    }
}

static expr *parseexpr(parser *p);
static stmt *parsestmt(parser *p);
static stmtlist *parseblock(parser *p);

static expr *parseprimary(parser *p) {
    token t = peek(p);
    
    if (t.type == tokident) {
        next(p);
        identexpr *e = malloc(sizeof(identexpr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->name = strdup(t.text);
        
        if (peek(p).type == toklparen) {
            next(p);
            callexpr *call = malloc(sizeof(callexpr));
            call->base.eval = NULL;
            call->base.node.line = t.line;
            call->callee = (expr*)e;
            call->args = malloc(sizeof(exprlist));
            call->args->items = NULL;
            call->args->count = 0;
            if (peek(p).type != tokrparen) {
                while (1) {
                    call->args->count++;
                    call->args->items = realloc(call->args->items, sizeof(expr*) * call->args->count);
                    call->args->items[call->args->count - 1] = parseexpr(p);
                    if (peek(p).type == tokcomma) next(p);
                    else break;
                }
            }
            expect(p, tokrparen);
            return (expr*)call;
        }
        return (expr*)e;
    }
    
    if (t.type == toknumber) {
        next(p);
        numberexpr *e = malloc(sizeof(numberexpr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = atof(t.text);
        return (expr*)e;
    }
    
    if (t.type == tokstring) {
        next(p);
        stringexpr *e = malloc(sizeof(stringexpr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = strdup(t.text);
        return (expr*)e;
    }
    
    if (t.type == toktrue) {
        next(p);
        boolexpr *e = malloc(sizeof(boolexpr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 1;
        return (expr*)e;
    }
    
    if (t.type == tokfalse) {
        next(p);
        boolexpr *e = malloc(sizeof(boolexpr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 0;
        return (expr*)e;
    }
    
    if (t.type == toknone) {
        next(p);
        noneexpr *e = malloc(sizeof(noneexpr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        return (expr*)e;
    }
    
    if (t.type == toklparen) {
        next(p);
        expr *e = parseexpr(p);
        expect(p, tokrparen);
        return e;
    }
    
    platformlog("parse error\n");
    exit(1);
}

static expr *parseunary(parser *p) {
    if (peek(p).type == tokminus || peek(p).type == toknot) {
        token op = next(p);
        unaryexpr *e = malloc(sizeof(unaryexpr));
        e->base.eval = NULL;
        e->base.node.line = op.line;
        e->op = op.type;
        e->operand = parseunary(p);
        return (expr*)e;
    }
    return parseprimary(p);
}

static expr *parsemul(parser *p) {
    expr *left = parseunary(p);
    while (peek(p).type == tokstar || peek(p).type == tokslash) {
        token op = next(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = op.type;
        e->right = parseunary(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parseadd(parser *p) {
    expr *left = parsemul(p);
    while (peek(p).type == tokplus || peek(p).type == tokminus) {
        token op = next(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = op.type;
        e->right = parsemul(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parsecompare(parser *p) {
    expr *left = parseadd(p);
    while (peek(p).type == tokeqeq || peek(p).type == tokne ||
           peek(p).type == toklt || peek(p).type == tokgt ||
           peek(p).type == tokle || peek(p).type == tokge) {
        token op = next(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = op.type;
        e->right = parseadd(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parseand(parser *p) {
    expr *left = parsecompare(p);
    while (peek(p).type == tokand) {
        next(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = tokand;
        e->right = parsecompare(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parseor(parser *p) {
    expr *left = parseand(p);
    while (peek(p).type == tokor) {
        next(p);
        binaryexpr *e = malloc(sizeof(binaryexpr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = tokor;
        e->right = parseand(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parseexpr(parser *p) {
    return parseor(p);
}

static stmtlist *parseblock(parser *p) {
    expect(p, tokindent);
    stmtlist *block = malloc(sizeof(stmtlist));
    block->items = NULL;
    block->count = 0;
    while (peek(p).type != tokdedent && peek(p).type != tokeof) {
        block->count++;
        block->items = realloc(block->items, sizeof(stmt*) * block->count);
        block->items[block->count - 1] = parsestmt(p);
    }
    expect(p, tokdedent);
    return block;
}

static stmt *parselet(parser *p) {
    next(p);
    letstmt *s = malloc(sizeof(letstmt));
    token name = next(p);
    if (name.type != tokident) exit(1);
    s->name = strdup(name.text);
    expect(p, tokeq);
    s->value = parseexpr(p);
    s->isconst = 0;
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parseconst(parser *p) {
    next(p);
    letstmt *s = malloc(sizeof(letstmt));
    token name = next(p);
    if (name.type != tokident) exit(1);
    s->name = strdup(name.text);
    expect(p, tokeq);
    s->value = parseexpr(p);
    s->isconst = 1;
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parsereturn(parser *p) {
    next(p);
    returnstmt *s = malloc(sizeof(returnstmt));
    s->base.node.line = peek(p).line;
    s->value = NULL;
    if (peek(p).type != toknewline && peek(p).type != tokdedent) {
        s->value = parseexpr(p);
    }
    s->base.exec = NULL;
    return (stmt*)s;
}

static stmt *parseif(parser *p) {
    next(p);
    ifstmt *s = malloc(sizeof(ifstmt));
    s->cond = parseexpr(p);
    expect(p, tokcolon);
    expect(p, toknewline);
    s->body = parseblock(p);
    s->elifs = NULL;
    s->elifcount = 0;
    s->elsebody = NULL;
    while (peek(p).type == tokelif) {
        next(p);
        s->elifcount++;
        s->elifs = realloc(s->elifs, sizeof(s->elifs[0]) * s->elifcount);
        s->elifs[s->elifcount - 1].cond = parseexpr(p);
        expect(p, tokcolon);
        expect(p, toknewline);
        s->elifs[s->elifcount - 1].body = parseblock(p);
    }
    if (peek(p).type == tokelse) {
        next(p);
        expect(p, tokcolon);
        expect(p, toknewline);
        s->elsebody = parseblock(p);
    }
    s->base.exec = NULL;
    s->base.node.line = s->cond->line;
    return (stmt*)s;
}

static stmt *parsewhile(parser *p) {
    next(p);
    whilestmt *s = malloc(sizeof(whilestmt));
    s->cond = parseexpr(p);
    expect(p, tokcolon);
    expect(p, toknewline);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = s->cond->line;
    return (stmt*)s;
}

static stmt *parsefor(parser *p) {
    next(p);
    forstmt *s = malloc(sizeof(forstmt));
    token var = next(p);
    if (var.type != tokident) exit(1);
    s->var = strdup(var.text);
    expect(p, tokin);
    s->iter = parseexpr(p);
    expect(p, tokcolon);
    expect(p, toknewline);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = var.line;
    return (stmt*)s;
}

static stmt *parsebreak(parser *p) {
    next(p);
    breakstmt *s = malloc(sizeof(breakstmt));
    s->base.exec = NULL;
    s->base.node.line = peek(p).line;
    return (stmt*)s;
}

static stmt *parsenext(parser *p) {
    next(p);
    nextstmt *s = malloc(sizeof(nextstmt));
    s->base.exec = NULL;
    s->base.node.line = peek(p).line;
    return (stmt*)s;
}

static stmt *parsefn(parser *p) {
    next(p);
    fnstmt *s = malloc(sizeof(fnstmt));
    token name = next(p);
    if (name.type != tokident) exit(1);
    s->name = strdup(name.text);
    expect(p, toklparen);
    s->params = NULL;
    s->pcount = 0;
    if (peek(p).type != tokrparen) {
        while (1) {
            s->pcount++;
            s->params = realloc(s->params, sizeof(char*) * s->pcount);
            token param = next(p);
            if (param.type != tokident) exit(1);
            s->params[s->pcount - 1] = strdup(param.text);
            if (peek(p).type == tokcomma) next(p);
            else break;
        }
    }
    expect(p, tokrparen);
    expect(p, tokcolon);
    expect(p, toknewline);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parseclass(parser *p) {
    next(p);
    classstmt *s = malloc(sizeof(classstmt));
    token name = next(p);
    if (name.type != tokident) exit(1);
    s->name = strdup(name.text);
    expect(p, tokcolon);
    expect(p, toknewline);
    s->body = parseblock(p);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parseexprstmt(parser *p) {
    exprstmt *s = malloc(sizeof(exprstmt));
    s->expression = parseexpr(p);
    s->base.exec = NULL;
    s->base.node.line = s->expression->line;
    return (stmt*)s;
}

static stmt *parsestmt(parser *p) {
    token t = peek(p);
    switch (t.type) {
        case toklet: return parselet(p);
        case tokconst: return parseconst(p);
        case tokfn: return parsefn(p);
        case tokclass: return parseclass(p);
        case tokif: return parseif(p);
        case tokwhile: return parsewhile(p);
        case tokfor: return parsefor(p);
        case tokbreak: return parsebreak(p);
        case toknext: return parsenext(p);
        case tokreturn: return parsereturn(p);
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
    while (peek(p).type != tokeof) {
        list->count++;
        list->items = realloc(list->items, sizeof(stmt*) * list->count);
        list->items[list->count - 1] = parsestmt(p);
    }
    return (stmt*)list;
}
