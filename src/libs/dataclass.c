#include "../core/axton.h"

object *dataclass(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("dataclass needs name and fields");
    char *name = args[0]->sval;
    object *fields = args[1];
    if (fields->type != 5) throwexception("fields must be list");
    object *cls = makeclass(name, envnew(NULL), NULL);
    envset(cls->klass.attrs, "__dataclass_fields__", fields, 0);
    char *init_params = malloc(1024);
    char *init_body = malloc(2048);
    strcpy(init_params, "(");
    strcpy(init_body, "");
    for (int i = 0; i < fields->list.count; i++) {
        char *fname = fields->list.items[i]->sval;
        if (i > 0) strcat(init_params, ", ");
        strcat(init_params, fname);
        char line[256];
        snprintf(line, sizeof(line), "    let this.%s = %s\n", fname, fname);
        strcat(init_body, line);
    }
    strcat(init_params, ")");
    char *init_code = malloc(4096);
    snprintf(init_code, 4096, "let __init__ = fn%s {\n%s}", init_params, init_body);
    token *toks = tokenize(init_code);
    stmt *prog = parsetokens(toks, tcount);
    evalprogram(prog, cls->klass.attrs);
    free(init_params);
    free(init_body);
    free(init_code);
    return cls;
}

void registerdataclasslib(environment *env) {
    envset(env, "dataclass", makebuiltin(dataclass), 0);
}
