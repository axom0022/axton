#include "axton.h"
#include <math.h>

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
    (void)env;
    if (e->value == (double)(long)e->value) return makeint((long)e->value);
    return makefloat(e->value);
}

static object *evalstring(stringexpr *e, environment *env) {
    (void)env;
    return makestring(e->value);
}

static object *evalbool(boolexpr *e, environment *env) {
    (void)env;
    return makebool(e->value);
}

static object *evalcall(callexpr *e, environment *env) {
    object *callee = evalexpr(e->callee, env);
    if (!callee) throwexception("call on undefined");
    object **args = malloc(sizeof(object*) * (e->args->count + 1));
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
        if (i < 0) i = strlen(target->sval) + i;
        if (i < 0 || i >= (long)strlen(target->sval)) throwexception("index out of range");
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
        if (target->instance.klass && target->instance.klass->klass.attrs) {
            val = envget((environment*)target->instance.klass->klass.attrs, e->attr);
            if (val) return val;
        }
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
    switch (e->type) {
        case EXPR_BINARY: return evalbinary((binaryexpr*)e, env);
        case EXPR_UNARY:  return evalunary((unaryexpr*)e, env);
        case EXPR_IDENT:  return evalident((identexpr*)e, env);
        case EXPR_NUMBER: return evalnumber((numberexpr*)e, env);
        case EXPR_STRING: return evalstring((stringexpr*)e, env);
        case EXPR_BOOL:   return evalbool((boolexpr*)e, env);
        case EXPR_NONE:   return makenone();
        case EXPR_CALL:   return evalcall((callexpr*)e, env);
        case EXPR_INDEX:  return evalindex((indexexpr*)e, env);
        case EXPR_ATTR:   return evalattr((attribexpr*)e, env);
        case EXPR_LIST: {
            listexpr *le = (listexpr*)e;
            object *list = makelist();
            gcaddroot(list);
            for (int i = 0; i < le->count; i++) {
                listappend(list, evalexpr(le->items[i], env));
            }
            gcremoveroot(list);
            return list;
        }
        case EXPR_DICT: {
            dictexpr *de = (dictexpr*)e;
            object *dict = makedict();
            gcaddroot(dict);
            for (int i = 0; i < de->count; i++) {
                object *k = evalexpr(de->keys[i], env);
                gcaddroot(k);
                object *v = evalexpr(de->values[i], env);
                gcremoveroot(k);
                dictset(dict, k, v);
            }
            gcremoveroot(dict);
            return dict;
        }
    }
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
        gcaddroot(list);
        for (long i = iterable->range.start; i < iterable->range.stop; i += iterable->range.step) {
            listappend(list, makeint(i));
        }
    } else if (iterable->type == 5) {
        list = iterable;
    } else if (iterable->type == 2) {
        list = makelist();
        gcaddroot(list);
        for (int i = 0; iterable->sval[i]; i++) {
            char buf[2] = {iterable->sval[i], 0};
            listappend(list, makestring(buf));
        }
    } else {
        throwexception("for loop expects iterable");
        return NULL;
    }
    int isnew = (iterable->type != 5);
    for (int i = 0; i < list->list.count; i++) {
        environment *blockenv = envnew(env);
        envset(blockenv, s->var, list->list.items[i], 0);
        for (int j = 0; j < s->body->count; j++) {
            evalstmt(s->body->items[j], blockenv);
        }
    }
    if (isnew) gcremoveroot(list);
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

static object *evaltry(tryexpr *s, environment *env) {
    frame f;
    frame *oldframe = currentframe;
    currentframe = &f;
    object *result = makenone();
    if (setjmp(f.jump) == 0) {
        environment *blockenv = envnew(env);
        for (int i = 0; i < s->body->count; i++) {
            evalstmt(s->body->items[i], blockenv);
        }
    } else {
        object *ex = catchexception();
        if (s->catchbody) {
            environment *catchenv = envnew(env);
            if (s->catchvar) envset(catchenv, s->catchvar, ex, 0);
            for (int i = 0; i < s->catchbody->count; i++) {
                evalstmt(s->catchbody->items[i], catchenv);
            }
        } else {
            currentframe = oldframe;
            if (ex && ex->type == 2) throwexception(ex->sval);
            throwexception("unhandled exception");
        }
    }
    if (s->finallybody) {
        environment *finenv = envnew(env);
        for (int i = 0; i < s->finallybody->count; i++) {
            evalstmt(s->finallybody->items[i], finenv);
        }
    }
    currentframe = oldframe;
    return result;
}

static object *evalthrow(throwexpr *s, environment *env) {
    object *val = evalexpr(s->value, env);
    if (val->type == 2) throwexception(val->sval);
    throwexception("thrown value");
    return NULL;
}

static object *evalexprstmt(exprstmt *s, environment *env) {
    return evalexpr(s->expression, env);
}

static object *evalstmt(stmt *s, environment *env) {
    switch (s->type) {
        case STMT_LET:     return evallet((letexpr*)s, env);
        case STMT_RETURN:  return evalreturn((returnexpr*)s, env);
        case STMT_IF:      return evalif((ifexpr*)s, env);
        case STMT_WHILE:   return evalwhile((whileexpr*)s, env);
        case STMT_FOR:     return evalfor((forexp*)s, env);
        case STMT_BREAK:   return makenone();
        case STMT_NEXT:    return makenone();
        case STMT_FN:      return evalfn((fnexp*)s, env);
        case STMT_CLASS:   return evalclass((classexp*)s, env);
        case STMT_EXPR:    return evalexprstmt((exprstmt*)s, env);
        case STMT_TRY:     return evaltry((tryexpr*)s, env);
        case STMT_THROW:   return evalthrow((throwexpr*)s, env);
    }
    throwexception("unknown statement");
    return NULL;
}

object *callfunc(object *fn, object **args, int argc, environment *env) {
    if (fn->type == 8) {
        return fn->builtin.fn(args, argc, env);
    }
    if (fn->type == 7) {
        if (argc != fn->func.pcount) {
            char msg[128];
            snprintf(msg, sizeof(msg), "expected %d args got %d", fn->func.pcount, argc);
            throwexception(msg);
            return NULL;
        }
        environment *callenv = envnew(fn->func.closure);
        for (int i = 0; i < argc; i++) {
            envset(callenv, fn->func.params[i], args[i], 0);
        }
        frame f;
        frame *oldframe = currentframe;
        currentframe = &f;
        f.result = makenone();
        f.file = fn->func.name;
        object *result = makenone();
        gcaddroot(result);
        if (setjmp(f.jump) == 0) {
            for (int i = 0; i < fn->func.bcount; i++) {
                evalstmt(fn->func.body[i], callenv);
            }
        } else {
            result = f.result;
        }
        gcremoveroot(result);
        currentframe = oldframe;
        return result;
    }
    if (fn->type == 9) {
        object *inst = makeinstance(fn, args, argc);
        gcaddroot(inst);
        object *init = envget(fn->klass.attrs, "init");
        if (init && init->type == 7) {
            callfunc(init, args, argc, fn->klass.attrs);
        }
        gcremoveroot(inst);
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
