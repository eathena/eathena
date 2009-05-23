// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "charserverdb_sql.h"
#include "guilddb.h"
#include <string.h>

static const char dataToHex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


/// internal structure
typedef struct GuildDB_SQL
{
	GuildDB vtable;    // public interface

	CharServerDB_SQL* owner;
	Sql* guilds;       // SQL guild storage

	// other settings
	const char* guild_db;
	const char* guild_alliance_db;
	const char* guild_expulsion_db;
	const char* guild_member_db;
	const char* guild_position_db;
	const char* guild_skill_db;

} GuildDB_SQL;



static bool mmo_guild_fromsql(GuildDB_SQL* db, struct guild* g, int guild_id)
{
/*
	struct guild *g;
	char* data;
	size_t len;
	char* p;
	int i;

	if( guild_id <= 0 )
		return NULL;

	
//	g = (struct guild*)idb_get(guild_db_, guild_id);
//	if( g )
//		return g;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `name`,`master`,`guild_lv`,`connect_member`,`max_member`,`average_lv`,`exp`,`next_exp`,`skill_point`,`mes1`,`mes2`,`emblem_len`,`emblem_id`,`emblem_data` "
		"FROM `%s` WHERE `guild_id`='%d'", guild_db, guild_id) )
	{
		Sql_ShowDebug(sql_handle);
		return NULL;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
		return NULL;// Guild does not exists.

	CREATE(g, struct guild, 1);

	g->guild_id = guild_id;
	Sql_GetData(sql_handle,  0, &data, &len); memcpy(g->name, data, min(len, NAME_LENGTH));
	Sql_GetData(sql_handle,  1, &data, &len); memcpy(g->master, data, min(len, NAME_LENGTH));
	Sql_GetData(sql_handle,  2, &data, NULL); g->guild_lv = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); g->connect_member = atoi(data);
	Sql_GetData(sql_handle,  4, &data, NULL); g->max_member = atoi(data);
	if( g->max_member > MAX_GUILD )
	{	// Fix reduction of MAX_GUILD [PoW]
		ShowWarning("Guild %d:%s specifies higher capacity (%d) than MAX_GUILD (%d)\n", guild_id, g->name, g->max_member, MAX_GUILD);
		g->max_member = MAX_GUILD;
	}
	Sql_GetData(sql_handle,  5, &data, NULL); g->average_lv = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); g->exp = (unsigned int)strtoul(data, NULL, 10);
	Sql_GetData(sql_handle,  7, &data, NULL); g->next_exp = (unsigned int)strtoul(data, NULL, 10);
	Sql_GetData(sql_handle,  8, &data, NULL); g->skill_point = atoi(data);
	Sql_GetData(sql_handle,  9, &data, &len); memcpy(g->mes1, data, min(len, sizeof(g->mes1)));
	Sql_GetData(sql_handle, 10, &data, &len); memcpy(g->mes2, data, min(len, sizeof(g->mes2)));
	Sql_GetData(sql_handle, 11, &data, &len); g->emblem_len = atoi(data);
	Sql_GetData(sql_handle, 12, &data, &len); g->emblem_id = atoi(data);
	Sql_GetData(sql_handle, 13, &data, &len);
	// convert emblem data from hexadecimal to binary
	//TODO: why not store it in the db as binary directly? [ultramage]
	for( i = 0, p = g->emblem_data; i < g->emblem_len; ++i, ++p )
	{
		if( *data >= '0' && *data <= '9' )
			*p = *data - '0';
		else if( *data >= 'a' && *data <= 'f' )
			*p = *data - 'a' + 10;
		else if( *data >= 'A' && *data <= 'F' )
			*p = *data - 'A' + 10;
		*p <<= 4;
		++data;

		if( *data >= '0' && *data <= '9' )
			*p |= *data - '0';
		else if( *data >= 'a' && *data <= 'f' )
			*p |= *data - 'a' + 10;
		else if( *data >= 'A' && *data <= 'F' )
			*p |= *data - 'A' + 10;
		++data;
	}

	// load guild member info
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`name` "
		"FROM `%s` WHERE `guild_id`='%d' ORDER BY `position`", guild_member_db, guild_id) )
	{
		Sql_ShowDebug(sql_handle);
		aFree(g);
		return NULL;
	}
	for( i = 0; i < g->max_member && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		struct guild_member* m = &g->member[i];

		Sql_GetData(sql_handle,  0, &data, NULL); m->account_id = atoi(data);
		Sql_GetData(sql_handle,  1, &data, NULL); m->char_id = atoi(data);
		Sql_GetData(sql_handle,  2, &data, NULL); m->hair = atoi(data);
		Sql_GetData(sql_handle,  3, &data, NULL); m->hair_color = atoi(data);
		Sql_GetData(sql_handle,  4, &data, NULL); m->gender = atoi(data);
		Sql_GetData(sql_handle,  5, &data, NULL); m->class_ = atoi(data);
		Sql_GetData(sql_handle,  6, &data, NULL); m->lv = atoi(data);
		Sql_GetData(sql_handle,  7, &data, NULL); m->exp = (unsigned int)strtoul(data, NULL, 10);
		Sql_GetData(sql_handle,  8, &data, NULL); m->exp_payper = (unsigned int)atoi(data);
		Sql_GetData(sql_handle,  9, &data, NULL); m->online = atoi(data);
		Sql_GetData(sql_handle, 10, &data, NULL); m->position = atoi(data);
		if( m->position >= MAX_GUILDPOSITION ) // Fix reduction of MAX_GUILDPOSITION [PoW]
			m->position = MAX_GUILDPOSITION - 1;
		Sql_GetData(sql_handle, 11, &data, &len); memcpy(m->name, data, min(len, NAME_LENGTH));
		m->modified = GS_MEMBER_UNMODIFIED;
	}

	//printf("- Read guild_position %d from sql \n",guild_id);
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `position`,`name`,`mode`,`exp_mode` FROM `%s` WHERE `guild_id`='%d'", guild_position_db, guild_id) )
	{
		Sql_ShowDebug(sql_handle);
		aFree(g);
		return NULL;
	}
	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		int position;
		struct guild_position* p;

		Sql_GetData(sql_handle, 0, &data, NULL); position = atoi(data);
		if( position < 0 || position >= MAX_GUILDPOSITION )
			continue;// invalid position
		p = &g->position[position];
		Sql_GetData(sql_handle, 1, &data, &len); memcpy(p->name, data, min(len, NAME_LENGTH));
		Sql_GetData(sql_handle, 2, &data, NULL); p->mode = atoi(data);
		Sql_GetData(sql_handle, 3, &data, NULL); p->exp_mode = atoi(data);
		p->modified = GS_POSITION_UNMODIFIED;
	}

	//printf("- Read guild_alliance %d from sql \n",guild_id);
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `opposition`,`alliance_id`,`name` FROM `%s` WHERE `guild_id`='%d'", guild_alliance_db, guild_id) )
	{
		Sql_ShowDebug(sql_handle);
		aFree(g);
		return NULL;
	}
	for( i = 0; i < MAX_GUILDALLIANCE && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		struct guild_alliance* a = &g->alliance[i];

		Sql_GetData(sql_handle, 0, &data, NULL); a->opposition = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); a->guild_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, &len); memcpy(a->name, data, min(len, NAME_LENGTH));
	}

	//printf("- Read guild_expulsion %d from sql \n",guild_id);
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`,`name`,`mes` FROM `%s` WHERE `guild_id`='%d'", guild_expulsion_db, guild_id) )
	{
		Sql_ShowDebug(sql_handle);
		aFree(g);
		return NULL;
	}
	for( i = 0; i < MAX_GUILDEXPULSION && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		struct guild_expulsion *e = &g->expulsion[i];

		Sql_GetData(sql_handle, 0, &data, NULL); e->account_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, &len); memcpy(e->name, data, min(len, NAME_LENGTH));
		Sql_GetData(sql_handle, 2, &data, &len); memcpy(e->mes, data, min(len, sizeof(e->mes)));
	}

	//printf("- Read guild_skill %d from sql \n",guild_id);
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id`,`lv` FROM `%s` WHERE `guild_id`='%d' ORDER BY `id`", guild_skill_db, guild_id) )
	{
		Sql_ShowDebug(sql_handle);
		aFree(g);
		return NULL;
	}

	for(i = 0; i < MAX_GUILDSKILL; i++)
	{	//Skill IDs must always be initialized. [Skotlex]
		g->skill[i].id = i + GD_SKILLBASE;
	}

	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		int id;
		Sql_GetData(sql_handle, 0, &data, NULL); id = atoi(data) - GD_SKILLBASE;
		if( id < 0 && id >= MAX_GUILDSKILL )
			continue;// invalid guild skill
		Sql_GetData(sql_handle, 1, &data, NULL); g->skill[id].lv = atoi(data);
	}
	Sql_FreeResult(sql_handle);

	idb_put(guild_db_, guild_id, g); //Add to cache
	g->save_flag |= GS_REMOVE; //But set it to be removed, in case it is not needed for long.
*/
}


static bool mmo_guild_tosql(GuildDB_SQL* db, const struct guild* g, int flag)
{
	//TODO: recognize GS_MEMBER_DELETED modified flag in guild member structure
/*
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE from `%s` where `account_id` = '%d' and `char_id` = '%d'", guild_member_db, account_id, char_id) )
		Sql_ShowDebug(sql_handle);
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `guild_id` = '0' WHERE `char_id` = '%d'", char_db, char_id) )
		Sql_ShowDebug(sql_handle);
*/
/*
	// Table guild (GS_BASIC_MASK)
	// GS_EMBLEM `emblem_len`,`emblem_id`,`emblem_data`
	// GS_CONNECT `connect_member`,`average_lv`
	// GS_MES `mes1`,`mes2`
	// GS_LEVEL `guild_lv`,`max_member`,`exp`,`next_exp`,`skill_point`
	// GS_BASIC `name`,`master`,`char_id`

	// GS_MEMBER `guild_member` (`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`name`)
	// GS_POSITION `guild_position` (`guild_id`,`position`,`name`,`mode`,`exp_mode`)
	// GS_ALLIANCE `guild_alliance` (`guild_id`,`opposition`,`alliance_id`,`name`)
	// GS_EXPULSION `guild_expulsion` (`guild_id`,`account_id`,`name`,`mes`)
	// GS_SKILL `guild_skill` (`guild_id`,`id`,`lv`) 

	// temporary storage for str convertion. They must be twice the size of the
	// original string to ensure no overflows will occur. [Skotlex]
	char t_info[256];
	char esc_name[NAME_LENGTH*2+1];
	char esc_master[NAME_LENGTH*2+1];
	char new_guild = 0;
	int i=0;

	if (g->guild_id<=0 && g->guild_id != -1) return 0;
	
	Sql_EscapeStringLen(sql_handle, esc_name, g->name, strnlen(g->name, NAME_LENGTH));
	Sql_EscapeStringLen(sql_handle, esc_master, g->master, strnlen(g->master, NAME_LENGTH));
	*t_info = '\0';

#ifndef TXT_SQL_CONVERT
	// Insert a new guild the guild
	if (flag&GS_BASIC && g->guild_id == -1)
	{
		strcat(t_info, " guild_create");

		// Create a new guild
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` "
			"(`name`,`master`,`guild_lv`,`max_member`,`average_lv`,`char_id`) "
			"VALUES ('%s', '%s', '%d', '%d', '%d', '%d')",
			guild_db, esc_name, esc_master, g->guild_lv, g->max_member, g->average_lv, g->member[0].char_id) )
		{
			Sql_ShowDebug(sql_handle);
			if (g->guild_id == -1)
				return 0; //Failed to create guild!
		}
		else
		{
			g->guild_id = (int)Sql_LastInsertId(sql_handle);
			new_guild = 1;
		}
	}
#else
	// Insert a new guild the guild
	if (flag&GS_BASIC)
	{
		strcat(t_info, " guild_create");
		// Since the PK is guild id + master id, a replace will not be enough if we are overwriting data, we need to wipe the previous guild.
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` where `guild_id` = '%d'", guild_db, g->guild_id) )
			Sql_ShowDebug(sql_handle);
		// Create a new guild
		if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` "
			"(`guild_id`,`name`,`master`,`guild_lv`,`max_member`,`average_lv`,`char_id`) "
			"VALUES ('%d', '%s', '%s', '%d', '%d', '%d', '%d')",
			guild_db, g->guild_id, esc_name, esc_master, g->guild_lv, g->max_member, g->average_lv, g->member[0].char_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0; //Failed to create guild.
		}
	}
#endif //TXT_SQL_CONVERT

	// If we need an update on an existing guild or more update on the new guild
	if (((flag & GS_BASIC_MASK) && !new_guild) || ((flag & (GS_BASIC_MASK & ~GS_BASIC)) && new_guild))
	{
		StringBuf buf;
		bool add_comma = false;

		StringBuf_Init(&buf);
		StringBuf_Printf(&buf, "UPDATE `%s` SET ", guild_db);

		if (flag & GS_EMBLEM)
		{
			char emblem_data[sizeof(g->emblem_data)*2+1];
			char* pData = emblem_data;

			strcat(t_info, " emblem");
			// Convert emblem_data to hex
			//TODO: why not use binary directly? [ultramage]
			for(i=0; i<g->emblem_len; i++){
				*pData++ = dataToHex[(g->emblem_data[i] >> 4) & 0x0F];
				*pData++ = dataToHex[g->emblem_data[i] & 0x0F];
			}
			*pData = 0;
			StringBuf_Printf(&buf, "`emblem_len`=%d, `emblem_id`=%d, `emblem_data`='%s'", g->emblem_len, g->emblem_id, emblem_data);
			add_comma = true;
		}
		if (flag & GS_BASIC) 
		{
			strcat(t_info, " basic");
			if( add_comma )
				StringBuf_AppendStr(&buf, ", ");
			else
				add_comma = true;
			StringBuf_Printf(&buf, "`name`='%s', `master`='%s', `char_id`=%d", esc_name, esc_master, g->member[0].char_id);
		}
		if (flag & GS_CONNECT)
		{
			strcat(t_info, " connect");
			if( add_comma )
				StringBuf_AppendStr(&buf, ", ");
			else
				add_comma = true;
			StringBuf_Printf(&buf, "`connect_member`=%d, `average_lv`=%d", g->connect_member, g->average_lv);
		}
		if (flag & GS_MES)
		{
			char esc_mes1[sizeof(g->mes1)*2+1];
			char esc_mes2[sizeof(g->mes2)*2+1];

			strcat(t_info, " mes");
			if( add_comma )
				StringBuf_AppendStr(&buf, ", ");
			else
				add_comma = true;
			Sql_EscapeStringLen(sql_handle, esc_mes1, g->mes1, strnlen(g->mes1, sizeof(g->mes1)));
			Sql_EscapeStringLen(sql_handle, esc_mes2, g->mes2, strnlen(g->mes2, sizeof(g->mes2)));
			StringBuf_Printf(&buf, "`mes1`='%s', `mes2`='%s'", esc_mes1, esc_mes2);
		}
		if (flag & GS_LEVEL)
		{
			strcat(t_info, " level");
			if( add_comma )
				StringBuf_AppendStr(&buf, ", ");
			else
				add_comma = true;
			StringBuf_Printf(&buf, "`guild_lv`=%d, `skill_point`=%d, `exp`=%u, `next_exp`=%u, `max_member`=%d", g->guild_lv, g->skill_point, g->exp, g->next_exp, g->max_member);
		}
		StringBuf_Printf(&buf, " WHERE `guild_id`=%d", g->guild_id);
		if( SQL_ERROR == Sql_Query(sql_handle, "%s", StringBuf_Value(&buf)) )
			Sql_ShowDebug(sql_handle);
		StringBuf_Destroy(&buf);
	}

	if (flag&GS_MEMBER)
	{
		struct guild_member *m;

		strcat(t_info, " members");
		// Update only needed players
		for(i=0;i<g->max_member;i++){
			m = &g->member[i];
#ifndef TXT_SQL_CONVERT
			if (!m->modified)
				continue;
#endif
			if(m->account_id) {
				//Since nothing references guild member table as foreign keys, it's safe to use REPLACE INTO
				Sql_EscapeStringLen(sql_handle, esc_name, m->name, strnlen(m->name, NAME_LENGTH));
				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`name`) "
					"VALUES ('%d','%d','%d','%d','%d','%d','%d','%d','%u','%d','%d','%d','%s')",
					guild_member_db, g->guild_id, m->account_id, m->char_id,
					m->hair, m->hair_color, m->gender,
					m->class_, m->lv, m->exp, m->exp_payper, m->online, m->position, esc_name) )
					Sql_ShowDebug(sql_handle);
				if (m->modified & GS_MEMBER_NEW)
				{
					if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `guild_id` = '%d' WHERE `char_id` = '%d'",
						char_db, g->guild_id, m->char_id) )
						Sql_ShowDebug(sql_handle);
				}
				m->modified = GS_MEMBER_UNMODIFIED;
			}
		}
	}

	if (flag&GS_POSITION){
		strcat(t_info, " positions");
		//printf("- Insert guild %d to guild_position\n",g->guild_id);
		for(i=0;i<MAX_GUILDPOSITION;i++){
			struct guild_position *p = &g->position[i];
#ifndef TXT_SQL_CONVERT
			if (!p->modified)
				continue;
#endif
			Sql_EscapeStringLen(sql_handle, esc_name, p->name, strnlen(p->name, NAME_LENGTH));
			if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`guild_id`,`position`,`name`,`mode`,`exp_mode`) VALUES ('%d','%d','%s','%d','%d')",
				guild_position_db, g->guild_id, i, esc_name, p->mode, p->exp_mode) )
				Sql_ShowDebug(sql_handle);
			p->modified = GS_POSITION_UNMODIFIED;
		}
	}

	if (flag&GS_ALLIANCE)
	{
		// Delete current alliances 
		// NOTE: no need to do it on both sides since both guilds in memory had 
		// their info changed, not to mention this would also mess up oppositions!
		// [Skotlex]
		//if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id`='%d' OR `alliance_id`='%d'", guild_alliance_db, g->guild_id, g->guild_id) )
		if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id`='%d'", guild_alliance_db, g->guild_id) )
		{
			Sql_ShowDebug(sql_handle);
		}
		else
		{
			//printf("- Insert guild %d to guild_alliance\n",g->guild_id);
			for(i=0;i<MAX_GUILDALLIANCE;i++)
			{
				struct guild_alliance *a=&g->alliance[i];
				if(a->guild_id>0)
				{
					Sql_EscapeStringLen(sql_handle, esc_name, a->name, strnlen(a->name, NAME_LENGTH));
					if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`guild_id`,`opposition`,`alliance_id`,`name`) "
						"VALUES ('%d','%d','%d','%s')",
						guild_alliance_db, g->guild_id, a->opposition, a->guild_id, esc_name) )
						Sql_ShowDebug(sql_handle);
				}
			}
		}
	}

	if (flag&GS_EXPULSION){
		strcat(t_info, " expulsions");
		//printf("- Insert guild %d to guild_expulsion\n",g->guild_id);
		for(i=0;i<MAX_GUILDEXPULSION;i++){
			struct guild_expulsion *e=&g->expulsion[i];
			if(e->account_id>0){
				char esc_mes[sizeof(e->mes)*2+1];

				Sql_EscapeStringLen(sql_handle, esc_name, e->name, strnlen(e->name, NAME_LENGTH));
				Sql_EscapeStringLen(sql_handle, esc_mes, e->mes, strnlen(e->mes, sizeof(e->mes)));
				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`guild_id`,`account_id`,`name`,`mes`) "
					"VALUES ('%d','%d','%s','%s')", guild_expulsion_db, g->guild_id, e->account_id, esc_name, esc_mes) )
					Sql_ShowDebug(sql_handle);
			}
		}
	}

	if (flag&GS_SKILL){
		strcat(t_info, " skills");
		//printf("- Insert guild %d to guild_skill\n",g->guild_id);
		for(i=0;i<MAX_GUILDSKILL;i++){
			if (g->skill[i].id>0 && g->skill[i].lv>0){
				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`guild_id`,`id`,`lv`) VALUES ('%d','%d','%d')",
					guild_skill_db, g->guild_id, g->skill[i].id, g->skill[i].lv) )
					Sql_ShowDebug(sql_handle);
			}
		}
	}
*/
}


static bool guild_db_sql_init(GuildDB* self)
{
	GuildDB_SQL* db = (GuildDB_SQL*)self;
	db->guilds = db->owner->sql_handle;
	return true;
}

static void guild_db_sql_destroy(GuildDB* self)
{
	GuildDB_SQL* db = (GuildDB_SQL*)self;
	db->guilds = NULL;
	aFree(db);
}

static bool guild_db_sql_sync(GuildDB* self)
{
	return true;
}

static bool guild_db_sql_create(GuildDB* self, struct guild* g)
{
/*
	if (!inter_guild_tosql(g,GS_BASIC|GS_POSITION|GS_SKILL)) {
*/
}

static bool guild_db_sql_remove(GuildDB* self, const int guild_id)
{
/*
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_member_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_castle_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_position_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_skill_db, guild_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_expulsion_db, guild_id) )
		Sql_ShowDebug(sql_handle);
*/
}

static bool guild_db_sql_save(GuildDB* self, const struct guild* g)
{
}

static bool guild_db_sql_load(GuildDB* self, struct guild* g, int guild_id)
{
}

static bool guild_db_sql_name2id(GuildDB* self, const char* name, int* guild_id)
{
/*
	int guild_id;
	char esc_name[NAME_LENGTH*2+1];
	
	Sql_EscapeStringLen(sql_handle, esc_name, str, safestrnlen(str, NAME_LENGTH));
	//Lookup guilds with the same name
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT guild_id FROM `%s` WHERE name='%s'", guild_db, esc_name) )
	{
		Sql_ShowDebug(sql_handle);
		return -1;
	}

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		char* data;

		Sql_GetData(sql_handle, 0, &data, NULL);
		guild_id = atoi(data);
	}
	else
	{
		guild_id = 0;
	}
	Sql_FreeResult(sql_handle);
	return guild_id;
*/
}


/// public constructor
GuildDB* guild_db_sql(CharServerDB_SQL* owner)
{
	GuildDB_SQL* db = (GuildDB_SQL*)aCalloc(1, sizeof(GuildDB_SQL));

	// set up the vtable
	db->vtable.init      = &guild_db_sql_init;
	db->vtable.destroy   = &guild_db_sql_destroy;
	db->vtable.sync      = &guild_db_sql_sync;
	db->vtable.create    = &guild_db_sql_create;
	db->vtable.remove    = &guild_db_sql_remove;
	db->vtable.save      = &guild_db_sql_save;
	db->vtable.load      = &guild_db_sql_load;
	db->vtable.name2id   = &guild_db_sql_name2id;

	// initialize to default values
	db->owner = owner;
	db->guilds = NULL;

	// other settings
	db->guild_db = db->owner->table_guilds;
	db->guild_alliance_db = db->owner->table_guild_alliances;
	db->guild_expulsion_db = db->owner->table_guild_expulsions;
	db->guild_member_db = db->owner->table_guild_members;
	db->guild_position_db = db->owner->table_guild_positions;
	db->guild_skill_db = db->owner->table_guild_skills;

	return &db->vtable;
}
