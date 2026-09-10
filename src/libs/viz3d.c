#include "../core/axton.h"

object *viz3dcreate(object **args, int argc, void *env) {
    void *viz = platformviz3dcreate();
    if (!viz) throwexception("viz3d create failed");
    return makeviz3d(viz);
}

object *viz3dadd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs points");
    object *viz = args[0];
    object *plist = args[1];
    int n = plist->list.count / 3;
    float *x = malloc(n * sizeof(float));
    float *y = malloc(n * sizeof(float));
    float *z = malloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        x[i] = plist->list.items[i*3]->fval;
        y[i] = plist->list.items[i*3+1]->fval;
        z[i] = plist->list.items[i*3+2]->fval;
    }
    platformviz3dadd(viz->viz3d.viz, x, y, z, n);
    free(x);
    free(y);
    free(z);
    return makenone();
}

object *viz3drender(object **args, int argc, void *env) {
    if (argc < 1) throwexception("render needs viz");
    object *viz = args[0];
    platformviz3drender(viz->viz3d.viz);
    return makenone();
}

void registerviz3dmodule(environment *env) {
    object *mod = makemodule("viz3d", NULL);
    envset(mod->module.exports, "create", makebuiltin(viz3dcreate), 0);
    envset(mod->module.exports, "add", makebuiltin(viz3dadd), 0);
    envset(mod->module.exports, "render", makebuiltin(viz3drender), 0);
    envset(env, "viz3d", mod, 0);
}
