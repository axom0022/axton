#include "../core/axton.h"
#include <regex.h>

object *builtinrematch(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("re.match needs pattern and string");
    regex_t regex;
    if (regcomp(&regex, args[0]->sval, REG_EXTENDED) != 0) {
        throwexception("invalid regex");
        return NULL;
    }
    int result = regexec(&regex, args[1]->sval, 0, NULL, 0);
    regfree(&regex);
    return makebool(result == 0);.
}

object *builtinresearch(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("re.search needs pattern and string");
    regex_t regex;
    if (regcomp(&regex, args[0]->sval, REG_EXTENDED) != 0) {
        throwexception("invalid regex");
        return NULL;
    }
    regmatch_t match;
    int result = regexec(&regex, args[1]->sval, 1, &match, 0);
    regfree(&regex);
    if (result != 0) return makenone();
    int len = match.rm_eo - match.rm_so;
    char *matched = malloc(len + 1);
    memcpy(matched, args[1]->sval + match.rm_so, len);
    matched[len] = 0;
    return makestring(matched);
}

object *builtinrefindall(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("re.findall needs pattern and string");
    regex_t regex;
    if (regcomp(&regex, args[0]->sval, REG_EXTENDED) != 0) {
        throwexception("invalid regex");
        return NULL;
    }
    object *list = makelist();
    const char *str = args[1]->sval;
    regmatch_t match;
    int offset = 0;
    while (regexec(&regex, str + offset, 1, &match, 0) == 0) {
        int len = match.rm_eo - match.rm_so;
        char *found = malloc(len + 1);
        memcpy(found, str + offset + match.rm_so, len);
        found[len] = 0;
        listappend(list, makestring(found));
        offset += match.rm_eo;
        if (match.rm_so == match.rm_eo) break;
    }
    regfree(&regex);
    return list;
}

object *builtinresub(object **args, int argc, environment *env) {
    if (argc < 3) throwexception("re.sub needs pattern, repl, string");
    regex_t regex;
    if (regcomp(&regex, args[0]->sval, REG_EXTENDED) != 0) {
        throwexception("invalid regex");
        return NULL;
    }
    char *result = malloc(strlen(args[2]->sval) + 256);
    char *ptr = result;
    const char *str = args[2]->sval;
    regmatch_t match;
    int offset = 0;
    while (regexec(&regex, str + offset, 1, &match, 0) == 0) {
        int len = match.rm_so;
        memcpy(ptr, str + offset, len);
        ptr += len;
        strcpy(ptr, args[1]->sval);
        ptr += strlen(args[1]->sval);
        offset += match.rm_eo;
        if (match.rm_so == match.rm_eo) break;
    }
    strcpy(ptr, str + offset);
    regfree(&regex);
    return makestring(result);
}

object *builtinresplit(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("re.split needs pattern and string");
    regex_t regex;
    if (regcomp(&regex, args[0]->sval, REG_EXTENDED) != 0) {
        throwexception("invalid regex");
        return NULL;
    }
    object *list = makelist();
    const char *str = args[1]->sval;
    regmatch_t match;
    int offset = 0;
    while (regexec(&regex, str + offset, 1, &match, 0) == 0) {
        int len = match.rm_so;
        char *part = malloc(len + 1);
        memcpy(part, str + offset, len);
        part[len] = 0;
        listappend(list, makestring(part));
        offset += match.rm_eo;
        if (match.rm_so == match.rm_eo) break;
    }
    listappend(list, makestring(str + offset));
    regfree(&regex);
    return list;
}

object *builtinrecompile(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("re.compile needs pattern");
    regex_t regex;
    if (regcomp(&regex, args[0]->sval, REG_EXTENDED) != 0) {
        throwexception("invalid regex");
        return NULL;
    }
    object *obj = makemodule("regex", NULL);
    obj->type = 24;
    obj->regexobj.regex = regex;
    obj->regexobj.pattern = strdup(args[0]->sval);
    return obj;
}

void registerrelib(environment *env) {
    object *remod = makemodule("re", NULL);
    envset(remod->module.exports, "match", makebuiltin(builtinrematch), 0);
    envset(remod->module.exports, "search", makebuiltin(builtinresearch), 0);
    envset(remod->module.exports, "findall", makebuiltin(builtinrefindall), 0);
    envset(remod->module.exports, "sub", makebuiltin(builtinresub), 0);
    envset(remod->module.exports, "split", makebuiltin(builtinresplit), 0);
    envset(remod->module.exports, "compile", makebuiltin(builtinrecompile), 0);
    envset(env, "re", remod, 0);
}
