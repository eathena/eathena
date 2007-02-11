// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////


#include "eabuildin.h"
#include "eaengine.h"


///////////////////////////////////////////////////////////////////////////////
basics::smap<basics::string<>, buildin::declaration>	buildin::table;

// protection for the static
static struct _delete_notify
{
	bool deleted;

	_delete_notify() : deleted(false)
	{}
	~_delete_notify()
	{
		deleted=true;
	}
} delete_notify;

bool buildin::exists(const basics::string<>& name)
{
	return NULL!=buildin::table.search(name);
}

size_t buildin::parameter_count(const basics::string<>& name)
{
	declaration *ptr = buildin::table.search(name);
	if(ptr && !ptr->overload)
	{
		return ptr->minparam;
	}
	return 0;
}

bool buildin::create(const basics::string<>& name, buildin_function f, size_t param)
{
	buildin::declaration &obj = buildin::table[name];
	if( obj.function && obj.function!=f )
	{
		fprintf(stderr, "function '%s' already defined\n", name.c_str());
		return false;
	}
	else
	{
		obj.function = f;
		obj.minparam = param;
		return true;
	}
}
bool buildin::create(const basics::string<>& name, scriptdecl* o)
{
	declaration &obj = buildin::table[name];
	if( obj.overload)
	{
		fprintf(stderr, "function '%s' already defined\n", name.c_str());
		return false;
	}
	else
	{
		obj.overload = o;
		return true;
	}
}
bool buildin::erase(const basics::string<>& name)
{
	declaration *ptr = 	( delete_notify.deleted )?NULL:buildin::table.search(name);
	if( ptr && ptr->overload )
	{
		if( ptr->function )
			ptr->overload = NULL;
		else
			buildin::table.erase(name);
		return true;
	}
	else
	{
		return false;
	}
}


///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin_system : public buildin
{
	static basics::variant myfunction(CStackEngine& st)
	{
		printf("calling %s\n", "myfunction");
		return basics::variant();
	}
	static basics::variant yourfunction(CStackEngine& st)
	{
		printf("calling %s\n", "yourfunction");
		return basics::variant();
	}
	

	static basics::variant buildin_debugmes(CStackEngine& st) { return 0; }
	static basics::variant buildin_getarg(CStackEngine& st) { return 0; }
	static basics::variant buildin_rand(CStackEngine& st) { return 0; }
	static basics::variant buildin_regex(CStackEngine& st) { return 0; }
	static basics::variant buildin_strlen(CStackEngine& st) { return 0; }

	buildin_system()
	{
		buildin::create("myfunction", myfunction);
		buildin::create("yourfunction", yourfunction);

		buildin::create("debugmes", buildin_debugmes);
		buildin::create("getarg", buildin_getarg);
		buildin::create("rand", buildin_rand);
		buildin::create("regex", buildin_regex);
		buildin::create("strlen", buildin_strlen);
	}
} buildin_system_i;

///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin_gui : public buildin
{
	static basics::variant buildin_close(CStackEngine& st) { return 0; } 
	static basics::variant buildin_dialog(CStackEngine& st) { return 0; } 
	static basics::variant buildin_inputnumber(CStackEngine& st) { return 0; } 
	static basics::variant buildin_inputstring(CStackEngine& st) { return 0; } 
	static basics::variant buildin_menu(CStackEngine& st) { return 0; } 
	static basics::variant buildin_mes(CStackEngine& st) { return 0; } 
	static basics::variant buildin_next(CStackEngine& st) { return 0; } 
	static basics::variant buildin_select(CStackEngine& st) { return 0; } 
	buildin_gui()
	{
		buildin::create("close", buildin_close);
		buildin::create("dialog", buildin_dialog);
		buildin::create("inputnumber", buildin_inputnumber);
		buildin::create("inputstring", buildin_inputstring);
		buildin::create("menu", buildin_menu);
		buildin::create("mes", buildin_mes);
		buildin::create("next", buildin_next);
		buildin::create("select", buildin_select);
	}
}buildin_gui_i;

///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin_specific : public buildin
{
	static basics::variant buildin_activatepset(CStackEngine& st) { return 0; } 
	static basics::variant buildin_addtimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_addtimercount(CStackEngine& st) { return 0; } 
	static basics::variant buildin_addtoskill(CStackEngine& st) { return 0; } 
	static basics::variant buildin_adopt(CStackEngine& st) { return 0; } 
	static basics::variant buildin_agitcheck(CStackEngine& st) { return 0; } 
	static basics::variant buildin_agitend(CStackEngine& st) { return 0; } 
	static basics::variant buildin_agitstart(CStackEngine& st) { return 0; } 
	static basics::variant buildin_announce(CStackEngine& st) { return 0; } 
	static basics::variant buildin_areaannounce(CStackEngine& st) { return 0; } 
	static basics::variant buildin_areamonster(CStackEngine& st) { return 0; } 
	static basics::variant buildin_areawarp(CStackEngine& st) { return 0; } 
	static basics::variant buildin_attachnpctimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_attachrid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_basicskillcheck(CStackEngine& st) { return 0; } 
	static basics::variant buildin_birthpet(CStackEngine& st) { return 0; } 
	static basics::variant buildin_bonus(CStackEngine& st) { return 0; } 
	static basics::variant buildin_bonus2(CStackEngine& st) { return 0; } 
	static basics::variant buildin_bonus3(CStackEngine& st) { return 0; } 
	static basics::variant buildin_bonus4(CStackEngine& st) { return 0; } 
	static basics::variant buildin_callshop(CStackEngine& st) { return 0; }
	static basics::variant buildin_cardscnt(CStackEngine& st) { return 0; } 
	static basics::variant buildin_catchpet(CStackEngine& st) { return 0; } 
	static basics::variant buildin_changebase(CStackEngine& st) { return 0; } 
	static basics::variant buildin_changesex(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkcart(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkequipedcard(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkfalcon(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkoption(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkoption1(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkoption2(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkriding(CStackEngine& st) { return 0; } 
	static basics::variant buildin_checkweight(CStackEngine& st) { return 0; } 
	static basics::variant buildin_classchange(CStackEngine& st) { return 0; } 
	static basics::variant buildin_clearitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_cmdothernpc(CStackEngine& st) { return 0; } 
	static basics::variant buildin_countitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_cutin(CStackEngine& st) { return 0; } 
	static basics::variant buildin_cutincard(CStackEngine& st) { return 0; } 
	static basics::variant buildin_day(CStackEngine& st) { return 0; } 
	static basics::variant buildin_deactivatepset(CStackEngine& st) { return 0; } 
	static basics::variant buildin_defpattern(CStackEngine& st) { return 0; } 
	static basics::variant buildin_deletepset(CStackEngine& st) { return 0; }
	static basics::variant buildin_delitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_deltimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_delwaitingroom(CStackEngine& st) { return 0; } 
	static basics::variant buildin_detachnpctimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_detachrid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_disablearena(CStackEngine& st) { return 0; } 
	static basics::variant buildin_disablenpc(CStackEngine& st) { return 0; } 
	static basics::variant buildin_disablewaitingroomevent(CStackEngine& st) { return 0; } 
	static basics::variant buildin_dispbottom(CStackEngine& st) { return 0; } 
	static basics::variant buildin_divorce(CStackEngine& st) { return 0; } 
	static basics::variant buildin_doevent(CStackEngine& st) { return 0; } 
	static basics::variant buildin_donpcevent(CStackEngine& st) { return 0; } 
	static basics::variant buildin_emotion(CStackEngine& st) { return 0; } 
	static basics::variant buildin_enablearena(CStackEngine& st) { return 0; } 
	static basics::variant buildin_enablenpc(CStackEngine& st) { return 0; } 
	static basics::variant buildin_enablewaitingroomevent(CStackEngine& st) { return 0; } 
	static basics::variant buildin_end(CStackEngine& st) { return 0; }
	static basics::variant buildin_failedrefitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_failedremovecards(CStackEngine& st) { return 0; } 
	static basics::variant buildin_fakenpcname(CStackEngine& st) { return 0; }
	static basics::variant buildin_flagemblem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getareadropitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getareausers(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getbrokenid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getcastledata(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getcastlename(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getcharid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getchildid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequipcardcnt(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequipid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequipisenableref(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequipisequiped(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequipisidentify(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequipname(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequippercentrefinery(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequiprefinerycnt(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getequipweaponlv(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getexp(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getgdskilllv(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getgmlevel(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getguildmaster(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getguildmasterid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getguildname(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getinventorylist(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getitem2(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getiteminfo(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getitemname(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getlook(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getmapmobs(CStackEngine& st) { return 0; }
	static basics::variant buildin_getmapusers(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getmapxy(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getnameditem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getnpctimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getpartnerid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getpartymember(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getpartyname(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getpetinfo(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getrefine(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getsavepoint(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getscrate(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getskilllist(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getskilllv(CStackEngine& st) { return 0; } 
	static basics::variant buildin_gettime(CStackEngine& st) { return 0; } 
	static basics::variant buildin_gettimestr(CStackEngine& st) { return 0; } 
	static basics::variant buildin_gettimetick(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getusers(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getusersname(CStackEngine& st) { return 0; } 
	static basics::variant buildin_getwaitingroomstate(CStackEngine& st) { return 0; } 
	static basics::variant buildin_globalmes(CStackEngine& st) { return 0; } 
	static basics::variant buildin_gmcommand(CStackEngine& st) { return 0; } 
	static basics::variant buildin_guardian(CStackEngine& st) { return 0; } 
	static basics::variant buildin_guardianinfo(CStackEngine& st) { return 0; } 
	static basics::variant buildin_guildgetexp(CStackEngine& st) { return 0; } 
	static basics::variant buildin_guildopenstorage(CStackEngine& st) { return 0; } 
	static basics::variant buildin_guildskill(CStackEngine& st) { return 0; } 
	static basics::variant buildin_gvgoff(CStackEngine& st) { return 0; } 
	static basics::variant buildin_gvgon(CStackEngine& st) { return 0; } 
	static basics::variant buildin_hasitems(CStackEngine& st) { return 0; } 
	static basics::variant buildin_heal(CStackEngine& st) { return 0; } 
	static basics::variant buildin_hideoffnpc(CStackEngine& st) { return 0; } 
	static basics::variant buildin_hideonnpc(CStackEngine& st) { return 0; } 
	static basics::variant buildin_initnpctimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_inittimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_isday(CStackEngine& st) { return 0; } 
	static basics::variant buildin_isequipped(CStackEngine& st) { return 0; } 
	static basics::variant buildin_isequippedcnt(CStackEngine& st) { return 0; } 
	static basics::variant buildin_isloggedin(CStackEngine& st) { return 0; } 
	static basics::variant buildin_isnight(CStackEngine& st) { return 0; } 
	static basics::variant buildin_ispartneron(CStackEngine& st) { return 0; } 
	static basics::variant buildin_itemheal(CStackEngine& st) { return 0; } 
	static basics::variant buildin_itemskill(CStackEngine& st) { return 0; } 
	static basics::variant buildin_jobchange(CStackEngine& st) { return 0; } 
	static basics::variant buildin_killmonster(CStackEngine& st) { return 0; } 
	static basics::variant buildin_killmonsterall(CStackEngine& st) { return 0; } 
	static basics::variant buildin_logmes(CStackEngine& st) { return 0; } 
	static basics::variant buildin_makeitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_makepet(CStackEngine& st) { return 0; } 
	static basics::variant buildin_mapannounce(CStackEngine& st) { return 0; } 
	static basics::variant buildin_maprespawnguildid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_mapwarp(CStackEngine& st) { return 0; } 
	static basics::variant buildin_marriage(CStackEngine& st) { return 0; } 
	static basics::variant buildin_message(CStackEngine& st) { return 0; } 
	static basics::variant buildin_misceffect(CStackEngine& st) { return 0; } 
	static basics::variant buildin_mobcount(CStackEngine& st) { return 0; } 
	static basics::variant buildin_monster(CStackEngine& st) { return 0; } 
	static basics::variant buildin_movenpc(CStackEngine& st) { return 0; } 
	static basics::variant buildin_night(CStackEngine& st) { return 0; } 
	static basics::variant buildin_npcskilleffect(CStackEngine& st) { return 0; } 
	static basics::variant buildin_npcspeed(CStackEngine& st) { return 0; } 
	static basics::variant buildin_npcstop(CStackEngine& st) { return 0; } 
	static basics::variant buildin_npctalk(CStackEngine& st) { return 0; } 
	static basics::variant buildin_npcwalkto(CStackEngine& st) { return 0; } 
	static basics::variant buildin_nude(CStackEngine& st) { return 0; } 
	static basics::variant buildin_openstorage(CStackEngine& st) { return 0; } 
	static basics::variant buildin_pc_emotion(CStackEngine& st) { return 0; } 
	static basics::variant buildin_pcstrcharinfo(CStackEngine& st) { return 0; } 
	static basics::variant buildin_percentheal(CStackEngine& st) { return 0; } 
	static basics::variant buildin_petheal(CStackEngine& st) { return 0; } 
	static basics::variant buildin_petloot(CStackEngine& st) { return 0; } 
	static basics::variant buildin_petrecovery(CStackEngine& st) { return 0; } 
	static basics::variant buildin_petskillattack(CStackEngine& st) { return 0; } 
	static basics::variant buildin_petskillattack2(CStackEngine& st) { return 0; } 
	static basics::variant buildin_petskillbonus(CStackEngine& st) { return 0; } 
	static basics::variant buildin_petskillsupport(CStackEngine& st) { return 0; } 
	static basics::variant buildin_produce(CStackEngine& st) { return 0; } 
	static basics::variant buildin_pvpoff(CStackEngine& st) { return 0; } 
	static basics::variant buildin_pvpon(CStackEngine& st) { return 0; } 
	static basics::variant buildin_readparam(CStackEngine& st) { return 0; } 
	static basics::variant buildin_recovery(CStackEngine& st) { return 0; } 
	static basics::variant buildin_removemapflag(CStackEngine& st) { return 0; } 
	static basics::variant buildin_repair(CStackEngine& st) { return 0; } 
	static basics::variant buildin_requestguildinfo(CStackEngine& st) { return 0; } 
	static basics::variant buildin_resetlvl(CStackEngine& st) { return 0; } 
	static basics::variant buildin_resetskill(CStackEngine& st) { return 0; } 
	static basics::variant buildin_resetstatus(CStackEngine& st) { return 0; } 
	static basics::variant buildin_savepoint(CStackEngine& st) { return 0; } 
	static basics::variant buildin_sc_end(CStackEngine& st) { return 0; } 
	static basics::variant buildin_sc_start(CStackEngine& st) { return 0; } 
	static basics::variant buildin_sc_start2(CStackEngine& st) { return 0; } 
	static basics::variant buildin_sc_start4(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setcart(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setcastledata(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setfalcon(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setlook(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setmapflag(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setmapflagnosave(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setnpctimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setoption(CStackEngine& st) { return 0; } 
	static basics::variant buildin_setriding(CStackEngine& st) { return 0; } 
	static basics::variant buildin_skill(CStackEngine& st) { return 0; } 
	static basics::variant buildin_skilleffect(CStackEngine& st) { return 0; } 
	static basics::variant buildin_skilluseid(CStackEngine& st) { return 0; } 
	static basics::variant buildin_skillusepos(CStackEngine& st) { return 0; } 
	static basics::variant buildin_soundeffect(CStackEngine& st) { return 0; } 
	static basics::variant buildin_soundeffectall(CStackEngine& st) { return 0; } 
	static basics::variant buildin_specialeffect(CStackEngine& st) { return 0; } 
	static basics::variant buildin_specialeffect2(CStackEngine& st) { return 0; } 
	static basics::variant buildin_startnpctimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_statusup(CStackEngine& st) { return 0; } 
	static basics::variant buildin_statusup2(CStackEngine& st) { return 0; } 
	static basics::variant buildin_stopnpctimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_stoptimer(CStackEngine& st) { return 0; } 
	static basics::variant buildin_strcharinfo(CStackEngine& st) { return 0; } 
	static basics::variant buildin_strmobinfo(CStackEngine& st) { return 0; } 
	static basics::variant buildin_successrefitem(CStackEngine& st) { return 0; } 
	static basics::variant buildin_successremovecards(CStackEngine& st) { return 0; } 
	static basics::variant buildin_summon(CStackEngine& st) { return 0; } 
	static basics::variant buildin_unequip(CStackEngine& st) { return 0; }
	static basics::variant buildin_viewpoint(CStackEngine& st) { return 0; } 
	static basics::variant buildin_waitingroom(CStackEngine& st) { return 0; } 
	static basics::variant buildin_warp(CStackEngine& st) { return 0; } 
	static basics::variant buildin_warpguild(CStackEngine& st) { return 0; } 
	static basics::variant buildin_warppartner(CStackEngine& st) { return 0; } 
	static basics::variant buildin_warpparty(CStackEngine& st) { return 0; } 
	static basics::variant buildin_warpwaitingpc(CStackEngine& st) { return 0; } 
	static basics::variant buildin_wedding_effect(CStackEngine& st) { return 0; } 

	buildin_specific()
	{
		buildin::create("activatepset", buildin_activatepset);
		buildin::create("addtimer", buildin_addtimer);
		buildin::create("addtimercount", buildin_addtimercount);
		buildin::create("addtoskill", buildin_addtoskill);
		buildin::create("adopt", buildin_adopt);
		buildin::create("agitcheck", buildin_agitcheck);
		buildin::create("agitend", buildin_agitend);
		buildin::create("agitstart", buildin_agitstart);
		buildin::create("announce", buildin_announce);
		buildin::create("areaannounce", buildin_areaannounce);
		buildin::create("areamonster", buildin_areamonster);
		buildin::create("areawarp", buildin_areawarp);
		buildin::create("attachnpctimer", buildin_attachnpctimer);
		buildin::create("attachrid", buildin_attachrid);
		buildin::create("basicskillcheck", buildin_basicskillcheck);
		buildin::create("birthpet", buildin_birthpet);
		buildin::create("bonus", buildin_bonus);
		buildin::create("bonus2", buildin_bonus2);
		buildin::create("bonus3", buildin_bonus3);
		buildin::create("bonus4", buildin_bonus4);
		buildin::create("callshop", buildin_callshop);
		buildin::create("cardscnt", buildin_cardscnt);
		buildin::create("catchpet", buildin_catchpet);
		buildin::create("changebase", buildin_changebase);
		buildin::create("changesex", buildin_changesex);
		buildin::create("checkcart", buildin_checkcart);
		buildin::create("checkequipedcard", buildin_checkequipedcard);
		buildin::create("checkfalcon", buildin_checkfalcon);
		buildin::create("checkoption", buildin_checkoption);
		buildin::create("checkoption1", buildin_checkoption1);
		buildin::create("checkoption2", buildin_checkoption2);
		buildin::create("checkriding", buildin_checkriding);
		buildin::create("checkweight", buildin_checkweight);
		buildin::create("classchange", buildin_classchange);
		buildin::create("clearitem", buildin_clearitem);
		buildin::create("cmdothernpc", buildin_cmdothernpc);
		buildin::create("countitem", buildin_countitem);
		buildin::create("cutin", buildin_cutin);
		buildin::create("cutincard", buildin_cutincard);
		buildin::create("day", buildin_day);
		buildin::create("deactivatepset", buildin_deactivatepset);
		buildin::create("defpattern", buildin_defpattern);
		buildin::create("deletepset", buildin_deletepset);
		buildin::create("delitem", buildin_delitem);
		buildin::create("deltimer", buildin_deltimer);
		buildin::create("delwaitingroom", buildin_delwaitingroom);
		buildin::create("detachnpctimer", buildin_detachnpctimer);
		buildin::create("detachrid", buildin_detachrid);
		buildin::create("disablearena", buildin_disablearena);
		buildin::create("disablenpc", buildin_disablenpc);
		buildin::create("disablewaitingroomevent", buildin_disablewaitingroomevent);
		buildin::create("dispbottom", buildin_dispbottom);
		buildin::create("divorce", buildin_divorce);
		buildin::create("doevent", buildin_doevent);
		buildin::create("donpcevent", buildin_donpcevent);
		buildin::create("emotion", buildin_emotion);
		buildin::create("enablearena", buildin_enablearena);
		buildin::create("enablenpc", buildin_enablenpc);
		buildin::create("enablewaitingroomevent", buildin_enablewaitingroomevent);
		buildin::create("end", buildin_end);
		buildin::create("failedrefitem", buildin_failedrefitem);
		buildin::create("failedremovecards", buildin_failedremovecards);
		buildin::create("fakenpcname", buildin_fakenpcname);
		buildin::create("flagemblem", buildin_flagemblem);
		buildin::create("getareadropitem", buildin_getareadropitem);
		buildin::create("getareausers", buildin_getareausers);
		buildin::create("getbrokenid", buildin_getbrokenid);
		buildin::create("getcastledata", buildin_getcastledata);
		buildin::create("getcastlename", buildin_getcastlename);
		buildin::create("getcharid", buildin_getcharid);
		buildin::create("getchildid", buildin_getchildid);
		buildin::create("getequipcardcnt", buildin_getequipcardcnt);
		buildin::create("getequipid", buildin_getequipid);
		buildin::create("getequipisenableref", buildin_getequipisenableref);
		buildin::create("getequipisequiped", buildin_getequipisequiped);
		buildin::create("getequipisidentify", buildin_getequipisidentify);
		buildin::create("getequipname", buildin_getequipname);
		buildin::create("getequippercentrefinery", buildin_getequippercentrefinery);
		buildin::create("getequiprefinerycnt", buildin_getequiprefinerycnt);
		buildin::create("getequipweaponlv", buildin_getequipweaponlv);
		buildin::create("getexp", buildin_getexp);
		buildin::create("getgdskilllv", buildin_getgdskilllv);
		buildin::create("getgmlevel", buildin_getgmlevel);
		buildin::create("getguildmaster", buildin_getguildmaster);
		buildin::create("getguildmasterid", buildin_getguildmasterid);
		buildin::create("getguildname", buildin_getguildname);
		buildin::create("getinventorylist", buildin_getinventorylist);
		buildin::create("getitem", buildin_getitem);
		buildin::create("getitem2", buildin_getitem2);
		buildin::create("getiteminfo", buildin_getiteminfo);
		buildin::create("getitemname", buildin_getitemname);
		buildin::create("getlook", buildin_getlook);
		buildin::create("getmapmobs", buildin_getmapmobs);
		buildin::create("getmapusers", buildin_getmapusers);
		buildin::create("getmapxy", buildin_getmapxy);
		buildin::create("getnameditem", buildin_getnameditem);
		buildin::create("getnpctimer", buildin_getnpctimer);
		buildin::create("getpartnerid", buildin_getpartnerid);
		buildin::create("getpartymember", buildin_getpartymember);
		buildin::create("getpartyname", buildin_getpartyname);
		buildin::create("getpetinfo", buildin_getpetinfo);
		buildin::create("getrefine", buildin_getrefine);
		buildin::create("getsavepoint", buildin_getsavepoint);
		buildin::create("getscrate", buildin_getscrate);
		buildin::create("getskilllist", buildin_getskilllist);
		buildin::create("getskilllv", buildin_getskilllv);
		buildin::create("gettime", buildin_gettime);
		buildin::create("gettimestr", buildin_gettimestr);
		buildin::create("gettimetick", buildin_gettimetick);
		buildin::create("getusers", buildin_getusers);
		buildin::create("getusersname", buildin_getusersname);
		buildin::create("getwaitingroomstate", buildin_getwaitingroomstate);
		buildin::create("globalmes", buildin_globalmes);
		buildin::create("gmcommand", buildin_gmcommand);
		buildin::create("guardian", buildin_guardian);
		buildin::create("guardianinfo", buildin_guardianinfo);
		buildin::create("guildgetexp", buildin_guildgetexp);
		buildin::create("guildopenstorage", buildin_guildopenstorage);
		buildin::create("guildskill", buildin_guildskill);
		buildin::create("gvgoff", buildin_gvgoff);
		buildin::create("gvgon", buildin_gvgon);
		buildin::create("hasitems", buildin_hasitems);
		buildin::create("heal", buildin_heal);
		buildin::create("hideoffnpc", buildin_hideoffnpc);
		buildin::create("hideonnpc", buildin_hideonnpc);
		buildin::create("initnpctimer", buildin_initnpctimer);
		buildin::create("inittimer", buildin_inittimer);
		buildin::create("isday", buildin_isday);
		buildin::create("isequipped", buildin_isequipped);
		buildin::create("isequippedcnt", buildin_isequippedcnt);
		buildin::create("isloggedin", buildin_isloggedin);
		buildin::create("isnight", buildin_isnight);
		buildin::create("ispartneron", buildin_ispartneron);
		buildin::create("itemheal", buildin_itemheal);
		buildin::create("itemskill", buildin_itemskill);
		buildin::create("jobchange", buildin_jobchange);
		buildin::create("killmonster", buildin_killmonster);
		buildin::create("killmonsterall", buildin_killmonsterall);
		buildin::create("logmes", buildin_logmes);
		buildin::create("makeitem", buildin_makeitem);
		buildin::create("makepet", buildin_makepet);
		buildin::create("mapannounce", buildin_mapannounce);
		buildin::create("maprespawnguildid", buildin_maprespawnguildid);
		buildin::create("mapwarp", buildin_mapwarp);
		buildin::create("marriage", buildin_marriage);
		buildin::create("message", buildin_message);
		buildin::create("misceffect", buildin_misceffect);
		buildin::create("mobcount", buildin_mobcount);
		buildin::create("monster", buildin_monster);
		buildin::create("movenpc", buildin_movenpc);
		buildin::create("night", buildin_night);
		buildin::create("npcskilleffect", buildin_npcskilleffect);
		buildin::create("npcspeed", buildin_npcspeed);
		buildin::create("npcstop", buildin_npcstop);
		buildin::create("npctalk", buildin_npctalk);
		buildin::create("npcwalkto", buildin_npcwalkto);
		buildin::create("nude", buildin_nude);
		buildin::create("openstorage", buildin_openstorage);
		buildin::create("pc_emotion", buildin_pc_emotion);
		buildin::create("pcstrcharinfo", buildin_pcstrcharinfo);
		buildin::create("percentheal", buildin_percentheal);
		buildin::create("petheal", buildin_petheal);
		buildin::create("petloot", buildin_petloot);
		buildin::create("petrecovery", buildin_petrecovery);
		buildin::create("petskillattack", buildin_petskillattack);
		buildin::create("petskillattack2", buildin_petskillattack2);
		buildin::create("petskillbonus", buildin_petskillbonus);
		buildin::create("petskillsupport", buildin_petskillsupport);
		buildin::create("produce", buildin_produce);
		buildin::create("pvpoff", buildin_pvpoff);
		buildin::create("pvpon", buildin_pvpon);
		buildin::create("readparam", buildin_readparam);
		buildin::create("recovery", buildin_recovery);
		buildin::create("removemapflag", buildin_removemapflag);
		buildin::create("repair", buildin_repair);
		buildin::create("requestguildinfo", buildin_requestguildinfo);
		buildin::create("resetlvl", buildin_resetlvl);
		buildin::create("resetskill", buildin_resetskill);
		buildin::create("resetstatus", buildin_resetstatus);
		buildin::create("savepoint", buildin_savepoint);
		buildin::create("sc_end", buildin_sc_end);
		buildin::create("sc_start", buildin_sc_start);
		buildin::create("sc_start2", buildin_sc_start2);
		buildin::create("sc_start4", buildin_sc_start4);
		buildin::create("setcart", buildin_setcart);
		buildin::create("setcastledata", buildin_setcastledata);
		buildin::create("setfalcon", buildin_setfalcon);
		buildin::create("setlook", buildin_setlook);
		buildin::create("setmapflag", buildin_setmapflag);
		buildin::create("setmapflagnosave", buildin_setmapflagnosave);
		buildin::create("setnpctimer", buildin_setnpctimer);
		buildin::create("setoption", buildin_setoption);
		buildin::create("setriding", buildin_setriding);
		buildin::create("skill", buildin_skill);
		buildin::create("skilleffect", buildin_skilleffect);
		buildin::create("skilluseid", buildin_skilluseid);
		buildin::create("skillusepos", buildin_skillusepos);
		buildin::create("soundeffect", buildin_soundeffect);
		buildin::create("soundeffectall", buildin_soundeffectall);
		buildin::create("specialeffect", buildin_specialeffect);
		buildin::create("specialeffect2", buildin_specialeffect2);
		buildin::create("startnpctimer", buildin_startnpctimer);
		buildin::create("statusup", buildin_statusup);
		buildin::create("statusup2", buildin_statusup2);
		buildin::create("stopnpctimer", buildin_stopnpctimer);
		buildin::create("stoptimer", buildin_stoptimer);
		buildin::create("strcharinfo", buildin_strcharinfo);
		buildin::create("strmobinfo", buildin_strmobinfo);
		buildin::create("successrefitem", buildin_successrefitem);
		buildin::create("successremovecards", buildin_successremovecards);
		buildin::create("summon", buildin_summon);
		buildin::create("unequip", buildin_unequip);
		buildin::create("viewpoint", buildin_viewpoint);
		buildin::create("waitingroom", buildin_waitingroom);
		buildin::create("warp", buildin_warp);
		buildin::create("warpguild", buildin_warpguild);
		buildin::create("warppartner", buildin_warppartner);
		buildin::create("warpparty", buildin_warpparty);
		buildin::create("warpwaitingpc", buildin_warpwaitingpc);
		buildin::create("wedding_effect", buildin_wedding_effect);
	}
} buildin_specific_i;

///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin_obsolete : public buildin
{
	static basics::variant buildin_cleararray(CStackEngine& st) { return 0; }
	static basics::variant buildin_compare(CStackEngine& st) { return 0; } 
	static basics::variant buildin_copyarray(CStackEngine& st) { return 0; }
	static basics::variant buildin_deletearray(CStackEngine& st) { return 0; }
	static basics::variant buildin_getarraysize(CStackEngine& st) { return 0; }
	static basics::variant buildin_getelementofarray(CStackEngine& st) { return 0; }
	static basics::variant buildin_set(CStackEngine& st) { return 0; }
	static basics::variant buildin_setarray(CStackEngine& st) { return 0; }

	buildin_obsolete()
	{
		buildin::create("cleararray", buildin_cleararray);
		buildin::create("compare", buildin_compare);
		buildin::create("copyarray", buildin_copyarray);
		buildin::create("deletearray", buildin_deletearray);
		buildin::create("getarraysize", buildin_getarraysize);
		buildin::create("getelementofarray", buildin_getelementofarray);
		buildin::create("set", buildin_set);
		buildin::create("setarray", buildin_setarray);
	}
} buildin_obsolete_i;

