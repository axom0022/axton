#include "../core/axton.h"
. 
static object *jsonparse(char *s, int *pos) {
    while (s[*pos] == ' ' || s[*pos] == '\t' || s[*pos] == '\n') (*pos)++;
    if (s[*pos] == '"') {
        (*pos)++;
        int start = *pos;
        while (s[*pos] && s[*pos] != '"') (*pos)++;
        int len = *pos - start;
        char *val = malloc(len + 1);
        memcpy(val, s + start, len);
        val[len] = 0;
        (*pos)++;
        return makestring(val);
    }
    if (s[*pos] >= '0' && s[*pos] <= '9') {
        int start = *pos;
        while (s[*pos] && (isdigit(s[*pos]) || s[*pos] == '.')) (*pos)++;
        int len = *pos - start;
        char *num = malloc(len + 1);
        memcpy(num, s + start, len);
        num[len] = 0;
        if (strchr(num, '.')) return makefloat(atof(num));
        else return makeint(atol(num));
    }
    if (strncmp(s + *pos, "true", 4) == 0) { *pos += 4; return makebool(1); }
    if (strncmp(s + *pos, "false", 5) == 0) { *pos += 5; return makebool(0); }
    if (strncmp(s + *pos, "null", 4) == 0) { *pos += 4; return makenone(); }
    if (s[*pos] == '[') {
        (*pos)++;
        object *arr = makelist();
        while (s[*pos] && s[*pos] != ']') {
            object *item = jsonparse(s, pos);
            if (item) listappend(arr, item);
            if (s[*pos] == ',') (*pos)++;
        }
        if (s[*pos] == ']') (*pos)++;
        return arr;
    }
    if (s[*pos] == '{') {
        (*pos)++;
        object *obj = makedict();
        while (s[*pos] && s[*pos] != '}') {
            object *key = jsonparse(s, pos);
            if (s[*pos] == ':') (*pos)++;
            object *val = jsonparse(s, pos);
            if (key && val) dictset(obj, key, val);
            if (s[*pos] == ',') (*pos)++;
        }
        if (s[*pos] == '}') (*pos)++;
        return obj;
    }
    return NULL;
}

static void jsonstringify(object *obj, char **out, int *len) {
    if (obj->type == 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%ld", obj->ival);
        *out = realloc(*out, *len + strlen(buf) + 1);
        strcpy(*out + *len, buf);
        *len += strlen(buf);
    }
    else if (obj->type == 1) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%g", obj->fval);
        *out = realloc(*out, *len + strlen(buf) + 1);
        strcpy(*out + *len, buf);
        *len += strlen(buf);
    }
    else if (obj->type == 2) {
        *out = realloc(*out, *len + strlen(obj->sval) + 3);
        (*out)[*len] = '"';
        (*len)++;
        strcpy(*out + *len, obj->sval);
        *len += strlen(obj->sval);
        (*out)[*len] = '"';
        (*len)++;
    }
    else if (obj->type == 3) {
        if (obj->bval) { *out = realloc(*out, *len + 5); strcpy(*out + *len, "true"); *len += 4; }
        else { *out = realloc(*out, *len + 6); strcpy(*out + *len, "false"); *len += 5; }
    }
    else if (obj->type == 4) {
        *out = realloc(*out, *len + 5);
        strcpy(*out + *len, "null");
        *len += 4;
    }
    else if (obj->type == 5) {
        *out = realloc(*out, *len + 2);
        (*out)[*len] = '[';
        (*len)++;
        for (int i = 0; i < obj->list.count; i++) {
            jsonstringify(obj->list.items[i], out, len);
            if (i < obj->list.count - 1) {
                *out = realloc(*out, *len + 3);
                strcpy(*out + *len, ", ");
                *len += 2;
            }
        }
        *out = realloc(*out, *len + 2);
        (*out)[*len] = ']';
        (*len)++;
    }
    else if (obj->type == 6) {
        *out = realloc(*out, *len + 2);
        (*out)[*len] = '{';
        (*len)++;
        for (int i = 0; i < obj->dict.count; i++) {
            jsonstringify(obj->dict.keyvals[i], out, len);
            *out = realloc(*out, *len + 3);
            strcpy(*out + *len, ": ");
            *len += 2;
            jsonstringify(obj->dict.vals[i], out, len);
            if (i < obj->dict.count - 1) {
                *out = realloc(*out, *len + 3);
                strcpy(*out + *len, ", ");
                *len += 2;
            }
        }
        *out = realloc(*out, *len + 2);
        (*out)[*len] = '}';
        (*len)++;
    }
    (*out)[*len] = 0;
}

object *builtinjsonloads(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("json.loads needs string");
    if (args[0]->type != 2) throwexception("json.loads expects string");
    int pos = 0;
    return jsonparse(args[0]->sval, &pos);
}

object *builtinjsondumps(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("json.dumps needs object");
    char *out = malloc(1);
    int len = 0;
    out[0] = 0;
    jsonstringify(args[0], &out, &len);
    return makestring(out);
}

void registerjsonlib(environment *env) {
    object *jsonmod = makemodule("json", NULL);
    envset(jsonmod->module.exports, "loads", makebuiltin(builtinjsonloads), 0);
    envset(jsonmod->module.exports, "dumps", makebuiltin(builtinjsondumps), 0);
    envset(env, "json", jsonmod, 0);
}
