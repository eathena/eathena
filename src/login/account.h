#ifndef __ACCOUNT_H_INCLUDED__
#define __ACCOUNT_H_INCLUDED__

#include "../common/mmo.h" // ACCOUNT_REG2_NUM


typedef struct AccountDB AccountDB;

AccountDB* account_db_txt(void);
AccountDB* account_db_sql(void);

struct AccountDB
{
	/// makes the db ready for use
	bool (*init)(AccountDB* self);

	/// releases all allocated data and the db itself
	bool (*free)(AccountDB* self);

	/// if the option is supported, adjusts the internal state
	bool (*configure)(AccountDB* self, const char* option, const char* value);

	/// creates a new account and saves the provided data
	/// if acc->account_id is -1, the account id will be auto-generated
	bool (*create)(AccountDB* self, const struct mmo_account* acc);

	/// deletes an existing account
	bool (*remove)(AccountDB* self, const int account_id);

	/// replaces data of an existing account
	bool (*save)(AccountDB* self, const struct mmo_account* acc);

	/// looks up account data according to account id and stores it in the provided structure
	bool (*load_num)(AccountDB* self, struct mmo_account* acc, const int account_id);

	/// looks up account data according to account name and stores it in the provided structure
	bool (*load_str)(AccountDB* self, struct mmo_account* acc, const char* userid);
};

struct mmo_account {

	int account_id;
	char userid[24];
	char pass[32+1];        // 23+1 for plaintext, 32+1 for md5-ed passwords
	char sex;               // gender (M/F/S)
	char email[40];         // e-mail (by default: a@a.com)
	int level;              // GM level
	uint32 state;           // packet 0x006a value + 1 (0: compte OK)
	time_t unban_time;      // (timestamp): ban time limit of the account (0 = no ban)
	time_t expiration_time; // (timestamp): validity limit of the account (0 = unlimited)
	int logincount;         // number of successful auth attempts
	char lastlogin[24];     // date+time of last successful login
	char last_ip[16];       // save of last IP of connection
	int account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM]; // account script variables (stored on login server)
};


#endif // __ACCOUNT_H_INCLUDED__
