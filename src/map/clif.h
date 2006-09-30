// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CLIF_H_
#define _CLIF_H_

#include "socket.h"
#include "map.h"
#include "chat.h"

bool clif_packetsend(int fd, map_session_data &sd, unsigned short cmd, int info[], size_t sz);

basics::netaddress& getcharaddress();
basics::ipset& getmapaddress();

void clif_ban_player(const map_session_data &sd, uint32 banoption, const char* reason="");
int clif_authok(map_session_data &sd);
int clif_authfail(map_session_data &sd, uint32 type);
int clif_charselectok(uint32 id);
int clif_dropflooritem(flooritem_data &fitem);
int clif_clearflooritem(flooritem_data &fitem, int fd=0);
int clif_clearchar(block_list &bl, unsigned char type);
int clif_clearchar(map_session_data &sd, block_list &bl); // self
#define clif_clearchar_area(bl, type)	clif_clearchar(bl,type)
int clif_clearchar_delay(unsigned long tick, block_list &bl, int type);
int clif_clearchar_id(const map_session_data &sd, uint32 id, unsigned char type);

bool clif_spawn(block_list& bl);
int clif_spawnpc(map_session_data &sd);	//area
int clif_spawnnpc(npc_data &nd);	// area
int clif_spawnnpc(map_session_data &sd, npc_data &nd);	// self
int clif_spawnmob(mob_data &md);	// area
int clif_spawnpet(pet_data &pd);	// area


int clif_fixpos(const block_list &bl);
int clif_fixobject(const block_list &bl);
int clif_moveobject(const block_list &bl);

int clif_changemap(map_session_data &sd, const char *mapname, unsigned short x, unsigned short y);	//self
int clif_changemapserver(map_session_data &sd, const char *mapname, unsigned short x, unsigned short y, basics::ipaddress ip, unsigned short port);	//self


int clif_npcbuysell(map_session_data &sd, uint32 id);	//self
int clif_buylist(map_session_data&, npc_data&);	//self
int clif_selllist(map_session_data&);	//self
int clif_scriptmes(map_session_data &sd, uint32 npcid, const char *mes);	//self
int clif_scriptnext(map_session_data &sd,uint32 npcid);	//self
int clif_scriptclose(map_session_data &sd, uint32 npcid);	//self
int clif_scriptmenu(map_session_data &sd, uint32 npcid, const char *mes);	//self
int clif_scriptinput(map_session_data &sd, uint32 npcid);	//self
int clif_scriptinputstr(map_session_data &sd,uint32 npcid);	// self
int clif_cutin(map_session_data &sd, const char *image, unsigned char type);	//self
int clif_viewpoint(map_session_data &sd, uint32 npc_id, uint32 id, uint32 x, uint32 y, unsigned char type, uint32 color);	//self
int clif_additem(map_session_data &sd, unsigned short n, unsigned short amount, unsigned char fail);	//self
int clif_delitem(map_session_data &sd,unsigned short n,unsigned short amount);	//self
int clif_updatestatus(map_session_data &sd,unsigned short type);	//self
int clif_changestatus(block_list &bl,unsigned short type,uint32 val);	//area
int clif_damage(block_list &src,block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,unsigned short damage,unsigned short div,unsigned char type,unsigned short damage2);	// area
#define clif_takeitem(src,dst) clif_damage(src,dst,0,0,0,0,0,1,0)
int clif_changelook(const block_list &bl,unsigned char type, unsigned short val);	// area
int clif_arrowequip(map_session_data &sd,unsigned short val); //self
int clif_arrow_fail(map_session_data &sd,unsigned short type); //self
int clif_arrow_create_list(map_session_data &sd);	//self
int clif_statusupack(map_session_data &sd,unsigned short type,unsigned char ok,unsigned char val);	// self
int clif_equipitemack(map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok);	// self
int clif_unequipitemack(map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok);	// self
int clif_misceffect(block_list &bl, uint32 type);	// area
int clif_setareaeffect(const block_list &bl, uint32 effect);
int clif_changeoption(block_list&bl);	// area
int clif_useitemack(map_session_data &sd,unsigned short index,unsigned short amount,unsigned char ok);	// self
int clif_GlobalMessage(block_list &bl,const char *message, size_t len);
int clif_createchat(map_session_data &sd,unsigned char fail);	// self
int clif_dispchat(chat_data &cd,int fd);	// area or fd
int clif_joinchatfail(map_session_data &sd,unsigned char fail);	// self
int clif_joinchatok(map_session_data&, chat_data&);	// self
int clif_addchat(chat_data&,map_session_data&);	// chat
int clif_changechatowner(chat_data&,map_session_data&);	// chat
int clif_clearchat(chat_data &cd,int fd);	// area or fd
int clif_leavechat(chat_data&,map_session_data&);	// chat
int clif_changechatstatus(chat_data&);	// chat
int clif_refresh(map_session_data&);	// self
int clif_charnameack(int fd, block_list &bl, bool clear=false);

int clif_pk_fame(map_session_data &sd, const uint32 total, int delta);
int clif_blacksmith_fame(map_session_data &sd, const uint32 total, int delta);
int clif_alchemist_fame(map_session_data &sd, const uint32 total, int delta);
int clif_taekwon_fame(const map_session_data &sd, const uint32 total, int delta);


int clif_mission_mob(map_session_data &sd, unsigned short mob_id, unsigned short progress);
int clif_hate_mob(map_session_data &sd, unsigned short skilllv, unsigned short  mob_id);
int clif_feel_info(map_session_data &sd, int feel_level);


int clif_emotion(block_list &bl,unsigned char type);
int clif_talkiebox(block_list &bl,const char* talkie);
int clif_wedding_effect(block_list &bl);
int clif_divorced(map_session_data &sd, const char *name);

int clif_adopt_process(map_session_data &sd);
int clif_sitting(map_session_data &sd);
int clif_soundeffect(map_session_data &sd,block_list &bl,const char *name,unsigned char type);
int clif_soundeffectall(block_list &bl, const char *name, unsigned char type);

// trade
int clif_traderequest(map_session_data &sd,const char *name);
int clif_tradestart(map_session_data &sd,unsigned char type);
int clif_tradeadditem(map_session_data &sd,map_session_data &tsd,unsigned short index,uint32 amount);
int clif_tradeitemok(map_session_data &sd,unsigned short index,unsigned char fail);
int clif_tradedeal_lock(map_session_data &sd,unsigned char fail);
int clif_tradecancelled(map_session_data &sd);
int clif_tradecompleted(map_session_data &sd,unsigned char fail);

// storage
#include "storage.h"
int clif_storageitemlist(map_session_data &sd,struct pc_storage &stor);
int clif_storageequiplist(map_session_data &sd,struct pc_storage &stor);
int clif_updatestorageamount(map_session_data &sd,struct pc_storage &stor);
int clif_storageitemadded(map_session_data &sd,struct pc_storage &stor,unsigned short index,uint32 amount);
int clif_storageitemremoved(map_session_data &sd,unsigned short index,uint32 amount);
int clif_storageclose(map_session_data &sd);
int clif_guildstorageitemlist(map_session_data &sd,struct guild_storage &stor);
int clif_guildstorageequiplist(map_session_data &sd,struct guild_storage &stor);
int clif_updateguildstorageamount(map_session_data &sd,struct guild_storage &stor);
int clif_guildstorageitemadded(map_session_data &sd,struct guild_storage &stor,unsigned short index,uint32 amount);


class CClifInsight : public CMapProcessor
{
	block_list &tbl;
	map_session_data *tsd;
public:
	CClifInsight(block_list &b):tbl(b), tsd(b.get_sd())	{}
	~CClifInsight()	{}
	virtual int process(block_list& bl) const;
};
class CClifOutsight : public CMapProcessor
{
	block_list &tbl;
	map_session_data *tsd;
public:
	CClifOutsight(block_list &b):tbl(b), tsd(b.get_sd())	{}
	~CClifOutsight()	{}
	virtual int process(block_list& bl) const;
};



int clif_class_change(const block_list &bl,unsigned short class_,unsigned char type);
int clif_mob_class_change(const mob_data &md);
int clif_mob_equip(const mob_data &md,unsigned short nameid); // [Valaris]

int clif_skillinfo(map_session_data &sd,unsigned short skillid, short type,short range);
int clif_skillinfoblock(map_session_data &sd);
int clif_skillup(map_session_data &sd,unsigned short skill_num);

int clif_skillcasting(block_list &bl,uint32 src_id,uint32 dst_id,unsigned short dst_x,unsigned short dst_y,unsigned short skill_id,uint32 casttime);
int clif_skillcastcancel(block_list &bl);
int clif_skill_damage(block_list &src,block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type);
int clif_skill_damage2(block_list &src,block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type);
int clif_skill_nodamage(block_list &src,block_list &dst,unsigned short skill_id,unsigned short heal,unsigned char fail);
int clif_skill_poseffect(block_list &src,unsigned short skill_id,unsigned short val,unsigned short x,unsigned short y,unsigned long tick);
int clif_skill_estimation(const map_session_data &sd, const mob_data &md);
int clif_skill_warppoint(map_session_data &sd,unsigned short skill_id,const char *map1,const char *map2,const char *map3,const char *map4);
int clif_skill_memo(map_session_data &sd,unsigned char flag);
int clif_skill_teleportmessage(map_session_data &sd,unsigned short flag);
int clif_skill_produce_mix_list(map_session_data &sd,int trigger);

int clif_produceeffect(map_session_data &sd,unsigned short nameid, unsigned short flag);
int clif_repaireffect(map_session_data &sd, unsigned short nameid, unsigned char flag);
int clif_skill_setunit(struct skill_unit &unit);
int clif_skill_delunit(struct skill_unit &unit);

int clif_01ac(block_list &bl);

int clif_autospell(map_session_data &sd,unsigned short skilllv);
int clif_devotion(map_session_data &sd,uint32 target_id);
int clif_marionette(block_list &src, block_list *target);
int clif_spiritball(map_session_data &sd);
int clif_combo_delay(block_list &src,uint32 wait);
int clif_bladestop(block_list &src,block_list &dst,uint32 bool_);
int clif_changemapcell(unsigned short m,unsigned short x,unsigned short y,unsigned short cell_type,int type);

int clif_status_change(block_list &bl,unsigned short type,unsigned char flag);

int clif_wis_message(int fd,const char *nick,const char *mes,size_t mes_len);
int clif_wis_end(int fd, unsigned short flag);

int clif_solved_charname(const map_session_data &sd, uint32 char_id, const char*name);
int clif_update_mobhp(mob_data &md);

int clif_use_card(map_session_data &sd, unsigned short idx);
int clif_insert_card(map_session_data &sd,unsigned short idx_equip,unsigned short idx_card,unsigned char flag);

int clif_itemlist(map_session_data &sd);
int clif_equiplist(map_session_data &sd);

int clif_cart_additem(map_session_data &sd,unsigned short n,uint32 amount,unsigned char fail);
int clif_cart_delitem(map_session_data &sd,unsigned short n,uint32 amount);
int clif_cart_itemlist(map_session_data &sd);
int clif_cart_equiplist(map_session_data &sd);

int clif_item_identify_list(map_session_data &sd);
int clif_item_identified(map_session_data &sd,unsigned short idx,unsigned char flag);
int clif_item_repair_list(map_session_data &sd);
int clif_item_refine_list(map_session_data &sd);

int clif_item_skill(map_session_data &sd,unsigned short skillid,unsigned short skilllv,const char *name);

int clif_mvp_effect(map_session_data &sd);
int clif_mvp_item(map_session_data &sd,unsigned short nameid);
int clif_mvp_exp(map_session_data &sd, uint32 exp);
int clif_changed_dir(block_list &bl);

// vending
int clif_openvendingreq(map_session_data &sd,unsigned short num);
int clif_showvendingboard(block_list &bl,const char *message,int fd);
int clif_closevendingboard(block_list &bl,int fd);
int clif_vendinglist(map_session_data &sd,uint32 id, vending_element vend_list[]);
int clif_buyvending(map_session_data &sd,unsigned short index,unsigned short amount,unsigned char fail);
int clif_openvending(map_session_data &sd,uint32 id, vending_element vend_list[]);
int clif_vendingreport(map_session_data &sd,unsigned short index,unsigned short amount);

int clif_movetoattack(map_session_data &sd,block_list &bl);

// party
int clif_party_created(map_session_data &sd,unsigned char flag);
int clif_party_info(struct party &p,int fd);
int clif_party_invite(map_session_data &sd,map_session_data &tsd);
int clif_party_inviteack(map_session_data &sd,const char *nick,unsigned char flag);
int clif_party_option(struct party &p,map_session_data *sd,int flag);
int clif_party_leaved(struct party &p,map_session_data *sd,uint32 account_id,const char *name,unsigned char flag);
int clif_party_message(struct party &p,uint32 account_id,const char *mes,size_t len);
int clif_party_move(struct party &p,map_session_data &sd,unsigned char online);
int clif_party_xy(struct party &p,map_session_data &sd);
int clif_party_xy_remove(map_session_data &sd);
int clif_party_hp(struct party &p,map_session_data &sd);
int clif_hpmeter(map_session_data &sd);

// guild
int clif_guild_created(map_session_data &sd,unsigned char flag);
int clif_guild_belonginfo(map_session_data &sd,struct guild &g);
int clif_guild_basicinfo(map_session_data &sd);
int clif_guild_allianceinfo(map_session_data &sd);
int clif_guild_memberlist(map_session_data &sd);
int clif_guild_skillinfo(map_session_data &sd);
int clif_guild_memberlogin_notice(struct guild &g,uint32 idx,uint32 flag);
int clif_guild_invite(map_session_data &sd,struct guild &g);
int clif_guild_inviteack(map_session_data &sd,unsigned char flag);
int clif_guild_leave(map_session_data &sd,const char *name,const char *mes);
int clif_guild_explusion(map_session_data &sd,const char *name,const char *mes,uint32 account_id);
int clif_guild_positionchanged(struct guild &g,uint32 idx);
int clif_guild_memberpositionchanged(struct guild &g,uint32 idx);
int clif_guild_emblem(map_session_data &sd,struct guild &g);
int clif_guild_notice(map_session_data &sd,struct guild &g);
int clif_guild_message(struct guild &g,uint32 account_id,const char *mes,size_t len);
int clif_guild_skillup(map_session_data &sd,unsigned short skillid,unsigned short skilllv);
int clif_guild_reqalliance(map_session_data &sd,uint32 account_id,const char *name);
int clif_guild_allianceack(map_session_data &sd,uint32 flag);
int clif_guild_delalliance(map_session_data &sd,uint32 guild_id,uint32 flag);
int clif_guild_oppositionack(map_session_data &sd,unsigned char flag);
int clif_guild_broken(map_session_data &sd,uint32 flag);
int clif_guild_xy(map_session_data &sd);
int clif_guild_xy_remove(map_session_data &sd);


// atcommand
int clif_displaymessage(int fd,const char* mes);
int clif_disp_onlyself(map_session_data &sd,const char *mes);
int clif_GMmessage(block_list *bl,const char* mes, size_t len, int flag);
int clif_heal(int fd,unsigned short type,unsigned short val);
int clif_resurrection(block_list &bl,unsigned short type);
int clif_set0199(int fd,unsigned short type);
int clif_pvpset(map_session_data &sd,uint32 pvprank,uint32 pvpnum,int type);
int clif_send0199(unsigned short map,unsigned short type);
int clif_refine(int fd,map_session_data &sd,unsigned short fail,unsigned short index,unsigned short val);

//petsystem
int clif_catch_process(map_session_data &sd);
int clif_pet_rulet(map_session_data &sd,unsigned char data);
int clif_sendegg(map_session_data &sd);
int clif_send_petdata(map_session_data &sd,unsigned char type,uint32 param);
int clif_send_petstatus(map_session_data &sd);
int clif_pet_emotion(struct pet_data &pd,uint32 param);
int clif_pet_performance(block_list &bl,uint32 param);
int clif_pet_equip(struct pet_data &pd);
int clif_pet_food(map_session_data &sd,unsigned short foodid,unsigned char fail);

//homun
int clif_send_homdata(const map_session_data &sd, unsigned short type, uint32 param);
int clif_send_homstatus(const map_session_data &sd, int flag);
int clif_spawnhom(const homun_data &hd);
int clif_movehom(const homun_data &hd);
int clif_hom_food(const map_session_data &sd, unsigned short foodid, int fail);
int clif_homskillinfoblock(const map_session_data &sd);
int clif_homskillup(const map_session_data &sd, unsigned short skill_num);

//friends list
int clif_friend_send_info(map_session_data &sd);
int clif_friend_add_request(map_session_data &sd, map_session_data &from_sd);
int clif_friend_add_ack(map_session_data &sd, uint32 friend_account_id, uint32 friend_char_id, const char* name, unsigned short flag);



// mail system
int clif_openmailbox(map_session_data &sd);
int clif_send_mailbox(map_session_data &sd, uint32 count, const unsigned char* buffer);
int clif_res_sendmail(map_session_data &sd, bool ok);
int clif_res_sendmail_setappend(map_session_data &sd, int flag);
int clif_arrive_newmail(map_session_data &sd, const CMailHead& mh);
int clif_receive_mail(map_session_data &sd, const CMail& md);
int clif_deletemail_res(map_session_data &sd, uint32 msgid, bool ok);


int clif_seteffect(const map_session_data& sd, uint32 id, uint32 effect);
int clif_updateweather(unsigned short m);

int clif_specialeffect(const block_list &bl, uint32 type, int flag); // special effects [Valaris]
int clif_message(block_list &bl, const char* msg); // messages (from mobs/npcs) [Valaris]

int clif_GM_kickack(map_session_data &sd,uint32 id);
int clif_GM_kick(map_session_data &sd,map_session_data &tsd,int type);
int clif_GM_silence(map_session_data &sd,map_session_data &tsd,int type);
int clif_timedout(map_session_data &sd);


class CClifProcessor : public basics::noncopyable
{
public:
	CClifProcessor()			{}
	virtual ~CClifProcessor()	{}
	virtual bool process(map_session_data& sd) const=0;
};
int clif_foreachclient(const CClifProcessor& elem);
int clif_disp_overhead(map_session_data &sd, const char* mes);


int clif_terminate(int fd);
int check_connect_map_port();
int do_final_clif(void);
int do_init_clif(void);


#endif


