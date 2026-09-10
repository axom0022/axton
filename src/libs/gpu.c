#include "../core/axton.h"

object *gpucreate(object **args, int argc, void *env) {
    void *gpu = platformgpucreate();
    if (!gpu) throwexception("gpu create failed");
    return makegpuobj(gpu);
}

object *gpukernel(object **args, int argc, void *env) {
    if (argc < 2) throwexception("kernel needs src");
    object *gpu = args[0];
    char *src = args[1]->sval;
    void *kernel = platformgpukernel(gpu->gpuobj.gpu, src);
    return makenative(kernel, NULL);
}

object *gpurun(object **args, int argc, void *env) {
    if (argc < 3) throwexception("run needs kernel n");
    object *gpu = args[0];
    void *kernel = args[1];
    int n = args[2]->ival;
    platformgpurun(gpu->gpuobj.gpu, kernel, n);
    return makenone();
}

void registergpumodule(environment *env) {
    object *mod = makemodule("gpu", NULL);
    envset(mod->module.exports, "create", makebuiltin(gpucreate), 0);
    envset(mod->module.exports, "kernel", makebuiltin(gpukernel), 0);
    envset(mod->module.exports, "run", makebuiltin(gpurun), 0);
    envset(env, "gpu", mod, 0);
}
