#include "../core/axton.h"

object *audiomixercreate(object **args, int argc, void *env) {
    int ch = argc > 0 ? args[0]->ival : 2;
    void *dev = platformaudiomixcreate(ch);
    if (!dev) throwexception("audio mixer failed");
    return makeaudiomix(dev, NULL, ch, 1.0);
}

object *audiomixeradd(object **args, int argc, void *env) {
    if (argc < 2) throwexception("add needs mixer samples");
    object *mixer = args[0];
    object *samples = args[1];
    int count = samples->list.count;
    float *data = malloc(count * sizeof(float));
    for (int i=0;i<count;i++) data[i] = samples->list.items[i]->fval;
    platformaudiomixadd(mixer->audiomix.dev, data, count);
    free(data);
    return makenone();
}

object *audiomixerplay(object **args, int argc, void *env) {
    if (argc < 1) throwexception("play needs mixer");
    object *mixer = args[0];
    platformaudiomixplay(mixer->audiomix.dev);
    return makenone();
}

object *audiomixersetspeaker(object **args, int argc, void *env) {
    return makenone();
}

void registeraudiomixerlib(environment *env) {
    object *mod = makemodule("audiomixer", NULL);
    envset(mod->module.exports, "create", makebuiltin(audiomixercreate), 0);
    envset(mod->module.exports, "add", makebuiltin(audiomixeradd), 0);
    envset(mod->module.exports, "play", makebuiltin(audiomixerplay), 0);
    envset(mod->module.exports, "setspeaker", makebuiltin(audiomixersetspeaker), 0);
    envset(env, "audiomixer", mod, 0);
}
