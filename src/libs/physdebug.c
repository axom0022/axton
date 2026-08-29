#include "../core/axton.h"

object *physdebugcreate(object **args, int argc, void *env) {
    if (argc < 3) throwexception("create needs positions count color");
    object *poslist = args[0];
    int count = args[1]->ival;
    int color = args[2]->ival;
    float *pos = malloc(count * 3 * sizeof(float));
    for (int i=0;i<count;i++) {
        pos[i*3] = poslist->list.items[i*3]->fval;
        pos[i*3+1] = poslist->list.items[i*3+1]->fval;
        pos[i*3+2] = poslist->list.items[i*3+2]->fval;
    }
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    return makedebugphys(pos, count, r, g, b);
}

object *physdebugdraw(object **args, int argc, void *env) {
    if (argc < 1) throwexception("draw needs physdebug");
    object *pd = args[0];
    platformphysdebugdraw(pd->debugphys.positions, pd->debugphys.count,
                          pd->debugphys.color[0], pd->debugphys.color[1], pd->debugphys.color[2]);
    return makenone();
}

void registerphysdebuglib(environment *env) {
    object *mod = makemodule("physdebug", NULL);
    envset(mod->module.exports, "create", makebuiltin(physdebugcreate), 0);
    envset(mod->module.exports, "draw", makebuiltin(physdebugdraw), 0);
    envset(env, "physdebug", mod, 0);
}
