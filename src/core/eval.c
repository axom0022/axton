#include "axton.h"
#include <math.h>

static object *evalexpr(expr *e, environment *env);
static object *evalstmt(stmt *s, environment *env);

static object *evalbinary(binaryexpr *e, environment *env) {
    object *left = evalexpr(e->left, env);
    object *right = evalexpr(e->right, env);
    switch (e->op) {
        case TOKPLUS: return addvalues(left, right);
        case TOKMINUS: return subvalues(left, right);
        case TOKSTAR: return mulvalues(left, right);
        case TOKSLASH: return divvalues(left, right);
        case TOKEQEQ: return makebool(valuesequal(left, right));
        case TOKNE: return makebool(!valuesequal(left, right));
        case TOKLT: return makebool(lessthan(left, right));
        case TOKGT: return makebool(greaterthan(left, right));
        case TOKLE: return makebool(!greaterthan(left, right));
        case TOKGE: return makebool(!lessthan(left, right));
        case TOKAND: return istruthy(left) ? right : left;
        case TOKOR: return istruthy(left) ? left : right;
        default:
            throwexception("unknown binary operator");
            return NULL;
    }
}

static object *evalunary(unaryexpr *e, environment *env) {
    object *operand = evalexpr(e->operand, env);
    if (e->op == TOKNOT) return makebool(!istruthy(operand));
    if (e->op == TOKMINUS) {
        if (operand->type == 0) return makeint(-operand->ival);
        if (operand->type == 1) return makefloat(-operand->fval);
        throwexception("bad operand for unary minus");
    }
    throwexception("unknown unary operator");
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

static object *evalindex(indexexpr *e, environment *env) {
    object *target = evalexpr(e->target, env);
    object *idx = evalexpr(e->index, env);
    if (target->type == 5) {
        if (idx->type != 0) throwexception("list index must be integer");
        long i = idx->ival;
        if (i < 0) i = target->list.count + i;
        if (i < 0 || i >= target->list.count) throwexception("index out of range");
        return target->list.items[i];
    }
    if (target->type == 6) {
        object *val = dictget(target, idx);
        if (!val) throwexception("key not found");
        return val;
    }
    if (target->type == 2) {
        if (idx->type != 0) throwexception("string index must be integer");
        long i = idx->ival;
        char c = target->sval[i];
        char buf[2] = {c, 0};
        return makestring(buf);
    }
    throwexception("cannot index");
    return NULL;
}

static object *evalattr(attribexpr *e, environment *env) {
    object *target = evalexpr(e->target, env);
    if (target->type == 9 && target->klass.attrs) {
        object *val = envget((environment*)target->klass.attrs, e->attr);
        if (val) return val;
    }
    if (target->type == 10 && target->instance.attrs) {
        object *val = envget((environment*)target->instance.attrs, e->attr);
        if (val) return val;
    }
    if (target->type == 12 && target->module.exports) {
        object *val = envget((environment*)target->module.exports, e->attr);
        if (val) return val;
    }
    char msg[256];
    snprintf(msg, sizeof(msg), "attribute '%s' not found", e->attr);
    throwexception(msg);
    return NULL;
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
    if (((indexexpr*)e)->target) return evalindex((indexexpr*)e, env);
    if (((attribexpr*)e)->attr) return evalattr((attribexpr*)e, env);
    throwexception("unknown expression");
    return NULL;
}

static object *evallet(letexpr *s, environment *env) {
    object *val = evalexpr(s->value, env);
    envset(env, s->name, val, s->isconst);
    return makenone();
}

static object *evalreturn(returnexpr *s, environment *env) {
    object *val = s->value ? evalexpr(s->value, env) : makenone();
    if (currentframe) {
        currentframe->result = val;
        longjmp(currentframe->jump, 1);
    }
    return val;
}

static object *evalif(ifexpr *s, environment *env) {
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

static object *evalwhile(whileexpr *s, environment *env) {
    while (istruthy(evalexpr(s->cond, env))) {
        environment *blockenv = envnew(env);
        for (int i = 0; i < s->body->count; i++) {
            evalstmt(s->body->items[i], blockenv);
        }
    }
    return makenone();
}

static object *evalfor(forexp *s, environment *env) {
    object *iterable = evalexpr(s->iter, env);
    object *list = NULL;
    if (iterable->type == 11) {
        list = makelist();
        for (long i = iterable->range.start; i < iterable->range.stop; i += iterable->range.step) {
            listappend(list, makeint(i));
        }
    } else if (iterable->type == 5) {
        list = iterable;
    } else {
        throwexception("for loop expects iterable");
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

static object *evalbreak(breakexp *s, environment *env) {
    return makenone();
}

static object *evalnext(nextexp *s, environment *env) {
    return makenone();
}

static object *evalfn(fnexp *s, environment *env) {
    object *fn = makefunc(s->params, NULL, s->pcount, (stmt**)s->body->items, s->body->count, env, s->name, 0, 0);
    envset(env, s->name, fn, 0);
    return makenone();
}

static object *evalclass(classexp *s, environment *env) {
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
    if (((letexpr*)s)->name) return evallet((letexpr*)s, env);
    if (((returnexpr*)s)->value != NULL || ((returnexpr*)s)->value == NULL) 
        return evalreturn((returnexpr*)s, env);
    if (((ifexpr*)s)->cond) return evalif((ifexpr*)s, env);
    if (((whileexpr*)s)->cond) return evalwhile((whileexpr*)s, env);
    if (((forexp*)s)->var) return evalfor((forexp*)s, env);
    if (((breakexp*)s)) return evalbreak((breakexp*)s, env);
    if (((nextexp*)s)) return evalnext((nextexp*)s, env);
    if (((fnexp*)s)->name) return evalfn((fnexp*)s, env);
    if (((classexp*)s)->name) return evalclass((classexp*)s, env);
    if (((exprstmt*)s)->expression) return evalexprstmt((exprstmt*)s, env);
    throwexception("unknown statement");
    return NULL;
}

object *callfunc(object *fn, object **args, int argc, environment *env) {
    if (fn->type == 8) {
        return fn->builtin.fn(args, argc, env);
    }
    if (fn->type == 7) {
        if (argc != fn->func.pcount) {
            throwexception("wrong number of arguments");
            return NULL;
        }
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
        object *inst = makeinstance(fn, args, argc);
        object *init = envget(fn->klass.attrs, "init");
        if (init && init->type == 7) {
            callfunc(init, args, argc, fn->klass.attrs);
        }
        return inst;
    }
    throwexception("object is not callable");
    return NULL;
}

object *evalprogram(stmt *program, environment *env) {
    stmtlist *list = (stmtlist*)program;
    for (int i = 0; i < list->count; i++) {
        evalstmt(list->items[i], env);
    }
    return makenone();
}
