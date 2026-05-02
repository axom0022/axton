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
        for (int i = 0; i < s->body->count; i++) {
                        evalstmt(s->body->items[i], blockenv);
        }
        return makenone();
    }
    for (int i = 0; i < s->elifcount; i++) {
        if (istruthy(evalexpr(s->elifs[i].cond, env))) {
            environment *blockenv = envnew(env);
            for (int j = 0; j < s->elifs[i].body->count; j++) {
                evalstmt(s->elifs[i].body->items[j], blockenv);
            }
            return makenone();
        }
    }
    if (s->elsebody) {
        environment *blockenv = envnew(env);
        for (int i = 0; i < s->elsebody->count; i++) {
            evalstmt(s->elsebody->items[i], blockenv);
        }
    }
    return makenone();
}

static object *evalwhile(whilestmt *s, environment *env) {
    while (istruthy(evalexpr(s->cond, env))) {
        environment *blockenv = envnew(env);
        for (int i = 0; i < s->body->count; i++) {
            evalstmt(s->body->items[i], blockenv);
        }
    }
    return makenone();
}

static object *evalfor(forstmt *s, environment *env) {
    object *iterable = evalexpr(s->iter, env);
    object *list = NULL;
    if (iterable->type == 11) {
        list = makelist();
        long step = iterable->range.step > 0 ? 1 : -1;
        for (long i = iterable->range.start; i < iterable->range.stop; i += step) {
            listappend(list, makeint(i));
        }
    } else if (iterable->type == 5) {
        list = iterable;
    } else {
        throwexception("for needs iterable");
        return NULL;
    }
    for (int i = 0; i < list->list.count; i++) {
        environment *blockenv = envnew(env);
        envset(blockenv, s->var, list->list.items[i], 0);
        for (int j = 0; j < s->body->count; j++) {
            evalstmt(s->body->items[j], blockenv);
        }
    }
    return makenone();
}

static object *evalbreak(breakstmt *s, environment *env) {
    return makenone();
}

static object *evalnext(nextstmt *s, environment *env) {
    return makenone();
}

static object *evalfn(fnstmt *s, environment *env) {
    object *fn = makefunc(s->params, s->pcount, (stmt**)s->body->items, s->body->count, env, s->name);
    envset(env, s->name, fn, 0);
    return makenone();
}

static object *evalclass(classstmt *s, environment *env) {
    environment *classenv = envnew(env);
    for (int i = 0; i < s->body->count; i++) {
        evalstmt(s->body->items[i], classenv);
    }
    object *cls = makeclass(s->name, classenv, NULL);
    envset(env, s->name, cls, 0);
    return makenone();
}

static object *evalexprstmt(exprstmt *s, environment *env) {
    return evalexpr(s->expression, env);
}

static object *evalstmt(stmt *s, environment *env) {
    if (((letstmt*)s)->name) return evallet((letstmt*)s, env);
    if (((returnstmt*)s)->value != NULL || ((returnstmt*)s)->value == NULL)
        return evalreturn((returnstmt*)s, env);
    if (((ifstmt*)s)->cond) return evalif((ifstmt*)s, env);
    if (((whilestmt*)s)->cond) return evalwhile((whilestmt*)s, env);
    if (((forstmt*)s)->var) return evalfor((forstmt*)s, env);
    if (((breakstmt*)s)) return evalbreak((breakstmt*)s, env);
    if (((nextstmt*)s)) return evalnext((nextstmt*)s, env);
    if (((fnstmt*)s)->name) return evalfn((fnstmt*)s, env);
    if (((classstmt*)s)->name) return evalclass((classstmt*)s, env);
    if (((exprstmt*)s)->expression) return evalexprstmt((exprstmt*)s, env);
    throwexception("unknown statement");
    return NULL;
}

object *callfunc(object *fn, object **args, int argc, environment *env) {
    if (fn->type == 8) {
        return fn->builtin.fn(args, argc, env);
    }
    if (fn->type == 7) {
        if (argc != fn->func.pcount) throwexception("wrong argument count");
        environment *callenv = envnew(fn->func.closure);
        for (int i = 0; i < argc; i++) {
            envset(callenv, fn->func.params[i], args[i], 0);
        }
        frame f;
        frame *oldframe = currentframe;
        currentframe = &f;
        object *result = makenone();
        if (setjmp(f.jump) == 0) {
            for (int i = 0; i < fn->func.bcount; i++) {
                evalstmt(fn->func.body[i], callenv);
            }
        } else {
            result = f.result;
        }
        currentframe = oldframe;
        return result;
    }
    if (fn->type == 9) {
        object *inst = makeinstance(fn);
        object *init = envget(fn->klass.attrs, "init");
        if (init && init->type == 7) {
            callfunc(init, args, argc, fn->klass.attrs);
        }
        return inst;
    }
    throwexception("not callable");
    return NULL;
}

object *evalprogram(stmt *program, environment *env) {
    stmtlist *list = (stmtlist*)program;
    for (int i = 0; i < list->count; i++) {
        evalstmt(list->items[i], env);
    }
    return makenone();
}
