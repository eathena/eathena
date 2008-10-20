// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _HOMUNDB_H_
#define _HOMUNDB_H_

#include "../common/mmo.h" // struct s_homunculus

typedef struct HomunDB HomunDB;

// standard engines
#ifdef WITH_TXT
HomunDB* homun_db_txt(void);
#endif
#ifdef WITH_SQL
HomunDB* homun_db_sql(void);
#endif


struct HomunDB
{
	bool (*init)(HomunDB* self);

	void (*destroy)(HomunDB* self);

	bool (*sync)(HomunDB* self);

	bool (*create)(HomunDB* self, struct s_homunculus* p);

	bool (*remove)(HomunDB* self, const int homun_id);

	bool (*save)(HomunDB* self, const struct s_homunculus* p);

	bool (*load_num)(HomunDB* self, struct s_homunculus* p, int homun_id);
};


#endif /* _HOMUNDB_H_ */
