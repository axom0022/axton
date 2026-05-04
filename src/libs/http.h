#ifndef http_h
#define http_h

#include "../core/axton.h"

typedef struct httprequest {
    char *method;
    char *path;
    char *query;
    object *headers;
    object *body;
    object *json;
    object *form;
    object *cookies;
    char *ip;
    char *sessionid;
} httprequest;

typedef struct httpresponse {
    int status;
    object *headers;
    char *body;
    object *json;
    char *file;
    object *cookies;
} httpresponse;

typedef struct websocket {
    int fd;
    object *onmessage;
    object *onclose;
    object *onerror;
    void *data;
} websocket;

object *httpcreateserver(int port);
object *httpget(char *path, object *handler);
object *httppost(char *path, object *handler);
object *httpput(char *path, object *handler);
object *httpdelete(char *path, object *handler);
object *httpstatic(char *dir);
object *httpsessionstart(httprequest *req);
object *httpsessionget(httprequest *req, char *key);
void httpsessionset(httprequest *req, char *key, object *val);
object *httpjson(object *data);
object *httphtml(char *html);
object *httpfile(char *path);
object *httpredirect(char *url);
object *httpstatus(int code);
object *httpsetcookie(char *name, char *value, int days);
object *httpstart(void);

websocket *websocketupgrade(httprequest *req);
void websocketsend(websocket *ws, char *data);
void websocketbroadcast(char *data);
void websocketclose(websocket *ws);

#endif
