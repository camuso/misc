/*****************************************************
** SQLite utilities and wrappers
******************************************************/
// if this were c++, db and table would be private members
// of a sql class, accessible only by member function calls.

static struct sqlite3 *db = NULL;
static sqlite3_stmt *stmt_row;
static sqlite3_stmt *stmt_decl;
static sqlite3_stmt *stmt_parentdecl;
static char *table;

inline struct sqlite3 *sql_get_db()
{
	return db;
}

inline void sql_set_table(const char *tablename)
{
	table = (char *)tablename;
}

inline const char *sql_get_table()
{
	return (const char *)table;
}

bool sql_exec(const char *stmt)
{
	char *errmsg;
	int ret = sqlite3_exec(db, stmt, 0, 0, &errmsg);

	if (ret != SQLITE_OK) {
		fprintf(stderr,
			"\nsql_exec: Error in statement: %s [%s].\n",
			stmt, errmsg);
		sqlite3_free(errmsg);
		return false;
	}
	sqlite3_free(errmsg);
	return true;
}

int sql_process_field(void *output, int argc, char **argv, char **colnames)
{
	strcpy(output, argv[0]);
	return 0;
}

bool sql_extract_field(char *field, void *id, char *output)
{
	char *errmsg;
	char *zsql = sqlite3_mprintf("select %q from %q where id==%d",
				     field, sql_get_table(), id);

	int retval = sqlite3_exec
			(db, zsql, sql_process_field, (void *)output, &errmsg);

	if (retval != SQLITE_OK) {
		fprintf(stderr,
			"\nError in statement: %s [%s].\n", zsql, errmsg);
		sqlite3_free(errmsg);
		return false;
	}
	return true;
}

bool sql_prepare_update_field
		(sqlite3_stmt **stmt, char *field, char *val)
{
	char *zsql;
	zsql = sqlite3_mprintf("update %q set %q=%q where id==:id",
			       sql_get_table(), field, val);
	int retval = sqlite3_prepare_v2(db, zsql, strlen(zsql), stmt, 0);
	if (retval != SQLITE_OK)
		fprintf(stderr, "Could not prepare update requested field. "
			"[%s]\n", sqlite3_errstr(retval));
	sqlite3_free(zsql);
	return retval == SQLITE_OK;
}

bool sql_step(sqlite3_stmt *stmt)
{
	int retval = sqlite3_step(stmt);

	if(retval != SQLITE_DONE) {
		fprintf (stderr, "Could not step statement. [%s]\n",
			 sqlite3_errstr(retval));
		return false;
	}
	return true;
}

inline void sql_reset(sqlite3_stmt *stmt)
{
	sqlite3_reset(stmt);
}

inline bool sql_finalize(sqlite3_stmt *stmt)
{
	return sqlite3_finalize(stmt) == SQLITE_OK;
}

// sql_init, if successful, initializes the global sqlite3 *db
// Send table schema as a string of comma separated fieldnames paired
// with their data type, e.g.
// "field1 txt,field2 integer"
bool sql_init(const char *sqlfilename,
	      const char *table_schema,
	      const char *table_name)
{
	char *zsql;
	int rval;

	rval = sqlite3_open(sqlfilename, &db);

	if (rval != SQLITE_OK) {
		fprintf(stderr, "Unable to create database: %s\n", sqlfilename);
		return false;
	}

	fprintf(stderr, "Created new database file: %s\n", sqlfilename);

	zsql = sqlite3_mprintf("create table %q(%q)",
			       table_name, table_schema);
	if (!sql_exec(zsql))
		return false;

	sql_set_table(table_name);
	fprintf(stderr, "Created new table: %s\n", table_name);
	fprintf(stderr, "Schema: %s\n", table_schema);
	sqlite3_free(zsql);
	rval = sql_exec("pragma synchronous = off;"
			"pragma journal_mode = off;"
			"begin transaction;");
	return rval;
}

bool sql_open(const char *sqlfilename, const char *tablename)
{
	int rval = sqlite3_open_v2
			(sqlfilename, &db, SQLITE_OPEN_READWRITE, NULL);

	if (rval != SQLITE_OK) {
		fprintf(stderr, "sqlite3_open_v2() returned %s\n",
			sqlite3_errstr(rval));
		return false;
	}
	sql_set_table(tablename);
	rval = sql_exec("begin transaction");
	return rval;
}

inline void sql_close(sqlite3 *db)
{
	sqlite3_close(db);
}

static void sql_prepare_kabi_stmts()
{
	char *zsql;
	int len;
	int retval;
	zsql = sqlite3_mprintf
			("insert into %q (id,parentid,level,flags,prefix) "
			 "values (:id,:parentid,:level,:flags,:prefix)",
			 sql_get_table());
	len = strlen(zsql);
	retval = sqlite3_prepare_v2(db, zsql, len, &stmt_row, 0);

	if (retval != SQLITE_OK)
		fprintf(stderr, "Could not prepare stmt_row. "
			"[%s]\n", sqlite3_errstr(retval));

	sqlite3_free(zsql);
	sql_prepare_update_field(&stmt_decl, "decl", ":decl");
	sql_prepare_update_field(&stmt_parentdecl, "parentdecl", ":parentdecl");
}

static void sql_finalize_kabi_stmts()
{
	sql_finalize(stmt_row);
	sql_finalize(stmt_decl);
	sql_finalize(stmt_parentdecl);
	sql_exec("commit transaction");
}

