#include "../core/axton.h"

char **lintcode(char *source, int *count) {
    char **errors = malloc(sizeof(char*) * 100);
    *count = 0;
    int len = strlen(source);
    int line = 1;
    for (int i = 0; i < len; i++) {
        if (source[i] == '\t') {
            errors[*count] = malloc(64);
            snprintf(errors[*count], 64, "line %d: tabs not allowed", line);
            (*count)++;
        }
        if (source[i] == '=' && source[i+1] == '=' && source[i+2] == '=') {
            errors[*count] = malloc(64);
            snprintf(errors[*count], 64, "line %d: use == not ===", line);
            (*count)++;
        }
        if (source[i] == '\n') line++;
    }
    return errors;
}

object *builtinlint(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("lint needs source");
    int count = 0;
    char **errors = lintcode(args[0]->sval, &count);
    object *result = makelist();
    for (int i = 0; i < count; i++) {
        listappend(result, makestring(errors[i]));
        free(errors[i]);
    }
    free(errors);
    return result;
}
