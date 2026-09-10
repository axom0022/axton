#include "../core/axton.h"

object *plotcreate(object **args, int argc, void *env) {
    void *plot = platformplotcreate();
    if (!plot) throwexception("plot create failed");
    return makeplot(plot);
}

object *plotline(object **args, int argc, void *env) {
    if (argc < 3) throwexception("line needs x y n");
    object *plot = args[0];
    object *xlist = args[1];
    object *ylist = args[2];
    int n = xlist->list.count;
    float *x = malloc(n * sizeof(float));
    float *y = malloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        x[i] = xlist->list.items[i]->fval;
        y[i] = ylist->list.items[i]->fval;
    }
    platformplotline(plot->plot.plot, x, y, n);
    free(x);
    free(y);
    return makenone();
}

object *plotscatter(object **args, int argc, void *env) {
    if (argc < 3) throwexception("scatter needs x y n");
    object *plot = args[0];
    object *xlist = args[1];
    object *ylist = args[2];
    int n = xlist->list.count;
    float *x = malloc(n * sizeof(float));
    float *y = malloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        x[i] = xlist->list.items[i]->fval;
        y[i] = ylist->list.items[i]->fval;
    }
    platformplotscatter(plot->plot.plot, x, y, n);
    free(x);
    free(y);
    return makenone();
}

object *plotsave(object **args, int argc, void *env) {
    if (argc < 2) throwexception("save needs path");
    object *plot = args[0];
    char *path = args[1]->sval;
    platformplotsave(plot->plot.plot, path);
    return makenone();
}

void registerplotmodule(environment *env) {
    object *mod = makemodule("plot", NULL);
    envset(mod->module.exports, "create", makebuiltin(plotcreate), 0);
    envset(mod->module.exports, "line", makebuiltin(plotline), 0);
    envset(mod->module.exports, "scatter", makebuiltin(plotscatter), 0);
    envset(mod->module.exports, "save", makebuiltin(plotsave), 0);
    envset(env, "plot", mod, 0);
}
