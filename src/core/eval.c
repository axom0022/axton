#include "axton.h"

static object *evalexpr(expr *e, environment *env);
static object *evalstmt(stmt *s, environment *env);

static object *evalbinary(binaryexpr *e, environment *env) {
    object *left = evalexpr(e->left, env);
    object *right = evalexpr(e->right, env);
    switch (e->op) {
        case tokplus: return addvalues(left, right);
        case tokminus: return subvalues(left, right);
        case tokstar: return mulvalues(left, right);
        case tokslash: return divvalues(left, right);
        case tokeqeq: return makebool(valuesequal(left, right));
        case tokne: return makebool(!valuesequal(left, right));
        case toklt: return makebool(lessthan(left, right));
        case tokgt: return makebool(greaterthan(left, right));
        case tokand: return istruthy(left) ? right : left;
        case tokor: return istruthy(left) ? left : right;
        default: throwexception("bad operator"); return NULL;
    }
}

static object *evalunary(unaryexpr *e, environment *env) {
    object *operand = evalexpr(e->operand, env);
    if (e->op == toknot) return makebool(!istruthy(operand));
    if (e->op == tokminus) {
        if (operand->type == 0) return makeint(-operand->ival);
        if (operand->type == 1) return makefloat(-operand->fval);
        throwexception("bad operand for minus");
    }
    throwexception("bad unary");
    return NULL;
}

static object *evalident(identexpr *e, environment *env) {
    object *v = envget(env, e->name);
    if (!v) {
        char msg[256];
        snprintf(msg, sizeof(msg), "name '%s' not defined", e->name);
        throwexception(msg);
    }
    return v;
}

static object *evalnumber(numberexpr *e, environment *env) {
    if (e->value == (double)(long)e->value) return makeint((long)e->value);
    return makefloat(e->value);
}

static object *evalstring(stringexpr *e, environment *env) {
    return makestring(e->value);
}

static object *evalbool(boolexpr *e, environment *env) {
    return makebool(e->value);
}

static object *evalnone(noneexpr *e, environment *env) {
    return makenone();
}

static object *evalcall(callexpr *e, environment *env) {
    object *callee = evalexpr(e->callee, env);
    object **args = malloc(sizeof(object*) * e->args->count);
    for (int i = 0; i < e->args->count; i++) {
        args[i] = evalexpr(e->args->items[i], env);
    }
    object *result = callfunc(callee, args, e->args->count, env);
    free(args);
    return result;
}

static object *evalexpr(expr *e, environment *env) {
    if (((binaryexpr*)e)->op) return evalbinary((binaryexpr*)e, env);
    if (((unaryexpr*)e)->op) return evalunary((unaryexpr*)e, env);
    if (((identexpr*)e)->name) return evalident((identexpr*)e, env);
    if (((numberexpr*)e)->value) return evalnumber((numberexpr*)e, env);
    if (((stringexpr*)e)->value) return evalstring((stringexpr*)e, env);
    if (((boolexpr*)e)->value) return evalbool((boolexpr*)e, env);
    if (((noneexpr*)e)) return evalnone((noneexpr*)e, env);
    if (((callexpr*)e)->callee) return evalcall((callexpr*)e, env);
    throwexception("unknown expression");
    return NULL;
}

static object *evallet(letstmt *s, environment *env) {
    object *val = evalexpr(s->value, env);
    envset(env, s->name, val, s->isconst);
    return makenone();
}

static object *evalreturn(returnstmt *s, environment *env) {
    object *val = s->value ? evalexpr(s->value, env) : makenone();
    if (currentframe) {
        currentframe->result = val;
        longjmp(currentframe->jump, 1);
    }
    return val;
}

static object *evalif(ifstmt *s, environment *env) {
    if (istruthy(evalexpr(s->cond, env))) {
        environment *blockenv = envnew(env);
        for (int i = 0; i < s->body->count; i++) ev
