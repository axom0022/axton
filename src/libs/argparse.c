#include "../core/axton.h"

object *argparseparser(object **args, int argc, void *env) {
    object *parser = makedict();
    dictset(parser, makestring("args"), makelist(), 0);
    return parser;
}

object *argparseaddarg(object **args, int argc, void *env) {
    if(argc<2 || args[0]->type!=6) throwexception("add_argument needs parser and name");
    object *parser = args[0];
    char *name = args[1]->sval;
    object *arg = makedict();
    dictset(arg, makestring("name"), makestring(name), 0);
    if(argc>2 && args[2]->type==6 && dictget(args[2],makestring("help")))
        dictset(arg, makestring("help"), dictget(args[2],makestring("help")), 0);
    listappend(dictget(parser,makestring("args")), arg);
    return parser;
}

object *argparseparseargs(object **args, int argc, void *env) {
    if(argc<1 || args[0]->type!=6) throwexception("parse_args needs parser");
    object *parser = args[0];
    object *result = makedict();
    object *arglist = dictget(parser,makestring("args"));
    for(int i=0;i<arglist->list.count;i++){
        object *arg = arglist->list.items[i];
        char *name = dictget(arg,makestring("name"))->sval;
        char *val = "";
        if(name[0]=='-') val = "value";
        dictset(result, makestring(name+(name[1]=='-'?2:1)), makestring(val), 0);
    }
    return result;
}

void registerargparselib(environment *env) {
    object *mod = makemodule("argparse", NULL);
    envset(mod->module.exports, "ArgumentParser", makebuiltin(argparseparser), 0);
    envset(mod->module.exports, "add_argument", makebuiltin(argparseaddarg), 0);
    envset(mod->module.exports, "parse_args", makebuiltin(argparseparseargs), 0);
    envset(env, "argparse", mod, 0);
}
