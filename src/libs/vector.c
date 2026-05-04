#include "vector.h"
#include <math.h>

object *vec_new(int size) {
    object *obj = makenative(NULL, NULL);
    obj->type = 60;
    vector *v = malloc(sizeof(vector));
    v->data = calloc(size, sizeof(float));
    v->size = size;
    v->cap = size;
    obj->native.data = v;
    return obj;
}

object *vec_fromlist(object *list) {
    if (list->type != 5) throwexception("vec.fromlist expects list");
    object *v = vec_new(list->list.count);
    vector *vec = (vector*)v->native.data;
    for (int i = 0; i < list->list.count; i++) {
        if (list->list.items[i]->type == 0) vec->data[i] = list->list.items[i]->ival;
        else if (list->list.items[i]->type == 1) vec->data[i] = list->list.items[i]->fval;
        else vec->data[i] = 0;
    }
    return v;
}

object *vec_zeros(int size) {
    return vec_new(size);
}

object *vec_ones(int size) {
    object *v = vec_new(size);
    vector *vec = (vector*)v->native.data;
    for (int i = 0; i < size; i++) vec->data[i] = 1.0;
    return v;
}

object *vec_range(float start, float stop, float step) {
    int size = (int)((stop - start) / step) + 1;
    object *v = vec_new(size);
    vector *vec = (vector*)v->native.data;
    for (int i = 0; i < size; i++) vec->data[i] = start + i * step;
    return v;
}

object *vec_arange(float start, float stop, float step) {
    return vec_range(start, stop, step);
}

object *vec_linspace(float start, float stop, int num) {
    object *v = vec_new(num);
    vector *vec = (vector*)v->native.data;
    float step = (stop - start) / (num - 1);
    for (int i = 0; i < num; i++) vec->data[i] = start + i * step;
    return v;
}

static vector *getvec(object *obj) {
    if (obj->type != 60) throwexception("expected vector");
    return (vector*)obj->native.data;
}

object *vec_add(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    if (va->size != vb->size) throwexception("vector size mismatch");
    object *result = vec_new(va->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < va->size; i++) vr->data[i] = va->data[i] + vb->data[i];
    return result;
}

object *vec_sub(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    if (va->size != vb->size) throwexception("vector size mismatch");
    object *result = vec_new(va->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < va->size; i++) vr->data[i] = va->data[i] - vb->data[i];
    return result;
}

object *vec_mul(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    if (va->size != vb->size) throwexception("vector size mismatch");
    object *result = vec_new(va->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < va->size; i++) vr->data[i] = va->data[i] * vb->data[i];
    return result;
}

object *vec_div(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    if (va->size != vb->size) throwexception("vector size mismatch");
    object *result = vec_new(va->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < va->size; i++) {
        if (vb->data[i] == 0) vr->data[i] = 0;
        else vr->data[i] = va->data[i] / vb->data[i];
    }
    return result;
}

object *vec_pow(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    if (va->size != vb->size) throwexception("vector size mismatch");
    object *result = vec_new(va->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < va->size; i++) vr->data[i] = powf(va->data[i], vb->data[i]);
    return result;
}

object *vec_dot(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    if (va->size != vb->size) throwexception("vector size mismatch");
    float total = 0;
    for (int i = 0; i < va->size; i++) total += va->data[i] * vb->data[i];
    return makefloat(total);
}

object *vec_cross(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    if (va->size != 3 || vb->size != 3) throwexception("cross product requires 3D vectors");
    object *result = vec_new(3);
    vector *vr = (vector*)result->native.data;
    vr->data[0] = va->data[1] * vb->data[2] - va->data[2] * vb->data[1];
    vr->data[1] = va->data[2] * vb->data[0] - va->data[0] * vb->data[2];
    vr->data[2] = va->data[0] * vb->data[1] - va->data[1] * vb->data[0];
    return result;
}

object *vec_norm(object *v) {
    vector *vec = getvec(v);
    float total = 0;
    for (int i = 0; i < vec->size; i++) total += vec->data[i] * vec->data[i];
    return makefloat(sqrt(total));
}

object *vec_normalize(object *v) {
    vector *vec = getvec(v);
    float norm = sqrt(vec_norm(v)->fval);
    if (norm == 0) return vec_new(vec->size);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = vec->data[i] / norm;
    return result;
}

object *vec_sum(object *v) {
    vector *vec = getvec(v);
    float total = 0;
    for (int i = 0; i < vec->size; i++) total += vec->data[i];
    return makefloat(total);
}

object *vec_mean(object *v) {
    vector *vec = getvec(v);
    float total = 0;
    for (int i = 0; i < vec->size; i++) total += vec->data[i];
    return makefloat(total / vec->size);
}

object *vec_std(object *v) {
    vector *vec = getvec(v);
    float mean = vec_mean(v)->fval;
    float variance = 0;
    for (int i = 0; i < vec->size; i++) variance += pow(vec->data[i] - mean, 2);
    variance /= vec->size;
    return makefloat(sqrt(variance));
}

object *vec_min(object *v) {
    vector *vec = getvec(v);
    if (vec->size == 0) return makefloat(0);
    float min = vec->data[0];
    for (int i = 1; i < vec->size; i++) if (vec->data[i] < min) min = vec->data[i];
    return makefloat(min);
}

object *vec_max(object *v) {
    vector *vec = getvec(v);
    if (vec->size == 0) return makefloat(0);
    float max = vec->data[0];
    for (int i = 1; i < vec->size; i++) if (vec->data[i] > max) max = vec->data[i];
    return makefloat(max);
}

object *vec_abs(object *v) {
    vector *vec = getvec(v);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = fabs(vec->data[i]);
    return result;
}

object *vec_sqrt(object *v) {
    vector *vec = getvec(v);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = sqrt(vec->data[i]);
    return result;
}

object *vec_exp(object *v) {
    vector *vec = getvec(v);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = exp(vec->data[i]);
    return result;
}

object *vec_log(object *v) {
    vector *vec = getvec(v);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = log(vec->data[i]);
    return result;
}

object *vec_sin(object *v) {
    vector *vec = getvec(v);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = sin(vec->data[i]);
    return result;
}

object *vec_cos(object *v) {
    vector *vec = getvec(v);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = cos(vec->data[i]);
    return result;
}

object *vec_tan(object *v) {
    vector *vec = getvec(v);
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < vec->size; i++) vr->data[i] = tan(vec->data[i]);
    return result;
}

object *vec_reshape(object *v, int rows, int cols) {
    vector *vec = getvec(v);
    if (vec->size != rows * cols) throwexception("cannot reshape");
    object *result = vec_new(vec->size);
    vector *vr = (vector*)result->native.data;
    vr->data = vec->data;
    vr->size = vec->size;
    return result;
}

object *vec_transpose(object *v) {
    return v;
}

object *vec_matmul(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    int m = va->size;
    int n = vb->size;
    if (m != n) throwexception("matrix multiply size mismatch");
    float total = 0;
    for (int i = 0; i < m; i++) total += va->data[i] * vb->data[i];
    return makefloat(total);
}

object *vec_outer(object *a, object *b) {
    vector *va = getvec(a);
    vector *vb = getvec(b);
    object *result = vec_new(va->size * vb->size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0; i < va->size; i++) {
        for (int j = 0; j < vb->size; j++) {
            vr->data[i * vb->size + j] = va->data[i] * vb->data[j];
        }
    }
    return result;
}

object *vec_concatenate(object **vecs, int count) {
    int total = 0;
    for (int i = 0; i < count; i++) total += getvec(vecs[i])->size;
    object *result = vec_new(total);
    vector *vr = (vector*)result->native.data;
    int pos = 0;
    for (int i = 0; i < count; i++) {
        vector *v = getvec(vecs[i]);
        for (int j = 0; j < v->size; j++) vr->data[pos++] = v->data[j];
    }
    return result;
}

object *vec_slice(object *v, int start, int stop, int step) {
    vector *vec = getvec(v);
    if (start < 0) start = vec->size + start;
    if (stop < 0) stop = vec->size + stop;
    if (step == 0) step = 1;
    int size = (stop - start + step - 1) / step;
    if (size < 0) size = 0;
    object *result = vec_new(size);
    vector *vr = (vector*)result->native.data;
    for (int i = 0, j = start; j < stop && i < size; j += step, i++) {
        vr->data[i] = vec->data[j];
    }
    return result;
}

void vec_set(object *v, int idx, float val) {
    vector *vec = getvec(v);
    if (idx < 0) idx = vec->size + idx;
    if (idx >= 0 && idx < vec->size) vec->data[idx] = val;
}

float vec_get(object *v, int idx) {
    vector *vec = getvec(v);
    if (idx < 0) idx = vec->size + idx;
    if (idx >= 0 && idx < vec->size) return vec->data[idx];
    return 0;
}

int vec_len(object *v) {
    vector *vec = getvec(v);
    return vec->size;
}

void registervectorlib(environment *env) {
    object *vecmod = makemodule("vector", NULL);
    envset(vecmod->module.exports, "new", makebuiltin(vec_new), 0);
    envset(vecmod->module.exports, "fromlist", makebuiltin(vec_fromlist), 0);
    envset(vecmod->module.exports, "zeros", makebuiltin(vec_zeros), 0);
    envset(vecmod->module.exports, "ones", makebuiltin(vec_ones), 0);
    envset(vecmod->module.exports, "range", makebuiltin(vec_range), 0);
    envset(vecmod->module.exports, "arange", makebuiltin(vec_arange), 0);
    envset(vecmod->module.exports, "linspace", makebuiltin(vec_linspace), 0);
    envset(vecmod->module.exports, "add", makebuiltin(vec_add), 0);
    envset(vecmod->module.exports, "sub", makebuiltin(vec_sub), 0);
    envset(vecmod->module.exports, "mul", makebuiltin(vec_mul), 0);
    envset(vecmod->module.exports, "div", makebuiltin(vec_div), 0);
    envset(vecmod->module.exports, "pow", makebuiltin(vec_pow), 0);
    envset(vecmod->module.exports, "dot", makebuiltin(vec_dot), 0);
    envset(vecmod->module.exports, "cross", makebuiltin(vec_cross), 0);
    envset(vecmod->module.exports, "norm", makebuiltin(vec_norm), 0);
    envset(vecmod->module.exports, "normalize", makebuiltin(vec_normalize), 0);
    envset(vecmod->module.exports, "sum", makebuiltin(vec_sum), 0);
    envset(vecmod->module.exports, "mean", makebuiltin(vec_mean), 0);
    envset(vecmod->module.exports, "std", makebuiltin(vec_std), 0);
    envset(vecmod->module.exports, "min", makebuiltin(vec_min), 0);
    envset(vecmod->module.exports, "max", makebuiltin(vec_max), 0);
    envset(vecmod->module.exports, "abs", makebuiltin(vec_abs), 0);
    envset(vecmod->module.exports, "sqrt", makebuiltin(vec_sqrt), 0);
    envset(vecmod->module.exports, "exp", makebuiltin(vec_exp), 0);
    envset(vecmod->module.exports, "log", makebuiltin(vec_log), 0);
    envset(vecmod->module.exports, "sin", makebuiltin(vec_sin), 0);
    envset(vecmod->module.exports, "cos", makebuiltin(vec_cos), 0);
    envset(vecmod->module.exports, "tan", makebuiltin(vec_tan), 0);
    envset(vecmod->module.exports, "reshape", makebuiltin(vec_reshape), 0);
    envset(vecmod->module.exports, "transpose", makebuiltin(vec_transpose), 0);
    envset(vecmod->module.exports, "matmul", makebuiltin(vec_matmul), 0);
    envset(vecmod->module.exports, "outer", makebuiltin(vec_outer), 0);
    envset(vecmod->module.exports, "concat", makebuiltin(vec_concatenate), 0);
    envset(vecmod->module.exports, "slice", makebuiltin(vec_slice), 0);
    envset(vecmod->module.exports, "set", makebuiltin(vec_set), 0);
    envset(vecmod->module.exports, "get", makebuiltin(vec_get), 0);
    envset(vecmod->module.exports, "len", makebuiltin(vec_len), 0);
    envset(env, "vector", vecmod, 0);
}
