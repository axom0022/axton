#include "../core/axton.h"
#include <sqlite3.h>

typedef struct ormmodel {
    char *tablename;
    object *fields;
    sqlite3 *db;
} ormmodel;

object *ormconnect(char *path) {
    sqlite3 *db;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        throwexception(sqlite3_errmsg(db));
        return NULL;
    }
    return makenative(db, NULL);
}

object *ormmodel(char *tablename, object *fields) {
    object *model = makedict();
    dictset(model, makestring("_tablename"), makestring(tablename));
    dictset(model, makestring("_fields"), fields);
    return model;
}

object *ormcreateall(object *dbconn, object *models) {
    sqlite3 *db = (sqlite3*)dbconn->native.handle;
    for (int i = 0; i < models->list.count; i++) {
        object *model = models->list.items[i];
        char *tablename = dictget(model, makestring("_tablename"))->sval;
        object *fields = dictget(model, makestring("_fields"));
        char sql[1024];
        snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS %s (id INTEGER PRIMARY KEY AUTOINCREMENT", tablename);
        for (int j = 0; j < fields->list.count; j++) {
            object *field = fields->list.items[j];
            char *fieldname = field->sval;
            char *fieldtype = "TEXT";
            if (j < fields->list.count - 1) fieldtype = "TEXT";
            snprintf(sql + strlen(sql), sizeof(sql) - strlen(sql), ", %s %s", fieldname, fieldtype);
        }
        snprintf(sql + strlen(sql), sizeof(sql) - strlen(sql), ")");
        char *errmsg;
        if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
            throwexception(errmsg);
            sqlite3_free(errmsg);
        }
    }
    return makenone();
}

object *orminsert(object *dbconn, object *model, object *data) {
    sqlite3 *db = (sqlite3*)dbconn->native.handle;
    char *tablename = dictget(model, makestring("_tablename"))->sval;
    object *fields = dictget(model, makestring("_fields"));
    char sql[2048];
    snprintf(sql, sizeof(sql), "INSERT INTO %s (", tablename);
    char values[1024] = "";
    for (int i = 0; i < fields->list.count; i++) {
        char *fieldname = fields->list.items[i]->sval;
        object *val = dictget(data, fields->list.items[i]);
        if (i > 0) {
            strcat(sql, ",");
            strcat(values, ",");
        }
        strcat(sql, fieldname);
        char *valstr = val ? (val->type == 2 ? val->sval : "NULL") : "NULL";
        char escaped[512];
        int vi = 0;
        for (int j = 0; valstr[j]; j++) {
            if (valstr[j] == '\'') escaped[vi++] = '\'';
            escaped[vi++] = valstr[j];
        }
        escaped[vi] = 0;
        snprintf(values + strlen(values), sizeof(values) - strlen(values), "'%s'", escaped);
    }
    snprintf(sql + strlen(sql), sizeof(sql) - strlen(sql), ") VALUES (%s)", values);
    char *errmsg;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        throwexception(errmsg);
        sqlite3_free(errmsg);
        return makeint(-1);
    }
    return makeint(sqlite3_last_insert_rowid(db));
}

object *ormselect(object *dbconn, object *model, object *where) {
    sqlite3 *db = (sqlite3*)dbconn->native.handle;
    char *tablename = dictget(model, makestring("_tablename"))->sval;
    char sql[1024];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s", tablename);
    if (where && dictget(where, makestring("where"))) {
        strcat(sql, " WHERE ");
        strcat(sql, dictget(where, makestring("where"))->sval);
    }
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

object *ormupdate(object *dbconn, object *model, object *data, object *where) {
    sqlite3 *db = (sqlite3*)dbconn->native.handle;
    char *tablename = dictget(model, makestring("_tablename"))->sval;
    char sql[2048];
    snprintf(sql, sizeof(sql), "UPDATE %s SET ", tablename);
    object *fields = dictget(model, makestring("_fields"));
    for (int i = 0; i < fields->list.count; i++) {
        char *fieldname = fields->list.items[i]->sval;
        object *val = dictget(data, fields->list.items[i]);
        if (i > 0) strcat(sql, ",");
        char *valstr = val ? (val->type == 2 ? val->sval : "NULL") : "NULL";
        char escaped[512];
        int vi = 0;
        for (int j = 0; valstr[j]; j++) {
            if (valstr[j] == '\'') escaped[vi++] = '\'';
            escaped[vi++] = valstr[j];
        }
        escaped[vi] = 0;
        snprintf(sql + strlen(sql), sizeof(sql) - strlen(sql), "%s='%s'", fieldname, escaped);
    }
    if (where && dictget(where, makestring("where"))) {
        strcat(sql, " WHERE ");
        strcat(sql, dictget(where, makestring("where"))->sval);
    }
    char *errmsg;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        throwexception(errmsg);
        sqlite3_free(errmsg);
        return makeint(0);
    }
    return makeint(sqlite3_changes(db));
}

object *ormdelete(object *dbconn, object *model, object *where) {
    sqlite3 *db = (sqlite3*)dbconn->native.handle;
    char *tablename = dictget(model, makestring("_tablename"))->sval;
    char sql[1024];
    snprintf(sql, sizeof(sql), "DELETE FROM %s", tablename);
    if (where && dictget(where, makestring("where"))) {
        strcat(sql, " WHERE ");
        strcat(sql, dictget(where, makestring("where"))->sval);
    }
    char *errmsg;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        throwexception(errmsg);
        sqlite3_free(errmsg);
        return makeint(0);
    }
    return makeint(sqlite3_changes(db));
}

void registerormlib(environment *env) {
    object *ormmod = makemodule("orm", NULL);
    envset(ormmod->module.exports, "connect", makebuiltin(ormconnect), 0);
    envset(ormmod->module.exports, "model", makebuiltin(ormmodel), 0);
    envset(ormmod->module.exports, "createall", makebuiltin(ormcreateall), 0);
    envset(ormmod->module.exports, "insert", makebuiltin(orminsert), 0);
    envset(ormmod->module.exports, "select", makebuiltin(ormselect), 0);
    envset(ormmod->module.exports, "update", makebuiltin(ormupdate), 0);
    envset(ormmod->module.exports, "delete", makebuiltin(ormdelete), 0);
    envset(env, "orm", ormmod, 0);
}
