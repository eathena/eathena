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
#include "guilddb.h"
#include <stdio.h>
#include <string.h>

#define START_GUILD_NUM 1

/// internal structure
typedef struct GuildDB_TXT
{
	GuildDB vtable;      // public interface

	CharServerDB_TXT* owner;
	DBMap* guilds;       // in-memory guild storage
	int next_guild_id;   // auto_increment

	bool case_sensitive;
	const char* guild_db; // guild data storage file

} GuildDB_TXT;



/// parses the guild data string into a guild data structure
static bool mmo_guild_fromstr(struct guild* g, char* str)
{
	int i, c;

	memset(g, 0, sizeof(struct guild));

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
		int i;

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

	return true;
}


/// serializes the guild data structure into the provided string
static bool mmo_guild_tostr(const struct guild* g, char* str)
{
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


static bool mmo_guild_sync(GuildDB_TXT* db)
{
	FILE *fp;
	int lock;
	struct DBIterator* iter;
	struct guild* g;

	fp = lock_fopen(db->guild_db, &lock);
	if( fp == NULL )
	{
		ShowError("mmo_guild_sync: can't write [%s] !!! data is lost !!!\n", db->guild_db);
		return false;
	}

	iter = db->guilds->iterator(db->guilds);
	for( g = (struct guild*)iter->first(iter,NULL); iter->exists(iter); g = (struct guild*)iter->next(iter,NULL) )
	{
		char buf[16384]; // ought to be big enough ^^
		mmo_guild_tostr(g, buf);
		fprintf(fp, "%s\n", buf);
	}
	iter->destroy(iter);

	lock_fclose(fp, db->guild_db, &lock);

	return true;
}


static bool guild_db_txt_init(GuildDB* self)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	DBMap* guilds;

	char line[16384];
	FILE* fp;

	// create guild database
	db->guilds = idb_alloc(DB_OPT_RELEASE_DATA);
	guilds = db->guilds;

	// open data file
	fp = fopen(db->guild_db, "r");
	if( fp == NULL )
		return 1;

	// load data file
	while( fgets(line, sizeof(line), fp) )
	{

		int guild_id, n;
		struct guild g;
		struct guild* tmp;

		n = 0;
		if( sscanf(line, "%d\t%%newid%%%n", &guild_id, &n) == 1 && n > 0 && (line[n] == '\n' || line[n] == '\r') )
		{// auto-increment
			if( guild_id > db->next_guild_id )
				db->next_guild_id = guild_id;
			continue;
		}

		if( !mmo_guild_fromstr(&g, line) )
		{
			ShowError("guild_db_txt_init: skipping invalid data: %s", line);
			continue;
		}

		// record entry in db
		tmp = (struct guild*)aMalloc(sizeof(struct guild));
		memcpy(tmp, &g, sizeof(struct guild));
		idb_put(guilds, g.guild_id, tmp);

		if( g.guild_id >= db->next_guild_id )
			db->next_guild_id = g.guild_id + 1;
	}

	// close data file
	fclose(fp);

	return true;
}

static void guild_db_txt_destroy(GuildDB* self)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	DBMap* guilds = db->guilds;

	// write data
	mmo_guild_sync(db);

	// delete guild database
	guilds->destroy(guilds, NULL);
	db->guilds = NULL;

	// delete entire structure
	aFree(db);
}

static bool guild_db_txt_sync(GuildDB* self)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	return mmo_guild_sync(db);
}

static bool guild_db_txt_create(GuildDB* self, struct guild* g)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	DBMap* guilds = db->guilds;
	struct guild* tmp;

	// decide on the guild id to assign
	int guild_id = ( g->guild_id != -1 ) ? g->guild_id : db->next_guild_id;

	// check if the guild_id is free
	tmp = idb_get(guilds, guild_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("guild_db_txt_create: cannot create guild %d:'%s', this id is already occupied by %d:'%s'!\n", guild_id, g->name, guild_id, tmp->name);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct guild, 1);
	memcpy(tmp, g, sizeof(struct guild));
	tmp->guild_id = guild_id;
	idb_put(guilds, guild_id, tmp);

	// increment the auto_increment value
	if( guild_id >= db->next_guild_id )
		db->next_guild_id = guild_id + 1;

	// flush data
	mmo_guild_sync(db);

	// write output
	g->guild_id = guild_id;

	return true;
}

static bool guild_db_txt_remove(GuildDB* self, const int guild_id)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	DBMap* guilds = db->guilds;
	struct DBIterator* iter;

	struct guild* tmp = (struct guild*)idb_remove(guilds, guild_id);
	if( tmp == NULL )
	{// error condition - entry not present
		ShowError("guild_db_txt_remove: no such guild with id %d\n", guild_id);
		return false;
	}

	// end all alliances / oppositions
	iter = guilds->iterator(guilds);
	while( (tmp = (struct guild*)iter->next(iter,NULL)) != NULL )
	{
		int i;
		for( i = 0; i < MAX_GUILDALLIANCE; i++ )
			if( tmp->alliance[i].guild_id == guild_id )
				tmp->alliance[i].guild_id = 0;
	}
	iter->destroy(iter);

	return true;
}

static bool guild_db_txt_save(GuildDB* self, const struct guild* g, enum guild_save_flags flag)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	DBMap* guilds = db->guilds;
	int guild_id = g->guild_id;

	// retrieve previous data
	struct guild* tmp = idb_get(guilds, guild_id);
	if( tmp == NULL )
	{// error condition - entry not found
		return false;
	}
	
	// overwrite with new data
	memcpy(tmp, g, sizeof(struct guild));

	return true;
}

static bool guild_db_txt_load(GuildDB* self, struct guild* g, int guild_id)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	DBMap* guilds = db->guilds;

	// retrieve data
	struct guild* tmp = idb_get(guilds, guild_id);
	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	memcpy(g, tmp, sizeof(struct guild));

	return true;
}

static bool guild_db_txt_name2id(GuildDB* self, const char* name, int* guild_id)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	DBMap* guilds = db->guilds;

	// retrieve data
	struct DBIterator* iter;
	struct guild* tmp;
	int (*compare)(const char* str1, const char* str2) = ( db->case_sensitive ) ? strcmp : stricmp;

	iter = guilds->iterator(guilds);
	for( tmp = (struct guild*)iter->first(iter,NULL); iter->exists(iter); tmp = (struct guild*)iter->next(iter,NULL) )
		if( compare(name, tmp->name) == 0 )
			break;
	iter->destroy(iter);

	if( tmp == NULL )
	{// entry not found
		return false;
	}

	// store it
	if( guild_id != NULL )
		*guild_id = tmp->guild_id;

	return true;
}


/// Returns an iterator over all guilds.
static CSDBIterator* guild_db_txt_iterator(GuildDB* self)
{
	GuildDB_TXT* db = (GuildDB_TXT*)self;
	return csdb_txt_iterator(db_iterator(db->guilds));
}


/// public constructor
GuildDB* guild_db_txt(CharServerDB_TXT* owner)
{
	GuildDB_TXT* db = (GuildDB_TXT*)aCalloc(1, sizeof(GuildDB_TXT));

	// set up the vtable
	db->vtable.init      = &guild_db_txt_init;
	db->vtable.destroy   = &guild_db_txt_destroy;
	db->vtable.sync      = &guild_db_txt_sync;
	db->vtable.create    = &guild_db_txt_create;
	db->vtable.remove    = &guild_db_txt_remove;
	db->vtable.save      = &guild_db_txt_save;
	db->vtable.load      = &guild_db_txt_load;
	db->vtable.name2id   = &guild_db_txt_name2id;
	db->vtable.iterator  = &guild_db_txt_iterator;

	// initialize to default values
	db->owner = owner;
	db->guilds = NULL;
	db->next_guild_id = START_GUILD_NUM;

	// other settings
	db->case_sensitive = false;
	db->guild_db = db->owner->file_guilds;

	return &db->vtable;
}
