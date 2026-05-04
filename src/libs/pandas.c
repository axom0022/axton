#include "../core/axton.h"
#include <math.h>
#include <string.h>

typedef struct dataframe {
    char **columns;
    int colcount;
    object **data;
    int rowcount;
} dataframe;

object *pddataframe(object *dict) {
    if (dict->type != 6) throwexception("pd.DataFrame expects dict");
    dataframe *df = malloc(sizeof(dataframe));
    df->colcount = dict->dict.count;
    df->columns = malloc(sizeof(char*) * df->colcount);
    df->data = malloc(sizeof(object*) * df->colcount);
    for (int i = 0; i < df->colcount; i++) {
        df->columns[i] = strdup(dict->dict.keys[i]);
        object *col = dict->dict.vals[i];
        if (col->type != 5) throwexception("column values must be list");
        df->data[i] = col;
        if (i == 0) df->rowcount = col->list.count;
        else if (col->list.count != df->rowcount) throwexception("columns have different lengths");
    }
    object *obj = makenative(df, NULL);
    obj->type = 60;
    return obj;
}

object *pdhead(object *dfobj, int n) {
    dataframe *df = (dataframe*)dfobj->native.data;
    if (n <= 0 || n > df->rowcount) n = 5;
    object *result = makedict();
    for (int i = 0; i < df->colcount; i++) {
        object *col = makelist();
        for (int j = 0; j < n && j < df->data[i]->list.count; j++) {
            listappend(col, df->data[i]->list.items[j]);
        }
        dictset(result, makestring(df->columns[i]), col);
    }
    return pddataframe(result);
}

object *pddescribe(object *dfobj) {
    dataframe *df = (dataframe*)dfobj->native.data;
    object *stats = makedict();
    for (int i = 0; i < df->colcount; i++) {
        object *col = df->data[i];
        if (col->list.count == 0) continue;
        double sum = 0, min = 1e100, max = -1e100;
        for (int j = 0; j < col->list.count; j++) {
            double val = (col->list.items[j]->type == 0) ? col->list.items[j]->ival : col->list.items[j]->fval;
            sum += val;
            if (val < min) min = val;
            if (val > max) max = val;
        }
        double mean = sum / col->list.count;
        double variance = 0;
        for (int j = 0; j < col->list.count; j++) {
            double val = (col->list.items[j]->type == 0) ? col->list.items[j]->ival : col->list.items[j]->fval;
            variance += pow(val - mean, 2);
        }
        variance /= col->list.count;
        double std = sqrt(variance);
        object *colstats = makedict();
        dictset(colstats, makestring("count"), makeint(col->list.count));
        dictset(colstats, makestring("mean"), makefloat(mean));
        dictset(colstats, makestring("std"), makefloat(std));
        dictset(colstats, makestring("min"), makefloat(min));
        dictset(colstats, makestring("max"), makefloat(max));
        dictset(stats, makestring(df->columns[i]), colstats);
    }
    return stats;
}

object *pdiloc(object *dfobj, int row) {
    dataframe *df = (dataframe*)dfobj->native.data;
    if (row < 0 || row >= df->rowcount) throwexception("row index out of range");
    object *rowobj = makedict();
    for (int i = 0; i < df->colcount; i++) {
        dictset(rowobj, makestring(df->columns[i]), df->data[i]->list.items[row]);
    }
    return rowobj;
}

object *pdloc(object *dfobj, object *condition) {
    return makenone();
}

object *pdgroupby(object *dfobj, char *column) {
    return makenone();
}

object *pdmerge(object *left, object *right, char *on) {
    return makenone();
}

object *pdconcat(object *list) {
    return makenone();
}

object *pdreadcsv(char *path) {
    char *content = platformreadfile(path);
    if (!content) throwexception("cannot read file");
    char *lines = strtok(content, "\n");
    if (!lines) { free(content); throwexception("empty file"); }
    char *headers = strdup(lines);
    char *colnames[256];
    int colcount = 0;
    char *token = strtok(headers, ",");
    while (token) {
        colnames[colcount++] = strdup(token);
        token = strtok(NULL, ",");
    }
    object *data = makedict();
    for (int i = 0; i < colcount; i++) {
        dictset(data, makestring(colnames[i]), makelist());
    }
    lines = strtok(NULL, "\n");
    while (lines) {
        char *values[256];
        int valcount = 0;
        token = strtok(lines, ",");
        while (token && valcount < colcount) {
            values[valcount++] = token;
            token = strtok(NULL, ",");
        }
        for (int i = 0; i < valcount && i < colcount; i++) {
            object *col = dictget(data, makestring(colnames[i]));
            double num = atof(values[i]);
            if (num == 0 && values[i][0] != '0') {
                listappend(col, makestring(values[i]));
            } else {
                listappend(col, makefloat(num));
            }
        }
        lines = strtok(NULL, "\n");
    }
    free(content);
    free(headers);
    for (int i = 0; i < colcount; i++) free(colnames[i]);
    return pddataframe(data);
}

object *pdtocsv(object *dfobj, char *path) {
    dataframe *df = (dataframe*)dfobj->native.data;
    char *buffer = malloc(65536);
    int pos = 0;
    for (int i = 0; i < df->colcount; i++) {
        pos += snprintf(buffer + pos, 65536 - pos, "%s%s", df->columns[i], (i < df->colcount - 1) ? "," : "\n");
    }
    for (int row = 0; row < df->rowcount; row++) {
        for (int col = 0; col < df->colcount; col++) {
            object *val = df->data[col]->list.items[row];
            if (val->type == 0) pos += snprintf(buffer + pos, 65536 - pos, "%ld", val->ival);
            else if (val->type == 1) pos += snprintf(buffer + pos, 65536 - pos, "%g", val->fval);
            else if (val->type == 2) pos += snprintf(buffer + pos, 65536 - pos, "%s", val->sval);
            else pos += snprintf(buffer + pos, 65536 - pos, "");
            if (col < df->colcount - 1) buffer[pos++] = ',';
        }
        buffer[pos++] = '\n';
    }
    buffer[pos] = 0;
    int result = platformwritefile(path, buffer);
    free(buffer);
    return makebool(result);
}

void registerpandaslib(environment *env) {
    object *pdmod = makemodule("pd", NULL);
    envset(pdmod->module.exports, "DataFrame", makebuiltin(pddataframe), 0);
    envset(pdmod->module.exports, "read_csv", makebuiltin(pdreadcsv), 0);
    envset(pdmod->module.exports, "head", makebuiltin(pdhead), 0);
    envset(pdmod->module.exports, "describe", makebuiltin(pddescribe), 0);
    envset(pdmod->module.exports, "iloc", makebuiltin(pdiloc), 0);
    envset(pdmod->module.exports, "loc", makebuiltin(pdloc), 0);
    envset(pdmod->module.exports, "groupby", makebuiltin(pdgroupby), 0);
    envset(pdmod->module.exports, "merge", makebuiltin(pdmerge), 0);
    envset(pdmod->module.exports, "concat", makebuiltin(pdconcat), 0);
    envset(pdmod->module.exports, "to_csv", makebuiltin(pdtocsv), 0);
    envset(env, "pd", pdmod, 0);
}
