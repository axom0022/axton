#include "axton.h"

typedef struct {
    token *tokens;
    int pos;
    int count;
} parser_t;

typedef struct {
    expr **items;
    int count;
} expr_list;

typedef struct {
    stmt **items;
    int count;
} stmt_list;

typedef struct {
    expr base;
    char *name;
} ident_expr;

typedef struct {
    expr base;
    double value;
} number_expr;

typedef struct {
    expr base;
    char *value;
} string_expr;

typedef struct {
    expr base;
    int value;
} bool_expr;

typedef struct {
    expr base;
} none_expr;

typedef struct {
    expr base;
    expr *left;
    toktype op;
    expr *right;
} binary_expr;

typedef struct {
    expr base;
    toktype op;
    expr *operand;
} unary_expr;

typedef struct {
    expr base;
    expr *callee;
    expr_list *args;
} call_expr;

typedef struct {
    stmt base;
    char *name;
    expr *value;
    int isconst;
} let_stmt;

typedef struct {
    stmt base;
    expr *value;
} return_stmt;

typedef struct {
    stmt base;
    expr *cond;
    stmt_list *body;
    struct { expr *cond; stmt_list *body; } *elifs;
    int elif_count;
    stmt_list *else_body;
} if_stmt;

typedef struct {
    stmt base;
    expr *cond;
    stmt_list *body;
} while_stmt;

typedef struct {
    stmt base;
    char *var;
    expr *iter;
    stmt_list *body;
} for_stmt;

typedef struct {
    stmt base;
} break_stmt;

typedef struct {
    stmt base;
} next_stmt;

typedef struct {
    stmt base;
    char *name;
    char **params;
    int pcount;
    stmt_list *body;
} fn_stmt;

typedef struct {
    stmt base;
    char *name;
    stmt_list *body;
} class_stmt;

typedef struct {
    stmt base;
    expr *expression;
} expr_stmt;

static token peek_token(parser_t *p) {
    if (p->pos >= p->count) {
        token t = {TOK_EOF, NULL, 0, 0};
        return t;
    }
    return p->tokens[p->pos];
}

static token next_token(parser_t *p) {
    if (p->pos >= p->count) {
        token t = {TOK_EOF, NULL, 0, 0};
        return t;
    }
    return p->tokens[p->pos++];
}

static void expect_token(parser_t *p, toktype type) {
    token t = next_token(p);
    if (t.type != type) {
        platform_log("parse error\n");
        exit(1);
    }
}

static expr *parse_expr(parser_t *p);
static stmt *parse_stmt(parser_t *p);
static stmt_list *parse_block(parser_t *p);

static expr *parse_primary(parser_t *p) {
    token t = peek_token(p);
    
    if (t.type == TOK_IDENT) {
        next_token(p);
        ident_expr *e = malloc(sizeof(ident_expr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->name = strdup(t.text);
        
        if (peek_token(p).type == TOK_LPAREN) {
            next_token(p);
            call_expr *call = malloc(sizeof(call_expr));
            call->base.eval = NULL;
            call->base.node.line = t.line;
            call->callee = (expr*)e;
            call->args = malloc(sizeof(expr_list));
            call->args->items = NULL;
            call->args->count = 0;
            if (peek_token(p).type != TOK_RPAREN) {
                while (1) {
                    call->args->count++;
                    call->args->items = realloc(call->args->items, sizeof(expr*) * call->args->count);
                    call->args->items[call->args->count - 1] = parse_expr(p);
                    if (peek_token(p).type == TOK_COMMA) next_token(p);
                    else break;
                }
            }
            expect_token(p, TOK_RPAREN);
            return (expr*)call;
        }
        return (expr*)e;
    }
    
    if (t.type == TOK_NUMBER) {
        next_token(p);
        number_expr *e = malloc(sizeof(number_expr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = atof(t.text);
        return (expr*)e;
    }
    
    if (t.type == TOK_STRING) {
        next_token(p);
        string_expr *e = malloc(sizeof(string_expr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = strdup(t.text);
        return (expr*)e;
    }
    
    if (t.type == TOK_TRUE) {
        next_token(p);
        bool_expr *e = malloc(sizeof(bool_expr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 1;
        return (expr*)e;
    }
    
    if (t.type == TOK_FALSE) {
        next_token(p);
        bool_expr *e = malloc(sizeof(bool_expr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        e->value = 0;
        return (expr*)e;
    }
    
    if (t.type == TOK_NONE) {
        next_token(p);
        none_expr *e = malloc(sizeof(none_expr));
        e->base.eval = NULL;
        e->base.node.line = t.line;
        return (expr*)e;
    }
    
    if (t.type == TOK_LPAREN) {
        next_token(p);
        expr *e = parse_expr(p);
        expect_token(p, TOK_RPAREN);
        return e;
    }
    
    platform_log("parse error\n");
    exit(1);
}

static expr *parse_unary(parser_t *p) {
    if (peek_token(p).type == TOK_MINUS || peek_token(p).type == TOK_NOT) {
        token op = next_token(p);
        unary_expr *e = malloc(sizeof(unary_expr));
        e->base.eval = NULL;
        e->base.node.line = op.line;
        e->op = op.type;
        e->operand = parse_unary(p);
        return (expr*)e;
    }
    return parse_primary(p);
}

static expr *parse_mul(parser_t *p) {
    expr *left = parse_unary(p);
    while (peek_token(p).type == TOK_STAR || peek_token(p).type == TOK_SLASH) {
        token op = next_token(p);
        binary_expr *e = malloc(sizeof(binary_expr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = op.type;
        e->right = parse_unary(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parse_add(parser_t *p) {
    expr *left = parse_mul(p);
    while (peek_token(p).type == TOK_PLUS || peek_token(p).type == TOK_MINUS) {
        token op = next_token(p);
        binary_expr *e = malloc(sizeof(binary_expr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = op.type;
        e->right = parse_mul(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parse_compare(parser_t *p) {
    expr *left = parse_add(p);
    while (peek_token(p).type == TOK_EQEQ || peek_token(p).type == TOK_NE ||
           peek_token(p).type == TOK_LT || peek_token(p).type == TOK_GT ||
           peek_token(p).type == TOK_LE || peek_token(p).type == TOK_GE) {
        token op = next_token(p);
        binary_expr *e = malloc(sizeof(binary_expr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = op.type;
        e->right = parse_add(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parse_and(parser_t *p) {
    expr *left = parse_compare(p);
    while (peek_token(p).type == TOK_AND) {
        next_token(p);
        binary_expr *e = malloc(sizeof(binary_expr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = TOK_AND;
        e->right = parse_compare(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parse_or(parser_t *p) {
    expr *left = parse_and(p);
    while (peek_token(p).type == TOK_OR) {
        next_token(p);
        binary_expr *e = malloc(sizeof(binary_expr));
        e->base.eval = NULL;
        e->base.node.line = left->line;
        e->left = left;
        e->op = TOK_OR;
        e->right = parse_and(p);
        left = (expr*)e;
    }
    return left;
}

static expr *parse_expr(parser_t *p) {
    return parse_or(p);
}

static stmt_list *parse_block(parser_t *p) {
    expect_token(p, TOK_INDENT);
    stmt_list *block = malloc(sizeof(stmt_list));
    block->items = NULL;
    block->count = 0;
    while (peek_token(p).type != TOK_DEDENT && peek_token(p).type != TOK_EOF) {
        block->count++;
        block->items = realloc(block->items, sizeof(stmt*) * block->count);
        block->items[block->count - 1] = parse_stmt(p);
    }
    expect_token(p, TOK_DEDENT);
    return block;
}

static stmt *parse_let(parser_t *p) {
    next_token(p);
    let_stmt *s = malloc(sizeof(let_stmt));
    token name = next_token(p);
    if (name.type != TOK_IDENT) exit(1);
    s->name = strdup(name.text);
    expect_token(p, TOK_EQ);
    s->value = parse_expr(p);
    s->isconst = 0;
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parse_return(parser_t *p) {
    next_token(p);
    return_stmt *s = malloc(sizeof(return_stmt));
    s->base.node.line = peek_token(p).line;
    s->value = NULL;
    if (peek_token(p).type != TOK_NEWLINE && peek_token(p).type != TOK_DEDENT) {
        s->value = parse_expr(p);
    }
    s->base.exec = NULL;
    return (stmt*)s;
}

static stmt *parse_if(parser_t *p) {
    next_token(p);
    if_stmt *s = malloc(sizeof(if_stmt));
    s->cond = parse_expr(p);
    expect_token(p, TOK_COLON);
    expect_token(p, TOK_NEWLINE);
    s->body = parse_block(p);
    s->elifs = NULL;
    s->elif_count = 0;
    s->else_body = NULL;
    while (peek_token(p).type == TOK_ELIF) {
        next_token(p);
        s->elif_count++;
        s->elifs = realloc(s->elifs, sizeof(s->elifs[0]) * s->elif_count);
        s->elifs[s->elif_count - 1].cond = parse_expr(p);
        expect_token(p, TOK_COLON);
        expect_token(p, TOK_NEWLINE);
        s->elifs[s->elif_count - 1].body = parse_block(p);
    }
    if (peek_token(p).type == TOK_ELSE) {
        next_token(p);
        expect_token(p, TOK_COLON);
        expect_token(p, TOK_NEWLINE);
        s->else_body = parse_block(p);
    }
    s->base.exec = NULL;
    s->base.node.line = s->cond->line;
    return (stmt*)s;
}

static stmt *parse_while(parser_t *p) {
    next_token(p);
    while_stmt *s = malloc(sizeof(while_stmt));
    s->cond = parse_expr(p);
    expect_token(p, TOK_COLON);
    expect_token(p, TOK_NEWLINE);
    s->body = parse_block(p);
    s->base.exec = NULL;
    s->base.node.line = s->cond->line;
    return (stmt*)s;
}

static stmt *parse_for(parser_t *p) {
    next_token(p);
    for_stmt *s = malloc(sizeof(for_stmt));
    token var = next_token(p);
    if (var.type != TOK_IDENT) exit(1);
    s->var = strdup(var.text);
    expect_token(p, TOK_IN);
    s->iter = parse_expr(p);
    expect_token(p, TOK_COLON);
    expect_token(p, TOK_NEWLINE);
    s->body = parse_block(p);
    s->base.exec = NULL;
    s->base.node.line = var.line;
    return (stmt*)s;
}

static stmt *parse_break(parser_t *p) {
    next_token(p);
    break_stmt *s = malloc(sizeof(break_stmt));
    s->base.exec = NULL;
    s->base.node.line = peek_token(p).line;
    return (stmt*)s;
}

static stmt *parse_next(parser_t *p) {
    next_token(p);
    next_stmt *s = malloc(sizeof(next_stmt));
    s->base.exec = NULL;
    s->base.node.line = peek_token(p).line;
    return (stmt*)s;
}

static stmt *parse_fn(parser_t *p) {
    next_token(p);
    fn_stmt *s = malloc(sizeof(fn_stmt));
    token name = next_token(p);
    if (name.type != TOK_IDENT) exit(1);
    s->name = strdup(name.text);
    expect_token(p, TOK_LPAREN);
    s->params = NULL;
    s->pcount = 0;
    if (peek_token(p).type != TOK_RPAREN) {
        while (1) {
            s->pcount++;
            s->params = realloc(s->params, sizeof(char*) * s->pcount);
            token param = next_token(p);
            if (param.type != TOK_IDENT) exit(1);
            s->params[s->pcount - 1] = strdup(param.text);
            if (peek_token(p).type == TOK_COMMA) next_token(p);
            else break;
        }
    }
    expect_token(p, TOK_RPAREN);
    expect_token(p, TOK_COLON);
    expect_token(p, TOK_NEWLINE);
    s->body = parse_block(p);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parse_class(parser_t *p) {
    next_token(p);
    class_stmt *s = malloc(sizeof(class_stmt));
    token name = next_token(p);
    if (name.type != TOK_IDENT) exit(1);
    s->name = strdup(name.text);
    expect_token(p, TOK_COLON);
    expect_token(p, TOK_NEWLINE);
    s->body = parse_block(p);
    s->base.exec = NULL;
    s->base.node.line = name.line;
    return (stmt*)s;
}

static stmt *parse_expr_stmt(parser_t *p) {
    expr_stmt *s = malloc(sizeof(expr_stmt));
    s->expression = parse_expr(p);
    s->base.exec = NULL;
    s->base.node.line = s->expression->line;
    return (stmt*)s;
}

static stmt *parse_stmt(parser_t *p) {
    token t = peek_token(p);
    switch (t.type) {
        case TOK_LET: return parse_let(p);
        case TOK_CONST: return parse_let(p);
        case TOK_FN: return parse_fn(p);
        case TOK_CLASS: return parse_class(p);
        case TOK_IF: return parse_if(p);
        case TOK_WHILE: return parse_while(p);
        case TOK_FOR: return parse_for(p);
        case TOK_BREAK: return parse_break(p);
        case TOK_NEXT: return parse_next(p);
        case TOK_RETURN: return parse_return(p);
        default: return parse_expr_stmt(p);
    }
}

stmt *parse_tokens(token *tokens, int count) {
    parser_t *p = malloc(sizeof(parser_t));
    p->tokens = tokens;
    p->pos = 0;
    p->count = count;
    
    stmt_list *list = malloc(sizeof(stmt_list));
    list->items = NULL;
    list->count = 0;
    while (peek_token(p).type != TOK_EOF) {
        list->count++;
        list->items = realloc(list->items, sizeof(stmt*) * list->count);
        list->items[list->count - 1] = parse_stmt(p);
    }
    return (stmt*)list;
}
