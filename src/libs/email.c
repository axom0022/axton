#include "../core/axton.h"

object *emailmessage(object **a, int c, void *e) {
    return makedict();
}

object *emailsetfrom(object **a, int c, void *e) {
    if(c<2) return makedict();
    dictset(a[0], makestring("From"), a[1]);
    return a[0];
}

object *emailsetto(object **a, int c, void *e) {
    if(c<2) return makedict();
    dictset(a[0], makestring("To"), a[1]);
    return a[0];
}

object *emailsetsubject(object **a, int c, void *e) {
    if(c<2) return makedict();
    dictset(a[0], makestring("Subject"), a[1]);
    return a[0];
}

object *emailsetbody(object **a, int c, void *e) {
    if(c<2) return makedict();
    dictset(a[0], makestring("Body"), a[1]);
    return a[0];
}

object *emailtostring(object **a, int c, void *e) {
    if(c<1) return makestring("");
    char *from = dictget(a[0], makestring("From"))->sval;
    char *to = dictget(a[0], makestring("To"))->sval;
    char *subj = dictget(a[0], makestring("Subject"))->sval;
    char *body = dictget(a[0], makestring("Body"))->sval;
    char buf[4096];
    snprintf(buf,4096,"From: %s\nTo: %s\nSubject: %s\n\n%s", from, to, subj, body);
    return makestring(buf);
}

void registeremaillib(environment *env) {
    object *mod = makemodule("email", NULL);
    envset(mod->module.exports, "Message", makebuiltin(emailmessage), 0);
    envset(mod->module.exports, "set_from", makebuiltin(emailsetfrom), 0);
    envset(mod->module.exports, "set_to", makebuiltin(emailsetto), 0);
    envset(mod->module.exports, "set_subject", makebuiltin(emailsetsubject), 0);
    envset(mod->module.exports, "set_body", makebuiltin(emailsetbody), 0);
    envset(mod->module.exports, "to_string", makebuiltin(emailtostring), 0);
    envset(env, "email", mod, 0);
}
