// $Id: clif.h 1952 2004-10-23 14:05:01Z Yor $
#ifndef _CLIF_H_
#define _CLIF_H_

#include "socket.h"
#include "map.h"
#include "chat.h"

bool clif_packetsend(int fd, struct map_session_data &sd, unsigned short cmd, int info[], size_t sz);

basics::netaddress& getcharaddress();
basics::ipset& getmapaddress();

int clif_countusers(void);

int clif_authok(struct map_session_data &sd);
int clif_authfail(struct map_session_data &sd, uint32 type);
int clif_charselectok(uint32 id);
int clif_dropflooritem(flooritem_data &fitem);
int clif_clearflooritem(flooritem_data &fitem, int fd=0);
int clif_clearchar(struct block_list &bl, unsigned char type);
int clif_clearchar(struct map_session_data &sd, struct block_list &bl); // self
#define clif_clearchar_area(bl, type)	clif_clearchar(bl,type)
int clif_clearchar_delay(unsigned long tick, struct block_list &bl, int type);
int clif_clearchar_id(const map_session_data &sd, uint32 id, unsigned char type);

bool clif_spawn(block_list& bl);
int clif_spawnpc(struct map_session_data &sd);	//area
int clif_spawnnpc(struct npc_data &nd);	// area
int clif_spawnnpc(struct map_session_data &sd, struct npc_data &nd);	// self
int clif_spawnmob(struct mob_data &md);	// area
int clif_spawnpet(struct pet_data &pd);	// area


int clif_fixpos(const block_list &bl);
int clif_fixobject(const block_list &bl);
int clif_moveobject(const block_list &bl);

int clif_changemap(struct map_session_data &sd, const char *mapname, unsigned short x, unsigned short y);	//self
int clif_changemapserver(struct map_session_data &sd, const char *mapname, unsigned short x, unsigned short y, basics::ipaddress ip, unsigned short port);	//self


int clif_npcbuysell(struct map_session_data &sd, uint32 id);	//self
int clif_buylist(struct map_session_data&,struct npc_data&);	//self
int clif_selllist(struct map_session_data&);	//self
int clif_scriptmes(struct map_session_data &sd, uint32 npcid, const char *mes);	//self
int clif_scriptnext(struct map_session_data &sd,uint32 npcid);	//self
int clif_scriptclose(struct map_session_data &sd, uint32 npcid);	//self
int clif_scriptmenu(struct map_session_data &sd, uint32 npcid, const char *mes);	//self
int clif_scriptinput(struct map_session_data &sd, uint32 npcid);	//self
int clif_scriptinputstr(struct map_session_data &sd,uint32 npcid);	// self
int clif_cutin(struct map_session_data &sd, const char *image, unsigned char type);	//self
int clif_viewpoint(struct map_session_data &sd, uint32 npc_id, uint32 id, uint32 x, uint32 y, unsigned char type, uint32 color);	//self
int clif_additem(struct map_session_data &sd, unsigned short n, unsigned short amount, unsigned char fail);	//self
int clif_delitem(struct map_session_data &sd,unsigned short n,unsigned short amount);	//self
int clif_updatestatus(struct map_session_data &sd,unsigned short type);	//self
int clif_changestatus(struct block_list &bl,unsigned short type,uint32 val);	//area
int clif_damage(struct block_list &src,struct block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,unsigned short damage,unsigned short div,unsigned char type,unsigned short damage2);	// area
#define clif_takeitem(src,dst) clif_damage(src,dst,0,0,0,0,0,1,0)
int clif_changelook(const block_list &bl,unsigned char type, unsigned short val);	// area
int clif_arrowequip(struct map_session_data &sd,unsigned short val); //self
int clif_arrow_fail(struct map_session_data &sd,unsigned short type); //self
int clif_arrow_create_list(struct map_session_data &sd);	//self
int clif_statusupack(struct map_session_data &sd,unsigned short type,unsigned char ok,unsigned char val);	// self
int clif_equipitemack(struct map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok);	// self
int clif_unequipitemack(struct map_session_data &sd,unsigned short n,unsigned short pos,unsigned char ok);	// self
int clif_misceffect(struct block_list &bl, uint32 type);	// area
int clif_misceffect2(struct block_list &bl, uint32 type);
int clif_changeoption(struct block_list&bl);	// area
int clif_useitemack(struct map_session_data &sd,unsigned short index,unsigned short amount,unsigned char ok);	// self
int clif_GlobalMessage(struct block_list &bl,const char *message, size_t len);
int clif_createchat(struct map_session_data &sd,unsigned char fail);	// self
int clif_dispchat(chat_data &cd,int fd);	// area or fd
int clif_joinchatfail(struct map_session_data &sd,unsigned char fail);	// self
int clif_joinchatok(struct map_session_data&, chat_data&);	// self
int clif_addchat(chat_data&,struct map_session_data&);	// chat
int clif_changechatowner(chat_data&,struct map_session_data&);	// chat
int clif_clearchat(chat_data &cd,int fd);	// area or fd
int clif_leavechat(chat_data&,struct map_session_data&);	// chat
int clif_changechatstatus(chat_data&);	// chat
int clif_refresh(struct map_session_data&);	// self
int clif_charnameack(int fd, struct block_list &bl, bool clear=false);

int clif_pk_fame(struct map_session_data &sd, const uint32 total, int delta);
int clif_blacksmith_fame(struct map_session_data &sd, const uint32 total, int delta);
int clif_alchemist_fame(struct map_session_data &sd, const uint32 total, int delta);
int clif_taekwon_fame(const struct map_session_data &sd, const uint32 total, int delta);


int clif_mission_mob(struct map_session_data &sd, unsigned short mob_id, unsigned short progress);
int clif_hate_mob(struct map_session_data &sd, unsigned short skilllv, unsigned short  mob_id);
int clif_feel_info(struct map_session_data &sd, int feel_level);


int clif_emotion(struct block_list &bl,unsigned char type);
int clif_talkiebox(struct block_list &bl,const char* talkie);
int clif_wedding_effect(struct block_list &bl);
int clif_divorced(struct map_session_data &sd, const char *name);

int clif_adopt_process(struct map_session_data &sd);
int clif_sitting(struct map_session_data &sd);
int clif_soundeffect(struct map_session_data &sd,struct block_list &bl,const char *name,unsigned char type);
int clif_soundeffectall(struct block_list &bl, const char *name, unsigned char type);

// trade
int clif_traderequest(struct map_session_data &sd,const char *name);
int clif_tradestart(struct map_session_data &sd,unsigned char type);
int clif_tradeadditem(struct map_session_data &sd,struct map_session_data &tsd,unsigned short index,uint32 amount);
int clif_tradeitemok(struct map_session_data &sd,unsigned short index,unsigned char fail);
int clif_tradedeal_lock(struct map_session_data &sd,unsigned char fail);
int clif_tradecancelled(struct map_session_data &sd);
int clif_tradecompleted(struct map_session_data &sd,unsigned char fail);

// storage
#include "storage.h"
int clif_storageitemlist(struct map_session_data &sd,struct pc_storage &stor);
int clif_storageequiplist(struct map_session_data &sd,struct pc_storage &stor);
int clif_updatestorageamount(struct map_session_data &sd,struct pc_storage &stor);
int clif_storageitemadded(struct map_session_data &sd,struct pc_storage &stor,unsigned short index,uint32 amount);
int clif_storageitemremoved(struct map_session_data &sd,unsigned short index,uint32 amount);
int clif_storageclose(struct map_session_data &sd);
int clif_guildstorageitemlist(struct map_session_data &sd,struct guild_storage &stor);
int clif_guildstorageequiplist(struct map_session_data &sd,struct guild_storage &stor);
int clif_updateguildstorageamount(struct map_session_data &sd,struct guild_storage &stor);
int clif_guildstorageitemadded(struct map_session_data &sd,struct guild_storage &stor,unsigned short index,uint32 amount);

/*
int clif_pcinsight(struct block_list &bl,va_list &ap);	// map_forallinmovearea callback
int clif_pcoutsight(struct block_list &bl,va_list &ap);	// map_forallinmovearea callback
int clif_mobinsight(struct block_list &bl,va_list &ap);	// map_forallinmovearea callback
int clif_moboutsight(struct block_list &bl,va_list &ap);	// map_forallinmovearea callback
int clif_petoutsight(struct block_list &bl,va_list &ap);
int clif_petinsight(struct block_list &bl,va_list &ap);
int clif_npcoutsight(struct block_list &bl,va_list &ap);
int clif_npcinsight(struct block_list &bl,va_list &ap);
*/
class CClifPCInsight : public CMapProcessor
{
	struct map_session_data &sd;
public:
	CClifPCInsight(struct map_session_data &s):sd(s)	{}
	~CClifPCInsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifPCOutsight : public CMapProcessor
{
	struct map_session_data &sd;
public:
	CClifPCOutsight(struct map_session_data &s):sd(s)	{}
	~CClifPCOutsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifMobInsight : public CMapProcessor
{
	struct mob_data &md;
public:
	CClifMobInsight(struct mob_data &m):md(m)	{}
	~CClifMobInsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifMobOutsight : public CMapProcessor
{
	struct mob_data &md;
public:
	CClifMobOutsight(struct mob_data &m):md(m)	{}
	~CClifMobOutsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifPetInsight : public CMapProcessor
{
	struct pet_data &pd;
public:
	CClifPetInsight(struct pet_data &p):pd(p)	{}
	~CClifPetInsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifPetOutsight : public CMapProcessor
{
	struct pet_data &pd;
public:
	CClifPetOutsight(struct pet_data &p):pd(p)	{}
	~CClifPetOutsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifNpcInsight : public CMapProcessor
{
	struct npc_data &nd;
public:
	CClifNpcInsight(struct npc_data &n):nd(n)	{}
	~CClifNpcInsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifNpcOutsight : public CMapProcessor
{
	struct npc_data &nd;
public:
	CClifNpcOutsight(struct npc_data &n):nd(n)	{}
	~CClifNpcOutsight()	{}
	virtual int process(struct block_list& bl) const;
};



class CClifInsight : public CMapProcessor
{
	struct block_list &tbl;
	map_session_data *tsd;
public:
	CClifInsight(struct block_list &b):tbl(b), tsd(b.get_sd())	{}
	~CClifInsight()	{}
	virtual int process(struct block_list& bl) const;
};
class CClifOutsight : public CMapProcessor
{
	struct block_list &tbl;
	map_session_data *tsd;
public:
	CClifOutsight(struct block_list &b):tbl(b), tsd(b.get_sd())	{}
	~CClifOutsight()	{}
	virtual int process(struct block_list& bl) const;
};







int clif_class_change(const block_list &bl,unsigned short class_,unsigned char type);
int clif_mob_class_change(const mob_data &md);
int clif_mob_equip(const mob_data &md,unsigned short nameid); // [Valaris]

int clif_skillinfo(struct map_session_data &sd,unsigned short skillid, short type,short range);
int clif_skillinfoblock(struct map_session_data &sd);
int clif_skillup(struct map_session_data &sd,unsigned short skill_num);

int clif_skillcasting(struct block_list &bl,uint32 src_id,uint32 dst_id,unsigned short dst_x,unsigned short dst_y,unsigned short skill_id,uint32 casttime);
int clif_skillcastcancel(struct block_list &bl);
int clif_skill_fail(struct map_session_data &sd,unsigned short skill_id,unsigned char type,unsigned short btype);
int clif_skill_damage(struct block_list &src,struct block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type);
int clif_skill_damage2(struct block_list &src,struct block_list &dst,unsigned long tick,uint32 sdelay,uint32 ddelay,uint32 damage,unsigned short div,unsigned short skill_id,unsigned short skill_lv,int type);
int clif_skill_nodamage(struct block_list &src,struct block_list &dst,unsigned short skill_id,unsigned short heal,unsigned char fail);
int clif_skill_poseffect(struct block_list &src,unsigned short skill_id,unsigned short val,unsigned short x,unsigned short y,unsigned long tick);
int clif_skill_estimation(struct map_session_data &sd,struct block_list &dst);
int clif_skill_warppoint(struct map_session_data &sd,unsigned short skill_id,const char *map1,const char *map2,const char *map3,const char *map4);
int clif_skill_memo(struct map_session_data &sd,unsigned char flag);
int clif_skill_teleportmessage(struct map_session_data &sd,unsigned short flag);
int clif_skill_produce_mix_list(struct map_session_data &sd,int trigger);

int clif_produceeffect(struct map_session_data &sd,unsigned short nameid, unsigned short flag);
int clif_repaireffect(struct map_session_data &sd, unsigned short nameid, unsigned char flag);
int clif_skill_setunit(struct skill_unit &unit);
int clif_skill_delunit(struct skill_unit &unit);

int clif_01ac(struct block_list &bl);

int clif_autospell(struct map_session_data &sd,unsigned short skilllv);
int clif_devotion(struct map_session_data &sd,uint32 target_id);
int clif_marionette(struct block_list &src, struct block_list *target);
int clif_spiritball(struct map_session_data &sd);
int clif_combo_delay(struct block_list &src,uint32 wait);
int clif_bladestop(struct block_list &src,struct block_list &dst,uint32 bool_);
int clif_changemapcell(unsigned short m,unsigned short x,unsigned short y,unsigned short cell_type,int type);

int clif_status_change(struct block_list &bl,unsigned short type,unsigned char flag);

int clif_wis_message(int fd,const char *nick,const char *mes,size_t mes_len);
int clif_wis_end(int fd, unsigned short flag);

int clif_solved_charname(struct map_session_data &sd, uint32 char_id);
int clif_update_mobhp(struct mob_data &md);

int clif_use_card(struct map_session_data &sd, unsigned short idx);
int clif_insert_card(struct map_session_data &sd,unsigned short idx_equip,unsigned short idx_card,unsigned char flag);

int clif_itemlist(struct map_session_data &sd);
int clif_equiplist(struct map_session_data &sd);

int clif_cart_additem(struct map_session_data &sd,unsigned short n,uint32 amount,unsigned char fail);
int clif_cart_delitem(struct map_session_data &sd,unsigned short n,uint32 amount);
int clif_cart_itemlist(struct map_session_data &sd);
int clif_cart_equiplist(struct map_session_data &sd);

int clif_item_identify_list(struct map_session_data &sd);
int clif_item_identified(struct map_session_data &sd,unsigned short idx,unsigned char flag);
int clif_item_repair_list(struct map_session_data &sd);
int clif_item_refine_list(struct map_session_data &sd);

int clif_item_skill(struct map_session_data &sd,unsigned short skillid,unsigned short skilllv,const char *name);

int clif_mvp_effect(struct map_session_data &sd);
int clif_mvp_item(struct map_session_data &sd,unsigned short nameid);
int clif_mvp_exp(struct map_session_data &sd, uint32 exp);
int clif_changed_dir(struct block_list &bl);

// vending
int clif_openvendingreq(struct map_session_data &sd,unsigned short num);
int clif_showvendingboard(struct block_list &bl,const char *message,int fd);
int clif_closevendingboard(struct block_list &bl,int fd);
int clif_vendinglist(struct map_session_data &sd,uint32 id,struct vending vending[]);
int clif_buyvending(struct map_session_data &sd,unsigned short index,unsigned short amount,unsigned char fail);
int clif_openvending(struct map_session_data &sd,uint32 id,struct vending vending[]);
int clif_vendingreport(struct map_session_data &sd,unsigned short index,unsigned short amount);

int clif_movetoattack(struct map_session_data &sd,struct block_list &bl);

// party
int clif_party_created(struct map_session_data &sd,unsigned char flag);
int clif_party_info(struct party &p,int fd);
int clif_party_invite(struct map_session_data &sd,struct map_session_data &tsd);
int clif_party_inviteack(struct map_session_data &sd,const char *nick,unsigned char flag);
int clif_party_option(struct party &p,struct map_session_data *sd,int flag);
int clif_party_leaved(struct party &p,struct map_session_data *sd,uint32 account_id,const char *name,unsigned char flag);
int clif_party_message(struct party &p,uint32 account_id,const char *mes,size_t len);
int clif_party_move(struct party &p,struct map_session_data &sd,unsigned char online);
int clif_party_xy(struct party &p,struct map_session_data &sd);
int clif_party_xy_remove(struct map_session_data &sd);
int clif_party_hp(struct party &p,struct map_session_data &sd);
int clif_hpmeter(struct map_session_data &sd);

// guild
int clif_guild_created(struct map_session_data &sd,unsigned char flag);
int clif_guild_belonginfo(struct map_session_data &sd,struct guild &g);
int clif_guild_basicinfo(struct map_session_data &sd);
int clif_guild_allianceinfo(struct map_session_data &sd);
int clif_guild_memberlist(struct map_session_data &sd);
int clif_guild_skillinfo(struct map_session_data &sd);
int clif_guild_memberlogin_notice(struct guild &g,uint32 idx,uint32 flag);
int clif_guild_invite(struct map_session_data &sd,struct guild &g);
int clif_guild_inviteack(struct map_session_data &sd,unsigned char flag);
int clif_guild_leave(struct map_session_data &sd,const char *name,const char *mes);
int clif_guild_explusion(struct map_session_data &sd,const char *name,const char *mes,uint32 account_id);
int clif_guild_positionchanged(struct guild &g,uint32 idx);
int clif_guild_memberpositionchanged(struct guild &g,uint32 idx);
int clif_guild_emblem(struct map_session_data &sd,struct guild &g);
int clif_guild_notice(struct map_session_data &sd,struct guild &g);
int clif_guild_message(struct guild &g,uint32 account_id,const char *mes,size_t len);
int clif_guild_skillup(struct map_session_data &sd,unsigned short skillid,unsigned short skilllv);
int clif_guild_reqalliance(struct map_session_data &sd,uint32 account_id,const char *name);
int clif_guild_allianceack(struct map_session_data &sd,uint32 flag);
int clif_guild_delalliance(struct map_session_data &sd,uint32 guild_id,uint32 flag);
int clif_guild_oppositionack(struct map_session_data &sd,unsigned char flag);
int clif_guild_broken(struct map_session_data &sd,uint32 flag);
int clif_guild_xy(struct map_session_data &sd);
int clif_guild_xy_remove(struct map_session_data &sd);


// atcommand
int clif_displaymessage(int fd,const char* mes);
int clif_disp_onlyself(struct map_session_data &sd,const char *mes);
int clif_GMmessage(struct block_list *bl,const char* mes, size_t len, int flag);
int clif_heal(int fd,unsigned short type,unsigned short val);
int clif_resurrection(struct block_list &bl,unsigned short type);
int clif_set0199(int fd,unsigned short type);
int clif_pvpset(struct map_session_data &sd,uint32 pvprank,uint32 pvpnum,int type);
int clif_send0199(unsigned short map,unsigned short type);
int clif_refine(int fd,struct map_session_data &sd,unsigned short fail,unsigned short index,unsigned short val);

//petsystem
int clif_catch_process(struct map_session_data &sd);
int clif_pet_rulet(struct map_session_data &sd,unsigned char data);
int clif_sendegg(struct map_session_data &sd);
int clif_send_petdata(struct map_session_data &sd,unsigned char type,uint32 param);
int clif_send_petstatus(struct map_session_data &sd);
int clif_pet_emotion(struct pet_data &pd,uint32 param);
int clif_pet_performance(struct block_list &bl,uint32 param);
int clif_pet_equip(struct pet_data &pd);
int clif_pet_food(struct map_session_data &sd,unsigned short foodid,unsigned char fail);

//homun
int clif_send_homdata(const map_session_data &sd, unsigned short type, uint32 param);
int clif_send_homstatus(const map_session_data &sd, int flag);
int clif_spawnhom(const homun_data &hd);
int clif_movehom(const homun_data &hd);
int clif_hom_food(const map_session_data &sd, unsigned short foodid, int fail);
int clif_homskillinfoblock(const map_session_data &sd);
int clif_homskillup(const map_session_data &sd, unsigned short skill_num);

//friends list
int clif_friend_send_info(struct map_session_data &sd);
int clif_friend_add_request(struct map_session_data &sd, struct map_session_data &from_sd);
int clif_friend_add_ack(struct map_session_data &sd, uint32 friend_account_id, uint32 friend_char_id, const char* name, unsigned short flag);



// mail system
int clif_openmailbox(struct map_session_data &sd);
int clif_send_mailbox(struct map_session_data &sd, uint32 count, const unsigned char* buffer);
int clif_res_sendmail(struct map_session_data &sd, bool ok);
int clif_res_sendmail_setappend(struct map_session_data &sd, int flag);
int clif_arrive_newmail(struct map_session_data &sd, const CMailHead& mh);
int clif_receive_mail(struct map_session_data &sd, const CMail& md);
int clif_deletemail_res(struct map_session_data &sd, uint32 msgid, bool ok);


int clif_clearweather(unsigned short m);
int clif_weather2(unsigned short m, int type);
int clif_weather1(int fd, int type);

int clif_specialeffect(const block_list &bl, uint32 type, int flag); // special effects [Valaris]
int clif_message(struct block_list &bl, const char* msg); // messages (from mobs/npcs) [Valaris]

int clif_GM_kickack(struct map_session_data &sd,uint32 id);
int clif_GM_kick(struct map_session_data &sd,struct map_session_data &tsd,int type);
int clif_GM_silence(struct map_session_data &sd,struct map_session_data &tsd,int type);
int clif_timedout(struct map_session_data &sd);


class CClifProcessor : public basics::noncopyable
{
public:
	CClifProcessor()			{}
	virtual ~CClifProcessor()	{}
	virtual bool process(struct map_session_data& sd) const=0;
};
int clif_foreachclient(const CClifProcessor& elem);
//int clif_foreachclient(int (*)(struct map_session_data&,va_list &),...);
int clif_disp_overhead(struct map_session_data &sd, const char* mes);


int clif_terminate(int fd);
int check_connect_map_port();
int do_final_clif(void);
int do_init_clif(void);


#endif


