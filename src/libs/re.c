#include "../core/axton.h"
#include <regex.h>

object *rematch(object **a, int c, void *e) {
    if (c < 2 || a[0]->type != 2 || a[1]->type != 2)
        throwexception("match needs pattern and string");
    regex_t re;
    if (regcomp(&re, a[0]->sval, REG_EXTENDED) != 0)
        throwexception("invalid regex");
    int res = regexec(&re, a[1]->sval, 0, NULL, 0);
    regfree(&re);
    return makebool(res == 0);
}

object *research(object **a, int c, void *e) {
    if (c < 2) throwexception("search needs pattern and string");
    regex_t re;
    if (regcomp(&re, a[0]->sval, REG_EXTENDED) != 0)
        throwexception("invalid regex");
    regmatch_t match;
    int res = regexec(&re, a[1]->sval, 1, &match, 0);
    regfree(&re);
    if (res != 0) return makenone();
    int len = match.rm_eo - match.rm_so;
    char *found = malloc(len + 1);
    memcpy(found, a[1]->sval + match.rm_so, len);
    found[len] = 0;
    object *result = makestring(found);
    free(found);
    return result;
}

object *refindall(object **a, int c, void *e) {
    if (c < 2) throwexception("findall needs pattern and string");
    regex_t re;
    if (regcomp(&re, a[0]->sval, REG_EXTENDED) != 0)
        throwexception("invalid regex");
    object *list = makelist();
    const char *str = a[1]->sval;
    regmatch_t match;
    int offset = 0;
    while (regexec(&re, str + offset, 1, &match, 0) == 0) {
        int len = match.rm_eo - match.rm_so;
        char *found = malloc(len + 1);
        memcpy(found, str + offset + match.rm_so, len);
        found[len] = 0;
        listappend(list, makestring(found));
        offset += match.rm_eo;
        if (match.rm_so == match.rm_eo) break;
    }
    regfree(&re);
    return list;
}

object *resub(object **a, int c, void *e) {
    if (c < 3) throwexception("sub needs pattern repl string");
    regex_t re;
    if (regcomp(&re, a[0]->sval, REG_EXTENDED) != 0)
        throwexception("invalid regex");
    char *result = malloc(strlen(a[2]->sval) + 256);
    char *ptr = result;
    const char *str = a[2]->sval;
    regmatch_t match;
    int offset = 0;
    while (regexec(&re, str + offset, 1, &match, 0) == 0) {
        int len = match.rm_so;
        memcpy(ptr, str + offset, len);
        ptr += len;
        strcpy(ptr, a[1]->sval);
        ptr += strlen(a[1]->sval);
        offset += match.rm_eo;
        if (match.rm_so == match.rm_eo) break;
    }
    strcpy(ptr, str + offset);
    regfree(&re);
    return makestring(result);
}

object *resplit(object **a, int c, void *e) {
    if (c < 2) throwexception("split needs pattern and string");
    regex_t re;
    if (regcomp(&re, a[0]->sval, REG_EXTENDED) != 0)
        throwexception("invalid regex");
    object *list = makelist();
    const char *str = a[1]->sval;
    regmatch_t match;
    int offset = 0;
    while (regexec(&re, str + offset, 1, &match, 0) == 0) {
        int len = match.rm_so;
        char *part = malloc(len + 1);
        memcpy(part, str + offset, len);
        part[len] = 0;
        listappend(list, makestring(part));
        offset += match.rm_eo;
        if (match.rm_so == match.rm_eo) break;
    }
    listappend(list, makestring(str + offset));
    regfree(&re);
    return list;
}

object *recompile(object **a, int c, void *e) {
    if (c < 1) throwexception("compile needs pattern");
    regex_t *re = malloc(sizeof(regex_t));
    if (regcomp(re, a[0]->sval, REG_EXTENDED) != 0) {
        free(re);
        throwexception("invalid regex");
    }
    object *obj = makenative(re, free);
    return obj;
}

void registerrelib(environment *env) {
    object *mod = makemodule("re", NULL);
    envset(mod->module.exports, "match", makebuiltin(rematch), 0);
    envset(mod->module.exports, "search", makebuiltin(research), 0);
    envset(mod->module.exports, "findall", makebuiltin(refindall), 0);
    envset(mod->module.exports, "sub", makebuiltin(resub), 0);
    envset(mod->module.exports, "split", makebuiltin(resplit), 0);
    envset(mod->module.exports, "compile", makebuiltin(recompile), 0);
    envset(env, "re", mod, 0);
}
