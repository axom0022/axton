#include "axton.h"
#include "bytecode.h"
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

object **axvmstack = NULL;
int axvmsp = 0;
static int axvmstackcap = 0;
static int axvmdepth = 0;

static unsigned int vmhash(const char *s) {
    unsigned int h = 2166136261u;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h;
}

static void vmgrow(void) {
    axvmstackcap = axvmstackcap ? axvmstackcap * 2 : 1024;
    axvmstack = realloc(axvmstack, axvmstackcap * sizeof(object*));
}

#define vmpush(v) do { \
    if (axvmsp >= axvmstackcap) vmgrow(); \
    axvmstack[axvmsp++] = (v); \
} while (0)

static object *vmgetattr(object *target, char *name) {
    object *result = NULL;
    if (otype(target) == 9 && target->klass.attrs) {
        result = envget((environment*)target->klass.attrs, name);
        if (result) return result;
    }
    if (otype(target) == 10 && target->instance.attrs) {
        result = envget((environment*)target->instance.attrs, name);
        if (!result && target->instance.klass && target->instance.klass->klass.attrs) {
            result = envget((environment*)target->instance.klass->klass.attrs, name);
        }
        if (result) return result;
    }
    if (otype(target) == 12 && target->module.exports) {
        result = envget((environment*)target->module.exports, name);
        if (result) return result;
    }
    char msg[256];
    snprintf(msg, sizeof(msg), "attribute '%s' not found", name);
    throwexception(msg);
    return NULL;
}

static void vmsetattr(object *target, char *name, object *val) {
    if (otype(target) == 10) {
        if (!target->instance.attrs) target->instance.attrs = envnew(NULL);
        envset((environment*)target->instance.attrs, name, val, 0);
        return;
    }
    if (otype(target) == 9) {
        if (!target->klass.attrs) target->klass.attrs = envnew(NULL);
        envset((environment*)target->klass.attrs, name, val, 0);
        return;
    }
    if (otype(target) == 12) {
        if (!target->module.exports) target->module.exports = envnew(NULL);
        envset((environment*)target->module.exports, name, val, 0);
        return;
    }
    throwexception("cannot set attribute on this type");
}

static object *vmgetindex(object *target, object *idx) {
    if (otype(target) == 5) {
        if (otype(idx) != 0) throwexception("list index must be integer");
        long i = getival(idx);
        if (i < 0) i = target->list.count + i;
        if (i < 0 || i >= target->list.count) throwexception("index out of range");
        return target->list.items[i];
    }
    if (otype(target) == 6) {
        object *r = dictget(target, idx);
        if (!r) throwexception("key not found");
        return r;
    }
    if (otype(target) == 2) {
        if (otype(idx) != 0) throwexception("string index must be integer");
        long i = getival(idx);
        if (i < 0) i = (long)strlen(target->sval) + i;
        if (i < 0 || i >= (long)strlen(target->sval)) throwexception("index out of range");
        char buf[2] = {target->sval[i], 0};
        return makestring(buf);
    }
    throwexception("cannot index");
    return NULL;
}

static void vmsetindex(object *target, object *idx, object *val) {
    if (otype(target) == 5) {
        if (otype(idx) != 0) throwexception("list index must be integer");
        listset(target, getival(idx), val);
        return;
    }
    if (otype(target) == 6) {
        dictset(target, idx, val);
        return;
    }
    throwexception("cannot index-assign this type");
}

static object *vmlookup(environment *env, char *name, int *gcache) {
    environment *g = env->globals;
    int cache = *gcache;
    if (cache > 0) {
        int idx = cache - 1;
        if (idx < g->lcount && g->lnames[idx] && strcmp(g->lnames[idx], name) == 0) {
            return g->lvals[idx];
        }
    } else if (cache < 0) {
        int raw = -cache - 1;
        envslot *sl = &g->slots[raw & (g->cap - 1)];
        if (sl->used && sl->hash == vmhash(name) && strcmp(sl->name, name) == 0) {
            return sl->value;
        }
    }
    unsigned int h = vmhash(name);
    environment *e = env;
    while (e) {
        for (int i = 0; i < e->lcount; i++) {
            if (e->lnames[i] && strcmp(e->lnames[i], name) == 0) {
                object *v = e->lvals[i];
                if (e == g) *gcache = i + 1;
                return v;
            }
        }
        unsigned int idx = h & (e->cap - 1);
        unsigned int start = idx;
        while (e->slots[idx].used) {
            if (e->slots[idx].hash == h && strcmp(e->slots[idx].name, name) == 0) {
                object *v = e->slots[idx].value;
                if (e == g) *gcache = -(int)(idx + 1);
                return v;
            }
            idx = (idx + 1) & (e->cap - 1);
            if (idx == start) break;
        }
        e = e->parent;
    }
    return NULL;
}

static void vmupdatestore(environment *env, char *name, object *val) {
    unsigned int h = vmhash(name);
    environment *e = env;
    while (e) {
        for (int i = 0; i < e->lcount; i++) {
            if (e->lnames[i] && strcmp(e->lnames[i], name) == 0) {
                if (e->lconst && e->lconst[i]) throwexception("cannot reassign constant");
                e->lvals[i] = val;
                return;
            }
        }
        unsigned int idx = h & (e->cap - 1);
        unsigned int start = idx;
        while (e->slots[idx].used) {
            if (e->slots[idx].hash == h && strcmp(e->slots[idx].name, name) == 0) {
                if (e->slots[idx].isconst) throwexception("cannot reassign constant");
                e->slots[idx].value = val;
                return;
            }
            idx = (idx + 1) & (e->cap - 1);
            if (idx == start) break;
        }
        e = e->parent;
    }
    envset(env, name, val, 0);
}

static object **lvfreelist[12];
static int lvfreecount = 0;

void axlvpush(object **lv, int cap) {
    if (!lv) return;
    if (lvfreecount >= 65536) {
        free(lv);
        return;
    }
    int cls = 0;
    int p = 4;
    while (p < cap && cls < 11) {
        p *= 2;
        cls++;
    }
    lv[cap] = (object*)lvfreelist[cls];
    lvfreelist[cls] = lv;
    lvfreecount++;
}

object **axlvpop(int n) {
    int need = n > 4 ? n : 4;
    int cls = 0;
    int p = 4;
    while (p < need && cls < 11) {
        p *= 2;
        cls++;
    }
    if (lvfreelist[cls]) {
        object **lv = lvfreelist[cls];
        lvfreelist[cls] = (object**)lv[4 << cls];
        lvfreecount--;
        return lv;
    }
    return calloc((size_t)p + 1, sizeof(object*));
}

static inline object *vmmkint(long v) {
    return (object*)(((intptr_t)v << 1) | 1);
}

static inline int vmtruthy(object *v) {
    if (!v) return 0;
    if (isintval(v)) return getival(v) != 0;
    if (isboolval(v)) return getbval(v);
    if (isnoneval(v)) return 0;
    switch (otype(v)) {
        case 1: return v->fval != 0;
        case 2: return v->sval && strlen(v->sval) > 0;
        case 5: return v->list.count > 0;
        case 6: return v->dict.count > 0;
        default: return 1;
    }
}

static object *vmrun(bytecode *bc, environment *env);

#define vmpoolsize 128
static environment *envpool[vmpoolsize];
static object **lvpool[vmpoolsize];
static int lvpoolcap[vmpoolsize];

object *axvmcall(object *fn, object **args, int argc, environment *env, object *self) {
    if (otype(fn) == 8) {
        return fn->builtin.fn(args, argc, env);
    }
    if (otype(fn) == 9) {
        object *inst = makeinstance(fn, args, argc);
        gcaddroot(inst);
        object *init = envget(fn->klass.attrs, "init");
        if (init && otype(init) == 7) {
            axvmcall(init, args, argc, fn->klass.attrs, inst);
        }
        gcremoveroot(inst);
        return inst;
    }
    if (otype(fn) == 7) {
        if (argc != fn->func.pcount) {
            char msg[128];
            snprintf(msg, sizeof(msg), "expected %d args got %d", fn->func.pcount, argc);
            throwexception(msg);
            return NULL;
        }
        if (!fn->func.code) {
            fn->func.code = axcompilefn(fn->func.name, fn->func.params, fn->func.pcount, fn->func.body, fn->func.bcount);
        }
        bytecode *bc = fn->func.code;
        if (++axvmdepth > 60000) {
            axvmdepth--;
            throwexception("stack overflow");
            return NULL;
        }
        int pi = axvmdepth - 1;
        int pooled = 0;
        environment *callenv = NULL;
        if (!bc->hascapture && pi < vmpoolsize) {
            callenv = envpool[pi];
            if (callenv) pooled = 1;
        }
        size_t rd = gcrootscount();
        size_t erd = gcrootenvscount();
        if (!callenv) {
            callenv = envnew(fn->func.closure);
            gcaddrootenv(callenv);
            if (!bc->hascapture && pi < vmpoolsize) {
                envpool[pi] = callenv;
                pooled = 1;
            }
        }
        callenv->parent = (environment*)fn->func.closure;
        callenv->globals = ((environment*)fn->func.closure)->globals;
        callenv->lnames = bc->slotnames;
        callenv->lconst = bc->slotconst;
        callenv->lcount = bc->slotcount;
        callenv->lownnames = 0;
        if (pooled && callenv->count > 0) {
            for (int j = 0; j < callenv->cap; j++) {
                if (callenv->slots[j].used) {
                    free(callenv->slots[j].name);
                    callenv->slots[j].used = 0;
                    callenv->slots[j].name = NULL;
                }
            }
        }
        callenv->count = 0;
        int lvneed = bc->slotcount > 0 ? bc->slotcount : 1;
        if (pooled && lvpool[pi] && lvpoolcap[pi] >= lvneed) {
            callenv->lvals = lvpool[pi];
            callenv->lvcap = lvpoolcap[pi];
        } else {
            if (pooled && lvpool[pi]) {
                axlvpush(lvpool[pi], lvpoolcap[pi]);
                lvpool[pi] = NULL;
            }
            callenv->lvals = axlvpop(lvneed);
            int lvp = 4;
            while (lvp < lvneed) lvp *= 2;
            callenv->lvcap = lvp;
            if (pooled) {
                lvpool[pi] = callenv->lvals;
                lvpoolcap[pi] = lvp;
            }
        }
        if (bc->usesthis || self) {
            callenv->lvals[0] = self ? self : makenone();
        }
        for (int i = 0; i < argc; i++) {
            callenv->lvals[i + 1] = args[i];
        }
        if (bc->slotcount > argc + 1) {
            memset(&callenv->lvals[argc + 1], 0, (size_t)(bc->slotcount - argc - 1) * sizeof(object*));
        }
        frame f;
        frame *oldframe = currentframe;
        currentframe = &f;
        f.result = makenone();
        f.file = fn->func.name;
        f.line = 0;
        f.prev = NULL;
        f.env = NULL;
        f.generator = NULL;
        object *result;
        if (setjmp(f.jump) == 0) {
            result = vmrun(bc, callenv);
        } else {
            result = f.result;
        }
        axvmdepth--;
        gctruncroots(rd);
        gctruncrootenvs(erd);
        currentframe = oldframe;
        if (!pooled) {
            gcremoverootenv(callenv);
            if (!bc->hascapture) {
                for (int j = 0; j < callenv->cap; j++) {
                    if (callenv->slots[j].used) {
                        free(callenv->slots[j].name);
                        callenv->slots[j].used = 0;
                        callenv->slots[j].name = NULL;
                    }
                }
                callenv->count = 0;
                axlvpush(callenv->lvals, callenv->lvcap);
                callenv->lvals = NULL;
                callenv->lcount = 0;
                callenv->lnames = NULL;
                callenv->lconst = NULL;
                if (callenv->prev) callenv->prev->next = callenv->next;
                else *envlisthead() = callenv->next;
                if (callenv->next) callenv->next->prev = callenv->prev;
                envrecycle(callenv);
            }
        }
        return result;
    }
    throwexception("object is not callable");
    return NULL;
}

object *axvmrun(bytecode *bc, environment *env) {
    bc->refs++;
    if (bc->istop) {
        int base = env->lcount;
        int need = base + bc->slotcount;
        if (need > env->lcount || !env->lvals) {
            int old = env->lcount;
            int p = 4;
            while (p < need) p *= 2;
            env->lvals = realloc(env->lvals, (p + 1) * sizeof(object*));
            memset(env->lvals + old, 0, (need - old) * sizeof(object*));
            env->lnames = realloc(env->lnames, need * sizeof(char*));
            env->lconst = realloc(env->lconst, need);
            env->lownnames = 1;
            env->lvcap = p;
        }
        for (int i = 0; i < bc->slotcount; i++) {
            env->lnames[base + i] = strdup(bc->slotnames[i]);
            env->lconst[base + i] = bc->slotconst[i];
        }
        env->lcount = need;
        bc->slotbase = base;
    }
    object *r = vmrun(bc, env);
    bc->refs--;
    return r;
}

static object *vmrun(bytecode *bc, environment *env) {
    unsigned char *pc = bc->code;
    int lbase = bc->slotbase;
    static void *dispatch[opcount] = {
        &&lnop,
        &&lloadconst, &&lloadint, &&lloadnone, &&lloadbool,
        &&lloadlocal, &&lstorelocal, &&lstorelocalc,
        &&lloadupvalue, &&lstoreupvalue,
        &&lloadglobal, &&lstoreglobal,
        &&lpop, &&ldup, &&lpopn,
        &&ladd, &&lsub, &&lmul, &&ldiv, &&lmod, &&lpow, &&lneg, &&lnot,
        &&leq, &&lne, &&llt, &&lgt, &&lle, &&lge,
        &&ljump, &&ljumpiffalse, &&ljumpiftrue,
        &&lcall, &&lcallattr, &&lreturn,
        &&lmakelist, &&lmakedict, &&lindex, &&lsetindex, &&lgetattr, &&lsetattr,
        &&lmakefunc, &&ltrystmt, &&lclassstmt, &&lclassexpr, &&lthrow,
        &&lforbegin, &&lfornext, &&lforrangebegin, &&lforrangenext,
        &&laddloc, &&ladd3, &&lsublocint, &&llltlocint
    };
#define VMNEXT() goto *dispatch[*pc++]
#define VMIMM() (pc += 4, (int)(((unsigned)pc[-4] << 24) | ((unsigned)pc[-3] << 16) | ((unsigned)pc[-2] << 8) | (unsigned)pc[-1]))
    VMNEXT();
lnop: VMNEXT();
lloadconst: { int i = VMIMM(); vmpush(bc->constants[i]); VMNEXT(); }
lloadint: { int v = VMIMM(); vmpush(vmmkint(v)); VMNEXT(); }
lloadnone: { vmpush(makenone()); VMNEXT(); }
lloadbool: { int v = VMIMM(); vmpush(makebool(v)); VMNEXT(); }
lloadlocal: {
    int s = VMIMM();
    object *v = env->lvals[lbase + s];
    if (!v) {
        char msg[300];
        snprintf(msg, sizeof(msg), "name '%s' not defined", bc->slotnames[s]);
        throwexception(msg);
    }
    vmpush(v);
    VMNEXT();
}
lstorelocal: { int s = VMIMM(); env->lvals[lbase + s] = axvmstack[--axvmsp]; VMNEXT(); }
lstorelocalc: { int s = VMIMM(); if (bc->slotconst[s]) throwexception("cannot reassign constant"); env->lvals[lbase + s] = axvmstack[--axvmsp]; VMNEXT(); }
lloadupvalue: {
    int ni = VMIMM();
    int ioff = (int)(pc - bc->code) - 5;
    char *name = bc->names[ni];
    object *v = vmlookup(env, name, &bc->gcache[ioff]);
    if (!v) {
        char msg[300];
        snprintf(msg, sizeof(msg), "name '%s' not defined", name);
        throwexception(msg);
    }
    vmpush(v);
    VMNEXT();
}
lstoreupvalue: {
    int ni = VMIMM();
    vmupdatestore(env, bc->names[ni], axvmstack[--axvmsp]);
    VMNEXT();
}
lloadglobal: {
    int ioff = (int)(pc - bc->code) - 5;
    int ni = VMIMM();
    char *name = bc->names[ni];
    object *v = vmlookup(env, name, &bc->gcache[ioff]);
    if (!v) {
        char msg[300];
        snprintf(msg, sizeof(msg), "name '%s' not defined", name);
        throwexception(msg);
    }
    vmpush(v);
    VMNEXT();
}
lstoreglobal: { int ni = VMIMM(); vmupdatestore(env, bc->names[ni], axvmstack[--axvmsp]); VMNEXT(); }
lpop: { axvmsp--; VMNEXT(); }
ldup: { vmpush(axvmstack[axvmsp - 1]); VMNEXT(); }
lpopn: { axvmsp -= VMIMM(); VMNEXT(); }
ladd: {
    axvmsp -= 2;
    object *a = axvmstack[axvmsp];
    object *b = axvmstack[axvmsp + 1];
    if (isintval(a) && isintval(b)) axvmstack[axvmsp] = vmmkint(getival(a) + getival(b));
    else axvmstack[axvmsp] = addvalues(a, b);
    axvmsp++;
    VMNEXT();
}
lsub: {
    axvmsp -= 2;
    object *a = axvmstack[axvmsp];
    object *b = axvmstack[axvmsp + 1];
    if (isintval(a) && isintval(b)) axvmstack[axvmsp] = vmmkint(getival(a) - getival(b));
    else axvmstack[axvmsp] = subvalues(a, b);
    axvmsp++;
    VMNEXT();
}
lmul: {
    axvmsp -= 2;
    object *a = axvmstack[axvmsp];
    object *b = axvmstack[axvmsp + 1];
    if (isintval(a) && isintval(b)) axvmstack[axvmsp] = vmmkint(getival(a) * getival(b));
    else axvmstack[axvmsp] = mulvalues(a, b);
    axvmsp++;
    VMNEXT();
}
ldiv: { axvmsp -= 2; axvmstack[axvmsp] = divvalues(axvmstack[axvmsp], axvmstack[axvmsp + 1]); axvmsp++; VMNEXT(); }
lmod: { axvmsp -= 2; axvmstack[axvmsp] = modvalues(axvmstack[axvmsp], axvmstack[axvmsp + 1]); axvmsp++; VMNEXT(); }
lpow: { axvmsp -= 2; axvmstack[axvmsp] = powvalues(axvmstack[axvmsp], axvmstack[axvmsp + 1]); axvmsp++; VMNEXT(); }
lneg: {
    object *v = axvmstack[axvmsp - 1];
    if (otype(v) == 0) axvmstack[axvmsp - 1] = makeint(-getival(v));
    else if (otype(v) == 1) axvmstack[axvmsp - 1] = makefloat(-v->fval);
    else throwexception("bad operand for unary minus");
    VMNEXT();
}
lnot: { axvmstack[axvmsp - 1] = makebool(!vmtruthy(axvmstack[axvmsp - 1])); VMNEXT(); }
leq: { axvmsp -= 2; axvmstack[axvmsp] = makebool(valuesequal(axvmstack[axvmsp], axvmstack[axvmsp + 1])); axvmsp++; VMNEXT(); }
lne: { axvmsp -= 2; axvmstack[axvmsp] = makebool(!valuesequal(axvmstack[axvmsp], axvmstack[axvmsp + 1])); axvmsp++; VMNEXT(); }
llt: { axvmsp -= 2; axvmstack[axvmsp] = makebool(lessthan(axvmstack[axvmsp], axvmstack[axvmsp + 1])); axvmsp++; VMNEXT(); }
lgt: { axvmsp -= 2; axvmstack[axvmsp] = makebool(greaterthan(axvmstack[axvmsp], axvmstack[axvmsp + 1])); axvmsp++; VMNEXT(); }
lle: { axvmsp -= 2; axvmstack[axvmsp] = makebool(!greaterthan(axvmstack[axvmsp], axvmstack[axvmsp + 1])); axvmsp++; VMNEXT(); }
lge: { axvmsp -= 2; axvmstack[axvmsp] = makebool(!lessthan(axvmstack[axvmsp], axvmstack[axvmsp + 1])); axvmsp++; VMNEXT(); }
ljump: { pc += VMIMM(); VMNEXT(); }
ljumpiffalse: {
    int off = VMIMM();
    object *v = axvmstack[--axvmsp];
    if (!vmtruthy(v)) pc += off;
    VMNEXT();
}
ljumpiftrue: {
    int off = VMIMM();
    object *v = axvmstack[--axvmsp];
    if (vmtruthy(v)) pc += off;
    VMNEXT();
}
lcall: {
    int argc = VMIMM();
    int base = axvmsp - argc - 1;
    object *fn = axvmstack[base];
    object *r = axvmcall(fn, &axvmstack[base + 1], argc, env, NULL);
    axvmsp = base;
    vmpush(r);
    VMNEXT();
}
lcallattr: {
    int ni = VMIMM();
    int argc = VMIMM();
    int base = axvmsp - argc - 1;
    object *target = axvmstack[base];
    object *fn = vmgetattr(target, bc->names[ni]);
    object *r = axvmcall(fn, &axvmstack[base + 1], argc, env, target);
    axvmsp = base;
    vmpush(r);
    VMNEXT();
}
lreturn: {
    object *r = axvmstack[--axvmsp];
    return r;
}
lmakelist: {
    int n = VMIMM();
    object *list = makelist();
    for (int i = 0; i < n; i++) listappend(list, axvmstack[axvmsp - n + i]);
    axvmsp -= n;
    vmpush(list);
    VMNEXT();
}
lmakedict: {
    int n = VMIMM();
    object *d = makedict();
    for (int i = 0; i < n; i++) {
        object *k = axvmstack[axvmsp - 2 * n + 2 * i];
        object *v = axvmstack[axvmsp - 2 * n + 2 * i + 1];
        dictset(d, k, v);
    }
    axvmsp -= 2 * n;
    vmpush(d);
    VMNEXT();
}
lindex: {
    object *idx = axvmstack[--axvmsp];
    object *target = axvmstack[--axvmsp];
    object *r = vmgetindex(target, idx);
    vmpush(r);
    VMNEXT();
}
lsetindex: {
    object *idx = axvmstack[--axvmsp];
    object *target = axvmstack[--axvmsp];
    object *val = axvmstack[--axvmsp];
    vmsetindex(target, idx, val);
    vmpush(val);
    VMNEXT();
}
lgetattr: {
    int ni = VMIMM();
    object *target = axvmstack[--axvmsp];
    vmpush(vmgetattr(target, bc->names[ni]));
    VMNEXT();
}
lsetattr: {
    int ni = VMIMM();
    object *target = axvmstack[--axvmsp];
    object *val = axvmstack[--axvmsp];
    vmsetattr(target, bc->names[ni], val);
    vmpush(val);
    VMNEXT();
}
lmakefunc: {
    int aux = VMIMM();
    fnexp *f = bc->auxfuncs[aux];
    vmpush(makefunc(f->params, NULL, f->pcount, (stmt**)f->body->items, f->body->count, env, f->name ? f->name : "anonymous", 0, 0));
    VMNEXT();
}
ltrystmt: {
    int aux = VMIMM();
    evaltrystmt(bc->auxstmts[aux], env);
    VMNEXT();
}
lclassstmt: {
    int aux = VMIMM();
    evalclassstmt(bc->auxstmts[aux], env);
    VMNEXT();
}
lclassexpr: {
    int aux = VMIMM();
    vmpush(evalclassexpr(bc->auxstmts[aux], env));
    VMNEXT();
}
lthrow: {
    object *v = axvmstack[--axvmsp];
    if (otype(v) == 2) throwexception(v->sval);
    throwexception("thrown value");
    VMNEXT();
}
lforbegin: {
    object *it = axvmstack[--axvmsp];
    int t = otype(it);
    long cur = 0;
    if (t == 11) cur = it->range.start;
    else if (t == 5 || t == 2) cur = 0;
    else throwexception("for loop expects iterable");
    vmpush(it);
    vmpush(vmmkint(cur));
    VMNEXT();
}
lforrangebegin: {
    for (int i = 0; i < 3; i++) {
        if (otype(axvmstack[axvmsp - 3 + i]) != 0) throwexception("range needs integer");
    }
    VMNEXT();
}
lfornext: {
    int slot = VMIMM();
    int endoff = VMIMM();
    object *it = axvmstack[axvmsp - 2];
    long cur = getival(axvmstack[axvmsp - 1]);
    int t = otype(it);
    if (t == 11) {
        if (!(cur < it->range.stop)) {
            pc += endoff;
            VMNEXT();
        }
        env->lvals[lbase + slot] = vmmkint(cur);
        axvmstack[axvmsp - 1] = vmmkint(cur + it->range.step);
        VMNEXT();
    }
    if (t == 5) {
        if (cur >= it->list.count) {
            pc += endoff;
            VMNEXT();
        }
        env->lvals[lbase + slot] = it->list.items[cur];
        axvmstack[axvmsp - 1] = vmmkint(cur + 1);
        VMNEXT();
    }
    if (t == 2) {
        if (cur >= (long)strlen(it->sval)) {
            pc += endoff;
            VMNEXT();
        }
        char b[2] = {it->sval[cur], 0};
        env->lvals[lbase + slot] = makestring(b);
        axvmstack[axvmsp - 1] = vmmkint(cur + 1);
        VMNEXT();
    }
    throwexception("for loop expects iterable");
    VMNEXT();
}
lforrangenext: {
    int slot = VMIMM();
    int endoff = VMIMM();
    long cur = getival(axvmstack[axvmsp - 3]);
    long stop = getival(axvmstack[axvmsp - 2]);
    if (!(cur < stop)) {
        pc += endoff;
        VMNEXT();
    }
    env->lvals[lbase + slot] = vmmkint(cur);
    axvmstack[axvmsp - 3] = vmmkint(cur + getival(axvmstack[axvmsp - 1]));
    VMNEXT();
}
laddloc: {
    int s1 = VMIMM();
    int s2 = VMIMM();
    object *a = env->lvals[lbase + s1];
    object *b = env->lvals[lbase + s2];
    if (isintval(a) && isintval(b)) vmpush(vmmkint(getival(a) + getival(b)));
    else vmpush(addvalues(a, b));
    VMNEXT();
}
ladd3: {
    int dst = VMIMM();
    int s1 = VMIMM();
    int s2 = VMIMM();
    if (bc->slotconst[dst]) throwexception("cannot reassign constant");
    object *a = env->lvals[lbase + s1];
    object *b = env->lvals[lbase + s2];
    object *r;
    if (isintval(a) && isintval(b)) r = vmmkint(getival(a) + getival(b));
    else r = addvalues(a, b);
    env->lvals[lbase + dst] = r;
    vmpush(r);
    VMNEXT();
}
lsublocint: {
    int s = VMIMM();
    int iv = VMIMM();
    object *v = env->lvals[lbase + s];
    if (isintval(v)) vmpush(vmmkint(getival(v) - iv));
    else vmpush(subvalues(v, vmmkint(iv)));
    VMNEXT();
}
llltlocint: {
    int s = VMIMM();
    int iv = VMIMM();
    object *v = env->lvals[lbase + s];
    if (isintval(v)) vmpush(makebool(getival(v) < iv));
    else vmpush(makebool(lessthan(v, vmmkint(iv))));
    VMNEXT();
}
#undef VMNEXT
#undef VMIMM
    return makenone();
}

object *executebytecode(bytecode *bc, environment *env) {
    return axvmrun(bc, env);
}
