#include "../core/axton.h"

object *csvreader(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=2) throwexception("reader needs string");
    char *s = args[0]->sval;
    object *rows = makelist();
    char *line = strtok(s, "\n");
    while(line){
        object *row = makelist();
        char *field = strtok(line, ",");
        while(field){
            listappend(row, makestring(field));
            field = strtok(NULL, ",");
        }
        listappend(rows, row);
        line = strtok(NULL, "\n");
    }
    return rows;
}

object *csvwriter(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=5) throwexception("writer needs list of rows");
    object *rows = args[0];
    char *out = malloc(1);
    out[0]=0;
    int total=0;
    for(int i=0;i<rows->list.count;i++){
        object *row = rows->list.items[i];
        for(int j=0;j<row->list.count;j++){
            char *field = row->list.items[j]->sval;
            int len = strlen(field);
            out = realloc(out, total+len+2);
            strcpy(out+total, field);
            total+=len;
            if(j<row->list.count-1){ out[total++]=','; out[total]=0; }
        }
        out[total++]='\n';
        out[total]=0;
    }
    object *res = makestring(out);
    free(out);
    return res;
}

void registercsvlib(environment *env) {
    object *mod = makemodule("csv", NULL);
    envset(mod->module.exports, "reader", makebuiltin(csvreader), 0);
    envset(mod->module.exports, "writer", makebuiltin(csvwriter), 0);
    envset(env, "csv", mod, 0);
}
