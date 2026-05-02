#include "axton.h"

void listappend(object *list, object *item) {
    if (list->type != 5) return;
    if (list->list.count >= list->list.cap) {
        list->list.cap *= 2;
        list->list.items = realloc(list->list.items, sizeof(object*) * list->list.cap);
    }
    list->list.items[list->list.count++] = item;
}

object *listget(object *list, long idx) {
    if (list->type != 5) return NULL;
    if (idx < 0) idx = list->list.count + idx;
    if (idx < 0 || idx >= list->list.count) return NULL;
    return list->list.items[idx];
}

void listset(object *list, long idx, object *item) {
    if (list->type != 5) return;
    if (idx < 0) idx = list->list.count + idx;
    if (idx >= 0 && idx < list->list.count) list->list.items[idx] = item;
}

void dictset(object *dict, object *key, object *val) {
    if (dict->type != 6) return;
    for (int i = 0; i < dict->dict.count; i++) {
        if (valuesequal(dict->dict.keyvals[i], key)) {
            dict->dict.vals[i] = val;
            return;
        }
    }
    dict->dict.count++;
    dict->dict.keys = realloc(dict->dict.keys, sizeof(char*) * dict->dict.count);
    dict->dict.keyvals = realloc(dict->dict.keyvals, sizeof(object*) * dict->dict.count);
    dict->dict.vals = realloc(dict->dict.vals, sizeof(object*) * dict->dict.count);
    dict->dict.keyvals[dict->dict.count - 1] = key;
    dict->dict.vals[dict->dict.count - 1] = val;
}

object *dictget(object *dict, object *key) {
    if (dict->type != 6) return NULL;
    for (int i = 0; i < dict->dict.count; i++) {
        if (valuesequal(dict->dict.keyvals[i], key)) return dict->dict.vals[i];
    }
    return NULL;
}

int dicthas(object *dict, object *key) {
    if (dict->type != 6) return 0;
    for (int i = 0; i < dict->dict.count; i++) {
        if (valuesequal(dict->dict.keyvals[i], key)) return 1;
    }
    return 0;
}

int istruthy(object *v) {
    if (!v) return 0;
    switch (v->type) {
        case 3: return v->bval;
        case 0: return v->ival != 0;
        case 1: return v->fval != 0;
        case 2: return v->sval && strlen(v->sval) > 0;
        case 5: return v->list.count > 0;
        case 6: return v->dict.count > 0;
        case 4: return 0;
        default: return 1;
    }
}

int valuesequal(object *a, object *b) {
    if (a->type != b->type) return 0;
    switch (a->type) {
        case 0: return a->ival == b->ival;
        case 1: return a->fval == b->fval;
        case 2: return strcmp(a->sval, b->sval) == 0;
        case 3: return a->bval == b->bval;
        case 4: return 1;
        default: return a == b;
    }
}

object *addvalues(object *a, object *b) {
    if (a->type == 0 && b->type == 0) return makeint(a->ival + b->ival);
    if (a->type == 1 && b->type == 1) return makefloat(a->fval + b->fval);
    if (a->type == 0 && b->type == 1) return makefloat(a->ival + b->fval);
    if (a->type == 1 && b->type == 0) return makefloat(a->fval + b->ival);
    if (a->type == 2 && b->type == 2) {
        char *s = malloc(strlen(a->sval) + strlen(b->sval) + 1);
        strcpy(s, a->sval);
        strcat(s, b->sval);
        return makestring(s);
    }
    throwexception("bad operands for add");
    return NULL;
}

object *subvalues(object *a, object *b) {
    if (a->type == 0 && b->type == 0) return makeint(a->ival - b->ival);
    if (a->type == 1 && b->type == 1) return makefloat(a->fval - b->fval);
    if (a->type == 0 && b->type == 1) return makefloat(a->ival - b->fval);
    if (a->type == 1 && b->type == 0) return makefloat(a->fval - b->ival);
    throwexception("bad operands for sub");
    return NULL;
}

object *mulvalues(object *a, object *b) {
    if (a->type == 0 && b->type == 0) return makeint(a->ival * b->ival);
    if (a->type == 1 && b->type == 1) return makefloat(a->fval * b->fval);
    if (a->type == 0 && b->type == 1) return makefloat(a->ival * b->fval);
    if (a->type == 1 && b->type == 0) return makefloat(a->fval * b->ival);
    if (a->type == 2 && b->type == 0) {
        int len = strlen(a->sval) * b->ival;
        char *s = malloc(len + 1);
        s[0] = 0;
        for (int i = 0; i < b->ival; i++) strcat(s, a->sval);
        return makestring(s);
    }
    throwexception("bad operands for mul");
    return NULL;
}

object *divvalues(object *a, object *b) {
    if (b->type == 0 && b->ival == 0) throwexception("division by zero");
    if (b->type == 1 && b->fval == 0) throwexception("division by zero");
    if (a->type == 0 && b->type == 0) return makefloat((double)a->ival / b->ival);
    if (a->type == 1 && b->type == 1) return makefloat(a->fval / b->fval);
    if (a->type == 0 && b->type == 1) return makefloat(a->ival / b->fval);
    if (a->type == 1 && b->type == 0) return makefloat(a->fval / b->ival);
    throwexception("bad operands for div");
    return NULL;
}

int lessthan(object *a, object *b) {
    if (a->type == 0 && b->type == 0) return a->ival < b->ival;
    if (a->type == 1 && b->type == 1) return a->fval < b->fval;
    if (a->type == 0 && b->type == 1) return a->ival < b->fval;
    if (a->type == 1 && b->type == 0) return a->fval < b->ival;
    if (a->type == 2 && b->type == 2) return strcmp(a->sval, b->sval) < 0;
    throwexception("cannot compare");
    return 0;
}

int greaterthan(object *a, object *b) {
    if (a->type == 0 && b->type == 0) return a->ival > b->ival;
    if (a->type == 1 && b->type == 1) return a->fval > b->fval;
    if (a->type == 0 && b->type == 1) return a->ival > b->fval;
    if (a->type == 1 && b->type == 0) return a->fval > b->ival;
    if (a->type == 2 && b->type == 2) return strcmp(a->sval, b->sval) > 0;
    throwexception("cannot compare");
    return 0;
}
