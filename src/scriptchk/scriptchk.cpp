// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "basesafeptr.h"
#include "basetime.h"
#include "baseparser.h"
#include "basefile.h"

#include "eacompiler.h"


#include "baseparam.h"

///////////////////////////////////////////////////////////////////////////////
// basic class for using the old way timers
///////////////////////////////////////////////////////////////////////////////
bool basics::CTimerBase::init(unsigned long interval)
{
	return false;
}

// external calling from external timer implementation
int basics::CTimerBase::timercallback(int timer, unsigned long tick, int id, basics::numptr data)
{
	return 0;
}
void basics::CTimerBase::timerfinalize()
{

}



///////////////////////////////////////////////////////////////////////////////////////
// temporary placeholder for buildin function support
///////////////////////////////////////////////////////////////////////////////////////
class CScriptEngine
{
};

int buildin_mes(CScriptEngine &st) { return 0; }
int buildin_goto(CScriptEngine &st) { return 0; }
int buildin_callsub(CScriptEngine &st) { return 0; }
int buildin_callfunc(CScriptEngine &st) { return 0; }
int buildin_return(CScriptEngine &st) { return 0; }
int buildin_getarg(CScriptEngine &st) { return 0; }
int buildin_next(CScriptEngine &st) { return 0; }
int buildin_close(CScriptEngine &st) { return 0; }
int buildin_close2(CScriptEngine &st) { return 0; }
int buildin_menu(CScriptEngine &st) { return 0; }
int buildin_rand(CScriptEngine &st) { return 0; }
int buildin_warp(CScriptEngine &st) { return 0; }
int buildin_areawarp(CScriptEngine &st) { return 0; }
int buildin_heal(CScriptEngine &st) { return 0; }
int buildin_itemheal(CScriptEngine &st) { return 0; }
int buildin_percentheal(CScriptEngine &st) { return 0; }
int buildin_jobchange(CScriptEngine &st) { return 0; }
int buildin_input(CScriptEngine &st) { return 0; }
int buildin_setlook(CScriptEngine &st) { return 0; }
int buildin_set(CScriptEngine &st) { return 0; }
int buildin_setarray(CScriptEngine &st) { return 0; }
int buildin_cleararray(CScriptEngine &st) { return 0; }
int buildin_copyarray(CScriptEngine &st) { return 0; }
int buildin_getarraysize(CScriptEngine &st) { return 0; }
int buildin_deletearray(CScriptEngine &st) { return 0; }
int buildin_getelementofarray(CScriptEngine &st) { return 0; }
int buildin_if(CScriptEngine &st) { return 0; }
int buildin_getitem(CScriptEngine &st) { return 0; }
int buildin_getitem2(CScriptEngine &st) { return 0; }
int buildin_makeitem(CScriptEngine &st) { return 0; }
int buildin_delitem(CScriptEngine &st) { return 0; }
int buildin_viewpoint(CScriptEngine &st) { return 0; }
int buildin_countitem(CScriptEngine &st) { return 0; }
int buildin_checkweight(CScriptEngine &st) { return 0; }
int buildin_readparam(CScriptEngine &st) { return 0; }
int buildin_getcharid(CScriptEngine &st) { return 0; }
int buildin_getpartyname(CScriptEngine &st) { return 0; }
int buildin_getpartymember(CScriptEngine &st) { return 0; }
int buildin_getguildname(CScriptEngine &st) { return 0; }
int buildin_getguildmaster(CScriptEngine &st) { return 0; }
int buildin_getguildmasterid(CScriptEngine &st) { return 0; }
int buildin_strcharinfo(CScriptEngine &st) { return 0; }
int buildin_getequipid(CScriptEngine &st) { return 0; }
int buildin_getequipname(CScriptEngine &st) { return 0; }
int buildin_getbrokenid(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_repair(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_getequipisequiped(CScriptEngine &st) { return 0; }
int buildin_getequipisenableref(CScriptEngine &st) { return 0; }
int buildin_getequipisidentify(CScriptEngine &st) { return 0; }
int buildin_getequiprefinerycnt(CScriptEngine &st) { return 0; }
int buildin_getequipweaponlv(CScriptEngine &st) { return 0; }
int buildin_getequippercentrefinery(CScriptEngine &st) { return 0; }
int buildin_successrefitem(CScriptEngine &st) { return 0; }
int buildin_failedrefitem(CScriptEngine &st) { return 0; }
int buildin_cutin(CScriptEngine &st) { return 0; }
int buildin_cutincard(CScriptEngine &st) { return 0; }
int buildin_statusup(CScriptEngine &st) { return 0; }
int buildin_statusup2(CScriptEngine &st) { return 0; }
int buildin_bonus(CScriptEngine &st) { return 0; }
int buildin_bonus2(CScriptEngine &st) { return 0; }
int buildin_bonus3(CScriptEngine &st) { return 0; }
int buildin_bonus4(CScriptEngine &st) { return 0; }
int buildin_skill(CScriptEngine &st) { return 0; }
int buildin_addtoskill(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_guildskill(CScriptEngine &st) { return 0; }
int buildin_getskilllv(CScriptEngine &st) { return 0; }
int buildin_getgdskilllv(CScriptEngine &st) { return 0; }
int buildin_basicskillcheck(CScriptEngine &st) { return 0; }
int buildin_getgmlevel(CScriptEngine &st) { return 0; }
int buildin_end(CScriptEngine &st) { return 0; }
int buildin_checkoption(CScriptEngine &st) { return 0; }
int buildin_setoption(CScriptEngine &st) { return 0; }
int buildin_setcart(CScriptEngine &st) { return 0; }
int buildin_checkcart(CScriptEngine &st) { return 0; } // check cart [Valaris]
int buildin_setfalcon(CScriptEngine &st) { return 0; }
int buildin_checkfalcon(CScriptEngine &st) { return 0; } // check falcon [Valaris]
int buildin_setriding(CScriptEngine &st) { return 0; }
int buildin_checkriding(CScriptEngine &st) { return 0; } // check for pecopeco [Valaris]
int buildin_savepoint(CScriptEngine &st) { return 0; }
int buildin_gettimetick(CScriptEngine &st) { return 0; }
int buildin_gettime(CScriptEngine &st) { return 0; }
int buildin_gettimestr(CScriptEngine &st) { return 0; }
int buildin_openstorage(CScriptEngine &st) { return 0; }
int buildin_guildopenstorage(CScriptEngine &st) { return 0; }
int buildin_itemskill(CScriptEngine &st) { return 0; }
int buildin_produce(CScriptEngine &st) { return 0; }
int buildin_monster(CScriptEngine &st) { return 0; }
int buildin_areamonster(CScriptEngine &st) { return 0; }
int buildin_killmonster(CScriptEngine &st) { return 0; }
int buildin_killmonsterall(CScriptEngine &st) { return 0; }
int buildin_doevent(CScriptEngine &st) { return 0; }
int buildin_donpcevent(CScriptEngine &st) { return 0; }
int buildin_addtimer(CScriptEngine &st) { return 0; }
int buildin_deltimer(CScriptEngine &st) { return 0; }
int buildin_addtimercount(CScriptEngine &st) { return 0; }
int buildin_initnpctimer(CScriptEngine &st) { return 0; }
int buildin_stopnpctimer(CScriptEngine &st) { return 0; }
int buildin_startnpctimer(CScriptEngine &st) { return 0; }
int buildin_setnpctimer(CScriptEngine &st) { return 0; }
int buildin_getnpctimer(CScriptEngine &st) { return 0; }
int buildin_attachnpctimer(CScriptEngine &st) { return 0; }	// [celest]
int buildin_detachnpctimer(CScriptEngine &st) { return 0; }	// [celest]
int buildin_announce(CScriptEngine &st) { return 0; }
int buildin_mapannounce(CScriptEngine &st) { return 0; }
int buildin_areaannounce(CScriptEngine &st) { return 0; }
int buildin_getusers(CScriptEngine &st) { return 0; }
int buildin_getmapusers(CScriptEngine &st) { return 0; }
int buildin_getareausers(CScriptEngine &st) { return 0; }
int buildin_getareadropitem(CScriptEngine &st) { return 0; }
int buildin_enablenpc(CScriptEngine &st) { return 0; }
int buildin_disablenpc(CScriptEngine &st) { return 0; }
int buildin_enablearena(CScriptEngine &st) { return 0; }	// Added by RoVeRT
int buildin_disablearena(CScriptEngine &st) { return 0; }	// Added by RoVeRT
int buildin_hideoffnpc(CScriptEngine &st) { return 0; }
int buildin_hideonnpc(CScriptEngine &st) { return 0; }
int buildin_sc_start(CScriptEngine &st) { return 0; }
int buildin_sc_start2(CScriptEngine &st) { return 0; }
int buildin_sc_start4(CScriptEngine &st) { return 0; }
int buildin_sc_end(CScriptEngine &st) { return 0; }
int buildin_getscrate(CScriptEngine &st) { return 0; }
int buildin_debugmes(CScriptEngine &st) { return 0; }
int buildin_catchpet(CScriptEngine &st) { return 0; }
int buildin_birthpet(CScriptEngine &st) { return 0; }
int buildin_resetlvl(CScriptEngine &st) { return 0; }
int buildin_resetstatus(CScriptEngine &st) { return 0; }
int buildin_resetskill(CScriptEngine &st) { return 0; }
int buildin_changebase(CScriptEngine &st) { return 0; }
int buildin_changesex(CScriptEngine &st) { return 0; }
int buildin_waitingroom(CScriptEngine &st) { return 0; }
int buildin_delwaitingroom(CScriptEngine &st) { return 0; }
int buildin_enablewaitingroomevent(CScriptEngine &st) { return 0; }
int buildin_disablewaitingroomevent(CScriptEngine &st) { return 0; }
int buildin_getwaitingroomstate(CScriptEngine &st) { return 0; }
int buildin_warpwaitingpc(CScriptEngine &st) { return 0; }
int buildin_attachrid(CScriptEngine &st) { return 0; }
int buildin_detachrid(CScriptEngine &st) { return 0; }
int buildin_isloggedin(CScriptEngine &st) { return 0; }
int buildin_setmapflagnosave(CScriptEngine &st) { return 0; }
int buildin_setmapflag(CScriptEngine &st) { return 0; }
int buildin_removemapflag(CScriptEngine &st) { return 0; }
int buildin_pvpon(CScriptEngine &st) { return 0; }
int buildin_pvpoff(CScriptEngine &st) { return 0; }
int buildin_gvgon(CScriptEngine &st) { return 0; }
int buildin_gvgoff(CScriptEngine &st) { return 0; }
int buildin_emotion(CScriptEngine &st) { return 0; }
int buildin_maprespawnguildid(CScriptEngine &st) { return 0; }
int buildin_agitstart(CScriptEngine &st) { return 0; }		// <Agit>
int buildin_agitend(CScriptEngine &st) { return 0; }
int buildin_agitcheck(CScriptEngine &st) { return 0; }  // <Agitcheck>
int buildin_flagemblem(CScriptEngine &st) { return 0; }		// Flag Emblem
int buildin_getcastlename(CScriptEngine &st) { return 0; }
int buildin_getcastledata(CScriptEngine &st) { return 0; }
int buildin_setcastledata(CScriptEngine &st) { return 0; }
int buildin_requestguildinfo(CScriptEngine &st) { return 0; }
int buildin_getequipcardcnt(CScriptEngine &st) { return 0; }
int buildin_successremovecards(CScriptEngine &st) { return 0; }
int buildin_failedremovecards(CScriptEngine &st) { return 0; }
int buildin_marriage(CScriptEngine &st) { return 0; }
int buildin_wedding_effect(CScriptEngine &st) { return 0; }
int buildin_divorce(CScriptEngine &st) { return 0; }
int buildin_ispartneron(CScriptEngine &st) { return 0; } // MouseJstr
int buildin_getpartnerid(CScriptEngine &st) { return 0; } // MouseJstr
int buildin_getchildid(CScriptEngine &st) { return 0; } // Skotlex
int buildin_warppartner(CScriptEngine &st) { return 0; } // MouseJstr
int buildin_getitemname(CScriptEngine &st) { return 0; }
int buildin_makepet(CScriptEngine &st) { return 0; }
int buildin_getexp(CScriptEngine &st) { return 0; }
int buildin_getinventorylist(CScriptEngine &st) { return 0; }
int buildin_getskilllist(CScriptEngine &st) { return 0; }
int buildin_clearitem(CScriptEngine &st) { return 0; }
int buildin_classchange(CScriptEngine &st) { return 0; }
int buildin_misceffect(CScriptEngine &st) { return 0; }
int buildin_soundeffect(CScriptEngine &st) { return 0; }
int buildin_soundeffectall(CScriptEngine &st) { return 0; }
int buildin_mapwarp(CScriptEngine &st) { return 0; }
int buildin_inittimer(CScriptEngine &st) { return 0; }
int buildin_stoptimer(CScriptEngine &st) { return 0; }
int buildin_cmdothernpc(CScriptEngine &st) { return 0; }
int buildin_mobcount(CScriptEngine &st) { return 0; }
int buildin_strmobinfo(CScriptEngine &st) { return 0; } // Script for displaying mob info [Valaris]
int buildin_guardian(CScriptEngine &st) { return 0; } // Script for displaying mob info [Valaris]
int buildin_guardianinfo(CScriptEngine &st) { return 0; } // Script for displaying mob info [Valaris]
int buildin_petskillbonus(CScriptEngine &st) { return 0; } // petskillbonus [Valaris]
int buildin_petrecovery(CScriptEngine &st) { return 0; } // pet skill for curing status [Valaris]
int buildin_petloot(CScriptEngine &st) { return 0; } // pet looting [Valaris]
int buildin_petheal(CScriptEngine &st) { return 0; } // pet healing [Valaris]
int buildin_petskillattack(CScriptEngine &st) { return 0; } // pet skill attacks [Skotlex]
int buildin_petskillattack2(CScriptEngine &st) { return 0; } // pet skill attacks [Skotlex]
int buildin_petskillsupport(CScriptEngine &st) { return 0; } // pet support skill [Valaris]
int buildin_skilleffect(CScriptEngine &st) { return 0; } // skill effects [Celest]
int buildin_npcskilleffect(CScriptEngine &st) { return 0; } // skill effects for npcs [Valaris]
int buildin_specialeffect(CScriptEngine &st) { return 0; } // special effect script [Valaris]
int buildin_specialeffect2(CScriptEngine &st) { return 0; } // special effect script [Valaris]
int buildin_nude(CScriptEngine &st) { return 0; } // nude [Valaris]
int buildin_gmcommand(CScriptEngine &st) { return 0; } // [MouseJstr]
int buildin_movenpc(CScriptEngine &st) { return 0; } // [MouseJstr]
int buildin_message(CScriptEngine &st) { return 0; } // [MouseJstr]
int buildin_npctalk(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_hasitems(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_getlook(CScriptEngine &st) { return 0; }	//Lorky [Lupus]
int buildin_getsavepoint(CScriptEngine &st) { return 0; }	//Lorky [Lupus]
int buildin_npcspeed(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_npcwalkto(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_npcstop(CScriptEngine &st) { return 0; } // [Valaris]
int buildin_getmapxy(CScriptEngine &st) { return 0; }  //get map position for player/npc/pet/mob by Lorky [Lupus]
int buildin_checkoption1(CScriptEngine &st) { return 0; } // [celest]
int buildin_checkoption2(CScriptEngine &st) { return 0; } // [celest]
int buildin_guildgetexp(CScriptEngine &st) { return 0; } // [celest]
int buildin_skilluseid(CScriptEngine &st) { return 0; } // originally by Qamera [celest]
int buildin_skillusepos(CScriptEngine &st) { return 0; } // originally by Qamera [celest]
int buildin_logmes(CScriptEngine &st) { return 0; } // [Lupus]
int buildin_summon(CScriptEngine &st) { return 0; } // [celest]
int buildin_isnight(CScriptEngine &st) { return 0; } // [celest]
int buildin_isday(CScriptEngine &st) { return 0; } // [celest]
int buildin_isequipped(CScriptEngine &st) { return 0; } // [celest]
int buildin_isequippedcnt(CScriptEngine &st) { return 0; } // [celest]
int buildin_cardscnt(CScriptEngine &st) { return 0; } // [Lupus]
int buildin_getrefine(CScriptEngine &st) { return 0; } // [celest]
int buildin_adopt(CScriptEngine &st) { return 0; }
int buildin_night(CScriptEngine &st) { return 0; }
int buildin_day(CScriptEngine &st) { return 0; }
int buildin_getusersname(CScriptEngine &st) { return 0; } //jA commands added [Lupus]
int buildin_dispbottom(CScriptEngine &st) { return 0; }
int buildin_recovery(CScriptEngine &st) { return 0; }
int buildin_getpetinfo(CScriptEngine &st) { return 0; }
int buildin_checkequipedcard(CScriptEngine &st) { return 0; }
int buildin_globalmes(CScriptEngine &st) { return 0; }
int buildin_jump_zero(CScriptEngine &st) { return 0; }
int buildin_select(CScriptEngine &st) { return 0; }
int buildin_getmapmobs(CScriptEngine &st) { return 0; } //jA addition end
int buildin_getstrlen(CScriptEngine &st) { return 0; } //strlen [valaris]
int buildin_charisalpha(CScriptEngine &st) { return 0; }//isalpha [valaris]
int buildin_fakenpcname(CScriptEngine &st) { return 0; } // [Lance]

int buildin_defpattern(CScriptEngine &st) { return 0; } // MouseJstr
int buildin_activatepset(CScriptEngine &st) { return 0; } // MouseJstr
int buildin_deactivatepset(CScriptEngine &st) { return 0; } // MouseJstr
int buildin_deletepset(CScriptEngine &st) { return 0; } // MouseJstr

int buildin_unequip(CScriptEngine &st) { return 0; } // unequip [Spectre]

int buildin_pcstrcharinfo(CScriptEngine &st) { return 0; }
int buildin_getnameditem(CScriptEngine &st) { return 0; }
int buildin_compare(CScriptEngine &st) { return 0; }
int buildin_warpparty(CScriptEngine &st) { return 0; }
int buildin_warpguild(CScriptEngine &st) { return 0; }
int buildin_pc_emotion(CScriptEngine &st) { return 0; }
int buildin_getiteminfo(CScriptEngine &st) { return 0; }


struct _buildin_func{
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

	// array terminator
	{NULL,NULL,NULL},
};


///////////////////////////////////////////////////////////////////////////////////////
// code beautifier
///////////////////////////////////////////////////////////////////////////////////////
//FILE *output=NULL;
bool output=true;
void printoutput(const char* str, int scope, bool &newline, bool &limiter)
{
	if(str)
	{
		if(newline)
		{
			int i;
			for(i=0; i<scope; ++i)
				printf("\t");
			newline=false;
		}
		else if(limiter && isalnum(str[0]))
		{
			printf(" ");
		}
		printf(str);
		limiter = 0!=isalnum( str[strlen(str)-1] );
	}
}
void print_comments(CParser_CommentStore& parser, int &scope, bool &newline, bool &limiter, size_t linelimit)
{
	// print comments
	while( parser.cCommentList.size() )
	{
		if( parser.cCommentList[0].line < linelimit )
		{
			if(!newline) 
			{
				printoutput("\n", scope, newline, limiter);
				newline=true;
			}

			printoutput( (parser.cCommentList[0].multi)?"/*":"// ", scope, newline, limiter);
			printoutput( parser.cCommentList[0].content, scope, newline, limiter);
			printoutput( (parser.cCommentList[0].multi)?"*/\n":"\n", scope, newline, limiter);
			newline=true;
			parser.cCommentList.removeindex(0);
		}
		else
			break;
	}
}

bool print_beautified(CParser_CommentStore& parser, int rtpos, int &scope, bool &newline, bool &limiter)
{
	if( output )
	{
		bool ret = true;

		if( parser.rt[rtpos].symbol.Type == 1 )
		{	// terminals

			print_comments(parser, scope, newline, limiter, parser.rt[rtpos].cToken.line);

			switch( parser.rt[rtpos].symbol.idx )
			{
			case PT_RBRACE:
				scope--;
				if(!newline) printoutput("\n", scope, newline, limiter);
				newline=true;
				printoutput("}\n", scope, newline, limiter);
				newline=true;
				break;
			case PT_LBRACE:
				if(!newline) printoutput("\n", scope, newline, limiter);
				newline=true;
				printoutput("{\n", scope, newline, limiter);
				newline=true;
				scope++;
				break;
			case PT_SEMI:
				printoutput(";\n", scope, newline, limiter);
				newline=true;
				break;
			case PT_COMMA:
				printoutput(", ", scope, newline, limiter);
				break;
			case PT_LPARAN:
			case PT_RPARAN:
				printoutput((const char*)parser.rt[rtpos].cToken.cLexeme, scope, newline, limiter);
				break;
			default:
				// print the token
				printoutput((const char*)parser.rt[rtpos].cToken.cLexeme, scope, newline, limiter);
				break;
			}
		}
		else if( parser.rt[rtpos].cChildNum==1 )
		{	// only one child, just go down
			print_beautified(parser, parser.rt[rtpos].cChildPos, scope, newline, limiter);
		}
		else if( parser.rt[rtpos].cChildNum>1 )
		{	// nonterminals
			switch( parser.rt[rtpos].symbol.idx )
			{
			case PT_LABELSTM:
			{
				int tmpscope = scope;
				scope=0;
				print_beautified(parser, parser.rt[rtpos].cChildPos, scope, newline, limiter);
				printoutput(":\n", scope, newline, limiter);
				newline=true;
				scope = tmpscope;
				break;
			}
			case PT_CALLSTM:
				// transform to function calls
				print_beautified(parser, parser.rt[rtpos].cChildPos, scope, newline, limiter);
				printoutput("(", scope, newline, limiter);
				if( parser.rt[rtpos].cChildNum==3 )
					print_beautified(parser, parser.rt[rtpos].cChildPos+1, scope, newline, limiter);
				printoutput(");\n", scope, newline, limiter);
				newline=true;
				break;
			case PT_NORMALSTM:
			{	// can be:
				// if '(' <Expr> ')' <Normal Stm>
				// if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
				// while '(' <Expr> ')' <Normal Stm>
				// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
				// do <Normal Stm> while '(' <Expr> ')' ';'
				// switch '(' <Expr> ')' '{' <Case Stms> '}'
				// <ExprList> ';'
				// ';'              !Null statement
				if(!newline)
				{
					printoutput("\n", scope, newline, limiter);
					newline=true;
				}

				if( PT_IF == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx ||
					PT_WHILE == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("( ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput(" )\n", scope, newline, limiter);
					newline = true;

					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
						scope++;
					print_beautified(parser, parser.rt[rtpos].cChildPos+4, scope, newline, limiter);
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
						scope--;

					if( parser.rt[rtpos].cChildNum==7 )
					{
						print_beautified(parser, parser.rt[rtpos].cChildPos+5, scope, newline, limiter);
						if(!newline)
						{
							printoutput("\n", scope, newline, limiter);
							newline=true;
						}
						if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
							scope++;
						print_beautified(parser, parser.rt[rtpos].cChildPos+6, scope, newline, limiter);
						if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
							scope--;
					}
				}
				else if( PT_DO == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{	// do <Normal Stm> while '(' <Expr> ')' ';'
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("\n", scope, newline, limiter);
					newline = true;
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
						scope++;
					print_beautified(parser, parser.rt[rtpos].cChildPos+1, scope, newline, limiter);
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
						scope--;
					if(!newline)
					{
						printoutput("\n", scope, newline, limiter);
						newline=true;
					}
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput("( ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+4, scope, newline, limiter);
					printoutput(" )", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+6, scope, newline, limiter);
				}
				else if( PT_SWITCH == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{	// switch '(' <Expr> ')' '{' <Case Stms> '}'
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("( ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput(" )\n", scope, newline, limiter);
					newline = true;
					printoutput("{\n", scope, newline, limiter);
					newline = true;
					print_beautified(parser, parser.rt[rtpos].cChildPos+5, scope, newline, limiter);
					if(!newline)
					{
						printoutput("\n", scope, newline, limiter);
						newline=true;
					}
					printoutput("}\n", scope, newline, limiter);
					newline = true;
				}
				else if( PT_FOR == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{	// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("(", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput("; ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+4, scope, newline, limiter);
					printoutput("; ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+6, scope, newline, limiter);
					printoutput(")\n", scope, newline, limiter);
					newline=true;
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
						scope++;
					print_beautified(parser, parser.rt[rtpos].cChildPos+8, scope, newline, limiter);
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
						scope--;
				}
				else
				{
					size_t j,k;
					k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
					j = parser.rt[rtpos].cChildPos;
					for(; j<k; ++j)
					{
						print_beautified(parser, j, scope, newline, limiter);
					}
				}
				break;
			}
			case PT_CASESTMS:
			{	// <Case Stms>  ::= case <Value> ':' <Stm List> <Case Stms>
				//			   | default ':' <Stm List> <Case Stms>
				//			   |
				size_t j,k;
				int tmpscope = scope;
				k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
				for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
				{	// go down
					if( PT_COLON==parser.rt[j].symbol.idx )
					{
						printoutput(":\n", scope, newline, limiter);
						newline=true;
						scope++;
					}
					else
					{
						if( PT_CASESTMS==parser.rt[j].symbol.idx )
							scope--;
						print_beautified(parser, j, scope, newline, limiter);
					}
				}
				scope = tmpscope;
				break;
			}
			default:
			{
				size_t j,k;
				k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
				for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
				{	// go down
					print_beautified(parser, j, scope, newline, limiter);
				}
				break;
			}// end default case
			}// end switch
		}

		return ret;
	}
	return false;
}





///////////////////////////////////////////////////////////////////////////////////////
// parse processor
///////////////////////////////////////////////////////////////////////////////////////


#define OPT_PARSE			0x00
#define OPT_BEAUTIFY		0x01
#define OPT_PRINTTREE		0x02
#define OPT_TRANSFORM		0x04
#define OPT_COMPILEDEBUG	0x08
#define OPT_COMPILEOUTPUT	0x10

class PParser : public CFileProcessor
{
	CScriptCompiler&		compiler;
	CParser_CommentStore*	parser;
	int						option;
	

public:
	PParser(CScriptCompiler& c, CParser_CommentStore* p, int o) : compiler(c), parser(p), option(o)	{}

	virtual bool process(const char*name) const
	{
		bool ok = true;
		bool run = true;

		// Open input file
		if( !parser->input.open(name) ) {
			printf("Could not open input file %s\n", name);
			return false;
		}
		else
		{
			printf("processing input file %s\n", name);
		}

		while(run)
		{
			short p = parser->parse(PT_DECL);
			if (p < 0)
			{	// an error
				printf("Parse Error in file '%s', line %i, col %i\n", name, parser->input.line, parser->input.column);

				parser->print_expects();

				run = false;
				ok = false;
			}
			else if(0 == p)
			{	// finished
				run = false;
			}
			
			if( ok && parser->rt[0].symbol.idx==PT_DECL && parser->rt[0].cChildNum )
			{
				CStackElement *child = &(parser->rt[parser->rt[0].cChildPos]);
				if( child &&
					( child->symbol.idx == PT_BLOCK ||
					  child->symbol.idx == PT_FUNC ||
					  child->symbol.idx == PT_SCRIPT ||

					  child->symbol.idx == PT_OLDMAPFLAG ||
					  child->symbol.idx == PT_OLDSCRIPT ||
					  child->symbol.idx == PT_OLDFUNC ||
					  child->symbol.idx == PT_OLDNPC ||
					  child->symbol.idx == PT_OLDDUP ||
					  child->symbol.idx == PT_OLDMOB ||
					  child->symbol.idx == PT_OLDSHOP ||
					  child->symbol.idx == PT_OLDWARP ||
					  child->symbol.idx == PT_NPC ||
					  child->symbol.idx == PT_MOB ||
					  child->symbol.idx == PT_SHOP ||
					  child->symbol.idx == PT_WARP ||
					  
					  child->symbol.idx == PT_OLDMAPFLAGHEAD ||
					  child->symbol.idx == PT_OLDDUPHEAD ||
					  child->symbol.idx == PT_OLDSHOPHEAD ||
					  child->symbol.idx == PT_OLDWARPHEAD ||
					  child->symbol.idx == PT_OLDMONSTERHEAD
					  )
				  )
				{
					if( (option&OPT_BEAUTIFY)==OPT_BEAUTIFY )
					{
						bool newline=true;
						bool limiter=true;
						int scope = 0; 
						print_beautified(*parser, 0, scope, newline, limiter);
						print_comments(*parser, scope, newline, limiter, 0xFFFFFFFF);
					}
					if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
					{
						printf("(%li)----------------------------------------\n", (unsigned long)parser->rt.size());
						parser->print_rt_tree(0,0, false);
					}

					if( (option&OPT_TRANSFORM)==OPT_TRANSFORM || 
						(option&OPT_COMPILEDEBUG)==OPT_COMPILEDEBUG ||
						(option&OPT_COMPILEOUTPUT)==OPT_COMPILEOUTPUT )
					{
						//////////////////////////////////////////////////////////
						// tree transformation
						parsenode pnode(*parser);

						if( (option&OPT_TRANSFORM)==OPT_TRANSFORM )
							pnode.print_tree();
						
						//////////////////////////////////////////////////////////
						// compiling
						if( (option&OPT_COMPILEDEBUG)==OPT_COMPILEDEBUG ||
						(option&OPT_COMPILEOUTPUT)==OPT_COMPILEOUTPUT )
							run = compiler.CompileTree(pnode);
					}
					
					//////////////////////////////////////////////////////////
					// reinitialize parser
					parser->reinit();
//					printf("............................................(%i)\n", global::getcount());
				}
			}
		}
		parser->reset();
		return ok;
	}
};

void usage(const char*p)
{
	printf("usage: %s [engine file] [bptco] <input file/folder>\n", (p)?p:"<binary>");
	printf("     option b: outputs beautified code\n");
	printf("     option p: prints parse tree\n");
	printf("     option t: prints transformation tree\n");
	printf("     option c: prints compilation debug and output\n");
	printf("     option o: prints compilation output only\n");
}

int get_option(const char* p)
{
	int option = OPT_PARSE;
	if(p)
	{
		while(*p)
		{
			if(*p=='b')
				option |= OPT_BEAUTIFY;
			else if(*p=='p')
				option |= OPT_PRINTTREE;
			else if(*p=='t')
				option |= OPT_TRANSFORM;
			else if(*p=='c')
				option |= OPT_COMPILEDEBUG;
			else if(*p=='o')
				option |= OPT_COMPILEOUTPUT;
			p++;
		}
	}
	return option;
}


// Accepts 3 arguments [engine file] [option(s)] <input file>
int main(int argc, char *argv[])
{
//	buildEngine();

	ulong tick = GetTickCount();
	CParser_CommentStore* parser = 0;
	CParseConfig* parser_config = 0;
	bool ok;


	// parse commandline
	const char* enginefile=NULL;
	const char* inputfile=NULL;
	int i, option=OPT_PARSE;

	for(i=1; i<argc; ++i)
	{
		if( is_file(argv[i]) )
		{
			if(!enginefile)
				enginefile = argv[i];
			else 
				inputfile = argv[i];
		}
		else if( is_folder(argv[i]) )
		{
			inputfile = argv[i];
		}
		else
		{
			option = get_option(argv[i]);
		}
	}
	if(enginefile && !inputfile)
		swap(enginefile, inputfile);

	if(!inputfile)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if(enginefile)
	{
		try
		{
			parser_config = new CParseConfig( enginefile );
		}
		catch(...)
		{
			parser_config=NULL;
		}
		if (!parser_config)
		{
			printf("Could not open engine file %s\n", enginefile);
			return EXIT_FAILURE;		
		}
	}
	else
	{
		ulong sz;
		const unsigned char *e = getEngine(sz);
		if(!e)
		{
			printf("Error creating parser\n");
			return EXIT_FAILURE;
		}
		parser_config = new CParseConfig(e, sz);
		if (!parser_config)
			printf("Could not load engine\n");

	}

	parser = new CParser_CommentStore(parser_config);
	if (!parser){
		printf("Error creating parser\n");
		return EXIT_FAILURE;
	}

	CScriptEnvironment env;
	CScriptCompiler compiler(env, ((option&OPT_COMPILEDEBUG)==OPT_COMPILEDEBUG)?2:1);

	struct _buildin_func *pt = buildin_func;
	while(pt->name)
	{
		env.addFunction( pt->name, 0 );
		pt++;
	}

	PParser pp(compiler, parser, option);

	if( is_folder( inputfile ) )
	{
		ok=findFiles(inputfile, "*.txt", pp);
	}
	else
	{	// single file
		ok=pp.process( inputfile );
	}
	printf("\nready (%i)\n", ok);
	if (parser)  delete parser;
	if (parser_config) delete parser_config;

	printf("elapsed time: %li\n", (unsigned long)(GetTickCount()-tick));

	return EXIT_SUCCESS;
}
