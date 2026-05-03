#include "../core/axton.h"
#include <sqlite3.h>

typedef struct {
    sqlite3 *db;
} dbconn;

object *dbconnect(char *path) {
    sqlite3 *db;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        throwexception(sqlite3_errmsg(db));
        return NULL;
    }
    object *obj = makenative(db, NULL);
    obj->type = 13;
    return obj;
}

object *dbquery(object *conn, char *sql) {
    sqlite3 *db = (sqlite3*)conn->native.handle;
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        throwexception(sqlite3_errmsg(db));
        return makelist();
    }
    object *results = makelist();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        object *row = makedict();
        int cols = sqlite3_column_count(stmt);
        for (int i = 0; i < cols; i++) {
            char *name = (char*)sqlite3_column_name(stmt, i);
            char *value = (char*)sqlite3_column_text(stmt, i);
            dictset(row, makestring(name), makestring(value ? value : ""));
        }
        listappend(results, row);
    }
    sqlite3_finalize(stmt);
    return results;
}

object *dbexecute(object *conn, char *sql) {
    sqlite3 *db = (sqlite3*)conn->native.handle;
    char *errmsg;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        char err[256];
        snprintf(err, sizeof(err), "sqlite error: %s", errmsg);
        sqlite3_free(errmsg);
        throwexception(err);
        return makeint(0);
    }
    return makeint(sqlite3_changes(db));
}

object *builtindbconnect(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("db.connect needs path");
    return dbconnect(args[0]->sval);
}

object *builtindbquery(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("db.query needs connection and sql");
    return dbquery(args[0], args[1]->sval);
}

object *builtindbexecute(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("db.execute needs connection and sql");
    return dbexecute(args[0], args[1]->sval);
}

void registerdblib(environment *env) {
    object *dbmod = makemodule("db", NULL);
    envset(dbmod->module.exports, "connect", makebuiltin(builtindbconnect), 0);
    envset(dbmod->module.exports, "query", makebuiltin(builtindbquery), 0);
    envset(dbmod->module.exports, "execute", makebuiltin(builtindbexecute), 0);
    envset(env, "db", dbmod, 0);
}
