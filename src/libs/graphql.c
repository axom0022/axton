#include "../core/axton.h"

object *graphqlcreate(object **args, int argc, void *env) {
    void *schema = platformgraphqlcreate();
    if (!schema) throwexception("graphql create failed");
    return makegraphql(schema);
}

object *graphqladdquery(object **args, int argc, void *env) {
    if (argc < 3) throwexception("addquery needs name fn");
    object *gql = args[0];
    char *name = args[1]->sval;
    object *fn = args[2];
    platformgraphqladdquery(gql->graphql.schema, name, fn);
    return makenone();
}

object *graphqladdmutation(object **args, int argc, void *env) {
    if (argc < 3) throwexception("addmutation needs name fn");
    object *gql = args[0];
    char *name = args[1]->sval;
    object *fn = args[2];
    platformgraphqladdmutation(gql->graphql.schema, name, fn);
    return makenone();
}

object *graphqlserve(object **args, int argc, void *env) {
    if (argc < 2) throwexception("serve needs port");
    object *gql = args[0];
    int port = args[1]->ival;
    void *server = platformgraphqlserve(gql->graphql.schema, port);
    return makenative(server, NULL);
}

void registergraphqlmodule(environment *env) {
    object *mod = makemodule("graphql", NULL);
    envset(mod->module.exports, "create", makebuiltin(graphqlcreate), 0);
    envset(mod->module.exports, "addquery", makebuiltin(graphqladdquery), 0);
    envset(mod->module.exports, "addmutation", makebuiltin(graphqladdmutation), 0);
    envset(mod->module.exports, "serve", makebuiltin(graphqlserve), 0);
    envset(env, "graphql", mod, 0);
}
