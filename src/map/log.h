#ifndef _LOG_H_
#define _LOG_H_

#if defined(WITH_MYSQL)

extern char db_server_logdb[32];

#endif

// predeclaration
struct map_session_data;

int log_branch(struct map_session_data &sd);
int log_drop(struct map_session_data &sd, uint32 monster_id, int log_drop[]);
int log_mvpdrop(struct map_session_data &sd, uint32 monster_id, int log_mvp[]);
int log_present(struct map_session_data &sd, int source_type, unsigned short nameid);
int log_produce(struct map_session_data &sd, unsigned short nameid, int slot1, int slot2, int slot3, int success);
int log_refine(struct map_session_data &sd, int n, int success);
int log_trade(struct map_session_data &sd,struct map_session_data &target_sd,int n,int amount);
int log_tostorage(struct map_session_data &sd,int n, uint32 guild);
int log_fromstorage(struct map_session_data &sd,int n, uint32 guild);

int log_vend(struct map_session_data &sd,struct map_session_data &vsd,int n,int amount,int zeny);
int log_zeny(struct map_session_data &sd, struct map_session_data &target_sd,int amount);
int log_atcommand(const map_session_data &sd, const char *message, unsigned cmdlvl);
int log_npc(struct map_session_data &sd, const char *message);
int log_chat(const char *type, int type_id, int src_charid, int src_accid, const char *mapname, int x, int y, const char *dst_charname, const char *message);



int log_final(void);
int log_init(const char *cfgName);



struct LogConfig
{
	bool enable_logs;
	bool sql_logs;
	int rare_items_log;
	int refine_items_log;
	int price_items_log;
	int amount_items_log;
	int branch;
	int drop;
	int mvpdrop;
	int present;
	int produce;
	int refine;
	int trade;
	int vend;
	int zeny;
	unsigned char gmlevel;
	int npc;
	int storage;
	int chat;
	char log_branch[32];
	char log_drop[32];
	char log_mvpdrop[32];
	char log_present[32];
	char log_produce[32];
	char log_refine[32];
	char log_trade[32];
	char log_vend[32];
	char log_gm[32];
	char log_npc[32];
	char log_storage[32];
	char log_chat[32];
	char log_branch_db[32];
	char log_drop_db[32];
	char log_mvpdrop_db[32];
	char log_present_db[32];
	char log_produce_db[32];
	char log_refine_db[32];
	char log_trade_db[32];
	char log_vend_db[32];
	char log_gm_db[32];
	char log_npc_db[32];
	char log_chat_db[32];
	char log_uptime[32];
};

extern struct LogConfig log_config;

#endif
