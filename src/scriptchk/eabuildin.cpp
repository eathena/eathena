// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "baseregex.h"
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

const buildin::declaration* buildin::get(const basics::string<>& name)
{
	return  buildin::table.search(name);
}

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
/// buildin system functions.
struct buildin_system : public buildin
{
	static basics::variant buildin_debugmes(const buildin::callparameter& st);
	static basics::variant buildin_getarg(const buildin::callparameter& st);
	static basics::variant buildin_rand(const buildin::callparameter& st);
	static basics::variant buildin_regex(const buildin::callparameter& st);
	static basics::variant buildin_strlen(const buildin::callparameter& st);
	static basics::variant buildin_size(const buildin::callparameter& st);

	buildin_system()
	{
		buildin::create("debugmes", buildin_debugmes);
		buildin::create("getarg", buildin_getarg);
		buildin::create("rand", buildin_rand);
		buildin::create("regex", buildin_regex);
		buildin::create("strlen", buildin_strlen);
		buildin::create("size", buildin_size);
	}
} buildin_system_i;

///////////////////////////////////////////////////////////////////////////////
/// prints message to console.
/// arbitrary number of parameters, attach linefeed.
/// returns always zero
basics::variant buildin_system::buildin_debugmes(const buildin::callparameter& st)
{
	const size_t sz=st.size();
	size_t i;
	for(i=0; i<sz; ++i)
		printf( st[i].get_string().c_str() );
	printf("\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// get argument by number.
/// returns the parameter corresponding to the number in the first parameter.
basics::variant buildin_system::buildin_getarg(const buildin::callparameter& st)
{
	const CStackEngine::callframe* fp = st.engine.get_frame(0);
	return fp?fp->get_parameter(st[0].get_int()):basics::variant();
}

///////////////////////////////////////////////////////////////////////////////
/// random number.
/// with no parameter, returns random integer
/// with one parameter N1, returns half-open interval with 0..N1-1
/// with two parameters N1&N2, returns random number with N1..N2
basics::variant buildin_system::buildin_rand(const buildin::callparameter& st)
{
	sint64 a = rand();
	a *= rand();
	a *= rand();
	a *= rand();
	if(a<0)
		a = -a;
	if(st.size()==1)
	{	// 0...(st[0]-1)
		const sint64 v = st[0].get_int();
		if(v<0)
		{
			a %= -v;
			a = -a;
		}
		else
		{
			a %= v;
		}
	}
	else if(st.size()>=1)
	{	// (st[0])...(st[1])
		const sint64 v0 = st[0].get_int();
		const sint64 v1 = st[1].get_int();
		const sint64 d  = v1-v0;
		if(d<0)
		{
			a %= (1-d);
			a = -a;
		}
		else
		{
			a %= (1+d);
		}
		a += v0;
	}
	return a;
}
///////////////////////////////////////////////////////////////////////////////
/// regular expression.
/// executes regex on first parameter with match string on parameter 2
/// stores results in parameter 3, when existing, 
/// returns true or false depending on the match
basics::variant buildin_system::buildin_regex(const buildin::callparameter& st)
{
	basics::CRegExp rx(st[0].get_string());
	int ret = rx.match(st[1].get_string());
	if( ret && st.size()>2 )
	{	// fill the variables
		basics::variant &var = st[2];
		const size_t sz = rx.sub_count();
		size_t i;
		var.create_array( 1+sz );
		for(i=0; i<=sz; ++i)
			var[i] = rx[i];
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
/// string length.
/// return length of the string
basics::variant buildin_system::buildin_strlen(const buildin::callparameter& st)
{
	return st[0].get_string().length();
}
///////////////////////////////////////////////////////////////////////////////
/// size of value.
/// same as sizeof(parameter1)
basics::variant buildin_system::buildin_size(const buildin::callparameter& st)
{
	return st[0].size();
}


///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin_variable : public buildin
{
	static basics::variant buildin_player(const buildin::callparameter& st);
	static basics::variant buildin_account(const buildin::callparameter& st);
	static basics::variant buildin_login(const buildin::callparameter& st);
	static basics::variant buildin_party(const buildin::callparameter& st);
	static basics::variant buildin_guild(const buildin::callparameter& st);
	static basics::variant buildin_npc(const buildin::callparameter& st);
	static basics::variant buildin_map(const buildin::callparameter& st);

	buildin_variable()
	{
		buildin::create("player", buildin_player);
		buildin::create("account",buildin_account);
		buildin::create("login",  buildin_login);
		buildin::create("party",  buildin_party);
		buildin::create("guild",  buildin_guild);
		buildin::create("npc",    buildin_npc);
		buildin::create("map",    buildin_map);
	}
} buildin_variable_i;


basics::variant buildin_variable::buildin_player(const buildin::callparameter& st)
{
	// check current player
	// return none when invalid

	// return player as named variable
	basics::string<> name;
	name << "player::perm::" << 0;
	
	return st.engine.sGlobalVariables.get_variable(name);
}
basics::variant buildin_variable::buildin_account(const buildin::callparameter& st)
{
	// check current account
	// return none when invalid

	// return account as named variable
	basics::string<> name;
	name << "account::perm::" << 0;
	return st.engine.sGlobalVariables.get_variable(name);
}
basics::variant buildin_variable::buildin_login(const buildin::callparameter& st)
{
	// check current login
	// return none when invalid

	// return login as named variable
	basics::string<> name;
	name << "login::perm::" << 0;
	return st.engine.sGlobalVariables.get_variable(name);
}
basics::variant buildin_variable::buildin_party(const buildin::callparameter& st)
{
	// check current player's party
	// return none when invalid

	// return party as named variable
	basics::string<> name;
	name << "party::perm::" << 0;
	return st.engine.sGlobalVariables.get_variable(name);
}
basics::variant buildin_variable::buildin_guild(const buildin::callparameter& st)
{
	// check current player's guild
	// return none when invalid

	// return party as named variable
	basics::string<> name;
	name << "party::perm::" << 0;
	return st.engine.sGlobalVariables.get_variable(name);
}
basics::variant buildin_variable::buildin_npc(const buildin::callparameter& st)
{
	// check current player's guild
	// return none when invalid

	// return party as named variable
	basics::string<> name;
	name << "npc::perm::"<< 0;
	return st.engine.sGlobalVariables.get_variable(name);

}
basics::variant buildin_variable::buildin_map(const buildin::callparameter& st)
{
	// check current player's guild
	// return none when invalid

	// return party as named variable
	basics::string<> name;
	name << "map::perm::" << 0;
	return st.engine.sGlobalVariables.get_variable(name);
}



///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin_gui : public buildin
{
	static basics::variant buildin_close(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_dialog(const buildin::callparameter& st);
	static basics::variant buildin_inputnumber(const buildin::callparameter& st);
	static basics::variant buildin_inputstring(const buildin::callparameter& st);
	static basics::variant buildin_menu(const buildin::callparameter& st);
	static basics::variant buildin_mes(const buildin::callparameter& st) { return buildin_dialog(st); } 
	static basics::variant buildin_next(const buildin::callparameter& st);
	static basics::variant buildin_select(const buildin::callparameter& st);
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
} buildin_gui_i;


basics::variant buildin_gui::buildin_dialog(const buildin::callparameter& st)
{
	const size_t sz=st.size();
	size_t i;
	for(i=0; i<sz; ++i)
		printf( st[i].get_string().c_str() );
	printf("\n");
	return 0;
}
basics::variant buildin_gui::buildin_inputnumber(const buildin::callparameter& st)
{
	char buffer[128];
	size_t i;
	int ch;
	printf( "Enter a number: " );
	for(i=0; (i<sizeof(buffer)-1) &&  ((ch = getchar()) != EOF) && (ch != '\n'); ++i)
		buffer[i] = (char)ch;
	return basics::variant( basics::stringtoi(buffer) );
}
basics::variant buildin_gui::buildin_inputstring(const buildin::callparameter& st)
{
	char buffer[128];
	size_t i;
	int ch;
	printf( "Enter a string: " );
	for(i=0; (i<sizeof(buffer)-1) &&  ((ch = getchar()) != EOF) && (ch != '\n'); ++i)
		buffer[i] = (char)ch;
	buffer[i] = '\0';
	return basics::variant( basics::stringtoi(buffer) );
}
basics::variant buildin_gui::buildin_menu(const buildin::callparameter& st)
{
	printf("choose from:\n");
	size_t i, k, c=0;
	for(i=0; i<st.size(); ++i)
	{
		const basics::variant& elem = st[i];
		if( elem.is_array() )
		{
			for(k=0; k<elem.size(); ++k)
			{
				++c;
				printf("%i: '%s'\n", (int)c, basics::tostring(elem[k]).c_str());
			}
		}
		else
		{
			++c;
			printf("%i: '%s'\n", (int)c, basics::tostring(elem).c_str());
		}
	}
	return buildin_gui::buildin_inputnumber(st);
}
basics::variant buildin_gui::buildin_next(const buildin::callparameter& st)
{
	int ch;
	do
	{
		printf("\r<next>");
	}
	while( ((ch = getchar()) != EOF) && (ch != '\n') );
	return 0;
}
basics::variant buildin_gui::buildin_select(const buildin::callparameter& st)
{
	return buildin_gui::buildin_menu(st);
}


///////////////////////////////////////////////////////////////////////////////
/// 
struct buildin_specific : public buildin
{
	static basics::variant buildin_activatepset(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_addtimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_addtimercount(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_addtoskill(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_adopt(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_agitcheck(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_agitend(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_agitstart(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_announce(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_areaannounce(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_areamonster(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_areawarp(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_attachnpctimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_attachrid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_basicskillcheck(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_birthpet(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_bonus(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_bonus2(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_bonus3(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_bonus4(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_callshop(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_cardscnt(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_catchpet(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_changebase(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_changesex(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkcart(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkequipedcard(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkfalcon(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkoption(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkoption1(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkoption2(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkriding(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_checkweight(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_classchange(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_clearitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_cmdothernpc(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_countitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_cutin(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_cutincard(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_day(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_deactivatepset(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_defpattern(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_deletepset(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_delitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_deltimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_delwaitingroom(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_detachnpctimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_detachrid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_disablearena(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_disablenpc(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_disablewaitingroomevent(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_dispbottom(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_divorce(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_doevent(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_donpcevent(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_emotion(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_enablearena(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_enablenpc(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_enablewaitingroomevent(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_end(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_failedrefitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_failedremovecards(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_fakenpcname(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_flagemblem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getareadropitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getareausers(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getbrokenid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getcastledata(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getcastlename(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getcharid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getchildid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequipcardcnt(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequipid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequipisenableref(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequipisequiped(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequipisidentify(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequipname(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequippercentrefinery(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequiprefinerycnt(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getequipweaponlv(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getexp(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getgdskilllv(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getgmlevel(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getguildmaster(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getguildmasterid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getguildname(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getinventorylist(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getitem2(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getiteminfo(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getitemname(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getlook(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getmapmobs(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_getmapusers(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getmapxy(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getnameditem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getnpctimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getpartnerid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getpartymember(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getpartyname(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getpetinfo(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getrefine(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getsavepoint(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getscrate(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getskilllist(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getskilllv(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_gettime(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_gettimestr(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_gettimetick(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getusers(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getusersname(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_getwaitingroomstate(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_globalmes(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_gmcommand(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_guardian(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_guardianinfo(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_guildgetexp(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_guildopenstorage(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_guildskill(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_gvgoff(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_gvgon(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_hasitems(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_heal(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_hideoffnpc(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_hideonnpc(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_initnpctimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_inittimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_isday(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_isequipped(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_isequippedcnt(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_isloggedin(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_isnight(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_ispartneron(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_itemheal(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_itemskill(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_jobchange(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_killmonster(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_killmonsterall(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_logmes(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_makeitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_makepet(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_mapannounce(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_maprespawnguildid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_mapwarp(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_marriage(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_message(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_misceffect(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_mobcount(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_monster(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_movenpc(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_night(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_npcskilleffect(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_npcspeed(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_npcstop(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_npctalk(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_npcwalkto(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_nude(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_openstorage(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_pc_emotion(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_pcstrcharinfo(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_percentheal(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_petheal(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_petloot(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_petrecovery(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_petskillattack(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_petskillattack2(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_petskillbonus(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_petskillsupport(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_produce(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_pvpoff(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_pvpon(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_readparam(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_recovery(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_removemapflag(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_repair(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_requestguildinfo(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_resetlvl(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_resetskill(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_resetstatus(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_savepoint(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_sc_end(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_sc_start(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_sc_start2(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_sc_start4(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setcart(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setcastledata(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setfalcon(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setlook(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setmapflag(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setmapflagnosave(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setnpctimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setoption(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_setriding(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_skill(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_skilleffect(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_skilluseid(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_skillusepos(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_soundeffect(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_soundeffectall(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_specialeffect(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_specialeffect2(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_startnpctimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_statusup(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_statusup2(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_stopnpctimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_stoptimer(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_strcharinfo(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_strmobinfo(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_successrefitem(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_successremovecards(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_summon(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_unequip(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_viewpoint(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_waitingroom(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_warp(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_warpguild(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_warppartner(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_warpparty(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_warpwaitingpc(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_wedding_effect(const buildin::callparameter& st) { return 0; } 

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
	static basics::variant buildin_cleararray(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_compare(const buildin::callparameter& st) { return 0; } 
	static basics::variant buildin_copyarray(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_deletearray(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_getarraysize(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_getelementofarray(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_set(const buildin::callparameter& st) { return 0; }
	static basics::variant buildin_setarray(const buildin::callparameter& st) { return 0; }

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

