#include "../core/axton.h"
#include <uuid/uuid.h>

object *uuiduuid4(object **a, int c, void *e) {
    uuid_t uu;
    uuid_generate_random(uu);
    char str[37];
    uuid_unparse(uu, str);
    return makestring(str);
}

object *uuiduuid1(object **a, int c, void *e) {
    uuid_t uu;
    uuid_generate_time(uu);
    char str[37];
    uuid_unparse(uu, str);
    return makestring(str);
}

void registeruuidslib(environment *env) {
    object *mod = makemodule("uuid", NULL);
    envset(mod->module.exports, "uuid4", makebuiltin(uuiduuid4), 0);
    envset(mod->module.exports, "uuid1", makebuiltin(uuiduuid1), 0);
    envset(env, "uuid", mod, 0);
}
