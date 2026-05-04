#include "http.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct route {
    char *path;
    char *method;
    object *handler;
    struct route *next;
} route;

typedef struct session {
    char id[33];
    object *data;
    struct session *next;
} session;

typedef struct httpserver {
    route *routes;
    session *sessions;
    int port;
    int sslport;
    int running;
    pthread_t thread;
    char *staticdir;
    char *sesskey;
    SSL_CTX *sslctx;
} httpserver;

static httpserver server;
static pthread_mutex_t sessionlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t routelock = PTHREAD_MUTEX_INITIALIZER;

static char *getmimetype(char *path) {
    char *ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    ext++;
    if (strcmp(ext, "html") == 0) return "text/html";
    if (strcmp(ext, "css") == 0) return "text/css";
    if (strcmp(ext, "js") == 0) return "application/javascript";
    if (strcmp(ext, "json") == 0) return "application/json";
    if (strcmp(ext, "png") == 0) return "image/png";
    if (strcmp(ext, "jpg") == 0) return "image/jpeg";
    if (strcmp(ext, "jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, "gif") == 0) return "image/gif";
    if (strcmp(ext, "svg") == 0) return "image/svg+xml";
    if (strcmp(ext, "ico") == 0) return "image/x-icon";
    if (strcmp(ext, "txt") == 0) return "text/plain";
    if (strcmp(ext, "xml") == 0) return "application/xml";
    if (strcmp(ext, "pdf") == 0) return "application/pdf";
    if (strcmp(ext, "zip") == 0) return "application/zip";
    return "application/octet-stream";
}

static char *urlencode(char *s) {
    char *result = malloc(strlen(s) * 3 + 1);
    int ri = 0;
    for (int i = 0; s[i]; i++) {
        if ((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') ||
            (s[i] >= '0' && s[i] <= '9') || s[i] == '-' || s[i] == '_' ||
            s[i] == '.' || s[i] == '~') {
            result[ri++] = s[i];
        } else if (s[i] == ' ') {
            result[ri++] = '+';
        } else {
            ri += snprintf(result + ri, 4, "%%%02X", (unsigned char)s[i]);
        }
    }
    result[ri] = 0;
    return result;
}

static char *urldecode(char *s) {
    char *result = malloc(strlen(s) + 1);
    int ri = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '%' && s[i+1] && s[i+2]) {
            char hex[3] = {s[i+1], s[i+2], 0};
            result[ri++] = strtol(hex, NULL, 16);
            i += 2;
        } else if (s[i] == '+') {
            result[ri++] = ' ';
        } else {
            result[ri++] = s[i];
        }
    }
    result[ri] = 0;
    return result;
}

static object *parsequerystring(char *qs) {
    object *params = makedict();
    if (!qs || strlen(qs) == 0) return params;
    char *copy = strdup(qs);
    char *token = strtok(copy, "&");
    while (token) {
        char *eq = strchr(token, '=');
        if (eq) {
            *eq = 0;
            char *key = urldecode(token);
            char *val = urldecode(eq + 1);
            dictset(params, makestring(key), makestring(val));
            free(key);
            free(val);
        }
        token = strtok(NULL, "&");
    }
    free(copy);
    return params;
}

static char *generate_sessionid(void) {
    char id[33];
    char chars[] = "0123456789abcdef";
    for (int i = 0; i < 32; i++) {
        id[i] = chars[rand() % 16];
    }
    id[32] = 0;
    return strdup(id);
}

static session *getsession(char *id) {
    pthread_mutex_lock(&sessionlock);
    session *s = server.sessions;
    while (s) {
        if (strcmp(s->id, id) == 0) {
            pthread_mutex_unlock(&sessionlock);
            return s;
        }
        s = s->next;
    }
    pthread_mutex_unlock(&sessionlock);
    return NULL;
}

static session *createsession(void) {
    session *s = malloc(sizeof(session));
    char *id = generate_sessionid();
    strcpy(s->id, id);
    s->data = makedict();
    free(id);
    pthread_mutex_lock(&sessionlock);
    s->next = server.sessions;
    server.sessions = s;
    pthread_mutex_unlock(&sessionlock);
    return s;
}

static char *readfileto(char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    return buf;
}

static void sendresponse(int fd, int status, char *mime, char *body, object *headers, object *cookies) {
    char buf[65536];
    char *statusmsg = "";
    if (status == 200) statusmsg = "OK";
    else if (status == 201) statusmsg = "Created";
    else if (status == 400) statusmsg = "Bad Request";
    else if (status == 401) statusmsg = "Unauthorized";
    else if (status == 403) statusmsg = "Forbidden";
    else if (status == 404) statusmsg = "Not Found";
    else if (status == 500) statusmsg = "Internal Server Error";
    else statusmsg = "OK";
    int len = snprintf(buf, sizeof(buf), "HTTP/1.1 %d %s\r\n", status, statusmsg);
    len += snprintf(buf + len, sizeof(buf) - len, "Content-Type: %s\r\n", mime);
    if (headers) {
        object *keys = dictkeys(headers);
        for (int i = 0; i < keys->list.count; i++) {
            char *key = keys->list.items[i]->sval;
            object *val = dictget(headers, keys->list.items[i]);
            len += snprintf(buf + len, sizeof(buf) - len, "%s: %s\r\n", key, val->sval);
        }
    }
    if (cookies) {
        object *keys = dictkeys(cookies);
        for (int i = 0; i < keys->list.count; i++) {
            char *key = keys->list.items[i]->sval;
            object *val = dictget(cookies, keys->list.items[i]);
            len += snprintf(buf + len, sizeof(buf) - len, "Set-Cookie: %s=%s\r\n", key, val->sval);
        }
    }
    len += snprintf(buf + len, sizeof(buf) - len, "Content-Length: %ld\r\n", strlen(body));
    len += snprintf(buf + len, sizeof(buf) - len, "Connection: close\r\n");
    len += snprintf(buf + len, sizeof(buf) - len, "\r\n%s", body);
    send(fd, buf, len, 0);
}

static void sendfile(int fd, char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        sendresponse(fd, 404, "text/plain", "File not found", NULL, NULL);
        return;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *body = malloc(len);
    fread(body, 1, len, f);
    fclose(f);
    char *mime = getmimetype(path);
    sendresponse(fd, 200, mime, body, NULL, NULL);
    free(body);
}

static void *handleclient(void *arg) {
    int fd = *(int*)arg;
    free(arg);
    char buf[65536];
    int n = recv(fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close(fd);
        return NULL;
    }
    buf[n] = 0;
    char method[16] = {0};
    char path[1024] = {0};
    char version[16] = {0};
    sscanf(buf, "%s %s %s", method, path, version);
    char *query = strchr(path, '?');
    char *pathonly = path;
    if (query) {
        *query = 0;
        query++;
    }
    httprequest req;
    req.method = method;
    req.path = pathonly;
    req.query = query;
    req.headers = makedict();
    req.body = makenone();
    req.json = makenone();
    req.form = makedict();
    req.cookies = makedict();
    req.ip = NULL;
    req.sessionid = NULL;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    getpeername(fd, (struct sockaddr*)&addr, &addrlen);
    req.ip = inet_ntoa(addr.sin_addr);
    char *line = strtok(buf, "\r\n");
    line = strtok(NULL, "\r\n");
    int contentlength = 0;
    while (line && strlen(line) > 0) {
        if (strncmp(line, "Cookie: ", 8) == 0) {
            char *cookies = line + 8;
            char *token = strtok(cookies, "; ");
            while (token) {
                char *eq = strchr(token, '=');
                if (eq) {
                    *eq = 0;
                    dictset(req.cookies, makestring(token), makestring(eq + 1));
                }
                token = strtok(NULL, "; ");
            }
        } else if (strncmp(line, "Content-Length: ", 16) == 0) {
            contentlength = atoi(line + 16);
        } else {
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = 0;
                dictset(req.headers, makestring(line), makestring(colon + 2));
            }
        }
        line = strtok(NULL, "\r\n");
    }
    if (contentlength > 0) {
        char *body = strstr(buf, "\r\n\r\n");
        if (body) {
            body += 4;
            int bodylen = strlen(body);
            if (bodylen > contentlength) bodylen = contentlength;
            char *b = malloc(bodylen + 1);
            memcpy(b, body, bodylen);
            b[bodylen] = 0;
            req.body = makestring(b);
            free(b);
            if (strstr(req.headers->sval, "application/json")) {
                token *tokens = tokenize(req.body->sval);
                stmt *prog = parsetokens(tokens, tcount);
                req.json = evalprogram(prog, globalenv);
            } else if (strstr(req.headers->sval, "application/x-www-form-urlencoded")) {
                req.form = parsequerystring(req.body->sval);
            }
        }
    }
    char *sessioncookie = NULL;
    object *sid = dictget(req.cookies, makestring("sessionid"));
    if (sid && sid->type == 2) {
        req.sessionid = sid->sval;
        session *s = getsession(req.sessionid);
        if (!s) {
            s = createsession();
            req.sessionid = s->id;
        }
    } else {
        session *s = createsession();
        req.sessionid = s->id;
    }
    object *handler = NULL;
    pthread_mutex_lock(&routelock);
    route *r = server.routes;
    while (r) {
        if (strcmp(r->method, method) == 0 && strcmp(r->path, pathonly) == 0) {
            handler = r->handler;
            break;
        }
        r = r->next;
    }
    pthread_mutex_unlock(&routelock);
    httpresponse resp;
    resp.status = 200;
    resp.headers = makedict();
    resp.body = NULL;
    resp.json = NULL;
    resp.file = NULL;
    resp.cookies = makedict();
    if (handler) {
        object *reqobj = makenative(&req, NULL);
        reqobj->type = 30;
        object *resparg[1] = {reqobj};
        object *result = callfunc(handler, resparg, 1, globalenv);
        if (result && result->type == 31) {
            httpresponse *rresp = (httpresponse*)result->native.data;
            resp.status = rresp->status;
            resp.headers = rresp->headers;
            resp.body = rresp->body;
            resp.json = rresp->json;
            resp.file = rresp->file;
            resp.cookies = rresp->cookies;
        } else if (result && result->type == 2) {
            resp.body = result->sval;
        }
    } else if (server.staticdir && strncmp(pathonly, "/static/", 8) == 0) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", server.staticdir, pathonly + 8);
        sendfile(fd, fullpath);
        close(fd);
        return NULL;
    } else {
        resp.status = 404;
        resp.body = "Not Found";
    }
    if (resp.json) {
        char *jsonstr = builtinjsondumps(&resp.json, 1, globalenv)->sval;
        resp.body = jsonstr;
        dictset(resp.headers, makestring("Content-Type"), makestring("application/json"));
    }
    if (resp.file) {
        sendfile(fd, resp.file);
        close(fd);
        return NULL;
    }
    if (!resp.body) resp.body = "";
    dictset(resp.cookies, makestring("sessionid"), makestring(req.sessionid));
    sendresponse(fd, resp.status, "text/html", resp.body, resp.headers, resp.cookies);
    close(fd);
    return NULL;
}

static void *serverthread(void *arg) {
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(server.port);
    bind(serverfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(serverfd, 100);
    while (server.running) {
        int clientfd = accept(serverfd, NULL, NULL);
        if (clientfd < 0) continue;
        int *fdptr = malloc(sizeof(int));
        *fdptr = clientfd;
        pthread_t thread;
        pthread_create(&thread, NULL, handleclient, fdptr);
        pthread_detach(thread);
    }
    close(serverfd);
    return NULL;
}

object *httpcreateserver(int port) {
    server.port = port;
    server.running = 0;
    server.routes = NULL;
    server.sessions = NULL;
    server.staticdir = NULL;
    return makestring("server created");
}

object *httpget(char *path, object *handler) {
    pthread_mutex_lock(&routelock);
    route *r = malloc(sizeof(route));
    r->path = strdup(path);
    r->method = strdup("GET");
    r->handler = handler;
    r->next = server.routes;
    server.routes = r;
    pthread_mutex_unlock(&routelock);
    return makenone();
}

object *httppost(char *path, object *handler) {
    pthread_mutex_lock(&routelock);
    route *r = malloc(sizeof(route));
    r->path = strdup(path);
    r->method = strdup("POST");
    r->handler = handler;
    r->next = server.routes;
    server.routes = r;
    pthread_mutex_unlock(&routelock);
    return makenone();
}

object *httpput(char *path, object *handler) {
    pthread_mutex_lock(&routelock);
    route *r = malloc(sizeof(route));
    r->path = strdup(path);
    r->method = strdup("PUT");
    r->handler = handler;
    r->next = server.routes;
    server.routes = r;
    pthread_mutex_unlock(&routelock);
    return makenone();
}

object *httpdelete(char *path, object *handler) {
    pthread_mutex_lock(&routelock);
    route *r = malloc(sizeof(route));
    r->path = strdup(path);
    r->method = strdup("DELETE");
    r->handler = handler;
    r->next = server.routes;
    server.routes = r;
    pthread_mutex_unlock(&routelock);
    return makenone();
}

object *httpstatic(char *dir) {
    server.staticdir = strdup(dir);
    return makenone();
}

object *httpsessionstart(httprequest *req) {
    session *s = getsession(req->sessionid);
    if (!s) s = createsession();
    return s->data;
}

object *httpsessionget(httprequest *req, char *key) {
    session *s = getsession(req->sessionid);
    if (!s) return makenone();
    return dictget(s->data, makestring(key));
}

void httpsessionset(httprequest *req, char *key, object *val) {
    session *s = getsession(req->sessionid);
    if (!s) s = createsession();
    dictset(s->data, makestring(key), val);
}

object *httpjson(object *data) {
    httpresponse *resp = malloc(sizeof(httpresponse));
    resp->status = 200;
    resp->headers = makedict();
    resp->body = NULL;
    resp->json = data;
    resp->file = NULL;
    resp->cookies = makedict();
    object *obj = makenative(resp, free);
    obj->type = 31;
    return obj;
}

object *httphtml(char *html) {
    httpresponse *resp = malloc(sizeof(httpresponse));
    resp->status = 200;
    resp->headers = makedict();
    resp->body = strdup(html);
    resp->json = NULL;
    resp->file = NULL;
    resp->cookies = makedict();
    dictset(resp->headers, makestring("Content-Type"), makestring("text/html"));
    object *obj = makenative(resp, free);
    obj->type = 31;
    return obj;
}

object *httpfile(char *path) {
    httpresponse *resp = malloc(sizeof(httpresponse));
    resp->status = 200;
    resp->headers = makedict();
    resp->body = NULL;
    resp->json = NULL;
    resp->file = strdup(path);
    resp->cookies = makedict();
    object *obj = makenative(resp, free);
    obj->type = 31;
    return obj;
}

object *httpredirect(char *url) {
    httpresponse *resp = malloc(sizeof(httpresponse));
    resp->status = 302;
    resp->headers = makedict();
    dictset(resp->headers, makestring("Location"), makestring(url));
    resp->body = "";
    resp->json = NULL;
    resp->file = NULL;
    resp->cookies = makedict();
    object *obj = makenative(resp, free);
    obj->type = 31;
    return obj;
}

object *httpstatus(int code) {
    httpresponse *resp = malloc(sizeof(httpresponse));
    resp->status = code;
    resp->headers = makedict();
    resp->body = "";
    resp->json = NULL;
    resp->file = NULL;
    resp->cookies = makedict();
    object *obj = makenative(resp, free);
    obj->type = 31;
    return obj;
}

object *httpsetcookie(char *name, char *value, int days) {
    httpresponse *resp = malloc(sizeof(httpresponse));
    resp->status = 200;
    resp->headers = makedict();
    resp->body = NULL;
    resp->json = NULL;
    resp->file = NULL;
    resp->cookies = makedict();
    dictset(resp->cookies, makestring(name), makestring(value));
    object *obj = makenative(resp, free);
    obj->type = 31;
    return obj;
}

object *httpstart(void) {
    if (server.running) return makestring("server already running");
    server.running = 1;
    pthread_create(&server.thread, NULL, serverthread, NULL);
    pthread_detach(server.thread);
    return makestring("server started");
}

websocket *websocketupgrade(httprequest *req) {
    return NULL;
}

void websocketsend(websocket *ws, char *data) {}

void websocketbroadcast(char *data) {}

void websocketclose(websocket *ws) {}

void registerhttplib(environment *env) {
    object *httpmod = makemodule("http", NULL);
    envset(httpmod->module.exports, "createserver", makebuiltin(httpcreateserver), 0);
    envset(httpmod->module.exports, "get", makebuiltin(httpget), 0);
    envset(httpmod->module.exports, "post", makebuiltin(httppost), 0);
    envset(httpmod->module.exports, "put", makebuiltin(httpput), 0);
    envset(httpmod->module.exports, "delete", makebuiltin(httpdelete), 0);
    envset(httpmod->module.exports, "static", makebuiltin(httpstatic), 0);
    envset(httpmod->module.exports, "json", makebuiltin(httpjson), 0);
    envset(httpmod->module.exports, "html", makebuiltin(httphtml), 0);
    envset(httpmod->module.exports, "file", makebuiltin(httpfile), 0);
    envset(httpmod->module.exports, "redirect", makebuiltin(httpredirect), 0);
    envset(httpmod->module.exports, "status", makebuiltin(httpstatus), 0);
    envset(httpmod->module.exports, "cookie", makebuiltin(httpsetcookie), 0);
    envset(httpmod->module.exports, "start", makebuiltin(httpstart), 0);
    envset(env, "http", httpmod, 0);
}
