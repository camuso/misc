static bool sql_prepare_stmt(char *stmtstr, sqlite3_stmt **stmt)
{
	int len = strlen(stmtstr);
	int rval = sqlite3_prepare_v2(db, stmtstr, len, stmt, 0);
	if (rval != SQLITE_OK)
		fprintf(stderr, "Could not prepare %s\n[%s]\n",
			stmtstr, sqlite3_errstr(rval));
	return rval == SQLITE_OK;
}

static bool sql_prepare_kabi_stmts()
{
	char *zsql;
	bool rval;
	zsql = sqlite3_mprintf("select id,parentid,level from kabitree "
			       "where decl like \'%%?%%\'");
	puts(zsql);
	if ( !(rval = sql_prepare_stmt(zsql, &stmt_decl_search)))
		goto out;

	zsql = sqlite3_mprintf("select id,parentid,level from kabitree "
			       "where id==:id");
	if ( !(rval = sql_prepare_stmt(zsql, &stmt_id_search)))
		goto out;

	zsql = sqlite3_mprintf("select level,prefix,decl,parentdecl "
			       "from kabitree where id==:id");
	rval = sql_prepare_stmt(zsql, &stmt_get_row);
out:
	sqlite3_free(zsql);
	return rval;
}

static void sql_finalize_kabi_stmts()
{
	sql_finalize(stmt_decl_search);
	sql_finalize(stmt_id_search);
	sql_finalize(stmt_get_row);
}

