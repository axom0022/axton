#include "../core/axton.h"
#include <math.h>

object *makevector(int size) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 70;
    obj->vector.data = calloc(size, sizeof(float));
    obj->vector.size = size;
    return obj;
}

object *makevectorfromlist(object *list) {
    if (list->type != 5) throwexception("vector.fromlist expects list");
    int size = list->list.count;
    object *v = makevector(size);
    for (int i = 0; i < size; i++) {
        if (list->list.items[i]->type == 0) v->vector.data[i] = list->list.items[i]->ival;
        else if (list->list.items[i]->type == 1) v->vector.data[i] = list->list.items[i]->fval;
    }
    return v;
}

object *vec_new(object **args, int argc, environment *env) {
    int size = (argc > 0 && args[0]->type == 0) ? args[0]->ival : 0;
    return makevector(size);
}

object *vec_fromlist(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("vector.fromlist needs list");
    return makevectorfromlist(args[0]);
}

object *vec_zeros(object **args, int argc, environment *env) {
    int size = (argc > 0 && args[0]->type == 0) ? args[0]->ival : 0;
    return makevector(size);
}

object *vec_ones(object **args, int argc, environment *env) {
    int size = (argc > 0 && args[0]->type == 0) ? args[0]->ival : 0;
    object *v = makevector(size);
    for (int i = 0; i < size; i++) v->vector.data[i] = 1.0;
    return v;
}

object *vec_arange(object **args, int argc, environment *env) {
    float start = 0, stop = 0, step = 1;
    if (argc == 1) { stop = args[0]->ival; }
    else if (argc == 2) { start = args[0]->ival; stop = args[1]->ival; }
    else if (argc >= 3) { start = args[0]->ival; stop = args[1]->ival; step = args[2]->fval; }
    int size = (int)((stop - start) / step) + 1;
    object *v = makevector(size);
    for (int i = 0; i < size; i++) v->vector.data[i] = start + i * step;
    return v;
}

object *vec_add(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("vector.add needs two vectors");
    object *a = args[0];
    object *b = args[1];
    if (a->type != 70 || b->type != 70) throwexception("vector.add expects vectors");
    if (a->vector.size != b->vector.size) throwexception("size mismatch");
    object *r = makevector(a->vector.size);
    for (int i = 0; i < a->vector.size; i++) r->vector.data[i] = a->vector.data[i] + b->vector.data[i];
    return r;
}

object *vec_sub(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("vector.sub needs two vectors");
    object *a = args[0], *b = args[1];
    if (a->type != 70 || b->type != 70) throwexception("vector.sub expects vectors");
    if (a->vector.size != b->vector.size) throwexception("size mismatch");
    object *r = makevector(a->vector.size);
    for (int i = 0; i < a->vector.size; i++) r->vector.data[i] = a->vector.data[i] - b->vector.data[i];
    return r;
}

object *vec_mul(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("vector.mul needs two vectors");
    object *a = args[0], *b = args[1];
    if (a->type != 70 || b->type != 70) throwexception("vector.mul expects vectors");
    if (a->vector.size != b->vector.size) throwexception("size mismatch");
    object *r = makevector(a->vector.size);
    for (int i = 0; i < a->vector.size; i++) r->vector.data[i] = a->vector.data[i] * b->vector.data[i];
    return r;
}

object *vec_div(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("vector.div needs two vectors");
    object *a = args[0], *b = args[1];
    if (a->type != 70 || b->type != 70) throwexception("vector.div expects vectors");
    if (a->vector.size != b->vector.size) throwexception("size mismatch");
    object *r = makevector(a->vector.size);
    for (int i = 0; i < a->vector.size; i++) {
        r->vector.data[i] = (b->vector.data[i] == 0) ? 0 : a->vector.data[i] / b->vector.data[i];
    }
    return r;
}

object *vec_dot(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("vector.dot needs two vectors");
    object *a = args[0], *b = args[1];
    if (a->type != 70 || b->type != 70) throwexception("vector.dot expects vectors");
    if (a->vector.size != b->vector.size) throwexception("size mismatch");
    float sum = 0;
    for (int i = 0; i < a->vector.size; i++) sum += a->vector.data[i] * b->vector.data[i];
    return makefloat(sum);
}

object *vec_norm(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("vector.norm needs vector");
    object *v = args[0];
    if (v->type != 70) throwexception("vector.norm expects vector");
    float sum = 0;
    for (int i = 0; i < v->vector.size; i++) sum += v->vector.data[i] * v->vector.data[i];
    return makefloat(sqrt(sum));
}

object *vec_sum(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("vector.sum needs vector");
    object *v = args[0];
    if (v->type != 70) throwexception("vector.sum expects vector");
    float sum = 0;
    for (int i = 0; i < v->vector.size; i++) sum += v->vector.data[i];
    return makefloat(sum);
}

object *vec_mean(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("vector.mean needs vector");
    object *v = args[0];
    if (v->type != 70) throwexception("vector.mean expects vector");
    float sum = 0;
    for (int i = 0; i < v->vector.size; i++) sum += v->vector.data[i];
    return makefloat(sum / v->vector.size);
}

object *vec_len(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("vector.len needs vector");
    if (args[0]->type != 70) throwexception("vector.len expects vector");
    return makeint(args[0]->vector.size);
}

void registervectorlib(environment *env) {
    object *vecmod = makemodule("vector", NULL);
    envset(vecmod->module.exports, "new", makebuiltin(vec_new), 0);
    envset(vecmod->module.exports, "fromlist", makebuiltin(vec_fromlist), 0);
    envset(vecmod->module.exports, "zeros", makebuiltin(vec_zeros), 0);
    envset(vecmod->module.exports, "ones", makebuiltin(vec_ones), 0);
    envset(vecmod->module.exports, "arange", makebuiltin(vec_arange), 0);
    envset(vecmod->module.exports, "add", makebuiltin(vec_add), 0);
    envset(vecmod->module.exports, "sub", makebuiltin(vec_sub), 0);
    envset(vecmod->module.exports, "mul", makebuiltin(vec_mul), 0);
    envset(vecmod->module.exports, "div", makebuiltin(vec_div), 0);
    envset(vecmod->module.exports, "dot", makebuiltin(vec_dot), 0);
    envset(vecmod->module.exports, "norm", makebuiltin(vec_norm), 0);
    envset(vecmod->module.exports, "sum", makebuiltin(vec_sum), 0);
    envset(vecmod->module.exports, "mean", makebuiltin(vec_mean), 0);
    envset(vecmod->module.exports, "len", makebuiltin(vec_len), 0);
    envset(env, "vector", vecmod, 0);
}
