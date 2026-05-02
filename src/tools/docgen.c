#include "../core/axton.h"

char *generatedocs(char *source) {
    char *result = malloc(strlen(source) + 1024);
    int ridx = 0;
    int len = strlen(source);
    char *docs = "axton documentation\n\n";
    strcpy(result, docs);
    ridx = strlen(docs);
    for (int i = 0; i < len; i++) {
        if (source[i] == '/' && source[i+1] == '/') {
            result[ridx++] = '\n';
            result[ridx++] = ' ';
            while (source[i] && source[i] != '\n') {
                result[ridx++] = source[i++];
            }
            result[ridx++] = '\n';
        }
        if (source[i] == 'f' && source[i+1] == 'n' && source[i+2] == ' ') {
            result[ridx++] = '\n';
            result[ridx++] = 'f';
            result[ridx++] = 'u';
            result[ridx++] = 'n';
            result[ridx++] = 'c';
            result[ridx++] = 't';
            result[ridx++] = 'i';
            result[ridx++] = 'o';
            result[ridx++] = 'n';
            result[ridx++] = ' ';
            while (source[i] && source[i] != '{') {
                result[ridx++] = source[i++];
            }
            result[ridx++] = '\n';
        }
    }
    result[ridx] = 0;
    return result;
}

object *builtindocs(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("docs needs source");
    char *doc = generatedocs(args[0]->sval);
    object *result = makestring(doc);
    free(doc);
    return result;
}
