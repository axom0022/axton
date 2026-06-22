#include "../core/axton.h"
#include <sqlite3.h>

object *dbconnect(object **a, int c, void *e) {
    if (c < 1) throwexception("connect needs path");
    sqlite3 *db;
    sqlite3_open(a[0]->sval, &db);
    return makenative(db, NULL);
}

object *dbquery(object **a, int c, void *e) {
    if (c < 2) throwexception("query needs conn and sql");
    sqlite3 *db = a[0]->native.handle;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, a[1]->sval, -1, &stmt, NULL);
    object *rows = makelist();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        object *row = makedict();
        int cols = sqlite3_column_count(stmt);
        for (int i = 0; i < cols; i++) {
            char *name = (char*)sqlite3_column_name(stmt, i);
            char *val = (char*)sqlite3_column_text(stmt, i);
            dictset(row, makestring(name), makestring(val ? val : ""));
        }
        listappend(rows, row);
    }
    sqlite3_finalize(stmt);
    return rows;
}

object *dbexecute(object **a, int c, void *e) {
    if (c < 2) throwexception("execute needs conn and sql");
    sqlite3 *db = a[0]->native.handle;
    char *err;
    sqlite3_exec(db, a[1]->sval, NULL, NULL, &err);
    return makeint(sqlite3_changes(db));
}

object *dbpostgres(object **a, int c, void *e) {
    return makenone();
}

object *dbmongodb(object **a, int c, void *e) {
    return makenone();
}

object *dbredis(object **a, int c, void *e) {
    return makenone();
}

void registerdblib(environment *env) {
    object *mod = makemodule("db", NULL);
    envset(mod->module.exports, "connect", makebuiltin(dbconnect), 0);
    envset(mod->module.exports, "query", makebuiltin(dbquery), 0);
    envset(mod->module.exports, "execute", makebuiltin(dbexecute), 0);
    envset(mod->module.exports, "postgres", makebuiltin(dbpostgres), 0);
    envset(mod->module.exports, "mongodb", makebuiltin(dbmongodb), 0);
    envset(mod->module.exports, "redis", makebuiltin(dbredis), 0);
    envset(env, "db", mod, 0);
}
