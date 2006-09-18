// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTIF_H_
#define _INFIF_H_

int intif_parse(int fd);

int intif_GMmessage(const char* mes,size_t len, int flag);

int intif_wis_message(map_session_data &sd, const char *nick, const char *mes, size_t len);
int intif_wis_message_to_gm(const char *Wisp_name, int min_gm_level, const char *mes);

int intif_saveaccountreg(map_session_data &sd);
int intif_request_accountreg(map_session_data &sd);

int intif_request_storage(uint32 account_id);
int intif_send_storage(struct pc_storage &stor);
int intif_request_guild_storage(uint32 account_id, uint32 guild_id);
int intif_send_guild_storage(uint32 account_id, struct guild_storage &gstor);

int intif_create_party(map_session_data &sd,const char *name,int item,int item2);
int intif_request_partyinfo(uint32 party_id);
int intif_party_addmember(uint32 party_id, uint32 account_id);
int intif_party_changeoption(uint32 party_id, uint32 account_id, unsigned short expshare,unsigned short itemshare);
int intif_party_leave(uint32 party_id, uint32 accound_id);
int intif_party_changemap(map_session_data *sd, int online);
int intif_break_party(uint32 party_id);
int intif_party_message(uint32 party_id, uint32 account_id, const char *mes,size_t len);
int intif_party_checkconflict(uint32 party_id, uint32 account_id, char *nick);


int intif_guild_create(const char *name, const struct guild_member &master);
int intif_guild_request_info(uint32 guild_id);
int intif_guild_addmember(uint32 guild_id, struct guild_member &m);
int intif_guild_leave(uint32 guild_id, uint32 account_id, uint32 char_id, int flag, const char *mes);
int intif_guild_memberinfoshort(uint32 guild_id, uint32 account_id, uint32 char_id, int online, int lv, int class_);
int intif_guild_break(uint32 guild_id);
int intif_guild_message(uint32 guild_id, uint32 account_id, const char *mes, size_t len);
int intif_guild_checkconflict(uint32 guild_id, uint32 account_id, uint32 char_id);
int intif_guild_change_basicinfo(uint32 guild_id,int type, uint32 data);
int intif_guild_change_memberinfo(uint32 guild_id, uint32 account_id, uint32 char_id, unsigned short type, uint32 data);
int intif_guild_position(uint32 guild_id, uint32 idx, struct guild_position &p);
int intif_guild_skillup(uint32 guild_id, unsigned short skillid, uint32 account_id, int flag);
int intif_guild_alliance(uint32 guild_id1, uint32 guild_id2, uint32 account_id1, uint32 account_id2, int flag);
int intif_guild_notice(uint32 guild_id, const char *mes1, const char *mes2);
int intif_guild_emblem(uint32 guild_id, const unsigned char *data, size_t len);
int intif_guild_castle_dataload(unsigned short castle_id, int index);
int intif_guild_castle_datasave(unsigned short castle_id, int index, int value);

int intif_create_pet(uint32 account_id, uint32 char_id, short pet_type, short pet_lv, short pet_egg_id,
                     short pet_equip, short intimate, short hungry, char rename_flag, char incuvate, const char *pet_name);
int intif_request_petdata(uint32 account_id, uint32 char_id, uint32 pet_id);
int intif_save_petdata(uint32 account_id, struct petstatus &pet);
int intif_delete_petdata(uint32 pet_id);


int intif_create_homdata(uint32 account_id, uint32 char_id, const homun_data &hd);
int intif_request_homdata(uint32 account_id,uint32 char_id,uint32 homun_id);
int intif_save_homdata(uint32 account_id, uint32 char_id, const homun_data &hd);
int intif_delete_homdata(uint32 account_id,uint32 char_id,uint32 homun_id);
#endif
