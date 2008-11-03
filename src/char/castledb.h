// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CASTLEDB_H_
#define _CASTLEDB_H_

#include "../common/mmo.h" // struct guild_castle

typedef struct CastleDB CastleDB;

// standard engines
#ifdef WITH_TXT
CastleDB* castle_db_txt(void);
#endif
#ifdef WITH_SQL
CastleDB* castle_db_sql(void);
#endif


struct CastleDB
{
	bool (*init)(CastleDB* self);

	void (*destroy)(CastleDB* self);

	bool (*sync)(CastleDB* self);

	bool (*create)(CastleDB* self, struct guild_castle* gc);

	bool (*remove)(CastleDB* self, const int castle_id);

	bool (*save)(CastleDB* self, const struct guild_castle* gc);

	bool (*load_num)(CastleDB* self, struct guild_castle* gc, int castle_id);
};


#endif /* _CASTLEDB_H_ */
