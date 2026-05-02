#include "../core/axton.h"

char *formatcode(char *source) {
    char *result = malloc(strlen(source) * 2 + 1);
    int ridx = 0;
    int indent = 0;
    int i = 0;
    while (source[i]) {
        if (source[i] == '{') {
            result[ridx++] = source[i++];
            result[ridx++] = '\n';
            indent++;
            for (int j = 0; j < indent; j++) result[ridx++] = ' ';
        } else if (source[i] == '}') {
            result[ridx++] = '\n';
            indent--;
            for (int j = 0; j < indent; j++) result[ridx++] = ' ';
            result[ridx++] = source[i++];
        } else if (source[i] == ';') {
            result[ridx++] = source[i++];
            result[ridx++] = '\n';
            for (int j = 0; j < indent; j++) result[ridx++] = ' ';
        } else if (source[i] == '\n') {
            i++;
            result[ridx++] = '\n';
            for (int j = 0; j < indent; j++) result[ridx++] = ' ';
            while (source[i] == ' ' || source[i] == '\t') i++;
        } else {
            result[ridx++] = source[i++];
        }
    }
    result[ridx] = 0;
    return result;
}

object *builtinformat(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("format needs source");
    char *formatted = formatcode(args[0]->sval);
    object *result = makestring(formatted);
    free(formatted);
    return result;
}
