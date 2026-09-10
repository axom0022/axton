#include "../core/axton.h"

object *channelcreate(object **args, int argc, void *env) {
    int size = argc > 0 ? args[0]->ival : 0;
    void *ch = platformchannelcreate(size);
    if (!ch) throwexception("channel create failed");
    return makechannel(ch);
}

object *channelsend(object **args, int argc, void *env) {
    if (argc < 2) throwexception("send needs data");
    object *ch = args[0];
    object *data = args[1];
    platformchannelsend(ch->channel.ch, data);
    return makenone();
}

object *channelrecv(object **args, int argc, void *env) {
    if (argc < 1) throwexception("recv needs channel");
    object *ch = args[0];
    void *data = platformchannelrecv(ch->channel.ch);
    return (object*)data;
}

void registerchannelmodule(environment *env) {
    object *mod = makemodule("channel", NULL);
    envset(mod->module.exports, "create", makebuiltin(channelcreate), 0);
    envset(mod->module.exports, "send", makebuiltin(channelsend), 0);
    envset(mod->module.exports, "recv", makebuiltin(channelrecv), 0);
    envset(env, "channel", mod, 0);
}
