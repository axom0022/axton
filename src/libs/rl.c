#include "../core/axton.h"

object *rlcreate(object **args, int argc, void *env) {
    void *agent = platformrlcreate();
    if (!agent) throwexception("rl create failed");
    return makerl(agent);
}

object *rltrain(object **args, int argc, void *env) {
    if (argc < 4) throwexception("train needs states actions lr");
    object *rl = args[0];
    int states = args[1]->ival;
    int actions = args[2]->ival;
    float lr = args[3]->fval;
    platformrltrain(rl->rl.agent, states, actions, lr);
    return makenone();
}

object *rlpredict(object **args, int argc, void *env) {
    if (argc < 2) throwexception("predict needs state");
    object *rl = args[0];
    float *state = malloc(sizeof(float));
    state[0] = args[1]->fval;
    void *result = platformrlpredict(rl->rl.agent, state);
    free(state);
    return makenative(result, NULL);
}

void registerrlmodule(environment *env) {
    object *mod = makemodule("rl", NULL);
    envset(mod->module.exports, "create", makebuiltin(rlcreate), 0);
    envset(mod->module.exports, "train", makebuiltin(rltrain), 0);
    envset(mod->module.exports, "predict", makebuiltin(rlpredict), 0);
    envset(env, "rl", mod, 0);
}
