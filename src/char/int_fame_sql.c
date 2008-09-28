#include "../common/mmo.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "chardb.h"
#include "int_fame.h"
#include <stdlib.h>
#include <string.h>

//temporary imports
extern CharDB* chars;
extern Sql* sql_handle;


void char_read_fame_list(void)
{
	int i;
	char* data;
	struct fame_list* list;
	int size;
	char char_db[] = "char"; //TODO: remove this hack

	// Build Blacksmith ranking list
	get_fame_list(FAME_SMITH, &list, &size);
	memset(list, 0, MAX_FAME_LIST*sizeof(struct fame_list));
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d' OR `class`='%d' OR `class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", char_db, JOB_BLACKSMITH, JOB_WHITESMITH, JOB_BABY_BLACKSMITH, size) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < size && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		Sql_GetData(sql_handle, 0, &data, NULL); list[i].id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); list[i].fame = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(list[i].name, data, NAME_LENGTH);
	}

	// Build Alchemist ranking list
	get_fame_list(FAME_CHEMIST, &list, &size);
	memset(list, 0, MAX_FAME_LIST*sizeof(struct fame_list));
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d' OR `class`='%d' OR `class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", char_db, JOB_ALCHEMIST, JOB_CREATOR, JOB_BABY_ALCHEMIST, size) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < size && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		Sql_GetData(sql_handle, 0, &data, NULL); list[i].id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); list[i].fame = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(list[i].name, data, NAME_LENGTH);
	}

	// Build Taekwon ranking list
	get_fame_list(FAME_TAEKWON, &list, &size);
	memset(list, 0, MAX_FAME_LIST*sizeof(struct fame_list));
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`,`fame`,`name` FROM `%s` WHERE `fame`>0 AND (`class`='%d') ORDER BY `fame` DESC LIMIT 0,%d", char_db, JOB_TAEKWON, size) )
		Sql_ShowDebug(sql_handle);
	for( i = 0; i < size && SQL_SUCCESS == Sql_NextRow(sql_handle); ++i )
	{
		Sql_GetData(sql_handle, 0, &data, NULL); list[i].id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); list[i].fame = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(list[i].name, data, NAME_LENGTH);
	}

	Sql_FreeResult(sql_handle);
}
