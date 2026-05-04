#include "../core/axton.h"
#include <stdlib.h>
#include <time.h>

static int seeded = 0;
static void seedonce(void) { if(!seeded){ srand(time(NULL)); seeded=1; } }

object *random_random(object **args, int argc, void *env) {
    seedonce();
    return makefloat((double)rand() / RAND_MAX);
}

object *random_randint(object **args, int argc, void *env) {
    seedonce();
    if(argc<2) throwexception("randint needs a and b");
    long a = args[0]->ival, b = args[1]->ival;
    long r = a + rand() % (b - a + 1);
    return makeint(r);
}

object *random_choice(object **args, int argc, void *env) {
    seedonce();
    if(argc<1 || args[0]->type!=5) throwexception("choice needs list");
    if(args[0]->list.count==0) throwexception("empty list");
    int idx = rand() % args[0]->list.count;
    return args[0]->list.items[idx];
}

object *random_shuffle(object **args, int argc, void *env) {
    seedonce();
    if(argc<1 || args[0]->type!=5) throwexception("shuffle needs list");
    int n = args[0]->list.count;
    for(int i=n-1;i>0;i--){
        int j = rand() % (i+1);
        object *tmp = args[0]->list.items[i];
        args[0]->list.items[i] = args[0]->list.items[j];
        args[0]->list.items[j] = tmp;
    }
    return makenone();
}

object *random_seed(object **args, int argc, void *env) {
    unsigned int s = (argc>0 && args[0]->type==0) ? args[0]->ival : time(NULL);
    srand(s);
    seeded = 1;
    return makenone();
}

void registerrandomlib(environment *env) {
    object *mod = makemodule("random", NULL);
    envset(mod->module.exports, "random", makebuiltin(random_random), 0);
    envset(mod->module.exports, "randint", makebuiltin(random_randint), 0);
    envset(mod->module.exports, "choice", makebuiltin(random_choice), 0);
    envset(mod->module.exports, "shuffle", makebuiltin(random_shuffle), 0);
    envset(mod->module.exports, "seed", makebuiltin(random_seed), 0);
    envset(env, "random", mod, 0);
}
