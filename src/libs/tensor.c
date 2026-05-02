#include "../core/axton.h"

object *maketensor(float *data, int rows, int cols) {
    object *obj = gcalloc(sizeof(object));
    obj->type = 20;
    obj->tensor.data = data;
    obj->tensor.rows = rows;
    obj->tensor.cols = cols;
    return obj;
}

object *tensorcreate(int rows, int cols) {
    float *data = calloc(rows * cols, sizeof(float));
    for (int i = 0; i < rows * cols; i++) {
        data[i] = (float)rand() / RAND_MAX;
    }
    return maketensor(data, rows, cols);
}

object *tensoradd(object *a, object *b) {
    if (a->type != 20 || b->type != 20) throwexception("tensor add needs tensors");
    if (a->tensor.rows != b->tensor.rows || a->tensor.cols != b->tensor.cols) {
        throwexception("tensor shape mismatch");
    }
    int rows = a->tensor.rows;
    int cols = a->tensor.cols;
    float *result = malloc(rows * cols * sizeof(float));
    for (int i = 0; i < rows * cols; i++) {
        result[i] = a->tensor.data[i] + b->tensor.data[i];
    }
    return maketensor(result, rows, cols);
}

object *tensormul(object *a, object *b) {
    if (a->type != 20 || b->type != 20) throwexception("tensor mul needs tensors");
    int rows = a->tensor.rows;
    int cols = a->tensor.cols;
    int k = b->tensor.cols;
    float *result = calloc(rows * k, sizeof(float));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < k; j++) {
            float sum = 0;
            for (int x = 0; x < cols; x++) {
                sum += a->tensor.data[i * cols + x] * b->tensor.data[x * k + j];
            }
            result[i * k + j] = sum;
        }
    }
    return maketensor(result, rows, k);
}

object *builtintensor(object **args, int argc, environment *env) {
    int rows = argc > 0 && args[0]->type == 0 ? args[0]->ival : 3;
    int cols = argc > 1 && args[1]->type == 0 ? args[1]->ival : 3;
    return tensorcreate(rows, cols);
}

object *builtintensoradd(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("tensor.add needs two tensors");
    return tensoradd(args[0], args[1]);
}

object *builtintensormul(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("tensor.mul needs two tensors");
    return tensormul(args[0], args[1]);
}

void registertensorlib(environment *env) {
    object *tensormod = makemodule("tensor", NULL);
    envset(tensormod->module.exports, "new", makebuiltin(builtintensor), 0);
    envset(tensormod->module.exports, "add", makebuiltin(builtintensoradd), 0);
    envset(tensormod->module.exports, "mul", makebuiltin(builtintensormul), 0);
    envset(env, "tensor", tensormod, 0);
}
