#ifndef vector_h
#define vector_h

#include "../core/axton.h"

typedef struct vector {
    float *data;
    int size;
    int cap;
} vector;

object *vec_new(int size);
object *vec_fromlist(object *list);
object *vec_zeros(int size);
object *vec_ones(int size);
object *vec_range(float start, float stop, float step);
object *vec_arange(float start, float stop, float step);
object *vec_linspace(float start, float stop, int num);
object *vec_add(object *a, object *b);
object *vec_sub(object *a, object *b);
object *vec_mul(object *a, object *b);
object *vec_div(object *a, object *b);
object *vec_pow(object *a, object *b);
object *vec_dot(object *a, object *b);
object *vec_cross(object *a, object *b);
object *vec_norm(object *v);
object *vec_normalize(object *v);
object *vec_sum(object *v);
object *vec_mean(object *v);
object *vec_std(object *v);
object *vec_min(object *v);
object *vec_max(object *v);
object *vec_abs(object *v);
object *vec_sqrt(object *v);
object *vec_exp(object *v);
object *vec_log(object *v);
object *vec_sin(object *v);
object *vec_cos(object *v);
object *vec_tan(object *v);
object *vec_reshape(object *v, int rows, int cols);
object *vec_transpose(object *v);
object *vec_matmul(object *a, object *b);
object *vec_outer(object *a, object *b);
object *vec_concatenate(object **vecs, int count);
object *vec_slice(object *v, int start, int stop, int step);
void vec_set(object *v, int idx, float val);
float vec_get(object *v, int idx);
int vec_len(object *v);

#endif
