// $Id: intif.h,v 1.2 2004/09/25 05:32:18 MouseJstr Exp $
#ifndef _INTIF_H_
#define _INFIF_H_

int intif_parse(int fd);

int intif_GMmessage(const char* mes,int flag);

int intif_wis_message(struct map_session_data &sd, const char *nick, const char *mes, size_t len);
int intif_wis_message_to_gm(const char *Wisp_name, int min_gm_level, const char *mes);

int intif_saveaccountreg(struct map_session_data &sd);
int intif_request_accountreg(struct map_session_data &sd);

int intif_request_storage(unsigned long account_id);
int intif_send_storage(struct pc_storage &stor);
int intif_request_guild_storage(unsigned long account_id, unsigned long guild_id);
int intif_send_guild_storage(unsigned long account_id, struct guild_storage &gstor);

int intif_create_party(struct map_session_data &sd,const char *name,int item,int item2);
int intif_request_partyinfo(unsigned long party_id);
int intif_party_addmember(unsigned long party_id, unsigned long account_id);
int intif_party_changeoption(unsigned long party_id, unsigned long account_id, unsigned short expshare,unsigned short itemshare);
int intif_party_leave(unsigned long party_id, unsigned long accound_id);
int intif_party_changemap(struct map_session_data *sd, int online);
int intif_break_party(unsigned long party_id);
int intif_party_message(unsigned long party_id, unsigned long account_id, const char *mes,size_t len);
int intif_party_checkconflict(unsigned long party_id, unsigned long account_id, char *nick);


int intif_guild_create(const char *name, const struct guild_member &master);
int intif_guild_request_info(unsigned long guild_id);
int intif_guild_addmember(unsigned long guild_id, struct guild_member &m);
int intif_guild_leave(unsigned long guild_id, unsigned long account_id, unsigned long char_id, int flag, const char *mes);
int intif_guild_memberinfoshort(unsigned long guild_id, unsigned long account_id, unsigned long char_id, int online, int lv, int class_);
int intif_guild_break(unsigned long guild_id);
int intif_guild_message(unsigned long guild_id, unsigned long account_id, const char *mes, size_t len);
int intif_guild_checkconflict(unsigned long guild_id, unsigned long account_id, unsigned long char_id);
int intif_guild_change_basicinfo(unsigned long guild_id,int type, unsigned long data);
int intif_guild_change_memberinfo(unsigned long guild_id, unsigned long account_id, unsigned long char_id, unsigned short type, unsigned long data);
int intif_guild_position(unsigned long guild_id, unsigned long idx, struct guild_position &p);
int intif_guild_skillup(unsigned long guild_id, unsigned short skillid, unsigned long account_id, int flag);
int intif_guild_alliance(unsigned long guild_id1, unsigned long guild_id2, unsigned long account_id1, unsigned long account_id2, int flag);
int intif_guild_notice(unsigned long guild_id, const char *mes1, const char *mes2);
int intif_guild_emblem(unsigned long guild_id, const unsigned char *data, size_t len);
int intif_guild_castle_dataload(unsigned short castle_id, int index);
int intif_guild_castle_datasave(unsigned short castle_id, int index, int value);

int intif_create_pet(unsigned long account_id, unsigned long char_id, short pet_type, short pet_lv, short pet_egg_id,
                     short pet_equip, short intimate, short hungry, char rename_flag, char incuvate, const char *pet_name);
int intif_request_petdata(unsigned long account_id, unsigned long char_id, unsigned long pet_id);
int intif_save_petdata(unsigned long account_id, struct s_pet &pet);
int intif_delete_petdata(unsigned long pet_id);

#endif
