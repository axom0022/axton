#include "../core/axton.h"

object *llmcreate(object **args, int argc, void *env) {
    void *model = platformllmcreate();
    if (!model) throwexception("llm create failed");
    return makellm(model);
}

object *llmload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("load needs path");
    object *llm = args[0];
    char *path = args[1]->sval;
    int res = platformllmload(llm->llm.model, path);
    if (!res) throwexception("llm load failed");
    llm->llm.loaded = 1;
    return makebool(1);
}

object *llmgenerate(object **args, int argc, void *env) {
    if (argc < 2) throwexception("generate needs prompt");
    object *llm = args[0];
    if (!llm->llm.loaded) throwexception("llm not loaded");
    char *prompt = args[1]->sval;
    char *result = platformllmgenerate(llm->llm.model, prompt);
    return makestring(result);
}

void registerllmmodule(environment *env) {
    object *mod = makemodule("llm", NULL);
    envset(mod->module.exports, "create", makebuiltin(llmcreate), 0);
    envset(mod->module.exports, "load", makebuiltin(llmload), 0);
    envset(mod->module.exports, "generate", makebuiltin(llmgenerate), 0);
    envset(env, "llm", mod, 0);
}
