// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//#define DEBUG_FUNCIN
//#define DEBUG_DISP
//#define DEBUG_RUN

#include "socket.h"
#include "timer.h"
#include "malloc.h"
#include "lock.h"
#include "nullpo.h"
#include "db.h"
#include "socket.h"
#include "showmsg.h"
#include "utils.h"
#include "log.h"


#include "map.h"
#include "clif.h"
#include "chrif.h"
#include "itemdb.h"
#include "pc.h"
#include "status.h"
#include "script.h"
#include "storage.h"
#include "mob.h"
#include "npc.h"
#include "pet.h"
#include "intif.h"
#include "skill.h"
#include "chat.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"



/*==========================================
 * ローカルプロトタイプ宣言 (必要な物のみ)
 *------------------------------------------
 */

int buildin_mes(CScriptEngine &st);
int buildin_goto(CScriptEngine &st);
int buildin_callsub(CScriptEngine &st);
int buildin_callfunc(CScriptEngine &st);
int buildin_return(CScriptEngine &st);
int buildin_getarg(CScriptEngine &st);
int buildin_next(CScriptEngine &st);
int buildin_close(CScriptEngine &st);
int buildin_close2(CScriptEngine &st);
int buildin_menu(CScriptEngine &st);
int buildin_rand(CScriptEngine &st);
int buildin_warp(CScriptEngine &st);
int buildin_areawarp(CScriptEngine &st);
int buildin_heal(CScriptEngine &st);
int buildin_itemheal(CScriptEngine &st);
int buildin_percentheal(CScriptEngine &st);
int buildin_jobchange(CScriptEngine &st);
int buildin_input(CScriptEngine &st);
int buildin_setlook(CScriptEngine &st);
int buildin_set(CScriptEngine &st);
int buildin_setarray(CScriptEngine &st);
int buildin_cleararray(CScriptEngine &st);
int buildin_copyarray(CScriptEngine &st);
int buildin_getarraysize(CScriptEngine &st);
int buildin_deletearray(CScriptEngine &st);
int buildin_getelementofarray(CScriptEngine &st);
int buildin_if(CScriptEngine &st);
int buildin_getitem(CScriptEngine &st);
int buildin_getitem2(CScriptEngine &st);
int buildin_makeitem(CScriptEngine &st);
int buildin_delitem(CScriptEngine &st);
int buildin_viewpoint(CScriptEngine &st);
int buildin_countitem(CScriptEngine &st);
int buildin_checkweight(CScriptEngine &st);
int buildin_readparam(CScriptEngine &st);
int buildin_getcharid(CScriptEngine &st);
int buildin_getpartyname(CScriptEngine &st);
int buildin_getpartymember(CScriptEngine &st);
int buildin_getguildname(CScriptEngine &st);
int buildin_getguildmaster(CScriptEngine &st);
int buildin_getguildmasterid(CScriptEngine &st);
int buildin_strcharinfo(CScriptEngine &st);
int buildin_getequipid(CScriptEngine &st);
int buildin_getequipname(CScriptEngine &st);
int buildin_getbrokenid(CScriptEngine &st); // [Valaris]
int buildin_repair(CScriptEngine &st); // [Valaris]
int buildin_getequipisequiped(CScriptEngine &st);
int buildin_getequipisenableref(CScriptEngine &st);
int buildin_getequipisidentify(CScriptEngine &st);
int buildin_getequiprefinerycnt(CScriptEngine &st);
int buildin_getequipweaponlv(CScriptEngine &st);
int buildin_getequippercentrefinery(CScriptEngine &st);
int buildin_successrefitem(CScriptEngine &st);
int buildin_failedrefitem(CScriptEngine &st);
int buildin_cutin(CScriptEngine &st);
int buildin_cutincard(CScriptEngine &st);
int buildin_statusup(CScriptEngine &st);
int buildin_statusup2(CScriptEngine &st);
int buildin_bonus(CScriptEngine &st);
int buildin_bonus2(CScriptEngine &st);
int buildin_bonus3(CScriptEngine &st);
int buildin_bonus4(CScriptEngine &st);
int buildin_skill(CScriptEngine &st);
int buildin_addtoskill(CScriptEngine &st); // [Valaris]
int buildin_guildskill(CScriptEngine &st);
int buildin_getskilllv(CScriptEngine &st);
int buildin_getgdskilllv(CScriptEngine &st);
int buildin_basicskillcheck(CScriptEngine &st);
int buildin_getgmlevel(CScriptEngine &st);
int buildin_end(CScriptEngine &st);
int buildin_checkoption(CScriptEngine &st);
int buildin_setoption(CScriptEngine &st);
int buildin_setcart(CScriptEngine &st);
int buildin_checkcart(CScriptEngine &st); // check cart [Valaris]
int buildin_setfalcon(CScriptEngine &st);
int buildin_checkfalcon(CScriptEngine &st); // check falcon [Valaris]
int buildin_setriding(CScriptEngine &st);
int buildin_checkriding(CScriptEngine &st); // check for pecopeco [Valaris]
int buildin_savepoint(CScriptEngine &st);
int buildin_gettimetick(CScriptEngine &st);
int buildin_gettime(CScriptEngine &st);
int buildin_gettimestr(CScriptEngine &st);
int buildin_openstorage(CScriptEngine &st);
int buildin_guildopenstorage(CScriptEngine &st);
int buildin_itemskill(CScriptEngine &st);
int buildin_produce(CScriptEngine &st);
int buildin_monster(CScriptEngine &st);
int buildin_areamonster(CScriptEngine &st);
int buildin_killmonster(CScriptEngine &st);
int buildin_killmonsterall(CScriptEngine &st);
int buildin_doevent(CScriptEngine &st);
int buildin_donpcevent(CScriptEngine &st);
int buildin_addtimer(CScriptEngine &st);
int buildin_deltimer(CScriptEngine &st);
int buildin_addtimercount(CScriptEngine &st);
int buildin_initnpctimer(CScriptEngine &st);
int buildin_stopnpctimer(CScriptEngine &st);
int buildin_startnpctimer(CScriptEngine &st);
int buildin_setnpctimer(CScriptEngine &st);
int buildin_getnpctimer(CScriptEngine &st);
int buildin_attachnpctimer(CScriptEngine &st);	// [celest]
int buildin_detachnpctimer(CScriptEngine &st);	// [celest]
int buildin_announce(CScriptEngine &st);
int buildin_mapannounce(CScriptEngine &st);
int buildin_areaannounce(CScriptEngine &st);
int buildin_getusers(CScriptEngine &st);
int buildin_getmapusers(CScriptEngine &st);
int buildin_getareausers(CScriptEngine &st);
int buildin_getareadropitem(CScriptEngine &st);
int buildin_enablenpc(CScriptEngine &st);
int buildin_disablenpc(CScriptEngine &st);
int buildin_enablearena(CScriptEngine &st);	// Added by RoVeRT
int buildin_disablearena(CScriptEngine &st);	// Added by RoVeRT
int buildin_hideoffnpc(CScriptEngine &st);
int buildin_hideonnpc(CScriptEngine &st);
int buildin_sc_start(CScriptEngine &st);
int buildin_sc_start2(CScriptEngine &st);
int buildin_sc_start4(CScriptEngine &st);
int buildin_sc_end(CScriptEngine &st);
int buildin_getscrate(CScriptEngine &st);
int buildin_debugmes(CScriptEngine &st);
int buildin_catchpet(CScriptEngine &st);
int buildin_birthpet(CScriptEngine &st);
int buildin_resetlvl(CScriptEngine &st);
int buildin_resetstatus(CScriptEngine &st);
int buildin_resetskill(CScriptEngine &st);
int buildin_changebase(CScriptEngine &st);
int buildin_changesex(CScriptEngine &st);
int buildin_waitingroom(CScriptEngine &st);
int buildin_delwaitingroom(CScriptEngine &st);
int buildin_enablewaitingroomevent(CScriptEngine &st);
int buildin_disablewaitingroomevent(CScriptEngine &st);
int buildin_getwaitingroomstate(CScriptEngine &st);
int buildin_warpwaitingpc(CScriptEngine &st);
int buildin_attachrid(CScriptEngine &st);
int buildin_detachrid(CScriptEngine &st);
int buildin_isloggedin(CScriptEngine &st);
int buildin_setmapflagnosave(CScriptEngine &st);
int buildin_setmapflag(CScriptEngine &st);
int buildin_removemapflag(CScriptEngine &st);
int buildin_pvpon(CScriptEngine &st);
int buildin_pvpoff(CScriptEngine &st);
int buildin_gvgon(CScriptEngine &st);
int buildin_gvgoff(CScriptEngine &st);
int buildin_emotion(CScriptEngine &st);
int buildin_maprespawnguildid(CScriptEngine &st);
int buildin_agitstart(CScriptEngine &st);		// <Agit>
int buildin_agitend(CScriptEngine &st);
int buildin_agitcheck(CScriptEngine &st);  // <Agitcheck>
int buildin_flagemblem(CScriptEngine &st);		// Flag Emblem
int buildin_getcastlename(CScriptEngine &st);
int buildin_getcastledata(CScriptEngine &st);
int buildin_setcastledata(CScriptEngine &st);
int buildin_requestguildinfo(CScriptEngine &st);
int buildin_getequipcardcnt(CScriptEngine &st);
int buildin_successremovecards(CScriptEngine &st);
int buildin_failedremovecards(CScriptEngine &st);
int buildin_marriage(CScriptEngine &st);
int buildin_wedding_effect(CScriptEngine &st);
int buildin_divorce(CScriptEngine &st);
int buildin_ispartneron(CScriptEngine &st); // MouseJstr
int buildin_getpartnerid(CScriptEngine &st); // MouseJstr
int buildin_getchildid(CScriptEngine &st); // Skotlex
int buildin_warppartner(CScriptEngine &st); // MouseJstr
int buildin_getitemname(CScriptEngine &st);
int buildin_makepet(CScriptEngine &st);
int buildin_getexp(CScriptEngine &st);
int buildin_getinventorylist(CScriptEngine &st);
int buildin_getskilllist(CScriptEngine &st);
int buildin_clearitem(CScriptEngine &st);
int buildin_classchange(CScriptEngine &st);
int buildin_misceffect(CScriptEngine &st);
int buildin_soundeffect(CScriptEngine &st);
int buildin_soundeffectall(CScriptEngine &st);
int buildin_setcastledata(CScriptEngine &st);
int buildin_mapwarp(CScriptEngine &st);
int buildin_inittimer(CScriptEngine &st);
int buildin_stoptimer(CScriptEngine &st);
int buildin_cmdothernpc(CScriptEngine &st);
int buildin_mobcount(CScriptEngine &st);
int buildin_strmobinfo(CScriptEngine &st); // Script for displaying mob info [Valaris]
int buildin_guardian(CScriptEngine &st); // Script for displaying mob info [Valaris]
int buildin_guardianinfo(CScriptEngine &st); // Script for displaying mob info [Valaris]
int buildin_petskillbonus(CScriptEngine &st); // petskillbonus [Valaris]
int buildin_petrecovery(CScriptEngine &st); // pet skill for curing status [Valaris]
int buildin_petloot(CScriptEngine &st); // pet looting [Valaris]
int buildin_petheal(CScriptEngine &st); // pet healing [Valaris]
int buildin_petskillattack(CScriptEngine &st); // pet skill attacks [Skotlex]
int buildin_petskillattack2(CScriptEngine &st); // pet skill attacks [Skotlex]
int buildin_petskillsupport(CScriptEngine &st); // pet support skill [Valaris]
int buildin_skilleffect(CScriptEngine &st); // skill effects [Celest]
int buildin_npcskilleffect(CScriptEngine &st); // skill effects for npcs [Valaris]
int buildin_specialeffect(CScriptEngine &st); // special effect script [Valaris]
int buildin_specialeffect2(CScriptEngine &st); // special effect script [Valaris]
int buildin_nude(CScriptEngine &st); // nude [Valaris]
int buildin_gmcommand(CScriptEngine &st); // [MouseJstr]
int buildin_movenpc(CScriptEngine &st); // [MouseJstr]
int buildin_message(CScriptEngine &st); // [MouseJstr]
int buildin_npctalk(CScriptEngine &st); // [Valaris]
int buildin_hasitems(CScriptEngine &st); // [Valaris]
int buildin_getlook(CScriptEngine &st);	//Lorky [Lupus]
int buildin_getsavepoint(CScriptEngine &st);	//Lorky [Lupus]
int buildin_npcspeed(CScriptEngine &st); // [Valaris]
int buildin_npcwalkto(CScriptEngine &st); // [Valaris]
int buildin_npcstop(CScriptEngine &st); // [Valaris]
int buildin_getmapxy(CScriptEngine &st);  //get map position for player/npc/pet/mob by Lorky [Lupus]
int buildin_checkoption1(CScriptEngine &st); // [celest]
int buildin_checkoption2(CScriptEngine &st); // [celest]
int buildin_guildgetexp(CScriptEngine &st); // [celest]
int buildin_skilluseid(CScriptEngine &st); // originally by Qamera [celest]
int buildin_skillusepos(CScriptEngine &st); // originally by Qamera [celest]
int buildin_logmes(CScriptEngine &st); // [Lupus]
int buildin_summon(CScriptEngine &st); // [celest]
int buildin_isnight(CScriptEngine &st); // [celest]
int buildin_isday(CScriptEngine &st); // [celest]
int buildin_isequipped(CScriptEngine &st); // [celest]
int buildin_isequippedcnt(CScriptEngine &st); // [celest]
int buildin_cardscnt(CScriptEngine &st); // [Lupus]
int buildin_getrefine(CScriptEngine &st); // [celest]
int buildin_adopt(CScriptEngine &st);
int buildin_night(CScriptEngine &st);
int buildin_day(CScriptEngine &st);
int buildin_getusersname(CScriptEngine &st); //jA commands added [Lupus]
int buildin_dispbottom(CScriptEngine &st);
int buildin_recovery(CScriptEngine &st);
int buildin_getpetinfo(CScriptEngine &st);
int buildin_checkequipedcard(CScriptEngine &st);
int buildin_globalmes(CScriptEngine &st);
int buildin_jump_zero(CScriptEngine &st);
int buildin_select(CScriptEngine &st);
int buildin_getmapmobs(CScriptEngine &st); //jA addition end
int buildin_getstrlen(CScriptEngine &st); //strlen [valaris]
int buildin_charisalpha(CScriptEngine &st);//isalpha [valaris]
int buildin_fakenpcname(CScriptEngine &st); // [Lance]

int buildin_defpattern(CScriptEngine &st); // MouseJstr
int buildin_activatepset(CScriptEngine &st); // MouseJstr
int buildin_deactivatepset(CScriptEngine &st); // MouseJstr
int buildin_deletepset(CScriptEngine &st); // MouseJstr

int buildin_unequip(CScriptEngine &st); // unequip [Spectre]

int buildin_pcstrcharinfo(CScriptEngine &st);
int buildin_getnameditem(CScriptEngine &st);
int buildin_compare(CScriptEngine &st);
int buildin_warpparty(CScriptEngine &st);
int buildin_warpguild(CScriptEngine &st);
int buildin_pc_emotion(CScriptEngine &st);
int buildin_getiteminfo(CScriptEngine &st);
int buildin_callshop(CScriptEngine &st);


int buildin_regex(CScriptEngine &st);


struct {
	int (*func)(CScriptEngine &);
	char *name;
	char *arg;
} buildin_func[]={
	{buildin_mes,"mes","s"},
	{buildin_next,"next",""},
	{buildin_close,"close",""},
	{buildin_close2,"close2",""},
	{buildin_menu,"menu","*"},
	{buildin_goto,"goto","l"},
	{buildin_callsub,"callsub","i*"},
	{buildin_callfunc,"callfunc","s*"},
	{buildin_return,"return","*"},
	{buildin_getarg,"getarg","i"},
	{buildin_jobchange,"jobchange","i*"},
	{buildin_input,"input","*"},
	{buildin_warp,"warp","sii"},
	{buildin_areawarp,"areawarp","siiiisii"},
	{buildin_setlook,"setlook","ii"},
	{buildin_set,"set","ii"},
	{buildin_setarray,"setarray","ii*"},
	{buildin_cleararray,"cleararray","iii"},
	{buildin_copyarray,"copyarray","iii"},
	{buildin_getarraysize,"getarraysize","i"},
	{buildin_deletearray,"deletearray","ii"},
	{buildin_getelementofarray,"getelementofarray","ii"},
	{buildin_if,"if","i*"},
	{buildin_getitem,"getitem","ii**"},
	{buildin_getitem2,"getitem2","iiiiiiiii*"},
	{buildin_getnameditem,"getnameditem","is"},
	{buildin_makeitem,"makeitem","iisii"},
	{buildin_delitem,"delitem","ii"},
	{buildin_cutin,"cutin","si"},
	{buildin_cutincard,"cutincard","i"},
	{buildin_viewpoint,"viewpoint","iiiii"},
	{buildin_heal,"heal","ii"},
	{buildin_itemheal,"itemheal","ii"},
	{buildin_percentheal,"percentheal","ii"},
	{buildin_rand,"rand","i*"},
	{buildin_countitem,"countitem","i"},
	{buildin_checkweight,"checkweight","ii"},
	{buildin_readparam,"readparam","i*"},
	{buildin_getcharid,"getcharid","i*"},
	{buildin_getpartyname,"getpartyname","i"},
	{buildin_getpartymember,"getpartymember","i"},
	{buildin_getguildname,"getguildname","i"},
	{buildin_getguildmaster,"getguildmaster","i"},
	{buildin_getguildmasterid,"getguildmasterid","i"},
	{buildin_strcharinfo,"strcharinfo","i"},
	{buildin_getequipid,"getequipid","i"},
	{buildin_getequipname,"getequipname","i"},
	{buildin_getbrokenid,"getbrokenid","i"}, // [Valaris]
	{buildin_repair,"repair","i"}, // [Valaris]
	{buildin_getequipisequiped,"getequipisequiped","i"},
	{buildin_getequipisenableref,"getequipisenableref","i"},
	{buildin_getequipisidentify,"getequipisidentify","i"},
	{buildin_getequiprefinerycnt,"getequiprefinerycnt","i"},
	{buildin_getequipweaponlv,"getequipweaponlv","i"},
	{buildin_getequippercentrefinery,"getequippercentrefinery","i"},
	{buildin_successrefitem,"successrefitem","i"},
	{buildin_failedrefitem,"failedrefitem","i"},
	{buildin_statusup,"statusup","i"},
	{buildin_statusup2,"statusup2","ii"},
	{buildin_bonus,"bonus","ii"},
	{buildin_bonus2,"bonus2","iii"},
	{buildin_bonus3,"bonus3","iiii"},
	{buildin_bonus4,"bonus4","iiiii"},
	{buildin_skill,"skill","ii*"},
	{buildin_addtoskill,"addtoskill","ii*"}, // [Valaris]
	{buildin_guildskill,"guildskill","ii"},
	{buildin_getskilllv,"getskilllv","i"},
	{buildin_getgdskilllv,"getgdskilllv","ii"},
	{buildin_basicskillcheck,"basicskillcheck","*"},
	{buildin_getgmlevel,"getgmlevel","*"},
	{buildin_end,"end",""},
	{buildin_end,"break",""},
	{buildin_checkoption,"checkoption","i"},
	{buildin_setoption,"setoption","i"},
	{buildin_setcart,"setcart",""},
	{buildin_checkcart,"checkcart","*"},		//fixed by Lupus (added '*')
	{buildin_setfalcon,"setfalcon",""},
	{buildin_checkfalcon,"checkfalcon","*"},	//fixed by Lupus (fixed wrong pointer, added '*')
	{buildin_setriding,"setriding",""},
	{buildin_checkriding,"checkriding","*"},	//fixed by Lupus (fixed wrong pointer, added '*')
	{buildin_savepoint,"save","sii"},
	{buildin_savepoint,"savepoint","sii"},
	{buildin_gettimetick,"gettimetick","i"},
	{buildin_gettime,"gettime","i"},
	{buildin_gettimestr,"gettimestr","si"},
	{buildin_openstorage,"openstorage",""},
	{buildin_guildopenstorage,"guildopenstorage","*"},
	{buildin_itemskill,"itemskill","iis"},
	{buildin_produce,"produce","i"},
	{buildin_monster,"monster","siisii*"},
	{buildin_areamonster,"areamonster","siiiisii*"},
	{buildin_killmonster,"killmonster","ss"},
	{buildin_killmonsterall,"killmonsterall","s"},
	{buildin_doevent,"doevent","s"},
	{buildin_donpcevent,"donpcevent","s"},
	{buildin_addtimer,"addtimer","is"},
	{buildin_deltimer,"deltimer","s"},
	{buildin_addtimercount,"addtimercount","si"},
	{buildin_initnpctimer,"initnpctimer","*"},
	{buildin_stopnpctimer,"stopnpctimer","*"},
	{buildin_startnpctimer,"startnpctimer","*"},
	{buildin_setnpctimer,"setnpctimer","*"},
	{buildin_getnpctimer,"getnpctimer","i*"},
	{buildin_attachnpctimer,"attachnpctimer","*"}, // attached the player id to the npc timer [Celest]
	{buildin_detachnpctimer,"detachnpctimer","*"}, // detached the player id from the npc timer [Celest]
	{buildin_announce,"announce","si"},
	{buildin_mapannounce,"mapannounce","ssi"},
	{buildin_areaannounce,"areaannounce","siiiisi"},
	{buildin_getusers,"getusers","i"},
	{buildin_getmapusers,"getmapusers","s"},
	{buildin_getareausers,"getareausers","siiii"},
	{buildin_getareadropitem,"getareadropitem","siiiii"},
	{buildin_enablenpc,"enablenpc","s"},
	{buildin_disablenpc,"disablenpc","s"},
	{buildin_enablearena,"enablearena",""},		// Added by RoVeRT
	{buildin_disablearena,"disablearena",""},	// Added by RoVeRT
	{buildin_hideoffnpc,"hideoffnpc","s"},
	{buildin_hideonnpc,"hideonnpc","s"},
	{buildin_sc_start,"sc_start","iii*"},
	{buildin_sc_start2,"sc_start2","iiii*"},
	{buildin_sc_start4,"sc_start4","iiiiii*"},
	{buildin_sc_end,"sc_end","i"},
	{buildin_getscrate,"getscrate","ii*"},
	{buildin_debugmes,"debugmes","s"},
	{buildin_catchpet,"pet","i"},
	{buildin_birthpet,"bpet",""},
	{buildin_resetlvl,"resetlvl","i"},
	{buildin_resetstatus,"resetstatus",""},
	{buildin_resetskill,"resetskill",""},
	{buildin_changebase,"changebase","i"},
	{buildin_changesex,"changesex",""},
	{buildin_waitingroom,"waitingroom","si*"},
	{buildin_warpwaitingpc,"warpwaitingpc","sii"},
	{buildin_delwaitingroom,"delwaitingroom","*"},
	{buildin_enablewaitingroomevent,"enablewaitingroomevent","*"},
	{buildin_disablewaitingroomevent,"disablewaitingroomevent","*"},
	{buildin_getwaitingroomstate,"getwaitingroomstate","i*"},
	{buildin_warpwaitingpc,"warpwaitingpc","sii*"},
	{buildin_attachrid,"attachrid","i"},
	{buildin_detachrid,"detachrid",""},
	{buildin_isloggedin,"isloggedin","i"},
	{buildin_setmapflagnosave,"setmapflagnosave","ssii"},
	{buildin_setmapflag,"setmapflag","si"},
	{buildin_removemapflag,"removemapflag","si"},
	{buildin_pvpon,"pvpon","s"},
	{buildin_pvpoff,"pvpoff","s"},
	{buildin_gvgon,"gvgon","s"},
	{buildin_gvgoff,"gvgoff","s"},
	{buildin_emotion,"emotion","i"},
	{buildin_maprespawnguildid,"maprespawnguildid","sii"},
	{buildin_agitstart,"agitstart",""},	// <Agit>
	{buildin_agitend,"agitend",""},
	{buildin_agitcheck,"agitcheck","i"},   // <Agitcheck>
	{buildin_flagemblem,"flagemblem","i"},	// Flag Emblem
	{buildin_getcastlename,"getcastlename","s"},
	{buildin_getcastledata,"getcastledata","si*"},
	{buildin_setcastledata,"setcastledata","sii"},
	{buildin_requestguildinfo,"requestguildinfo","i*"},
	{buildin_getequipcardcnt,"getequipcardcnt","i"},
	{buildin_successremovecards,"successremovecards","i"},
	{buildin_failedremovecards,"failedremovecards","ii"},
	{buildin_marriage,"marriage","s"},
	{buildin_wedding_effect,"wedding",""},
	{buildin_divorce,"divorce","*"},
	{buildin_ispartneron,"ispartneron","*"},
	{buildin_getpartnerid,"getpartnerid","*"},
	{buildin_getchildid,"getchildid",""},
	{buildin_warppartner,"warppartner","sii"},
	{buildin_getitemname,"getitemname","i"},
	{buildin_makepet,"makepet","i"},
	{buildin_getexp,"getexp","ii"},
	{buildin_getinventorylist,"getinventorylist",""},
	{buildin_getskilllist,"getskilllist",""},
	{buildin_clearitem,"clearitem",""},
	{buildin_classchange,"classchange","ii"},
	{buildin_misceffect,"misceffect","i"},
	{buildin_soundeffect,"soundeffect","si"},
	{buildin_soundeffectall,"soundeffectall","si"},	// SoundEffectAll [Codemaster]
	{buildin_strmobinfo,"strmobinfo","ii"},	// display mob data [Valaris]
	{buildin_guardian,"guardian","siisii*i"},	// summon guardians
	{buildin_guardianinfo,"guardianinfo","i"},	// display guardian data [Valaris]
	{buildin_petskillbonus,"petskillbonus","iiii"}, // [Valaris]
	{buildin_petrecovery,"petrecovery","ii"}, // [Valaris]
	{buildin_petloot,"petloot","i"}, // [Valaris]
	{buildin_petheal,"petheal","iiii"}, // [Valaris]
	{buildin_petskillattack,"petskillattack","iiii"}, // [Skotlex]
	{buildin_petskillattack2,"petskillattack2","iiiii"}, // [Valaris]
	{buildin_petskillsupport,"petskillsupport","iiiii"}, // [Skotlex]
	{buildin_skilleffect,"skilleffect","ii"}, // skill effect [Celest]
	{buildin_npcskilleffect,"npcskilleffect","iiii"}, // npc skill effect [Valaris]
	{buildin_specialeffect,"specialeffect","i"}, // npc skill effect [Valaris]
	{buildin_specialeffect2,"specialeffect2","i"}, // skill effect on players[Valaris]
	{buildin_nude,"nude",""}, // nude command [Valaris]
	{buildin_mapwarp,"mapwarp","ssii"},		// Added by RoVeRT
	{buildin_inittimer,"inittimer",""},
	{buildin_stoptimer,"stoptimer",""},
	{buildin_cmdothernpc,"cmdothernpc","ss"},
	{buildin_gmcommand,"gmcommand","*"}, // [MouseJstr]
	{buildin_movenpc,"movenpc","siis"}, // [MouseJstr]
	{buildin_message,"message","s*"}, // [MouseJstr]
	{buildin_npctalk,"npctalk","*"}, // [Valaris]
	{buildin_hasitems,"hasitems","*"}, // [Valaris]
	{buildin_mobcount,"mobcount","ss"},
	{buildin_getlook,"getlook","i"},
	{buildin_getsavepoint,"getsavepoint","i"},
	{buildin_npcspeed,"npcspeed","i"}, // [Valaris]
	{buildin_npcwalkto,"npcwalkto","ii"}, // [Valaris]
	{buildin_npcstop,"npcstop",""}, // [Valaris]
	{buildin_getmapxy,"getmapxy","siii*"},	//by Lorky [Lupus]
	{buildin_checkoption1,"checkoption1","i"},
	{buildin_checkoption2,"checkoption2","i"},
	{buildin_guildgetexp,"guildgetexp","i"},
	{buildin_skilluseid,"skilluseid","ii"}, // originally by Qamera [Celest]
	{buildin_skilluseid,"doskill","ii"}, // since a lot of scripts would already use 'doskill'...
	{buildin_skillusepos,"skillusepos","iiii"}, // [Celest]
	{buildin_logmes,"logmes","s"}, //this command actls as MES but prints info into LOG file either SQL/TXT [Lupus]
	{buildin_summon,"summon","si*"}, // summons a slave monster [Celest]
	{buildin_isnight,"isnight",""}, // check whether it is night time [Celest]
	{buildin_isday,"isday",""}, // check whether it is day time [Celest]
	{buildin_isequipped,"isequipped","i*"}, // check whether another item/card has been equipped [Celest]
	{buildin_isequippedcnt,"isequippedcnt","i*"}, // check how many items/cards are being equipped [Celest]
	{buildin_cardscnt,"cardscnt","i*"}, // check how many items/cards are being equipped in the same arm [Lupus]
	{buildin_getrefine,"getrefine","*"}, // returns the refined number of the current item, or an item with index specified [celest]
	{buildin_adopt,"adopt","sss"}, // allows 2 parents to adopt a child
	{buildin_night,"night",""}, // sets the server to night time
	{buildin_day,"day",""}, // sets the server to day time
	{buildin_defpattern, "defpattern", "iss"}, // Define pattern to listen for [MouseJstr]
	{buildin_activatepset, "activatepset", "i"}, // Activate a pattern set [MouseJstr]
	{buildin_deactivatepset, "deactivatepset", "i"}, // Deactive a pattern set [MouseJstr]
	{buildin_deletepset, "deletepset", "i"}, // Delete a pattern set [MouseJstr]
	{buildin_dispbottom,"dispbottom","s"}, //added from jA [Lupus]
	{buildin_getusersname,"getusersname","*"},
	{buildin_recovery,"recovery",""},
	{buildin_getpetinfo,"getpetinfo","i"},
	{buildin_checkequipedcard,"checkequipedcard","i"},
	{buildin_jump_zero,"jump_zero","ii"}, //for future jA script compatibility
	{buildin_select,"select","*"}, //for future jA script compatibility
	{buildin_globalmes,"globalmes","s*"},
	{buildin_getmapmobs,"getmapmobs","s"}, //end jA addition
	{buildin_unequip,"unequip","i"}, // unequip command [Spectre]
	{buildin_pcstrcharinfo,"pcstrcharinfo","ii"},
	{buildin_getstrlen,"getstrlen","s"}, //strlen [Valaris]
	{buildin_charisalpha,"charisalpha","si"}, //isalpha [Valaris]
	{buildin_fakenpcname,"fakenpcname","ssi"}, // [Lance]
	{buildin_compare,"compare","ss"},
	{buildin_warpparty,"warpparty","siii"},
	{buildin_warpguild,"warpguild","siii"},
	{buildin_pc_emotion,"pc_emotion","i"},
	{buildin_getiteminfo,"getiteminfo","i"},
	{buildin_callshop,"callshop","si"}, // [Skotlex]
	{buildin_regex,"regex","ss"},
};




enum { LABEL_NEXTLINE=1, LABEL_START };



struct Script_Config script_config;


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Script globals
//
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////



#define SCRIPT_BLOCK_SIZE 256
static char * script_buf = NULL;
static size_t script_pos, script_size;



///////////////////////////////////////////////////////////////////////////////
/// linear string buffer. returns position offsets of inserted strings
/// could be replaced by basics::dbstring<> later
class CStringBuffer
{
	char*	cBuf;
	char*	cWpp;
	char*	cEnd;
public:
	CStringBuffer() : cBuf(NULL),cWpp(NULL),cEnd(NULL)
	{	// add an default empty string in front
		this->insert("");	
	}
	~CStringBuffer()
	{
		clear();
	}

	void clear()
	{
		if(this->cBuf)
		{
			delete[] this->cBuf;
			this->cBuf=NULL;
			this->cWpp=NULL;
			this->cEnd=NULL;
		}
	}
	bool realloc(size_t addition)
	{
		if( (size_t)(this->cEnd-this->cWpp)< addition )
		{
			size_t cn = this->cWpp-this->cBuf;
			size_t sz = this->cEnd-this->cBuf;
			sz += (sz<addition)?addition:sz;

			char* tmp = new char[sz];
			if(NULL==tmp)
				return false;
			if(this->cBuf)
			{
				memcpy(tmp, this->cBuf, cn);
				delete[] this->cBuf;
			}
			this->cBuf = tmp;
			this->cWpp = tmp+cn;
			this->cEnd = tmp+sz;
		}
		return true;
	}

	size_t insert(const char* str)
	{
		if(str)
		{
			size_t ret= cWpp-cBuf;
			size_t sz = 1+strlen(str);
			if( realloc(sz) )
			{
				// doing the obvious way
				//while( *str ) *cWpp++ = *str++;	// copy the string
				//*cWpp++ = 0;						// add the terminator

				// memcopy is faster because it copies full bus width 
				// and we already did our log(n) operation with counting the string
				memcpy(cWpp,str,sz);
				cWpp+=sz;
			}
			return ret;
		}
		// the default empty string
		return 0;
	}

	const char* get(size_t ofs)
	{
		if( (size_t)(this->cWpp-this->cBuf) > ofs )
		{
			return cBuf+ofs;
		}
		return cBuf;
	}

	const char* operator()(size_t ofs)	{ return this->get(ofs); }

};


struct s_str_data
{
	int type;
	int str;
	int backpatch;
	int label;
	int (*func)(CScriptEngine &);
	int val;
	int next;

private:
	static CStringBuffer cStrbuf;

public:
	s_str_data() : 
		type(CScriptEngine::C_NOP),
		str(0),
		backpatch(-1),
		label(-1),
		func(NULL),
		val(0),
		next(0)
	{}

	const char*string()
	{
		return this->cStrbuf.get(this->str);
	}
	void insert(const char* str)
	{
		this->str = this->cStrbuf.insert(str);
		this->type=CScriptEngine::C_NOP;
		this->next=0;
		this->func=NULL;
		this->backpatch=-1;
		this->label=-1;
	}
};

CStringBuffer s_str_data::cStrbuf;


static  s_str_data* str_data = NULL;


int str_num=LABEL_START,str_data_size;
int str_hash[16];

static struct dbt *mapreg_db=NULL;
static struct dbt *mapregstr_db=NULL;
static int mapreg_dirty=-1;
char mapreg_txt[256]="save/mapreg.txt";
#define MAPREG_AUTOSAVE_INTERVAL	(10*1000)

static struct dbt *userfunc_db=NULL;

struct dbt* script_get_userfunc_db(){ if(!userfunc_db) userfunc_db=strdb_init(50); return userfunc_db; }

static char positions[11][64] = {"頭","体","左手","右手","ローブ","靴","アクセサリー1","アクセサリー2","頭2","頭3","装着していない"};



static int parse_cmd_if=0;
static int parse_cmd;

extern int current_equip_item_index; //for New CARS Scripts. It contains Inventory Index of the EQUIP_SCRIPT caller item. [Lupus]


char* parse_subexpr(char *p,int limit);
int mapreg_setregstr(int num,const char *str);
int mapreg_setregnum(int num,int val);



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Script Parser Implementation
//
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


void debug_script(const char*script, size_t i, size_t sz)
{
	sz += i&15;
	i &= ~15;
	while(sz>0)
	{
		if((i&15)==0) ShowMessage("%04x : ",i);
		ShowMessage("%02x ", 0xFF&script[i]);
		if((i&15)==15) ShowMessage("\n");

		sz--;
		i++;
	}
	ShowMessage("\n");
}




/*==========================================
 * 文字列のハッシュを計算
 *------------------------------------------
 */
unsigned char calc_hash(const char *str)
{
	size_t h=0;
	if(str)
	{	
		while(*str)
		{	// calculate hash on lowercase inputs
			h=(h<<1)+(h>>3)+(h>>5)+(h>>8);
			h+= (unsigned char)(tolower(*str++));
		}
	}
	return h&0xF;
}

/*==========================================
 * str_dataの中に名前があるか検索する
 *------------------------------------------
 */
// 既存のであれば番号、無ければ-1
int search_str(const char *p)
{
	int i;
	i=str_hash[calc_hash(p)];
	while(i)
	{	// function names could be case-insensitive
		//bool casesens = (str_data[i].type!=CScriptEngine::C_FUNC);
		bool casesens = false;
		if( 0==( (casesens)?strcmp(str_data[i].string(), p):strcasecmp(str_data[i].string(), p) ) )
			return i;
		i=str_data[i].next;
	}
	return -1;
}

/*==========================================
 * str_dataに名前を登録
 *------------------------------------------
 */
// 既存のであれば番号、無ければ登録して新規番号
int add_str(const char *p)
{
	int i;
	if(NULL==p) return -1; // should not happen

	i=search_str(p);
	if(i >= 0)
		return i;
	// else need to insert it

	i=calc_hash(p);
	if(str_hash[i]==0)
	{
		str_hash[i]=str_num;
	}
	else
	{
		i=str_hash[i];
		for(;;)
		{
			if(strcmp(str_data[i].string(),p)==0)
				return i;
			if(str_data[i].next==0)
				break;
			i=str_data[i].next;
		}
		str_data[i].next=str_num;
	}

	if(str_num>=str_data_size)
	{
		str_data_size = new_realloc(str_data, str_data_size, 128);
	}

	str_data[str_num].insert(p);
	return str_num++;
}

/*==========================================
 * スクリプトバッファサイズの確認と拡張
 *------------------------------------------
 */
void check_script_buf(int size)
{
	if(script_pos+size>=script_size)
	{
		script_size = new_realloc(script_buf,script_size,SCRIPT_BLOCK_SIZE);
	}
}

/*==========================================
 * スクリプトバッファに１バイト書き込む
 *------------------------------------------
 */
void add_scriptb(int a)
{
	check_script_buf(1);
	script_buf[script_pos++] = (0xFF&a);
}

/*==========================================
 * スクリプトバッファにデータタイプを書き込む
 *------------------------------------------
 */
void add_scriptc(int a)
{
	while(a>=0x40){
		add_scriptb((a&0x3f)|0x40);
		a=(a-0x40)>>6;
	}
	add_scriptb(a&0x3f);
}

/*==========================================
 * スクリプトバッファに整数を書き込む
 *------------------------------------------
 */
void add_scripti(int a)
{
	while(a>=0x40)
	{
		add_scriptb(a|0xc0);
		a=(a-0x40)>>6;
	}
	add_scriptb(a|0x80);
}

/*==========================================
 * スクリプトバッファにラベル/変数/関数を書き込む
 *------------------------------------------
 */
// 最大16Mまで
// 2^24
void add_scriptl(int l)
{
	int backpatch = str_data[l].backpatch;

	switch(str_data[l].type){
	case CScriptEngine::C_POS:
		add_scriptc(CScriptEngine::C_POS);
		add_scriptb(str_data[l].label);
		add_scriptb(str_data[l].label>>8);
		add_scriptb(str_data[l].label>>16);
		break;
	case CScriptEngine::C_NOP:
		// ラベルの可能性があるのでbackpatch用データ埋め込み
		add_scriptc(CScriptEngine::C_NAME);
		str_data[l].backpatch=script_pos;
		add_scriptb(backpatch);
		add_scriptb(backpatch>>8);
		add_scriptb(backpatch>>16);
		break;
	case CScriptEngine::C_INT:
		add_scripti((str_data[l].val<0)?-str_data[l].val:str_data[l].val);
		if(str_data[l].val < 0) add_scriptc(CScriptEngine::C_NEG);
		break;
	default:
		// もう他の用途と確定してるので数字をそのまま
		add_scriptc(CScriptEngine::C_NAME);
		add_scriptb(l);
		add_scriptb(l>>8);
		add_scriptb(l>>16);
		break;
	}
}

/*==========================================
 * ラベルを解決する
 *------------------------------------------
 */
void set_label(int l,int pos)
{
	int i,next;

	str_data[l].type=CScriptEngine::C_POS;
	str_data[l].label=pos;

	i=str_data[l].backpatch;
	while(i>0 && i!=0x00ffffff)
	{
		next = (0xFF&script_buf[i])
			 | (0xFF&script_buf[i+1])<<8
			 | (0xFF&script_buf[i+2])<<16;
		script_buf[i-1]=CScriptEngine::C_POS;
		script_buf[i]=pos;
		script_buf[i+1]=pos>>8;
		script_buf[i+2]=pos>>16;
		i=next;
	}
}


/*==========================================
 * 数値の所得
 *------------------------------------------
 */
int get_num(const char *script, size_t &pos)
{
	const unsigned char *s = (const unsigned char *)script;
	int i=0,j=0;

	while(s[pos]>=0xc0){
		i+=(s[pos++]&0x7f)<<j;
		j+=6;
	}
	return i+((s[pos++]&0x7f)<<j);
}

/*==========================================
 * コマンドのプッシュバック
 *------------------------------------------
// actually unused maybe remove it
 */
static int unget_com_data=-1;
void unget_com(int c)
{
	if(unget_com_data!=-1){
		if(config.error_log)
			ShowMessage("unget_com can back only 1 data\n");
	}
	unget_com_data=c;
}

/*==========================================
 * コマンドの読み取り
 *------------------------------------------
 */
int get_com(const char *script, size_t &pos)
{
	const unsigned char *s = (const unsigned char *)script;
	int i,j;
	if(unget_com_data>=0){
		i=unget_com_data;
		unget_com_data=-1;
		return i;
	}
	if(s[pos]>=0x80){
		return CScriptEngine::C_INT;
	}
	i=0; j=0;
	while(s[pos]>=0x40){
		i=s[pos++]<<j;
		j+=6;
	}
	return i+(s[pos++]<<j);
}




/*==========================================
 * スペース/コメント読み飛ばし
 *------------------------------------------
 */
char *skip_space(const char *p)
{
	while(1){
		while( *p==0x20 || (*p>=0x09 && *p<=0x0D) )
			p++;
		if(p[0]=='/' && p[1]=='/'){
			while(*p && *p!='\n')
				p++;
		} else if(p[0]=='/' && p[1]=='*'){
			p++;
			while(*p && (p[-1]!='*' || p[0]!='/'))
				p++;
			if(*p) p++;
		} else
			break;
	}
	return (char*)p;
}

/*==========================================
 * １単語スキップ
 *------------------------------------------
 */
char *skip_word(const char *str)
{
	unsigned char*p =(unsigned char*)str;
	if(p)
	{	// prefix
		if(*p=='$') p++;	// MAP鯖内共有変数用
		if(*p=='@') p++;	// 一時的変数用(like weiss)
		if(*p=='#') p++;	// account変数用
		if(*p=='#') p++;	// ワールドaccount変数用
		if(*p=='l') p++;	// 一時的変数用(like weiss)

		// this is for skipping multibyte characters
		// I do not modify here but just set the string pointer back to unsigned
		while(isalnum((int)((unsigned char)*p))||*p=='_'|| *p>=0x81)
		{
			if(*p>=0x81 && p[1])
				p+=2;
			else
				p++;
		}

		// postfix
		if(*p=='$') p++;	// 文字列変数
	}
	return (char*)p;
}

static char *startptr;
static int startline;

/*==========================================
 * エラーメッセージ出力
 *------------------------------------------
 */
void disp_error_message(const char *mes,const char *pos)
{
	int line, on=0;
	const char *p,*linestart,*lineend;

	for(line=startline, p=startptr; p && *p; ++line)
	{
		linestart=p;
		lineend=strchr(p,'\n');
		if(lineend==NULL || pos<lineend)
		{
			// skip all leading control chars
			for(; basics::stringcheck::iscntrl(*p); ++p) {}

			if(lineend==NULL)
			{	// go to the end of the string
				for(lineend=p; *lineend; ++lineend) {}
			}
			else
			{	// skip all preceeding control chars
				for(; basics::stringcheck::iscntrl(*lineend); --lineend) {}
			}

			ShowError("\n%s line "CL_WHITE"\'%d\'"CL_RESET":\n", mes, line);
			for(; *p &&  p<lineend; ++p)
			{
				if(p==pos)
				{
					on=1;
					ShowMessage("\'"CL_BT_RED);
				}
				else if( on && !(*p=='_' || basics::stringcheck::isalnum(*p)) )
				{
					on=0;
					ShowMessage(CL_RESET"\'");
				}
				ShowMessage("%c",*p);
			}
			ShowMessage(CL_RESET"\n");
			return;
		}
		p=lineend+1;
	}
}



/*==========================================
 * 組み込み関数の追加
 *------------------------------------------
 */
void add_buildin_func(void)
{
	size_t i,n;
	for(i=0; i<sizeof(buildin_func)/sizeof(*buildin_func); ++i)
	{
		if(buildin_func[i].name && buildin_func[i].func)
		{
			n=add_str(buildin_func[i].name);
			str_data[n].type=CScriptEngine::C_FUNC;
			str_data[n].val=i;
			str_data[n].func=buildin_func[i].func;
		}
	}
}

/*==========================================
 * 定数データベースの読み込み
 *------------------------------------------
 */
void read_constdb(void)
{
	FILE *fp;
	char line[1024];
	char name[1024];
	int val,n,type;

	fp=basics::safefopen("db/const.txt","r");
	if(fp==NULL){
		ShowError("can't read %s\n","db/const.txt");
		return ;
	}
	while(fgets(line,sizeof(line),fp))
	{
		if( !is_valid_line(line) )
			continue;
		type=0;
		if(sscanf(line,"%1024[A-Za-z0-9_],%d,%d",name,&val,&type)>=2 ||
		   sscanf(line,"%1024[A-Za-z0-9_] %d %d",name,&val,&type)>=2)
		{
			basics::tolower(name);
			basics::itrim(name);
			n=add_str(name);
			if(type==0)
				str_data[n].type=CScriptEngine::C_INT;
			else
				str_data[n].type=CScriptEngine::C_PARAM;
			str_data[n].val=val;
		}
	}
	fclose(fp);
}


/*==========================================
 * 項の解析
 *------------------------------------------
 */
char* parse_simpleexpr(char *p)
{
	p=skip_space(p);

#ifdef DEBUG_FUNCIN
	if(config.etc_log)
		ShowMessage("parse_simpleexpr %s\n",p);
#endif
	if(*p==';' || *p==','){
		disp_error_message("unexpected expr end",p);
		exit(1);
	}
	if(*p=='('){

		p=parse_subexpr(p+1,-1);
		p=skip_space(p);
		if((*p++)!=')'){
			disp_error_message("unmatch ')'",p);
			exit(1);
		}
	} else if(isdigit((int)((unsigned char)*p)) || ((*p=='-' || *p=='+') && isdigit((int)((unsigned char)p[1])))){
		char *np;
		int i=strtoul(p,&np,0);
		add_scripti(i);
		p= np;
	} else if(*p=='"'){
		add_scriptc(CScriptEngine::C_STR);
		p++;
		while(*p && *p!='"'){
			if(p[-1]<=0x7e && *p=='\\')
				p++;
			else if(*p=='\n'){
				disp_error_message("unexpected newline @ string", p);
				exit(1);
			}
			add_scriptb(*p++);
		}
		if(!*p){
			disp_error_message("unexpected eof @ string", p);
			exit(1);
		}
		add_scriptb(0);
		p++;	//'"'
	} else {
		// label , register , function etc
		int c,l;
		char *p2 = skip_word(p);
		if(p2==p && !(*p==')' && p[-1]=='(')){
			disp_error_message("unexpected character",p);
			exit(1);
		}
		c=*p2;
		*p2=0;	// 名前をadd_strする
		l=add_str(p);

		parse_cmd=l;	// warn_*_mismatch_paramnumのために必要
		if(l==search_str("if"))	// warn_cmd_no_commaのために必要
			parse_cmd_if++;
/*
		// 廃止予定のl14/l15,およびプレフィックスｌの警告
		if(	strcmp(str_buf+str_data[l].str,"l14")==0 ||
			strcmp(str_buf+str_data[l].str,"l15")==0 ){
			disp_error_message("l14 and l15 is DEPRECATED. use @menu instead of l15.",p);
		}else if(str_buf[str_data[l].str]=='l'){
			disp_error_message("prefix 'l' is DEPRECATED. use prefix '@' instead.",p2);
		}
*/
		*p2=c;	
		p=p2;

		if(str_data[l].type!=CScriptEngine::C_FUNC && c=='['){
			// array(name[i] => getelementofarray(name,i) )
			add_scriptl(search_str("getelementofarray"));
			add_scriptc(CScriptEngine::C_ARG);
			add_scriptl(l);
			p=parse_subexpr(p+1,-1);
			p=skip_space(p);
			if((*p++)!=']'){
				disp_error_message("unmatch ']'",p);
				exit(1);
			}
			add_scriptc(CScriptEngine::C_FUNC);
		}else
			add_scriptl(l);

	}

#ifdef DEBUG_FUNCIN
	if(config.etc_log)
		ShowMessage("parse_simpleexpr end %s\n",p);
#endif
	return p;
}

/*==========================================
 * 式の解析
 *------------------------------------------
 */
char* parse_subexpr(char *p,int limit)
{
	int op,opl,len;
	char *tmpp;

#ifdef DEBUG_FUNCIN
	if(config.etc_log)
		ShowMessage("parse_subexpr %s\n",p);
#endif
	p=skip_space(p);

	if(*p=='-')
	{
		tmpp = skip_space(p+1);
		if(*tmpp==';' || *tmpp==','){
			add_scriptl(LABEL_NEXTLINE);
			p++;
			return p;
		}
	}
	tmpp = p;
	if((op=CScriptEngine::C_NEG,*p=='-') || (op=CScriptEngine::C_LNOT,*p=='!') || (op=CScriptEngine::C_NOT,*p=='~'))
	{
		p=parse_subexpr(p+1,8);
		add_scriptc(op);
	}
	else
		p=parse_simpleexpr(p);
	p=skip_space(p);
	while(((op=CScriptEngine::C_ADD,opl=6,len=1,*p=='+') ||
		   (op=CScriptEngine::C_SUB,opl=6,len=1,*p=='-') ||
		   (op=CScriptEngine::C_MUL,opl=7,len=1,*p=='*') ||
		   (op=CScriptEngine::C_DIV,opl=7,len=1,*p=='/') ||
		   (op=CScriptEngine::C_MOD,opl=7,len=1,*p=='%') ||
		   (op=CScriptEngine::C_FUNC,opl=9,len=1,*p=='(') ||
		   (op=CScriptEngine::C_LAND,opl=1,len=2,*p=='&' && p[1]=='&') ||
		   (op=CScriptEngine::C_AND,opl=5,len=1,*p=='&') ||
		   (op=CScriptEngine::C_LOR,opl=0,len=2,*p=='|' && p[1]=='|') ||
		   (op=CScriptEngine::C_OR,opl=4,len=1,*p=='|') ||
		   (op=CScriptEngine::C_XOR,opl=3,len=1,*p=='^') ||
		   (op=CScriptEngine::C_EQ,opl=2,len=2,*p=='=' && p[1]=='=') ||
		   (op=CScriptEngine::C_NE,opl=2,len=2,*p=='!' && p[1]=='=') ||
		   (op=CScriptEngine::C_R_SHIFT,opl=5,len=2,*p=='>' && p[1]=='>') ||
		   (op=CScriptEngine::C_GE,opl=2,len=2,*p=='>' && p[1]=='=') ||
		   (op=CScriptEngine::C_GT,opl=2,len=1,*p=='>') ||
		   (op=CScriptEngine::C_L_SHIFT,opl=5,len=2,*p=='<' && p[1]=='<') ||
		   (op=CScriptEngine::C_LE,opl=2,len=2,*p=='<' && p[1]=='=') ||
		   (op=CScriptEngine::C_LT,opl=2,len=1,*p=='<')) && opl>limit){
		p+=len;
		if(op==CScriptEngine::C_FUNC){
			int i=0,func=parse_cmd;
			const char *plist[128];

			if( str_data[func].type!=CScriptEngine::C_FUNC ){
				disp_error_message("expect function",tmpp);
				exit(0);
			}

			add_scriptc(CScriptEngine::C_ARG);
			do {
				plist[i]=p;
				p=parse_subexpr(p,-1);
				p=skip_space(p);
				if(*p==',') p++;
				else if(*p!=')' && script_config.warn_func_no_comma){
					disp_error_message("expect ',' or ')' at func params",p);
				}
				p=skip_space(p);
				i++;
			} while(*p && *p!=')' && i<128);
			plist[i]=p;
			if(*(p++)!=')'){
				disp_error_message("func request '(' ')'",p);
				exit(1);
			}

			if (str_data[func].type == CScriptEngine::C_FUNC && script_config.warn_func_mismatch_paramnum) {
				const char *arg = buildin_func[str_data[func].val].arg;
				int j = 0;
				for (; arg[j]; ++j) if (arg[j] == '*') break;
				if (!(i <= 1 && j == 0) && ((arg[j] == 0 && i != j) || (arg[j] == '*' && i < j))) {
					disp_error_message("illegal number of parameters",plist[(i<j)?i:j]);
				}
			}
		} else {
			p=parse_subexpr(p,opl);
		}
		add_scriptc(op);
		p=skip_space(p);
	}
#ifdef DEBUG_FUNCIN
	if(config.etc_log)
		ShowMessage("parse_subexpr end %s\n",p);
#endif
	return p;  // return first untreated operator 
}

/*==========================================
 * 式の評価
 *------------------------------------------
 */
char* parse_expr(char *p)
{
#ifdef DEBUG_FUNCIN
	if(config.etc_log)
		ShowMessage("parse_expr %s\n",p);
#endif
	switch(*p){
	case ')': case ';': case ':': case '[': case ']':
	case '}':
		disp_error_message("unexpected char",p);
		exit(1);
	}
	p=parse_subexpr(p,-1);
#ifdef DEBUG_FUNCIN
	if(config.etc_log)
		ShowMessage("parse_expr end %s\n",p);
#endif
	return p;
}

/*==========================================
 * 行の解析
 *------------------------------------------
 */
char* parse_line(char *p)
{
	int i=0,cmd;
	const char *plist[128];
	char *p2;

	p=skip_space(p);
	if(*p==';')
		return p+1;

	parse_cmd_if=0;	// warn_cmd_no_commaのために必要

	// 最初は関数名
	p2 = p;
	p=parse_simpleexpr(p);
	p=skip_space(p);

	cmd=parse_cmd;
	if( str_data[cmd].type!=CScriptEngine::C_FUNC )
	{
		disp_error_message("expect command", p2);

		if( 0==strcasecmp( str_data[cmd].string(), "atcommand") ||
			0==strcasecmp( str_data[cmd].string(), "charcommand") )
		{
			ShowMessage("command obsolete. use \"gmcommand\" instead.\n");
		}
		// skip until end of command
		p2 = strchr(p,';');
		return p2?p2+1:p;
	}

	add_scriptc(CScriptEngine::C_ARG);
	while(p && *p && *p!=';' && i<128){
		plist[i]=p;

		p=parse_expr(p);
		p=skip_space(p);
		// 引数区切りの,処理
		if(*p==',') p++;
		else if(*p!=';' && script_config.warn_cmd_no_comma && parse_cmd_if*2<=i ){
			disp_error_message("expect ',' or ';' at cmd params",p);
		}
		p=skip_space(p);
		i++;
	}
	plist[i]=(char *) p;
	if(!p || *(p++)!=';'){
		disp_error_message("need ';'",p);
		exit(1);
	}
	add_scriptc(CScriptEngine::C_FUNC);

	if( str_data[cmd].type==CScriptEngine::C_FUNC && script_config.warn_cmd_mismatch_paramnum){
		const char *arg=buildin_func[str_data[cmd].val].arg;
		int j=0;
		for(j=0;arg[j];++j) if(arg[j]=='*')break;
		if( (arg[j]==0 && i!=j) || (arg[j]=='*' && i<j) ){
			disp_error_message("illegal number of parameters",plist[(i<j)?i:j]);
		}
	}


	return p;
}


///////////////////////////////////////////////////////////////////////////////
size_t script_object::get_labelpos(const char* labelname) const
{
	if(labelname)
	{
		size_t i;
		for(i=0; i<label_list_num; ++i)
		{
			if(0==strcasecmp(label_list[i].name, labelname) )
			{
				return label_list[i].pos;
			}
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
void script_object::insert_label(const char* labelname, size_t len, size_t pos)
{
	if(labelname)
	{
		const char *p = strchr(labelname,':');	// double check; should not happen
		if(p) len = (p-labelname);
		if( len>23 )
		{
			basics::basestring<> label(labelname, len);
			ShowMessage("\n");
			ShowError("parse_script: label '%s' longer than 23 chars, ignoring it.\n", (const char*)label);
		}
		else
		{
			size_t num = this->label_list_num;
			this->label_list_num = new_realloc(this->label_list, this->label_list_num, 1);
			memcpy(this->label_list[num].name, labelname, len);
			this->label_list[num].name[len] = 0; // add eos
			this->label_list[num].pos = pos;
		}
	}
}

/*==========================================
 * スクリプトの解析
 *------------------------------------------
 */
script_object* parse_script(unsigned char *src, size_t line)
{
	char *p,*tmpp;
	int i;
	static int first = 1;
	if(first)
	{
		add_buildin_func();
		read_constdb();
	}
	first = 0;

	////////////////////////////////////////////// 
	// additional check on the input to filter empty scripts ("{}" and "{ }") 
	p = (char*)src;
	p = skip_space(p);
	if(*p!='{')
	{
		disp_error_message("not found '{'", p);
		return NULL;
	}
	p++;
	p = skip_space(p);
	if(*p=='}')
	{	// an empty function, just return
		return NULL;
	}
	//////////////////////////////////////////////

	// the returnning script object 
	script_object *scr = new script_object(NULL);

	// the global script buffer
	if(script_buf) delete[] script_buf;
	script_buf = new char[SCRIPT_BLOCK_SIZE];
	memset(script_buf,0,sizeof(char)*SCRIPT_BLOCK_SIZE);

	script_pos = 0;
	script_size = SCRIPT_BLOCK_SIZE;
	str_data[LABEL_NEXTLINE].type = CScriptEngine::C_NOP;
	str_data[LABEL_NEXTLINE].backpatch = -1;
	str_data[LABEL_NEXTLINE].label = -1;
	for (i = LABEL_START; i < str_num; ++i)
	{
		if (str_data[i].type == CScriptEngine::C_POS || str_data[i].type == CScriptEngine::C_NAME)
		{
			str_data[i].type = CScriptEngine::C_NOP;
			str_data[i].backpatch = -1;
			str_data[i].label = -1;
		}
	}

	// for error message
	startptr = (char*)src;
	startline = line;

	while(p && *p && *p!='}')
	{
		p = skip_space(p);
		// labelだけ特殊処理
		tmpp = skip_space(skip_word(p));
		if(*tmpp==':')
		{	
			char* label_end = skip_word(p);
			int l, c = *label_end;
			*label_end = 0;
			l = add_str(p);
			if (str_data[l].label != -1)
			{
				*skip_word(p) = c;
				disp_error_message("duplicated label ", p);
				exit(1);
			}
			set_label(l, script_pos);

			scr->insert_label(p, label_end-p, script_pos);

			*label_end = c;
			p = tmpp + 1;
		}
		else
		{	// 他は全部一緒くた
			p = parse_line(p);
			p = skip_space(p);
			add_scriptc(CScriptEngine::C_EOL);
			set_label(LABEL_NEXTLINE, script_pos);
			str_data[LABEL_NEXTLINE].type = CScriptEngine::C_NOP;
			str_data[LABEL_NEXTLINE].backpatch = -1;
			str_data[LABEL_NEXTLINE].label = -1;
		}
	}

	add_scriptc(CScriptEngine::C_NOP);
	script_size = script_pos;

	char* tmp=new char[1+script_pos];
	memcpy(tmp,script_buf,1+script_pos);
	delete[] script_buf;
	script_buf = tmp;

	// 未解決のラベルを解決
	for (i = LABEL_START; i < str_num; ++i) {
		if (str_data[i].type == CScriptEngine::C_NOP) {
			int j, next;
			str_data[i].type = CScriptEngine::C_NAME;
			str_data[i].label = i;
			j = str_data[i].backpatch;
			while(j>0 && j != 0x00ffffff)
			{
				next = ((0xFF&script_buf[j])      )
					 | ((0xFF&script_buf[j+1])<< 8)
					 | ((0xFF&script_buf[j+2])<<16);
				script_buf[j  ] = (uchar)(i    );
				script_buf[j+1] = (uchar)(i>> 8);
				script_buf[j+2] = (uchar)(i>>16);
				j = next;
			}
		}
	}

#ifdef DEBUG_DISP
	for (i = 0; i < script_pos; ++i) {
		if((i&15)==0) ShowMessage("%04x : ",i);
		ShowMessage("%02x ", 0xFF&script_buf[i]);
		if((i&15)==15) ShowMessage("\n");
	}
	ShowMessage("\n");
#endif

	// setup the return object
	scr->script = script_buf;
	script_buf = NULL;
	return scr;
}




///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Script Engine Implementation
//
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////



/*==========================================
 * 変数設定用
 *------------------------------------------
 */

int set_var(const char *name, const void *v)
{
	if(name)
	{
		int num = add_str(name);
		char prefix =name[0];
		char postfix=name[strlen(name)-1];
		if(prefix=='$')
		{
			if( postfix=='$' )
			{
				chrif_var_save(name, (const char*)v);
				mapreg_setregstr(num,(const char*)v);
			}
			else
			{
				chrif_var_save(name, (ssize_t)((size_t)v));
				mapreg_setregnum(num,(ssize_t)((size_t)v));
			}
		}
		else
		{
			ShowError("script: set_var: illegal scope string variable '%s'!\n", name);
		}
	}
    return 0;
}
int set_var(map_session_data &sd, const char *name, const void *v)
{
	if(name)
	{
		int num = add_str(name);
		char prefix =name[0];
		char postfix=name[strlen(name)-1];
		if( prefix=='@')
		{
			if( postfix=='$' )
			{
				pc_setregstr(sd,num,(char*)v);
			}
			else
			{
				pc_setreg(sd,num,(ssize_t)((size_t)v));
			}
		}
		else
		{
			ShowError("script: set_var: illegal scope string variable '%s'!\n", name);
		}
	}
    return 0;
}


int set_reg(CScriptEngine &st,int num,const char *name, void *v)
{
	if(name)
	{
		char prefix =name[0];
		char postfix=name[strlen(name)-1];
		
		if( postfix=='$' )
		{	// string variable
			char *str=(char*)v;
			if( prefix=='@')
			{
				if(st.sd)
					pc_setregstr(*st.sd,num,str);
				else
				{
					chrif_var_save(name,str);
					mapreg_setregstr(num,str);
					//ShowError("set_reg error name?:%s\n",name);
				}		
			}
			else if(prefix=='$')
			{
				chrif_var_save(name,str);
				mapreg_setregstr(num,str);
			}
			else
			{
				ShowError("script: set_reg: illegal scope string variable '%s'!\n", name);
			}
		}
		else 
		{	// 数値
			int val = (ssize_t)((size_t)v);
			if(str_data[num&0x00ffffff].type==CScriptEngine::C_PARAM)
			{
				if(st.sd)
					pc_setparam(*st.sd,str_data[num&0x00ffffff].val,val);
			}
			else if(prefix=='@' || prefix=='l')
			{
				if(st.sd)
					pc_setreg(*st.sd,num,val);
				else
				{
					chrif_var_save(name,val);
					mapreg_setregnum(num,val);
					//ShowError("set_reg error name?:%s\n",name);
				}
			}
			else if(prefix=='$')
			{
				chrif_var_save(name,val);
				mapreg_setregnum(num,val);
			}
			else if(prefix=='#')
			{
				if(st.sd)
				{
					if( name[1]=='#' )
						pc_setaccountreg2(*st.sd,name,val);
					else
						pc_setaccountreg(*st.sd,name,val);
				}
				else
					ShowError("set_reg error name?:%s\n",name);
			}
			else
			{
				if(st.sd)
					pc_setglobalreg(*st.sd,name,val);
				else
					ShowError("set_reg error name?:%s\n",name);
			}
		}
	}
	return 0;
}


/*==========================================
 * 変数の読み取り2
 *------------------------------------------
 */
void* get_val2(CScriptEngine &st, int num)
{
	CScriptEngine::CValue dat;
	dat.type=CScriptEngine::C_NAME;
	dat.num=num;
	st.ConvertName(dat);
	if( dat.type==CScriptEngine::C_INT )
		return (void*)((size_t)dat.num);
	else 
		return (void*)dat.str;
}


///////////////////////////////////////////////////////////////////////////////
// static class variable
uint32 CScriptEngine::defoid = npc_get_new_npc_id();

uint32 CScriptEngine::send_defaultnpc(bool send)
{
/*/////////////////////////////////////////////////////////////////////////////
 v5	2004-05-25	no text & no input
 v5	2004-06-28	text & input ok
 v6	xxxx-xx-xx	...untested...
 v7	xxxx-xx-xx	...untested...
 v8	xxxx-xx-xx	...untested...
 v9	xxxx-xx-xx	...untested...
v10	xxxx-xx-xx	text & input ok
v11	xxxx-xx-xx	text & input ok
v12	xxxx-xx-xx	...untested...
v13	xxxx-xx-xx	...untested...
v14	xxxx-xx-xx	...untested...
v15	xxxx-xx-xx	...untested...
v16	xxxx-xx-xx	text & input ok
v17	xxxx-xx-xx	text & input ok
v18 xxxx-xx-xx	text & input ok
*//////////////////////////////////////////////////////////////////////////////

	uint32 ret = 0;
	if( this->sd )
	{	// a npc is only necessary in combination with a sd
		static npcscript_data defnpc;
		static bool needinit = true;

		if(needinit)	//!! integrate to a default constructor
		{	// and initialize
			needinit = false;

			defnpc.block_list::id = this->defoid;
			defnpc.class_ = 111; // hidden npc
			defnpc.speed = 200;
		}

		if(send)
		{
			if( NONE==this->npcstate )
			{	// determine what npc is to be used
				if( this->nd && this->nd->class_>0 && this->nd->is_near(*(this->sd)) )
				{	// npc is ok (not floating and near the pc)
//printf("npc ok ");
					ret = this->nd->block_list::id;
					this->npcstate = NPC_GIVEN;
				}
				else
				{	// not ok, so we send the defaultnpc
					static basics::Mutex mx;
					basics::ScopeLock sl(mx);	// lock the scope

					// the targeted npc is not real or out of sight
					// send a new one to the client
					this->npcstate = NPC_DEFAULT;

					// set current player position
					defnpc.block_list::m = this->sd->block_list::m;
					defnpc.block_list::x = this->sd->block_list::x;
					defnpc.block_list::y = this->sd->block_list::y;
					// spawn only on it's own client
					clif_spawnnpc(*this->sd, defnpc);
//printf("send default npc (%i,%i,%i) ", defnpc.block_list::m, defnpc.block_list::x, defnpc.block_list::y);

					// tell the engine to refer the default npc
					ret = this->defoid;
				}
			}
			else if( this->nd && NPC_GIVEN  ==this->npcstate )
				ret = this->nd->block_list::id;
			else //if( NPC_DEFAULT==this->npcstate )
				ret = this->defoid;
		}
		else
		{
			if( NPC_DEFAULT==this->npcstate )
				clif_clearchar(*(this->sd), defnpc);
			this->npcstate = NONE;
//printf("npc clearing ");
		}
	}
	else
	{
//		printf("no sd ");
	}
//printf("npc using %i\n", ret);
	return ret;
}


///////////////////////////////////////////////////////////////////////////////
// 変数の読み取り
// converts a variable to a value
void CScriptEngine::ConvertName(CScriptEngine::CValue &data)
{
	if(data.type==CScriptEngine::C_NAME)
	{
		int datanum = data.num;
		const char *name=str_data[data.num&0x00ffffff].string();
		const char prefix=*name;
		const char postfix=name[strlen(name)-1];

		if(postfix=='$')
		{
			data.type=CScriptEngine::C_CONSTSTR;
			//data.str = ""; // default
			if( prefix=='@')
			{
				if(this->sd)
					data.str = pc_readregstr(*sd,datanum);
				else
					data.str = (char *)numdb_search(mapregstr_db,datanum);
			}
			else if(prefix=='$')
			{
				data.str = (char *)numdb_search(mapregstr_db,datanum);
			}
			else
			{
				ShowError("script: get_val: illegal scope string variable '%s'!\n", name);
				data.str = "!!ERROR!!";
			}
		}
		else
		{
			data.type=CScriptEngine::C_INT;
			data.num = 0; // default
			
			if(str_data[datanum&0x00ffffff].type==CScriptEngine::C_INT)
			{
				data.num = str_data[datanum&0x00ffffff].val;
			}
			else if(str_data[datanum&0x00ffffff].type==CScriptEngine::C_PARAM)
			{
				if(this->sd)
					data.num = pc_readparam(*sd,str_data[datanum&0x00ffffff].val);
			}
			else if(prefix=='@')
			{
				if(this->sd)
					data.num = pc_readreg(*sd,datanum);
				else
					data.num = (size_t)numdb_search(mapreg_db,datanum);
			}
			else if(prefix=='$')
			{
				data.num = (size_t)numdb_search(mapreg_db,datanum);
			}
			else if(prefix=='#')
			{
				if(this->sd)
				{
					if( name[1]=='#')
						data.num = pc_readaccountreg2(*sd,name);
					else
						data.num = pc_readaccountreg(*sd,name);
				}
			}
			else
			{
				if(this->sd)
					data.num = pc_readglobalreg(*sd,name);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// 文字列への変換
// returns a string from a stack value
// does conversion when necessary
const char* CScriptEngine::GetString(CScriptEngine::CValue &data)
{
	ConvertName(data);
	if(data.type==CScriptEngine::C_INT)
	{
		char*tmp = new char[24];
		snprintf(tmp,24,"%d",data.num);
		data.type= CScriptEngine::C_STR;
		data.str = tmp;
	}
	else if(data.type==CScriptEngine::C_NAME)
	{	// テンポラリ。本来無いはず
		data.type=CScriptEngine::C_CONSTSTR;
		data.str=str_data[data.num].string();
	}
	return (data.str)?data.str:"";
}

///////////////////////////////////////////////////////////////////////////////
// 数値へ変換
// returns an integer from a stack value
// does conversion when necessary
int CScriptEngine::GetInt(CScriptEngine::CValue &data)
{
	ConvertName(data);
	if( data.isString() )
	{
		int val = strtol( data.str, NULL, 0 );
		data.clear();

		data.type=CScriptEngine::C_INT;
		data.num = val;
	}
	return data.num;
}


///////////////////////////////////////////////////////////////////////////////
// スタックへ数値をプッシュ
// push integer on the stack
void CScriptEngine::push_val(int type,int val)
{
	alloc();
//	if(config.etc_log)
//		ShowMessage("push (%d,%d)-> %d\n",type,val,stack_ptr);

	// previous stack value needs clearing
	stack_data[stack_ptr].clear();

	stack_data[stack_ptr].type=type;
	stack_data[stack_ptr].num=val;
	stack_ptr++;
}

///////////////////////////////////////////////////////////////////////////////
// スタックへ文字列をプッシュ
// push string on the stack
void CScriptEngine::push_str(int type, const char *str)
{
	alloc();
//	if(config.etc_log)
//		ShowMessage("push (%d,%x)-> %d\n",type,str,stack_ptr);

	// previous stack value needs clearing
	stack_data[stack_ptr].clear();

	stack_data[stack_ptr].type=type;
	stack_data[stack_ptr].str=str;
	stack_ptr++;
}

///////////////////////////////////////////////////////////////////////////////
// スタックへ複製をプッシュ
// push a copy on the stack
void CScriptEngine::push_copy(size_t pos)
{
	switch(stack_data[pos].type){
	case CScriptEngine::C_CONSTSTR:
		push_str(CScriptEngine::C_CONSTSTR,stack_data[pos].str);
		break;
	case CScriptEngine::C_STR:
	{
		char *str = new char[1+strlen(stack_data[pos].str)];
		memcpy(str, stack_data[pos].str, 1+strlen(stack_data[pos].str));
		push_str(CScriptEngine::C_STR, str);
		break;
	}
	default:
		push_val(stack_data[pos].type,stack_data[pos].num);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// スタックからポップ
// remove values from start to end from the stack
void CScriptEngine::pop_stack(size_t start, size_t end)
{
	if(end>start)
	{	
		size_t i, k;
		if(end>stack_ptr)
			end = stack_ptr;
		// move the values behind 'end' to 'start'
		for(i=start,k=end; k<stack_ptr; ++i,++k)
			stack_data[i] << stack_data[k];
		// reduce the stack pointer
		stack_ptr -= end-start;
	}
	else if(end<start)
	{
		ShowError("Script: pop_stack called irregulary with %i, %i\n", start, end);
		this->Quit();
	}

}

///////////////////////////////////////////////////////////////////////////////
// Binomial operators (string) 二項演算子(文字列)
void CScriptEngine::op_2str(int op)
{
	int a=0;
	char *buf=NULL;
	const char *s2 = GetString( stack_data[stack_ptr  ] );
	const char *s1 = GetString( stack_data[stack_ptr-1] );

	switch(op)
	{
	case C_AND:
	case C_ADD:
		buf=new char[ 1+strlen(stack_data[stack_ptr-1].str)+
			            strlen(stack_data[stack_ptr  ].str) ];
		strcpy(buf,stack_data[stack_ptr-1].str);
		strcat(buf,stack_data[stack_ptr].str);
		break;
	case C_EQ:
		a= (strcmp(s1,s2)==0);
		break;
	case C_NE:
		a= (strcmp(s1,s2)!=0);
		break;
	case C_GT:
		a= (strcmp(s1,s2)> 0);
		break;
	case C_GE:
		a= (strcmp(s1,s2)>=0);
		break;
	case C_LT:
		a= (strcmp(s1,s2)< 0);
		break;
	case C_LE:
		a= (strcmp(s1,s2)<=0);
		break;
	default:
		ShowError("Script: illegal string operater\n");
		break;
	}
	// clear the stack values
	stack_data[stack_ptr-1].clear();
	stack_data[stack_ptr  ].clear();

	// set the return value of the operation
	if(buf)
	{
		stack_data[stack_ptr-1].type= C_STR;
		stack_data[stack_ptr-1].str = buf;
	}
	else
	{
		stack_data[stack_ptr-1].type= C_INT;
		stack_data[stack_ptr-1].num = a;
	}
}
///////////////////////////////////////////////////////////////////////////////
// Binomial operators (numbers) 二項演算子(数値)
void CScriptEngine::op_2int(int op)
{
	int i2 = GetInt( stack_data[stack_ptr  ] );
	int i1 = GetInt( stack_data[stack_ptr-1] );

	switch(op)
	{
	case C_ADD:
		i1+=i2;
		break;
	case C_SUB:
		i1-=i2;
		break;
	case C_MUL:
	{
		sint64 res = (sint64)i1 * (sint64)i2;
		if (res >  LLCONST( 2147483647 ) )
			i1 = INT_MAX;
		else if (res <  LLCONST(-2147483648) )
			i1 = INT_MIN;
		else
			i1 = (int)res;
		break;
	}
	case C_DIV:
		if(i2!=0)
			i1/=i2;
		else
			i1 = (i1<0)? INT_MIN:INT_MAX; // but no mathematical correct left/right side limiting value
		break;
	case C_MOD:
		if(i2!=0)
			i1%=i2;
		else
			i1 = 0;	// not mathematical correct op
		break;
	case C_AND:
		i1&=i2;
		break;
	case C_OR:
		i1|=i2;
		break;
	case C_XOR:
		i1^=i2;
		break;
	case C_LAND:
		i1=i1&&i2;
		break;
	case C_LOR:
		i1=i1||i2;
		break;
	case C_EQ:
		i1=i1==i2;
		break;
	case C_NE:
		i1=i1!=i2;
		break;
	case C_GT:
		i1=i1>i2;
		break;
	case C_GE:
		i1=i1>=i2;
		break;
	case C_LT:
		i1=i1<i2;
		break;
	case C_LE:
		i1=i1<=i2;
		break;
	case C_R_SHIFT:
		i1=i1>>i2;
		break;
	case C_L_SHIFT:
		i1=i1<<i2;
		break;
	}
	// stack values are ints and don't need clearing

	// set the return value of the operation
	stack_data[stack_ptr-1].type= C_INT;
	stack_data[stack_ptr-1].num = i1;
}
///////////////////////////////////////////////////////////////////////////////
// Binomial Operator 二項演算子
void CScriptEngine::op_2(int op)
{	// 2 input values on the stack result in 1 output value
	stack_ptr--;
	// convert variables
	ConvertName(stack_data[stack_ptr  ]);
	ConvertName(stack_data[stack_ptr-1]);
	// check for string or number operation
	if( stack_data[stack_ptr].isString() || stack_data[stack_ptr-1].isString() )
	{	// at least one string, so let it be string operation
		op_2str(op);
	}
	else
	{	// numbers
		op_2int(op);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Unary Operators 単項演算子
void CScriptEngine::op_1(int op)
{
	ConvertName(stack_data[stack_ptr-1]);
	int i1 = GetInt( stack_data[stack_ptr-1] );
	switch(op){
	case C_NEG:
		i1 = -i1;
		break;
	case C_NOT:
		i1 = ~i1;
		break;
	case C_LNOT:
		i1 = !i1;
		break;
	}
	stack_data[stack_ptr-1].num=i1;
}


///////////////////////////////////////////////////////////////////////////////
// 関数の実行
// run a buildin function
int CScriptEngine::run_func()
{
	int i,start_sp,end_sp,func;

	end_sp=stack_ptr;
	for(i=end_sp-1;i>=0 && stack_data[i].type!=C_ARG;i--);
	if(i==0)
	{
		if(config.error_log)
			ShowError("function not found\n");
		state=END;
		return 0;
	}
	this->start=start_sp=i-1;
	this->end  =end_sp;
	
	func=stack_data[this->start].num;
	if( stack_data[this->start].type!=C_NAME || str_data[func].type!=C_FUNC )
	{
		ShowMessage ("run_func: '"CL_WHITE"%s"CL_RESET"' (type %d) is not function and command!\n",
			str_data[func].string(), str_data[func].type);
		state=END;
		return 0;
	}
#ifdef DEBUG_RUN
	if(config.etc_log) {
		ShowMessage("run_func : %s? (%d(%d))\n",str_buf+str_data[func].str,func,str_data[func].type);
		ShowMessage("stack dump :");
		for(i=0;i<end_sp;++i){
			switch(stack_data[i].type){
			case C_INT:
				ShowMessage("| int(%d)", stack_data[i].num);
				break;
			case C_NAME:
				ShowMessage("| name(%s)",(str_num>(stack_data[i].num&0x00ffffff))?str_buf+str_data[(stack_data[i].num&0x00ffffff)].str:"out of range");
				break;
			case C_ARG:
				ShowMessage("| arg");
				break;
			case C_POS:
				ShowMessage("| pos(%d)",stack_data[i].num);
				break;
			default:
				ShowMessage("| %d,%d",stack_data[i].type,stack_data[i].num);
			}
		}
		ShowMessage("\n");
		ShowMessage("function heap %i..%i\n", start_sp, end_sp);
	}
#endif
	if(str_data[func].func){
		str_data[func].func(*this);
	}
	else
	{
		if(config.error_log)
			ShowError("run_func : %s? (%d(%d))\n",str_data[func].string(),func,str_data[func].type);
		push_val(C_INT,0);
	}
	pop_stack(start_sp, end_sp);

	if(state==RETFUNC)
	{	// ユーザー定義関数からの復帰
		int olddefsp=defsp;
		int i;

		pop_stack(defsp,start_sp);	// 復帰に邪魔なスタック削除
		if( defsp<4 || stack_data[defsp-1].type!=C_RETINFO)
		{
			ShowError("script:run_func(return) return without callfunc or callsub (%i,%i)!\n", defsp, stack_data[defsp-1].type);
			state=END;
		}
		else
		{	// get back the calling heap
			pos	  = GetInt   (stack_data[this->defsp-1]);	// スクリプト位置の復元
			script= GetString(stack_data[this->defsp-2]);	// スクリプトを復元
			i	  = GetInt   (stack_data[this->defsp-4]);	// 引数の数所得
			defsp = GetInt   (stack_data[this->defsp-3]);	// 基準スタックポインタを復元
			// get back the default sp as last item since it is he reference for the get-back
#ifdef DEBUG_RUN
			printf("ret %i %p %i %i\n", pos, script, defsp, i);
#endif
			pop_stack(olddefsp-4-i,olddefsp);		// 要らなくなったスタック(引数と復帰用データ)削除

			state=GOTO;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// スクリプトの実行メイン部分
// main script execution loop
int CScriptEngine::run_main()
{
	if(!script)
	{
		this->state=END;
	}
	else
	{
		int cmdcount=script_config.check_cmdcount;
		int gotocount=script_config.check_gotocount;
		int c=0;
		size_t rerun_pos = this->pos;
		this->state = RUN;
		while( RUN == this->state )
		{
			c = get_com(script, pos);
			switch(c)
			{
			case C_EOL:
			{
				if(stack_ptr!=defsp)
				{	// possibly the function pushed a value on the heap which is not used
					// just clear silently in this case, print error log otherwise
					if(stack_ptr!=defsp+1 && config.error_log)
					{
						ShowMessage("stack_ptr(%d) != default(%d)\n",stack_ptr,defsp);
						//!!
						if(this->nd)
						{
							printf("npc script '%s'/'%s' , map %s, ", nd->name?nd->name:"", nd->exname?nd->exname:"", maps[nd->block_list::m].mapname);
						}
						else
						{
							printf("no npc script, ");
						}
						printf("prog pos (%ld)\n", (unsigned long)pos);
						debug_script(script,((pos>32)?pos-32:0),((pos>32)?32:pos));
					}
					stack_ptr=defsp;
				}
				rerun_pos = pos;
				break;
			}
			case C_INT:
			{
				push_val(C_INT,get_num(script,pos));
				break;
			}
			case C_POS:
			case C_NAME:
			{
				uint32 tmp;
				tmp = (0xFF&script[pos])
					| (0xFF&script[pos+1])<<8
					| (0xFF&script[pos+2])<<16;
				push_val(c,tmp);
				pos+=3;
				break;
			}
			case C_ARG:
			{
				push_val(c,0);
				break;
			}
			case C_STR:
			{
				push_str( C_CONSTSTR, (script+pos));
				while(script[pos++]);
				break;
			}
			case CScriptEngine::C_FUNC:
			{
				run_func();
				if(state==GOTO)
				{
					if( gotocount>0 && (--gotocount)<=0 )
					{
						ShowMessage("run_script: infinity loop func!\n");
						state=END;
					}
					else
					{
						rerun_pos=pos;
						state=RUN;					
					}
				}
				break;
			}
			case C_ADD:
			case C_SUB:
			case C_MUL:
			case C_DIV:
			case C_MOD:
			case C_EQ:
			case C_NE:
			case C_GT:
			case C_GE:
			case C_LT:
			case C_LE:
			case C_AND:
			case C_OR:
			case C_XOR:
			case C_LAND:
			case C_LOR:
			case C_R_SHIFT:
			case C_L_SHIFT:
			{
				op_2(c);
				break;
			}
			case C_NEG:
			case C_NOT:
			case C_LNOT:
			{
				op_1(c);
				break;
			}
			case C_NOP:
			{
				pos++;
				state=END;
				break;
			}
			default:
			{
				if(config.error_log)
					ShowMessage("unknown command : %d @ %d\n",c,pos);
				state=END;
				break;
			}
			}//end switch
			if( cmdcount>0 && (--cmdcount)<=0 )
			{
				ShowMessage("run_script: infinity loop cmd!\n");
				state=END;
			}
		}
		if( state==RERUNLINE )
			pos=rerun_pos;
	}
	return state;
}

///////////////////////////////////////////////////////////////////////////////
// スクリプトの実行
// script entry point
int CScriptEngine::run(const char*rootscript, size_t pos, uint32 rid, uint32 oid)
{
	static CScriptEngine defaultengine;	
	//!! change to nonstatic and give attention to attachrid on a multithread scheme

	static basics::Mutex mx;
	if( rootscript )
	{	// threadlock
		mx.lock();
		map_session_data *localsd = rid?map_session_data::from_blid(rid):NULL;
		npcscript_data *localnd   = oid?npcscript_data::from_blid(oid):NULL;
		CScriptEngine &engine = (localsd)? localsd->ScriptEngine : defaultengine;

		if( rid && !localsd )
		{
			ShowWarning("session rid=%u not available, abort script\n", rid);
		}
		else if( oid && !localnd )
		{
			ShowWarning("npc oid=%u not available, abort script\n", oid);
		}
		else if( engine.script && (engine.script != rootscript || engine.pos != pos) )
		{
//ShowWarning("PlayerScript already started, queueing %p %i %i %i\n", rootscript, pos, rid, oid);
			// will be queued automatically
			new CScriptEngine::CCallScript(engine.queue, rootscript, pos, rid, oid);
		}
		else
		{
//ShowWarning("Script start %p %i %i %i, state in %i\n", rootscript, pos, rid, oid, engine.state);

			if( !engine.script )
			{	// start a new script
				engine.stack_ptr= 0;
				engine.defsp	= 0;
				engine.script	= rootscript;
				engine.pos		= pos;
				engine.npcstate = NONE;
			}
			// else we continue the old one
			engine.sd		= localsd;
			engine.nd		= localnd;

			// start the engine;
			mx.unlock();
			engine.run_main();
			mx.lock();

			if( ENVSWAP==engine.state && engine.sd )
			{	// attach_rid has been called and we need to swap the engine environment
				// local sd is the context where the script was started in (possibly NULL)
				// engine.sd is the context where the script is transfered to
				if(engine.sd->ScriptEngine.isRunning() )
				{	// the other script engine is either running or waiting for data
					// so we need to queue in the complete stack and run parameters

					// will be queued automatically
					new CScriptEngine::CCallStack(engine.sd->ScriptEngine.queue,				// target queue
						engine.script, engine.pos, 
						engine.sd?engine.sd->block_list::id:0, 
						engine.nd?engine.nd->block_list::id:0,									// script data
						engine.defsp, engine.stack_ptr, engine.stack_max, engine.stack_data);	// the stack
					// clear this stack, it has been moved
					engine.stack_ptr = 0;
					engine.stack_max = 0;
					engine.stack_data=NULL;

//ShowWarning("PlayerScript already started, queueing stack for %p %i %i %i\n", rootscript, pos, rid, oid);
				}
				else
				{	// the other script is OFF, so just swap

//ShowWarning("Swap Environment for %p %i %i %i\n", rootscript, pos, rid, oid);
					// swap the stack with the target
					basics::swap(engine.sd->ScriptEngine.stack_max,	engine.stack_max);
					basics::swap(engine.sd->ScriptEngine.stack_ptr,	engine.stack_ptr);
					basics::swap(engine.sd->ScriptEngine.stack_data,engine.stack_data);

					// copy the run parameter
					engine.sd->ScriptEngine.script = engine.script;
					engine.sd->ScriptEngine.pos    = engine.pos;
					engine.sd->ScriptEngine.sd     = engine.sd;
					engine.sd->ScriptEngine.nd     = engine.nd;

					// and run the copied script
					CScriptEngine::run(	engine.sd->ScriptEngine.script, 
										engine.sd->ScriptEngine.pos, 
										engine.sd?engine.sd->block_list::id:0,
										engine.nd?engine.nd->block_list::id:0);
				}

				// and end this script here
				engine.state=END;
			}
			else if( STOP==engine.state || RERUNLINE==engine.state )
			{	// script has stoped to wait for a player response
				if(!engine.sd)
				{
					ShowWarning("Server Script not finished with state 'End'. Terminating.\n");
					engine.state = END;
				}
				else
					engine.state = STOP;
			}

			if( STOP!=engine.state )
			{	// any state other then STOP will terminate the script

				// set the engine to OFF state
				engine.state	= OFF;
				engine.script   = NULL;
				
				// dequeue the next script caller if any
				// will be dequeued automatically
				CScriptEngine::CCallScript* elem = CCallScript::dequeue(engine.queue);
				if(elem)
				{
//ShowWarning("start queued script with %p %i %i %i\n", elem->script, elem->pos, elem->rid, elem->oid);
					// get the stack (if any)
					elem->setStack(engine.defsp, engine.stack_ptr, engine.stack_max, engine.stack_data);
					// run the script
					CScriptEngine::run(elem->script, elem->pos, elem->rid, elem->oid);
					// delete the caller
					delete elem;
				}
			}
			if( OFF==engine.state )// clear
			{
				// send a close button in case of finishing with an open message window,
				// so the client can close it
//!! this might be reconsidered generally since the window might still be open when a new script is dequeued
				if( engine.sd && engine.isMessage() )
				{	
					ShowWarning("script: open textwindow not closed before quiting the script.\n");
					clif_scriptclose(*engine.sd, engine.send_defaultnpc());
					engine.clearMessage();
				}

				// clear the npc, if the default npc was used
				if(engine.npcstate == NPC_DEFAULT)
					engine.send_defaultnpc(false);

				// clear possible eventtimers that have been stoped but not finished
				if(engine.nd)
					engine.nd->eventtimer_clear(engine.sd?engine.sd->block_list::id:0);

				// clear the default engine completely
				// might be not necessary
				//if(!sd)	engine.clear();

				//!! ... other necessary clearing

			}
		}
		// and release the lock
		mx.unlock();
//ShowWarning("script out with state %i\n", engine.state);
	}
	return 0;
}

int CScriptEngine::restart(uint32 npcid)
{	
	if( this->state==STOP && this->sd && this->nd && (npcid == this->nd->block_list::id || npcid == this->defoid) )
		return CScriptEngine::run(this->script, this->pos, this->sd->block_list::id, this->nd->block_list::id);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// buildin functions
// 埋め込み関数
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// script user gui functions
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// user gui message. 
/// display a message, opens a test window if not exist
/// otherwise display text in the existing text window
int buildin_mes(CScriptEngine &st)
{
	if(st.sd)
	{
		clif_scriptmes(*st.sd,st.send_defaultnpc(),st.GetString(st[2]) );
		st.setMessage();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// user gui next button. 
/// display next button on the client.
/// continue the script when button is pressed
/// skip command if no message window exists
int buildin_next(CScriptEngine &st)
{
	if(st.sd && st.isMessage())
	{	// halt the script and run the next command when restarted
		st.Stop();
		clif_scriptnext(*st.sd, st.send_defaultnpc());
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// user gui close button. 
/// display close button which closes the window on the client
/// terminate the script when pressed 
/// or terminate immediately if no message window exists
int buildin_close(CScriptEngine &st)
{
	if(st.sd && st.isMessage() )
	{
		if( st.Rerun() )
		{
			clif_scriptclose(*st.sd, st.send_defaultnpc());
			return 0;
		}
		else
		{	// message window is now closed
			st.clearMessage();
		}
	}
	// always quit here
	st.Quit();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// user gui close button. 
/// displays a close button which closes the window on the client
/// but continue the script when button is pressed,
/// skip the command if no message window exists
int buildin_close2(CScriptEngine &st)
{
	if(st.sd && st.isMessage() )
	{
		if( st.Rerun() )
		{
			clif_scriptclose(*st.sd, st.send_defaultnpc());
		}
		else
		{	// message window is now closed
			st.clearMessage();
		}
	}
	// always continue here
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// user gui selection menu.
/// does a goto command to the label at selected position or quits the script
/// when cancel has been selected. returns nothing
int buildin_menu(CScriptEngine &st)
{
	if(st.sd)
	{
		if( st.Rerun() )
		{
			size_t len,i,c;
			char *buf;
			for(i=st.start+2,len=16;i<st.end;i+=2)
			{
				c=strlen( st.GetString(st.stack_data[i]) );
				if(c) len+=1+c;
			}
			buf= new char[len+1];
			buf[0]=0;
			for(i=st.start+2,len=0;i<st.end;i+=2)
			{
				if( st.stack_data[i].str && *st.stack_data[i].str )
				{
					strcat(buf, st.stack_data[i].str);
					strcat(buf,":");
				}
			}
			clif_scriptmenu(*st.sd, st.send_defaultnpc(), buf);
			delete[] buf;
		}
		else
		{
			size_t menu = 0xFF & st.GetInt( st.cExtData );
			if( menu>0 && menu<(st.end-st.start)/2 && st[menu*2+1].type==CScriptEngine::C_POS )
			{	// goto動作
				// ragemu互換のため
				pc_setreg(*st.sd,add_str("l15"),menu);
				pc_setreg(*st.sd,add_str("@menu"),menu);
				// do the goto
				st.Goto( st[menu*2+1].num );
				// and
			}
			else 
			{	// canceled or wrong values
				if(menu!=0xFF)
				{	// some error when not canceled
					ShowMessage("script: menu point %i is not a label !\n", menu);
					st.clearMessage();
				}
				// quit the script
				st.Quit();
			}
		}
	}
	else
	{	// menu without player is lame
		st.Quit();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// user gui selection menu.
/// returns number of selected entry, -1 when cancel has been pressed
int buildin_select(CScriptEngine &st)
{
	if(st.sd)
	{
		if( st.Rerun() )
		{
			char *buf;
			size_t len,i,c;
			for(i=st.start+2,len=16;i<st.end;++i)
			{	
				c = strlen( st.GetString(st.stack_data[i]) );
				if(c) len+=1+c;
			}
			buf = new char[len+1];
			buf[0]=0;
			for(i=st.start+2,len=0;i<st.end;++i)
			{
				if( st.stack_data[i].str && *st.stack_data[i].str )
				{
					strcat(buf, st.stack_data[i].str);
					strcat(buf,":");
				}
			}
			clif_scriptmenu(*st.sd, st.send_defaultnpc(), buf);
			delete[] buf;
		}
		else
		{	// client only sends a byte, so onyl use a byte
			int menu = 0xFF & st.GetInt( st.cExtData );
			if(menu==0xFF)
			{	// cancel
				menu = -1;
			}
			pc_setreg(*st.sd,add_str( "l15"),menu);
			pc_setreg(*st.sd,add_str( "@menu"),menu);
			st.push_val(CScriptEngine::C_INT,menu);
		}
	}
	else
	{	// select without a player is lame
		st.Quit();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// user gui input box.
/// returns entered string/number depending on the type of given variable.
int buildin_input(CScriptEngine &st)
{
	if( st.sd )
	{
		int num=(st.end>st.start+2)?st[2].num:0;
		const char *name=(st.end>st.start+2)?str_data[num&0x00ffffff].string():"";
		const char postfix=name[strlen(name)-1];
		if( st.Rerun() )
		{
			uint32 id = st.send_defaultnpc();
			if(postfix=='$')clif_scriptinputstr(*st.sd, id);
			else			clif_scriptinput(*st.sd, id);
		}
		else
		{
			if( postfix=='$' )
			{	// 文字列
				if(st.end>st.start+2)
				{	// 引数1個
					set_reg(st,num,name,(void*)st.GetString(st.cExtData) );
				}
				else
				{
					ShowError("buildin_input: string discarded !!\n");
				}
			}
			else
			{	// 数値
				if(st.end>st.start+2)
				{	// 引数1個
					set_reg(st,num,name,(void*)((size_t)st.GetInt(st.cExtData)));
				}
				else
				{	// ragemu互換のため
					pc_setreg(*st.sd,add_str( "l14"),st.GetInt(st.cExtData));
				}
			}
		}
	}
	else
	{	// input without a player is lame
		st.Quit();
	}
	return 0;
}



///////////////////////////////////////////////////////////////////////////////
//
// script flow control functions
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// 
///
int buildin_if(CScriptEngine &st)
{
	size_t sel,i;

	sel=st.GetInt(st[2]);
	if(!sel)
		return 0;

	// 関数名をコピー
	st.push_copy(st.start+3);
	// 間に引数マーカを入れて
	st.push_val(CScriptEngine::C_ARG,0);
	// 残りの引数をコピー
	for(i=st.start+4;i<st.end;++i){
		st.push_copy(i);
	}
	st.run_func();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///
///
int buildin_goto(CScriptEngine &st)
{
	if(st[2].type != CScriptEngine::C_POS)
	{
		int func = st.GetInt(st[2]);
		ShowMessage("script: goto '"CL_WHITE"%s"CL_RESET"': not label!\n", str_data[func].string());
		st.Quit();
		return 0;
	}
	st.Goto( st.GetInt(st[2]) );
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///
///
int buildin_jump_zero(CScriptEngine &st)
{
	int sel;
	sel=st.GetInt(st[2]);
	if(!sel)
	{
		if( st[3].type!=CScriptEngine::C_POS )
		{
			ShowMessage("script: jump_zero: not label !\n");
			st.Quit();
		}
		else
		{
			st.Goto( st.GetInt(st[3]) );
			// ShowMessage("script: jump_zero: jumpto : %d\n",pos);
		}
	}
	else
	{
		// ShowMessage("script: jump_zero: fail\n");
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
/// ユーザー定義関数の呼び出し
///
int buildin_callfunc(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	script_object *scr = (script_object *)strdb_search(script_get_userfunc_db(),str);

	if(scr)
	{
		size_t i,j;
		for(i=st.start+3,j=0; i<st.end; ++i,++j)
			st.push_copy(i);

		st.push_val(CScriptEngine::C_INT,j);				// 引数の数をプッシュ
		st.push_val(CScriptEngine::C_INT,st.defsp);			// 現在の基準スタックポインタをプッシュ
		st.push_str(CScriptEngine::C_CONSTSTR,st.script);	// 現在のスクリプトをプッシュ
		st.push_val(CScriptEngine::C_RETINFO,st.pos);		// 現在のスクリプト位置をプッシュ

		st.pos=0;
		st.script=scr->script;
		st.defsp=st.start+4+j;
		st.state=CScriptEngine::GOTO;
	}
	else
	{
		ShowError("script:callfunc: function not found! [%s]\n",str);
		st.Quit();
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
/// サブルーティンの呼び出し
///
int buildin_callsub(CScriptEngine &st)
{
	int pos=st.GetInt(st[2]);
	size_t i,j;
	for(i=st.start+3,j=0;i<st.end;++i,++j)
		st.push_copy(i);

	st.push_val(CScriptEngine::C_INT,j);				// 引数の数をプッシュ
	st.push_val(CScriptEngine::C_INT,st.defsp);			// 現在の基準スタックポインタをプッシュ
	st.push_str(CScriptEngine::C_CONSTSTR,st.script);	// 現在のスクリプトをプッシュ
	st.push_val(CScriptEngine::C_RETINFO,st.pos);		// 現在のスクリプト位置をプッシュ

	st.pos=pos;
	st.defsp=st.start+4+j;
	st.state=CScriptEngine::GOTO;	

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// gets an argument from callfunc.
/// 引数の所得
int buildin_getarg(CScriptEngine &st)
{
	if( st.defsp<4 || st.stack_data[st.defsp-1].type!=CScriptEngine::C_RETINFO )
	{
		ShowError("script:getarg without callfunc or callsub!\n");
		st.Quit();
	}
	else
	{
		size_t num = st.GetInt(st[2]);							// argument number
		size_t max = st.GetInt( st.stack_data[st.defsp-4] );	// length of function heap
		if( num < max )
		{	// push the argument 
			st.push_copy(st.defsp-4-max + num);
//printf("getarg %i: ", num);
//if( st.stack_data[st.stack_ptr-1].isString() ) printf("str '%s'\n", st.stack_data[st.stack_ptr-1].str );
//else printf("int '%i'\n", st.stack_data[st.stack_ptr-1].num );
		}
		else
		{	// out of range
			ShowError("script:getarg arg1(%d) out of range(%d) !\n",num,max);
			// push a default
			st.push_str(CScriptEngine::C_CONSTSTR, "");
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// returns a value from callfunction.
/// サブルーチン/ユーザー定義関数の終了
int buildin_return(CScriptEngine &st)
{
	if( st.Arguments() > 2 )
	{	// 戻り値有り
		st.push_copy(st.start+2);
	}
	st.Return();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// RIDのアタッチ
///
int buildin_attachrid(CScriptEngine &st)
{
	int val = 0;
	uint32 rid = st.GetInt(st[2]);
	map_session_data *sd = map_session_data::from_blid(rid);
	if(sd)
	{	// need to swap the environment if not in the targeted context already
		if(st.sd != sd)
		{
			st.sd = sd;
			st.EnvSwap();
		}
		val=1;
	}
	// otherwise switch was not sucessful, 
	// and we return "false" to the script
	st.push_val(CScriptEngine::C_INT, val);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////
/// RIDのデタッチ
///
int buildin_detachrid(CScriptEngine &st)
{	// just clear the environment, 
	// script will behave like a server script from this point
	// even if it still runs within a player context
	st.sd=NULL;
	return 0;
}




///////////////////////////////////////////////////////////////////////////////
//
// other script functions
//
///////////////////////////////////////////////////////////////////////////////


/*==========================================
 *
 *------------------------------------------
 */
int buildin_viewpoint(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		int x	=st.GetInt(st[3]);
		int y	=st.GetInt(st[4]);
		int id	=st.GetInt(st[5]);
		int color=st.GetInt(st[6]);
		
		clif_viewpoint(*st.sd, st.send_defaultnpc(), type, x, y, id, color);
	}
	return 0;
}
/*==========================================
 * 存在チェック
 *------------------------------------------
 */
int buildin_isloggedin(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, map_session_data::from_blid( st.GetInt(st[2]) )!=NULL );
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_rand(CScriptEngine &st)
{
	int range;
	int min = 0;
	if( st.Arguments() > 3 )
	{	// numbers from #1...#2 (sign independend, including the limits)
		int max;
		min = st.GetInt(st[2]);
		max = st.GetInt(st[3]);
		if(min>max)	basics::swap(max, min);
		range = max - min;
	}
	else
	{	// zero based numbers
		range = st.GetInt(st[2]);
		if(range<0)
		{	// numbers from -#...0
			min = range;
			range = -range;
		}
		// else	numbers from 0...#
	}
	if(range>0)	// include the limits
		min += rand()%(range+1);
	st.push_val(CScriptEngine::C_INT,min);
	return 0;
}
/*==========================================
 * 変数設定
 *------------------------------------------
 */
int buildin_set(CScriptEngine &st)
{
	int num=st[2].num;
	if( (num&0x00ffffff) < str_num )
	{
		const char *name=str_data[num&0x00ffffff].string();
		const char postfix=name[strlen(name)-1];

		if( st[2].type!=CScriptEngine::C_NAME )
		{
			ShowMessage("script: buildin_set: not name\n");
		}
		else if( postfix=='$' )
		{
			// 文字列
			const char *str = st.GetString((st[3]));
			set_reg(st,num,name,(void*)str);
		}
		else
		{
			// 数値
			int val = st.GetInt(st[3]);
			set_reg(st,num,name,(void*)((size_t)val));
		}
	}
	return 0;
}
/*==========================================
 * 配列変数設定
 *------------------------------------------
 */
int buildin_setarray(CScriptEngine &st)
{
	int num=st[2].num;
	if( (num&0x00ffffff) < str_num )
	{
		const char *name=str_data[num&0x00ffffff].string();
		const char prefix=*name;
		const char postfix=name[strlen(name)-1];
		size_t i,j;

		if( prefix!='$' && prefix!='@' )
		{
			ShowMessage("buildin_setarray: illegal scope !\n");
		}
		else
		{
			for(j=0,i=st.start+3; i<st.end && j<128;++i,++j){
				void *v;
				if( postfix=='$' )
					v=(void*)st.GetString(st.stack_data[i]);
				else
					v=(void*)((size_t)st.GetInt(st.stack_data[i]));
				set_reg(st, num+(j<<24), name, v);
			}
		}
	}
	return 0;
}
/*==========================================
 * 配列変数クリア
 *------------------------------------------
 */
int buildin_cleararray(CScriptEngine &st)
{
	int num=st[2].num;
	if( (num&0x00ffffff) < str_num )
	{
		const char *name=str_data[num&0x00ffffff].string();
		const char prefix=*name;
		const char postfix=name[strlen(name)-1];
		int sz=st.GetInt(st[4]);
		int i;
		void *v;

		if( prefix!='$' && prefix!='@' )
		{
			ShowMessage("buildin_cleararray: illegal scope !\n");
		}
		else
		{
			if( postfix=='$' )
				v=(void*)st.GetString((st[3]));
			else
				v=(void*)((size_t)st.GetInt(st[3]));

			for(i=0;i<sz;++i)
				set_reg(st,num+(i<<24),name,v);
		}
	}
	return 0;
}
/*==========================================
 * 配列変数コピー
 *------------------------------------------
 */
int buildin_copyarray(CScriptEngine &st)
{
	int num=st[2].num;
	int num2=st[3].num;
	if( (num&0x00ffffff) < str_num && (num2&0x00ffffff) < str_num )
	{
		const char *name=str_data[num&0x00ffffff].string();
		const char prefix=*name;
		const char postfix=name[strlen(name)-1];
		const char *name2=str_data[num2&0x00ffffff].string();
		const char prefix2=*name2;
		const char postfix2=name2[strlen(name2)-1];
		int sz=st.GetInt(st[4]);
		int i;

		if( (prefix!='$' && prefix!='@') || (prefix2!='$' && prefix2!='@') )
		{
			ShowMessage("buildin_copyarray: illegal scope !\n");
		}
		else if( (postfix=='$' || postfix2=='$') && postfix!=postfix2 )
		{
			ShowMessage("buildin_copyarray: type mismatch !\n");
		}
		else
		{
			for(i=0;i<sz;++i)
				set_reg(st,num+(i<<24),name, get_val2(st,num2+(i<<24)) );
		}
	}
	return 0;
}
/*==========================================
 * 配列変数のサイズ所得
 *------------------------------------------
 */
int getarraysize(CScriptEngine &st,int num,int postfix)
{
	int i=(num>>24),c=i;
	for(;i<128;++i){
		void *v=get_val2(st,num+(i<<24));
		if(postfix=='$' && *((char*)v) ) c=i;
		if(postfix!='$' && (size_t)v )c=i;
	}
	return c+1;
}
int buildin_getarraysize(CScriptEngine &st)
{
	int num=st[2].num;
	if( (num&0x00ffffff) < str_num )
	{
		const char *name=str_data[num&0x00ffffff].string();
		const char prefix=*name;
		const char postfix=name[strlen(name)-1];

		if( prefix!='$' && prefix!='@' )
		{
			ShowMessage("buildin_copyarray: illegal scope !\n");
		}
		else
		{
			st.push_val(CScriptEngine::C_INT,getarraysize(st,num,postfix) );
		}
	}
	return 0;
}
/*==========================================
 * 配列変数から要素削除
 *------------------------------------------
 */
int buildin_deletearray(CScriptEngine &st)
{
	int num=st[2].num;
	if( (num&0x00ffffff) < str_num )
	{
		const char *name=str_data[num&0x00ffffff].string();
		const char prefix=*name;
		const char postfix=name[strlen(name)-1];
		int count=1;
		int i,sz=getarraysize(st,num,postfix)-(num>>24)-count+1;


		if( st.Arguments() > 3 )
			count=st.GetInt(st[3]);

		if( prefix!='$' && prefix!='@' )
		{
			ShowMessage("buildin_deletearray: illegal scope !\n");
		}
		else
		{
			for(i=0;i<sz;++i){
				set_reg(st,num+(i<<24),name, get_val2(st,num+((i+count)<<24) ) );
			}
			for(;i<(128-(num>>24));++i){
				if( postfix!='$' ) set_reg(st,num+(i<<24),name, 0);
				if( postfix=='$' ) set_reg(st,num+(i<<24),name, (void *) "");
			}
		}
	}
	return 0;
}

/*==========================================
 * 指定要素を表す値(キー)を所得する
 *------------------------------------------
 */
int buildin_getelementofarray(CScriptEngine &st)
{
	if( st[2].type==CScriptEngine::C_NAME )
	{
		int i=st.GetInt(st[3]);
		if(i>127 || i<0)
		{
			ShowMessage("script: getelementofarray (operator[]): param2 illegal number %d\n",i);
			st.push_val(CScriptEngine::C_INT,0);
		}
		else
		{
			st.push_val(CScriptEngine::C_NAME, (i<<24) | st[2].num );
		}
	}
	else
	{
		ShowMessage("script: getelementofarray (operator[]): param1 not name !\n");
		st.push_val(CScriptEngine::C_INT,0);
	}
	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
int buildin_warp(CScriptEngine &st)
{
	const char *str=st.GetString((st[2]));
	int x=st.GetInt(st[3]);
	int y=st.GetInt(st[4]);

	if(st.sd==NULL || str==NULL)
	{
		;//
	}
	else if( 0==strcmp(str,"Random") )
	{
		pc_randomwarp(*st.sd,3);
	}
	else if( 0==strcmp(str,"SavePoint") )
	{
		if( !maps[st.sd->block_list::m].flag.noreturn )	// 蝶禁止
			pc_setpos(*st.sd,st.sd->status.save_point.mapname,st.sd->status.save_point.x,st.sd->status.save_point.y,3);
	}
	else if( 0==strcmp(str,"Save") )
	{
		if( !maps[st.sd->block_list::m].flag.noreturn )	// 蝶禁止
			pc_setpos(*st.sd,st.sd->status.save_point.mapname,st.sd->status.save_point.x,st.sd->status.save_point.y,3);
	}
	else
	{
		pc_setpos(*st.sd,str,x,y,0);
	}
	return 0;
}
/*==========================================
 * エリア指定ワープ
 *------------------------------------------
 */
class CBuildinAreawarpXY : public CMapProcessor
{
	const char*& map;
	ushort x;
	ushort y;
public:
	CBuildinAreawarpXY(const char*&m, ushort xx, ushort yy)
		: map(m), x(xx), y(yy)
	{}
	~CBuildinAreawarpXY()	{}
	virtual int process(block_list& bl) const
	{
		map_session_data *sd = bl.get_sd();
		if(sd)
		{
			pc_setpos(*sd,map,x,y,0);
			return 1;
		}
		return 0;
	}
};
class CBuildinAreawarpRnd : public CMapProcessor
{
	ushort x;
	ushort y;
public:
	CBuildinAreawarpRnd()	{}
	~CBuildinAreawarpRnd()	{}
	virtual int process(block_list& bl) const
	{
		map_session_data *sd = bl.get_sd();
		if(sd)
		{
			pc_randomwarp(*sd,3);
			return 1;
		}
		return 0;
	}
};

int buildin_areawarp(CScriptEngine &st)
{
	const char *mapname=st.GetString((st[2]));
	int x0=st.GetInt(st[3]);
	int y0=st.GetInt(st[4]);
	int x1=st.GetInt(st[5]);
	int y1=st.GetInt(st[6]);
	const char *targetmap=st.GetString((st[7]));
	int x=st.GetInt(st[8]);
	int y=st.GetInt(st[9]);
	int m=map_mapname2mapid(mapname);

	if( m>=0 && (size_t)m<map_num)
	{	//!! broadcast command if not on this mapserver

		if( 0==strcmp(targetmap,"Random") )
			block_list::foreachinarea( CBuildinAreawarpRnd(),
				m,x0,y0,x1,y1,BL_PC);
		else
			block_list::foreachinarea( CBuildinAreawarpXY(targetmap,x,y),
				m,x0,y0,x1,y1,BL_PC);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_heal(CScriptEngine &st)
{
	if(st.sd)
	{
		int hp=st.GetInt(st[2]);
		int sp=st.GetInt(st[3]);
		st.sd->heal(hp,sp);
	}
	
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_itemheal(CScriptEngine &st)
{
	if(st.sd)
	{
		int hp=st.GetInt(st[2]);
		int sp=st.GetInt(st[3]);
		pc_itemheal(*st.sd,hp,sp);
	}
	
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_percentheal(CScriptEngine &st)
{
	if(st.sd)
	{
		int hp=st.GetInt(st[2]);
		int sp=st.GetInt(st[3]);
		pc_percentheal(*st.sd,hp,sp);
	}
	
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_jobchange(CScriptEngine &st)
{
	if(st.sd)
	{
		int upper=-1;
		int job = st.GetInt(st[2]);
		if( st.Arguments() > 3 )
			upper=st.GetInt(st[3]);
		if( job>=0 && job<MAX_PC_CLASS )
			pc_jobchange(*st.sd,job, upper);
	}
	
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_setlook(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		int val =st.GetInt(st[3]);
		pc_changelook(*st.sd,type,val);
	}
	
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_cutin(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[3]);
		clif_cutin(*st.sd, st.GetString(st[2]), type);
	}
	return 0;
}
/*==========================================
 * カードのイラストを表示する
 *------------------------------------------
 */
int buildin_cutincard(CScriptEngine &st)
{
	if(st.sd)
	{
		int itemid =st.GetInt(st[2]);
		struct item_data* idata = itemdb_exists(itemid);
		if(idata)
			clif_cutin(*st.sd, idata->cardillustname, 4);
	}
	return 0;
}



/*==========================================
 *
 *------------------------------------------
 */
int buildin_countitem(CScriptEngine &st)
{
	unsigned short nameid=0;
	size_t count=0, i;
	if(st.sd)
	{
		CScriptEngine::CValue &data = st[2];
		st.ConvertName(data);

		if( data.isString() )
		{
			const char *name=st.GetString(data);
			struct item_data *item_data;
			if( (item_data = itemdb_searchname(name)) != NULL)
				nameid=item_data->nameid;
		}
		else
			nameid=st.GetInt(data);
		
		if(nameid>=500 && nameid<MAX_ITEMS)
		{
			for(i=0;i<MAX_INVENTORY;++i)
			{
				if(st.sd->status.inventory[i].nameid==nameid)
					count+=st.sd->status.inventory[i].amount;
			}
		}
		else if(config.error_log)
			ShowMessage("wrong item ID : countitem(%i)\n",nameid);
	}
	st.push_val(CScriptEngine::C_INT,count);
	return 0;
}

/*==========================================
 * 重量チェック
 *------------------------------------------
 */
int buildin_checkweight(CScriptEngine &st)
{
	int val = 1;
	if(st.sd)
	{
		unsigned short nameid=0, amount;
		CScriptEngine::CValue &data= st[2];
		st.ConvertName(data);
		if( data.isString() )
		{
			const char *name=st.GetString(data);
			struct item_data *item_data = itemdb_searchname(name);
			if( item_data )
				nameid=item_data->nameid;
		}
		else
			nameid=st.GetInt(data);

		amount = st.GetInt(st[3]);
		if( amount<MAX_AMOUNT && nameid>=500 && nameid<MAX_ITEMS)
		{
			if( itemdb_weight(nameid)*amount + st.sd->weight <= st.sd->max_weight )
				val = 0;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_getitem(CScriptEngine &st)
{
	//!! some stupid game with signs
	short nameid,nameidsrc,amount;
	int flag = 0;
	struct item item_tmp;
	CScriptEngine::CValue &data=st[2];
	map_session_data *sd = st.sd;

	st.ConvertName(data);
	if( data.isString() )
	{
		const char *name=st.GetString(data);
		struct item_data *item_data = itemdb_searchname(name);
		nameid=512; //Apple item ID
		if( item_data != NULL)
			nameid=item_data->nameid;
	}else
		nameid=st.GetInt(data);

	if ( ( amount=st.GetInt( (st[3])) ) <= 0) {
		return 0; //return if amount <=0, skip the useles iteration
	}
	//Violet Box, Blue Box, etc - random item pick
	nameidsrc = nameid;
	if( sd && nameid<0 )
	{	// Save real ID of the source Box [Lupus]
		nameid=itemdb_searchrandomid(-nameid);

		if(log_config.present > 0)
			log_present(*sd, -nameidsrc, nameid); //fixed missing ID by Lupus
		flag = 1;
	}

	if(nameid > 0)
	{
		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid=nameid;
		if(!flag)
			item_tmp.identify=1;
		else
			item_tmp.identify = !itemdb_isEquipment(nameid);
		if( st.Arguments() > 5 ) //アイテムを指定したIDに渡す
			sd=map_session_data::from_blid(st.GetInt( (st[5])));
		if(sd == NULL) //アイテムを渡す相手がいなかったらお帰り
			return 0;
		if((flag = pc_additem(*sd,item_tmp,amount)))
		{	// additem failed
			clif_additem(*sd,0,0,flag);
			// create it on floor if dropable, let it vanish otherwise
			if( itemdb_isdropable(nameid, sd->isGM()) )
				map_addflooritem(item_tmp,amount,sd->block_list::m,sd->block_list::x,sd->block_list::y,NULL,NULL,NULL,0);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_getitem2(CScriptEngine &st)
{
	//!! some stupid game with signs
	short nameid,amount;
	int flag = 0;
	int iden,ref,attr,c1,c2,c3,c4;
	struct item_data *item_data;
	struct item item_tmp;
	CScriptEngine::CValue &data= st[2];
	map_session_data *sd = st.sd;

	
	st.ConvertName(data);
	if( data.isString() ){
		const char *name=st.GetString(data);
		struct item_data *item_data = itemdb_searchname(name);
		nameid=512; //Apple item ID
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=st.GetInt(data);

	amount=st.GetInt(st[3]);
	iden=st.GetInt(st[4]);
	ref=st.GetInt(st[5]);
	attr=st.GetInt(st[6]);
	c1=st.GetInt(st[7]);
	c2=st.GetInt(st[8]);
	c3=st.GetInt(st[9]);
	c4=st.GetInt(st[10]);
	if( st.Arguments() > 11 ) //アイテムを指定したIDに渡す
		sd=map_session_data::from_blid( st.GetInt(st[11]) );
	if(sd == NULL) //アイテムを渡す相手がいなかったらお帰り
		return 0;

	if(nameid<0) { // ランダム
		nameid=itemdb_searchrandomid(-nameid);
		flag = 1;
	}

	if(nameid > 0) {
		memset(&item_tmp,0,sizeof(item_tmp));
		item_data=itemdb_search(nameid);
		if(item_data->type==4 || item_data->type==5){
			if(ref > 10) ref = 10;
		}
		else if(item_data->type==7) {
			iden = 1;
			ref = 0;
		}
		else {
			iden = 1;
			ref = attr = 0;
		}

		item_tmp.nameid=nameid;
		if(!flag)
			item_tmp.identify=iden;
		else if(item_data->type==4 || item_data->type==5)
			item_tmp.identify=0;
		item_tmp.refine=ref;
		item_tmp.attribute=attr;
		item_tmp.card[0]=c1;
		item_tmp.card[1]=c2;
		item_tmp.card[2]=c3;
		item_tmp.card[3]=c4;
		if((flag = pc_additem(*sd,item_tmp,amount))) {
			clif_additem(*sd,0,0,flag);
			map_addflooritem(item_tmp,amount,sd->block_list::m,sd->block_list::x,sd->block_list::y,NULL,NULL,NULL,0);
		}
	}

	return 0;
}

/*==========================================
 * gets an item with someone's name inscribed [Skotlex]
 * getinscribeditem item_num, character_name
 * Returned Qty is always 1, only works on equip-able
 * equipment
 *------------------------------------------
 */
int buildin_getnameditem(CScriptEngine &st)
{
	int nameid, type;
	struct item item_tmp;
	map_session_data *tsd;
	CScriptEngine::CValue &data = st[2];

	if(st.sd == NULL)
	{	//Player not attached!
		st.push_val(CScriptEngine::C_INT,0);
		return 0; 
	}
	
	st.ConvertName(data);
	if( data.isString() ){
		const char *name=st.GetString(data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data == NULL)
		{	//Failed
			st.push_val(CScriptEngine::C_INT,0);
			return 0;
		}
		nameid = item_data->nameid;
		type   = item_data->type;
	}
	else
	{
		nameid = st.GetInt(data);
		struct item_data *item_data = itemdb_exists(nameid);
		type   = (item_data) ? item_data->type : 0;
	}

	if(!itemdb_exists(nameid) || !itemdb_isEquipment(nameid))
	{	//We don't allow non-equipable/stackable items to be named
		//to avoid any qty exploits that could happen because of it.
		st.push_val(CScriptEngine::C_INT,0);
		return 0;
	}

	data = st[3];
	st.ConvertName(data);
	if( data.isString() )	//Char Name
		tsd=map_session_data::nick2sd(st.GetString(data));
	else	//Char Id was given
		tsd=map_session_data::charid2sd(st.GetInt(data));
	
	if( tsd == NULL )
	{	//Failed
		st.push_val(CScriptEngine::C_INT,0);
		return 0;
	}

	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid=nameid;
	item_tmp.amount=1;
	item_tmp.identify=1;
	item_tmp.card[0]=(type==4) ? 0xff : 0xfe;
	item_tmp.card[1]=0;
	item_tmp.card[2]=tsd->status.char_id;
	item_tmp.card[3]=tsd->status.char_id >> 16;
	if(pc_additem(*st.sd,item_tmp,1)) {
		st.push_val(CScriptEngine::C_INT,0);
		return 0;	//Failed to add item, we will not drop if they don't fit
	}

	st.push_val(CScriptEngine::C_INT,1);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_makeitem(CScriptEngine &st)
{
	//!! some stupid game with signs
	short nameid, amount;
	int flag = 0;
	int x,y,m;
	const char *mapname;
	struct item item_tmp;

	CScriptEngine::CValue &data=st[2];
	st.ConvertName(data);
	if( data.isString() )
	{
		const char *name=st.GetString(data);
		struct item_data *item_data = itemdb_searchname(name);
		nameid=512; //Apple Item ID
		if( item_data )
			nameid=item_data->nameid;
	}
	else
		nameid=st.GetInt(data);

	amount=st.GetInt(st[3]);
	mapname	=st.GetString((st[4]));
	x	=st.GetInt(st[5]);
	y	=st.GetInt(st[6]);

	if( st.sd && strcmp(mapname,"this")==0)
		m=st.sd->block_list::m;
	else
		m=map_mapname2mapid(mapname);

	if(nameid<0) { // ランダム
		nameid=itemdb_searchrandomid(-nameid);
		flag = 1;
	}

	if(nameid > 0) {
		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid=nameid;
		if(!flag)
			item_tmp.identify=1;
		else
			item_tmp.identify = !itemdb_isEquipment(nameid);
//		clif_additem(sd,0,0,flag);
		map_addflooritem(item_tmp,amount,m,x,y,NULL,NULL,NULL,0);
	}

	return 0;
}
/*==========================================
 * script DELITEM command (fixed 2 bugs by Lupus, added deletion priority by Lupus)
 *------------------------------------------
 */
int buildin_delitem(CScriptEngine &st)
{
	unsigned short nameid=0,amount;
	int i,important_item=0;
	CScriptEngine::CValue &data = st[2];
	st.ConvertName(data);

	if( data.isString() )
	{
		const char *name=st.GetString(data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid=item_data->nameid;
	}
	else
		nameid=st.GetInt(data);

	amount=st.GetInt(st[3]);

	if(st.sd && nameid>500 && nameid<20000 && amount<=MAX_AMOUNT)
	{	//by Lupus. Don't run FOR if u got wrong item ID or amount<=0
		
		//1st pass
		//here we won't delete items with CARDS, named items but we count them
		for(i=0;i<MAX_INVENTORY && amount>0; ++i)
		{	//we don't delete wrong item or equipped item
			if( st.sd->status.inventory[i].nameid!=nameid || st.sd->inventory_data[i] == NULL ||
				st.sd->status.inventory[i].nameid>=20000  || st.sd->status.inventory[i].amount>=MAX_AMOUNT )
				continue;

			//1 egg uses 1 cell in the inventory. so it's ok to delete 1 pet / per cycle
			if(st.sd->inventory_data[i]->type==7 && st.sd->status.inventory[i].card[0] == 0xff00 && search_petDB_index(nameid, PET_EGG) >= 0 )
			{
				intif_delete_petdata( basics::MakeDWord(st.sd->status.inventory[i].card[1], st.sd->status.inventory[i].card[2]) );
				//clear egg flag. so it won't be put in IMPORTANT items (eggs look like item with 2 cards ^_^)
				st.sd->status.inventory[i].card[1] = st.sd->status.inventory[i].card[0] = 0;
				//now this egg'll be deleted as a common unimportant item
			}
			//is this item important? does it have cards? or Player's name? or Refined/Upgraded
			if( st.sd->status.inventory[i].card[0] || st.sd->status.inventory[i].card[1] ||
				st.sd->status.inventory[i].card[2] || st.sd->status.inventory[i].card[3] || st.sd->status.inventory[i].refine)
			{
				//this is important item, count it
				important_item++;
				continue;
			}
			if(st.sd->status.inventory[i].amount>=amount)
			{
				pc_delitem(*st.sd,i,amount,0);
				return 0; //we deleted exact amount of items. now exit
			}
			else
			{
				amount-=st.sd->status.inventory[i].amount;
				pc_delitem(*st.sd,i,st.sd->status.inventory[i].amount,0);
			}
		}
		//2nd pass
		//now if there WERE items with CARDs/REFINED/NAMED... and if we still have to delete some items. we'll delete them finally
		if (important_item>0 && amount>0)
		{
			for(i=0;i<MAX_INVENTORY && amount>0;++i)
			{
				//we don't delete wrong item
				if( st.sd->status.inventory[i].nameid!=nameid || st.sd->inventory_data[i] == NULL ||
					st.sd->status.inventory[i].nameid>=20000  || st.sd->status.inventory[i].amount>=MAX_AMOUNT )
					continue;

				if(st.sd->status.inventory[i].amount>=amount)
				{
					pc_delitem(*st.sd,i,amount,0);
					return 0; //we deleted exact amount of items. now exit
				}
				else
				{
					amount-=st.sd->status.inventory[i].amount;
					pc_delitem(*st.sd,i,st.sd->status.inventory[i].amount,0);
				}
			}
		}
		if(amount>0) 
			ShowWarning("delitem (item %i) on player %s failed, missing %i pieces\n", nameid, st.sd->status.name, amount);
	}
	return 0;
}

/*==========================================
 *キャラ関係のパラメータ取得
 *------------------------------------------
 */
int buildin_readparam(CScriptEngine &st)
{
	int type;
	map_session_data *sd;

	type=st.GetInt(st[2]);
	if( st.Arguments() > 3 )
		sd=map_session_data::nick2sd(st.GetString((st[3])));
	else
		sd=st.sd;

	st.push_val(CScriptEngine::C_INT, (sd) ? pc_readparam(*sd,type) : -1);
	return 0;
}
/*==========================================
 *キャラ関係のID取得
 *------------------------------------------
 */
int buildin_getcharid(CScriptEngine &st)
{
	int num, val=0;
	map_session_data *sd;

	num=st.GetInt(st[2]);
	if( st.Arguments() > 3 )
		sd=map_session_data::nick2sd(st.GetString((st[3])));
	else
		sd=st.sd;

	if(sd)
	{
		if(num==0) val = sd->status.char_id;
		else if(num==1) val = sd->status.party_id;
		else if(num==2) val = sd->status.guild_id;
		else if(num==3) val = sd->status.account_id;
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}
/*==========================================
 *指定IDのPT名取得
 *------------------------------------------
 */
char *buildin_getpartyname_sub(uint32 party_id)
{
	struct party *p;

	p=NULL;
	p=party_search(party_id);

	if(p!=NULL){
		char *buf;
		buf=new char[24];
		memcpy(buf, p->name, 24);
		return buf;
	}
	return NULL;
}
int buildin_getpartyname(CScriptEngine &st)
{
	char *name;
	int party_id;

	party_id=st.GetInt(st[2]);
	name=buildin_getpartyname_sub(party_id);
	if(name!=0)
		st.push_str(CScriptEngine::C_STR, name);
	else
		st.push_str(CScriptEngine::C_CONSTSTR, "(not in party)");

	return 0;
}
/*==========================================
 *指定IDのPT人数とメンバーID取得
 *------------------------------------------
 */
int buildin_getpartymember(CScriptEngine &st)
{
	int i,j=0;
	struct party *p=party_search(st.GetInt( st[2]));
	if(p!=NULL)
	{
		for(i=0;i<MAX_PARTY;++i)
		{
			if(p->member[i].account_id)
			{
//				ShowMessage("name:%s %d\n",p->member[i].name,i);
				mapreg_setregstr(add_str("$@partymembername$")+(i<<24),p->member[i].name);
				j++;
			}
		}
	}
	mapreg_setregnum(add_str( "$@partymembercount"),j);

	return 0;
}
/*==========================================
 *指定IDのギルド名取得
 *------------------------------------------
 */
char *buildin_getguildname_sub(int guild_id)
{
	struct guild *g=NULL;
	g=guild_search(guild_id);

	if(g!=NULL)
	{
		char *buf = new char[24];
		memcpy(buf, g->name, 24);
		return buf;
	}
	return 0;
}
int buildin_getguildname(CScriptEngine &st)
{
	char *name;
	int guild_id=st.GetInt(st[2]);
	name=buildin_getguildname_sub(guild_id);
	if(name!=0)
		st.push_str(CScriptEngine::C_STR, name);
	else
		st.push_str(CScriptEngine::C_CONSTSTR, "(not in guild)");
	return 0;
}

/*==========================================
 *指定IDのGuildMaster名取得
 *------------------------------------------
 */
char *buildin_getguildmaster_sub(int guild_id)
{
	struct guild *g=NULL;
	g=guild_search(guild_id);
	if(g!=NULL){
		char *buf;
		buf= new char[24];
		memcpy(buf,g->master, 24);//EOS included
		return buf;
	}
	return NULL;
}
int buildin_getguildmaster(CScriptEngine &st)
{
	char *master;
	int guild_id=st.GetInt(st[2]);
	master=buildin_getguildmaster_sub(guild_id);
	if(master!=0)
		st.push_str(CScriptEngine::C_STR, master);
	else
		st.push_str(CScriptEngine::C_CONSTSTR, "(not available)");
	return 0;
}

int buildin_getguildmasterid(CScriptEngine &st)
{
	char *master;
	map_session_data *sd=NULL;
	int val=0;
	int guild_id=st.GetInt(st[2]);
	master=buildin_getguildmaster_sub(guild_id);
	if( master &&  (sd=map_session_data::nick2sd(master)) != NULL )
		val = sd->status.char_id;

	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 * キャラクタの名前
 *------------------------------------------
 */
int buildin_strcharinfo(CScriptEngine &st)
{
	char *buf=NULL;
	int num=st.GetInt(st[2]);

	if(st.sd==NULL)
	{	// nothing
	}
	else if(num==0)
	{
		buf= new char[24];
		memcpy(buf,st.sd->status.name, 24);//EOS included	
	}
	else if(num==1)
		buf=buildin_getpartyname_sub(st.sd->status.party_id);
	else if(num==2)
		buf=buildin_getguildname_sub(st.sd->status.guild_id);

	if(buf)
		st.push_str(CScriptEngine::C_STR, buf);
	else
		st.push_str(CScriptEngine::C_CONSTSTR, "");
	return 0;
}

int buildin_pcstrcharinfo(CScriptEngine &st)
{
	char *buf=NULL;
	int aid=st.GetInt(st[2]);
	int num=st.GetInt(st[3]);
	map_session_data *sd=map_session_data::from_blid(aid);

	if(sd==NULL)
	{	// nothing
	}
	else if(num==0)
	{
		buf=new char[24];
		safestrcpy(buf,24, sd->status.name);
		st.push_str(CScriptEngine::C_STR,buf);
	}
	else if(num==1)
		buf=buildin_getpartyname_sub(sd->status.party_id);
	else if(num==2)
		buf=buildin_getguildname_sub(sd->status.guild_id);

	st.push_str( (buf)?CScriptEngine::C_STR:CScriptEngine::C_CONSTSTR, (buf)?buf:"" );
	return 0;
}

unsigned short equip[10]={0x0100,0x0010,0x0020,0x0002,0x0004,0x0040,0x0008,0x0080,0x0200,0x0001};

/*==========================================
 * GetEquipID(Pos);     Pos: 1-10
 *------------------------------------------
 */
int buildin_getequipid(CScriptEngine &st)
{
	unsigned short itempos,num;
	map_session_data *sd = st.sd;
	struct item_data* item;
	int val = -1;

	if(sd)
	{
		num = st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY)
			{
				item=sd->inventory_data[itempos];
				val = (item) ? item->nameid : 0;
			}
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 * 装備名文字列（精錬メニュー用）
 *------------------------------------------
 */
int buildin_getequipname(CScriptEngine &st)
{
	map_session_data *sd=st.sd;
	struct item_data* item;
	unsigned short num, itempos;
	char *buf = NULL;
	if(sd)
	{
		num = st.GetInt(st[2]) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			buf = new char[128];
			if(itempos < MAX_INVENTORY && (item=sd->inventory_data[itempos])!=NULL)
				snprintf(buf,128,"%s-[%s+%i]",positions[num],item->jname, sd->status.inventory[itempos].refine);
			else
				snprintf(buf,128,"%s-[%s]",positions[num],positions[10]);
		}
	}
	st.push_str((buf)?CScriptEngine::C_STR:CScriptEngine::C_CONSTSTR, (buf)?buf:"");
	return 0;
}

/*==========================================
 * getbrokenid [Valaris]
 *------------------------------------------
 */
int buildin_getbrokenid(CScriptEngine &st)
{
	unsigned short itempos;
	size_t count, brokencounter=0;
	map_session_data *sd;
	int itemid=0;

	sd=st.sd;
	if(sd)
	{
		count = st.GetInt(st[2]);
		for(itempos=0; itempos<MAX_INVENTORY; itempos++)
		{
			if(sd->status.inventory[itempos].attribute==1)
			{
				brokencounter++;
				if(count==brokencounter)
				{
					itemid=sd->status.inventory[itempos].nameid;
					break;
				}
			}
		}
	}
	st.push_val(CScriptEngine::C_INT,itemid);
	return 0;
}

/*==========================================
 * repair [Valaris]
 *------------------------------------------
 */
int buildin_repair(CScriptEngine &st)
{
	unsigned short itempos;
	size_t count, repaircounter=0;
	map_session_data *sd=st.sd;

	if(sd)
	{
		count=st.GetInt(st[2]);
		for(itempos=0; itempos<MAX_INVENTORY; itempos++)
		{
			if(sd->status.inventory[itempos].attribute==1)
			{
				repaircounter++;
				if(count==repaircounter)
				{
					sd->status.inventory[itempos].attribute=0;
					clif_equiplist(*sd);
					clif_produceeffect(*sd, sd->status.inventory[itempos].nameid, 0);
					clif_misceffect(*sd, 3);
					clif_displaymessage(sd->fd,"Item has been repaired.");
					break;
				}
			}
		}
	}
	return 0;
}

/*==========================================
 * 装備チェック
 *------------------------------------------
 */
int buildin_getequipisequiped(CScriptEngine &st)
{
	unsigned short itempos,num;
	map_session_data *sd=st.sd;
	int val=0;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY)
				val = 1;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);

	return 0;
}

/*==========================================
 * 装備品精錬可能チェック
 *------------------------------------------
 */
int buildin_getequipisenableref(CScriptEngine &st)
{
	unsigned short itempos,num;
	map_session_data *sd = st.sd;
	int val=0;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if( itempos<MAX_INVENTORY && num<7 && 
				sd->inventory_data[itempos] && !sd->inventory_data[itempos]->flag.no_refine)
				val = 1;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 * 装備品鑑定チェック
 *------------------------------------------
 */
int buildin_getequipisidentify(CScriptEngine &st)
{
	unsigned short itempos, num;
	map_session_data *sd = st.sd;
	int val = 0;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY)
				val = sd->status.inventory[itempos].identify;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 * 装備品精錬度
 *------------------------------------------
 */
int buildin_getequiprefinerycnt(CScriptEngine &st)
{
	unsigned short itempos,num;
	map_session_data *sd=st.sd;
	int val = 0;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY)
				val = sd->status.inventory[itempos].refine;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 * 装備品武器LV
 *------------------------------------------
 */
int buildin_getequipweaponlv(CScriptEngine &st)
{
	unsigned short itempos,num;
	map_session_data *sd=st.sd;
	int val = 0;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY && sd->inventory_data[itempos])
				val = sd->inventory_data[itempos]->wlv;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 * 装備品精錬成功率
 *------------------------------------------
 */
int buildin_getequippercentrefinery(CScriptEngine &st)
{
	unsigned short itempos,num;
	map_session_data *sd = st.sd;
	int val = 0;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY)
				val = status_percentrefinery(*sd,sd->status.inventory[itempos]);
		}
	}
	st.push_val(CScriptEngine::C_INT,val);

	return 0;
}

/*==========================================
 * 精錬成功
 *------------------------------------------
 */
int buildin_successrefitem(CScriptEngine &st)
{
	unsigned short itempos,num, equippos;
	map_session_data *sd=st.sd;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY)
			{
				equippos=sd->status.inventory[itempos].equip;
				if(log_config.refine > 0)
					log_refine(*sd, itempos, 1);
				
				sd->status.inventory[itempos].refine++;
				pc_unequipitem(*sd,itempos,2);
				clif_refine(sd->fd,*sd,0,itempos,sd->status.inventory[itempos].refine);
				clif_delitem(*sd,itempos,1);
				clif_additem(*sd,itempos,1,0);
				pc_equipitem(*sd,itempos,equippos);
				clif_misceffect(*sd,3);
				if( sd->status.inventory[itempos].refine == 10 && 
					sd->status.inventory[itempos].card[0] == 0x00ff && 
					sd->status.char_id == basics::MakeDWord(sd->status.inventory[itempos].card[2],sd->status.inventory[itempos].card[3]) &&
					sd->inventory_data[itempos]->wlv>0 && sd->inventory_data[itempos]->wlv<4 )
				{	// Fame point system [DracoRPG]
					static const ushort fame_points[4] = {0,1,25,1000};
					chrif_updatefame(*sd, FAME_SMITH, fame_points[sd->inventory_data[itempos]->wlv]);
					// Success to refine to +10 a lv1 weapon you forged = +1 fame point
					// Success to refine to +10 a lv2 weapon you forged = +25 fame point
					// Success to refine to +10 a lv3 weapon you forged = +1000 fame point
				}
			}
		}
	}
	return 0;
}

/*==========================================
 * 精錬失敗
 *------------------------------------------
 */
int buildin_failedrefitem(CScriptEngine &st)
{
	unsigned short itempos,num;
	map_session_data *sd=st.sd;

	if(sd)
	{
		num=st.GetInt( (st[2])) - 1;
		if(num<(sizeof(equip)/sizeof(equip[0])))
		{
			itempos=pc_checkequip(*sd,equip[num]);
			if(itempos < MAX_INVENTORY)
			{
				if(log_config.refine > 0)
					log_refine(*sd, itempos, 0);
				sd->status.inventory[itempos].refine = 0;
				pc_unequipitem(*sd,itempos,3);
				// 精錬失敗エフェクトのパケット
				clif_refine(sd->fd,*sd,1,itempos,sd->status.inventory[itempos].refine);
				pc_delitem(*sd,itempos,1,0);
				// 他の人にも失敗を通知
				clif_misceffect(*sd,2);
			}
		}
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_statusup(CScriptEngine &st)
{
	int type;
	map_session_data *sd;

	type=st.GetInt(st[2]);
	sd=st.sd;
	if(sd) pc_statusup(*sd,type);

	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_statusup2(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		int val=st.GetInt(st[3]);
		pc_statusup2(*st.sd,type,val);
	}
	return 0;
}
/*==========================================
 * 装備品による能力値ボーナス
 *------------------------------------------
 */
int buildin_bonus(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		int val=st.GetInt(st[3]);
		pc_bonus(*st.sd,type,val);
	}
	return 0;
}
/*==========================================
 * 装備品による能力値ボーナス
 *------------------------------------------
 */
int buildin_bonus2(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		int type2=st.GetInt(st[3]);
		int val=st.GetInt(st[4]);
		pc_bonus2(*st.sd,type,type2,val);
	}
	return 0;
}
/*==========================================
 * 装備品による能力値ボーナス
 *------------------------------------------
 */
int buildin_bonus3(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		int type2=st.GetInt(st[3]);
		int type3=st.GetInt(st[4]);
		int val=st.GetInt(st[5]);
		pc_bonus3(*st.sd,type,type2,type3,val);
	}
	return 0;
}

int buildin_bonus4(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		int type2=st.GetInt(st[3]);
		int type3=st.GetInt(st[4]);
		int type4=st.GetInt(st[5]);
		int val=st.GetInt(st[6]);
		pc_bonus4(*st.sd,type,type2,type3,type4,val);
	}
	return 0;
}
/*==========================================
 * スキル所得
 *------------------------------------------
 */
int buildin_skill(CScriptEngine &st)
{
	if(st.sd)
	{
		int id=st.GetInt(st[2]);
		int level=st.GetInt(st[3]);
		int flag = (st.Arguments() > 4) ? st.GetInt(st[4]) : 1;
		pc_skill(*st.sd,id,level,flag);
	}

	return 0;
}

// add x levels of skill (stackable) [Valaris]
int buildin_addtoskill(CScriptEngine &st)
{
	if(st.sd)
	{
		int id=st.GetInt(st[2]);
		int level=st.GetInt(st[3]);
		int flag = (st.Arguments() > 4) ? st.GetInt((st[4]) ) : 2;
		pc_skill(*st.sd,id,level,flag);
	}
	return 0;
}

/*==========================================
 * ギルドスキル取得
 *------------------------------------------
 */
int buildin_guildskill(CScriptEngine &st)
{
	if(st.sd)
	{
		int i;
		int id=st.GetInt(st[2]);
		int level=st.GetInt(st[3]);
		int flag =(st.Arguments() > 4) ? st.GetInt(st[4]) : 0;
		for(i=0; i<level; ++i)
			guild_skillup(*st.sd,id,flag);
	}
	return 0;
}
/*==========================================
 * スキルレベル所得
 *------------------------------------------
 */
int buildin_getskilllv(CScriptEngine &st)
{
	int id=st.GetInt(st[2]);
	st.push_val(CScriptEngine::C_INT, (st.sd)?pc_checkskill(*st.sd,id):0 );
	return 0;
}
/*==========================================
 * getgdskilllv(Guild_ID, Skill_ID);
 * skill_id = 10000 : GD_APPROVAL
 *            10001 : GD_KAFRACONTRACT
 *            10002 : GD_GUARDIANRESEARCH
 *            10003 : GD_GUARDUP
 *            10004 : GD_EXTENSION
 *------------------------------------------
 */
int buildin_getgdskilllv(CScriptEngine &st)
{
	uint32 guild_id=st.GetInt(st[2]);
	unsigned short skill_id=st.GetInt(st[3]);    
	struct guild *g=guild_search(guild_id);
	st.push_val( CScriptEngine::C_INT, (g==NULL)?-1:guild_checkskill(*g,skill_id) );
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_basicskillcheck(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, config.basic_skill_check);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_getgmlevel(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, ((st.sd) ? st.sd->isGM():0) );
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_end(CScriptEngine &st)
{
	st.Quit();
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_checkoption(CScriptEngine &st)
{
	int type=st.GetInt(st[2]);
	st.push_val(CScriptEngine::C_INT, (st.sd && (st.sd->status.option & type)) );
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_checkoption1(CScriptEngine &st)
{
	int type=st.GetInt(st[2]);
	st.push_val(CScriptEngine::C_INT, (st.sd && (st.sd->opt1 & type)) );
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_checkoption2(CScriptEngine &st)
{
	int type=st.GetInt(st[2]);
	st.push_val(CScriptEngine::C_INT, (st.sd && (st.sd->opt2 & type)) );
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_setoption(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		pc_setoption(*st.sd,type);
	}
	return 0;
}

/*==========================================
 * Checkcart [Valaris]
 *------------------------------------------
 */

int buildin_checkcart(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, (st.sd && st.sd->is_carton()) );
	return 0;
}

/*==========================================
 * カートを付ける
 *------------------------------------------
 */
int buildin_setcart(CScriptEngine &st)
{
	if(st.sd)
		pc_setcart(*st.sd,1);
	return 0;
}

/*==========================================
 * checkfalcon [Valaris]
 *------------------------------------------
 */

int buildin_checkfalcon(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, (st.sd && st.sd->is_falcon()) );
	return 0;
}


/*==========================================
 * 鷹を付ける
 *------------------------------------------
 */
int buildin_setfalcon(CScriptEngine &st)
{
	if(st.sd)
		pc_setfalcon(*st.sd);
	return 0;
}

/*==========================================
 * Checkcart [Valaris]
 *------------------------------------------
 */

int buildin_checkriding(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, (st.sd && st.sd->is_riding()) );
	return 0;
}


/*==========================================
 * ペコペコ乗り
 *------------------------------------------
 */
int buildin_setriding(CScriptEngine &st)
{
	if(st.sd)
		pc_setriding(*st.sd);
	return 0;
}

/*==========================================
 *	セーブポイントの保存
 *------------------------------------------
 */
int buildin_savepoint(CScriptEngine &st)
{
	if(st.sd)
	{
		const char *str=st.GetString((st[2]));
		int x=st.GetInt(st[3]);
		int y=st.GetInt(st[4]);
		pc_setsavepoint(*st.sd,str,x,y);
	}
	return 0;
}

/*==========================================
 * GetTimeTick(0: System Tick, 1: Time Second Tick)
 *------------------------------------------
 */
int buildin_gettimetick(CScriptEngine &st)	// Asgard Version 
{
	time_t timer;
	struct tm *t;
	int type=st.GetInt(st[2]);

	switch(type){
	case 2: 
		//type 2:(Get the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC from the system clock.)
		st.push_val(CScriptEngine::C_INT,(int)time(NULL));
		break;
	case 1:
		//type 1:(Second Ticks: 0-86399, 00:00:00-23:59:59)
		time(&timer);
		t=localtime(&timer);
		st.push_val(CScriptEngine::C_INT,((t->tm_hour)*3600+(t->tm_min)*60+t->tm_sec));
		break;
	default:
		//type 0:(System Ticks)
		st.push_val(CScriptEngine::C_INT,(int)gettick());
		break;
	}
	return 0;
}

/*==========================================
 * GetTime(Type);
 * 1: Sec     2: Min     3: Hour
 * 4: WeekDay     5: MonthDay     6: Month
 * 7: Year
 *------------------------------------------
 */
int buildin_gettime(CScriptEngine &st)	// Asgard Version 
{
	time_t timer;
	struct tm *t;
	int type=st.GetInt(st[2]);

	time(&timer);
	t=localtime(&timer);

	switch(type){
	case 1://Sec(0~59)
		st.push_val(CScriptEngine::C_INT,t->tm_sec);
		break;
	case 2://Min(0~59)
		st.push_val(CScriptEngine::C_INT,t->tm_min);
		break;
	case 3://Hour(0~23)
		st.push_val(CScriptEngine::C_INT,t->tm_hour);
		break;
	case 4://WeekDay(0~6)
		st.push_val(CScriptEngine::C_INT,t->tm_wday);
		break;
	case 5://MonthDay(01~31)
		st.push_val(CScriptEngine::C_INT,t->tm_mday);
		break;
	case 6://Month(01~12)
		st.push_val(CScriptEngine::C_INT,t->tm_mon+1);
		break;
	case 7://Year(20xx)
		st.push_val(CScriptEngine::C_INT,t->tm_year+1900);
		break;
	default://(format error)
		st.push_val(CScriptEngine::C_INT,-1);
		break;
	}
	return 0;
}

/*==========================================
 * GetTimeStr("TimeFMT", Length);
 *------------------------------------------
 */
int buildin_gettimestr(CScriptEngine &st)
{
	char *tmpstr;
	const char *fmtstr;
	int maxlen;
	time_t now = time(NULL);

	fmtstr=st.GetString((st[2]));
	maxlen=st.GetInt(st[3]);

	tmpstr=new char[maxlen+1];
	strftime(tmpstr,maxlen,fmtstr,localtime(&now));
	tmpstr[maxlen]='\0';

	st.push_str(CScriptEngine::C_STR, tmpstr);
	return 0;
}

/*==========================================
 * カプラ倉庫を開く
 *------------------------------------------
 */
int buildin_openstorage(CScriptEngine &st)
{
	int ret=0;
	if(st.sd)
		ret = storage_storageopen(*st.sd);
	st.push_val(CScriptEngine::C_INT,ret);
	return 0;
}

int buildin_guildopenstorage(CScriptEngine &st)
{
	int ret=0;
	if(st.sd)
		ret = storage_guild_storageopen(*st.sd);
	st.push_val(CScriptEngine::C_INT,ret);
	return 0;
}

/*==========================================
 * アイテムによるスキル発動
 *------------------------------------------
 */
int buildin_itemskill(CScriptEngine &st)
{
	if(st.sd && st.sd->skilltimer == -1)
	{
		int id=st.GetInt(st[2]);
		int lv=st.GetInt(st[3]);
		const char*str=st.GetString((st[4]));

		// 詠唱中にスキルアイテムは使用できない
		st.sd->skillitem=id;
		st.sd->skillitemlv=lv;
		clif_item_skill(*st.sd,id,lv,str);
	}
	return 0;
}
/*==========================================
 * アイテム作成
 *------------------------------------------
 */
int buildin_produce(CScriptEngine &st)
{
	if(st.sd && st.sd->state.produce_flag != 1)
	{
		int trigger=st.GetInt(st[2]);
		clif_skill_produce_mix_list(*st.sd,trigger);
	}
	return 0;
}
/*==========================================
 * NPCでペット作る
 *------------------------------------------
 */
int buildin_makepet(CScriptEngine &st)
{
	if(st.sd)
	{
		int id = st.GetInt( st[2] );
		int pet_id = search_petDB_index(id, PET_CLASS);
		if (pet_id < 0)
			pet_id = search_petDB_index(id, PET_EGG);
		
		if(pet_id >= 0)
		{
			st.sd->catch_target_class = pet_db[pet_id].class_;
			intif_create_pet(
				st.sd->status.account_id, st.sd->status.char_id,		//long
				pet_db[pet_id].class_, mob_db[pet_db[pet_id].class_].lv,//short
				pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate, 100,	//short
				0, 1,													//char
				pet_db[pet_id].jname);									//char*
		}
	}
	return 0;
}
/*==========================================
 * NPCで経験値上げる
 *------------------------------------------
 */
int buildin_getexp(CScriptEngine &st)
{
	if(st.sd)
	{
		int base=st.GetInt(st[2]);
		int job =st.GetInt(st[3]);
		if(base>0 && job>0)
			pc_gainexp(*st.sd,base,job);
	}
	return 0;
}

/*==========================================
 * Gain guild exp [Celest]
 *------------------------------------------
 */
int buildin_guildgetexp(CScriptEngine &st)
{
	if(st.sd && st.sd->status.guild_id)
	{
		int exp = st.GetInt(st[2]);
		if(exp > 0)
			guild_getexp (*st.sd, exp);
	}
	return 0;
}

/*==========================================
 * モンスター発生
 *------------------------------------------
 */
int buildin_monster(CScriptEngine &st)
{
	int class_,amount,x,y;
	const char *str,*map,*event="";

	map	=st.GetString((st[2]));
	x	=st.GetInt(st[3]);
	y	=st.GetInt(st[4]);
	str	=st.GetString((st[5]));
	class_=st.GetInt(st[6]);
	amount=st.GetInt(st[7]);
	if( st.Arguments() > 8 )
		event=st.GetString((st[8]));
//!! broadcast command if not on this mapserver
	mob_once_spawn(st.sd,map,x,y,str,class_,amount,event);
	return 0;
}
/*==========================================
 * モンスター発生
 *------------------------------------------
 */
int buildin_areamonster(CScriptEngine &st)
{
	int class_,amount,x0,y0,x1,y1;
	const char *str,*map,*event="";

	map	=st.GetString(st[2]);
	x0	=st.GetInt(st[3]);
	y0	=st.GetInt(st[4]);
	x1	=st.GetInt(st[5]);
	y1	=st.GetInt(st[6]);
	str	=st.GetString(st[7]);
	class_=st.GetInt(st[8]);
	amount=st.GetInt(st[9]);
	if( st.Arguments() > 10 )
		event=st.GetString(st[10]);
//!! broadcast command if not on this mapserver
	mob_once_spawn_area(st.sd,map,x0,y0,x1,y1,str,class_,amount,event);
	return 0;
}
/*==========================================
 * モンスター削除
 *------------------------------------------
 */
class CBuildinKillSummonedmob : public CMapProcessor
{
public:
	CBuildinKillSummonedmob()	{}
	~CBuildinKillSummonedmob()	{}
	virtual int process(block_list& bl) const
	{
		mob_data *md = bl.get_md();
		if( md && !md->cache)
		{	// delete all script-summoned mobs
			md->freeblock();
			return 1;
		}
		return 0;
	}
};
class CBuildinKillEventmob : public CMapProcessor
{
	const char *event;
public:
	CBuildinKillEventmob(const char* e) : event(e)	{}
	~CBuildinKillEventmob()	{}
	virtual int process(block_list& bl) const
	{
		mob_data *md = bl.get_md();
		if( md && 0==strcmp(event, md->npc_event))
		{	// delete only mobs with same event name
			md->remove_map(0);
			return 1;
		}
		return 0;
	}
};
class CBuildinKillallmob : public CMapProcessor
{
public:
	CBuildinKillallmob()	{}
	~CBuildinKillallmob()	{}
	virtual int process(block_list& bl) const
	{
		mob_data *md = bl.get_md();
		if( md )
		{	
			md->remove_map(1);
			return 1;
		}
		return 0;
	}
};
int buildin_killmonster(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	const char *event=st.GetString(st[3]);
	unsigned short m=map_mapname2mapid(mapname);

	if( m < map_num )
	{	//!! broadcast command if not on this mapserver
		if(strcmp(event,"All")==0)
			block_list::foreachinarea( CBuildinKillSummonedmob(),
				m,0,0,maps[m].xs-1,maps[m].ys-1,BL_MOB);
		else
			block_list::foreachinarea( CBuildinKillEventmob(event),
				m,0,0,maps[m].xs-1,maps[m].ys-1,BL_MOB);
	}
	return 0;
}

int buildin_killmonsterall(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	ushort m =map_mapname2mapid(mapname);
	if( m < map_num )
	{	//!! broadcast command if not on this mapserver
		block_list::foreachinarea( CBuildinKillallmob(),
			m,0,0,maps[m].xs-1,maps[m].ys-1,BL_MOB);
	}
	return 0;
}

/*==========================================
 * イベント実行
 *------------------------------------------
 */
int buildin_doevent(CScriptEngine &st)
{
//!! broadcast command if not on this mapserver
	if(st.sd)
	{
		const char *event=st.GetString(st[2]);
		npc_data::event(event, *st.sd);
	}
	return 0;
}
/*==========================================
 * NPC主体イベント実行
 *------------------------------------------
 */
int buildin_donpcevent(CScriptEngine &st)
{
	const char *event;
	event=st.GetString(st[2]);
	npc_data::event(event);
	return 0;
}
/*==========================================
 * イベントタイマー追加
 *------------------------------------------
 */
int buildin_addtimer(CScriptEngine &st)
{
	if(st.sd)
	{
		unsigned long tick=st.GetInt(st[2]);
		const char* event=st.GetString(st[3]);
		pc_addeventtimer(*st.sd,tick,event);
	}
	return 0;
}
/*==========================================
 * イベントタイマー削除
 *------------------------------------------
 */
int buildin_deltimer(CScriptEngine &st)
{
	if(st.sd)
	{
		const char *event=st.GetString(st[2]);
		pc_deleventtimer(*st.sd,event);
	}
	return 0;
}
/*==========================================
 * イベントタイマーのカウント値追加
 *------------------------------------------
 */
int buildin_addtimercount(CScriptEngine &st)
{
	if(st.sd)
	{
		const char*event=st.GetString(st[2]);
		unsigned long tick=st.GetInt(st[3]);
		pc_addeventtimercount(*st.sd,event,tick);
	}
	return 0;
}

/*==========================================
 * NPCタイマー初期化
 *------------------------------------------
 */
int buildin_initnpctimer(CScriptEngine &st)
{
	npcscript_data *nd;
	ushort pos=0;
	if( st.Arguments() > 2 )
	{	// try name as second operand
		nd= npcscript_data::from_name(st.GetString(st[2]));
		// if failed and have exactly two operands take it as starting position
		if( !nd && st.Arguments() == 2 )
			pos = st.GetInt(st[2]);
	}
	else
		nd= st.nd;
	// take third operand as position if exists
	if( st.Arguments() > 3 )
		pos = st.GetInt(st[3]);

	if(nd)
	{
		nd->eventtimer_init(st.sd?st.sd->block_list::id:0, pos);
	}
	return 0;
}
/*==========================================
 * NPCタイマー開始
 *------------------------------------------
 */
int buildin_startnpctimer(CScriptEngine &st)
{
	npcscript_data *nd;
	if( st.Arguments() > 2 )
		nd=npcscript_data::from_name(st.GetString(st[2]));
	else
		nd=st.nd;

	if(nd) nd->eventtimer_start(st.sd?st.sd->block_list::id:0);
	return 0;
}
/*==========================================
 * NPCタイマー停止
 *------------------------------------------
 */
int buildin_stopnpctimer(CScriptEngine &st)
{
	npcscript_data *nd;
	if( st.Arguments() > 2 )
		nd=npcscript_data::from_name(st.GetString( (st[2])));
	else
		nd=st.nd;

	if(nd) nd->eventtimer_stop(st.sd?st.sd->block_list::id:0);
	return 0;
}
/*==========================================
 * NPCタイマー情報所得
 *------------------------------------------
 */
int buildin_getnpctimer(CScriptEngine &st)
{
	npcscript_data *nd;
	int type=st.GetInt(st[2]);
	int val=0;
	if( st.Arguments() > 3 )
		nd=npcscript_data::from_name(st.GetString( (st[3])));
	else
		nd=st.nd;
	if(nd)
	{
		switch(type)
		{
		case 0: val= nd->eventtimer_getpos(st.sd?st.sd->block_list::id:0); break;
		case 1: val=  nd->ontimer_cnt; break;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}
/*==========================================
 * NPCタイマー値設定
 *------------------------------------------
 */
int buildin_setnpctimer(CScriptEngine &st)
{
	ushort pos=st.GetInt(st[2]);
	npcscript_data *nd;
	if( st.Arguments() > 3 )
		nd=npcscript_data::from_name(st.GetString( (st[3])));
	else
		nd=st.nd;

	// from usage in current scripts, setnpctimer works same as initnpctimer
	// exept that it always used the current npc
	if(nd) nd->eventtimer_init(st.sd?st.sd->block_list::id:0, pos);
	return 0;
}

/*==========================================
 * attaches the player rid to the timer [Celest]
 *------------------------------------------
 */
int buildin_attachnpctimer(CScriptEngine &st)
{
	map_session_data *sd;
	if( st.nd && st.Arguments() > 2 && (sd=map_session_data::nick2sd(st.GetString(st[2]))) && sd!=st.sd )
	{	// attachin only makes sense when attaching it to a different pc
		st.nd->eventtimer_attach(st.sd?st.sd->block_list::id:0, sd->block_list::id);
	}
	return 0;
}

/*==========================================
 * detaches a player rid from the timer [Celest]
 *------------------------------------------
 */
int buildin_detachnpctimer(CScriptEngine &st)
{
/* useless
	npcscript_data *nd;
	if( st.Arguments() > 2 )
		nd=npcscript_data::from_name(st.GetString( (st[2])));
	else
		nd=st.nd;
	if(nd)
		st.nd->eventtimer_attach(0);
*/
	return 0;
}

/*==========================================
 * 天の声アナウンス
 *------------------------------------------
 */
int buildin_announce(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	int flag=st.GetInt(st[3]);
	size_t len=1+strlen(str);

	if(flag&0x0f)
	{
		block_list *bl=(flag&0x08) ? (block_list *)st.nd : (block_list *)st.sd;
		clif_GMmessage(bl, str, len, flag);
	}
	else
		intif_GMmessage(str,len,flag);
	return 0;
}
/*==========================================
 * 天の声アナウンス（特定マップ）
 *------------------------------------------
 */
/*
int buildin_mapannounce_sub(block_list &bl,va_list &ap)
{
	char *str;
	int len,flag;
	str=va_arg(ap,char *);
	len=va_arg(ap,int);
	flag=va_arg(ap,int);
	clif_GMmessage(&bl,str,len,flag|3);
	return 0;
}
*/
class CBuildinMapannounce : public CMapProcessor
{
	const char*&str;
	size_t len;
	int flag;
public:
	CBuildinMapannounce(const char*&s, size_t l, int f)
		: str(s),len(l),flag(f|3)	{}
	~CBuildinMapannounce()	{}
	virtual int process(block_list& bl) const
	{
		if( bl==BL_PC )
			clif_GMmessage(&bl,str,len,flag);
		return 0;
	}
};
int buildin_mapannounce(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	const char *str=st.GetString(st[3]);
	int flag=st.GetInt(st[4]);
	ushort m=map_mapname2mapid(mapname);
	if( m<map_num )
	{	//!! broadcast command if not on this mapserver
		block_list::foreachinarea( CBuildinMapannounce(str,1+strlen(str),flag),
			m,0,0,maps[m].xs-1,maps[m].ys-1,BL_PC);
//		map_foreachinarea(buildin_mapannounce_sub,
//			m,0,0,maps[m].xs-1,maps[m].ys-1,BL_PC,
//			str,strlen(str)+1,flag&0x10);
	}
	return 0;
}
/*==========================================
 * 天の声アナウンス（特定エリア）
 *------------------------------------------
 */
int buildin_areaannounce(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	int x0=st.GetInt(st[3]);
	int y0=st.GetInt(st[4]);
	int x1=st.GetInt(st[5]);
	int y1=st.GetInt(st[6]);
	const char *str=st.GetString(st[7]);
	int flag=st.GetInt(st[8]);
	ushort m=map_mapname2mapid(mapname);
	if( m<map_num )
	{	//!! broadcast command if not on this mapserver
		block_list::foreachinarea( CBuildinMapannounce(str,1+strlen(str),flag&0x10),
			m,x0,y0,x1,y1,BL_PC);
//		map_foreachinarea(buildin_mapannounce_sub,
//			m,x0,y0,x1,y1,BL_PC, 
//			str,strlen(str)+1,flag&0x10 );
	}
	return 0;
}
/*==========================================
 * ユーザー数所得
 *------------------------------------------
 */
int buildin_getusers(CScriptEngine &st)
{
	int flag=st.GetInt(st[2]);
	block_list *bl = (flag&0x08) ? (block_list *)st.nd : (block_list *)st.sd;
	int val=0;
	switch(flag&0x07)
	{
	case 0: val=maps[bl->m].users; break;
	case 1: val=map_getusers(); break;
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}
/*==========================================
 * Works like @WHO - displays all online users names in window
 *------------------------------------------
 */
// should not work since script is not paused to wait for the next-click
int buildin_getusersname(CScriptEngine &st)
{
	if(st.sd)
	{
		map_session_data *pl_sd = NULL;
		size_t i=0,disp_num=1;
		for (i=0;i<fd_max;++i)
		{
			if(session[i] && (pl_sd=(map_session_data *) session[i]->user_session) && pl_sd->state.auth)
			{
				if( !(config.hide_GM_session && pl_sd->isGM()) )
				{
					if((disp_num++)%10==0)
						clif_scriptnext(*st.sd, st.send_defaultnpc());
					clif_scriptmes(*st.sd,st.send_defaultnpc(),pl_sd->status.name);
				}
			}
		}
	}
	return 0;
}
/*==========================================
 * マップ指定ユーザー数所得
 *------------------------------------------
 */
int buildin_getmapusers(CScriptEngine &st)
{
	
	const char *mapname=st.GetString(st[2]);
	ushort m=map_mapname2mapid(mapname);
	//!! broadcast command if not on this mapserver
	int val = (m<map_num) ? (int)maps[m].users : -1;
	st.push_val(CScriptEngine::C_INT, val);
	return 0;
}
/*==========================================
 * エリア指定ユーザー数所得
 *------------------------------------------
 */
class CBuildinCountObject : public CMapProcessor
{
	object_t type;// double check actually not necessary, just paranoia
public:
	CBuildinCountObject(object_t t) : type(t)	{}
	~CBuildinCountObject()	{}
	virtual int process(block_list& bl) const
	{
		return bl.is_type(type);
	}
};
int buildin_getareausers(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	int x0=st.GetInt(st[3]);
	int y0=st.GetInt(st[4]);
	int x1=st.GetInt(st[5]);
	int y1=st.GetInt(st[6]);
	ushort m=map_mapname2mapid(mapname);
	//!! broadcast command if not on this mapserver
	int users = ( m>=map_num )? -1 : 
		block_list::foreachinarea( CBuildinCountObject(BL_PC), m,x0,y0,x1,y1,BL_PC);

	st.push_val(CScriptEngine::C_INT,users);
	return 0;
}

/*==========================================
 * エリア指定ドロップアイテム数所得
 *------------------------------------------
 */
class CBuildinCountDropitem : public CMapProcessor
{
	int item;
public:
	CBuildinCountDropitem(int i):item(i)	{}
	~CBuildinCountDropitem()	{}
	virtual int process(block_list& bl) const
	{
		flooritem_data &drop=(flooritem_data &)bl;
		return (drop.item_data.nameid==item) ? drop.item_data.amount : 0;
	}
};
int buildin_getareadropitem(CScriptEngine &st)
{
	const char *mapname = st.GetString(st[2]);
	int x0  = st.GetInt(st[3]);
	int y0  = st.GetInt(st[4]);
	int x1  = st.GetInt(st[5]);
	int y1  = st.GetInt(st[6]);
	int item;
	int amount=0;
	ushort m=map_mapname2mapid(mapname);

	if( m<map_num )
	{	//!! broadcast command if not on this mapserver
		CScriptEngine::CValue &data= st[7];
		st.ConvertName(data);
		if( data.isString() )
		{
			const char *name=st.GetString(data);
			struct item_data *item_data = itemdb_searchname(name);
			item=512;
			if( item_data )
				item=item_data->nameid;
		}
		else
			item=st.GetInt(data);

		amount = block_list::foreachinarea( CBuildinCountDropitem(item),
			m,x0,y0,x1,y1,BL_ITEM);

//		map_foreachinarea(buildin_getareadropitem_sub,
//			m,x0,y0,x1,y1,BL_ITEM,item,&amount);
	}
	st.push_val(CScriptEngine::C_INT,amount);
	return 0;
}
/*==========================================
 * NPCの有効化
 *------------------------------------------
 */
int buildin_enablenpc(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	st.push_val(CScriptEngine::C_INT, npc_data::enable(str,1) );
	return 0;
}
/*==========================================
 * NPCの無効化
 *------------------------------------------
 */
int buildin_disablenpc(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	st.push_val(CScriptEngine::C_INT, npc_data::enable(str,0) );
	return 0;
}

int buildin_enablearena(CScriptEngine &st)
{
	if( st.nd && st.nd->chat )
	{
		npcchat_data &cd = *st.nd->chat;
		st.nd->enable(1);
		st.nd->arenaflag=1;

		if(cd.users>=cd.trigger && cd.npc_event[0])
			npc_data::event(cd.npc_event);
	}
	return 0;
}
int buildin_disablearena(CScriptEngine &st)
{
	if(st.nd)
		st.nd->arenaflag=0;

	return 0;
}
/*==========================================
 * 隠れているNPCの表示
 *------------------------------------------
 */
int buildin_hideoffnpc(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	st.push_val(CScriptEngine::C_INT, npc_data::enable(str,2) );
	return 0;
}
/*==========================================
 * NPCをハイディング
 *------------------------------------------
 */
int buildin_hideonnpc(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	st.push_val(CScriptEngine::C_INT, npc_data::enable(str,4) );
	return 0;
}
/*==========================================
 * 状態異常にかかる
 *------------------------------------------
 */
int buildin_sc_start(CScriptEngine &st)
{
	block_list *bl;
	int type;
	unsigned long tick;
	int val1;
	type=st.GetInt(st[2]);
	tick=st.GetInt(st[3]);
	val1=st.GetInt(st[4]);
	if( st.Arguments() > 5 ) //指定したキャラを状態異常にする
		bl = block_list::from_blid(st.GetInt(st[5]));
	else
		bl = st.sd;

	if(bl)
	{
		map_session_data* sd = bl->get_sd();
		if( sd && sd->state.potion_flag==1 )
		{
			bl = block_list::from_blid(sd->skilltarget);
			tick/=2; //Thrown potions only last half.
		}
		if(bl) status_change_start(bl,type,val1,0,0,0,tick,0);
	}
	return 0;
}

/*==========================================
 * 状態異常にかかる(確率指定)
 *------------------------------------------
 */
int buildin_sc_start2(CScriptEngine &st)
{
	block_list *bl;
	int type;
	unsigned long tick;
	int val1,per;
	type=st.GetInt(st[2]);
	tick=st.GetInt(st[3]);
	val1=st.GetInt(st[4]);
	per=st.GetInt(st[5]);
	if( st.Arguments() > 6 ) //指定したキャラを状態異常にする
		bl = block_list::from_blid(st.GetInt(st[6]));
	else
		bl = st.sd;
	if(bl)
	{
		map_session_data *sd = bl->get_sd();
		if( sd && sd->state.potion_flag==1 )
			bl = block_list::from_blid(sd->skilltarget);
		if(rand()%10000 < per)
			status_change_start(bl,type,val1,0,0,0,tick,0);
	}
	return 0;
}
/*==========================================
 * Starts a SC_ change with the four values passed. [Skotlex]
 * Final optional argument is the ID of player to affect.
 * sc_start4 type, duration, val1, val2, val3, val4, <id>;
 *------------------------------------------
 */
int buildin_sc_start4(CScriptEngine &st)
{
	int type=st.GetInt(st[2]);
	int tick=st.GetInt(st[3]);
	int val1=st.GetInt(st[4]);
	int val2=st.GetInt(st[5]);
	int val3=st.GetInt(st[6]);
	int val4=st.GetInt(st[7]);
	block_list *bl;
	if( st.Arguments() > 8 )
		bl = block_list::from_blid( st.GetInt(st[8]) );
	else
		bl = st.sd;

	if (bl)
	{
		map_session_data*sd = bl->get_sd();
		if(sd && sd->state.potion_flag==1 )
			bl = block_list::from_blid(sd->skilltarget);
		status_change_start(bl,type,val1,val2,val3,val4,tick,0);
	}
	return 0;
}

/*==========================================
 * 状態異常が直る
 *------------------------------------------
 */
int buildin_sc_end(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		block_list *bl = st.sd;
		
		if( st.sd->state.potion_flag==1 )
			bl = block_list::from_blid(st.sd->skilltarget);
		status_change_end(bl,type,-1);
//		if(config.etc_log)
//			ShowMessage("sc_end : %d %d\n",st.rid,type);
	}
	return 0;
}
/*==========================================
 * 状態異常耐性を計算した確率を返す
 *------------------------------------------
 */
int buildin_getscrate(CScriptEngine &st)
{
	block_list *bl;
	int sc_def,type,rate;

	type=st.GetInt(st[2]);
	rate=st.GetInt(st[3]);
	if( st.Arguments() > 4 ) //指定したキャラの耐性を計算する
		bl = block_list::from_blid(st.GetInt(st[6]));
	else
		bl = st.sd;

	sc_def = status_get_sc_def(bl,type);

	rate = rate * sc_def / 100;
	st.push_val(CScriptEngine::C_INT,rate);

	return 0;

}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_debugmes(CScriptEngine &st)
{
	ShowMessage("script debug (rid=%d oid=%d) : %s\n", 
		st.sd?st.sd->block_list::id:0, st.nd?st.nd->block_list::id:0, st.GetString(st[2]));
	return 0;
}

/*==========================================
 *捕獲アイテム使用
 *------------------------------------------
 */
int buildin_catchpet(CScriptEngine &st)
{
	if(st.sd)
	{
		int pet_id= st.GetInt(st[2]);
		pet_catch_process1(*st.sd,pet_id);
	}
	return 0;
}

/*==========================================
 *携帯卵孵化機使用
 *------------------------------------------
 */
int buildin_birthpet(CScriptEngine &st)
{
	if(st.sd)
		clif_sendegg(*st.sd);
	return 0;
}

/*==========================================
 * reset player level
 *------------------------------------------
 */
int buildin_resetlvl(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		pc_resetlvl(*st.sd,type);
	}
	return 0;
}
/*==========================================
 * ステータスリセット
 *------------------------------------------
 */
int buildin_resetstatus(CScriptEngine &st)
{
	if(st.sd)
		pc_resetstate(*st.sd);
	return 0;
}

/*==========================================
 * スキルリセット
 *------------------------------------------
 */
int buildin_resetskill(CScriptEngine &st)
{
	if(st.sd)
		pc_resetskill(*st.sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_changebase(CScriptEngine &st)
{
	map_session_data *sd=st.sd;
	int vclass;

	if( st.Arguments() > 3 )
		sd=map_session_data::from_blid(st.GetInt( (st[3])));

	if(sd == NULL)
		return 0;

	vclass = st.GetInt(st[2]);
	if( vclass == 22 && 
		(!config.wedding_modifydisplay ||					// Do not show the wedding sprites
		(sd->status.class_ >= 4023 && sd->status.class_ <= 4045)) )	// Baby classes screw up when showing wedding sprites. [Skotlex]
		return 0;

//	if(vclass==22) {
//		pc_unequipitem(sd,sd->equip_index[9],0);	// 装備外
//	}

	sd->view_class = vclass;

	return 0;
}

/*==========================================
 * 性別変換
 *------------------------------------------
 */
int buildin_changesex(CScriptEngine &st)
{
	if(st.sd)
	{
		if(st.sd->status.sex == 0)
		{
			st.sd->status.sex = 1;
			if(st.sd->status.class_ == 20 || st.sd->status.class_ == 4021)
				st.sd->status.class_ -= 1;
		}
		else if(st.sd->status.sex == 1)
		{
			st.sd->status.sex = 0;
			if(st.sd->status.class_ == 19 || st.sd->status.class_ == 4020)
				st.sd->status.class_ += 1;
		}
		chrif_char_ask_name(-1, st.sd->status.name, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
		chrif_save(*st.sd);
	}
	return 0;
}

/*==========================================
 * npcチャット作成
 *------------------------------------------
 */
int buildin_waitingroom(CScriptEngine &st)
{
	if(st.nd)
	{	
		const char *name,*ev="";
		int limit, trigger = 0,pub=1;
		name=st.GetString(st[2]);
		limit= st.GetInt(st[3]);
		if(limit==0)
			pub=3;

		if( st.Arguments() > 5)
		{
			CScriptEngine::CValue* data=&(st[5]);
			st.ConvertName(*data);
			if(data->type==CScriptEngine::C_INT){
				// 新Athena仕様(旧Athena仕様と互換性あり)
				ev=st.GetString(st[4]);
				trigger=st.GetInt(st[5]);
			}else{
				// eathena仕様
				trigger=st.GetInt(st[4]);
				ev=st.GetString(st[5]);
			}
		}
		else
		{	// 旧Athena仕様
			if( st.Arguments() > 4 )
				ev=st.GetString(st[4]);
		}		
		
		npcchat_data::erase(*st.nd);
		npcchat_data::create(*st.nd,limit,pub,trigger,name,ev);
	}
	return 0;
}
/*==========================================
 * Works like 'announce' but outputs in the common chat window
 *------------------------------------------
 */
int buildin_globalmes(CScriptEngine &st)
{
	const char *mes;
	mes=st.GetString(st[2]);	// メッセージの取得
	if(mes)
	{
		npc_data *nd;
		if( st.Arguments() > 3 )
		{	// NPC名の取得(123#456)
			nd = npc_data::from_name( st.GetString(st[3]) );
		}
		else
		{
			nd = st.nd;
		}
		if( nd )
		{
			char temp[128];
			char ntemp[64];
			char *ltemp;

			safestrcpy(ntemp,sizeof(ntemp),nd->name);	// copy the name
			ltemp=strchr(ntemp,'#');					// check for a # numerator
			if(ltemp) *ltemp=0;							// and remove it
			
			size_t sz = snprintf(temp, sizeof(temp),"%s: %s",ntemp, mes);
			clif_GlobalMessage(*nd, temp, sz);
		}
	}
	return 0;
}
/*==========================================
 * npcチャット削除
 *------------------------------------------
 */
int buildin_delwaitingroom(CScriptEngine &st)
{
	npcscript_data *nd;
	if( st.Arguments() > 2 )
		nd = npcscript_data::from_name(st.GetString( (st[2])));
	else
		nd = st.nd;
	if( nd )
		npcchat_data::erase(*nd);
	return 0;
}
/*==========================================
 * npcチャット全員蹴り出す
 *------------------------------------------
 */
int buildin_waitingroomkickall(CScriptEngine &st)
{
	npcscript_data *nd;

	if( st.Arguments() > 2 )
		nd = npcscript_data::from_name(st.GetString( (st[2])));
	else
		nd = st.nd;

	if( nd && nd->chat )
		nd->chat->kickall();
	return 0;
}

/*==========================================
 * npcチャットイベント有効化
 *------------------------------------------
 */
int buildin_enablewaitingroomevent(CScriptEngine &st)
{
	npcscript_data *nd;

	if( st.Arguments() > 2 )
		nd = npcscript_data::from_name(st.GetString( (st[2])));
	else
		nd = st.nd;

	if(nd && nd->chat )
		nd->chat->enable_event();
	return 0;
}

/*==========================================
 * npcチャットイベント無効化
 *------------------------------------------
 */
int buildin_disablewaitingroomevent(CScriptEngine &st)
{
	npcscript_data *nd;

	if( st.Arguments() > 2 )
		nd = npcscript_data::from_name(st.GetString( (st[2])));
	else
		nd = st.nd;

	if(nd && nd->chat )
		nd->chat->disable_event();
	return 0;
}
/*==========================================
 * npcチャット状態所得
 *------------------------------------------
 */
int buildin_getwaitingroomstate(CScriptEngine &st)
{
	const npcscript_data *nd;
	int val=-1, type;
	type=st.GetInt(st[2]);
	if( st.Arguments() > 3 )
		nd=npcscript_data::from_name(st.GetString( (st[3])));
	else
		nd=st.nd;

	if( nd && nd->chat )
	{
		switch(type)
		{
		case 0: val=nd->chat->users; break;
		case 1: val=nd->chat->limit; break;
		case 2: val=nd->chat->trigger&0x7f; break;
		case 3: val=((nd->chat->trigger&0x80)>0); break;
		case 32: val=(nd->chat->users >= nd->chat->limit); break;
		case 33: val=(nd->chat->users >= nd->chat->trigger); break;
		case 4:
			st.push_str(CScriptEngine::C_CONSTSTR, nd->chat->title);
			return 0;
		case 5:
			st.push_str(CScriptEngine::C_CONSTSTR, nd->chat->pass);
			return 0;
		case 16:
			st.push_str(CScriptEngine::C_CONSTSTR, nd->chat->npc_event);
			return 0;
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*==========================================
 * チャットメンバー(規定人数)ワープ
 *------------------------------------------
 */
int buildin_warpwaitingpc(CScriptEngine &st)
{
	if( st.nd && st.nd->chat )
	{
		int i;
		const npcchat_data *cd = st.nd->chat;
		const char *str	= st.GetString(st[2]);
		int x			= st.GetInt(st[3]);
		int y			= st.GetInt(st[4]);
		int n			= ( st.Arguments() > 5 ) ? st.GetInt(st[5]) : cd->trigger&0x7f;
		map_session_data *sd;

		for(i=0, sd=cd->usersd[0]; sd && i<n; ++i, sd=cd->usersd[0])
		{
			mapreg_setregnum(add_str("$@warpwaitingpc")+(i<<24),sd->block_list::id);
			
			if(strcmp(str,"Random")==0)
			{
				pc_randomwarp(*sd,3);
			}
			else if(strcmp(str,"SavePoint")==0)
			{
				if(maps[sd->block_list::m].flag.noteleport)	// テレポ禁止
					return 0;
				pc_setpos(*sd,sd->status.save_point.mapname,sd->status.save_point.x,sd->status.save_point.y,3);
			}
			else
			{
				pc_setpos(*sd,str,x,y,0);
			}
		}
		mapreg_setregnum(add_str( "$@warpwaitingpcnum"), n);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
enum 
{
	MF_NOMEMO,			// 0
	MF_NOTELEPORT,		// 1
	MF_NOSAVE,			// 2
	MF_NOBRANCH,		// 3
	MF_NOPENALTY,		// 4
	MF_NOZENYPENALTY,	// 5
	MF_PVP,				// 6
	MF_PVP_NOPARTY,		// 7
	MF_PVP_NOGUILD,		// 8
	MF_GVG,				// 9
	MF_GVG_NOPARTY,		//10
	MF_NOTRADE,			//11
	MF_NOSKILL,			//12
	MF_NOWARP,			//13
	MF_NOPVP,			//14
	MF_NOICEWALL,		//15
	MF_SNOW,			//16
	MF_FOG,				//17
	MF_SAKURA,			//18
	MF_LEAVES,			//19
	MF_RAIN,			//20
	MF_INDOORS,			//21
	MF_NOGO,			//22
	MF_CLOUDS,			//23
	MF_CLOUDS2,			//24
	MF_FIREWORKS, 		//25
	MF_GVG_DUNGEON		//26
};



int buildin_setmapflagnosave(CScriptEngine &st)
{
	int m,x,y;
	const char *str,*str2;

	str=st.GetString(st[2]);
	str2=st.GetString(st[3]);
	x=st.GetInt(st[4]);
	y=st.GetInt(st[5]);
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0) {
		maps[m].flag.nosave=1;
		safestrcpy(maps[m].nosave.mapname, sizeof(maps[m].nosave.mapname), str2);
		char*ip=strchr(maps[m].nosave.mapname,'.');
		if(ip) *ip=0;
		maps[m].nosave.x=x;
		maps[m].nosave.y=y;
	}

	return 0;
}

int buildin_setmapflag(CScriptEngine &st)
{
	unsigned int m,i;
	const char *str;

	str=st.GetString(st[2]);
	i=st.GetInt(st[3]);
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m < map_num)
	{
		switch(i)
		{
			case MF_NOMEMO:
				maps[m].flag.nomemo=1;
				break;
			case MF_NOTELEPORT:
				maps[m].flag.noteleport=1;
				break;
			case MF_NOBRANCH:
				maps[m].flag.nobranch=1;
				break;
			case MF_NOPENALTY:
				maps[m].flag.nopenalty=1;
				break;
			case MF_NOZENYPENALTY:
				maps[m].flag.nozenypenalty=1;
				break;
			case MF_PVP:
				maps[m].flag.pvp=1;
				break;
			case MF_PVP_NOPARTY:
				maps[m].flag.pvp_noparty=1;
				break;
			case MF_PVP_NOGUILD:
				maps[m].flag.pvp_noguild=1;
				break;
			case MF_GVG:
				maps[m].flag.gvg=1;
				break;
			case MF_GVG_NOPARTY:
				maps[m].flag.gvg_noparty=1;
				break;
			case MF_GVG_DUNGEON:
				maps[m].flag.gvg_dungeon=1;
				break;
			case MF_NOTRADE:
				maps[m].flag.notrade=1;
				break;
			case MF_NOSKILL:
				maps[m].flag.noskill=1;
				break;
			case MF_NOWARP:
				maps[m].flag.nowarp=1;
				break;
			case MF_NOPVP:
				maps[m].flag.nopvp=1;
				break;
			case MF_NOICEWALL: // [Valaris]
				maps[m].flag.noicewall=1;
				break;
			case MF_SNOW: // [Valaris]
				maps[m].flag.snow=1;
				break;
			case MF_CLOUDS:
				maps[m].flag.clouds=1;
				break;
			case MF_CLOUDS2:
				maps[m].flag.clouds2=1;
				break;
			case MF_FOG: // [Valaris]
				maps[m].flag.fog=1;
				break;
			case MF_FIREWORKS:
				maps[m].flag.fireworks=1;
				break;
			case MF_SAKURA: // [Valaris]
				maps[m].flag.sakura=1;
				break;
			case MF_LEAVES: // [Valaris]
				maps[m].flag.leaves=1;
				break;
			case MF_RAIN: // [Valaris]
				maps[m].flag.rain=1;
				break;
			case MF_INDOORS: // celest
				maps[m].flag.indoors=1;
				break;
			case MF_NOGO: // celest
				maps[m].flag.nogo=1;
				break;
		}

		if( i==MF_SNOW ||
			i==MF_CLOUDS ||
			i==MF_CLOUDS2 ||
			i==MF_FOG ||
			i==MF_FIREWORKS ||
			i==MF_SAKURA ||
			i==MF_LEAVES ||
			i==MF_RAIN ||
			i==MF_INDOORS )
		clif_updateweather(m);
	}
	return 0;
}

int buildin_removemapflag(CScriptEngine &st)
{
	unsigned int m,i;
	const char *str;

	str=st.GetString(st[2]);
	i=st.GetInt(st[3]);
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m < map_num)
	{
		switch(i)
		{
			case MF_NOMEMO:
				maps[m].flag.nomemo=0;
				break;
			case MF_NOTELEPORT:
				maps[m].flag.noteleport=0;
				break;
			case MF_NOSAVE:
				maps[m].flag.nosave=0;
				break;
			case MF_NOBRANCH:
				maps[m].flag.nobranch=0;
				break;
			case MF_NOPENALTY:
				maps[m].flag.nopenalty=0;
				break;
			case MF_PVP:
				maps[m].flag.pvp=0;
				break;
			case MF_PVP_NOPARTY:
				maps[m].flag.pvp_noparty=0;
				break;
			case MF_PVP_NOGUILD:
				maps[m].flag.pvp_noguild=0;
				break;
			case MF_GVG:
				maps[m].flag.gvg=0;
				break;
			case MF_GVG_NOPARTY:
				maps[m].flag.gvg_noparty=0;
				break;
			case MF_GVG_DUNGEON:
				maps[m].flag.gvg_dungeon=0;
				break;
			case MF_NOZENYPENALTY:
				maps[m].flag.nozenypenalty=0;
				break;
			case MF_NOSKILL:
				maps[m].flag.noskill=0;
				break;
			case MF_NOWARP:
				maps[m].flag.nowarp=0;
				break;
			case MF_NOPVP:
				maps[m].flag.nopvp=0;
				break;
			case MF_NOICEWALL: // [Valaris]
				maps[m].flag.noicewall=0;
				break;
			case MF_SNOW: // [Valaris]
				maps[m].flag.snow=0;
				break;
			case MF_CLOUDS:
				maps[m].flag.clouds=0;
				break;
			case MF_CLOUDS2:
				maps[m].flag.clouds2=0;
				break;
			case MF_FOG: // [Valaris]
				maps[m].flag.fog=0;
				break;
			case MF_FIREWORKS:
				maps[m].flag.fireworks=0;
				break;
			case MF_SAKURA: // [Valaris]
				maps[m].flag.sakura=0;
				break;
			case MF_LEAVES: // [Valaris]
				maps[m].flag.leaves=0;
				break;
			case MF_RAIN: // [Valaris]
				maps[m].flag.rain=0;
				break;
			case MF_INDOORS: // celest
				maps[m].flag.indoors=0;
				break;
			case MF_NOGO: // celest
				maps[m].flag.nogo=0;
				break;
		}
		if( i==MF_SNOW ||
			i==MF_CLOUDS ||
			i==MF_CLOUDS2 ||
			i==MF_FOG ||
			i==MF_FIREWORKS ||
			i==MF_SAKURA ||
			i==MF_LEAVES ||
			i==MF_RAIN ||
			i==MF_INDOORS )
		clif_updateweather(m);
	}
	return 0;
}

int buildin_pvpon(CScriptEngine &st)
{
	size_t i;
	short m;
	const char *str;
	map_session_data *pl_sd=NULL;

	str=st.GetString(st[2]);
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && !maps[m].flag.pvp && !maps[m].flag.nopvp) {
		maps[m].flag.pvp = 1;
		clif_send0199(m,1);

		if(config.pk_mode) // disable ranking functions if pk_mode is on [Valaris]
			return 0;

		for(i=0;i<fd_max;++i){	//人数分ループ
			if(session[i] && (pl_sd=(map_session_data *) session[i]->user_session) && pl_sd->state.auth){
				if(m == pl_sd->block_list::m && pl_sd->pvp_timer == -1) {
					pl_sd->pvp_timer=add_timer(gettick()+200,pc_calc_pvprank_timer,pl_sd->block_list::id,0);
					pl_sd->pvp_rank=0;
					pl_sd->pvp_lastusers=0;
					pl_sd->pvp_point=5;
					pl_sd->pvp_won = 0;
					pl_sd->pvp_lost = 0;
				}
			}
		}
	}
	return 0;
}

int buildin_pvpoff(CScriptEngine &st)
{
	size_t i;
	short m;
	const char *str;
	map_session_data *pl_sd=NULL;

	str=st.GetString(st[2]);
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && maps[m].flag.pvp && maps[m].flag.nopvp) {
		maps[m].flag.pvp = 0;
		clif_send0199(m,0);

		if(config.pk_mode) // disable ranking options if pk_mode is on [Valaris]
			return 0;

		for(i=0;i<fd_max;++i){	//人数分ループ
			if(session[i] && (pl_sd=(map_session_data *) session[i]->user_session) && pl_sd->state.auth){
				if(m == pl_sd->block_list::m) {
					clif_pvpset(*pl_sd,0,0,2);
					if(pl_sd->pvp_timer != -1) {
						delete_timer(pl_sd->pvp_timer,pc_calc_pvprank_timer);
						pl_sd->pvp_timer = -1;
					}
				}
			}
		}
	}
	return 0;
}

int buildin_gvgon(CScriptEngine &st)
{
	int m;
	const char *str;

	str=st.GetString(st[2]);
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && !maps[m].flag.gvg) {
		maps[m].flag.gvg = 1;
		clif_send0199(m,3);
	}
	return 0;
}

int buildin_gvgoff(CScriptEngine &st)
{
	int m;
	const char *str;

	str=st.GetString(st[2]);
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && maps[m].flag.gvg) {
		maps[m].flag.gvg = 0;
		clif_send0199(m,0);
	}
	return 0;
}

/*==========================================
 *	NPCエモーション
 *------------------------------------------
 */
int buildin_emotion(CScriptEngine &st)
{
	uint32 type=st.GetInt(st[2]);
	if(type <= 100)
	{
		if( st.Arguments() > 3 )
		{
			map_session_data *sd = map_session_data::from_blid(st.GetInt(st[3]));
			if (sd)
				clif_emotion(*sd,type);
		}
		else if(st.nd)
		{
			clif_emotion(*st.nd,type);
		}
	}
	return 0;
}
class CBuildinRespawnGuild : public CMapProcessor
{
	uint32 g_id;
	int flag;
public:
	CBuildinRespawnGuild(uint32 g, int f) : g_id(g), flag(f)	{}
	~CBuildinRespawnGuild()	{}
	virtual int process(block_list& bl) const
	{
		map_session_data *sd;
		mob_data *md;
		if( (sd=bl.get_sd()) )
		{
			if( (sd->status.guild_id == 0) ||
				((sd->status.guild_id == g_id) && (flag&1)) ||
				((sd->status.guild_id != g_id) && (flag&2)) )
			{	// move players out that not belong here
				pc_setpos(*sd,sd->status.save_point.mapname,sd->status.save_point.x,sd->status.save_point.y,3);
			}
		}
		else if( (md=bl.get_md()) )
		{
			// guardians
			if( flag&4 && (md->class_ < 1285 || md->class_ > 1288) )
				md->remove_map(1);
		}
		return 0;
	}
};
int buildin_maprespawnguildid(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	int g_id=st.GetInt(st[3]);
	int flag=st.GetInt(st[4]);
	ushort m=map_mapname2mapid(mapname);
	if( m<map_num )
	{
		block_list::foreachinarea( CBuildinRespawnGuild(g_id,flag),
			m,0,0,maps[m].xs-1,maps[m].ys-1,BL_NUL);
//		map_foreachinarea(buildin_maprespawnguildid_sub,
//			m,0,0,maps[m].xs-1,maps[m].ys-1,BL_NUL,g_id,flag);
	}
	return 0;
}

int buildin_agitstart(CScriptEngine &st)
{
	if(agit_flag==1) return 1;      // Agit already Start.
	agit_flag=1;
	guild_agit_start();
	return 0;
}

int buildin_agitend(CScriptEngine &st)
{
	if(agit_flag==0) return 1;      // Agit already End.
	agit_flag=0;
	guild_agit_end();
	return 0;
}
/*==========================================
 * agitcheck 1;    // choice script
 * if(@agit_flag == 1) goto agit;
 * if(agitcheck(0) == 1) goto agit;
 *------------------------------------------
 */
int buildin_agitcheck(CScriptEngine &st)
{
	int cond=st.GetInt(st[2]);
	if(cond == 0) {
		if (agit_flag==1) st.push_val(CScriptEngine::C_INT,1);
		if (agit_flag==0) st.push_val(CScriptEngine::C_INT,0);
	}
	else
	{
		if (agit_flag==1) pc_setreg(*st.sd,add_str( "@agit_flag"),1);
		if (agit_flag==0) pc_setreg(*st.sd,add_str( "@agit_flag"),0);
	}
	return 0;
}

int buildin_flagemblem(CScriptEngine &st)
{
	if(st.nd)
	{
		int g_id=st.GetInt(st[2]);
		if(g_id > 0)
			st.nd->guild_id = g_id;

		//ShowMessage("buildin_flagemblem: [FlagEmblem] GuildID=%d, Emblem=%d.\n", g->guild_id, g->emblem_id);
	}
	return 1;
}

int buildin_getcastlename(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	struct guild_castle *gc;
	int i;
	char *buf=NULL;

	for(i=0;i<MAX_GUILDCASTLE;++i)
	{
		if( (gc=guild_castle_search(i)) != NULL )
		{
			if( strcasecmp(mapname,gc->mapname)==0 )
			{
				buf= new char[24];
				safestrcpy(buf,24,gc->castle_name);//EOS included
				break;
			}
		}
	}
	st.push_str(buf?CScriptEngine::C_STR:CScriptEngine::C_CONSTSTR, buf?buf:"");
	return 0;
}

int buildin_getcastledata(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	int index=st.GetInt(st[3]);
	struct guild_castle *gc;
	int i, val=0;

	for(i=0;i<MAX_GUILDCASTLE;++i)
	{
		if( (gc=guild_castle_search(i)) != NULL && 0==strcmp(mapname,gc->mapname) )
		{
			switch(index)
			{
			case  0:
				if( st.Arguments() > 4 )
				{
					const char *event = st.GetString(st[4]);
					guild_addcastleinfoevent(i,17,event);
				}
				for(i=1;i<26;++i) guild_castledataload(gc->castle_id,i);
				val = 0;
				break;  // Initialize[AgitInit]
			case  1: val = gc->guild_id; break;
			case  2: val = gc->economy; break;
			case  3: val = gc->defense; break;
			case  4: val = gc->triggerE; break;
			case  5: val = gc->triggerD; break;
			case  6: val = gc->nextTime; break;
			case  7: val = gc->payTime; break;
			case  8: val = gc->createTime; break;
			case  9: val = gc->visibleC; break;
			case 10: 
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
				val = gc->guardian[index-10].visible; break;
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
			case 25:
				val = gc->guardian[index-18].guardian_hp; break;
			}// end switch
		break; // the for loop
		}
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

int buildin_setcastledata(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	int index=st.GetInt(st[3]);
	int value=st.GetInt(st[4]);
	struct guild_castle *gc;
	int i;

	for(i=0;i<MAX_GUILDCASTLE;++i)
	{
		if( (gc=guild_castle_search(i)) != NULL )
		{
			if(strcmp(mapname,gc->mapname)==0)
			{
				// Save Data byself First
				switch(index)
				{
				case 1: gc->guild_id = value; break;
				case 2: gc->economy = value; break;
				case 3: gc->defense = value; break;
				case 4: gc->triggerE = value; break;
				case 5: gc->triggerD = value; break;
				case 6: gc->nextTime = value; break;
				case 7: gc->payTime = value; break;
				case 8: gc->createTime = value; break;
				case 9: gc->visibleC = value; break;
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
					gc->guardian[index-10].visible = (value!=0); break;
				case 18:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
				case 24:
				case 25:
					gc->guardian[index-18].guardian_hp = value; break;
				default:
					return 0;
				}
				guild_castledatasave(gc->castle_id,index,value);
				return 0;
			}
		}
	}
	return 0;
}

/* =====================================================================
 * ギルド情報を要求する
 * ---------------------------------------------------------------------
 */
int buildin_requestguildinfo(CScriptEngine &st)
{
	int guild_id=st.GetInt(st[2]);
	const char *event=NULL;

	if( st.Arguments() > 3 )
		event=st.GetString(st[3]);

	if(guild_id>0)
		guild_npc_request_info(guild_id,event);
	return 0;
}

/* =====================================================================
 * カードの数を得る
 * ---------------------------------------------------------------------
 */
int buildin_getequipcardcnt(CScriptEngine &st)
{
	int c=0;
	size_t num=st.GetInt(st[2]);
	if( st.sd && num<(sizeof(equip)/sizeof(equip[0])) )
	{
		size_t i=pc_checkequip(*st.sd,equip[num-1]);
		if( i<MAX_INVENTORY && st.sd->status.inventory[i].card[0] != 0x00ff )
		{	
			c=4;
			do
			{
				if( (st.sd->status.inventory[i].card[c-1] > 4000 &&
					st.sd->status.inventory[i].card[c-1] < 5000) ||
					itemdb_type(st.sd->status.inventory[i].card[c-1]) == 6)
				{
					break;
				}
				c--;
			} while(c);
		}
	}
	st.push_val(CScriptEngine::C_INT,c);
	return 0;
}

/* ================================================================
 * カード取り外し成功
 * ----------------------------------------------------------------
 */
int buildin_successremovecards(CScriptEngine &st)
{
	size_t num=st.GetInt(st[2]);
	if( st.sd && num < (sizeof(equip)/sizeof(equip[0])) )
	{
		size_t i=pc_checkequip(*st.sd,equip[num-1]);
		if(i<MAX_INVENTORY && st.sd->status.inventory[i].card[0]!=0x00ff)
		{
			int cardflag=0,flag;
			struct item item_tmp;
			int c=4;
			do
			{
				if( (st.sd->status.inventory[i].card[c-1] > 4000 &&
					st.sd->status.inventory[i].card[c-1] < 5000) ||
					itemdb_type(st.sd->status.inventory[i].card[c-1]) == 6)
				{	// [Celest]

					item_tmp.nameid = st.sd->status.inventory[i].card[c-1];
					item_tmp.equip=0;
					item_tmp.identify=1;
					item_tmp.refine=0;
					item_tmp.attribute=0;
					item_tmp.card[0]=0;
					item_tmp.card[1]=0;
					item_tmp.card[2]=0;
					item_tmp.card[3]=0;

					cardflag = 1;
					flag=pc_additem(*st.sd,item_tmp,1);
					if(flag)
					{	// 持てないならドロップ
						clif_additem(*st.sd,0,0,flag);
						map_addflooritem(item_tmp,1,st.sd->block_list::m,st.sd->block_list::x,st.sd->block_list::y,NULL,NULL,NULL,0);
					}
				}
				c--;
			}while(c);

			if(cardflag == 1)
			{	// カードを取り除いたアイテム所得
				item_tmp.nameid=st.sd->status.inventory[i].nameid;
				item_tmp.equip=0;
				item_tmp.identify=1;
				item_tmp.refine=st.sd->status.inventory[i].refine;
				item_tmp.attribute=st.sd->status.inventory[i].attribute;
				item_tmp.card[0]=0;
				item_tmp.card[1]=0;
				item_tmp.card[2]=0;
				item_tmp.card[3]=0;
				pc_delitem(*st.sd,i,1,0);
				flag=pc_additem(*st.sd,item_tmp,1);
				if(flag)
				{	// もてないならドロップ
					clif_additem(*st.sd,0,0,flag);
					map_addflooritem(item_tmp,1,st.sd->block_list::m,st.sd->block_list::x,st.sd->block_list::y,NULL,NULL,NULL,0);
				}
				clif_misceffect(*st.sd,3);
			}
		}
	}
	return 0;
}

/* ================================================================
 * カード取り外し失敗 slot,type
 * type=0: 両方損失、1:カード損失、2:武具損失、3:損失無し
 * ----------------------------------------------------------------
 */
int buildin_failedremovecards(CScriptEngine &st)
{
	size_t num=st.GetInt(st[2]);
	int typefail=st.GetInt(st[3]);

	if( st.sd && num < (sizeof(equip)/sizeof(equip[0])) )
	{
		int cardflag=0,flag;
		struct item item_tmp;
		int c=4;
		size_t i=pc_checkequip(*st.sd,equip[num-1]);
		if(i<MAX_INVENTORY && st.sd->status.inventory[i].card[0]!=0x00ff)
		{
			do
			{
				if( (st.sd->status.inventory[i].card[c-1] > 4000 &&
					st.sd->status.inventory[i].card[c-1] < 5000) ||
					itemdb_type(st.sd->status.inventory[i].card[c-1]) == 6)
				{
					cardflag = 1;
					if(typefail == 2)
					{	// 武具のみ損失なら、カードは受け取らせる
						item_tmp.nameid=st.sd->status.inventory[i].card[c-1];
						item_tmp.equip=0;
						item_tmp.identify=1;
						item_tmp.refine=0;
						item_tmp.attribute=0;
						item_tmp.card[0]=0;
						item_tmp.card[1]=0;
						item_tmp.card[2]=0;
						item_tmp.card[3]=0;
						flag=pc_additem(*st.sd,item_tmp,1);
						if(flag)
						{
							clif_additem(*st.sd,0,0,flag);
							map_addflooritem(item_tmp,1,st.sd->block_list::m,st.sd->block_list::x,st.sd->block_list::y,NULL,NULL,NULL,0);
						}
					}
				}
				c--;
			}
			while(c);

			if(cardflag == 1)
			{
				if(typefail == 0 || typefail == 2)
				{	// 武具損失
					pc_delitem(*st.sd,i,1,0);
				}
				else if(typefail == 1)
				{	// カードのみ損失（武具を返す）
					flag=0;
					item_tmp.nameid=st.sd->status.inventory[i].nameid;
					item_tmp.equip=0;
					item_tmp.identify=1;
					item_tmp.refine=st.sd->status.inventory[i].refine;
					item_tmp.attribute=st.sd->status.inventory[i].attribute;
					item_tmp.card[0]=0;
					item_tmp.card[1]=0;
					item_tmp.card[2]=0;
					item_tmp.card[3]=0;
					pc_delitem(*st.sd,i,1,0);
					if((flag=pc_additem(*st.sd,item_tmp,1)))
					{
						clif_additem(*st.sd,0,0,flag);
						map_addflooritem(item_tmp,1,st.sd->block_list::m,st.sd->block_list::x,st.sd->block_list::y,NULL,NULL,NULL,0);
					}
				}
				clif_misceffect(*st.sd,2);
			}
		}
	}
	return 0;
}

int buildin_mapwarp(CScriptEngine &st)	// Added by RoVeRT
{
	const char *mapname=st.GetString(st[2]);
	const char *targetmap=st.GetString(st[3]);
	int x=st.GetInt(st[4]);
	int y=st.GetInt(st[5]);
	ushort m = map_mapname2mapid(mapname);
	if( m<map_num )
	{	//!! broadcast command if not on this mapserver
		int x0=0;
		int y0=0;
		int x1=maps[m].xs;
		int y1=maps[m].ys;

		if( 0==strcmp(targetmap,"Random") )
			block_list::foreachinarea( CBuildinAreawarpRnd(),
				m,x0,y0,x1,y1,BL_PC);
		else
			block_list::foreachinarea( CBuildinAreawarpXY(targetmap,x,y),
				m,x0,y0,x1,y1,BL_PC);
	}
	return 0;
}

int buildin_cmdothernpc(CScriptEngine &st)
{
	const char *npcname=st.GetString(st[2]);
	const char *command=st.GetString(st[3]);
	npcscript_data* nd = npcscript_data::from_name(npcname);
	if(!nd)
		ShowError("npc_command: npc not found [%s]\n", npcname);
	else
		nd->command(command, st.sd);
	return 0;
}

int buildin_fakenpcname(CScriptEngine &st)
{
	const char *npcname = st.GetString(st[2]);
	const char *newname = st.GetString(st[3]);
	uint32 look = (st.Arguments() > 4) ? st.GetInt(st[4]) : 0;
	if(look <= 0xFFFF)	// Safety measure to prevent runtime errors
	{
		npc_data* nd = npc_data::from_name(npcname);
		if(!nd)
			ShowError("npc_command: npc not found [%s]\n", npcname);
		else
			nd->changename(newname, look);
	}
	return 0;
}

int buildin_inittimer(CScriptEngine &st)
{
	if(st.sd && st.nd)
		st.nd->do_ontimer(*st.sd, true);
	return 0;
}

int buildin_stoptimer(CScriptEngine &st)
{
	if(st.sd && st.nd)
		st.nd->do_ontimer(*st.sd, false);
	return 0;
}

class CBuildinMobCount : public CMapProcessor
{
	const char *event;
public:
	CBuildinMobCount(const char *e) : event(e)	{}
	~CBuildinMobCount()	{}
	virtual int process(block_list& bl) const
	{
		mob_data *md = bl.get_md();
		if(	md &&  (!event || 0==strcmp(event, md->npc_event)) )
			return 1;
		return 0;
	}
};
int buildin_mobcount(CScriptEngine &st)
{
	const char *mapname=st.GetString(st[2]);
	const char *event=st.GetString(st[3]);
	ushort m = map_mapname2mapid(mapname);
	int amount = -1;
	if( m<map_num )
	{	//!! broadcast if not on this mapserver
		amount = block_list::foreachinarea( CBuildinMobCount(event),
			m,0,0,maps[m].xs-1,maps[m].ys-1,BL_MOB);
		return 0;
	}
	st.push_val(CScriptEngine::C_INT, amount);
	return 0;
}
int buildin_marriage(CScriptEngine &st)
{
	const char *partner=st.GetString(st[2]);
	map_session_data *sd=st.sd;
	map_session_data *p_sd=map_session_data::nick2sd(partner);

	st.push_val(CScriptEngine::C_INT, (sd!=NULL && p_sd!=NULL && pc_marriage(*sd,*p_sd)) );

	return 0;
}

int buildin_wedding_effect(CScriptEngine &st)
{
	if(st.nd)
		clif_wedding_effect(*st.nd);
	return 0;
}
int buildin_divorce(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, (st.sd && pc_divorce(*st.sd)) );
	return 0;
}

int buildin_ispartneron(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT,
		(st.sd && pc_ismarried(*st.sd) &&
		NULL!=map_session_data::charid2sd(st.sd->status.partner_id))
		);
	return 0;
}

int buildin_getpartnerid(CScriptEngine &st)
{
	if(st.sd)
		st.push_val(CScriptEngine::C_INT,st.sd->status.partner_id);
	else
		st.push_val(CScriptEngine::C_INT,0);
	return 0;
}

int buildin_getchildid(CScriptEngine &st)
{
	if(st.sd)
		st.push_val(CScriptEngine::C_INT,st.sd->status.child_id);
	else
		st.push_val(CScriptEngine::C_INT,0);
	return 0;
}

int buildin_warppartner(CScriptEngine &st)
{
	int val=0;
	map_session_data *p_sd;
	if( st.sd && pc_ismarried(*st.sd) &&
		(p_sd=map_session_data::charid2sd(st.sd->status.partner_id)) )
	{
		const char *str=st.GetString(st[2]);
		int x=st.GetInt(st[3]);
		int y=st.GetInt(st[4]);
		
		pc_setpos(*p_sd,str,x,y,0);
	}
	st.push_val(CScriptEngine::C_INT,val);
	return 0;
}

/*================================================
 * Script for Displaying MOB Information [Valaris]
 *------------------------------------------------
 */
int buildin_strmobinfo(CScriptEngine &st)
{

	int num=st.GetInt(st[2]);
	int class_=st.GetInt(st[3]);

	if((class_>=0 && class_<=1000) || class_ >2000)
		return 0;

	switch (num) {
	case 1:
	{
		char *buf;
		buf = new char[24];
		strcpy(buf,mob_db[class_].name);
		st.push_str(CScriptEngine::C_STR, buf);
		break;
	}
	case 2:
	{
		char *buf;
		buf= new char[24];
		strcpy(buf,mob_db[class_].jname);
		st.push_str(CScriptEngine::C_STR, buf);
		break;
	}
	case 3:
		st.push_val(CScriptEngine::C_INT,mob_db[class_].lv);
		break;
	case 4:
		st.push_val(CScriptEngine::C_INT,mob_db[class_].max_hp);
		break;
	case 5:
		st.push_val(CScriptEngine::C_INT,mob_db[class_].max_sp);
		break;
	case 6:
		st.push_val(CScriptEngine::C_INT,mob_db[class_].base_exp);
		break;
	case 7:
		st.push_val(CScriptEngine::C_INT,mob_db[class_].job_exp);
		break;
	}
	return 0;
}

/*==========================================
 * Summon guardians [Valaris]
 *------------------------------------------
 */
int buildin_guardian(CScriptEngine &st)
{
	int class_=0,amount=1,x=0,y=0,guardian=0;
	const char *str,*map,*event="";

	map	=st.GetString(st[2]);
	x	=st.GetInt(st[3]);
	y	=st.GetInt(st[4]);
	str	=st.GetString(st[5]);
	class_=st.GetInt(st[6]);
	amount=st.GetInt(st[7]);
	event=st.GetString(st[8]);
	if( st.Arguments() > 9 )
		guardian=st.GetInt(st[9]);

	mob_spawn_guardian(st.sd,map,x,y,str,class_,amount,event,guardian);

	return 0;
}

/*================================================
 * Script for Displaying Guardian Info [Valaris]
 *------------------------------------------------
 */
int buildin_guardianinfo(CScriptEngine &st)
{
	int index=st.GetInt(st[2]);
	map_session_data *sd=st.sd;
	struct guild_castle *gc=guild_mapname2gc(maps[sd->block_list::m].mapname);
	st.push_val(CScriptEngine::C_INT,
		(index>=0 && index<MAX_GUARDIAN && gc && gc->guardian[index].visible) ?
		(int)gc->guardian[index].guardian_hp : -1		
		);
	return 0;
}
/*==========================================
 * IDからItem名
 *------------------------------------------
 */
int buildin_getitemname(CScriptEngine &st)
{
	struct item_data *item_data=NULL;
	CScriptEngine::CValue &data = st[2];
	st.ConvertName(data);

	if( data.isString() )
	{
		const char *name=st.GetString(data);
		item_data = itemdb_searchname(name);
	}
	else
	{
		int item_id=st.GetInt(data);
		item_data = itemdb_exists(item_id);
	}
	if(item_data)
	{
		char *item_name;
		item_name= new char[24];
		memcpy(item_name,item_data->jname,24);//EOS included
		st.push_str(CScriptEngine::C_STR, item_name);
	}
	else
		st.push_str(CScriptEngine::C_CONSTSTR, "unknown");
	return 0;
}

/*==========================================
 * petskillbonus [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */

int buildin_petskillbonus(CScriptEngine &st)
{
	if(st.sd && st.sd->pd)
	{
		struct pet_data *pd=st.sd->pd;
		if (pd->bonus)
		{	//Clear previous bonus
			if (pd->bonus->timer != -1)
				delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
		}
		else //init
		{
			pd->bonus = new pet_data::pet_bonus;
		}
		
		pd->bonus->type=st.GetInt(st[2]);
		pd->bonus->val=st.GetInt(st[3]);
		pd->bonus->duration=st.GetInt(st[4]);
		pd->bonus->delay=st.GetInt(st[5]);

		if (pd->state.skillbonus == -1)
			pd->state.skillbonus=0;	// waiting state

		// wait for timer to start
		if (config.pet_equip_required && pd->pet.equip_id == 0)
			pd->bonus->timer=-1;
		else
			pd->bonus->timer=add_timer(gettick()+pd->bonus->delay*1000, pet_skill_bonus_timer, st.sd->block_list::id, 0);
	}
	return 0;
}

/*==========================================
 * pet looting [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petloot(CScriptEngine &st)
{
	int max;
	struct pet_data *pd;
	map_session_data *sd=st.sd;
	
	if(sd==NULL || sd->pd==NULL)
		return 0;

	max=st.GetInt(st[2]);

	if(max < 1)
		max = 1;	//Let'em loot at least 1 item.
	else if (max > MAX_PETLOOT_SIZE)
		max = MAX_PETLOOT_SIZE;
	
	pd = sd->pd;
	if(pd && pd->loot)
	{	//Release whatever was there already and reallocate memory
		pd->droploot();
		delete[] pd->loot->itemlist;
	}
	else
	{
		pd->loot = new pet_data::pet_loot;
	}

	pd->loot->itemlist = new struct item[max];
	pd->loot->max=max;
	pd->loot->count = 0;
	pd->loot->weight = 0;
	pd->loot->loottick = gettick();

	return 0;
}
/*==========================================
 * PCの所持品情報読み取り
 *------------------------------------------
 */
int buildin_getinventorylist(CScriptEngine &st)
{
	map_session_data *sd=st.sd;
	int i,j=0;
	if(!sd) return 0;
	for(i=0;i<MAX_INVENTORY;++i){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount > 0){
			pc_setreg(*sd,add_str( "@inventorylist_id")+(j<<24),sd->status.inventory[i].nameid);
			pc_setreg(*sd,add_str( "@inventorylist_amount")+(j<<24),sd->status.inventory[i].amount);
			pc_setreg(*sd,add_str( "@inventorylist_equip")+(j<<24),sd->status.inventory[i].equip);
			pc_setreg(*sd,add_str( "@inventorylist_refine")+(j<<24),sd->status.inventory[i].refine);
			pc_setreg(*sd,add_str( "@inventorylist_identify")+(j<<24),sd->status.inventory[i].identify);
			pc_setreg(*sd,add_str( "@inventorylist_attribute")+(j<<24),sd->status.inventory[i].attribute);
			pc_setreg(*sd,add_str( "@inventorylist_card1")+(j<<24),sd->status.inventory[i].card[0]);
			pc_setreg(*sd,add_str( "@inventorylist_card2")+(j<<24),sd->status.inventory[i].card[1]);
			pc_setreg(*sd,add_str( "@inventorylist_card3")+(j<<24),sd->status.inventory[i].card[2]);
			pc_setreg(*sd,add_str( "@inventorylist_card4")+(j<<24),sd->status.inventory[i].card[3]);
			j++;
		}
	}
	pc_setreg(*sd,add_str( "@inventorylist_count"),j);
	return 0;
}

int buildin_getskilllist(CScriptEngine &st)
{
	map_session_data *sd=st.sd;
	int i,j=0;
	if(!sd) return 0;
	for(i=0;i<MAX_SKILL;++i){
		if(sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0){
			pc_setreg(*sd,add_str("@skilllist_id")+(j<<24),sd->status.skill[i].id);
			pc_setreg(*sd,add_str("@skilllist_lv")+(j<<24),sd->status.skill[i].lv);
			pc_setreg(*sd,add_str("@skilllist_flag")+(j<<24),sd->status.skill[i].flag);
			j++;
		}
	}
	pc_setreg(*sd,add_str( "@skilllist_count"),j);
	return 0;
}

int buildin_clearitem(CScriptEngine &st)
{
	if(st.sd)
	{
		size_t i;
		for (i=0; i<MAX_INVENTORY; ++i)
		{
			if(st.sd->status.inventory[i].amount)
				pc_delitem(*st.sd, i, st.sd->status.inventory[i].amount, 0);
		}
	}
	return 0;
}

/*==========================================
 * NPCクラスチェンジ
 * classは変わりたいclass
 * typeは通常0なのかな？
 *------------------------------------------
 */
int buildin_classchange(CScriptEngine &st)
{
	if(st.nd)
	{
		int class_=st.GetInt(st[2]);
		int type=st.GetInt(st[3]);
		clif_class_change(*st.nd,class_,type);
	}
	return 0;
}

/*==========================================
 * NPCから発生するエフェクト
 *------------------------------------------
 */
int buildin_misceffect(CScriptEngine &st)
{
	const block_list* bl = (st.nd)?((block_list*)st.nd):(st.sd)?((block_list*)st.sd):NULL;
	if(bl)
	{
		int effect = st.GetInt(st[2]);
		clif_setareaeffect(*bl, effect);
	}
	return 0;
}
/*==========================================
 * サウンドエフェクト
 *------------------------------------------
 */
int buildin_soundeffect(CScriptEngine &st)
{
	if(st.sd)
	{
		const char *name=st.GetString(st[2]);
		int type=st.GetInt(st[3]);
		if(st.nd)
			clif_soundeffect(*st.sd,*st.nd,name,type);
		else
			clif_soundeffect(*st.sd,*st.sd,name,type);
	}
	return 0;
}

int buildin_soundeffectall(CScriptEngine &st)
{
	const char *name=st.GetString(st[2]);
	int type=st.GetInt(st[3]);
	if(st.nd)
		clif_soundeffectall(*st.nd,name,type);
	else if(st.sd)
		clif_soundeffectall(*st.sd,name,type);
	return 0;
}
/*==========================================
 * pet status recovery [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petrecovery(CScriptEngine &st)
{
	struct pet_data *pd;

	if(st.sd==NULL || st.sd->pd==NULL)
		return 0;

	pd=st.sd->pd;
	
	if (pd->recovery)
	{ //Halt previous bonus
		if (pd->recovery->timer != -1)
			delete_timer(pd->recovery->timer, pet_recovery_timer);
	}
	else //Init
	{
		pd->recovery = new pet_data::pet_recovery;
	}
		
	pd->recovery->type=st.GetInt(st[2]);
	pd->recovery->delay=st.GetInt(st[3]);
	pd->recovery->timer=-1;

	return 0;
}

/*==========================================
 * pet healing [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petheal(CScriptEngine &st)
{
	struct pet_data *pd;

	if(st.sd==NULL || st.sd->pd==NULL)
		return 0;

	pd=st.sd->pd;
	if (pd->s_skill)
	{	//Clear previous skill
		if (pd->s_skill->timer != -1)
		{
			if (pd->s_skill->id)
				delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				delete_timer(pd->s_skill->timer, pet_heal_timer);
		}
	}
	else //init memory
	{
		pd->s_skill = new pet_data::pet_skill_support;
	}
	
	//This id identifies that it IS petheal rather than pet_skillsupport
	pd->s_skill->id=0;
	//Use the lv as the amount to heal
	pd->s_skill->lv=st.GetInt(st[2]);
	pd->s_skill->delay=st.GetInt(st[3]);
	pd->s_skill->hp=st.GetInt(st[4]);
	pd->s_skill->sp=st.GetInt(st[5]);

	//Use delay as initial offset to avoid skill/heal exploits
	if (config.pet_equip_required && pd->pet.equip_id == 0)
		pd->s_skill->timer=-1;
	else
		pd->s_skill->timer=add_timer(gettick()+pd->s_skill->delay*1000,pet_heal_timer,st.sd->block_list::id,0);

	return 0;
}

/*==========================================
 * pet attack skills [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petskillattack(CScriptEngine &st)
{
	struct pet_data *pd;
	if(st.sd==NULL || st.sd->pd==NULL)
		return 0;

	pd=st.sd->pd;
	if (pd->a_skill == NULL)
	{
		pd->a_skill = new pet_data::pet_skill_attack;
	}
				
	pd->a_skill->id=st.GetInt(st[2]);
	pd->a_skill->lv=st.GetInt(st[3]);
	pd->a_skill->div_ = 0;
	pd->a_skill->rate=st.GetInt(st[4]);
	pd->a_skill->bonusrate=st.GetInt(st[5]);

	return 0;
}

/*==========================================
 * pet attack skills [Valaris]
 *------------------------------------------
 */
int buildin_petskillattack2(CScriptEngine &st)
{
	struct pet_data *pd;
	map_session_data *sd=st.sd;

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->a_skill == NULL)
	{
		pd->a_skill = new pet_data::pet_skill_attack;
	}
				
	pd->a_skill->id=st.GetInt(st[2]);
	pd->a_skill->lv=st.GetInt(st[3]);
	pd->a_skill->div_ = st.GetInt(st[4]);
	pd->a_skill->rate=st.GetInt(st[5]);
	pd->a_skill->bonusrate=st.GetInt(st[6]);

	return 0;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------
 */
int buildin_petskillsupport(CScriptEngine &st)
{
	struct pet_data *pd;
	map_session_data *sd=st.sd;

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != -1)
		{
			if (pd->s_skill->id)
				delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				delete_timer(pd->s_skill->timer, pet_heal_timer);
		}
	} else //init memory
	{
		pd->s_skill = new pet_data::pet_skill_support;
	}
	
	pd->s_skill->id=st.GetInt(st[2]);
	pd->s_skill->lv=st.GetInt(st[3]);
	pd->s_skill->delay=st.GetInt(st[4]);
	pd->s_skill->hp=st.GetInt(st[5]);
	pd->s_skill->sp=st.GetInt(st[6]);

	//Use delay as initial offset to avoid skill/heal exploits
	if (config.pet_equip_required && pd->pet.equip_id == 0)
		pd->s_skill->timer=-1;
	else
		pd->s_skill->timer=add_timer(gettick()+pd->s_skill->delay*1000,pet_skill_support_timer,sd->block_list::id,0);

	return 0;
}

/*==========================================
 * Scripted skill effects [Celest]
 *------------------------------------------
 */
int buildin_skilleffect(CScriptEngine &st)
{
	map_session_data *sd;

	int skillid=st.GetInt(st[2]);
	int skilllv=st.GetInt(st[3]);
	sd=st.sd;

	clif_skill_nodamage(*sd,*sd,skillid,skilllv,1);

	return 0;
}

/*==========================================
 * NPC skill effects [Valaris]
 *------------------------------------------
 */
int buildin_npcskilleffect(CScriptEngine &st)
{
	if(st.nd)
	{
		int skillid=st.GetInt(st[2]);
		int skilllv=st.GetInt(st[3]);
		int x=st.GetInt(st[4]);
		int y=st.GetInt(st[5]);

		clif_skill_poseffect(*st.nd,skillid,skilllv,x,y,gettick());
	}
	return 0;
}

/*==========================================
 * Special effects [Valaris]
 *------------------------------------------
 */
int buildin_specialeffect(CScriptEngine &st)
{
	if(st.nd)
		clif_specialeffect(*st.nd,st.GetInt( (st[2])), 0);
	return 0;
}

int buildin_specialeffect2(CScriptEngine &st)
{
	map_session_data *sd=st.sd;

	if(sd) clif_specialeffect(*sd,st.GetInt( (st[2])), 0);
		return 0;
}

/*==========================================
 * Nude [Valaris]
 *------------------------------------------
 */

int buildin_nude(CScriptEngine &st)
{
	map_session_data *sd=st.sd;
	if(sd)
	{
		size_t i;
		register bool calcflag=false;
		for(i=0;i<MAX_EQUIP;++i)
		{
			if(sd->equip_index[i] < MAX_INVENTORY)
			{
				pc_unequipitem(*sd,sd->equip_index[i],2);
				calcflag=true;
			}
		}
		if(calcflag)
			status_calc_pc(*sd,1);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// gmcommand.
/// uses the a dummy mapsession when running without attached player.
/// note the modified calling convention
int buildin_gmcommand(CScriptEngine &st)
{
	// operator new is cleaning the whole structure
	map_session_data *obj = (st.sd)?st.sd : new map_session_data(0,0,0,0,0,0,0,0);
	const char *cmd = st.GetString(st[2]);
	char buffer[512];

	//////////////////////////////
	// security for new command format
	//## remove at the latest by 12/2006
	const char *ip = strchr(cmd, ':');
	if(ip)
	{
		cmd = ip;
		++cmd;
		while( basics::stringcheck::isspace(*cmd) ) ++cmd;
	}
	while( !basics::stringcheck::isalpha(*cmd) )
	{
		ip = cmd;
		++cmd;
	}
	if(ip)
	{	
		ShowWarning("invalid use of \"%s\"\n"
					CL_SPACE"new calling format for gmcommand function is\n"
					CL_SPACE"\"<command without leading command char> {parameter}\"\n"
					CL_SPACE"for example: \"summon poring\"\n"
					CL_SPACE"now using: \"%s\" as command\n", st.GetString(st[2]), cmd);
	}
	//////////////////////////////

	// build the command string
	int ret = snprintf(buffer, sizeof(buffer), "%s : %c%s", obj->status.name, CommandInfo::command_symbol, cmd);
	// add the rest of the script parameters to the string
	size_t i, sz=0;
	for(i=3; i<st.Arguments() && ret>=0; ++i)
	{
		sz+=ret;
		ret = snprintf(buffer+sz, sizeof(buffer)-sz, " %s", st.GetString(st[i]));
	}
	// make sure it's terminated even when buffer limit was hit
	buffer[sizeof(buffer)-1]=0;

	// and call the command processor
	CommandInfo::is_command(obj->fd, *obj, buffer, 99);

	if(!st.sd)
		delete obj;

	return 0;
}


/*==========================================
 * Displays a message for the player only (like system messages like "you got an apple" )
 *------------------------------------------
 */
int buildin_dispbottom(CScriptEngine &st)
{
	if(st.sd)
		clif_disp_onlyself(*st.sd, st.GetString(st[2]));
	return 0;
}

/*==========================================
 * All The Players Full Recovery
   (HP/SP full restore and resurrect if need)
 *------------------------------------------
 */
int buildin_recovery(CScriptEngine &st)
{
	size_t i;
	for (i = 0; i < fd_max; ++i) {
		if (session[i]){
			map_session_data *sd = (map_session_data *) session[i]->user_session;
			if (sd && sd->state.auth) {
				sd->status.hp = sd->status.max_hp;
				sd->status.sp = sd->status.max_sp;
				clif_updatestatus(*sd, SP_HP);
				clif_updatestatus(*sd, SP_SP);
				if( sd->is_dead() )
				{
					sd->set_stand();
					clif_resurrection(*sd, 1);
				}
				clif_displaymessage(sd->fd,"You have been recovered!");
			}
		}
	}
	return 0;
}
/*==========================================
 * Get your pet info: getpetinfo(n)  
 * n -> 0:pet_id 1:pet_class 2:pet_name
	3:friendly 4:hungry
 *------------------------------------------
 */
int buildin_getpetinfo(CScriptEngine &st)
{
	map_session_data *sd=st.sd;
	int type=st.GetInt(st[2]);

	if(sd && sd->status.pet_id && sd->pd)
	{
		switch(type){
			case 0:
				st.push_val(CScriptEngine::C_INT,sd->status.pet_id);
				break;
			case 1:
				st.push_val(CScriptEngine::C_INT,sd->pd->pet.class_);
				break;
			case 2:
			{
				char *buf = new char[24];
				memcpy(buf,sd->pd->pet.name, 24);//EOS included
				st.push_str(CScriptEngine::C_STR, buf);
				break;
			}
			case 3:
				st.push_val(CScriptEngine::C_INT,sd->pd->pet.intimate);
				break;
			case 4:
				st.push_val(CScriptEngine::C_INT,sd->pd->pet.hungry);
				break;
			default:
				st.push_val(CScriptEngine::C_INT,0);
				break;
		}
	}else{
		st.push_val(CScriptEngine::C_INT,0);
	}
	return 0;
}
/*==========================================
 * Shows wether your inventory(and equips) contain
   selected card or not.
	checkequipedcard(4001);
 *------------------------------------------
 */
int buildin_checkequipedcard(CScriptEngine &st)
{
	map_session_data *sd=st.sd;
	int n,i,c=0;
	c=st.GetInt(st[2]);

	if(sd){
		for(i=0;i<MAX_INVENTORY;++i){
			if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount){
				for(n=0;n<4;++n){
					if(sd->status.inventory[i].card[n]==c){
						st.push_val(CScriptEngine::C_INT,1);
						return 0;
					}
				}
			}
		}
	}
	st.push_val(CScriptEngine::C_INT,0);
	return 0;
}


/*==========================================
 * GetMapMobs
	returns mob counts on a set map:
	e.g. GetMapMobs("prontera.gat")
	use "this" - for player's map
 *------------------------------------------
 */
int buildin_getmapmobs(CScriptEngine &st)
{
	unsigned short m=0xFFFF;
	int count=-1;
	const char *str=st.GetString(st[2]);

	if( 0==strcmp(str,"this") ) 
	{
		if(st.sd)
			m=st.sd->block_list::m;
	}
	else
		m=map_mapname2mapid(str);
	if(m < map_num)
	{
		count = block_list::foreachinarea( CBuildinMobCount(NULL),
			m,0,0,maps[m].xs-1,maps[m].ys-1,BL_MOB);
	}
	st.push_val(CScriptEngine::C_INT,count);
	return 0;
}

/*==========================================
 * movenpc [MouseJstr]
 *------------------------------------------
 */

int buildin_movenpc(CScriptEngine &st)
{
	map_session_data *sd;
	const char *map,*npc;
	int x,y;

	sd = st.sd;

	map = st.GetString(st[2]);
	x = st.GetInt(st[3]);
	y = st.GetInt(st[4]);
	npc = st.GetString(st[5]);

	return 0;
}

/*==========================================
 * message [MouseJstr]
 *------------------------------------------
 */

int buildin_message(CScriptEngine &st)
{
	map_session_data *sd;
	const char *msg,*player;
	map_session_data *pl_sd = NULL;

	sd = st.sd;

	player = st.GetString(st[2]);
	msg = st.GetString(st[3]);

	if((pl_sd=map_session_data::nick2sd((char *) player)) == NULL)
             return 1;
	clif_displaymessage(pl_sd->fd, msg);

	return 0;
}

/*==========================================
 * npctalk (sends message to surrounding
 * area) [Valaris]
 *------------------------------------------
 */

int buildin_npctalk(CScriptEngine &st)
{
	if(st.nd)
	{
		const char *str=st.GetString(st[2]);
		char message[1024];
		snprintf(message, sizeof(message), "%s: %s", st.nd->name, str);
		message[sizeof(message)-1]=0;
		clif_message(*st.nd, message);
	}

	return 0;
}

/*==========================================
 * hasitems (checks to see if player has any
 * items on them, if so will return a 1)
 * [Valaris]
 *------------------------------------------
 */

int buildin_hasitems(CScriptEngine &st)
{
	int i;
	map_session_data *sd;

	sd=st.sd;

	for(i=0; i<MAX_INVENTORY; ++i) {
		if(sd->status.inventory[i].amount && sd->status.inventory[i].nameid!=2364 && sd->status.inventory[i].nameid!=2365)
		{
			st.push_val(CScriptEngine::C_INT,1);
			return 0;
		}
	}
	st.push_val(CScriptEngine::C_INT,0);
	return 0;
}
// change npc walkspeed [Valaris]
int buildin_npcspeed(CScriptEngine &st)
{
	if(st.nd)
	{
		st.nd->speed=st.GetInt(st[2]);
	}
	return 0;
}
// make an npc walk to a position [Valaris]
int buildin_npcwalkto(CScriptEngine &st)
{
	if(st.nd)
	{
		int x=st.GetInt(st[2]);
		int y=st.GetInt(st[3]);

		st.nd->walktoxy(x,y);
	}
	return 0;
}
// stop an npc's movement [Valaris]
int buildin_npcstop(CScriptEngine &st)
{
	if( st.nd )
		st.nd->stop_walking(1);
	return 0;
}


/*==========================================
  * getlook char info. getlook(arg)
  *------------------------------------------
  */
int buildin_getlook(CScriptEngine &st){
        int type,val;
        map_session_data *sd;
        sd=st.sd;

        type=st.GetInt(st[2]);
        val=-1;
        switch(type){
        case LOOK_HAIR:	//1
                val=sd->status.hair;
                break;
        case LOOK_WEAPON: //2
                val=sd->status.weapon;
                break;
        case LOOK_HEAD_BOTTOM: //3
                val=sd->status.head_bottom;
                break;
        case LOOK_HEAD_TOP: //4
                val=sd->status.head_top;
                break;
        case LOOK_HEAD_MID: //5
                val=sd->status.head_mid;
                break;
        case LOOK_HAIR_COLOR: //6
                val=sd->status.hair_color;
                break;
        case LOOK_CLOTHES_COLOR: //7
                val=sd->status.clothes_color;
                break;
        case LOOK_SHIELD: //8
                val=sd->status.shield;
                break;
        case LOOK_SHOES: //9
                break;
        }
        st.push_val(CScriptEngine::C_INT,val);
        return 0;
}

/*==========================================
  *     get char save point. argument: 0- map name, 1- x, 2- y
  *------------------------------------------
*/
int buildin_getsavepoint(CScriptEngine &st)
{
	int type=st.GetInt(st[2]);
	switch(type)
	{
	case 0:
	{
		char *mapname= new char[24];
		safestrcpy(mapname,24,st.sd ? st.sd->status.save_point.mapname : "unknown");
		st.push_str(CScriptEngine::C_STR,mapname);
		break;
	}
	case 1:
		st.push_val(CScriptEngine::C_INT, st.sd ? st.sd->status.save_point.x : 0 );
		break;
	case 2:
		st.push_val(CScriptEngine::C_INT, st.sd ? st.sd->status.save_point.y : 0);
		break;
	default:
		st.push_str(CScriptEngine::C_CONSTSTR, "");
	}
	return 0;
}

/*==========================================
  * Get position for  char/npc/pet/mob objects. Added by Lorky
  *
  *     int getMapXY(MapName$,MaxX,MapY,type,[CharName$]);
  *             where type:
  *                     MapName$ - String variable for output map name
  *                     MapX     - Integer variable for output coord X
  *                     MapY     - Integer variable for output coord Y
  *                     type     - type of object
  *                                0 - Character coord
  *                                1 - NPC coord
  *                                2 - Pet coord
  *                                3 - Mob coord (not released)
  *                     CharName$ - Name object. If miss or "this" the current object
  *
  *             Return:
  *                     0        - success
  *                     -1       - some error, MapName$,MapX,MapY contains unknown value.
  *------------------------------------------
*/
int buildin_getmapxy(CScriptEngine &st)
{
	int ret = -1;
	if(st.sd)
	{
		map_session_data *sd=NULL;
		npc_data *nd;
		struct pet_data *pd;
		int num;
		const char *name;
		int x,y,type;
		const char *mapname=NULL;

		if( st[2].type!=CScriptEngine::C_NAME )
		{
			ShowMessage("script: buildin_getmapxy: not mapname variable\n");
			st.push_val(CScriptEngine::C_INT,-1);
			return 0;
		}
		if( st[3].type!=CScriptEngine::C_NAME )
		{
			ShowMessage("script: buildin_getmapxy: not mapx variable\n");
			st.push_val(CScriptEngine::C_INT,-1);
			return 0;
		}
		if( st[4].type!=CScriptEngine::C_NAME )
		{
			ShowMessage("script: buildin_getmapxy: not mapy variable\n");
			st.push_val(CScriptEngine::C_INT,-1);
			return 0;
		}
		
		type=st.GetInt(st[5]);
		switch(type)
		{
		case 0:	//Get Character Position
			if( st.Arguments() > 6 )
				sd=map_session_data::nick2sd( st.GetString(st[6]) );
			else
				sd=st.sd;
			if( sd==NULL )
			{	//wrong char name or char offline
				st.push_val(CScriptEngine::C_INT,-1);
				return 0;
			}
			x=sd->block_list::x;
			y=sd->block_list::y;
			mapname = sd->mapname;
			ShowMessage(">>>>%s %d %d\n",mapname,x,y);
			break;
		case 1:	//Get NPC Position
			if( st.Arguments() > 6 )
				nd= npc_data::from_name( st.GetString(st[6]) );
			else
				nd= st.nd;
			if( nd==NULL )
			{	//wrong npc name or char offline
				st.push_val(CScriptEngine::C_INT,-1);
				return 0;
			}
			x=nd->block_list::x;
			y=nd->block_list::y;
			mapname=maps[nd->block_list::m].mapname;
			ShowMessage(">>>>%s %d %d\n",mapname,x,y);
			break;
		case 2:	//Get Pet Position
			if( st.Arguments() > 6 )
				sd=map_session_data::nick2sd( st.GetString(st[6]) );
			else
				sd=st.sd;
			if( sd==NULL )
			{	//wrong char name or char offline
				st.push_val(CScriptEngine::C_INT,-1);
				return 0;
			}
			pd=sd->pd;
			if(pd==NULL)
			{	//ped data not found
				st.push_val(CScriptEngine::C_INT,-1);
				return 0;
			}
			x=pd->block_list::x;
			y=pd->block_list::y;
			mapname=maps[pd->block_list::m].mapname;
			ShowMessage(">>>>%s %d %d\n",mapname,x,y);
			break;
		case 3:	//Get Mob Position
			st.push_val(CScriptEngine::C_INT,-1);
			return 0;
		default:	//Wrong type parameter
			st.push_val(CScriptEngine::C_INT,-1);
			return 0;
		}//end switch
	
		//Set MapName$
		num=st[2].num;
		name=str_data[num&0x00ffffff].string();
		set_reg(st,num,name,(void*)mapname);
		//Set MapX
		num=st[3].num;
		name=str_data[num&0x00ffffff].string();
		set_reg(st,num,name,(void*)((size_t)x));
		//Set MapY
		num=st[4].num;
		name=str_data[num&0x00ffffff].string();
		set_reg(st,num,name,(void*)((size_t)y));
		//Return Success value
		ret = 0;
	}
	st.push_val(CScriptEngine::C_INT, ret);
	return 0;
}

/*=====================================================
 * Allows players to use a skill - by Qamera
 *-----------------------------------------------------
 */
int buildin_skilluseid (CScriptEngine &st)
{
	int skid,sklv;
	map_session_data *sd;
	skid=st.GetInt(st[2]);
	sklv=st.GetInt(st[3]);
	sd=st.sd;
	if(sd) skill_use_id(sd,sd->status.account_id,skid,sklv);
	return 0;
}

/*=====================================================
 * Allows players to use a skill on a position [Celest]
 *-----------------------------------------------------
 */
int buildin_skillusepos(CScriptEngine &st)
{
	int skid,sklv,x,y;
	map_session_data *sd;
	skid=st.GetInt(st[2]);
	sklv=st.GetInt(st[3]);
	x=st.GetInt(st[4]);
	y=st.GetInt(st[5]);
	sd=st.sd;
	if(sd) skill_use_pos(sd,x,y,skid,sklv);
	return 0;
}

/*==========================================
 * Allows player to write NPC logs (i.e. Bank NPC, etc) [Lupus]
 *------------------------------------------
 */
int buildin_logmes(CScriptEngine &st)
{
	if( log_config.npc && st.sd)
		log_npc(*st.sd, st.GetString(st[2]));
	return 0;
}

int buildin_summon(CScriptEngine &st)
{
	int class_, id;
	const char *str,*event="";
	map_session_data *sd;
	mob_data *md;

	sd=st.sd;
	if (sd) {
		unsigned long tick = gettick();
		str	=st.GetString(st[2]);
		class_=st.GetInt(st[3]);
		if( st.Arguments() > 4 )
			event=st.GetString(st[4]);

		id=mob_once_spawn(sd, "this", 0, 0, str,class_,1,event);
		if( (md = mob_data::from_blid(id)) )
		{
			md->master_id=sd->block_list::id;
			md->state.special_mob_ai=1;
			md->mode=mob_db[md->class_].mode|0x04;
			md->deletetimer=add_timer(tick+60000,mob_timer_delete,id,0);
			clif_setareaeffect(*md,344);
		}
		clif_skill_poseffect(*sd,AM_CALLHOMUN,1,sd->block_list::x,sd->block_list::y,tick);
	}

	return 0;
}

/*==========================================
 * Checks whether it is daytime/nighttime
 *------------------------------------------
 */
int buildin_isnight(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, (daynight_flag == 1));
	return 0;
}

int buildin_isday(CScriptEngine &st)
{
	st.push_val(CScriptEngine::C_INT, (daynight_flag == 0));
	return 0;
}

/*================================================
 * Check whether another card has been
 * equipped - used for 2/15's cards patch [celest]
 * -- Items checked cannot be reused in another
 * card set to prevent exploits
 *------------------------------------------------
 */
int buildin_isequipped(CScriptEngine &st)
{
	int ret = 0;
	if(st.sd)
	{
		size_t i, j, k;
		unsigned short id = 1;
		ret = -1;

		for (i=0; id!=0; ++i)
		{
			int flag = 0;
			if( st.Arguments() > (i+2) )
				id=st.GetInt(st[(i+2)]);
			else
				id = 0;
			if (id <= 0)
				continue;
			for (j=0; j<10; ++j)
			{
				int index, type;
				index = st.sd->equip_index[j];
				if(index >= MAX_INVENTORY) continue;
				if(j == 9 && st.sd->equip_index[8] == index) continue;
				if(j == 5 && st.sd->equip_index[4] == index) continue;
				if(j == 6 && (st.sd->equip_index[5] == index || st.sd->equip_index[4] == index)) continue;
				type = itemdb_type(id);
				
				if(st.sd->inventory_data[index])
				{
					if (type == 4 || type == 5)
					{
						if (st.sd->inventory_data[index]->nameid == id)
							flag = 1;
					}
					else if (type == 6)
					{	// Item Hash format:
						// 1111 1111 1111 1111 1111 1111 1111 1111
						// [ left  ] [ right ] [ NA ] [  armor  ]
						for (k = 0; k < st.sd->inventory_data[index]->flag.slot; ++k)
						{	// --- Calculate hash for current card ---
							// Defense equipment
							// They *usually* have only 1 slot, so we just assign 1 bit
							int hash = 0;
							if (st.sd->inventory_data[index]->type == 5)
							{
								hash = st.sd->inventory_data[index]->equip;
							}
							// Weapons
							// right hand: slot 1 - 0x0010000 ... slot 4 - 0x0080000
							// left hand: slot 1 - 0x1000000 ... slot 4 - 0x8000000
							// We can support up to 8 slots each, just in case
							else if (st.sd->inventory_data[index]->type == 4)
							{
								if (st.sd->inventory_data[index]->equip & 2)	// right hand
									hash = 0x00010000 * (1<<k);	// pow(2,k) x slot number
								else if (st.sd->inventory_data[index]->equip & 32)	// left hand
									hash = 0x01000000 * (1<<k);	// pow(2,k) x slot number
							}
							else
								continue;	// slotted item not armour nor weapon? we're not going to support it
							if (st.sd->setitem_hash & hash)	// check if card is already used by another set
								continue;	// this item is used, move on to next card
							if( st.sd->status.inventory[index].card[0] != 0x00ff &&
								st.sd->status.inventory[index].card[0] != 0x00fe &&
								st.sd->status.inventory[index].card[0] != 0xff00 &&
								st.sd->status.inventory[index].card[k] == id )
							{	// We have found a match
								flag = 1;
								// Set hash so this card cannot be used by another
								st.sd->setitem_hash |= hash;
								break;
							}
						}
					}
					if (flag) break;
				}
			}
			if (ret == -1)
				ret = flag;
			else
				ret &= flag;
			if (!ret) break;
		}
	}
	st.push_val(CScriptEngine::C_INT,ret);
	return 0;
}

/*================================================
 * Check how many items/cards in the list are
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------
 */
int buildin_isequippedcnt(CScriptEngine &st)
{
	size_t i, j, k;
	uint32 id = 1;
	int ret = 0;
	int index, type;

	if(st.sd)
	{
		for (i=0; id!=0; ++i)
		{
			if( st.Arguments() > i+2 )
				id = st.GetInt(st[i+2]);
			else 
				id = 0;
			if (id <= 0)
				continue;
			for (j=0; j<10; ++j)
			{
				index = st.sd->equip_index[j];
				if(index >= MAX_INVENTORY) continue;
				if(j == 9 && st.sd->equip_index[8] == index) continue;
				if(j == 5 && st.sd->equip_index[4] == index) continue;
				if(j == 6 && (st.sd->equip_index[5] == index || st.sd->equip_index[4] == index)) continue;
				
				type = itemdb_type(id);
				
				if(st.sd->inventory_data[index])
				{
					if(type == 4 || type == 5)
					{
						if(st.sd->inventory_data[index]->nameid == id)
							ret++; //[Lupus]
					}
					else if (type == 6)
					{
						for(k=0; k<st.sd->inventory_data[index]->flag.slot; ++k)
						{
							if( st.sd->status.inventory[index].card[0]!=0x00ff &&
								st.sd->status.inventory[index].card[0]!=0x00fe &&
								st.sd->status.inventory[index].card[0]!=0xff00 &&
								st.sd->status.inventory[index].card[k] == id)
							{
								ret++; //[Lupus]
							}
						}
					}
				}
			}
		}
	}
	st.push_val(CScriptEngine::C_INT,ret);
	return 0;
}

/*================================================
 * Check how many given inserted cards in the CURRENT
 * weapon - used for 2/15's cards patch [Lupus]
 *------------------------------------------------
 */
int buildin_cardscnt(CScriptEngine &st)
{
	size_t i, k;
	unsigned short id = 1;
	int ret = 0;
	int index, type;

	if(st.sd)
	{
		for (i=0; id!=0; ++i)
		{
			if( st.Arguments() > (i+2) )
				id=st.GetInt(st[(i+2)]);
			else 
				id = 0;
			if (id <= 0)
				continue;
			
			index = current_equip_item_index; //we get CURRENT WEAPON inventory index from status.c [Lupus]
			if(index < 0) continue;
			
			type = itemdb_type(id);
			
			if(st.sd->inventory_data[index])
			{
				if (type == 4 || type == 5)
				{
					if (st.sd->inventory_data[index]->nameid == id)
						ret++;
				}
				else if (type == 6)
				{
					for(k=0; k<st.sd->inventory_data[index]->flag.slot; ++k)
					{
						if( st.sd->status.inventory[index].card[0]!=0x00ff &&
							st.sd->status.inventory[index].card[0]!=0x00fe &&
							st.sd->status.inventory[index].card[0]!=0xff00 &&
							st.sd->status.inventory[index].card[k] == id )
						{
							ret++;
						}
					}
				}
			}
		}
	}
	st.push_val(CScriptEngine::C_INT,ret);
	return 0;
}

/*=======================================================
 * Returns the refined number of the current item, or an
 * item with inventory index specified
 *-------------------------------------------------------
 */
int buildin_getrefine(CScriptEngine &st)
{
	if(st.sd)
		st.push_val( CScriptEngine::C_INT, st.sd->status.inventory[current_equip_item_index].refine);
	return 0;
}

/*=======================================================
 * Allows 2 Parents to adopt a character as a Baby
 *-------------------------------------------------------
 */
int buildin_adopt(CScriptEngine &st)
{
	int ret=0;
	
	map_session_data *p1_sd = map_session_data::nick2sd( st.GetString(st[2]) );
	map_session_data *p2_sd = map_session_data::nick2sd( st.GetString(st[3]) );
	map_session_data *c_sd  = map_session_data::nick2sd( st.GetString(st[4]) );

	if( p1_sd && p2_sd && c_sd &&
		p1_sd->status.base_level >= 70 &&
		p2_sd->status.base_level >= 70 )
		ret = pc_adoption(*p1_sd, *p2_sd, *c_sd);

	st.push_val( CScriptEngine::C_INT, ret);
	return 0;
}

/*=======================================================
 * Day/Night controls
 *-------------------------------------------------------
 */
int buildin_night(CScriptEngine &st)
{
	if(!daynight_flag)
		map_daynight_timer(-1, 0, 0, 1);
	return 0;
}
int buildin_day(CScriptEngine &st)
{
	if(daynight_flag)
		map_daynight_timer(-1, 0, 0, 0);
	return 0;
}

//=======================================================
// Unequip [Spectre]
//-------------------------------------------------------
int buildin_unequip(CScriptEngine &st)
{
	int i;
	size_t num = st.GetInt(st[2]) - 1;
	if(st.sd && num<(sizeof(equip)/sizeof(equip[0])))
	{
		i=pc_checkequip(*st.sd, equip[num]);
		pc_unequipitem(*st.sd,i,2);
	}
	return 0;
}

//=======================================================
// strlen [Valaris]
//-------------------------------------------------------
int buildin_getstrlen(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	int len = (str) ? (int)strlen(str) : 0;
	st.push_val(CScriptEngine::C_INT,len);
	return 0;
}

//=======================================================
// isalpha [Valaris]
//-------------------------------------------------------
int buildin_charisalpha(CScriptEngine &st)
{
	const char *str=st.GetString(st[2]);
	size_t pos =st.GetInt(st[3]);
	int val = ( str && pos>0 && pos<strlen(str) ) ? isalpha( (int)((unsigned char)str[pos]) ) : 0;
	st.push_val(CScriptEngine::C_INT, val);
	return 0;
}




int buildin_compare(CScriptEngine &st)                                 
{
	char buf1[1024],buf2[1024];

	const char *message = st.GetString(st[2]);
	const char *cmpstring = st.GetString(st[3]);
	
	strcpytolower(buf1, 1024, message);
	strcpytolower(buf2, 1024, cmpstring);
	
	st.push_val(CScriptEngine::C_INT,(strstr(buf1,buf2) != NULL));
	return 0;
}



/*==========================================
 * Warpparty - [Fredzilla]
 * Syntax: warpparty "mapname.gat",x,y,Party_ID;
 *------------------------------------------
 */
int buildin_warpparty(CScriptEngine &st)
{

	const char *str =st.GetString(st[2]);
	int x			=st.GetInt(st[3]);
	int y			=st.GetInt(st[4]);
	uint32 p	=st.GetInt(st[5]);
	map_session_data *sd=st.sd;

	if(!sd || maps[sd->block_list::m].flag.noreturn || maps[sd->block_list::m].flag.nowarp || NULL==party_search(p))
		return 0;
	
	if(p!=0)
	{
		size_t i;
		map_session_data *pl_sd;

		if( 0==strcasecmp(str,"Random") )
		{
			for(i=0; i<fd_max; ++i)
			{
				if(session[i] && (pl_sd = (map_session_data *)session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.party_id == p)
				{
					if(!maps[pl_sd->block_list::m].flag.nowarp)
						pc_randomwarp(*pl_sd,3);
				}
			}
		}
		else if( 0==strcasecmp(str,"SavePointAll") )
		{
			for(i=0; i<fd_max; ++i)
			{
				if(session[i] && (pl_sd = (map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.party_id == p)
				{
					if(!maps[pl_sd->block_list::m].flag.noreturn)
						pc_setpos(*pl_sd,pl_sd->status.save_point.mapname,pl_sd->status.save_point.x,pl_sd->status.save_point.y,3);
				}
			}
		}
		else if( 0==strcasecmp(str,"SavePoint") )
		{
			str=sd->status.save_point.mapname;
			x=sd->status.save_point.x;
			y=sd->status.save_point.y;
			for (i = 0; i < fd_max; ++i)
			{
				if(session[i] && (pl_sd = (map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.party_id == p)
				{
					if(!maps[pl_sd->block_list::m].flag.noreturn)
						pc_setpos(*pl_sd,str,x,y,3);
				}
			}
		}
		else
		{
			for (i = 0; i < fd_max; ++i)
			{
				if(session[i] && (pl_sd = (map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.party_id == p)
				{
					if(!maps[pl_sd->block_list::m].flag.noreturn)
						pc_setpos(*pl_sd,str,x,y,3);
				}
			}
		}
	}
	return 0;
}
/*==========================================
 * Warpguild - [Fredzilla]
 * Syntax: warpguild "mapname.gat",x,y,Guild_ID;
 *------------------------------------------
 */
int buildin_warpguild(CScriptEngine &st)
{
	const char *str =st.GetString(st[2]);
	int x			=st.GetInt(st[3]);
	int y			=st.GetInt(st[4]);
	uint32 g	=st.GetInt(st[5]);
	map_session_data *sd=st.sd;

	if(!sd || maps[sd->block_list::m].flag.noreturn || maps[sd->block_list::m].flag.nowarp || NULL==guild_search(g) )
		return 0;
	
	if(g!=0)
	{
		size_t i;
		map_session_data *pl_sd;

		if( 0==strcasecmp(str,"Random") )
		{
			for(i=0; i<fd_max; ++i)
			{
				if (session[i] && (pl_sd = (map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.guild_id == g)
				{
					if(!maps[pl_sd->block_list::m].flag.nowarp)
						pc_randomwarp(*pl_sd,3);
				}
			}
		}
		else if( 0==strcasecmp(str,"SavePointAll") )
		{
			for(i=0; i < fd_max; ++i)
			{
				if (session[i] && (pl_sd = (map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.guild_id == g)
				{
					if(!maps[pl_sd->block_list::m].flag.noreturn)
						pc_setpos(*pl_sd,pl_sd->status.save_point.mapname,pl_sd->status.save_point.x,pl_sd->status.save_point.y,3);
				}
			}
		}
		else if( 0==strcasecmp(str,"SavePoint")==0 )
		{
			str=sd->status.save_point.mapname;
			x=sd->status.save_point.x;
			y=sd->status.save_point.y;
			for(i=0; i<fd_max; ++i)
			{
				if(session[i] && (pl_sd = (map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.guild_id == g)
				{
					if(maps[pl_sd->block_list::m].flag.noreturn)
						pc_setpos(*pl_sd,str,x,y,3);
				}
			}
		}
		else
		{
			for(i=0; i<fd_max; ++i)
			{
				if(session[i] && (pl_sd = (map_session_data *) session[i]->user_session) && pl_sd->state.auth &&
					pl_sd->status.guild_id == g)
				{
					if(maps[pl_sd->block_list::m].flag.noreturn)
						pc_setpos(*pl_sd,str,x,y,3);
				}
			}
		}
	}
	return 0;
}
/*==========================================
 *	Shows an emoticon on top of the player.
 *------------------------------------------
 */
// Added by request as official npcs seem to use it. [Skotlex]
int buildin_pc_emotion(CScriptEngine &st)
{
	if(st.sd)
	{
		int type=st.GetInt(st[2]);
		if(type >= 0 && type <= 100)
			clif_emotion(*st.sd,type);
	}
	return 0;
}
/*==========================================
 * Returns some values of an item [Lupus]
 * Price, Weight, etc...
	iteminfo(itemID,n), where n
		0 value_buy;
		1 value_sell;
		2 type;
		3 class;
		4 sex;
		5 equip;
		6 weight;
		7 atk;
		8 def;
		9 range;
		10 slot;
		11 look;
		12 elv;
		13 wlv;
 *------------------------------------------
 */
int buildin_getiteminfo(CScriptEngine &st)
{
	int item_id	= st.GetInt(st[2]);
	size_t n	= st.GetInt(st[3]);
	struct item_data *data = itemdb_exists(item_id);
	int ret = -1;
	if(data) switch(n)
	{
	case  0: ret = data->value_buy; break;
	case  1: ret = data->value_sell; break;
	case  2: ret = data->type; break;
	case  3: ret = data->class_array; break;
	case  4: ret = data->flag.sex; break;
	case  5: ret = data->equip; break;
	case  6: ret = data->weight; break;
	case  7: ret = data->atk; break;
	case  8: ret = data->def; break;
	case  9: ret = data->range; break;
	case 10: ret = data->flag.slot; break;
	case 11: ret = data->look; break;
	case 12: ret = data->elv; break;
	case 13: ret = data->wlv; break;
	}
	st.push_val(CScriptEngine::C_INT,ret);

	return 0;
}

/*==========================================
 *	
 *------------------------------------------
 */
int buildin_callshop(CScriptEngine &st)
{
	int ret = 0;
	if(st.sd)
	{
		const char *shopname = st.GetString(st[2]);
		int flag = 0;
		if( st.Arguments() > (3) )
			flag = st.GetInt(st[3]);
		npcshop_data *sh = npcshop_data::from_name(shopname);
		if( sh)
		{
			switch (flag)
			{
			case 1: //Buy window
				sh->buywindow(*st.sd);
				break;
			case 2: //Sell window
				sh->sellwindow(*st.sd);
				break;
			default: //Show menu
				sh->OnClick(*st.sd);
				break;
			}
			st.sd->npc_shopid = sh->block_list::id;
			ret = 1;
		}
	}
	st.push_val(CScriptEngine::C_INT,ret);
	return 0;
}

/// regular expression.
/// returns true/false on match success, 
/// capture patterns are stored in variables $p0$..$p9$ as done in npc_listen
int buildin_regex(CScriptEngine &st)
{
	const char *regexpr = st.GetString(st[2]);
	const char *pattern = st.GetString(st[3]);
	basics::CRegExp rx(regexpr);
	int ret = rx.match(pattern);

	size_t i;
	char buffer[16];
	for(i=0; i<9; ++i)
	{
		snprintf(buffer, sizeof(buffer), "$p%i$", (int)i);
		set_var(buffer, (void*)(( i<=rx.sub_count() ) ? ((const char*)rx[i]) : "") );
	}

	st.push_val(CScriptEngine::C_INT,ret);
	return 0;
}


/*==========================================
 * マップ変数の変更
 *------------------------------------------
 */
int mapreg_setregnum(int num, int val)
{
	if(val!=0)
		numdb_insert(mapreg_db,num,val);
	else
		numdb_erase(mapreg_db,num);

	mapreg_dirty=1;
	return 0;
}
/*==========================================
 * 文字列型マップ変数の変更
 *------------------------------------------
 */
int mapreg_setregstr(int num, const char *str)
{
	char *p = (char *) numdb_search(mapregstr_db, num);
	if( p!=NULL )
		delete[] p;

	if( str==NULL || *str==0 )
	{
		numdb_erase(mapregstr_db,num);
	}
	else
	{
		p= new char[strlen(str)+1];
		memcpy(p,str,(strlen(str)+1)*sizeof(char));
		numdb_insert(mapregstr_db,num,p);
	}
	mapreg_dirty=1;
	return 0;
}

/*==========================================
 * 永続的マップ変数の読み込み
 *------------------------------------------
 */
int script_load_mapreg()
{
	FILE *fp;
	char line[1024];

	if( (fp=basics::safefopen(mapreg_txt,"rt"))==NULL )
		return -1;

	while(fgets(line,sizeof(line),fp)){
		char buf1[256],buf2[1024],*p;
		int n,v,s,i;
		if( sscanf(line,"%256[^,],%d\t%n",buf1,&i,&n)!=2 &&
			(i=0,sscanf(line,"%256[^\t]\t%n",buf1,&n)!=1) )
			continue;
		basics::itrim(buf1);
		if( buf1[strlen(buf1)-1]=='$' )
		{
			if( sscanf(line+n,"%1024[^\n\r]",buf2)!=1 )
			{
				ShowMessage("%s: %s broken data !\n",mapreg_txt,buf1);
				continue;
			}
			p= new char[(strlen(buf2) + 1)];
			strcpy(p,buf2);
			s= add_str( buf1);
			numdb_insert(mapregstr_db,(i<<24)|s,p);
		}
		else
		{
			if( sscanf(line+n,"%d",&v)!=1 )
			{
				ShowMessage("%s: %s broken data !\n",mapreg_txt,buf1);
				continue;
			}
			s= add_str( buf1);
			numdb_insert(mapreg_db,(i<<24)|s,v);
		}
	}
	fclose(fp);
	mapreg_dirty=0;
	return 0;
}
/*==========================================
 * 永続的マップ変数の書き込み
 *------------------------------------------
 */
int script_save_mapreg()
{
	FILE *fp;
	int lock;
	if( (fp=lock_fopen(mapreg_txt, lock))==NULL )
		return -1;

	db_iterator<size_t, int> iteri(mapreg_db);
	for(; iteri; ++iteri)
	{
		int num=iteri.key()&0x00FFFFFF;
		int i  =(iteri.key()>>24)&0xFF;
		const char *name=str_data[num].string();
		if( name[1]!='@' )
		{
			if(i==0)
				fprintf(fp,"%s\t%d\n", name, iteri.data());
			else
				fprintf(fp,"%s,%d\t%d\n", name, i, iteri.data());
		}
	}

	db_iterator<size_t, const char*> iters(mapregstr_db);
	for(; iters; ++iters)
	{
		int num=iters.key()&0x00FFFFFF;
		int i  =(iters.key()>>24)&0xFF;
		const char *name=str_data[num].string();
		if( name[1]!='@' )
		{
			if(i==0)
				fprintf(fp,"%s\t%s\n", name, iters.data());
			else
				fprintf(fp,"%s,%d\t%s\n", name, i, iters.data());
		}
	}

	lock_fclose(fp,mapreg_txt, lock);
	mapreg_dirty=0;
	return 0;
}


int script_autosave_mapreg(int tid, unsigned long tick, int id, basics::numptr data)
{
	if(mapreg_dirty)
		script_save_mapreg();
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int set_posword(const char *p)
{
	const char* np;
	int i;
	if(p)
	for(i=0;i<MAX_EQUIP;++i)
	{
		if((np=strchr(p,','))!=NULL)
		{	// copy up to the comma
			memcpy(positions[i],p,np-p);
			positions[i][np-p]=0; // set the eos explicitly
			p=np+1;
		}
		else
		{	//copy the rest including the eos
			memcpy(positions[i],p,1+strlen(p));
			p+=strlen(p);
		}
	}
	return 0;
}

int script_config_read(const char *cfgName)
{
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	script_config.verbose_mode = 0;
	script_config.warn_func_no_comma = 1;
	script_config.warn_cmd_no_comma = 1;
	script_config.warn_func_mismatch_paramnum = 1;
	script_config.warn_cmd_mismatch_paramnum = 1;
	script_config.check_cmdcount=16384;
	script_config.check_gotocount=1024;


	script_config.event_requires_trigger = 1;

	fp=basics::safefopen(cfgName,"r");
	if (fp == NULL) {
		ShowError("file not found: %s\n",cfgName);
		return 1;
	}
	while (fgets(line, sizeof(line), fp))
	{
		if( prepare_line(line) && 2==sscanf(line,"%1024[^:=]%*[:=]%1024[^\r\n]",w1,w2) )
		{
			basics::itrim(w1);
			if(!*w1) continue;
			basics::itrim(w2);

			if(strcasecmp(w1,"refine_posword")==0)
			{
				set_posword(w2);
			}
			else if(strcasecmp(w1,"verbose_mode")==0)
			{
				script_config.verbose_mode = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1,"warn_func_no_comma")==0)
			{
				script_config.warn_func_no_comma = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1,"warn_cmd_no_comma")==0)
			{
				script_config.warn_cmd_no_comma = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1,"warn_func_mismatch_paramnum")==0)
			{
				script_config.warn_func_mismatch_paramnum = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"warn_cmd_mismatch_paramnum")==0)
			{
				script_config.warn_cmd_mismatch_paramnum = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"check_cmdcount")==0)
			{
				script_config.check_cmdcount = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"check_gotocount")==0)
			{
				script_config.check_gotocount = basics::config_switch<int>(w2);
			}
			else if(strcasecmp(w1,"die_event_name")==0)
			{			
				safestrcpy(script_config.die_event_name, sizeof(script_config.die_event_name), w2);
			}
			else if(strcasecmp(w1,"kill_event_name")==0)
			{
				safestrcpy(script_config.kill_event_name, sizeof(script_config.kill_event_name), w2);
			}
			else if(strcasecmp(w1,"login_event_name")==0)
			{
				safestrcpy(script_config.login_event_name, sizeof(script_config.login_event_name), w2);
			}
			else if(strcasecmp(w1,"logout_event_name")==0)
			{
				safestrcpy(script_config.logout_event_name, sizeof(script_config.logout_event_name), w2);
			}
			else if(strcasecmp(w1,"mapload_event_name")==0)
			{
				safestrcpy(script_config.mapload_event_name, sizeof(script_config.mapload_event_name), w2);
			}
			else if(strcasecmp(w1,"event_requires_trigger")==0)
			{
				script_config.event_requires_trigger = basics::config_switch<bool>(w2);
			}
			else if(strcasecmp(w1,"import")==0)
			{
				script_config_read(w2);
			}
			else
			{
				ShowWarning("unknown option '%s' in '%s', ignored\n", w1, cfgName);
			}
		}
	}
	fclose(fp);

	return 0;
}

/*==========================================
 * 終了
 *------------------------------------------
 */
void mapregstr_db_final(void *key,void *data)
{

	delete[] ((char*)data);
}
void userfunc_db_final(void *key,void *data)
{
	delete[] ((char*)key);
	((script_object*)data)->release();
}
int do_final_script()
{
	if(mapreg_dirty>=0)
		script_save_mapreg();

	if(mapreg_db)
	{
		numdb_final(mapreg_db);
		mapreg_db=NULL;
	}
	if(mapregstr_db)
	{
		strdb_final(mapregstr_db,mapregstr_db_final);
		mapregstr_db=NULL;
	}
	if(userfunc_db)
	{
		strdb_final(userfunc_db,userfunc_db_final);
		userfunc_db=NULL;
	}

	if(str_data)
	{
		delete[] str_data;
		str_data=NULL;
	}

	return 0;
}
/*==========================================
 * 初期化
 *------------------------------------------
 */
int do_init_script()
{
	memset(&script_config, 0, sizeof(script_config));

	safestrcpy(script_config.die_event_name,sizeof(script_config.die_event_name)        ,"PCDieEvent");
	safestrcpy(script_config.kill_event_name,sizeof(script_config.kill_event_name)      ,"PCKillEvent");
	safestrcpy(script_config.login_event_name,sizeof(script_config.login_event_name)    ,"PCLoginEvent");
	safestrcpy(script_config.logout_event_name,sizeof(script_config.logout_event_name)  ,"PCLogoutEvent");
	safestrcpy(script_config.mapload_event_name,sizeof(script_config.mapload_event_name),"PCLoadMapEvent");
	script_config.verbose_mode = 0;
	script_config.warn_func_no_comma = 1;
	script_config.warn_cmd_no_comma = 1;
	script_config.warn_func_mismatch_paramnum = 1;
	script_config.warn_cmd_mismatch_paramnum = 1;
	script_config.event_requires_trigger = 1;
	script_config.check_cmdcount = 0;
	script_config.check_gotocount = 0;


	mapreg_db=numdb_init();
	mapregstr_db=numdb_init();
	script_load_mapreg();

	add_timer_func_list(script_autosave_mapreg,"script_autosave_mapreg");
	add_timer_interval(gettick()+MAPREG_AUTOSAVE_INTERVAL,MAPREG_AUTOSAVE_INTERVAL,script_autosave_mapreg,0,0);

	return 0;
}
