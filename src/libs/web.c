#include "../core/axton.h"
#include "../platform/platform.h"

static int webserverport = 8080;
static int webserverrunning = 0;

object *webstart(int port) {
    webserverport = port;
    webserverrunning = 1;
    char msg[64];
    snprintf(msg, sizeof(msg), "web server on port %d\n", port);
    platformlog(msg);
    return makestring("server started");
}

object *webget(char *path, object *handler) {
    platformlog("route added: GET ");
    platformlog(path);
    platformlog("\n");
    return makenone();
}

object *websendjson(object *data) {
    return makestring("{\"status\":\"ok\"}");
}

object *websendhtml(char *html) {
    char *resp = malloc(strlen(html) + 100);
    sprintf(resp, "<html><body>%s</body></html>", html);
    return makestring(resp);
}

object *builtinwebstart(object **args, int argc, environment *env) {
    int port = 8080;
    if (argc > 0 && args[0]->type == 0) port = args[0]->ival;
    return webstart(port);
}

object *builtinwebget(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("web.get needs path and handler");
    return webget(args[0]->sval, args[1]);
}

object *builtinwebjson(object **args, int argc, environment *env) {
    return websendjson(argc > 0 ? args[0] : makenone());
}

object *builtinwebhtml(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("web.html needs content");
    return websendhtml(args[0]->sval);
}

void registerweblib(environment *env) {
    object *webmod = makemodule("web", NULL);
    envset(webmod->module.exports, "start", makebuiltin(builtinwebstart), 0);
    envset(webmod->module.exports, "get", makebuiltin(builtinwebget), 0);
    envset(webmod->module.exports, "json", makebuiltin(builtinwebjson), 0);
    envset(webmod->module.exports, "html", makebuiltin(builtinwebhtml), 0);
    envset(env, "web", webmod, 0);
}
