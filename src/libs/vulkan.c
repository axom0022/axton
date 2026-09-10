#include "../core/axton.h"

object *vulkancreate(object **args, int argc, void *env) {
    void *vk = platformvulkancreate();
    if (!vk) throwexception("vulkan create failed");
    return makevulkanobj(vk);
}

object *vulkanrender(object **args, int argc, void *env) {
    if (argc < 1) throwexception("render needs vulkan");
    object *vk = args[0];
    platformvulkanrender(vk->vulkanobj.vulkan);
    return makenone();
}

void registervulkanmodule(environment *env) {
    object *mod = makemodule("vulkan", NULL);
    envset(mod->module.exports, "create", makebuiltin(vulkancreate), 0);
    envset(mod->module.exports, "render", makebuiltin(vulkanrender), 0);
    envset(env, "vulkan", mod, 0);
}
