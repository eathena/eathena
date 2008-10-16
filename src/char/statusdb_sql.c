// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "inter.h" // sql_handle
#include "statusdb.h"
#include <stdlib.h>


/// internal structure
typedef struct StatusDB_SQL
{
	StatusDB vtable;    // public interface

	Sql* statuses;      // SQL status storage

	// other settings
	char status_db[32];

} StatusDB_SQL;

/// internal functions
static bool status_db_sql_init(StatusDB* self);
static void status_db_sql_destroy(StatusDB* self);
static bool status_db_sql_sync(StatusDB* self);
static bool status_db_sql_create(StatusDB* self, struct scdata* sc);
static bool status_db_sql_remove(StatusDB* self, const int char_id);
static bool status_db_sql_save(StatusDB* self, const struct scdata* sc);
static bool status_db_sql_load_num(StatusDB* self, struct scdata* sc, int char_id);

static bool mmo_status_fromsql(StatusDB_SQL* db, struct scdata* sc, int char_id);
static bool mmo_status_tosql(StatusDB_SQL* db, const struct scdata* sc);

/// public constructor
StatusDB* status_db_sql(void)
{
	StatusDB_SQL* db = (StatusDB_SQL*)aCalloc(1, sizeof(StatusDB_SQL));

	// set up the vtable
	db->vtable.init      = &status_db_sql_init;
	db->vtable.destroy   = &status_db_sql_destroy;
	db->vtable.sync      = &status_db_sql_sync;
	db->vtable.create    = &status_db_sql_create;
	db->vtable.remove    = &status_db_sql_remove;
	db->vtable.save      = &status_db_sql_save;
	db->vtable.load_num  = &status_db_sql_load_num;

	// initialize to default values
	db->statuses = NULL;
	// other settings
	safestrncpy(db->status_db, "sc_data", sizeof(db->status_db));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


static bool status_db_sql_init(StatusDB* self)
{
}

static void status_db_sql_destroy(StatusDB* self)
{
}

static bool status_db_sql_sync(StatusDB* self)
{
}

static bool status_db_sql_create(StatusDB* self, struct scdata* sc)
{
}

static bool status_db_sql_remove(StatusDB* self, const int char_id)
{
}

static bool status_db_sql_save(StatusDB* self, const struct scdata* sc)
{
}

static bool status_db_sql_load_num(StatusDB* self, struct scdata* sc, int char_id)
{
}


static bool mmo_status_fromsql(StatusDB_SQL* db, struct scdata* sc, int char_id)
{
}

static bool mmo_status_tosql(StatusDB_SQL* db, const struct scdata* sc)
{
}
