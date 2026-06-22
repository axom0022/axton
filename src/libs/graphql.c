#include "../core/axton.h"

object *graphqlquery(object **a, int c, void *e) {
    if (c < 2) throwexception("query needs endpoint and query");
    return makedict();
}

object *graphqlmutate(object **a, int c, void *e) {
    if (c < 2) throwexception("mutate needs endpoint and query");
    return makedict();
}

void registergraphqllib(environment *env) {
    object *mod = makemodule("graphql", NULL);
    envset(mod->module.exports, "query", makebuiltin(graphqlquery), 0);
    envset(mod->module.exports, "mutate", makebuiltin(graphqlmutate), 0);
    envset(env, "graphql", mod, 0);
}
