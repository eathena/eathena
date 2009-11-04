// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "charserverdb_txt.h"
#include "csdb_txt.h"
#include "guilddb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// global defines
#define GUILDDB_TXT_DB_VERSION 00000000
#define START_GUILD_NUM 1


/// Internal structure.
/// @private
typedef struct GuildDB_TXT
{
	// public interface
	GuildDB vtable;

	// data provider
	CSDB_TXT* db;

	// indexes
	DBMap* idx_name;// char* name -> int guild_id (case-sensitive)

} GuildDB_TXT;


/// Parses string containing serialized data into the provided data structure.
/// @protected
static bool guild_db_txt_fromstr(const char* str, int* key, void* data, size_t size, size_t* out_size, unsigned int version)
{
	struct guild* g = (struct guild*)data;

	*out_size = sizeof(*g);

	if( size < sizeof(*g) )
		return true;

	memset(g, 0, sizeof(*g));

	{// load guild base info
		int guildid;
		char name[256]; // only 24 used
		char master[256]; // only 24 used
		int guildlv;
		int max_member;
		unsigned int exp;
		int skpoint;
		char mes1[256]; // only 60 used
		char mes2[256]; // only 120 used
		int len;

		if( sscanf(str, "%d\t%[^\t]\t%[^\t]\t%d,%d,%u,%d,%*d\t%[^\t]\t%[^\t]\t%n",
				   &guildid, name, master, &guildlv, &max_member, &exp, &skpoint, mes1, mes2, &len) < 9 )
			return false;

		// remove '#'
		mes1[strlen(mes1)-1] = '\0';
		mes2[strlen(mes2)-1] = '\0';

		g->guild_id = guildid;
		g->guild_lv = guildlv;
		g->max_member = max_member;
		g->exp = exp;
		g->skill_point = skpoint;
		safestrncpy(g->name, name, sizeof(g->name));
		safestrncpy(g->master, master, sizeof(g->master));
		safestrncpy(g->mes1, mes1, sizeof(g->mes1));
		safestrncpy(g->mes2, mes2, sizeof(g->mes2));

		str+= len;
	}

	{// load guild member info
		int accountid;
		int charid;
		int hair, hair_color, gender;
		int class_, lv;
		unsigned int exp;
		int exp_payper;
		int position;
		char name[256]; // only 24 used
		int len;
		int i;

		for( i = 0; i < g->max_member; i++ )
		{
			struct guild_member* m = &g->member[i];
			if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%u,%d,%d\t%[^\t]\t%n",
					   &accountid, &charid, &hair, &hair_color, &gender,
					   &class_, &lv, &exp, &exp_payper, &position,
					   name, &len) < 11)
				return false;

			m->account_id = accountid;
			m->char_id = charid;
			m->hair = hair;
			m->hair_color = hair_color;
			m->gender = gender;
			m->class_ = class_;
			m->lv = lv;
			m->exp = exp;
			m->exp_payper = exp_payper;
			m->position = position;
			safestrncpy(m->name, name, NAME_LENGTH);

			str+= len;
		}
	}

	{// load guild position info
		int mode, exp_mode;
		char name[256]; // only 24 used
		int len;
		int i = 0;
		int j;

		while (sscanf(str, "%d,%d%n", &mode, &exp_mode, &j) == 2 && str[j] == '\t')
		{
			struct guild_position *p = &g->position[i];
			if (sscanf(str, "%d,%d\t%[^\t]\t%n", &mode, &exp_mode, name, &len) < 3)
				return false;

			p->mode = mode;
			p->exp_mode = exp_mode;
			name[strlen(name)-1] = 0;
			safestrncpy(p->name, name, NAME_LENGTH);

			i++;
			str+= len;
		}
	}

	{// load guild emblem
		int emblemlen;
		int emblemid;
		char emblem[4096];
		int len;
		char* pstr;
		int i;

		emblemid = 0;
		if( sscanf(str, "%d,%d,%[^\t]\t%n", &emblemlen, &emblemid, emblem, &len) < 3 )
		if( sscanf(str, "%d,%[^\t]\t%n", &emblemlen, emblem, &len) < 2 ) //! pre-svn format
			return false;

		g->emblem_len = emblemlen;
		g->emblem_id = emblemid;
		for(i = 0, pstr = emblem; i < g->emblem_len; i++, pstr += 2) {
			int c1 = pstr[0], c2 = pstr[1], x1 = 0, x2 = 0;
			if (c1 >= '0' && c1 <= '9') x1 = c1 - '0';
			if (c1 >= 'a' && c1 <= 'f') x1 = c1 - 'a' + 10;
			if (c1 >= 'A' && c1 <= 'F') x1 = c1 - 'A' + 10;
			if (c2 >= '0' && c2 <= '9') x2 = c2 - '0';
			if (c2 >= 'a' && c2 <= 'f') x2 = c2 - 'a' + 10;
			if (c2 >= 'A' && c2 <= 'F') x2 = c2 - 'A' + 10;
			g->emblem_data[i] = (x1<<4) | x2;
		}

		str+= len;
	}

	{// load guild alliance info
		int guildid;
		int opposition;
		char name[256]; // only 24 used
		int len;
		int i, c;

		if (sscanf(str, "%d\t%n", &c, &len) < 1)
			return false;
		str+= len;

		for(i = 0; i < c; i++)
		{
			struct guild_alliance* a = &g->alliance[i];
			if (sscanf(str, "%d,%d\t%[^\t]\t%n", &guildid, &opposition, name, &len) < 3)
				return false;

			a->guild_id = guildid;
			a->opposition = opposition;
			safestrncpy(a->name, name, NAME_LENGTH);

			str+= len;
		}
	}

	{// load guild expulsion info
		int accountid;
		char name[256]; // only 24 used
		char message[256]; // only 40 used
		int len;
		int i, c;

		if (sscanf(str, "%d\t%n", &c, &len) < 1)
			return false;
		str+= len;

		for(i = 0; i < c; i++)
		{
			struct guild_expulsion *e = &g->expulsion[i];
			if (sscanf(str, "%d,%*d,%*d,%*d\t%[^\t]\t%*[^\t]\t%[^\t]\t%n", &accountid, name, message, &len) < 3)
				return false;

			e->account_id = accountid;
			safestrncpy(e->name, name, sizeof(e->name));
			message[strlen(message)-1] = 0; // remove '#'
			safestrncpy(e->mes, message, sizeof(e->mes));

			str+= len;
		}
	}

	{// load guild skill info
		int skillid;
		int skilllv;
		int len;
		int i;

		for(i = 0; i < MAX_GUILDSKILL; i++)
		{
			if (sscanf(str, "%d,%d %n", &skillid, &skilllv, &len) < 2)
				break;
			g->skill[i].id = skillid;
			g->skill[i].lv = skilllv;

			str+= len;
		}
		str = strchr(str, '\t');
	}

	*key = g->guild_id;
/*
	// uniqueness checks
	if( db->db->exists(db->db, g->guild_id) )
	{
		char tmp[NAME_LENGTH];
		db->vtable.id2name(&db->vtable, g->guild_id, tmp, sizeof(tmp));
		ShowError(CL_RED"mmo_guild_fromstr: Collision on id %d between guild '%s' and existing guild '%s'!\n", g->guild_id, g->name, tmp);
		return false;
	}

	if( strdb_exists(db->idx_name, g->name) )
	{
		int tmp;
		tmp = (int)strdb_get(db->idx_name, g->name);
		ShowError(CL_RED"mmo_guild_fromstr: Collision on name '%s' between guild %d and existing guild %d!\n", g->name, g->guild_id, tmp);
		return false;
	}
*/
	return true;
}


/// Serializes the provided data structure into a string.
/// @protected
static bool guild_db_txt_tostr(char* str, int key, const void* data, size_t size)
{
	const struct guild* g = (const struct guild*)data;
	int i, c;
	int len;

	// save guild base info
	len = sprintf(str, "%d\t%s\t%s\t%d,%d,%u,%d,%d\t%s#\t%s#\t",
	              g->guild_id, g->name, g->master, g->guild_lv, g->max_member, g->exp, g->skill_point, 0, g->mes1, g->mes2);

	// save guild member info
	for( i = 0; i < g->max_member; i++ )
	{
		const struct guild_member* m = &g->member[i];
		len += sprintf(str + len, "%d,%d,%d,%d,%d,%d,%d,%u,%d,%d\t%s\t",
		               m->account_id, m->char_id,
		               m->hair, m->hair_color, m->gender,
		               m->class_, m->lv, m->exp, m->exp_payper, m->position,
		               ((m->account_id > 0) ? m->name : "-"));
	}

	// save guild position info
	for( i = 0; i < MAX_GUILDPOSITION; i++ )
	{
		const struct guild_position* p = &g->position[i];
		len += sprintf(str + len, "%d,%d\t%s#\t", p->mode, p->exp_mode, p->name);
	}

	// save guild emblem
	len += sprintf(str + len, "%d,%d,", g->emblem_len, g->emblem_id);
	for( i = 0; i < g->emblem_len; i++ )
		len += sprintf(str + len, "%02x", (unsigned char)(g->emblem_data[i]));
	len += sprintf(str + len, "$\t");

	// save guild alliance info
	c = 0;
	for( i = 0; i < MAX_GUILDALLIANCE; i++ )
		if( g->alliance[i].guild_id > 0 )
			c++;
	len += sprintf(str + len, "%d\t", c);
	for( i = 0; i < MAX_GUILDALLIANCE; i++ )
	{
		const struct guild_alliance* a = &g->alliance[i];
		if( a->guild_id == 0 )
			continue;

		len += sprintf(str + len, "%d,%d\t%s\t", a->guild_id, a->opposition, a->name);
	}

	// save guild expulsion info
	c = 0;
	for( i = 0; i < MAX_GUILDEXPULSION; i++ )
		if( g->expulsion[i].account_id > 0 )
			c++;
	len += sprintf(str + len, "%d\t", c);
	for( i = 0; i < MAX_GUILDEXPULSION; i++ )
	{
		const struct guild_expulsion* e = &g->expulsion[i];
		if( e->account_id == 0 )
			continue;

		len += sprintf(str + len, "%d,%d,%d,%d\t%s\t%s\t%s#\t", e->account_id, 0, 0, 0, e->name, "#", e->mes );
	}

	// save guild skill info
	for( i = 0; i < MAX_GUILDSKILL; i++ )
		len += sprintf(str + len, "%d,%d ", g->skill[i].id, g->skill[i].lv);
	len += sprintf(str + len, "\t");

	return true;
}


/// @protected
static bool guild_db_txt_init(GuildDB* self)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	CSDBIterator* iter;
	int guild_id;

	if( !db->db->init(db->db) )
		return false;

	// create index
	if( db->idx_name == NULL )
		db->idx_name = strdb_alloc(DB_OPT_DUP_KEY, 0);
	db_clear(db->idx_name);
	iter = db->db->iterator(db->db);
	while( iter->next(iter, &guild_id) )
	{
		struct guild g;
		db->db->load(db->db, guild_id, &g, sizeof(g), NULL);
		strdb_put(db->idx_name, g.name, (void*)guild_id);
	}
	iter->destroy(iter);

	return true;
}


/// @protected
static void guild_db_txt_destroy(GuildDB* self)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;

	// delete guild database
	db->db->destroy(db->db);

	// delete indexes
	if( db->idx_name != NULL )
		db_destroy(db->idx_name);

	// delete entire structure
	aFree(db);
}


/// @protected
static bool guild_db_txt_sync(GuildDB* self, bool force)
{
	CSDB_TXT* db = ((GuildDB_TXT*)self)->db;
	return db->sync(db, force);
}


/// @protected
static bool guild_db_txt_create(GuildDB* self, struct guild* g)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;

	if( g->guild_id == -1 )
		g->guild_id = db->db->next_key(db->db);

	// data restrictions
	if( self->id2name(self, g->guild_id, NULL, 0) )
		return false;// id is being used
	if( self->name2id(self, g->name, NULL) )
		return false;// name is being used

	// store data
	db->db->insert(db->db, g->guild_id, g, sizeof(*g));
	strdb_put(db->idx_name, g->name, (void*)g->guild_id);

	return true;
}


/// @protected
static bool guild_db_txt_remove(GuildDB* self, const int guild_id)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	CSDBIterator* iter;
	struct guild g;
	int tmp;

	if( !db->db->load(db->db, guild_id, &g, sizeof(g), NULL) )
		return true; // nothing to delete

	// delete from database and index
	db->db->remove(db->db, guild_id);
	strdb_remove(db->idx_name, g.name);

	// cancel alliances
	iter = db->db->iterator(db->db);
	while( iter->next(iter, &tmp) )
	{
		int i;
		bool changed = false;

		if( !db->db->load(db->db, tmp, &g, sizeof(g), NULL) )
			continue;

		for( i = 0; i < MAX_GUILDALLIANCE; i++ )
			if( g.alliance[i].guild_id == guild_id )
			{
				g.alliance[i].guild_id = 0;
				changed = true;
			}

		if( changed )
			db->db->update(db->db, tmp, &g, sizeof(g));
	}

	return true;
}


/// @protected
static bool guild_db_txt_save(GuildDB* self, const struct guild* g, enum guild_save_flags flag)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	struct guild tmp;
	bool name_changed = false;

	// retrieve previous data
	if( !db->db->load(db->db, g->guild_id, &tmp, sizeof(tmp), NULL) )
		return false; // entry not found

	// check integrity constraints
	if( strcmp(g->name, tmp.name) != 0 )
	{
		name_changed = true;
		if( strdb_exists(db->idx_name, g->name) )
			return false; // name already taken
	}

	// write new data
	if( !db->db->update(db->db, g->guild_id, g, sizeof(*g)) )
		return false;
	
	// update index
	if( name_changed )
	{
		strdb_remove(db->idx_name, tmp.name);
		strdb_put(db->idx_name, g->name, (void*)g->guild_id);
	}

	return true;
}


/// @protected
static bool guild_db_txt_load(GuildDB* self, struct guild* g, int guild_id)
{
	CSDB_TXT* db = ((GuildDB_TXT*)self)->db;
	return db->load(db, guild_id, g, sizeof(*g), NULL);
}


/// @protected
static bool guild_db_txt_id2name(GuildDB* self, int guild_id, char* name, size_t size)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	struct guild g;

	if( !db->db->load(db->db, guild_id, &g, sizeof(g), NULL) )
		return false;

	if( name != NULL )
		safestrncpy(name, g.name, size);
	
	return true;
}


/// @protected
static bool guild_db_txt_name2id(GuildDB* self, const char* name, int* guild_id)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;

	if( !strdb_exists(db->idx_name, name) )
		return false;
	
	// store it
	if( guild_id != NULL )
		*guild_id = (int)strdb_get(db->idx_name, name);

	return true;
}


/// Returns an iterator over all guilds.
/// @protected
static CSDBIterator* guild_db_txt_iterator(GuildDB* self)
{
	CSDB_TXT* db = ((GuildDB_TXT*)self)->db;
	return db->iterator(db);
}


/// Constructs a new GuildDB interface.
/// @protected
GuildDB* guild_db_txt(CharServerDB_TXT* owner)
{
	GuildDB_TXT* db = (GuildDB_TXT*)aCalloc(1, sizeof(GuildDB_TXT));

	// call base class constructor and bind abstract methods
	db->db = csdb_txt(owner, owner->file_guilds, GUILDDB_TXT_DB_VERSION, START_GUILD_NUM);
	db->db->p.fromstr = &guild_db_txt_fromstr;
	db->db->p.tostr   = &guild_db_txt_tostr;

	// set up the vtable
	db->vtable.p.init    = &guild_db_txt_init;
	db->vtable.p.destroy = &guild_db_txt_destroy;
	db->vtable.p.sync    = &guild_db_txt_sync;
	db->vtable.create    = &guild_db_txt_create;
	db->vtable.remove    = &guild_db_txt_remove;
	db->vtable.save      = &guild_db_txt_save;
	db->vtable.load      = &guild_db_txt_load;
	db->vtable.id2name   = &guild_db_txt_id2name;
	db->vtable.name2id   = &guild_db_txt_name2id;
	db->vtable.iterator  = &guild_db_txt_iterator;

	// initialize to default values
	db->idx_name = NULL;

	return &db->vtable;
}
