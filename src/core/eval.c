#include "axton.h"
#include "bytecode.h"
#include <math.h>

static object *evalexpr(expr *e, environment *env);
static object *evalstmt(stmt *s, environment *env);

static object *evalbinary(binaryexpr *e, environment *env) {
    object *left = evalexpr(e->left, env);
    gcaddroot(left);
    object *right = evalexpr(e->right, env);
    gcaddroot(right);
    object *result = NULL;
    switch (e->op) {
        case TOKPLUS: result = addvalues(left, right); break;
        case TOKMINUS: result = subvalues(left, right); break;
        case TOKSTAR: result = mulvalues(left, right); break;
        case TOKSLASH: result = divvalues(left, right); break;
        case TOKPERCENT: result = modvalues(left, right); break;
        case TOKPOWER: result = powvalues(left, right); break;
        case TOKEQEQ: result = makebool(valuesequal(left, right)); break;
        case TOKNE: result = makebool(!valuesequal(left, right)); break;
        case TOKLT: result = makebool(lessthan(left, right)); break;
        case TOKGT: result = makebool(greaterthan(left, right)); break;
        case TOKLE: result = makebool(!greaterthan(left, right)); break;
        case TOKGE: result = makebool(!lessthan(left, right)); break;
        case TOKAND: result = istruthy(left) ? right : left; break;
        case TOKOR: result = istruthy(left) ? left : right; break;
        default:
            gcremoveroot(left);
            gcremoveroot(right);
            throwexception("unknown binary operator");
            return NULL;
    }
    gcremoveroot(left);
    gcremoveroot(right);
    return result;
}

static object *evalunary(unaryexpr *e, environment *env) {
    object *operand = evalexpr(e->operand, env);
    gcaddroot(operand);
    object *result = NULL;
    if (e->op == TOKNOT) result = makebool(!istruthy(operand));
    else if (e->op == TOKMINUS) {
        if (otype(operand) == 0) result = makeint(-getival(operand));
        else if (otype(operand) == 1) result = makefloat(-operand->fval);
        else {
            gcremoveroot(operand);
            throwexception("bad operand for unary minus");
            return NULL;
        }
    } else {
        gcremoveroot(operand);
        throwexception("unknown unary operator");
        return NULL;
    }
    gcremoveroot(operand);
    return result;
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
    object *self = NULL;
    if (e->callee->type == exprattr) {
        self = evalexpr(((attribexpr*)e->callee)->target, env);
        gcaddroot(self);
    }
    object *callee = evalexpr(e->callee, env);
    gcaddroot(callee);
    if (!callee) throwexception("call on undefined");
    object **args = malloc(sizeof(object*) * (e->args->count + 1));
    for (int i = 0; i < e->args->count; i++) {
        args[i] = evalexpr(e->args->items[i], env);
        gcaddroot(args[i]);
    }
    object *result = callfunc(callee, args, e->args->count, env, self);
    for (int i = e->args->count - 1; i >= 0; i--) gcremoveroot(args[i]);
    gcremoveroot(callee);
    if (self) gcremoveroot(self);
    free(args);
    return result;
}

static object *evalindex(indexexpr *e, environment *env) {
    object *target = evalexpr(e->target, env);
    gcaddroot(target);
    object *idx = evalexpr(e->index, env);
    gcaddroot(idx);
    object *result = NULL;
    if (target->type == 5) {
        if (otype(idx) != 0) {
            gcremoveroot(target); gcremoveroot(idx);
            throwexception("list index must be integer");
            return NULL;
        }
        long i = getival(idx);
        if (i < 0) i = target->list.count + i;
        if (i < 0 || i >= target->list.count) {
            gcremoveroot(target); gcremoveroot(idx);
            throwexception("index out of range");
            return NULL;
        }
        result = target->list.items[i];
    } else if (target->type == 6) {
        result = dictget(target, idx);
        if (!result) {
            gcremoveroot(target); gcremoveroot(idx);
            throwexception("key not found");
            return NULL;
        }
    } else if (target->type == 2) {
        if (otype(idx) != 0) {
            gcremoveroot(target); gcremoveroot(idx);
            throwexception("string index must be integer");
            return NULL;
        }
        long i = getival(idx);
        if (i < 0) i = strlen(target->sval) + i;
        if (i < 0 || i >= (long)strlen(target->sval)) {
            gcremoveroot(target); gcremoveroot(idx);
            throwexception("index out of range");
            return NULL;
        }
        char c = target->sval[i];
        char buf[2] = {c, 0};
        result = makestring(buf);
    } else {
        gcremoveroot(target);
        gcremoveroot(idx);
        throwexception("cannot index");
        return NULL;
    }
    gcremoveroot(target);
    gcremoveroot(idx);
    return result;
}

static object *evalattr(attribexpr *e, environment *env) {
    object *target = evalexpr(e->target, env);
    gcaddroot(target);
    object *result = NULL;
    if (target->type == 9 && target->klass.attrs) {
        result = envget((environment*)target->klass.attrs, e->attr);
        if (result) { gcremoveroot(target); return result; }
    }
    if (target->type == 10 && target->instance.attrs) {
        result = envget((environment*)target->instance.attrs, e->attr);
        if (!result && target->instance.klass && target->instance.klass->klass.attrs) {
            result = envget((environment*)target->instance.klass->klass.attrs, e->attr);
        }
        if (result) { gcremoveroot(target); return result; }
    }
    if (target->type == 12 && target->module.exports) {
        result = envget((environment*)target->module.exports, e->attr);
        if (result) { gcremoveroot(target); return result; }
    }
    gcremoveroot(target);
    char msg[256];
    snprintf(msg, sizeof(msg), "attribute '%s' not found", e->attr);
    throwexception(msg);
    return NULL;
}

static object *evalassign(assignexpr *e, environment *env) {
    object *val = evalexpr(e->value, env);
    gcaddroot(val);
    expr *t = e->target;
    object *target = NULL;
    object *idx = NULL;
    if (t->type == exprident) {
        envupdate(env, ((identexpr*)t)->name, val);
    } else if (t->type == exprattr) {
        attribexpr *a = (attribexpr*)t;
        target = evalexpr(a->target, env);
        gcaddroot(target);
        if (target->type == 10) {
            if (!target->instance.attrs) target->instance.attrs = envnew(NULL);
            envset((environment*)target->instance.attrs, a->attr, val, 0);
        } else if (target->type == 9) {
            if (!target->klass.attrs) target->klass.attrs = envnew(NULL);
            envset((environment*)target->klass.attrs, a->attr, val, 0);
        } else if (target->type == 12) {
            if (!target->module.exports) target->module.exports = envnew(NULL);
            envset((environment*)target->module.exports, a->attr, val, 0);
        } else {
            gcremoveroot(val);
            gcremoveroot(target);
            throwexception("cannot set attribute on this type");
            return NULL;
        }
    } else if (t->type == exprindex) {
        indexexpr *ix = (indexexpr*)t;
        target = evalexpr(ix->target, env);
        gcaddroot(target);
        idx = evalexpr(ix->index, env);
        gcaddroot(idx);
        if (target->type == 5) {
            if (otype(idx) != 0) {
                gcremoveroot(val); gcremoveroot(target); gcremoveroot(idx);
                throwexception("list index must be integer");
                return NULL;
            }
            listset(target, getival(idx), val);
        } else if (target->type == 6) {
            dictset(target, idx, val);
        } else {
            gcremoveroot(val);
            gcremoveroot(target);
            gcremoveroot(idx);
            throwexception("cannot index-assign this type");
            return NULL;
        }
    } else {
        gcremoveroot(val);
        throwexception("invalid assignment target");
        return NULL;
    }
    if (target) gcremoveroot(target);
    if (idx) gcremoveroot(idx);
    gcremoveroot(val);
    return val;
}

static object *evalexpr(expr *e, environment *env) {
    switch (e->type) {
        case exprbinary: return evalbinary((binaryexpr*)e, env);
        case exprunary:  return evalunary((unaryexpr*)e, env);
        case exprident:  return evalident((identexpr*)e, env);
        case exprnumber: return evalnumber((numberexpr*)e, env);
        case exprstring: return evalstring((stringexpr*)e, env);
        case exprbool:   return evalbool((boolexpr*)e, env);
        case exprnone:   return makenone();
        case exprcall:   return evalcall((callexpr*)e, env);
        case exprindex:  return evalindex((indexexpr*)e, env);
        case exprattr:   return evalattr((attribexpr*)e, env);
        case exprfn: {
            fnexp *f = ((fnextexpr*)e)->fn;
            return makefunc(f->params, NULL, f->pcount, (stmt**)f->body->items, f->body->count, env, f->name ? f->name : "anonymous", 0, 0);
        }
        case exprclass: return evalclassexpr((stmt*)((classexpr*)e)->cls, env);
        case exprassign: return evalassign((assignexpr*)e, env);
        case exprlistlit: {
            listexpr *le = (listexpr*)e;
            object *list = makelist();
            gcaddroot(list);
            for (int i = 0; i < le->count; i++) {
                listappend(list, evalexpr(le->items[i], env));
            }
            gcremoveroot(list);
            return list;
        }
        case exprdict: {
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
        gcaddrootenv(blockenv);
        for (int i = 0; i < s->body->count; i++) {
            evalstmt(s->body->items[i], blockenv);
        }
        gcremoverootenv(blockenv);
        return makenone();
    }
    for (int i = 0; i < s->elifcount; i++) {
        if (istruthy(evalexpr(s->elifs[i].cond, env))) {
            environment *blockenv = envnew(env);
            gcaddrootenv(blockenv);
            for (int j = 0; j < s->elifs[i].body->count; j++) {
                evalstmt(s->elifs[i].body->items[j], blockenv);
            }
            gcremoverootenv(blockenv);
            return makenone();
        }
    }
    if (s->elsebody) {
        environment *blockenv = envnew(env);
        gcaddrootenv(blockenv);
        for (int i = 0; i < s->elsebody->count; i++) {
            evalstmt(s->elsebody->items[i], blockenv);
        }
        gcremoverootenv(blockenv);
    }
    return makenone();
}

static object *evalwhile(whileexpr *s, environment *env) {
    while (istruthy(evalexpr(s->cond, env))) {
        environment *blockenv = envnew(env);
        gcaddrootenv(blockenv);
        for (int i = 0; i < s->body->count; i++) {
            evalstmt(s->body->items[i], blockenv);
        }
        gcremoverootenv(blockenv);
    }
    return makenone();
}

static object *evalfor(forexp *s, environment *env) {
    object *iterable = evalexpr(s->iter, env);
    gcaddroot(iterable);
    if (otype(iterable) == 11) {
        long rstart = iterable->range.start;
        long rstop = iterable->range.stop;
        long rstep = iterable->range.step;
        for (long i = rstart; i < rstop; i += rstep) {
            environment *blockenv = envnew(env);
            gcaddrootenv(blockenv);
            envset(blockenv, s->var, makeint(i), 0);
            for (int j = 0; j < s->body->count; j++) {
                evalstmt(s->body->items[j], blockenv);
            }
            gcremoverootenv(blockenv);
        }
    } else if (otype(iterable) == 5) {
        for (int i = 0; i < iterable->list.count; i++) {
            environment *blockenv = envnew(env);
            gcaddrootenv(blockenv);
            envset(blockenv, s->var, iterable->list.items[i], 0);
            for (int j = 0; j < s->body->count; j++) {
                evalstmt(s->body->items[j], blockenv);
            }
            gcremoverootenv(blockenv);
        }
    } else if (otype(iterable) == 2) {
        int slen = (int)strlen(iterable->sval);
        for (int i = 0; i < slen; i++) {
            environment *blockenv = envnew(env);
            gcaddrootenv(blockenv);
            char cbuf[2] = {iterable->sval[i], 0};
            envset(blockenv, s->var, makestring(cbuf), 0);
            for (int j = 0; j < s->body->count; j++) {
                evalstmt(s->body->items[j], blockenv);
            }
            gcremoverootenv(blockenv);
        }
    } else {
        gcremoveroot(iterable);
        throwexception("for loop expects iterable");
        return NULL;
    }
    gcremoveroot(iterable);
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
    memset(&f, 0, sizeof(f));
    frame *oldframe = currentframe;
    currentframe = &f;
    object *result = makenone();
    environment *blockenv = NULL;
    if (setjmp(f.jump) == 0) {
        blockenv = envnew(env);
        gcaddrootenv(blockenv);
        for (int i = 0; i < s->body->count; i++) {
            evalstmt(s->body->items[i], blockenv);
        }
        gcremoverootenv(blockenv);
        blockenv = NULL;
    } else {
        if (blockenv) {
            gcremoverootenv(blockenv);
            blockenv = NULL;
        }
        object *ex = catchexception();
        if (s->catchbody) {
            environment *catchenv = envnew(env);
            gcaddrootenv(catchenv);
            if (s->catchvar) envset(catchenv, s->catchvar, ex, 0);
            for (int i = 0; i < s->catchbody->count; i++) {
                evalstmt(s->catchbody->items[i], catchenv);
            }
            gcremoverootenv(catchenv);
        } else {
            currentframe = oldframe;
            if (ex && otype(ex) == 2) throwexception(ex->sval);
            throwexception("unhandled exception");
        }
    }
    if (s->finallybody) {
        environment *finenv = envnew(env);
        gcaddrootenv(finenv);
        for (int i = 0; i < s->finallybody->count; i++) {
            evalstmt(s->finallybody->items[i], finenv);
        }
        gcremoverootenv(finenv);
    }
    currentframe = oldframe;
    return result;
}

static object *evalthrow(throwexpr *s, environment *env) {
    object *val = evalexpr(s->value, env);
    if (otype(val) == 2) throwexception(val->sval);
    throwexception("thrown value");
    return NULL;
}

static object *evalexprstmt(exprstmt *s, environment *env) {
    return evalexpr(s->expression, env);
}

static object *evalstmt(stmt *s, environment *env) {
    switch (s->type) {
        case stmtlet:     return evallet((letexpr*)s, env);
        case stmtreturn:  return evalreturn((returnexpr*)s, env);
        case stmtif:      return evalif((ifexpr*)s, env);
        case stmtwhile:   return evalwhile((whileexpr*)s, env);
        case stmtfor:     return evalfor((forexp*)s, env);
        case stmtbreak:   return makenone();
        case stmtnext:    return makenone();
        case stmtfn:      return evalfn((fnexp*)s, env);
        case stmtclass:   return evalclass((classexp*)s, env);
        case stmtexpr:    return evalexprstmt((exprstmt*)s, env);
        case stmttry:     return evaltry((tryexpr*)s, env);
        case stmtthrow:   return evalthrow((throwexpr*)s, env);
    }
    throwexception("unknown statement");
    return NULL;
}

object *evaltrystmt(stmt *s, environment *env) {
    return evaltry((tryexpr*)s, env);
}

object *evalclassstmt(stmt *s, environment *env) {
    return evalclass((classexp*)s, env);
}

object *evalclassexpr(stmt *s, environment *env) {
    classexp *c = (classexp*)s;
    environment *classenv = envnew(env);
    gcaddrootenv(classenv);
    for (int i = 0; i < c->body->count; i++) {
        evalstmt(c->body->items[i], classenv);
    }
    object *klass = makeclass(c->name ? c->name : "anonymous", classenv, NULL);
    gcremoverootenv(classenv);
    return klass;
}

object *callfunc(object *fn, object **args, int argc, environment *env, object *self) {
    if (otype(fn) == 8) {
        return fn->builtin.fn(args, argc, env);
    }
    if (otype(fn) == 7) {
        return axvmcall(fn, args, argc, env, self);
    }
    if (otype(fn) == 9) {
        object *inst = makeinstance(fn, args, argc);
        gcaddroot(inst);
        object *init = envget(fn->klass.attrs, "init");
        if (init && init->type == 7) {
            callfunc(init, args, argc, fn->klass.attrs, inst);
        }
        gcremoveroot(inst);
        return inst;
    }
    throwexception("object is not callable");
    return NULL;
}

object *evalprogram(stmt *program, environment *env) {
    stmtlist *list = (stmtlist*)program;
    bytecode *bc = axcompileprog(list->items, list->count);
    if (axprogrambc) axprogretire(axprogrambc);
    axprogrambc = bc;
    axprogprune();
    return axvmrun(bc, env);
}
