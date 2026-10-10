#include "axton.h"

object *listget(object *list, long idx) {
    if (otype(list) != 5) return NULL;
    if (idx < 0) idx = list->list.count + idx;
    if (idx < 0 || idx >= list->list.count) return NULL;
    return list->list.items[idx];
}

void listset(object *list, long idx, object *item) {
    if (otype(list) != 5) return;
    if (idx < 0) idx = list->list.count + idx;
    if (idx >= 0 && idx < list->list.count) list->list.items[idx] = item;
}

int istruthy(object *v) {
    if (!v) return 0;
    switch (otype(v)) {
        case 3: return getbval(v);
        case 0: return getival(v) != 0;
        case 1: return v->fval != 0;
        case 2: return v->sval && strlen(v->sval) > 0;
        case 5: return v->list.count > 0;
        case 6: return v->dict.count > 0;
        case 4: return 0;
        default: return 1;
    }
}

int valuesequal(object *a, object *b) {
    if (otype(a) != otype(b)) return 0;
    switch (otype(a)) {
        case 0: return getival(a) == getival(b);
        case 1: return a->fval == b->fval;
        case 2: return strcmp(a->sval, b->sval) == 0;
        case 3: return getbval(a) == getbval(b);
        case 4: return 1;
        default: return a == b;
    }
}

unsigned long valhash(object *v) {
    if (!v) return 0;
    switch (otype(v)) {
        case 0: return (unsigned long)getival(v) * 2654435761ul;
        case 1: {
            if (v->fval == 0) return 0;
            unsigned long h;
            memcpy(&h, &v->fval, sizeof(h));
            return h;
        }
        case 2: {
            unsigned long h = 5381;
            for (char *p = v->sval; *p; p++) h = h * 33 + (unsigned char)*p;
            return h;
        }
        case 3: return (unsigned long)getbval(v);
        case 4: return 0;
        default: return (unsigned long)(uintptr_t)v;
    }
}

object *addvalues(object *a, object *b) {
    if (otype(a) == 0 && otype(b) == 0) return makeint(getival(a) + getival(b));
    if (otype(a) == 1 && otype(b) == 1) return makefloat(a->fval + b->fval);
    if (otype(a) == 0 && otype(b) == 1) return makefloat(getival(a) + b->fval);
    if (otype(a) == 1 && otype(b) == 0) return makefloat(a->fval + getival(b));
    if (otype(a) == 2 && otype(b) == 2) {
        size_t la = strlen(a->sval);
        size_t lb = strlen(b->sval);
        char *s = malloc(la + lb + 1);
        memcpy(s, a->sval, la);
        memcpy(s + la, b->sval, lb);
        s[la + lb] = 0;
        object *obj = gcalloc(sizeof(object));
        obj->type = 2;
        obj->sval = s;
        return obj;
    }
    throwexception("bad operands for add");
    return NULL;
}

object *subvalues(object *a, object *b) {
    if (otype(a) == 0 && otype(b) == 0) return makeint(getival(a) - getival(b));
    if (otype(a) == 1 && otype(b) == 1) return makefloat(a->fval - b->fval);
    if (otype(a) == 0 && otype(b) == 1) return makefloat(getival(a) - b->fval);
    if (otype(a) == 1 && otype(b) == 0) return makefloat(a->fval - getival(b));
    throwexception("bad operands for sub");
    return NULL;
}

object *mulvalues(object *a, object *b) {
    if (otype(a) == 0 && otype(b) == 0) return makeint(getival(a) * getival(b));
    if (otype(a) == 1 && otype(b) == 1) return makefloat(a->fval * b->fval);
    if (otype(a) == 0 && otype(b) == 1) return makefloat(getival(a) * b->fval);
    if (otype(a) == 1 && otype(b) == 0) return makefloat(a->fval * getival(b));
    if (otype(a) == 2 && otype(b) == 0) {
        int len = strlen(a->sval) * getival(b);
        char *s = malloc(len + 1);
        s[0] = 0;
        for (int i = 0; i < getival(b); i++) strcat(s, a->sval);
        return makestring(s);
    }
    throwexception("bad operands for mul");
    return NULL;
}

object *divvalues(object *a, object *b) {
    if (otype(b) == 0 && getival(b) == 0) throwexception("division by zero");
    if (otype(b) == 1 && b->fval == 0) throwexception("division by zero");
    if (otype(a) == 0 && otype(b) == 0) return makefloat((double)getival(a) / getival(b));
    if (otype(a) == 1 && otype(b) == 1) return makefloat(a->fval / b->fval);
    if (otype(a) == 0 && otype(b) == 1) return makefloat(getival(a) / b->fval);
    if (otype(a) == 1 && otype(b) == 0) return makefloat(a->fval / getival(b));
    throwexception("bad operands for div");
    return NULL;
}

int lessthan(object *a, object *b) {
    if (otype(a) == 0 && otype(b) == 0) return getival(a) < getival(b);
    if (otype(a) == 1 && otype(b) == 1) return a->fval < b->fval;
    if (otype(a) == 0 && otype(b) == 1) return getival(a) < b->fval;
    if (otype(a) == 1 && otype(b) == 0) return a->fval < getival(b);
    if (otype(a) == 2 && otype(b) == 2) return strcmp(a->sval, b->sval) < 0;
    throwexception("cannot compare");
    return 0;
}

int greaterthan(object *a, object *b) {
    if (otype(a) == 0 && otype(b) == 0) return getival(a) > getival(b);
    if (otype(a) == 1 && otype(b) == 1) return a->fval > b->fval;
    if (otype(a) == 0 && otype(b) == 1) return getival(a) > b->fval;
    if (otype(a) == 1 && otype(b) == 0) return a->fval > getival(b);
    if (otype(a) == 2 && otype(b) == 2) return strcmp(a->sval, b->sval) > 0;
    throwexception("cannot compare");
    return 0;
}

object *modvalues(object *a, object *b) {
    if (otype(a) == 0 && otype(b) == 0) {
        if (getival(b) == 0) throwexception("modulo by zero");
        return makeint(getival(a) % getival(b));
    }
    double x = otype(a) == 0 ? (double)getival(a) : a->fval;
    double y = otype(b) == 0 ? (double)getival(b) : b->fval;
    if (y == 0) throwexception("modulo by zero");
    return makefloat(fmod(x, y));
}

object *powvalues(object *a, object *b) {
    if (otype(a) == 0 && otype(b) == 0 && getival(b) >= 0) {
        long r = 1;
        for (long i = 0; i < getival(b); i++) r *= getival(a);
        return makeint(r);
    }
    double x = otype(a) == 0 ? (double)getival(a) : a->fval;
    double y = otype(b) == 0 ? (double)getival(b) : b->fval;
    return makefloat(pow(x, y));
}
