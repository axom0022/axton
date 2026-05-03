#include "../core/axton.h"

typedef struct {
    int fd;
    char method[16];
    char path[256];
    object *handler;
} route;

static route routes[100];
static int routecount = 0;
static int serverfd = -1;
static int serverrunning = 0;

#ifdef _WIN32
#include <winsock2.h>
static void webinit(void) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
}
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
static void webinit(void) {}
#endif

static int webstartserver(int port) {
    webinit();
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) return 0;
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(serverfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return 0;
    if (listen(serverfd, 10) < 0) return 0;
    serverrunning = 1;
    return 1;
}

static void webacceptloop(void) {
    char buffer[65536];
    while (serverrunning) {
        int client = accept(serverfd, NULL, NULL);
        if (client < 0) continue;
        int n = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = 0;
            char method[16] = {0};
            char path[256] = {0};
            sscanf(buffer, "%s %s", method, path);
            object *handler = NULL;
            for (int i = 0; i < routecount; i++) {
                if (strcmp(routes[i].method, method) == 0 && strcmp(routes[i].path, path) == 0) {
                    handler = routes[i].handler;
                    break;
                }
            }
            if (handler) {
                object *req = makedict();
                dictset(req, makestring("method"), makestring(method));
                dictset(req, makestring("path"), makestring(path));
                char *body = strstr(buffer, "\r\n\r\n");
                if (body) dictset(req, makestring("body"), makestring(body + 4));
                object *resp = callfunc(handler, &req, 1, globalenv);
                char *resbody = "ok";
                if (resp && resp->type == 2) resbody = resp->sval;
                char response[65536];
                snprintf(response, sizeof(response),
                    "HTTP/1.1 200 OK\r\n"
                    "ContentType: text/plain\r\n"
                    "ContentLength: %ld\r\n"
                    "\r\n%s", strlen(resbody), resbody);
                send(client, response, strlen(response), 0);
            } else {
                send(client, "HTTP/1.1 404 Not Found\r\n\r\n", 26, 0);
            }
        }
#ifdef _WIN32
        closesocket(client);
#else
        close(client);
#endif
    }
}

object *webstart(int port) {
    if (!webstartserver(port)) {
        throwexception("failed to start server");
        return makenone();
    }
    webacceptloop();
    return makestring("server started");
}

object *webget(char *path, object *handler) {
    if (routecount < 100) {
        strcpy(routes[routecount].method, "GET");
        strcpy(routes[routecount].path, path);
        routes[routecount].handler = handler;
        routecount++;
    }
    return makenone();
}

object *webpost(char *path, object *handler) {
    if (routecount < 100) {
        strcpy(routes[routecount].method, "POST");
        strcpy(routes[routecount].path, path);
        routes[routecount].handler = handler;
        routecount++;
    }
    return makenone();
}

object *websendjson(object *data) {
    char *json = "{\"status\":\"ok\"}";
    if (data && data->type == 2) {
        json = data->sval;
    }
    char *resp = malloc(strlen(json) + 50);
    sprintf(resp, "{\"data\":%s}", json);
    return makestring(resp);
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

object *builtinwebpost(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("web.post needs path and handler");
    return webpost(args[0]->sval, args[1]);
}

object *builtinwebjson(object **args, int argc, environment *env) {
    return websendjson(argc > 0 ? args[0] : NULL);
}

object *builtinwebhtml(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("web.html needs content");
    return websendhtml(args[0]->sval);
}

void registerweblib(environment *env) {
    object *webmod = makemodule("web", NULL);
    envset(webmod->module.exports, "start", makebuiltin(builtinwebstart), 0);
    envset(webmod->module.exports, "get", makebuiltin(builtinwebget), 0);
    envset(webmod->module.exports, "post", makebuiltin(builtinwebpost), 0);
    envset(webmod->module.exports, "json", makebuiltin(builtinwebjson), 0);
    envset(webmod->module.exports, "html", makebuiltin(builtinwebhtml), 0);
    envset(env, "web", webmod, 0);
}
