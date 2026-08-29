#include "../core/axton.h"
#include <regex.h>

static object *rules = NULL;

object *linteraddrule(object **args, int argc, void *env) {
    if (argc < 3) throwexception("addrule needs name pattern");
    if (!rules) rules = makelist();
    object *rule = makedict();
    dictset(rule, makestring("name"), args[0]);
    dictset(rule, makestring("pattern"), args[1]);
    listappend(rules, rule);
    return makelinter(rules, rules->list.count);
}

object *linterrun(object **args, int argc, void *env) {
    if (argc < 1) throwexception("run needs source");
    char *source = args[0]->sval;
    object *results = makelist();
    if (!rules) return results;
    for (int i=0;i<rules->list.count;i++) {
        object *rule = rules->list.items[i];
        char *pattern = dictget(rule, makestring("pattern"))->sval;
        regex_t re;
        if (regcomp(&re, pattern, REG_EXTENDED) != 0) continue;
        regmatch_t match;
        if (regexec(&re, source, 1, &match, 0) == 0) {
            char *name = dictget(rule, makestring("name"))->sval;
            object *entry = makedict();
            dictset(entry, makestring("rule"), makestring(name));
            dictset(entry, makestring("line"), makeint(1));
            listappend(results, entry);
        }
        regfree(&re);
    }
    return results;
}

void registerlinterlib(environment *env) {
    object *mod = makemodule("linter", NULL);
    envset(mod->module.exports, "addrule", makebuiltin(linteraddrule), 0);
    envset(mod->module.exports, "run", makebuiltin(linterrun), 0);
    envset(env, "linter", mod, 0);
}
