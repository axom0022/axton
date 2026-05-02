#include "axton.h"

static object *eval_expr(expr *e, environment *env);
static object *eval_stmt(stmt *s, environment *env);

static object *eval_binary(binary_expr *e, environment *env) {
    object *left = eval_expr(e->left, env);
    object *right = eval_expr(e->right, env);
    switch (e->op) {
        case TOK_PLUS: return add_values(left, right);
        case TOK_MINUS: return sub_values(left, right);
        case TOK_STAR: return mul_values(left, right);
        case TOK_SLASH: return div_values(left, right);
        case TOK_EQEQ: return make_bool(values_equal(left, right));
        case TOK_NE: return make_bool(!values_equal(left, right));
        case TOK_LT: return make_bool(less_than(left, right));
        case TOK_GT: return make_bool(greater_than(left, right));
        case TOK_AND: return is_truthy(left) ? right : left;
        case TOK_OR: return is_truthy(left) ? left : right;
        default: throw_exception("bad operator"); return NULL;
    }
}

static object *eval_unary(unary_expr *e, environment *env) {
    object *operand = eval_expr(e->operand, env);
    if (e->op == TOK_NOT) return make_bool(!is_truthy(operand));
    if (e->op == TOK_MINUS) {
        if (operand->type == 0) return make_int(-operand->ival);
        if (operand->type == 1) return make_float(-operand->fval);
        throw_exception("bad operand for minus");
    }
    throw_exception("bad unary");
    return NULL;
}

static object *eval_ident(ident_expr *e, environment *env) {
    object *v = env_get(env, e->name);
    if (!v) {
        char msg[256];
        snprintf(msg, sizeof(msg), "name '%s' not defined", e->name);
        throw_exception(msg);
    }
    return v;
}

static object *eval_number(number_expr *e, environment *env) {
    if (e->value == (double)(long)e->value) return make_int((long)e->value);
    return make_float(e->value);
}

static object *eval_string(string_expr *e, environment *env) {
    return make_string(e->value);
}

static object *eval_bool(bool_expr *e, environment *env) {
    return make_bool(e->value);
}

static object *eval_none(none_expr *e, environment *env) {
    return make_none();
}

static object *eval_call(call_expr *e, environment *env) {
    object *callee = eval_expr(e->callee, env);
    object **args = malloc(sizeof(object*) * e->args->count);
    for (int i = 0; i < e->args->count; i++) {
        args[i] = eval_expr(e->args->items[i], env);
    }
    object *result = call_function(callee, args, e->args->count, env);
    free(args);
    return result;
}

static object *eval_expr(expr *e, environment *env) {
    if (((binary_expr*)e)->op) return eval_binary((binary_expr*)e, env);
    if (((unary_expr*)e)->op) return eval_unary((unary_expr*)e, env);
    if (((ident_expr*)e)->name) return eval_ident((ident_expr*)e, env);
    if (((number_expr*)e)->value) return eval_number((number_expr*)e, env);
    if (((string_expr*)e)->value) return eval_string((string_expr*)e, env);
    if (((bool_expr*)e)->value) return eval_bool((bool_expr*)e, env);
    if (((none_expr*)e)) return eval_none((none_expr*)e, env);
    if (((call_expr*)e)->callee) return eval_call((call_expr*)e, env);
    throw_exception("unknown expression");
    return NULL;
}

static object *eval_let(let_stmt *s, environment *env) {
    object *val = eval_expr(s->value, env);
    env_set(env, s->name, val, s->isconst);
    return make_none();
}

static object *eval_return(return_stmt *s, environment *env) {
    object *val = s->value ? eval_expr(s->value, env) : make_none();
    if (currentframe) {
        currentframe->result = val;
        longjmp(currentframe->jump, 1);
    }
    return val;
}

static object *eval_if(if_stmt *s, environment *env) {
    if (is_truthy(eval_expr(s->cond, env))) {
        environment *blockenv = env_new(env);
        for (int i = 0; i < s->body->count; i++) eval_stmt(s->body->items[i], blockenv);
        return make_none();
    }
    for (int i = 0; i < s->elif_count; i++) {
        if (is_truthy(eval_expr(s->elifs[i].cond, env))) {
            environment *blockenv = env_new(env);
            for (int j = 0; j < s->elifs[i].body->count; j++) eval_stmt(s->elifs[i].body->items[j], blockenv);
            return make_none();
        }
    }
    if (s->else_body) {
        environment *blockenv = env_new(env);
        for (int i = 0; i < s->else_body->count; i++) eval_stmt(s->else_body->items[i], blockenv);
    }
    return make_none();
}

static object *eval_while(while_stmt *s, environment *env) {
    while (is_truthy(eval_expr(s->cond, env))) {
        environment *blockenv = env_new(env);
        for (int i = 0; i < s->body->count; i++) eval_stmt(s->body->items[i], blockenv);
    }
    return make_none();
}

static object *eval_for(for_stmt *s, environment *env) {
    object *iterable = eval_expr(s->iter, env);
    object *list = NULL;
    if (iterable->type == 11) {
        list = make_list();
        for (long i = iterable->range.start; i < iterable->range.stop; i += iterable->range.step) {
            list_append(list, make_int(i));
        }
    } else if (iterable->type == 5) {
        list = iterable;
    } else {
        throw_exception("for needs iterable");
        return NULL;
    }
    for (int i = 0; i < list->list.count; i++) {
        environment *blockenv = env_new(env);
        env_set(blockenv, s->var, list->list.items[i], 0);
        for (int j = 0; j < s->body->count; j++) eval_stmt(s->body->items[j], blockenv);
    }
    return make_none();
}

static object *eval_break(break_stmt *s, environment *env) {
    return make_none();
}

static object *eval_next(next_stmt *s, environment *env) {
    return make_none();
}

static object *eval_fn(fn_stmt *s, environment *env) {
    object *fn = make_func(s->params, s->pcount, (stmt**)s->body->items, s->body->count, env, s->name);
    env_set(env, s->name, fn, 0);
    return make_none();
}

static object *eval_class(class_stmt *s, environment *env) {
    environment *classenv = env_new(env);
    for (int i = 0; i < s->body->count; i++) eval_stmt(s->body->items[i], classenv);
    object *cls = make_class(s->name, classenv, NULL);
    env_set(env, s->name, cls, 0);
    return make_none();
}

static object *eval_expr_stmt(expr_stmt *s, environment *env) {
    return eval_expr(s->expression, env);
}

static object *eval_stmt(stmt *s, environment *env) {
    if (((let_stmt*)s)->name) return eval_let((let_stmt*)s, env);
    if (((return_stmt*)s)->value != NULL || ((return_stmt*)s)->value == NULL) 
        return eval_return((return_stmt*)s, env);
    if (((if_stmt*)s)->cond) return eval_if((if_stmt*)s, env);
    if (((while_stmt*)s)->cond) return eval_while((while_stmt*)s, env);
    if (((for_stmt*)s)->var) return eval_for((for_stmt*)s, env);
    if (((break_stmt*)s)) return eval_break((break_stmt*)s, env);
    if (((next_stmt*)s)) return eval_next((next_stmt*)s, env);
    if (((fn_stmt*)s)->name) return eval_fn((fn_stmt*)s, env);
    if (((class_stmt*)s)->name) return eval_class((class_stmt*)s, env);
    if (((expr_stmt*)s)->expression) return eval_expr_stmt((expr_stmt*)s, env);
    throw_exception("unknown statement");
    return NULL;
}

object *call_function(object *fn, object **args, int argc, environment *env) {
    if (fn->type == 8) {
        return fn->builtin.fn(args, argc, env);
    }
    if (fn->type == 7) {
        if (argc != fn->func.pcount) throw_exception("wrong argument count");
        environment *callenv = env_new(fn->func.closure);
        for (int i = 0; i < argc; i++) env_set(callenv, fn->func.params[i], args[i], 0);
        frame f;
        frame *oldframe = currentframe;
        currentframe = &f;
        object *result = make_none();
        if (setjmp(f.jump) == 0) {
            for (int i = 0; i < fn->func.bcount; i++) eval_stmt(fn->func.body[i], callenv);
        } else {
            result = f.result;
        }
        currentframe = oldframe;
        return result;
    }
    if (fn->type == 9) {
        object *inst = make_instance(fn);
        object *init = env_get(fn->klass.attrs, "init");
        if (init && init->type == 7) {
            call_function(init, args, argc, fn->klass.attrs);
        }
        return inst;
    }
    throw_exception("not callable");
    return NULL;
}

object *eval_program(stmt *program, environment *env) {
    stmt_list *list = (stmt_list*)program;
    for (int i = 0; i < list->count; i++) eval_stmt(list->items[i], env);
    return make_none();
}
