#include "../core/axton.h"
#include <math.h>

typedef struct ndarray {
    float *data;
    int *shape;
    int ndim;
    int size;
} ndarray;

object *nparray(object *list) {
    if (list->type != 5) throwexception("np.array expects list");
    int size = list->list.count;
    float *data = malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) {
        if (list->list.items[i]->type == 0) data[i] = list->list.items[i]->ival;
        else if (list->list.items[i]->type == 1) data[i] = list->list.items[i]->fval;
        else data[i] = 0;
    }
    int shape[1] = {size};
    object *arr = makenative(NULL, NULL);
    arr->type = 50;
    ndarray *nda = malloc(sizeof(ndarray));
    nda->data = data;
    nda->shape = malloc(sizeof(int));
    nda->shape[0] = size;
    nda->ndim = 1;
    nda->size = size;
    arr->native.data = nda;
    return arr;
}

object *npzeros(int *shape, int ndim) {
    int size = 1;
    for (int i = 0; i < ndim; i++) size *= shape[i];
    float *data = calloc(size, sizeof(float));
    object *arr = makenative(NULL, NULL);
    arr->type = 50;
    ndarray *nda = malloc(sizeof(ndarray));
    nda->data = data;
    nda->shape = malloc(sizeof(int) * ndim);
    nda->ndim = ndim;
    nda->size = size;
    for (int i = 0; i < ndim; i++) nda->shape[i] = shape[i];
    arr->native.data = nda;
    return arr;
}

object *npones(int *shape, int ndim) {
    int size = 1;
    for (int i = 0; i < ndim; i++) size *= shape[i];
    float *data = malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) data[i] = 1.0;
    object *arr = makenative(NULL, NULL);
    arr->type = 50;
    ndarray *nda = malloc(sizeof(ndarray));
    nda->data = data;
    nda->shape = malloc(sizeof(int) * ndim);
    nda->ndim = ndim;
    nda->size = size;
    for (int i = 0; i < ndim; i++) nda->shape[i] = shape[i];
    arr->native.data = nda;
    return arr;
}

object *nparange(int start, int stop, int step) {
    int size = (stop - start + step - 1) / step;
    float *data = malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) data[i] = start + i * step;
    int shape[1] = {size};
    object *arr = makenative(NULL, NULL);
    arr->type = 50;
    ndarray *nda = malloc(sizeof(ndarray));
    nda->data = data;
    nda->shape = malloc(sizeof(int));
    nda->shape[0] = size;
    nda->ndim = 1;
    nda->size = size;
    arr->native.data = nda;
    return arr;
}

object *npreshape(object *arr, int *shape, int ndim) {
    ndarray *nda = (ndarray*)arr->native.data;
    int new_size = 1;
    for (int i = 0; i < ndim; i++) new_size *= shape[i];
    if (new_size != nda->size) throwexception("cannot reshape");
    ndarray *new_nda = malloc(sizeof(ndarray));
    new_nda->data = nda->data;
    new_nda->shape = malloc(sizeof(int) * ndim);
    new_nda->ndim = ndim;
    new_nda->size = nda->size;
    for (int i = 0; i < ndim; i++) new_nda->shape[i] = shape[i];
    object *new_arr = makenative(NULL, NULL);
    new_arr->type = 50;
    new_arr->native.data = new_nda;
    return new_arr;
}

object *npadd(object *a, object *b) {
    ndarray *nda = (ndarray*)a->native.data;
    ndarray *ndb = (ndarray*)b->native.data;
    if (nda->size != ndb->size) throwexception("shape mismatch");
    float *result = malloc(nda->size * sizeof(float));
    for (int i = 0; i < nda->size; i++) result[i] = nda->data[i] + ndb->data[i];
    object *res = makenative(NULL, NULL);
    res->type = 50;
    ndarray *ndr = malloc(sizeof(ndarray));
    ndr->data = result;
    ndr->shape = nda->shape;
    ndr->ndim = nda->ndim;
    ndr->size = nda->size;
    res->native.data = ndr;
    return res;
}

object *npsub(object *a, object *b) {
    ndarray *nda = (ndarray*)a->native.data;
    ndarray *ndb = (ndarray*)b->native.data;
    if (nda->size != ndb->size) throwexception("shape mismatch");
    float *result = malloc(nda->size * sizeof(float));
    for (int i = 0; i < nda->size; i++) result[i] = nda->data[i] - ndb->data[i];
    object *res = makenative(NULL, NULL);
    res->type = 50;
    ndarray *ndr = malloc(sizeof(ndarray));
    ndr->data = result;
    ndr->shape = nda->shape;
    ndr->ndim = nda->ndim;
    ndr->size = nda->size;
    res->native.data = ndr;
    return res;
}

object *npmul(object *a, object *b) {
    ndarray *nda = (ndarray*)a->native.data;
    ndarray *ndb = (ndarray*)b->native.data;
    if (nda->size != ndb->size) throwexception("shape mismatch");
    float *result = malloc(nda->size * sizeof(float));
    for (int i = 0; i < nda->size; i++) result[i] = nda->data[i] * ndb->data[i];
    object *res = makenative(NULL, NULL);
    res->type = 50;
    ndarray *ndr = malloc(sizeof(ndarray));
    ndr->data = result;
    ndr->shape = nda->shape;
    ndr->ndim = nda->ndim;
    ndr->size = nda->size;
    res->native.data = ndr;
    return res;
}

object *npdiv(object *a, object *b) {
    ndarray *nda = (ndarray*)a->native.data;
    ndarray *ndb = (ndarray*)b->native.data;
    if (nda->size != ndb->size) throwexception("shape mismatch");
    float *result = malloc(nda->size * sizeof(float));
    for (int i = 0; i < nda->size; i++) {
        if (ndb->data[i] == 0) result[i] = 0;
        else result[i] = nda->data[i] / ndb->data[i];
    }
    object *res = makenative(NULL, NULL);
    res->type = 50;
    ndarray *ndr = malloc(sizeof(ndarray));
    ndr->data = result;
    ndr->shape = nda->shape;
    ndr->ndim = nda->ndim;
    ndr->size = nda->size;
    res->native.data = ndr;
    return res;
}

object *npsum(object *arr) {
    ndarray *nda = (ndarray*)arr->native.data;
    float total = 0;
    for (int i = 0; i < nda->size; i++) total += nda->data[i];
    return makefloat(total);
}

object *npmean(object *arr) {
    ndarray *nda = (ndarray*)arr->native.data;
    float total = 0;
    for (int i = 0; i < nda->size; i++) total += nda->data[i];
    return makefloat(total / nda->size);
}

object *npstd(object *arr) {
    ndarray *nda = (ndarray*)arr->native.data;
    float mean = 0;
    for (int i = 0; i < nda->size; i++) mean += nda->data[i];
    mean /= nda->size;
    float variance = 0;
    for (int i = 0; i < nda->size; i++) variance += pow(nda->data[i] - mean, 2);
    variance /= nda->size;
    return makefloat(sqrt(variance));
}

object *npdot(object *a, object *b) {
    ndarray *nda = (ndarray*)a->native.data;
    ndarray *ndb = (ndarray*)b->native.data;
    if (nda->ndim != 1 || ndb->ndim != 1) throwexception("dot expects 1D arrays");
    if (nda->size != ndb->size) throwexception("shape mismatch");
    float total = 0;
    for (int i = 0; i < nda->size; i++) total += nda->data[i] * ndb->data[i];
    return makefloat(total);
}

object *npmatmul(object *a, object *b) {
    ndarray *nda = (ndarray*)a->native.data;
    ndarray *ndb = (ndarray*)b->native.data;
    if (nda->ndim != 2 || ndb->ndim != 2) throwexception("matmul expects 2D arrays");
    int m = nda->shape[0];
    int n = nda->shape[1];
    int p = ndb->shape[1];
    if (n != ndb->shape[0]) throwexception("shape mismatch");
    float *result = calloc(m * p, sizeof(float));
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < p; j++) {
            for (int k = 0; k < n; k++) {
                result[i * p + j] += nda->data[i * n + k] * ndb->data[k * p + j];
            }
        }
    }
    int shape[2] = {m, p};
    return npzeros(shape, 2);
}

void registernumpylib(environment *env) {
    object *npmod = makemodule("np", NULL);
    envset(npmod->module.exports, "array", makebuiltin(nparray), 0);
    envset(npmod->module.exports, "zeros", makebuiltin(npzeros), 0);
    envset(npmod->module.exports, "ones", makebuiltin(npones), 0);
    envset(npmod->module.exports, "arange", makebuiltin(nparange), 0);
    envset(npmod->module.exports, "reshape", makebuiltin(npreshape), 0);
    envset(npmod->module.exports, "add", makebuiltin(npadd), 0);
    envset(npmod->module.exports, "sub", makebuiltin(npsub), 0);
    envset(npmod->module.exports, "mul", makebuiltin(npmul), 0);
    envset(npmod->module.exports, "div", makebuiltin(npdiv), 0);
    envset(npmod->module.exports, "sum", makebuiltin(npsum), 0);
    envset(npmod->module.exports, "mean", makebuiltin(npmean), 0);
    envset(npmod->module.exports, "std", makebuiltin(npstd), 0);
    envset(npmod->module.exports, "dot", makebuiltin(npdot), 0);
    envset(npmod->module.exports, "matmul", makebuiltin(npmatmul), 0);
    envset(env, "np", npmod, 0);
}
