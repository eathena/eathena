// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _ONLINEDB_H_
#define _ONLINEDB_H_

typedef struct OnlineDB OnlineDB;


struct OnlineDB
{
	bool (*init)(OnlineDB* self);
	void (*destroy)(OnlineDB* self);
	bool (*sync)(OnlineDB* self);
	bool (*get_property)(OnlineDB* self, const char* key, char* buf, size_t buflen);
	bool (*set_property)(OnlineDB* self, const char* key, const char* value);

	/// Mark the character of this account as online.
	bool (*set_online)(OnlineDB* self, int account_id, int char_id);

	/// Mark the character of this account as online.
	/// If 'char_id' is -1, apply this action to all characters of the account.
	/// If 'account_id' is -1 as well, apply to all characters in the database.
	bool (*set_offline)(OnlineDB* self, int account_id, int char_id);
};


// standard engines
#ifdef WITH_TXT
OnlineDB* online_db_txt(void);
#endif
#ifdef WITH_SQL
OnlineDB* online_db_sql(void);
#endif


#endif /* _ONLINEDB_H_ */
