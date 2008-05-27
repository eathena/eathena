// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/timer.h"
#include "../common/lock.h"
#include "../common/db.h"
#include "../common/mmo.h"
#include "account.h"

#include <openrj/openrj.h> // Open-RJ - http://openrj.sourceforge.net/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ACCOUNT_RJ_DB_FILENAME				"save/accounts.rj"
#define ACCOUNT_RJ_DB_CREATE				1
#define ACCOUNT_RJ_DB_CASE_SENSITIVE		1
#define ACCOUNT_RJ_DB_TYPE					"Accounts"
#define ACCOUNT_RJ_DB_VERSION				20080420
#define ACCOUNT_RJ_DB_SEPARATOR				"%%"
#define ACCOUNT_RJ_SAVE_RETRY_DELAY			5000
#define ACCOUNT_RJ_SAVE_CHANGE_DELAY		1000
#define ACCOUNT_RJ_SAVE_MAX_DELAY			10000
#define ACCOUNT_RJ_FIELD_DATABASE			"Database"
#define ACCOUNT_RJ_FIELD_VERSION			"Version"
#define ACCOUNT_RJ_FIELD_NEXT_ID			"NextId"
#define ACCOUNT_RJ_FIELD_ACCOUNT_ID			"AccountId"
#define ACCOUNT_RJ_FIELD_USERNAME			"Username"
#define ACCOUNT_RJ_FIELD_PASSWORD			"Password"
#define ACCOUNT_RJ_FIELD_SEX				"Sex"
#define ACCOUNT_RJ_FIELD_EMAIL				"Email"
#define ACCOUNT_RJ_FIELD_GM_LEVEL			"GmLevel"
#define ACCOUNT_RJ_FIELD_STATE				"State"
#define ACCOUNT_RJ_FIELD_UNBAN_TIME			"UnbanTime"
#define ACCOUNT_RJ_FIELD_EXPIRATION_TIME	"ExpirationTime"
#define ACCOUNT_RJ_FIELD_LOGIN_COUNT		"LoginCount"
#define ACCOUNT_RJ_FIELD_LAST_LOGIN			"LastLogin"
#define ACCOUNT_RJ_FIELD_LAST_IP			"LastIp"
#define ACCOUNT_RJ_FIELD_VARIABLE			"Variable"
#define ACCOUNT_RJ_TAG						"account.rj."
#define ACCOUNT_RJ_TAG_DB_FILENAME			"db.filename"
#define ACCOUNT_RJ_TAG_DB_CREATE			"db.create"
#define ACCOUNT_RJ_TAG_DB_CASE_SENSITIVE	"db.case_sensitive"
#define ACCOUNT_RJ_TAG_SAVE_RETRY_DELAY		"save.retry_delay"
#define ACCOUNT_RJ_TAG_SAVE_CHANGE_DELAY	"save.change_delay"
#define ACCOUNT_RJ_TAG_SAVE_MAX_DELAY		"save.max_delay"


/// Database using Open-RJ
typedef struct AccountDB_RJ
{
	AccountDB vtable;
	int uid;
	// settings
	char db_filename[256];	// database filename
	bool db_create;			// create the database if not found?
	bool db_case_sensitive;	// case-sensitive usernames?
	int save_retry_delay;	// save delay when retrying
	int save_change_delay;	// save delay when a changing the database
	int save_max_delay;		// maximum cumulative save delay
	// data
	DBMap* idx_username;	// const char* username -> struct mmo_account* (username index)
	DBMap* accounts;		// int account_id -> struct mmo_account*
	int next_id;
	int save_timer;
	int save_delay;
} AccountDB_RJ;


// forward declarations
static int accdb_rj_save_timer(int tid, unsigned int tick, int id, int data);



///////////////////////////////////////////////////////////////////////////////
// private



// reference to all allocated databases
static int g_accdb_rj_uid = 0;
static struct linkdb_node* g_accdb_rj_list = NULL;// int uid -> AccountDB_RJ*


// use our memory manager
static void* accdb_rj_malloc(IORJAllocator* aloc, size_t sz)			{ return aMalloc(sz);    }
static void* accdb_rj_realloc(IORJAllocator* aloc, void *p, size_t sz)	{ return aRealloc(p,sz); }
static void  accdb_rj_free(IORJAllocator* aloc, void *p)				{ aFree(p);              }
static IORJAllocator g_accdb_rjAllocator =
{
	&accdb_rj_malloc,
	&accdb_rj_realloc,
	&accdb_rj_free
};


static int accdb_rj_destroy_mmo_account(DBKey key, void* data, va_list args)
{
	struct mmo_account* acc = (struct mmo_account*)data;
	aFree(acc);
	return 0;
}


/// Checks if a name represents an account variable and puts the type in isString.
/// @return true if it's a valid account variable name
/// @private
static bool accdb_rj_getVariableType(const char* name, bool* isString)
{
	*isString = false;
	if( name[0] != '#' || name[1] != '#' )
		return false;// invalid type
	name += 2;
	while( ISALNUM(*name) || *name == '_' ) ++name;
	if( *name == '$' )
	{
		*isString = true;
		++name;
	}
	return (*name == '\0');// is name complete?
}


/// Appends a character field to the buffer.
/// @private
static void accdb_rj_appendCharField(StringBuf* buf, const char* name, char value)
{
	StringBuf_Printf(buf, "%s: %c\n", name, value);
}


/// Appends a long field to the buffer.
/// @private
static void accdb_rj_appendLongField(StringBuf* buf, const char* name, long value)
{
	StringBuf_Printf(buf, "%s: %ld\n", name, value);
}


/// Appends a string field to the buffer.
/// The value is escaped and possibly enclosed in double-quotes.
/// @private
static void accdb_rj_appendStringField(StringBuf* buf, const char* name, const char* value)
{
	char tmp[4096];
	char* p;
	size_t len;
	size_t n;

	p = tmp;
	len = strlen(value);
	if( sizeof(tmp) < len*4+1 )
		p = aMalloc(len*4+1);
	n = sv_escape_c(p, value, len, "\"");
	if( n != len || ISSPACE(p[0]) || ISSPACE(p[n-1]) )
		StringBuf_Printf(buf, "%s: \"%s\"\n", name, p);// needs double-quotes
	else
		StringBuf_Printf(buf, "%s: %s\n", name, p);
	if( p != tmp )
		aFree(p);
}


/// Appends a variable field to the buffer.
/// The field name follow the format: <name>.<variable name>
/// @private
static bool accdb_rj_appendVariableField(StringBuf* buf, const char* name, struct global_reg* reg)
{
	bool isString = false;

	// check name
	if( !accdb_rj_getVariableType(reg->str, &isString) )
		return false;// invalid name
	// append variable
	if( isString )
	{
		if( reg->value[0] == '\0' ) return false;// default value "" (shouldn't happen)

		StringBuf_AppendStr(buf, name);
		StringBuf_AppendStr(buf, ".");
		accdb_rj_appendStringField(buf, reg->str, reg->value);
	}
	else
	{
		long value = atol(reg->value);

		if( value == 0 ) return false;// default value 0 (shouldn't happen)

		StringBuf_AppendStr(buf, name);
		StringBuf_AppendStr(buf, ".");
		accdb_rj_appendLongField(buf, reg->str, value);
	}
	return true;
}


/// Writes the current database to file.
/// @private
static bool accdb_rj_writeToFile(AccountDB_RJ* db)
{
	StringBuf buf;
	FILE* fp;
	DBIterator* iter;
	struct mmo_account* acc;
	int lock;

	// generate contents
	StringBuf_Init(&buf);
	accdb_rj_appendStringField(&buf, ACCOUNT_RJ_FIELD_DATABASE, ACCOUNT_RJ_DB_TYPE);
	accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_VERSION, ACCOUNT_RJ_DB_VERSION);
	accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_NEXT_ID, db->next_id);
	StringBuf_Printf(&buf, "%s\n", ACCOUNT_RJ_DB_SEPARATOR);
	iter = db_iterator(db->accounts);
	for( acc = (struct mmo_account*)dbi_first(iter); dbi_exists(iter); acc = (struct mmo_account*)dbi_next(iter) )
	{
		int i;

		accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_ACCOUNT_ID, acc->account_id);
		accdb_rj_appendCharField(&buf, ACCOUNT_RJ_FIELD_SEX, acc->sex);
		accdb_rj_appendStringField(&buf, ACCOUNT_RJ_FIELD_USERNAME, acc->userid);
		if( acc->pass[0] )
			accdb_rj_appendStringField(&buf, ACCOUNT_RJ_FIELD_PASSWORD, acc->pass);
		if( acc->email[0] )
			accdb_rj_appendStringField(&buf, ACCOUNT_RJ_FIELD_EMAIL, acc->email);
		if( acc->level )
			accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_GM_LEVEL, acc->level);
		if( acc->state )
			accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_STATE, (long)acc->state);
		if( acc->unban_time )
			accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_UNBAN_TIME, (long)acc->unban_time);
		if( acc->expiration_time )
			accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_EXPIRATION_TIME, (long)acc->expiration_time);
		if( acc->logincount )
			accdb_rj_appendLongField(&buf, ACCOUNT_RJ_FIELD_LOGIN_COUNT, acc->logincount);
		if( acc->lastlogin[0] )
			accdb_rj_appendStringField(&buf, ACCOUNT_RJ_FIELD_LAST_LOGIN, acc->lastlogin);
		if( acc->last_ip[0] )
			accdb_rj_appendStringField(&buf, ACCOUNT_RJ_FIELD_LAST_IP, acc->last_ip);
		for( i = 0; i < acc->account_reg2_num; ++i )
		{
			struct global_reg* reg = &(acc->account_reg2[i]);
			if( !accdb_rj_appendVariableField(&buf, ACCOUNT_RJ_FIELD_VARIABLE, reg) )
			{
				ShowWarning("account_db_rj_writeToFile: discarting invalid variable '%s' (value='%s', AID=%d)\n", reg->str, reg->value, acc->account_id);
				continue;
			}
		}
		StringBuf_Printf(&buf, "%s\n", ACCOUNT_RJ_DB_SEPARATOR);
	}
	dbi_destroy(iter);

	// write to file
	fp = lock_fopen(db->db_filename, &lock);
	if( fp == NULL )
	{
		ShowError("account_db_rj_readFromFile: failed to create '%s' (%s)\n", db->db_filename, strerror(errno));
		StringBuf_Destroy(&buf);
		return false;
	}
	fprintf(fp, "%s", StringBuf_Value(&buf));
	lock_fclose(fp, db->db_filename, &lock);
	StringBuf_Destroy(&buf);
	return true;
}


/// Reads a char field from the record.
/// @private
static bool accdb_rj_readCharField(const ORJRecordA* record, const char* name, char* out)
{
	const ORJFieldA* field;
	const ORJStringA* value;

	*out = '\0';
	field = ORJ_Record_FindFieldByNameA(record, name, NULL);
	if( field == NULL )
		return false;
	value = &(field->value);
	if( value->len > 0 )
		*out = value->ptr[0];
	return true;
}


/// Reads a long field from the record.
/// @private
static bool accdb_rj_readLongField(const ORJRecordA* record, const char* name, long* out)
{
	const ORJFieldA* field;
	const ORJStringA* value;

	*out = 0;
	field = ORJ_Record_FindFieldByNameA(record, name, NULL);
	if( field == NULL )
		return false;
	value = &(field->value);
	if( value->len > 0 )
		*out = atol(value->ptr);
	return true;
}


/// Unescapes a string and removes enclosing double-quotes.
/// @return true is it fit in the buffer
/// @private
static bool accdb_rj_unescapeString(const char* str, char* buf, size_t buflen)
{
	size_t len = strlen(str);

	if( buflen < len + 1 )
	{
		char* p = aMalloc(len + 1);
		size_t n = sv_unescape_c(p, str, len);
		if( n >= 2 && p[0] == '"' && p[n-1] == '"' )
		{// remove double quotes
			p[n-1] = '\0';
			memmove(p, p+1, n-1);
			n -= 2;
		}
		safestrncpy(buf, p, buflen);
		aFree(p);
		return n < buflen;// all ok? (not truncated)
	}
	else
	{
		size_t n = sv_unescape_c(buf, str, len);
		if( n >= 2 && buf[0] == '"' && buf[n-1] == '"' )
		{// remove double quotes
			buf[n-1] = '\0';
			memmove(buf, buf+1, n-1);
		}
		return true;
	}
}


/// Reads a string field from the record.
/// @private
static bool accdb_rj_readStringField(const ORJRecordA* record, const char* name, char* buf, size_t buflen)
{
	const ORJFieldA* field;
	const ORJStringA* value;

	memset(buf, 0, buflen);
	field = ORJ_Record_FindFieldByNameA(record, name, NULL);
	if( field == NULL )
		return false;
	value = &(field->value);
	if( value->len == 0 )
		return true;
	accdb_rj_unescapeString(value->ptr, buf, buflen);
	return true;
}


/// Checks if the field represents a variable.
/// @return true if the field represents a variable
/// @private
static bool accdb_rj_isVariableField(const ORJFieldA* field)
{
	return (field->name.len > 0 && strstr(field->name.ptr, ACCOUNT_RJ_FIELD_VARIABLE) == field->name.ptr && field->name.ptr[strlen(ACCOUNT_RJ_FIELD_VARIABLE)] == '.');
}


/// Reads a variable field.
/// @private
static bool accdb_rj_readVariableField(const ORJFieldA* field, struct global_reg* out)
{
	const char* p;
	size_t len;
	bool isString;

	memset(out, 0, sizeof(struct global_reg));
	// variable name
	p = field->name.ptr + strlen(ACCOUNT_RJ_FIELD_VARIABLE) + 1;
	len = strlen(p);
	if( len >= sizeof(out->str) )
		return false;// too long
	safestrncpy(out->str, p, sizeof(out->str));
	if( !accdb_rj_getVariableType(out->str, &isString) )
		return false;// invalid
	// variable value
	return accdb_rj_unescapeString(field->value.ptr, out->value, sizeof(out->value));
}


/// Reads an account from the record.
/// @private
static bool accdb_rj_readAccount(AccountDB_RJ* db, const ORJRecordA* record, struct mmo_account* out)
{
	const ORJFieldA* field;
	long l;
	size_t i;

	// required fields
	memset(out, 0, sizeof(struct mmo_account));
	if( !accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_ACCOUNT_ID, &l) )
		return false;
	out->account_id = (int)l;
	if( !accdb_rj_readCharField(record, ACCOUNT_RJ_FIELD_SEX, &(out->sex)) )
		return false;
	if( !accdb_rj_readStringField(record, ACCOUNT_RJ_FIELD_USERNAME, out->userid, sizeof(out->userid)) )
		return false;
	if( out->userid[0] == '\0' || !(out->sex == 'F' || out->sex == 'M' || out->sex == 'S') || out->account_id < 0 || out->account_id > END_ACCOUNT_NUM )
		return false;
	// optional fields
	accdb_rj_readStringField(record, ACCOUNT_RJ_FIELD_PASSWORD, out->pass, sizeof(out->pass));
	accdb_rj_readStringField(record, ACCOUNT_RJ_FIELD_EMAIL, out->email, sizeof(out->email));
	accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_GM_LEVEL, &l); out->level = (int)l;
	accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_STATE, &l); out->state = (uint32)l;
	accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_UNBAN_TIME, &l); out->unban_time = (time_t)l;
	accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_EXPIRATION_TIME, &l); out->expiration_time = (time_t)l;
	accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_LOGIN_COUNT, &l); out->logincount = (int)l;
	accdb_rj_readStringField(record, ACCOUNT_RJ_FIELD_LAST_LOGIN, out->lastlogin, sizeof(out->lastlogin));
	accdb_rj_readStringField(record, ACCOUNT_RJ_FIELD_LAST_IP, out->last_ip, sizeof(out->last_ip));
	for( i = 0; i < record->numFields; ++i )
	{
		field = &(record->fields[i]);
		if( accdb_rj_isVariableField(field) )
		{
			struct global_reg* reg;
			if( out->account_reg2_num == ARRAYLENGTH(out->account_reg2) )
			{
				ShowWarning("accdb_rj_readAccount: discarting extra variable '%s' in '%s' (AID:%ld sex=%c userid='%s' value=%s)",
					field->name.ptr, db->db_filename, out->account_id, out->sex, out->userid, field->value.ptr);
				continue;
			}
			reg = &(out->account_reg2[out->account_reg2_num]);
			if( !accdb_rj_readVariableField(field, reg) )
			{
				ShowWarning("accdb_rj_readAccount: discarting invalid variable '%s' in '%s' (AID:%ld sex=%c userid='%s' value=%s)",
					field->name.ptr, db->db_filename, out->account_id, out->sex, out->userid, field->value.ptr);
				continue;
			}
			++(out->account_reg2_num);
		}
	}
	return true;
}


/// Reads the database from file.
/// @private
static bool accdb_rj_readFromFile(AccountDB_RJ* db)
{
	const ORJDatabaseA* rjDb;
	unsigned int rjFlags = ORJ_FLAG_ELIDEBLANKRECORDS|ORJ_FLAG_NOREINTERPRETFIELDIDS;
	ORJError rjError;
	ORJRC rjRet;
	bool result = false;

	db->accounts->clear(db->accounts, accdb_rj_destroy_mmo_account);
	rjRet = ORJ_ReadDatabaseA(db->db_filename, &g_accdb_rjAllocator, rjFlags, &rjDb, &rjError);
	if( rjRet == ORJ_RC_CANNOTOPENJARFILE && db->db_create )
	{// try to create file
		ShowStatus("account_db_rj_readFromFile: '%s' not found, creating...\n", db->db_filename);
		if( !accdb_rj_writeToFile(db) )
			return false;
		return true;
	}
	if( rjRet == ORJ_RC_PARSEERROR )
	{
		ShowError("account_db_rj_readFromFile: failed to read '%s': %s (at line %ld, column %ld, %s)\n",
			db->db_filename, ORJ_GetErrorStringA(rjRet), (long)rjError.invalidLine, (long)rjError.invalidColumn, ORJ_GetParseErrorStringA(rjError.parseError));
		return false;
	}
	if( rjRet != ORJ_RC_SUCCESS )
	{
		ShowError("account_db_rj_readFromFile: failed to read '%s': %s\n",
			db->db_filename, ORJ_GetErrorStringA(rjRet));
		return false;
	}
	// read database
	do{
		char db_type[256];
		long db_version;
		long next_id;
		size_t i;
		const ORJRecordA* record;

		// header
		if( ORJ_RC_SUCCESS != ORJ_Database_GetRecordA(rjDb, 0, &record) )
		{
			ShowError("account_db_rj_readFromFile: missing header record in '%s'\n", db->db_filename);
			break;
		}
		// database type
		if( !accdb_rj_readStringField(record, ACCOUNT_RJ_FIELD_DATABASE, db_type, sizeof(db_type)) )
		{
			ShowError("account_db_rj_readFromFile: missing database type in '%s'\n", db->db_filename);
			break;
		}
		if( strcmp(db_type, ACCOUNT_RJ_DB_TYPE) != 0 )
		{
			ShowError("account_db_rj_readFromFile: invalid database type '%s' in '%s' (expected '%s')\n", db_type, db->db_filename, ACCOUNT_RJ_DB_TYPE);
			break;
		}
		// database version
		if( !accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_VERSION, &db_version) )
		{
			ShowError("account_db_rj_readFromFile: missing database version in '%s'\n", db->db_filename);
			break;
		}
		if( db_version != ACCOUNT_RJ_DB_VERSION )
		{
			ShowError("account_db_rj_readFromFile: unsupported version %ld in '%s'\n", db_version, db->db_filename);
			break;
		}
		// next id
		next_id = START_ACCOUNT_NUM;
		if( accdb_rj_readLongField(record, ACCOUNT_RJ_FIELD_NEXT_ID, &next_id) )
		{
			if( next_id < START_ACCOUNT_NUM || next_id > END_ACCOUNT_NUM )
			{
				ShowWarning("account_db_rj_readFromFile: invalid next_id %ld in '%s'. Defaulting to %ld.\n", next_id, db->db_filename, (long)START_ACCOUNT_NUM);
				next_id = START_ACCOUNT_NUM;
			}
		}
		db->next_id = (int)next_id;

		// accounts
		result = true;
		for( i = 1; i < rjDb->numRecords; ++i )
		{
			struct mmo_account acc_buf;
			record = &(rjDb->records[i]);
			if( !accdb_rj_readAccount(db, record, &acc_buf) )
			{
				const ORJFieldA* field;
				size_t j;
				ShowError("account_db_rj_readFromFile: discarting invalid account in '%s'\n", db->db_filename);
				for( j = 0; j < record->numFields; ++j )
				{
					field = &(record->fields[j]);
					ShowDebug("account_db_rj_readFromFile: %s: %s\n", field->name.ptr, field->value.ptr);
				}
			}
			else
			{
				struct mmo_account* acc;
				CREATE(acc, struct mmo_account, 1);
				memcpy(acc, &acc_buf, sizeof(struct mmo_account));
				acc = (struct mmo_account*)idb_put(db->accounts, acc->account_id, acc);
				if( acc )
				{
					ShowError("account_db_rj_readFromFile: duplicate account AID:%d (userid='%s')\n", acc->account_id, acc->userid);
					aFree(acc);
					result = false;
					break;
				}
			}
		}
	}while( false );
	ORJ_FreeDatabaseA(rjDb);
	return result;
}


/// Schedules a save operation with the specified delay.
/// If already scheduled, adds more delay to it.
/// @private
static void accdb_rj_scheduleSave(AccountDB_RJ* db, int delay)
{
	static bool registered = false;
	if( !registered )
	{
		add_timer_func_list(accdb_rj_save_timer, "accdb_rj_save_timer");
		registered = true;
	}

	if( db->save_timer == INVALID_TIMER )
	{
		if( delay > db->save_max_delay )
			delay = db->save_max_delay;
		db->save_delay = delay;
		db->save_timer = add_timer(gettick() + delay, accdb_rj_save_timer, db->uid, 0);
	}
	else
	{
		if( delay + db->save_delay > db->save_max_delay )
			delay = db->save_max_delay - db->save_delay;
		if( delay )
		{
			db->save_delay += delay;
			addtick_timer(db->save_timer, delay);
		}
	}
}


/// Timer function. 
/// Triggers a save to file.
/// Reschedules the save if the operation failed.
/// @private
static int accdb_rj_save_timer(int tid, unsigned int tick, int id, int data)
{
	AccountDB_RJ* db = (AccountDB_RJ*)linkdb_search(&g_accdb_rj_list, (void*)id);
	if( db )
	{
		db->save_timer = INVALID_TIMER;
		if( !accdb_rj_writeToFile(db) )
			accdb_rj_scheduleSave(db, db->save_retry_delay);
	}
	return 0;
}


/// Creates the username index.
/// @private
static bool accdb_rj_createIndexUsername(AccountDB_RJ* db)
{
	DBIterator* iter;
	struct mmo_account* acc;
	bool result = true;

	if( db->accounts == NULL )
		return false;// not initialized yet

	// destroy old index
	if( db->idx_username )
	{
		db_destroy(db->idx_username);
		db->idx_username = NULL;
	}
	// generate index
	if( db->db_case_sensitive )
		db->idx_username = strdb_alloc(DB_OPT_BASE, 0);
	else
		db->idx_username = stridb_alloc(DB_OPT_BASE, 0);
	iter = db_iterator(db->accounts);
	for( acc = (struct mmo_account*)dbi_first(iter); dbi_exists(iter); acc = (struct mmo_account*)dbi_next(iter) )
	{
		struct mmo_account* tmp = (struct mmo_account*)strdb_put(db->idx_username, acc->userid, acc);
		if( tmp )
		{
			ShowError("accdb_rj_createIndexUsername: duplicate username AID=%d username='%s', AID=%d username='%s' (case_sensitive=%d)\n",
				acc->account_id, acc->userid, tmp->account_id, tmp->userid, db->db_case_sensitive);
			result = false;
		}
	}
	dbi_destroy(iter);
	if( !result )
	{// error, clear index
		db_destroy(db->idx_username);
		db->idx_username = NULL;
	}
	return result;
}


/// Discards old data.
/// @private
static void accdb_rj_discard(AccountDB_RJ* db)
{
	if( db->save_timer != INVALID_TIMER )
	{
		delete_timer(db->save_timer, accdb_rj_save_timer);
		db->save_timer = INVALID_TIMER;
	}
	if( db->idx_username )
	{
		db_destroy(db->idx_username);
		db->idx_username = NULL;
	}
	if( db->accounts )
	{
		db->accounts->destroy(db->accounts, accdb_rj_destroy_mmo_account);
		db->accounts = NULL;
	}
}



///////////////////////////////////////////////////////////////////////////////
// protected (API)



/// Initializes this database, making it ready for use.
/// @protected
static bool account_db_rj_init(AccountDB* self)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;

	// discard old data
	accdb_rj_discard(db);
	// prepare new data
	db->accounts = idb_alloc(DB_OPT_BASE);
	db->next_id = START_ACCOUNT_NUM;
	if( !accdb_rj_readFromFile(db) || !accdb_rj_createIndexUsername(db) )
	{
		accdb_rj_discard(db);
		return false;
	}
	return true;
}


/// Destroys this database, releasing all allocated memory (including itself).
/// @protected
static void account_db_rj_destroy(AccountDB* self)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;

	if( db->save_timer != INVALID_TIMER )
		accdb_rj_writeToFile(db);// try to save pending data
	accdb_rj_discard(db);
	linkdb_erase(&g_accdb_rj_list, (void*)db->uid);
	aFree(db);
}


/// Gets a property from this database.
/// These read-only properties must be implemented:
/// "engine.name" -> "txt", "sql", ...
/// "engine.version" -> internal version
/// "engine.comment" -> anything (suggestion: description or specs of the engine)
/// @protected
bool account_db_rj_get_property(AccountDB* self, const char* key, char* buf, size_t buflen)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;

	if( strcmp(key, "engine.name") == 0 )
	{
		safesnprintf(buf, buflen, "rj");
		return true;
	}
	if( strcmp(key, "engine.version") == 0 )
	{
		safesnprintf(buf, buflen, "%d", ACCOUNT_RJ_DB_VERSION);
		return true;
	}
	if( strcmp(key, "engine.comment") == 0 )
	{
		safesnprintf(buf, buflen, "RJ Account Database %d (Open-RJ %d.%d.%d)", ACCOUNT_RJ_DB_VERSION, OPENRJ_VER_MAJOR, OPENRJ_VER_MINOR, OPENRJ_VER_REVISION);
		return true;
	}

	if( strstr(key, ACCOUNT_RJ_TAG) != key )
		return false;// not tag present

	key += strlen(ACCOUNT_RJ_TAG);
	if( strcmp(key, ACCOUNT_RJ_TAG_DB_FILENAME) == 0 )
		safesnprintf(buf, buflen, "%s", db->db_filename);
	else if( strcmp(key, ACCOUNT_RJ_TAG_DB_CREATE) == 0 )
		safesnprintf(buf, buflen, "%d", (db->db_create ? 1 : 0));
	else if( strcmp(key, ACCOUNT_RJ_TAG_DB_CASE_SENSITIVE) == 0 )
		safesnprintf(buf, buflen, "%d", (db->db_case_sensitive ? 1 : 0));
	else if( strcmp(key, ACCOUNT_RJ_TAG_SAVE_RETRY_DELAY) == 0 )
		safesnprintf(buf, buflen, "%d", db->save_retry_delay);
	else if( strcmp(key, ACCOUNT_RJ_TAG_SAVE_CHANGE_DELAY) == 0 )
		safesnprintf(buf, buflen, "%d", db->save_change_delay);
	else if( strcmp(key, ACCOUNT_RJ_TAG_SAVE_MAX_DELAY) == 0 )
		safesnprintf(buf, buflen, "%d", db->save_max_delay);
	else
		return false;
	return true;
}


/// Sets a property in this database.
/// @protected
static bool account_db_rj_set_property(AccountDB* self, const char* key, const char* value)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;

	if( strstr(key, ACCOUNT_RJ_TAG) != key )
		return false;// not tag present

	key += strlen(ACCOUNT_RJ_TAG);
	if( strcmp(key, ACCOUNT_RJ_TAG_DB_FILENAME) == 0 )
		safestrncpy(db->db_filename, value, sizeof(db->db_filename));
	else if( strcmp(key, ACCOUNT_RJ_TAG_DB_CREATE) == 0 )
		db->db_create = (atoi(value) != 0);
	else if( strcmp(key, ACCOUNT_RJ_TAG_DB_CASE_SENSITIVE) == 0 )
	{
		db->db_case_sensitive = (atoi(value) != 0);
		accdb_rj_createIndexUsername(db);
	}
	else if( strcmp(key, ACCOUNT_RJ_TAG_SAVE_RETRY_DELAY) == 0 )
		db->save_retry_delay = atoi(value);
	else if( strcmp(key, ACCOUNT_RJ_TAG_SAVE_CHANGE_DELAY) == 0 )
		db->save_change_delay = atoi(value);
	else if( strcmp(key, ACCOUNT_RJ_TAG_SAVE_MAX_DELAY) == 0 )
		db->save_max_delay = atoi(value);
	else
		return false;
	return true;
}


/// Creates a new account in this database.
/// If acc->account_id is not -1, the provided value will be used.
/// Otherwise the account_id will be auto-generated and written to acc->account_id.
/// @protected
static bool account_db_rj_create(AccountDB* self, struct mmo_account* acc)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;
	int account_id;
	struct mmo_account* buf;

	if( db->idx_username == NULL && !accdb_rj_createIndexUsername(db) )
		return false;// no username index

	account_id = acc->account_id;
	if( account_id == -1 )
	{// generate account_id
		account_id = db->next_id;
		while( idb_get(db->accounts, account_id) && account_id <= END_ACCOUNT_NUM )
			++account_id;
		if( account_id > END_ACCOUNT_NUM )
		{
			account_id = START_ACCOUNT_NUM;
			while( idb_get(db->accounts, account_id) && account_id < db->next_id )
				++account_id;
		}
	}
	if( idb_get(db->accounts, account_id) )
		return false;// already exists
	if( strdb_get(db->idx_username, acc->userid) )
		return false;// already exists
	db->next_id = account_id + 1;
	buf = (struct mmo_account*)aMalloc(sizeof(struct mmo_account));
	memcpy(buf, acc, sizeof(struct mmo_account));
	buf->account_id = account_id;
	idb_put(db->accounts, account_id, buf);
	strdb_put(db->idx_username, buf->userid, buf);
	accdb_rj_scheduleSave(db, db->save_change_delay);
	acc->account_id = account_id;
	return true;
}


/// Removes an account from this database.
/// @protected
static bool account_db_rj_remove(AccountDB* self, const int account_id)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;
	struct mmo_account* acc;

	if( db->accounts == NULL )
		return false;// not initialized

	acc = (struct mmo_account*)idb_remove(db->accounts, account_id);
	if( acc )
	{
		if( db->idx_username )
			strdb_remove(db->idx_username, acc->userid);
		aFree(acc);
		accdb_rj_scheduleSave(db, db->save_change_delay);
		return true;
	}
	return false;
}


/// Modifies the data of an existing account.
/// Uses acc->account_id to identify the account.
/// @protected
static bool account_db_rj_save(AccountDB* self, const struct mmo_account* acc)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;
	struct mmo_account* tmp;

	if( db->idx_username == NULL && !accdb_rj_createIndexUsername(db) )
		return false;// no username index

	tmp = (struct mmo_account*)idb_get(db->accounts, acc->account_id);
	if( tmp == NULL )
		return false;
	if( strdb_get(db->idx_username, acc->userid) != tmp )
	{// different username
		if( strdb_get(db->idx_username, acc->userid) )
			return false;// already taken
		strdb_remove(db->idx_username, tmp->userid);
		memcpy(tmp, acc, sizeof(struct mmo_account));
		strdb_put(db->idx_username, tmp->userid, tmp);
	}
	else
	{// same username
		memcpy(tmp, acc, sizeof(struct mmo_account));
	}
	accdb_rj_scheduleSave(db, db->save_change_delay);
	return true;
}


/// Finds an account with account_id and copies it to acc.
/// @protected
static bool account_db_rj_load_num(AccountDB* self, struct mmo_account* acc, const int account_id)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;
	struct mmo_account* tmp;

	if( db->accounts == NULL )
		return false;// not initialized

	tmp = (struct mmo_account*)idb_get(db->accounts, account_id);
	if( tmp == NULL )
		return false;
	memcpy(acc, tmp, sizeof(struct mmo_account));
	return true;
}


/// Finds an account with userid and copies it to acc.
/// @protected
static bool account_db_rj_load_str(AccountDB* self, struct mmo_account* acc, const char* userid)
{
	AccountDB_RJ* db = (AccountDB_RJ*)self;
	struct mmo_account* tmp;

	memset(acc, 0, sizeof(struct mmo_account));
	if( db->idx_username == NULL && !accdb_rj_createIndexUsername(db) )
		return false;// no username index

	tmp = (struct mmo_account*)strdb_get(db->idx_username, userid);
	if( tmp == NULL )
		return false;// not found
	memcpy(acc, tmp, sizeof(struct mmo_account));
	return true;
}



///////////////////////////////////////////////////////////////////////////////
// public



/// Public constructor.
///
/// @return New database
/// @public
AccountDB* account_db_rj(void)
{
	AccountDB_RJ* db = (AccountDB_RJ*)aCalloc(1, sizeof(AccountDB_RJ));

	db->vtable.init         = &account_db_rj_init;
	db->vtable.destroy      = &account_db_rj_destroy;
	db->vtable.get_property = &account_db_rj_get_property;
	db->vtable.set_property = &account_db_rj_set_property;
	db->vtable.save         = &account_db_rj_save;
	db->vtable.create       = &account_db_rj_create;
	db->vtable.remove       = &account_db_rj_remove;
	db->vtable.load_num     = &account_db_rj_load_num;
	db->vtable.load_str     = &account_db_rj_load_str;

	// settings
	safestrncpy(db->db_filename, ACCOUNT_RJ_DB_FILENAME, sizeof(db->db_filename));
#if ACCOUNT_RJ_DB_CREATE
	db->db_create = true;
#endif
#if ACCOUNT_RJ_DB_CASE_SENSITIVE
	db->db_case_sensitive = true;
#endif
	db->save_retry_delay = ACCOUNT_RJ_SAVE_RETRY_DELAY;
	db->save_change_delay = ACCOUNT_RJ_SAVE_CHANGE_DELAY;
	db->save_max_delay = ACCOUNT_RJ_SAVE_MAX_DELAY;

	// data
	db->save_timer = INVALID_TIMER;
	db->uid = ++g_accdb_rj_uid;// global reference number
	linkdb_insert(&g_accdb_rj_list, (void*)db->uid, (void*)db);
	return &db->vtable;
}
