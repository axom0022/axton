#include "../core/axton.h"

object *builtinitertoolscycle(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("cycle needs iterable");
    object *iterable = args[0];
    object *cycle = makelist();
    for (int i = 0; i < iterable->list.count; i++) {
        listappend(cycle, iterable->list.items[i]);
    }
    return cycle;
}

object *builtinitertoolsrepeat(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("repeat needs element");
    object *repeat = makelist();
    int times = (argc > 1 && args[1]->type == 0) ? args[1]->ival : 10;
    for (int i = 0; i < times; i++) {
        listappend(repeat, args[0]);
    }
    return repeat;
}

object *builtinitertoolschain(object **args, int argc, environment *env) {
    object *chain = makelist();
    for (int i = 0; i < argc; i++) {
        if (args[i]->type == 5) {
            for (int j = 0; j < args[i]->list.count; j++) {
                listappend(chain, args[i]->list.items[j]);
            }
        }
    }
    return chain;
}

object *builtinitertoolsproduct(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("product needs at least two iterables");
    object *result = makelist();
    object *a = args[0];
    object *b = args[1];
    for (int i = 0; i < a->list.count; i++) {
        for (int j = 0; j < b->list.count; j++) {
            object *pair = makelist();
            listappend(pair, a->list.items[i]);
            listappend(pair, b->list.items[j]);
            listappend(result, pair);
        }
    }
    return result;
}

object *builtinitertoolspermutations(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("permutations needs iterable");
    object *iterable = args[0];
    object *result = makelist();
    int r = (argc > 1 && args[1]->type == 0) ? args[1]->ival : iterable->list.count;
    if (r > iterable->list.count) return result;
    int *indices = malloc(sizeof(int) * r);
    for (int i = 0; i < r; i++) indices[i] = i;
    while (1) {
        object *perm = makelist();
        for (int i = 0; i < r; i++) listappend(perm, iterable->list.items[indices[i]]);
        listappend(result, perm);
        int i = r - 1;
        while (i >= 0 && indices[i] == iterable->list.count - (r - i)) i--;
        if (i < 0) break;
        indices[i]++;
        for (int j = i + 1; j < r; j++) indices[j] = indices[j - 1] + 1;
    }
    free(indices);
    return result;
}

object *builtinitertoolscombinations(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("combinations needs iterable and r");
    object *iterable = args[0];
    int r = args[1]->ival;
    object *result = makelist();
    if (r > iterable->list.count) return result;
    int *indices = malloc(sizeof(int) * r);
    for (int i = 0; i < r; i++) indices[i] = i;
    while (1) {
        object *comb = makelist();
        for (int i = 0; i < r; i++) listappend(comb, iterable->list.items[indices[i]]);
        listappend(result, comb);
        int i = r - 1;
        while (i >= 0 && indices[i] == iterable->list.count - (r - i)) i--;
        if (i < 0) break;
        indices[i]++;
        for (int j = i + 1; j < r; j++) indices[j] = indices[j - 1] + 1;
    }
    free(indices);
    return result;
}

void registeritertools(environment *env) {
    object *itmod = makemodule("itertools", NULL);
    envset(itmod->module.exports, "cycle", makebuiltin(builtinitertoolscycle), 0);
    envset(itmod->module.exports, "repeat", makebuiltin(builtinitertoolsrepeat), 0);
    envset(itmod->module.exports, "chain", makebuiltin(builtinitertoolschain), 0);
    envset(itmod->module.exports, "product", makebuiltin(builtinitertoolsproduct), 0);
    envset(itmod->module.exports, "permutations", makebuiltin(builtinitertoolspermutations), 0);
    envset(itmod->module.exports, "combinations", makebuiltin(builtinitertoolscombinations), 0);
    envset(env, "itertools", itmod, 0);
}
