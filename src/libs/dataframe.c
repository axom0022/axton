#include "../core/axton.h"

object *dataframecreate(object **args, int argc, void *env) {
    void *df = platformdataframecreate();
    if (!df) throwexception("dataframe create failed");
    return makedataframe(df);
}

object *dataframeload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("load needs path");
    object *df = args[0];
    char *path = args[1]->sval;
    platformdataframeload(df->dataframe.df, path);
    return makenone();
}

object *dataframefilter(object **args, int argc, void *env) {
    if (argc < 2) throwexception("filter needs fn");
    object *df = args[0];
    object *fn = args[1];
    void *result = platformdataframefilter(df->dataframe.df, fn);
    return makedataframe(result);
}

void registerdataframemodule(environment *env) {
    object *mod = makemodule("dataframe", NULL);
    envset(mod->module.exports, "create", makebuiltin(dataframecreate), 0);
    envset(mod->module.exports, "load", makebuiltin(dataframeload), 0);
    envset(mod->module.exports, "filter", makebuiltin(dataframefilter), 0);
    envset(env, "dataframe", mod, 0);
}
