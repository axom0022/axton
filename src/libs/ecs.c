#include "../core/axton.h"
#include <stdlib.h>
#include <string.h>

typedef struct ecsentity {
    int id;
    int *componentmask;
    void **components;
    int maxcomp;
} ecsentity;

typedef struct ecscomponent {
    int type;
    int size;
    void *data;
} ecscomponent;

typedef struct ecssystem {
    int type;
    object *updatefn;
} ecssystem;

static ecsentity *entities = NULL;
static int entitycount = 0;
static int entitycap = 0;
static ecscomponent **comptypes = NULL;
static int comptypescount = 0;
static ecssystem *systems = NULL;
static int systemcount = 0;

object *ecscreate(object **args, int argc, void *env) {
    if (entitycount >= entitycap) {
        entitycap = entitycap ? entitycap*2 : 16;
        entities = realloc(entities, entitycap * sizeof(ecsentity));
    }
    ecsentity *e = &entities[entitycount++];
    e->id = entitycount;
    e->maxcomp = 0;
    e->componentmask = NULL;
    e->components = NULL;
    return makeint(e->id);
}

object *ecsaddcomp(object **args, int argc, void *env) {
    if (argc<3) throwexception("addcomp needs entity type data");
    int eid = args[0]->ival;
    int type = args[1]->ival;
    char *data = args[2]->sval;
    ecsentity *ent = NULL;
    for (int i=0;i<entitycount;i++) if (entities[i].id == eid) { ent=&entities[i]; break; }
    if (!ent) throwexception("entity not found");
    int compidx = -1;
    for (int i=0;i<ent->maxcomp;i++) if (ent->componentmask[i] == type) { compidx=i; break; }
    if (compidx == -1) {
        ent->maxcomp++;
        ent->componentmask = realloc(ent->componentmask, ent->maxcomp * sizeof(int));
        ent->components = realloc(ent->components, ent->maxcomp * sizeof(void*));
        compidx = ent->maxcomp-1;
        ent->componentmask[compidx] = type;
        ent->components[compidx] = malloc(strlen(data)+1);
        strcpy(ent->components[compidx], data);
    } else {
        free(ent->components[compidx]);
        ent->components[compidx] = malloc(strlen(data)+1);
        strcpy(ent->components[compidx], data);
    }
    return makenone();
}

object *ecsgetcomp(object **args, int argc, void *env) {
    if (argc<2) throwexception("getcomp needs entity type");
    int eid=args[0]->ival, type=args[1]->ival;
    ecsentity *ent=NULL;
    for (int i=0;i<entitycount;i++) if (entities[i].id==eid) { ent=&entities[i]; break; }
    if (!ent) throwexception("entity not found");
    for (int i=0;i<ent->maxcomp;i++) if (ent->componentmask[i]==type) {
        return makestring(ent->components[i]);
    }
    return makenone();
}

object *ecsremcomp(object **args, int argc, void *env) {
    if (argc<2) throwexception("remcomp needs entity type");
    int eid=args[0]->ival, type=args[1]->ival;
    ecsentity *ent=NULL;
    for (int i=0;i<entitycount;i++) if (entities[i].id==eid) { ent=&entities[i]; break; }
    if (!ent) throwexception("entity not found");
    for (int i=0;i<ent->maxcomp;i++) if (ent->componentmask[i]==type) {
        free(ent->components[i]);
        for (int j=i;j<ent->maxcomp-1;j++) {
            ent->componentmask[j]=ent->componentmask[j+1];
            ent->components[j]=ent->components[j+1];
        }
        ent->maxcomp--;
        ent->componentmask = realloc(ent->componentmask, ent->maxcomp * sizeof(int));
        ent->components = realloc(ent->components, ent->maxcomp * sizeof(void*));
        break;
    }
    return makenone();
}

object *ecsaddsystem(object **args, int argc, void *env) {
    if (argc<2) throwexception("addsystem needs type and updatefn");
    int type = args[0]->ival;
    object *fn = args[1];
    systemcount++;
    systems = realloc(systems, systemcount * sizeof(ecssystem));
    systems[systemcount-1].type = type;
    systems[systemcount-1].updatefn = fn;
    return makenone();
}

object *ecsupdate(object **args, int argc, void *env) {
    (void)args; (void)argc;
    for (int i=0;i<systemcount;i++) {
        for (int j=0;j<entitycount;j++) {
            ecsentity *ent = &entities[j];
            int has = 0;
            for (int k=0;k<ent->maxcomp;k++) if (ent->componentmask[k]==systems[i].type) { has=1; break; }
            if (has && systems[i].updatefn) {
                object *params[2] = {makeint(ent->id), makeint(systems[i].type)};
                callfunc(systems[i].updatefn, params, 2, env);
            }
        }
    }
    return makenone();
}

void registerecslib(environment *env) {
    object *mod = makemodule("ecs", NULL);
    envset(mod->module.exports, "create", makebuiltin(ecscreate), 0);
    envset(mod->module.exports, "addcomp", makebuiltin(ecsaddcomp), 0);
    envset(mod->module.exports, "getcomp", makebuiltin(ecsgetcomp), 0);
    envset(mod->module.exports, "remcomp", makebuiltin(ecsremcomp), 0);
    envset(mod->module.exports, "addsystem", makebuiltin(ecsaddsystem), 0);
    envset(mod->module.exports, "update", makebuiltin(ecsupdate), 0);
    envset(env, "ecs", mod, 0);
}
