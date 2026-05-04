#include "../core/axton.h"

object *arrayarray(object **a, int c, void *e) {
    object *arr = makelist();
    if(c>1 && a[1]->type==5){
        for(int i=0;i<a[1]->list.count;i++) listappend(arr, a[1]->list.items[i]);
    }
    return arr;
}
void registerarraylib(environment *env) {
    object *mod = makemodule("array", NULL);
    envset(mod->module.exports, "array", makebuiltin(arrayarray), 0);
    envset(env, "array", mod, 0);
}
