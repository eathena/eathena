// $Id: script.c 148 2004-09-30 14:05:37Z MouseJstr $
//#define DEBUG_FUNCIN
//#define DEBUG_DISP
//#define DEBUG_RUN

#include "base.h"
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
#include "charcommand.h"

#define SCRIPT_BLOCK_SIZE 256

enum { LABEL_NEXTLINE=1,LABEL_START };
static char * script_buf = NULL;
static int script_pos,script_size;

char *str_buf;
size_t str_pos;
size_t str_size;

static struct s_str_data {
	int type;
	int str;
	int backpatch;
	int label;
	int (*func)(struct script_state &);
	int val;
	int next;
} *str_data = NULL;
int str_num=LABEL_START,str_data_size;
int str_hash[16];

static struct dbt *mapreg_db=NULL;
static struct dbt *mapregstr_db=NULL;
static int mapreg_dirty=-1;
char mapreg_txt[256]="save/mapreg.txt";
#define MAPREG_AUTOSAVE_INTERVAL	(10*1000)

static struct dbt *scriptlabel_db=NULL;
static struct dbt *userfunc_db=NULL;

struct dbt* script_get_label_db(){ return scriptlabel_db; }
struct dbt* script_get_userfunc_db(){ if(!userfunc_db) userfunc_db=strdb_init(50); return userfunc_db; }

int scriptlabel_final(void *k,void *d,va_list ap){ return 0; }
static char positions[11][64] = {"頭","体","左手","右手","ローブ","靴","アクセサリー1","アクセサリー2","頭2","頭3","装着していない"};

struct Script_Config script_config;

static int parse_cmd_if=0;
static int parse_cmd;

extern int current_equip_item_index; //for New CARS Scripts. It contains Inventory Index of the EQUIP_SCRIPT caller item. [Lupus]

/*==========================================
 * ローカルプロトタイプ宣言 (必要な物のみ)
 *------------------------------------------
 */
char* parse_subexpr(char *,int);
int buildin_mes(struct script_state &st);
int buildin_goto(struct script_state &st);
int buildin_callsub(struct script_state &st);
int buildin_callfunc(struct script_state &st);
int buildin_return(struct script_state &st);
int buildin_getarg(struct script_state &st);
int buildin_next(struct script_state &st);
int buildin_close(struct script_state &st);
int buildin_close2(struct script_state &st);
int buildin_menu(struct script_state &st);
int buildin_rand(struct script_state &st);
int buildin_warp(struct script_state &st);
int buildin_areawarp(struct script_state &st);
int buildin_heal(struct script_state &st);
int buildin_itemheal(struct script_state &st);
int buildin_percentheal(struct script_state &st);
int buildin_jobchange(struct script_state &st);
int buildin_input(struct script_state &st);
int buildin_setlook(struct script_state &st);
int buildin_set(struct script_state &st);
int buildin_setarray(struct script_state &st);
int buildin_cleararray(struct script_state &st);
int buildin_copyarray(struct script_state &st);
int buildin_getarraysize(struct script_state &st);
int buildin_deletearray(struct script_state &st);
int buildin_getelementofarray(struct script_state &st);
int buildin_if(struct script_state &st);
int buildin_getitem(struct script_state &st);
int buildin_getitem2(struct script_state &st);
int buildin_makeitem(struct script_state &st);
int buildin_delitem(struct script_state &st);
int buildin_viewpoint(struct script_state &st);
int buildin_countitem(struct script_state &st);
int buildin_checkweight(struct script_state &st);
int buildin_readparam(struct script_state &st);
int buildin_getcharid(struct script_state &st);
int buildin_getpartyname(struct script_state &st);
int buildin_getpartymember(struct script_state &st);
int buildin_getguildname(struct script_state &st);
int buildin_getguildmaster(struct script_state &st);
int buildin_getguildmasterid(struct script_state &st);
int buildin_strcharinfo(struct script_state &st);
int buildin_getequipid(struct script_state &st);
int buildin_getequipname(struct script_state &st);
int buildin_getbrokenid(struct script_state &st); // [Valaris]
int buildin_repair(struct script_state &st); // [Valaris]
int buildin_getequipisequiped(struct script_state &st);
int buildin_getequipisenableref(struct script_state &st);
int buildin_getequipisidentify(struct script_state &st);
int buildin_getequiprefinerycnt(struct script_state &st);
int buildin_getequipweaponlv(struct script_state &st);
int buildin_getequippercentrefinery(struct script_state &st);
int buildin_successrefitem(struct script_state &st);
int buildin_failedrefitem(struct script_state &st);
int buildin_cutin(struct script_state &st);
int buildin_cutincard(struct script_state &st);
int buildin_statusup(struct script_state &st);
int buildin_statusup2(struct script_state &st);
int buildin_bonus(struct script_state &st);
int buildin_bonus2(struct script_state &st);
int buildin_bonus3(struct script_state &st);
int buildin_bonus4(struct script_state &st);
int buildin_skill(struct script_state &st);
int buildin_addtoskill(struct script_state &st); // [Valaris]
int buildin_guildskill(struct script_state &st);
int buildin_getskilllv(struct script_state &st);
int buildin_getgdskilllv(struct script_state &st);
int buildin_basicskillcheck(struct script_state &st);
int buildin_getgmlevel(struct script_state &st);
int buildin_end(struct script_state &st);
int buildin_checkoption(struct script_state &st);
int buildin_setoption(struct script_state &st);
int buildin_setcart(struct script_state &st);
int buildin_checkcart(struct script_state &st); // check cart [Valaris]
int buildin_setfalcon(struct script_state &st);
int buildin_checkfalcon(struct script_state &st); // check falcon [Valaris]
int buildin_setriding(struct script_state &st);
int buildin_checkriding(struct script_state &st); // check for pecopeco [Valaris]
int buildin_savepoint(struct script_state &st);
int buildin_gettimetick(struct script_state &st);
int buildin_gettime(struct script_state &st);
int buildin_gettimestr(struct script_state &st);
int buildin_openstorage(struct script_state &st);
int buildin_guildopenstorage(struct script_state &st);
int buildin_itemskill(struct script_state &st);
int buildin_produce(struct script_state &st);
int buildin_monster(struct script_state &st);
int buildin_areamonster(struct script_state &st);
int buildin_killmonster(struct script_state &st);
int buildin_killmonsterall(struct script_state &st);
int buildin_doevent(struct script_state &st);
int buildin_donpcevent(struct script_state &st);
int buildin_addtimer(struct script_state &st);
int buildin_deltimer(struct script_state &st);
int buildin_addtimercount(struct script_state &st);
int buildin_initnpctimer(struct script_state &st);
int buildin_stopnpctimer(struct script_state &st);
int buildin_startnpctimer(struct script_state &st);
int buildin_setnpctimer(struct script_state &st);
int buildin_getnpctimer(struct script_state &st);
int buildin_attachnpctimer(struct script_state &st);	// [celest]
int buildin_detachnpctimer(struct script_state &st);	// [celest]
int buildin_announce(struct script_state &st);
int buildin_mapannounce(struct script_state &st);
int buildin_areaannounce(struct script_state &st);
int buildin_getusers(struct script_state &st);
int buildin_getmapusers(struct script_state &st);
int buildin_getareausers(struct script_state &st);
int buildin_getareadropitem(struct script_state &st);
int buildin_enablenpc(struct script_state &st);
int buildin_disablenpc(struct script_state &st);
int buildin_enablearena(struct script_state &st);	// Added by RoVeRT
int buildin_disablearena(struct script_state &st);	// Added by RoVeRT
int buildin_hideoffnpc(struct script_state &st);
int buildin_hideonnpc(struct script_state &st);
int buildin_sc_start(struct script_state &st);
int buildin_sc_start2(struct script_state &st);
int buildin_sc_end(struct script_state &st);
int buildin_getscrate(struct script_state &st);
int buildin_debugmes(struct script_state &st);
int buildin_catchpet(struct script_state &st);
int buildin_birthpet(struct script_state &st);
int buildin_resetlvl(struct script_state &st);
int buildin_resetstatus(struct script_state &st);
int buildin_resetskill(struct script_state &st);
int buildin_changebase(struct script_state &st);
int buildin_changesex(struct script_state &st);
int buildin_waitingroom(struct script_state &st);
int buildin_delwaitingroom(struct script_state &st);
int buildin_enablewaitingroomevent(struct script_state &st);
int buildin_disablewaitingroomevent(struct script_state &st);
int buildin_getwaitingroomstate(struct script_state &st);
int buildin_warpwaitingpc(struct script_state &st);
int buildin_attachrid(struct script_state &st);
int buildin_detachrid(struct script_state &st);
int buildin_isloggedin(struct script_state &st);
int buildin_setmapflagnosave(struct script_state &st);
int buildin_setmapflag(struct script_state &st);
int buildin_removemapflag(struct script_state &st);
int buildin_pvpon(struct script_state &st);
int buildin_pvpoff(struct script_state &st);
int buildin_gvgon(struct script_state &st);
int buildin_gvgoff(struct script_state &st);
int buildin_emotion(struct script_state &st);
int buildin_maprespawnguildid(struct script_state &st);
int buildin_agitstart(struct script_state &st);		// <Agit>
int buildin_agitend(struct script_state &st);
int buildin_agitcheck(struct script_state &st);  // <Agitcheck>
int buildin_flagemblem(struct script_state &st);		// Flag Emblem
int buildin_getcastlename(struct script_state &st);
int buildin_getcastledata(struct script_state &st);
int buildin_setcastledata(struct script_state &st);
int buildin_requestguildinfo(struct script_state &st);
int buildin_getequipcardcnt(struct script_state &st);
int buildin_successremovecards(struct script_state &st);
int buildin_failedremovecards(struct script_state &st);
int buildin_marriage(struct script_state &st);
int buildin_wedding_effect(struct script_state &st);
int buildin_divorce(struct script_state &st);
int buildin_ispartneron(struct script_state &st); // MouseJstr
int buildin_getpartnerid(struct script_state &st); // MouseJstr
int buildin_warppartner(struct script_state &st); // MouseJstr
int buildin_getitemname(struct script_state &st);
int buildin_makepet(struct script_state &st);
int buildin_getexp(struct script_state &st);
int buildin_getinventorylist(struct script_state &st);
int buildin_getskilllist(struct script_state &st);
int buildin_clearitem(struct script_state &st);
int buildin_classchange(struct script_state &st);
int buildin_misceffect(struct script_state &st);
int buildin_soundeffect(struct script_state &st);
int buildin_soundeffectall(struct script_state &st);
int buildin_setcastledata(struct script_state &st);
int buildin_mapwarp(struct script_state &st);
int buildin_inittimer(struct script_state &st);
int buildin_stoptimer(struct script_state &st);
int buildin_cmdothernpc(struct script_state &st);
int buildin_mobcount(struct script_state &st);
int buildin_strmobinfo(struct script_state &st); // Script for displaying mob info [Valaris]
int buildin_guardian(struct script_state &st); // Script for displaying mob info [Valaris]
int buildin_guardianinfo(struct script_state &st); // Script for displaying mob info [Valaris]
int buildin_petskillbonus(struct script_state &st); // petskillbonus [Valaris]
int buildin_petrecovery(struct script_state &st); // pet skill for curing status [Valaris]
int buildin_petloot(struct script_state &st); // pet looting [Valaris]
int buildin_petheal(struct script_state &st); // pet healing [Valaris]
//int buildin_petmag(struct script_state &st); // pet magnificat [Valaris]
int buildin_petskillattack(struct script_state &st); // pet skill attacks [Skotlex]
int buildin_petskillattack2(struct script_state &st); // pet skill attacks [Skotlex]
int buildin_petskillsupport(struct script_state &st); // pet support skill [Valaris]
int buildin_skilleffect(struct script_state &st); // skill effects [Celest]
int buildin_npcskilleffect(struct script_state &st); // skill effects for npcs [Valaris]
int buildin_specialeffect(struct script_state &st); // special effect script [Valaris]
int buildin_specialeffect2(struct script_state &st); // special effect script [Valaris]
int buildin_nude(struct script_state &st); // nude [Valaris]
int buildin_gmcommand(struct script_state &st); // [MouseJstr]
int buildin_atcommand(struct script_state &st); // [MouseJstr]
int buildin_charcommand(struct script_state &st); // [MouseJstr]
int buildin_movenpc(struct script_state &st); // [MouseJstr]
int buildin_message(struct script_state &st); // [MouseJstr]
int buildin_npctalk(struct script_state &st); // [Valaris]
int buildin_hasitems(struct script_state &st); // [Valaris]
int buildin_getlook(struct script_state &st);	//Lorky [Lupus]
int buildin_getsavepoint(struct script_state &st);	//Lorky [Lupus]
int buildin_npcspeed(struct script_state &st); // [Valaris]
int buildin_npcwalkto(struct script_state &st); // [Valaris]
int buildin_npcstop(struct script_state &st); // [Valaris]
int buildin_getmapxy(struct script_state &st);  //get map position for player/npc/pet/mob by Lorky [Lupus]
int buildin_checkoption1(struct script_state &st); // [celest]
int buildin_checkoption2(struct script_state &st); // [celest]
int buildin_guildgetexp(struct script_state &st); // [celest]
int buildin_skilluseid(struct script_state &st); // originally by Qamera [celest]
int buildin_skillusepos(struct script_state &st); // originally by Qamera [celest]
int buildin_logmes(struct script_state &st); // [Lupus]
int buildin_summon(struct script_state &st); // [celest]
int buildin_isnight(struct script_state &st); // [celest]
int buildin_isday(struct script_state &st); // [celest]
int buildin_isequipped(struct script_state &st); // [celest]
int buildin_isequippedcnt(struct script_state &st); // [celest]
int buildin_cardscnt(struct script_state &st); // [Lupus]
int buildin_getrefine(struct script_state &st); // [celest]
int buildin_adopt(struct script_state &st);
int buildin_night(struct script_state &st);
int buildin_day(struct script_state &st);
int buildin_getusersname(struct script_state &st); //jA commands added [Lupus]
int buildin_dispbottom(struct script_state &st);
int buildin_recovery(struct script_state &st);
int buildin_getpetinfo(struct script_state &st);
int buildin_checkequipedcard(struct script_state &st);
int buildin_globalmes(struct script_state &st);
int buildin_jump_zero(struct script_state &st);
int buildin_select(struct script_state &st);
int buildin_getmapmobs(struct script_state &st); //jA addition end
int buildin_getstrlen(struct script_state &st); //strlen [valaris]
int buildin_charisalpha(struct script_state &st);//isalpha [valaris]

int buildin_defpattern(struct script_state &st); // MouseJstr
int buildin_activatepset(struct script_state &st); // MouseJstr
int buildin_deactivatepset(struct script_state &st); // MouseJstr
int buildin_deletepset(struct script_state &st); // MouseJstr

int buildin_unequip(struct script_state &st); // unequip [Spectre]

int buildin_pcstrcharinfo(struct script_state &st);
int buildin_getnameditem(struct script_state &st);

void push_val(struct script_stack &stack,int type,int val);
int run_func(struct script_state &st);

int mapreg_setreg(int num,int val);
int mapreg_setregstr(int num,const char *str);


struct {
	int (*func)(struct script_state &);
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
//	{buildin_petmag,"petmag","iiii"}, // [Valaris]
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
	{buildin_atcommand,"atcommand","*"}, // [MouseJstr]
	{buildin_charcommand,"charcommand","*"}, // [MouseJstr]
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

	{NULL,NULL,NULL},
};

enum {
	C_NOP,C_POS,C_INT,C_PARAM,C_FUNC,C_STR,C_CONSTSTR,C_ARG,
	C_NAME,C_EOL, C_RETINFO,

	C_LOR,C_LAND,C_LE,C_LT,C_GE,C_GT,C_EQ,C_NE,   //operator
	C_XOR,C_OR,C_AND,C_ADD,C_SUB,C_MUL,C_DIV,C_MOD,C_NEG,C_LNOT,C_NOT,C_R_SHIFT,C_L_SHIFT
};

/*==========================================
 * 文字列のハッシュを計算
 *------------------------------------------
 */
unsigned char calc_hash(const char *str)
{
	size_t h=0;
	if(str)
	{	// we need it unsigned here
		const unsigned char *p = (const unsigned char *)str;
		while(*p)
		{
			h=(h<<1)+(h>>3)+(h>>5)+(h>>8);
			h+=*p++;
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
	{
		if(strcasecmp(str_buf+str_data[i].str,p)==0)
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
	// maybe a fixed size buffer would be sufficient
	char *lowcase;

	if(NULL==p) return -1; // should not happen

	lowcase=(char *)aMalloc((strlen(p)+1)*sizeof(char));
	strcpytolower(lowcase, p);
	i=search_str(lowcase);
	aFree(lowcase);

	if(i >= 0)
		return i;

	i=calc_hash(p);
	if(str_hash[i]==0){
		str_hash[i]=str_num;
	} else {
		i=str_hash[i];
		for(;;){
			if(strcmp(str_buf+str_data[i].str,p)==0){
				return i;
			}
			if(str_data[i].next==0)
				break;
			i=str_data[i].next;
		}
		str_data[i].next=str_num;
	}
	if(str_num>=str_data_size){
		str_data_size+=128;
		str_data=(struct s_str_data*)aRealloc(str_data,str_data_size*sizeof(struct s_str_data));
		memset(str_data + (str_data_size - 128), 0, 128*sizeof(struct s_str_data));
	}
	while(str_pos+(int)strlen(p)+1>=str_size){
		str_size+=256;
		str_buf=(char *)aRealloc(str_buf,str_size*sizeof(char));
		memset(str_buf + (str_size - 256), 0, 256*sizeof(char));
	}

	memcpy(str_buf+str_pos,p,strlen(p)+1);
	str_data[str_num].type=C_NOP;
	str_data[str_num].str=str_pos;
	str_data[str_num].next=0;
	str_data[str_num].func=NULL;
	str_data[str_num].backpatch=-1;
	str_data[str_num].label=-1;
	str_pos += (int)strlen(p)+1;
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
		script_size+=SCRIPT_BLOCK_SIZE;
		script_buf=(char *)aRealloc(script_buf,script_size*sizeof(char));
		memset(script_buf + (script_size - SCRIPT_BLOCK_SIZE), 0, SCRIPT_BLOCK_SIZE*sizeof(char));
	}
}

/*==========================================
 * スクリプトバッファに１バイト書き込む
 *------------------------------------------
 */
void add_scriptb(int a)
{
	check_script_buf(1);
	script_buf[script_pos++]=a;
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
	while(a>=0x40){
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
void add_scriptl(int l)
{
	int backpatch = str_data[l].backpatch;

	switch(str_data[l].type){
	case C_POS:
		add_scriptc(C_POS);
		add_scriptb(str_data[l].label);
		add_scriptb(str_data[l].label>>8);
		add_scriptb(str_data[l].label>>16);
		break;
	case C_NOP:
		// ラベルの可能性があるのでbackpatch用データ埋め込み
		add_scriptc(C_NAME);
		str_data[l].backpatch=script_pos;
		add_scriptb(backpatch);
		add_scriptb(backpatch>>8);
		add_scriptb(backpatch>>16);
		break;
	case C_INT:
		add_scripti(str_data[l].val);
		break;
	default:
		// もう他の用途と確定してるので数字をそのまま
		add_scriptc(C_NAME);
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

	str_data[l].type=C_POS;
	str_data[l].label=pos;
	for(i=str_data[l].backpatch;i>=0 && i!=0x00ffffff;){
		next = (0xFF&script_buf[i])
			 | (0xFF&script_buf[i+1])<<8
			 | (0xFF&script_buf[i+2])<<16;
		script_buf[i-1]=C_POS;
		script_buf[i]=pos;
		script_buf[i+1]=pos>>8;
		script_buf[i+2]=pos>>16;
		i=next;
	}
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
	int line,c=0,i;
	char *p,*linestart,*lineend;

	for(line=startline,p=startptr;p && *p;line++){
		linestart=p;
		lineend=strchr(p,'\n');
		if(lineend){
			c=*lineend;
			*lineend=0;
		}
		if(lineend==NULL || pos<lineend){
			ShowMessage("%s line "CL_WHITE"\'%d\'"CL_RESET" : ", mes, line);
			for(i=0;(linestart[i]!='\r') && (linestart[i]!='\n') && linestart[i];i++){
				if(linestart+i!=pos)
					ShowMessage("%c",linestart[i]);
				else
					ShowMessage("\'%c\'",linestart[i]);
			}
			ShowMessage("\a\n");
			if(lineend)
				*lineend=c;
			return;
		}
		*lineend=c;
		p=lineend+1;
	}
}

/*==========================================
 * 項の解析
 *------------------------------------------
 */
char* parse_simpleexpr(char *p)
{
	p=skip_space(p);

#ifdef DEBUG_FUNCIN
	if(battle_config.etc_log)
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
	} else if(isdigit((int)((unsigned char)*p)) || ((*p=='-' || *p=='+') && eapp::isdigit((int)((unsigned char)p[1])))){
		char *np;
		int i=strtoul(p,&np,0);
		add_scripti(i);
		p= np;
	} else if(*p=='"'){
		add_scriptc(C_STR);
		p++;
		while(*p && *p!='"'){
			if(p[-1]<=0x7e && *p=='\\')
				p++;
			else if(*p=='\n'){
				disp_error_message("unexpected newline @ string",p);
				exit(1);
			}
			add_scriptb(*p++);
		}
		if(!*p){
			disp_error_message("unexpected eof @ string",p);
			exit(1);
		}
		add_scriptb(0);
		p++;	//'"'
	} else {
		int c,l;
		char *p2;
		// label , register , function etc
		if(skip_word(p)==p && !(*p==')' && p[-1]=='(')){
			disp_error_message("unexpected character",p);
			exit(1);
		}
		p2 = skip_word(p);
		c=*p2;	*p2=0;	// 名前をadd_strする
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

		if(str_data[l].type!=C_FUNC && c=='['){
			// array(name[i] => getelementofarray(name,i) )
			add_scriptl(search_str("getelementofarray"));
			add_scriptc(C_ARG);
			add_scriptl(l);
			p=parse_subexpr(p+1,-1);
			p=skip_space(p);
			if((*p++)!=']'){
				disp_error_message("unmatch ']'",p);
				exit(1);
			}
			add_scriptc(C_FUNC);
		}else
			add_scriptl(l);

	}

#ifdef DEBUG_FUNCIN
	if(battle_config.etc_log)
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
	if(battle_config.etc_log)
		ShowMessage("parse_subexpr %s\n",p);
#endif
	p=skip_space(p);

	if(*p=='-'){
		tmpp = skip_space(p+1);
		if(*tmpp==';' || *tmpp==','){
			add_scriptl(LABEL_NEXTLINE);
			p++;
			return p;
		}
	}
	tmpp = p;
	if((op=C_NEG,*p=='-') || (op=C_LNOT,*p=='!') || (op=C_NOT,*p=='~')){
		p=parse_subexpr(p+1,100);
		add_scriptc(op);
	} else
		p=parse_simpleexpr(p);
	p=skip_space(p);
	while(((op=C_ADD,opl=6,len=1,*p=='+') ||
		   (op=C_SUB,opl=6,len=1,*p=='-') ||
		   (op=C_MUL,opl=7,len=1,*p=='*') ||
		   (op=C_DIV,opl=7,len=1,*p=='/') ||
		   (op=C_MOD,opl=7,len=1,*p=='%') ||
		   (op=C_FUNC,opl=8,len=1,*p=='(') ||
		   (op=C_LAND,opl=1,len=2,*p=='&' && p[1]=='&') ||
		   (op=C_AND,opl=5,len=1,*p=='&') ||
		   (op=C_LOR,opl=0,len=2,*p=='|' && p[1]=='|') ||
		   (op=C_OR,opl=4,len=1,*p=='|') ||
		   (op=C_XOR,opl=3,len=1,*p=='^') ||
		   (op=C_EQ,opl=2,len=2,*p=='=' && p[1]=='=') ||
		   (op=C_NE,opl=2,len=2,*p=='!' && p[1]=='=') ||
		   (op=C_R_SHIFT,opl=5,len=2,*p=='>' && p[1]=='>') ||
		   (op=C_GE,opl=2,len=2,*p=='>' && p[1]=='=') ||
		   (op=C_GT,opl=2,len=1,*p=='>') ||
		   (op=C_L_SHIFT,opl=5,len=2,*p=='<' && p[1]=='<') ||
		   (op=C_LE,opl=2,len=2,*p=='<' && p[1]=='=') ||
		   (op=C_LT,opl=2,len=1,*p=='<')) && opl>limit){
		p+=len;
		if(op==C_FUNC){
			int i=0,func=parse_cmd;
			const char *plist[128];

			if( str_data[func].type!=C_FUNC ){
				disp_error_message("expect function",tmpp);
				exit(0);
			}

			add_scriptc(C_ARG);
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

			if (str_data[func].type == C_FUNC && script_config.warn_func_mismatch_paramnum) {
				const char *arg = buildin_func[str_data[func].val].arg;
				int j = 0;
				for (; arg[j]; j++) if (arg[j] == '*') break;
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
	if(battle_config.etc_log)
		ShowMessage("parse_subexpr end %s\n",p);
#endif
	return p;  /* return first untreated operator */
}

/*==========================================
 * 式の評価
 *------------------------------------------
 */
char* parse_expr(char *p)
{
#ifdef DEBUG_FUNCIN
	if(battle_config.etc_log)
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
	if(battle_config.etc_log)
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
		return p;

	parse_cmd_if=0;	// warn_cmd_no_commaのために必要

	// 最初は関数名
	p2 = p;
	p=parse_simpleexpr(p);
	p=skip_space(p);

	cmd=parse_cmd;
	if( str_data[cmd].type!=C_FUNC ){
		disp_error_message("expect command", p2);
//		exit(0);
	}

	add_scriptc(C_ARG);
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
	add_scriptc(C_FUNC);

	if( str_data[cmd].type==C_FUNC && script_config.warn_cmd_mismatch_paramnum){
		const char *arg=buildin_func[str_data[cmd].val].arg;
		int j=0;
		for(j=0;arg[j];j++) if(arg[j]=='*')break;
		if( (arg[j]==0 && i!=j) || (arg[j]=='*' && i<j) ){
			disp_error_message("illegal number of parameters",plist[(i<j)?i:j]);
		}
	}


	return p;
}

/*==========================================
 * 組み込み関数の追加
 *------------------------------------------
 */
void add_buildin_func(void)
{
	int i,n;
	for(i=0;buildin_func[i].func;i++){
		n=add_str(buildin_func[i].name);
		str_data[n].type=C_FUNC;
		str_data[n].val=i;
		str_data[n].func=buildin_func[i].func;
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

	fp=savefopen("db/const.txt","r");
	if(fp==NULL){
		ShowMessage("can't read %s\n","db/const.txt");
		return ;
	}
	while(fgets(line,1020,fp)){
		if( !skip_empty_line(line) )
			continue;
		type=0;
		if(sscanf(line,"%[A-Za-z0-9_],%d,%d",name,&val,&type)>=2 ||
		   sscanf(line,"%[A-Za-z0-9_] %d %d",name,&val,&type)>=2)
		{
			tolower(name);
			n=add_str(name);
			if(type==0)
				str_data[n].type=C_INT;
			else
				str_data[n].type=C_PARAM;
			str_data[n].val=val;
		}
	}
	fclose(fp);
}



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
 * スクリプトの解析
 *------------------------------------------
 */
char* parse_script(unsigned char *src,int line)
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


	if(script_buf) aFree(script_buf);
	script_buf=(char *)aCallocA(SCRIPT_BLOCK_SIZE,sizeof(char));

	script_pos = 0;
	script_size = SCRIPT_BLOCK_SIZE;
	str_data[LABEL_NEXTLINE].type = C_NOP;
	str_data[LABEL_NEXTLINE].backpatch = -1;
	str_data[LABEL_NEXTLINE].label = -1;
	for (i = LABEL_START; i < str_num; i++) {
		if (str_data[i].type == C_POS || str_data[i].type == C_NAME) {
			str_data[i].type = C_NOP;
			str_data[i].backpatch = -1;
			str_data[i].label = -1;
		}
	}

	// 外部用label dbの初期化
	if (scriptlabel_db)
		strdb_final (scriptlabel_db, scriptlabel_final);
	scriptlabel_db = strdb_init(50);

	// for error message
	startptr = (char*)src;
	startline = line;

	////////
	// input check is above
	while(p && *p && *p!='}')
	{
		p = skip_space(p);
		// labelだけ特殊処理
		tmpp = skip_space(skip_word(p));
		if(*tmpp==':')
		{
			int l, c;
			c = *skip_word(p);
			*skip_word(p) = 0;
			l = add_str(p);
			if (str_data[l].label != -1) {
				*skip_word(p) = c;
				disp_error_message("dup label ", p);
				exit(1);
			}
			set_label(l, script_pos);
			strdb_insert(scriptlabel_db,p,script_pos);
			*skip_word(p) = c;
			p = tmpp + 1;
		}
		else
		{	// 他は全部一緒くた
			p = parse_line(p);
			p = skip_space(p);
			add_scriptc(C_EOL);
			set_label(LABEL_NEXTLINE, script_pos);
			str_data[LABEL_NEXTLINE].type = C_NOP;
			str_data[LABEL_NEXTLINE].backpatch = -1;
			str_data[LABEL_NEXTLINE].label = -1;
		}
	}

	add_scriptc(C_NOP);
	script_size = script_pos;
	script_buf=(char*)aRealloc(script_buf,(script_pos + 1)*sizeof(char));

	// 未解決のラベルを解決
	for (i = LABEL_START; i < str_num; i++) {
		if (str_data[i].type == C_NOP) {
			int j, next;
			str_data[i].type = C_NAME;
			str_data[i].label = i;
			for (j = str_data[i].backpatch; j >= 0 && j != 0x00ffffff; ) {
				next = (0xFF&script_buf[j])
					 | (0xFF&script_buf[j+1])<<8
					 | (0xFF&script_buf[j+2])<<16;
				script_buf[j] = i;
				script_buf[j+1] = i>>8;
				script_buf[j+2] = i>>16;
				j = next;
			}
		}
	}

#ifdef DEBUG_DISP
	for (i = 0; i < script_pos; i++) {
		if((i&15)==0) ShowMessage("%04x : ",i);
		ShowMessage("%02x ", 0xFF&script_buf[i]);
		if((i&15)==15) ShowMessage("\n");
	}
	ShowMessage("\n");
#endif

	char *local = script_buf;
	script_buf = NULL;
	return local;
}

//
// 実行系
//
enum {STOP=1,END,RERUNLINE,GOTO,RETFUNC};

/*==========================================
 * ridからsdへの解決
 *------------------------------------------
 */
struct map_session_data *script_rid2sd(struct script_state &st)
{
	struct map_session_data *sd=map_id2sd(st.rid);
	if(!sd)
		ShowError("script_rid2sd: fatal error ! player not attached!\n");
	return sd;
}


/*==========================================
 * 変数の読み取り
 *------------------------------------------
 */
int get_val(struct script_state &st, struct script_data &data)
{
	if(data.type==C_NAME)
	{
		char *name=str_buf+str_data[data.u.num&0x00ffffff].str;
		char prefix=*name;
		char postfix=name[strlen(name)-1];

		if(postfix=='$')
		{
			data.type=C_CONSTSTR;
			if( prefix=='@'/* || prefix=='l' */)
			{
				struct map_session_data *sd=script_rid2sd(st);
				if(sd!=NULL)
					data.u.str = pc_readregstr(*sd,data.u.num);
			}
			else if(prefix=='$')
			{
				data.u.str = (char *)numdb_search(mapregstr_db,data.u.num);
			}
			else
			{
				ShowError("script: get_val: illegal scope string variable.\n");
				data.u.str = "!!ERROR!!";
			}
			if( data.u.str == NULL )
				data.u.str = "";
		}
		else
		{
			data.type=C_INT;
			if(str_data[data.u.num&0x00ffffff].type==C_INT)
			{
				data.u.num = str_data[data.u.num&0x00ffffff].val;
			}
			else if(str_data[data.u.num&0x00ffffff].type==C_PARAM)
			{
				struct map_session_data *sd=script_rid2sd(st);
				if(sd!=NULL)
					data.u.num = pc_readparam(*sd,str_data[data.u.num&0x00ffffff].val);
			}
			else if(prefix=='@'/* || prefix=='l'*/)
			{
				struct map_session_data *sd=script_rid2sd(st);
				if(sd!=NULL)
					data.u.num = pc_readreg(*sd,data.u.num);
			}
			else if(prefix=='$')
			{
				data.u.num = (int)numdb_search(mapreg_db,data.u.num);
			}
			else if(prefix=='#')
			{
				struct map_session_data *sd=script_rid2sd(st);
				if(sd!=NULL)
				{
					if( name[1]=='#')
						data.u.num = pc_readaccountreg2(*sd,name);
					else
						data.u.num = pc_readaccountreg(*sd,name);
				}
			}
			else
			{
				struct map_session_data *sd=script_rid2sd(st);
				if(sd!=NULL)
					data.u.num = pc_readglobalreg(*sd,name);
			}
		}
	}
	return 0;
}
/*==========================================
 * 変数の読み取り2
 *------------------------------------------
 */
void* get_val2(struct script_state &st,int num)
{
	struct script_data dat;
	dat.type=C_NAME;
	dat.u.num=num;
	get_val(st,dat);
	if( dat.type==C_INT )
		return (void*)dat.u.num;
	else 
		return (void*)dat.u.str;
}

/*==========================================
 * 変数設定用
 *------------------------------------------
 */

int set_var(const char *name, void *v)
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
				mapreg_setregstr(num,(char*)v);
			}
			else
			{
				mapreg_setreg(num,(int)v);
			}
		}
		else
		{
			ShowError("script: set_var: illegal scope string variable !");
		}
	}
    return 0;
}


int set_reg(struct script_state &st,int num,const char *name, void *v)
{
	if(name)
	{
		struct map_session_data *sd = NULL;
		char prefix =name[0];
		char postfix=name[strlen(name)-1];

		if( postfix=='$' )
		{	// string variable
		char *str=(char*)v;
			if( prefix=='@' || prefix=='l')
			{
				sd = script_rid2sd(st);
				if(sd==NULL)
					ShowError("set_reg error name?:%s\n",name);
				else
					pc_setregstr(*sd,num,str);
			}
			else if(prefix=='$')
			{
			mapreg_setregstr(num,str);
		}
			else
			{
				ShowError("script: set_reg: illegal scope string variable !");
			}
		}
		else 
		{	// 数値
		int val = (int)v;
			if(str_data[num&0x00ffffff].type==C_PARAM)
			{
				if(sd) pc_setparam(*sd,str_data[num&0x00ffffff].val,val);
			}
			else if(prefix=='@' || prefix=='l')
			{
				sd=script_rid2sd(st);
				if(sd==NULL)
					ShowError("set_reg error name?:%s\n",name);
				else
					pc_setreg(*sd,num,val);
			}
			else if(prefix=='$')
			{
			mapreg_setreg(num,val);
			}
			else if(prefix=='#')
			{
				sd=script_rid2sd(st);
				if(sd==NULL)
					ShowError("set_reg error name?:%s\n",name);
				else
				{
			if( name[1]=='#' )
						pc_setaccountreg2(*sd,name,val);
			else
						pc_setaccountreg(*sd,name,val);
		}
	}
			else
			{
				sd=script_rid2sd(st);
				if(sd==NULL)
					ShowError("set_reg error name?:%s\n",name);
				else
					pc_setglobalreg(*sd,name,val);
			}
		}
	}
	return 0;
}


/*==========================================
 * 文字列への変換
 *------------------------------------------
 */
const char* conv_str(struct script_state &st,struct script_data &data)
{
	get_val(st,data);
	if(data.type==C_INT)
	{
		char *buf;
		buf=(char *)aMalloc(16*sizeof(char));
		sprintf(buf,"%d",data.u.num);
		data.type=C_STR;
		data.u.str=buf;
	}
	else if(data.type==C_NAME)
	{	// テンポラリ。本来無いはず
		data.type=C_CONSTSTR;
		data.u.str=str_buf+str_data[data.u.num].str;
}
	return data.u.str;
}

/*==========================================
 * 数値へ変換
 *------------------------------------------
 */
int conv_num(struct script_state &st,struct script_data &data)
{
	const char *p;
	get_val(st,data);
	if(data.type==C_STR || data.type==C_CONSTSTR){
		p=data.u.str;
		data.u.num = atoi(p);
		if(data.type==C_STR)
			aFree((void*)p);
		data.type=C_INT;
	}
	return data.u.num;
}

/*==========================================
 * スタックへ数値をプッシュ
 *------------------------------------------
 */
void push_val(struct script_stack &stack,int type,int val)
{
	if((size_t)stack.sp >= stack.sp_max){
		stack.sp_max += 64;
		stack.stack_data = (struct script_data *)aRealloc(stack.stack_data, stack.sp_max*sizeof(struct script_data) );
		memset(stack.stack_data + (stack.sp_max - 64), 0, 64 * sizeof(struct script_data));
	}
//	if(battle_config.etc_log)
//		ShowMessage("push (%d,%d)-> %d\n",type,val,stack.sp);
	stack.stack_data[stack.sp].type=type;
	stack.stack_data[stack.sp].u.num=val;
	stack.sp++;
}

/*==========================================
 * スタックへ文字列をプッシュ
 *------------------------------------------
 */
void push_str(struct script_stack &stack, int type, const char *str)
{
	if((size_t)stack.sp >= stack.sp_max){
		stack.sp_max += 64;
		stack.stack_data = (struct script_data *)aRealloc(stack.stack_data, stack.sp_max * sizeof(struct script_data));
		memset(stack.stack_data + (stack.sp_max - 64), 0, 64 * sizeof(struct script_data));
	}
//	if(battle_config.etc_log)
//		ShowMessage("push (%d,%x)-> %d\n",type,str,stack.sp);
	stack.stack_data[stack.sp].type=type;
	stack.stack_data[stack.sp].u.str=str;
	stack.sp++;
}

/*==========================================
 * スタックへ複製をプッシュ
 *------------------------------------------
 */
void push_copy(struct script_stack &stack,int pos)
{
	switch(stack.stack_data[pos].type){
	case C_CONSTSTR:
		push_str(stack,C_CONSTSTR,stack.stack_data[pos].u.str);
		break;
	case C_STR:
		push_str(stack,C_STR, aStrdup(stack.stack_data[pos].u.str));
		break;
	default:
		push_val(stack,stack.stack_data[pos].type,stack.stack_data[pos].u.num);
		break;
	}
}

/*==========================================
 * スタックからポップ
 *------------------------------------------
 */
void pop_stack(struct script_stack& stack,int start,int end)
{
	int i;
	for(i=start;i<end;i++){
		if(stack.stack_data[i].type==C_STR)
		{
			aFree( (void*)(stack.stack_data[i].u.str) );
		}
	}
	if(stack.sp>end){
		memmove(&stack.stack_data[start],&stack.stack_data[end],sizeof(struct script_data)*(stack.sp-end));
	}
	stack.sp -= end-start;
}

//
// 埋め込み関数
//
/*==========================================
 *
 *------------------------------------------
 */
int buildin_mes(struct script_state &st)
{
	conv_str(st,(st.stack.stack_data[st.start+2]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) clif_scriptmes(*sd,st.oid,st.stack.stack_data[st.start+2].u.str);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_goto(struct script_state &st)
{
	int pos;

	if (st.stack.stack_data[st.start+2].type != C_POS){
		int func = st.stack.stack_data[st.start+2].u.num;
		ShowMessage("script: goto '"CL_WHITE"%s"CL_RESET"': not label!\n", str_buf + str_data[func].str);
		st.state = END;
		return 0;
	}

	pos = conv_num(st, (st.stack.stack_data[st.start+2]));
	st.pos = pos;
	st.state = GOTO;
	return 0;
}

/*==========================================
 * ユーザー定義関数の呼び出し
 *------------------------------------------
 */
int buildin_callfunc(struct script_state &st)
{
	char *scr;
	const char *str=conv_str(st,(st.stack.stack_data[st.start+2]));

	if( (scr=(char *) strdb_search(script_get_userfunc_db(),str)) ){
		size_t i,j;
		for(i=st.start+3,j=0;i<st.end;i++,j++)
			push_copy(st.stack,i);

		push_val(st.stack,C_INT,j);				// 引数の数をプッシュ
		push_val(st.stack,C_INT,st.defsp);	// 現在の基準スタックポインタをプッシュ
		push_val(st.stack,C_INT,(int)st.script);	// 現在のスクリプトをプッシュ
		push_val(st.stack,C_RETINFO,st.pos);		// 現在のスクリプト位置をプッシュ

		st.pos=0;
		st.script=scr;
		st.defsp=st.start+4+j;
		st.state=GOTO;
	}else{
		ShowMessage("script:callfunc: function not found! [%s]\n",str);
		st.state=END;
	}
	return 0;
}
/*==========================================
 * サブルーティンの呼び出し
 *------------------------------------------
 */
int buildin_callsub(struct script_state &st)
{
	int pos=conv_num(st, (st.stack.stack_data[st.start+2]));
	size_t i,j;
	for(i=st.start+3,j=0;i<st.end;i++,j++)
		push_copy(st.stack,i);

	push_val(st.stack,C_INT,j);				// 引数の数をプッシュ
	push_val(st.stack,C_INT,st.defsp);	// 現在の基準スタックポインタをプッシュ
	push_val(st.stack,C_INT,(int)st.script);	// 現在のスクリプトをプッシュ
	push_val(st.stack,C_RETINFO,st.pos);		// 現在のスクリプト位置をプッシュ

	st.pos=pos;
	st.defsp=st.start+4+j;
	st.state=GOTO;
	return 0;
}

/*==========================================
 * 引数の所得
 *------------------------------------------
 */
int buildin_getarg(struct script_state &st)
{
	int num=conv_num(st, (st.stack.stack_data[st.start+2]));
	int max,stsp;
	if( st.defsp<4 || st.stack.stack_data[st.defsp-1].type!=C_RETINFO ){
		ShowError("script:getarg without callfunc or callsub!\n");
		st.state=END;
		return 0;
	}
	max=conv_num(st, (st.stack.stack_data[st.defsp-4]));
	stsp=st.defsp - max -4;
	if( num >= max ){
		ShowError("script:getarg arg1(%d) out of range(%d) !\n",num,max);
		st.state=END;
		return 0;
	}
	push_copy(st.stack,stsp+num);
	return 0;
}

/*==========================================
 * サブルーチン/ユーザー定義関数の終了
 *------------------------------------------
 */
int buildin_return(struct script_state &st)
{
	if(st.end>st.start+2){	// 戻り値有り
		push_copy(st.stack,st.start+2);
	}
	st.state=RETFUNC;
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_next(struct script_state &st)
{
	st.state=STOP;
	map_session_data *sd = script_rid2sd(st);
	if(sd) clif_scriptnext(*sd,st.oid);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_close(struct script_state &st)
{
	st.state=END;
	map_session_data *sd = script_rid2sd(st);
	if(sd) clif_scriptclose(*sd,st.oid);
	return 0;
}
int buildin_close2(struct script_state &st)
{
	st.state=STOP;
	map_session_data *sd = script_rid2sd(st);
	if(sd) clif_scriptclose(*sd,st.oid);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_menu(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);

	if(!sd)
		return 0;

	if(sd->state.menu_or_input==0)
	{
		size_t len,i;
		char *buf;
		st.state=RERUNLINE;
		sd->state.menu_or_input=1;
		for(i=st.start+2,len=16;i<st.end;i+=2){
			conv_str(st,(st.stack.stack_data[i]));
			len+=strlen(st.stack.stack_data[i].u.str)+1;
		}
		buf=(char *)aMalloc((len+1)*sizeof(char));
		buf[0]=0;
		for(i=st.start+2,len=0;i<st.end;i+=2){
			strcat(buf,st.stack.stack_data[i].u.str);
			strcat(buf,":");
		}
		clif_scriptmenu(*sd,st.oid,buf);
		aFree(buf);
	}
	else if(sd->npc_menu==0xff)
	{	// cansel
		sd->state.menu_or_input=0;
		st.state=END;
	}
	else
	{	// goto動作
		// ragemu互換のため
		pc_setreg(*sd,add_str("l15"),sd->npc_menu);
		pc_setreg(*sd,add_str("@menu"),sd->npc_menu);
		sd->state.menu_or_input=0;
		if(sd->npc_menu>0 && sd->npc_menu<(st.end-st.start)/2){
			size_t pos;
			if( st.stack.stack_data[st.start+sd->npc_menu*2+1].type!=C_POS ){
				ShowMessage("script: menu: not label !\n");
				st.state=END;
				return 0;
			}
			pos=conv_num(st, (st.stack.stack_data[st.start+sd->npc_menu*2+1]));
			st.pos=pos;
			st.state=GOTO;
		}
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_rand(struct script_state &st)
{
	int range;

	if (st.end > st.start+3){
		int min, max;
		min = conv_num(st, (st.stack.stack_data[st.start+2]));
		max = conv_num(st, (st.stack.stack_data[st.start+3]));
		if (max < min){
			int tmp = min;
			min = max;
			max = tmp;
		}
		range = max - min + 1;
		if (range == 0) range = 1;
		push_val(st.stack,C_INT,rand()%range+min);
	} else {
		range = conv_num(st, (st.stack.stack_data[st.start+2]));
		if (range == 0) range = 1;
		push_val(st.stack,C_INT,rand()%range);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_warp(struct script_state &st)
{
	int x,y;
	const char *str;
	struct map_session_data *sd=script_rid2sd(st);

	str=conv_str(st,(st.stack.stack_data[st.start+2]));
	x=conv_num(st, (st.stack.stack_data[st.start+3]));
	y=conv_num(st, (st.stack.stack_data[st.start+4]));

	if(sd==NULL || str==NULL)
		return 0;

	if( 0==strcmp(str,"Random") )
	{
		pc_randomwarp(*sd,3);
	}
	else if( 0==strcmp(str,"SavePoint") )
	{
		if(map[sd->bl.m].flag.noreturn)	// 蝶禁止
			return 0;
		pc_setpos(*sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
	}
	else if( 0==strcmp(str,"Save") )
	{
		if(map[sd->bl.m].flag.noreturn)	// 蝶禁止
			return 0;
		pc_setpos(*sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
	}
	else
	{
		pc_setpos(*sd,str,x,y,0);
	}
	return 0;
}
/*==========================================
 * エリア指定ワープ
 *------------------------------------------
 */
int buildin_areawarp_sub(struct block_list &bl,va_list ap)
{
	int x,y;
	char *map;
	map=va_arg(ap, char *);
	x=va_arg(ap,int);
	y=va_arg(ap,int);
	if(strcmp(map,"Random")==0)
		pc_randomwarp(((struct map_session_data &)bl),3);
	else
		pc_setpos(((struct map_session_data &)bl),map,x,y,0);
	return 0;
}
int buildin_areawarp(struct script_state &st)
{
	int x,y,m;
	const char *str;
	const char *mapname;
	int x0,y0,x1,y1;

	mapname=conv_str(st,(st.stack.stack_data[st.start+2]));
	x0=conv_num(st, (st.stack.stack_data[st.start+3]));
	y0=conv_num(st, (st.stack.stack_data[st.start+4]));
	x1=conv_num(st, (st.stack.stack_data[st.start+5]));
	y1=conv_num(st, (st.stack.stack_data[st.start+6]));
	str=conv_str(st,(st.stack.stack_data[st.start+7]));
	x=conv_num(st, (st.stack.stack_data[st.start+8]));
	y=conv_num(st, (st.stack.stack_data[st.start+9]));

	if( (m=map_mapname2mapid(mapname))< 0)
		return 0;
//!! broadcast command if not on this mapserver

	map_foreachinarea(buildin_areawarp_sub,
		m,x0,y0,x1,y1,BL_PC,	str,x,y );
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_heal(struct script_state &st)
{
	int hp,sp;

	hp=conv_num(st, (st.stack.stack_data[st.start+2]));
	sp=conv_num(st, (st.stack.stack_data[st.start+3]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) pc_heal(*sd,hp,sp);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_itemheal(struct script_state &st)
{
	int hp,sp;

	hp=conv_num(st, (st.stack.stack_data[st.start+2]));
	sp=conv_num(st, (st.stack.stack_data[st.start+3]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) pc_itemheal(*sd,hp,sp);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_percentheal(struct script_state &st)
{
	int hp,sp;

	hp=conv_num(st, (st.stack.stack_data[st.start+2]));
	sp=conv_num(st, (st.stack.stack_data[st.start+3]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) pc_percentheal(*sd,hp,sp);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_jobchange(struct script_state &st)
{
	int job, upper=-1;

	job=conv_num(st, (st.stack.stack_data[st.start+2]));
	if( st.end>st.start+3 )
		upper=conv_num(st, (st.stack.stack_data[st.start+3]));

	map_session_data *sd = script_rid2sd(st);
	if( sd && job>=0 && job<MAX_PC_CLASS )
		pc_jobchange(*sd,job, upper);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_input(struct script_state &st)
{
	int num=(st.end>st.start+2)?st.stack.stack_data[st.start+2].u.num:0;
	char *name=(st.end>st.start+2)?str_buf+str_data[num&0x00ffffff].str:(char*)"";
//	char prefix=*name;
	char postfix=name[strlen(name)-1];
	struct map_session_data *sd=script_rid2sd(st);
	if(sd)
	{
		if(sd->state.menu_or_input)
		{
		sd->state.menu_or_input=0;
			if( postfix=='$' )
			{
			// 文字列
				if(st.end>st.start+2){ // 引数1個
					set_reg(st,num,name,(void*)sd->npc_str);
			}else{
					ShowError("buildin_input: string discarded !!\n");
			}
		}else{
			// 数値
				if(st.end>st.start+2){ // 引数1個
					set_reg(st,num,name,(void*)sd->npc_amount);
			} else {
				// ragemu互換のため
					pc_setreg(*sd,add_str( "l14"),sd->npc_amount);
			}
		}
	} else {
			st.state=RERUNLINE;
			if(postfix=='$')clif_scriptinputstr(*sd,st.oid);
			else			clif_scriptinput(*sd,st.oid);
		sd->state.menu_or_input=1;
	}
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_if(struct script_state &st)
{
	size_t sel,i;

	sel=conv_num(st, (st.stack.stack_data[st.start+2]));
	if(!sel)
		return 0;

	// 関数名をコピー
	push_copy(st.stack,st.start+3);
	// 間に引数マーカを入れて
	push_val(st.stack,C_ARG,0);
	// 残りの引数をコピー
	for(i=st.start+4;i<st.end;i++){
		push_copy(st.stack,i);
	}
	run_func(st);

	return 0;
}


/*==========================================
 * 変数設定
 *------------------------------------------
 */
int buildin_set(struct script_state &st)
{
	int num=st.stack.stack_data[st.start+2].u.num;
	char *name=str_buf+str_data[num&0x00ffffff].str;
	char postfix=name[strlen(name)-1];

	if( st.stack.stack_data[st.start+2].type!=C_NAME ){
		ShowMessage("script: buildin_set: not name\n");
		return 0;
	}

	if( postfix=='$' ){
		// 文字列
		const char *str = conv_str(st,(st.stack.stack_data[st.start+3]));
		set_reg(st,num,name,(void*)str);
	}else{
		// 数値
		int val = conv_num(st, (st.stack.stack_data[st.start+3]));
		set_reg(st,num,name,(void*)val);
	}

	return 0;
}
/*==========================================
 * 配列変数設定
 *------------------------------------------
 */
int buildin_setarray(struct script_state &st)
{
	int num=st.stack.stack_data[st.start+2].u.num;
	char *name=str_buf+str_data[num&0x00ffffff].str;
	char prefix=*name;
	char postfix=name[strlen(name)-1];
	size_t i,j;

	if( prefix!='$' && prefix!='@' ){
		ShowMessage("buildin_setarray: illegal scope !\n");
		return 0;
	}

	for(j=0,i=st.start+3; i<st.end && j<128;i++,j++){
		void *v;
		if( postfix=='$' )
			v=(void*)conv_str(st,(st.stack.stack_data[i]));
		else
			v=(void*)conv_num(st, (st.stack.stack_data[i]));
		set_reg(st, num+(j<<24), name, v);
	}
	return 0;
}
/*==========================================
 * 配列変数クリア
 *------------------------------------------
 */
int buildin_cleararray(struct script_state &st)
{
	int num=st.stack.stack_data[st.start+2].u.num;
	char *name=str_buf+str_data[num&0x00ffffff].str;
	char prefix=*name;
	char postfix=name[strlen(name)-1];
	int sz=conv_num(st, (st.stack.stack_data[st.start+4]));
	int i;
	void *v;

	if( prefix!='$' && prefix!='@' ){
		ShowMessage("buildin_cleararray: illegal scope !\n");
		return 0;
	}

	if( postfix=='$' )
		v=(void*)conv_str(st,(st.stack.stack_data[st.start+3]));
	else
		v=(void*)conv_num(st, (st.stack.stack_data[st.start+3]));

	for(i=0;i<sz;i++)
		set_reg(st,num+(i<<24),name,v);
	return 0;
}
/*==========================================
 * 配列変数コピー
 *------------------------------------------
 */
int buildin_copyarray(struct script_state &st)
{
	int num=st.stack.stack_data[st.start+2].u.num;
	char *name=str_buf+str_data[num&0x00ffffff].str;
	char prefix=*name;
	char postfix=name[strlen(name)-1];
	int num2=st.stack.stack_data[st.start+3].u.num;
	char *name2=str_buf+str_data[num2&0x00ffffff].str;
	char prefix2=*name2;
	char postfix2=name2[strlen(name2)-1];
	int sz=conv_num(st, (st.stack.stack_data[st.start+4]));
	int i;

	if( prefix!='$' && prefix!='@' && prefix2!='$' && prefix2!='@' ){
		ShowMessage("buildin_copyarray: illegal scope !\n");
		return 0;
	}
	if( (postfix=='$' || postfix2=='$') && postfix!=postfix2 ){
		ShowMessage("buildin_copyarray: type mismatch !\n");
		return 0;
	}

	for(i=0;i<sz;i++)
		set_reg(st,num+(i<<24),name, get_val2(st,num2+(i<<24)) );
	return 0;
}
/*==========================================
 * 配列変数のサイズ所得
 *------------------------------------------
 */
int getarraysize(struct script_state &st,int num,int postfix)
{
	int i=(num>>24),c=i;
	for(;i<128;i++){
		void *v=get_val2(st,num+(i<<24));
		if(postfix=='$' && *((char*)v) ) c=i;
		if(postfix!='$' && (int)v )c=i;
	}
	return c+1;
}
int buildin_getarraysize(struct script_state &st)
{
	int num=st.stack.stack_data[st.start+2].u.num;
	char *name=str_buf+str_data[num&0x00ffffff].str;
	char prefix=*name;
	char postfix=name[strlen(name)-1];

	if( prefix!='$' && prefix!='@' ){
		ShowMessage("buildin_copyarray: illegal scope !\n");
		return 0;
	}

	push_val(st.stack,C_INT,getarraysize(st,num,postfix) );
	return 0;
}
/*==========================================
 * 配列変数から要素削除
 *------------------------------------------
 */
int buildin_deletearray(struct script_state &st)
{
	int num=st.stack.stack_data[st.start+2].u.num;
	char *name=str_buf+str_data[num&0x00ffffff].str;
	char prefix=*name;
	char postfix=name[strlen(name)-1];
	int count=1;
	int i,sz=getarraysize(st,num,postfix)-(num>>24)-count+1;


	if( (st.end > st.start+3) )
		count=conv_num(st, (st.stack.stack_data[st.start+3]));

	if( prefix!='$' && prefix!='@' ){
		ShowMessage("buildin_deletearray: illegal scope !\n");
		return 0;
	}

	for(i=0;i<sz;i++){
		set_reg(st,num+(i<<24),name, get_val2(st,num+((i+count)<<24) ) );
	}
	for(;i<(128-(num>>24));i++){
		if( postfix!='$' ) set_reg(st,num+(i<<24),name, 0);
		if( postfix=='$' ) set_reg(st,num+(i<<24),name, (void *) "");
	}
	return 0;
}

/*==========================================
 * 指定要素を表す値(キー)を所得する
 *------------------------------------------
 */
int buildin_getelementofarray(struct script_state &st)
{
	if( st.stack.stack_data[st.start+2].type==C_NAME ){
		int i=conv_num(st, (st.stack.stack_data[st.start+3]));
		if(i>127 || i<0){
			ShowMessage("script: getelementofarray (operator[]): param2 illegal number %d\n",i);
			push_val(st.stack,C_INT,0);
		}else{
			push_val(st.stack,C_NAME,
				(i<<24) | st.stack.stack_data[st.start+2].u.num );
		}
	}else{
		ShowMessage("script: getelementofarray (operator[]): param1 not name !\n");
		push_val(st.stack,C_INT,0);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_setlook(struct script_state &st)
{
	int type,val;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	val=conv_num(st, (st.stack.stack_data[st.start+3]));

	struct map_session_data *sd = script_rid2sd(st);
	if(sd) pc_changelook(*sd,type,val);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_cutin(struct script_state &st)
{
	int type;

	conv_str(st,(st.stack.stack_data[st.start+2]));
	type=conv_num(st, (st.stack.stack_data[st.start+3]));

	struct map_session_data *sd = script_rid2sd(st);
	if(sd) clif_cutin(*sd,st.stack.stack_data[st.start+2].u.str,type);

	return 0;
}
/*==========================================
 * カードのイラストを表示する
 *------------------------------------------
 */
int buildin_cutincard(struct script_state &st)
{
	int itemid =conv_num(st, (st.stack.stack_data[st.start+2]));
	map_session_data *sd = script_rid2sd(st);
	if(sd)
	{	struct item_data* idata = itemdb_exists(itemid);
		if(idata) clif_cutin(*sd,idata->cardillustname,4);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_viewpoint(struct script_state &st)
{
	int type,x,y,id,color;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	x=conv_num(st, (st.stack.stack_data[st.start+3]));
	y=conv_num(st, (st.stack.stack_data[st.start+4]));
	id=conv_num(st, (st.stack.stack_data[st.start+5]));
	color=conv_num(st, (st.stack.stack_data[st.start+6]));

	map_session_data *sd = script_rid2sd(st);
	if(sd) clif_viewpoint(*sd,st.oid,type,x,y,id,color);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_countitem(struct script_state &st)
{
	unsigned short nameid=0;
	size_t count=0,i;
	struct map_session_data *sd = script_rid2sd(st);
	if(sd)
	{
	struct script_data *data;
		data=&(st.stack.stack_data[st.start+2]);

		get_val(st,*data);
		if( data->type==C_STR || data->type==C_CONSTSTR )
		{
			const char *name=conv_str(st,*data);
		struct item_data *item_data;
		if( (item_data = itemdb_searchname(name)) != NULL)
			nameid=item_data->nameid;
		}
		else
			nameid=conv_num(st,*data);

	if (nameid>=500) //if no such ID then skip this iteration
			for(i=0;i<MAX_INVENTORY;i++)
			{
			if(sd->status.inventory[i].nameid==nameid)
				count+=sd->status.inventory[i].amount;
		}
		else
		{
		if(battle_config.error_log)
				ShowMessage("wrong item ID : countitem(%i)\n",nameid);
	}
	}
	push_val(st.stack,C_INT,count);

	return 0;
}

/*==========================================
 * 重量チェック
 *------------------------------------------
 */
int buildin_checkweight(struct script_state &st)
{
	int val = 0;
	unsigned short nameid=0,amount;
	struct script_data &data=(st.stack.stack_data[st.start+2]);
	struct map_session_data *sd = script_rid2sd(st);

	if(sd)
	{
	get_val(st,data);
		if( data.type==C_STR || data.type==C_CONSTSTR )
		{
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid=item_data->nameid;
		}
		else
		nameid=conv_num(st,data);

		amount=conv_num(st, (st.stack.stack_data[st.start+3]));
		if( amount<MAX_AMOUNT && nameid>=500 && nameid<MAX_ITEMS)
		{
			if( itemdb_weight(nameid)*amount + sd->weight <= sd->max_weight )
			{
				val = 1;
	}
	}
	}
	push_val(st.stack,C_INT,val);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_getitem(struct script_state &st)
{
	//!! some stupid game with signs
	short nameid,nameidsrc,amount;
	int flag = 0;
	struct item item_tmp;
	struct script_data *data;
	struct map_session_data *sd = script_rid2sd(st);

	data=&(st.stack.stack_data[st.start+2]);
	get_val(st,*data);
	if( data->type==C_STR || data->type==C_CONSTSTR ){
		const char *name=conv_str(st,*data);
		struct item_data *item_data = itemdb_searchname(name);
		nameid=512; //Apple item ID
		if( item_data != NULL)
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,*data);

	if ( ( amount=conv_num(st, (st.stack.stack_data[st.start+3])) ) <= 0) {
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
			item_tmp.identify=!itemdb_isequip3(nameid);
		if( st.end>st.start+5 ) //アイテムを指定したIDに渡す
			sd=map_id2sd(conv_num(st, (st.stack.stack_data[st.start+5])));
		if(sd == NULL) //アイテムを渡す相手がいなかったらお帰り
			return 0;
		if((flag = pc_additem(*sd,item_tmp,amount))) {
			clif_additem(*sd,0,0,flag);
			if( !pc_candrop(*sd,nameid) )
				map_addflooritem(item_tmp,amount,sd->bl.m,sd->bl.x,sd->bl.y,NULL,NULL,NULL,0);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_getitem2(struct script_state &st)
{
	//!! some stupid game with signs
	short nameid,amount;
	int flag = 0;
	int iden,ref,attr,c1,c2,c3,c4;
	struct item_data *item_data;
	struct item item_tmp;
	struct script_data *data;
	struct map_session_data *sd = script_rid2sd(st);

	data=&(st.stack.stack_data[st.start+2]);
	get_val(st,*data);
	if( data->type==C_STR || data->type==C_CONSTSTR ){
		const char *name=conv_str(st,*data);
		struct item_data *item_data = itemdb_searchname(name);
		nameid=512; //Apple item ID
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,*data);

	amount=conv_num(st, (st.stack.stack_data[st.start+3]));
	iden=conv_num(st, (st.stack.stack_data[st.start+4]));
	ref=conv_num(st, (st.stack.stack_data[st.start+5]));
	attr=conv_num(st, (st.stack.stack_data[st.start+6]));
	c1=conv_num(st, (st.stack.stack_data[st.start+7]));
	c2=conv_num(st, (st.stack.stack_data[st.start+8]));
	c3=conv_num(st, (st.stack.stack_data[st.start+9]));
	c4=conv_num(st, (st.stack.stack_data[st.start+10]));
	if( st.end>st.start+11 ) //アイテムを指定したIDに渡す
		sd=map_id2sd(conv_num(st, (st.stack.stack_data[st.start+11])));
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
			map_addflooritem(item_tmp,amount,sd->bl.m,sd->bl.x,sd->bl.y,NULL,NULL,NULL,0);
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
int buildin_getnameditem(struct script_state &st)
{
	int nameid;
	struct item item_tmp;
	struct map_session_data *sd, *tsd;
	struct script_data &data = st.stack.stack_data[st.start+2];


	sd = script_rid2sd(st);
	if (sd == NULL)
	{	//Player not attached!
		push_val(st.stack,C_INT,0);
		return 0; 
	}
	
	get_val(st,data);
	if( data.type==C_STR || data.type==C_CONSTSTR ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data == NULL)
		{	//Failed
			push_val(st.stack,C_INT,0);
			return 0;
		}
		nameid = item_data->nameid;
	}else
		nameid = conv_num(st,data);

	if(!itemdb_exists(nameid) || !itemdb_isequip3(nameid))
	{	//We don't allow non-equipable/stackable items to be named
		//to avoid any qty exploits that could happen because of it.
		push_val(st.stack,C_INT,0);
		return 0;
	}

	data = st.stack.stack_data[st.start+3];
	get_val(st,data);
	if( data.type==C_STR || data.type==C_CONSTSTR )	//Char Name
		tsd=map_nick2sd(conv_str(st,data));
	else	//Char Id was given
		tsd=map_charid2sd(conv_num(st,data));
	
	if( tsd == NULL )
	{	//Failed
		push_val(st.stack,C_INT,0);
		return 0;
	}

	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid=nameid;
	item_tmp.amount=1;
	item_tmp.identify=1;
	item_tmp.card[0]=255;
	item_tmp.card[2]=tsd->status.char_id;
	item_tmp.card[3]=tsd->status.char_id >> 16;
	if(pc_additem(*sd,item_tmp,1)) {
		push_val(st.stack,C_INT,0);
		return 0;	//Failed to add item, we will not drop if they don't fit
	}

	push_val(st.stack,C_INT,1);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_makeitem(struct script_state &st)
{
	//!! some stupid game with signs
	short nameid, amount;
	int flag = 0;
	int x,y,m;
	const char *mapname;
	struct item item_tmp;
	struct map_session_data *sd;
	struct script_data *data;

	sd = script_rid2sd(st);

	data=&(st.stack.stack_data[st.start+2]);
	get_val(st,*data);
	if( data->type==C_STR || data->type==C_CONSTSTR ){
		const char *name=conv_str(st,*data);
		struct item_data *item_data = itemdb_searchname(name);
		nameid=512; //Apple Item ID
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,*data);

	amount=conv_num(st, (st.stack.stack_data[st.start+3]));
	mapname	=conv_str(st,(st.stack.stack_data[st.start+4]));
	x	=conv_num(st, (st.stack.stack_data[st.start+5]));
	y	=conv_num(st, (st.stack.stack_data[st.start+6]));

	if( sd && strcmp(mapname,"this")==0)
		m=sd->bl.m;
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
			item_tmp.identify=!itemdb_isequip3(nameid);

//		clif_additem(sd,0,0,flag);
		map_addflooritem(item_tmp,amount,m,x,y,NULL,NULL,NULL,0);
	}

	return 0;
}
/*==========================================
 * script DELITEM command (fixed 2 bugs by Lupus, added deletion priority by Lupus)
 *------------------------------------------
 */
int buildin_delitem(struct script_state &st)
{
	unsigned short nameid=0,amount;
	int i,important_item=0;
	struct map_session_data *sd;
	struct script_data *data;

	sd = script_rid2sd(st);

	data=&(st.stack.stack_data[st.start+2]);
	get_val(st,*data);
	if( data->type==C_STR || data->type==C_CONSTSTR ){
		const char *name=conv_str(st,*data);
		struct item_data *item_data = itemdb_searchname(name);
		//nameid=512;
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,*data);

	amount=conv_num(st, (st.stack.stack_data[st.start+3]));

	if (nameid<500 || nameid>20000 || amount>MAX_AMOUNT )
	{	//by Lupus. Don't run FOR if u got wrong item ID or amount<=0
		//ShowMessage("wrong item ID or amount<=0 : delitem %i,\n",nameid,amount);
		return 0;
	}
	sd=script_rid2sd(st);
	//1st pass
	//here we won't delete items with CARDS, named items but we count them
	for(i=0;i<MAX_INVENTORY;i++)
	{	//we don't delete wrong item or equipped item
		if( sd->status.inventory[i].nameid!=nameid || sd->inventory_data[i] == NULL ||
			sd->status.inventory[i].nameid>=20000  || sd->status.inventory[i].amount>=MAX_AMOUNT )
			continue;
		//1 egg uses 1 cell in the inventory. so it's ok to delete 1 pet / per cycle
		if(sd->inventory_data[i]->type==7 && sd->status.inventory[i].card[0] == 0xff00 && search_petDB_index(nameid, PET_EGG) >= 0 )
		{
			intif_delete_petdata( MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2]) );
			//clear egg flag. so it won't be put in IMPORTANT items (eggs look like item with 2 cards ^_^)
			sd->status.inventory[i].card[1] = sd->status.inventory[i].card[0] = 0;
			//now this egg'll be deleted as a common unimportant item
		}
		//is this item important? does it have cards? or Player's name? or Refined/Upgraded
		if( sd->status.inventory[i].card[0] || sd->status.inventory[i].card[1] ||
			sd->status.inventory[i].card[2] || sd->status.inventory[i].card[3] || sd->status.inventory[i].refine)
		{
			//this is important item, count it
			important_item++;
			continue;
		}

		if(sd->status.inventory[i].amount>=amount)
		{
			pc_delitem(*sd,i,amount,0);
			return 0; //we deleted exact amount of items. now exit
		}
		else
		{
			amount-=sd->status.inventory[i].amount;
			pc_delitem(*sd,i,sd->status.inventory[i].amount,0);
		}
	}
	//2nd pass
	//now if there WERE items with CARDs/REFINED/NAMED... and if we still have to delete some items. we'll delete them finally
	if (important_item>0 && amount>0)
	{
		for(i=0;i<MAX_INVENTORY;i++)
		{
			//we don't delete wrong item
			if( sd->status.inventory[i].nameid!=nameid || sd->inventory_data[i] == NULL ||
				sd->status.inventory[i].nameid>=20000  || sd->status.inventory[i].amount>=MAX_AMOUNT )
				continue;

			if(sd->status.inventory[i].amount>=amount)
			{
				pc_delitem(*sd,i,amount,0);
				return 0; //we deleted exact amount of items. now exit
			}
			else
			{
				amount-=sd->status.inventory[i].amount;
				pc_delitem(*sd,i,sd->status.inventory[i].amount,0);
			}
		}
	}
	return 0;
}

/*==========================================
 *キャラ関係のパラメータ取得
 *------------------------------------------
 */
int buildin_readparam(struct script_state &st)
{
	int type;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	if( st.end>st.start+3 )
		sd=map_nick2sd(conv_str(st,(st.stack.stack_data[st.start+3])));
	else
	sd=script_rid2sd(st);

	if(sd==NULL){
		push_val(st.stack,C_INT,-1);
		return 0;
	}

	push_val(st.stack,C_INT,pc_readparam(*sd,type));

	return 0;
}
/*==========================================
 *キャラ関係のID取得
 *------------------------------------------
 */
int buildin_getcharid(struct script_state &st)
{
	int num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	if( st.end>st.start+3 )
		sd=map_nick2sd(conv_str(st,(st.stack.stack_data[st.start+3])));
	else
	sd=script_rid2sd(st);
	if(sd==NULL){
		push_val(st.stack,C_INT,-1);
		return 0;
	}
	if(num==0)
		push_val(st.stack,C_INT,sd->status.char_id);
	if(num==1)
		push_val(st.stack,C_INT,sd->status.party_id);
	if(num==2)
		push_val(st.stack,C_INT,sd->status.guild_id);
	if(num==3)
		push_val(st.stack,C_INT,sd->status.account_id);
	return 0;
}
/*==========================================
 *指定IDのPT名取得
 *------------------------------------------
 */
char *buildin_getpartyname_sub(unsigned long party_id)
{
	struct party *p;

	p=NULL;
	p=party_search(party_id);

	if(p!=NULL){
		char *buf;
		buf=(char *)aMalloc(24*sizeof(char));
		memcpy(buf, p->name, 24);
		return buf;
	}

	return 0;
}
int buildin_getpartyname(struct script_state &st)
{
	char *name;
	int party_id;

	party_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	name=buildin_getpartyname_sub(party_id);
	if(name!=0)
		push_str(st.stack,C_STR, name);
	else
		push_str(st.stack,C_CONSTSTR, "null");

	return 0;
}
/*==========================================
 *指定IDのPT人数とメンバーID取得
 *------------------------------------------
 */
int buildin_getpartymember(struct script_state &st)
{
	struct party *p;
	int i,j=0;

	p=NULL;
	p=party_search(conv_num(st, (st.stack.stack_data[st.start+2])));

	if(p!=NULL){
		for(i=0;i<MAX_PARTY;i++){
			if(p->member[i].account_id){
//				ShowMessage("name:%s %d\n",p->member[i].name,i);
				mapreg_setregstr(add_str( "$@partymembername$")+(i<<24),p->member[i].name);
				j++;
			}
		}
	}
	mapreg_setreg(add_str( "$@partymembercount"),j);

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

	if(g!=NULL){
		char *buf;
		buf=(char *)aMalloc(24*sizeof(char));
		memcpy(buf, g->name, 24);
		return buf;
	}
	return 0;
}
int buildin_getguildname(struct script_state &st)
{
	char *name;
	int guild_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	name=buildin_getguildname_sub(guild_id);
	if(name!=0)
		push_str(st.stack,C_STR, name);
	else
		push_str(st.stack,C_CONSTSTR, "null");
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
		buf=(char *)aMalloc(24*sizeof(char));
		memcpy(buf,g->master, 24);//EOS included
		return buf;
	}
	return NULL;
}
int buildin_getguildmaster(struct script_state &st)
{
	char *master;
	int guild_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	master=buildin_getguildmaster_sub(guild_id);
	if(master!=0)
		push_str(st.stack,C_STR, master);
	else
		push_str(st.stack,C_CONSTSTR, "null");
	return 0;
}

int buildin_getguildmasterid(struct script_state &st)
{
	char *master;
	struct map_session_data *sd=NULL;
	int guild_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	master=buildin_getguildmaster_sub(guild_id);
	if(master!=0){
		if((sd=map_nick2sd(master)) == NULL){
			push_val(st.stack,C_INT,0);
			return 0;
		}
		push_val(st.stack,C_INT,sd->status.char_id);
	}else{
		push_val(st.stack,C_INT,0);
	}
	return 0;
}

/*==========================================
 * キャラクタの名前
 *------------------------------------------
 */
int buildin_strcharinfo(struct script_state &st)
{
	struct map_session_data *sd;
	int num;

	sd=script_rid2sd(st);
	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	if(num==0){
		char *buf;
		buf=(char *)aMalloc(24*sizeof(char));
		memcpy(buf,sd->status.name, 24);//EOS included
		push_str(st.stack,C_STR, buf);
	}
	if(num==1){
		char *buf;
		buf=buildin_getpartyname_sub(sd->status.party_id);
		if(buf!=0)
			push_str(st.stack,C_STR, buf);
		else
			push_str(st.stack,C_CONSTSTR, "");
	}
	if(num==2){
		char *buf;
		buf=buildin_getguildname_sub(sd->status.guild_id);
		if(buf!=0)
			push_str(st.stack,C_STR, buf);
		else
			push_str(st.stack,C_CONSTSTR, "");
	}

	return 0;
}

unsigned int equip[10]={0x0100,0x0010,0x0020,0x0002,0x0004,0x0040,0x0008,0x0080,0x0200,0x0001};

/*==========================================
 * GetEquipID(Pos);     Pos: 1-10
 *------------------------------------------
 */
int buildin_getequipid(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;
	struct item_data* item;

	sd=script_rid2sd(st);
	if(!sd) return 0;
	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0){
		item=sd->inventory_data[i];
		if(item)
			push_val(st.stack,C_INT,item->nameid);
		else
			push_val(st.stack,C_INT,0);
	}else{
		push_val(st.stack,C_INT,-1);
	}
	return 0;
}

/*==========================================
 * 装備名文字列（精錬メニュー用）
 *------------------------------------------
 */
int buildin_getequipname(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;
	struct item_data* item;
	char *buf;

	buf=(char *)aMalloc(64*sizeof(char));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0){
		item=sd->inventory_data[i];
		if(item)
			sprintf(buf,"%s-[%s]",positions[num-1],item->jname);
		else
			sprintf(buf,"%s-[%s]",positions[num-1],positions[10]);
	}else{
		sprintf(buf,"%s-[%s]",positions[num-1],positions[10]);
	}
	push_str(st.stack,C_STR, buf);

	return 0;
}

/*==========================================
 * getbrokenid [Valaris]
 *------------------------------------------
 */
int buildin_getbrokenid(struct script_state &st)
{
	int i,num,id=0,brokencounter=0;
	struct map_session_data *sd;

	sd=script_rid2sd(st);

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute==1){
				brokencounter++;
				if(num==brokencounter){
					id=sd->status.inventory[i].nameid;
					break;
				}
		}
	}

	push_val(st.stack,C_INT,id);

	return 0;
}

/*==========================================
 * repair [Valaris]
 *------------------------------------------
 */
int buildin_repair(struct script_state &st)
{
	int i,num;
	int repaircounter=0;
	struct map_session_data *sd;


	sd=script_rid2sd(st);

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute==1){
				repaircounter++;
				if(num==repaircounter){
					sd->status.inventory[i].attribute=0;
					clif_equiplist(*sd);
					clif_produceeffect(*sd, 0, sd->status.inventory[i].nameid);
					clif_misceffect(sd->bl, 3);
					clif_displaymessage(sd->fd,"Item has been repaired.");
					break;
				}
		}
	}

	return 0;
}

/*==========================================
 * 装備チェック
 *------------------------------------------
 */
int buildin_getequipisequiped(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0){
		push_val(st.stack,C_INT,1);
	}else{
		push_val(st.stack,C_INT,0);
	}

	return 0;
}

/*==========================================
 * 装備品精錬可能チェック
 *------------------------------------------
 */
int buildin_getequipisenableref(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0 && num<7 && sd->inventory_data[i] && !sd->inventory_data[i]->flag.no_refine)
			// replaced by Celest
			/*(num!=1
				 || sd->inventory_data[i]->def > 1
	             || (sd->inventory_data[i]->def==1 && sd->inventory_data[i]->equip_script==NULL)
	             || (sd->inventory_data[i]->def<=0 && sd->inventory_data[i]->equip_script!=NULL)))*/

	{
		push_val(st.stack,C_INT,1);
	} else {
		push_val(st.stack,C_INT,0);
	}

	return 0;
}

/*==========================================
 * 装備品鑑定チェック
 *------------------------------------------
 */
int buildin_getequipisidentify(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0)
		push_val(st.stack,C_INT,sd->status.inventory[i].identify);
	else
		push_val(st.stack,C_INT,0);

	return 0;
}

/*==========================================
 * 装備品精錬度
 *------------------------------------------
 */
int buildin_getequiprefinerycnt(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0)
		push_val(st.stack,C_INT,sd->status.inventory[i].refine);
	else
		push_val(st.stack,C_INT,0);

	return 0;
}

/*==========================================
 * 装備品武器LV
 *------------------------------------------
 */
int buildin_getequipweaponlv(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0 && sd->inventory_data[i])
		push_val(st.stack,C_INT,sd->inventory_data[i]->wlv);
	else
		push_val(st.stack,C_INT,0);

	return 0;
}

/*==========================================
 * 装備品精錬成功率
 *------------------------------------------
 */
int buildin_getequippercentrefinery(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(i >= 0)
		push_val(st.stack,C_INT,status_percentrefinery(*sd,sd->status.inventory[i]));
	else
		push_val(st.stack,C_INT,0);

	return 0;
}

/*==========================================
 * 精錬成功
 *------------------------------------------
 */
int buildin_successrefitem(struct script_state &st)
{
	int i,num,ep;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd) return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(sd && i >= 0) {
		ep=sd->status.inventory[i].equip;

		if(log_config.refine > 0)
			log_refine(*sd, i, 1);

		sd->status.inventory[i].refine++;
		pc_unequipitem(*sd,i,2);
		clif_refine(sd->fd,*sd,0,i,sd->status.inventory[i].refine);
		clif_delitem(*sd,i,1);
		clif_additem(*sd,i,1,0);
		pc_equipitem(*sd,i,ep);
		clif_misceffect(sd->bl,3);
		if( sd->status.inventory[i].refine == 10 && 
			sd->status.inventory[i].card[0] == 0x00ff && 
			sd->status.char_id == MakeDWord(sd->status.inventory[i].card[2],sd->status.inventory[i].card[3])  )
		{	// Fame point system [DracoRPG]
	 		switch (sd->inventory_data[i]->wlv){
				case 1:
					pc_addfame(*sd,1,0); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
					break;
				case 2:
					pc_addfame(*sd,25,0); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
					break;
				case 3:
					pc_addfame(*sd,1000,0); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
					break;
	 	 	 }
		}
	}

	return 0;
}

/*==========================================
 * 精錬失敗
 *------------------------------------------
 */
int buildin_failedrefitem(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd)
		return 0;
	i=pc_checkequip(*sd,equip[num-1]);
	if(sd && i >= 0) {
		if(log_config.refine > 0)
			log_refine(*sd, i, 0);

		sd->status.inventory[i].refine = 0;
		pc_unequipitem(*sd,i,3);
		// 精錬失敗エフェクトのパケット
		clif_refine(sd->fd,*sd,1,i,sd->status.inventory[i].refine);
		pc_delitem(*sd,i,1,0);
		// 他の人にも失敗を通知
		clif_misceffect(sd->bl,2);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_statusup(struct script_state &st)
{
	int type;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(sd) pc_statusup(*sd,type);

	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_statusup2(struct script_state &st)
{
	int type,val;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	val=conv_num(st, (st.stack.stack_data[st.start+3]));
	sd=script_rid2sd(st);
	if(sd) pc_statusup2(*sd,type,val);

	return 0;
}
/*==========================================
 * 装備品による能力値ボーナス
 *------------------------------------------
 */
int buildin_bonus(struct script_state &st)
{
	int type,val;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	val=conv_num(st, (st.stack.stack_data[st.start+3]));
	sd=script_rid2sd(st);
	if(sd) pc_bonus(*sd,type,val);

	return 0;
}
/*==========================================
 * 装備品による能力値ボーナス
 *------------------------------------------
 */
int buildin_bonus2(struct script_state &st)
{
	int type,type2,val;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	type2=conv_num(st, (st.stack.stack_data[st.start+3]));
	val=conv_num(st, (st.stack.stack_data[st.start+4]));
	sd=script_rid2sd(st);
	if(sd) pc_bonus2(*sd,type,type2,val);

	return 0;
}
/*==========================================
 * 装備品による能力値ボーナス
 *------------------------------------------
 */
int buildin_bonus3(struct script_state &st)
{
	int type,type2,type3,val;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	type2=conv_num(st, (st.stack.stack_data[st.start+3]));
	type3=conv_num(st, (st.stack.stack_data[st.start+4]));
	val=conv_num(st, (st.stack.stack_data[st.start+5]));
	sd=script_rid2sd(st);
	if(sd) pc_bonus3(*sd,type,type2,type3,val);

	return 0;
}

int buildin_bonus4(struct script_state &st)
{
	int type,type2,type3,type4,val;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	type2=conv_num(st, (st.stack.stack_data[st.start+3]));
	type3=conv_num(st, (st.stack.stack_data[st.start+4]));
	type4=conv_num(st, (st.stack.stack_data[st.start+5]));
	val=conv_num(st, (st.stack.stack_data[st.start+6]));
	sd=script_rid2sd(st);
	if(sd) pc_bonus4(*sd,type,type2,type3,type4,val);

	return 0;
}
/*==========================================
 * スキル所得
 *------------------------------------------
 */
int buildin_skill(struct script_state &st)
{
	int id,level,flag=1;
	struct map_session_data *sd;

	id=conv_num(st, (st.stack.stack_data[st.start+2]));
	level=conv_num(st, (st.stack.stack_data[st.start+3]));
	if( st.end>st.start+4 )
		flag=conv_num(st,(st.stack.stack_data[st.start+4]) );
	sd=script_rid2sd(st);
	if(sd) pc_skill(*sd,id,level,flag);

	return 0;
}

// add x levels of skill (stackable) [Valaris]
int buildin_addtoskill(struct script_state &st)
{
	int id,level,flag=2;
	struct map_session_data *sd;

	id=conv_num(st, (st.stack.stack_data[st.start+2]));
	level=conv_num(st, (st.stack.stack_data[st.start+3]));
	if( st.end>st.start+4 )
		flag=conv_num(st,(st.stack.stack_data[st.start+4]) );
	sd=script_rid2sd(st);
	if(sd) pc_skill(*sd,id,level,flag);

	return 0;
}

/*==========================================
 * ギルドスキル取得
 *------------------------------------------
 */
int buildin_guildskill(struct script_state &st)
{
	int id,level,flag=0;
	struct map_session_data *sd;
	int i=0;

	id=conv_num(st, (st.stack.stack_data[st.start+2]));
	level=conv_num(st, (st.stack.stack_data[st.start+3]));
	if( st.end>st.start+4 )
		flag=conv_num(st,(st.stack.stack_data[st.start+4]) );
	sd=script_rid2sd(st);
	for(i=0;i<level;i++)
		guild_skillup(*sd,id,flag);

	return 0;
}
/*==========================================
 * スキルレベル所得
 *------------------------------------------
 */
int buildin_getskilllv(struct script_state &st)
{
	int id=conv_num(st, (st.stack.stack_data[st.start+2]));
	map_session_data *sd = script_rid2sd(st);
	push_val(st.stack,C_INT, (sd)?pc_checkskill(*sd,id):0 );
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
int buildin_getgdskilllv(struct script_state &st)
{
	unsigned long guild_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	unsigned short skill_id=conv_num(st, (st.stack.stack_data[st.start+3]));
        struct guild *g=guild_search(guild_id);
	push_val(st.stack, C_INT, (g==NULL)?-1:guild_checkskill(*g,skill_id) );
	return 0;
/*
	struct map_session_data *sd=NULL;
	struct guild *g=NULL;
	short skill_id;

	skill_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(sd && sd->status.guild_id > 0) g=guild_search(sd->status.guild_id);
	if(sd && g) {
		push_val(st.stack,C_INT, guild_checkskill(*g,skill_id+9999) );
	} else {
		push_val(st.stack,C_INT,-1);
	}
	return 0;
*/
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_basicskillcheck(struct script_state &st)
{
	push_val(st.stack,C_INT, battle_config.basic_skill_check);
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_getgmlevel(struct script_state &st)
{
	map_session_data *sd = script_rid2sd(st);
	push_val(st.stack,C_INT, ((sd) ? pc_isGM(*sd):0) );
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_end(struct script_state &st)
{
	st.state = END;
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_checkoption(struct script_state &st)
{
	int type;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);

	if(sd->status.option & type){
		push_val(st.stack,C_INT,1);
	} else {
		push_val(st.stack,C_INT,0);
	}

	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_checkoption1(struct script_state &st)
{
	int type;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);

	if(sd->opt1 & type){
		push_val(st.stack,C_INT,1);
	} else {
		push_val(st.stack,C_INT,0);
	}

	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int buildin_checkoption2(struct script_state &st)
{
	int type;
	struct map_session_data *sd;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);

	if(sd->opt2 & type){
		push_val(st.stack,C_INT,1);
	} else {
		push_val(st.stack,C_INT,0);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_setoption(struct script_state &st)
{
	int type;
	struct map_session_data *sd=script_rid2sd(st);
	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	if(sd) pc_setoption(*sd,type);
	return 0;
}

/*==========================================
 * Checkcart [Valaris]
 *------------------------------------------
 */

int buildin_checkcart(struct script_state &st)
{
	struct map_session_data *sd;

	sd=script_rid2sd(st);

	if(sd && pc_iscarton(*sd)){
		push_val(st.stack,C_INT,1);
	} else {
		push_val(st.stack,C_INT,0);
	}
	return 0;
}

/*==========================================
 * カートを付ける
 *------------------------------------------
 */
int buildin_setcart(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) pc_setcart(*sd,1);

	return 0;
}

/*==========================================
 * checkfalcon [Valaris]
 *------------------------------------------
 */

int buildin_checkfalcon(struct script_state &st)
{
	struct map_session_data *sd;

	sd=script_rid2sd(st);

	if(sd && pc_isfalcon(*sd)){
		push_val(st.stack,C_INT,1);
	} else {
		push_val(st.stack,C_INT,0);
	}

	return 0;
}


/*==========================================
 * 鷹を付ける
 *------------------------------------------
 */
int buildin_setfalcon(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) pc_setfalcon(*sd);

	return 0;
}

/*==========================================
 * Checkcart [Valaris]
 *------------------------------------------
 */

int buildin_checkriding(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd && pc_isriding(*sd))
		push_val(st.stack,C_INT,1);
	else
		push_val(st.stack,C_INT,0);

	return 0;
}


/*==========================================
 * ペコペコ乗り
 *------------------------------------------
 */
int buildin_setriding(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) pc_setriding(*sd);
	return 0;
}

/*==========================================
 *	セーブポイントの保存
 *------------------------------------------
 */
int buildin_savepoint(struct script_state &st)
{
	int x,y;
	const char *str;

	str=conv_str(st,(st.stack.stack_data[st.start+2]));
	x=conv_num(st, (st.stack.stack_data[st.start+3]));
	y=conv_num(st, (st.stack.stack_data[st.start+4]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) pc_setsavepoint(*sd,str,x,y);
	return 0;
}

/*==========================================
 * GetTimeTick(0: System Tick, 1: Time Second Tick)
 *------------------------------------------
 */
int buildin_gettimetick(struct script_state &st)	/* Asgard Version */
{
	int type;
	time_t timer;
	struct tm *t;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));

	switch(type){
	case 1:
		//type 1:(Second Ticks: 0-86399, 00:00:00-23:59:59)
		time(&timer);
		t=localtime(&timer);
		push_val(st.stack,C_INT,((t->tm_hour)*3600+(t->tm_min)*60+t->tm_sec));
		break;
	case 0:
	default:
		//type 0:(System Ticks)
		push_val(st.stack,C_INT,gettick());
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
int buildin_gettime(struct script_state &st)	/* Asgard Version */
{
	int type;
	time_t timer;
	struct tm *t;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));

	time(&timer);
	t=localtime(&timer);

	switch(type){
	case 1://Sec(0~59)
		push_val(st.stack,C_INT,t->tm_sec);
		break;
	case 2://Min(0~59)
		push_val(st.stack,C_INT,t->tm_min);
		break;
	case 3://Hour(0~23)
		push_val(st.stack,C_INT,t->tm_hour);
		break;
	case 4://WeekDay(0~6)
		push_val(st.stack,C_INT,t->tm_wday);
		break;
	case 5://MonthDay(01~31)
		push_val(st.stack,C_INT,t->tm_mday);
		break;
	case 6://Month(01~12)
		push_val(st.stack,C_INT,t->tm_mon+1);
		break;
	case 7://Year(20xx)
		push_val(st.stack,C_INT,t->tm_year+1900);
		break;
	default://(format error)
		push_val(st.stack,C_INT,-1);
		break;
	}
	return 0;
}

/*==========================================
 * GetTimeStr("TimeFMT", Length);
 *------------------------------------------
 */
int buildin_gettimestr(struct script_state &st)
{
	char *tmpstr;
	const char *fmtstr;
	int maxlen;
	time_t now = time(NULL);

	fmtstr=conv_str(st,(st.stack.stack_data[st.start+2]));
	maxlen=conv_num(st, (st.stack.stack_data[st.start+3]));

	tmpstr=(char *)aMalloc((maxlen+1)*sizeof(char));
	strftime(tmpstr,maxlen,fmtstr,localtime(&now));
	tmpstr[maxlen]='\0';

	push_str(st.stack,C_STR, tmpstr);
	return 0;
}

/*==========================================
 * カプラ倉庫を開く
 *------------------------------------------
 */
int buildin_openstorage(struct script_state &st)
{
	struct map_session_data *sd = script_rid2sd(st);
	int ret=0;
	if(sd) ret = storage_storageopen(*sd);
	push_val(st.stack,C_INT,ret);
	return 0;
}

int buildin_guildopenstorage(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	int ret=0;
	if(sd) ret = storage_guild_storageopen(*sd);
	push_val(st.stack,C_INT,ret);
	return 0;
}

/*==========================================
 * アイテムによるスキル発動
 *------------------------------------------
 */
int buildin_itemskill(struct script_state &st)
{
	int id,lv;
	const char *str;
	struct map_session_data *sd=script_rid2sd(st);

	id=conv_num(st, (st.stack.stack_data[st.start+2]));
	lv=conv_num(st, (st.stack.stack_data[st.start+3]));
	str=conv_str(st,(st.stack.stack_data[st.start+4]));

	// 詠唱中にスキルアイテムは使用できない
	if(sd->skilltimer != -1)
		return 0;

	sd->skillitem=id;
	sd->skillitemlv=lv;
	clif_item_skill(*sd,id,lv,str);
	return 0;
}
/*==========================================
 * アイテム作成
 *------------------------------------------
 */
int buildin_produce(struct script_state &st)
{
	int trigger;
	struct map_session_data *sd=script_rid2sd(st);

	if(	sd->state.produce_flag == 1) return 0;
	trigger=conv_num(st, (st.stack.stack_data[st.start+2]));
	clif_skill_produce_mix_list(*sd,trigger);
	return 0;
}
/*==========================================
 * NPCでペット作る
 *------------------------------------------
 */
int buildin_makepet(struct script_state &st)
{
	struct map_session_data *sd = script_rid2sd(st);
	struct script_data *data;
	int id,pet_id;

	data=&(st.stack.stack_data[st.start+2]);
	get_val(st,*data);

	id=conv_num(st,*data);

	pet_id = search_petDB_index(id, PET_CLASS);

	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0 && sd) {
		sd->catch_target_class = pet_db[pet_id].class_;
		intif_create_pet(
			sd->status.account_id, sd->status.char_id,				//long
			pet_db[pet_id].class_, mob_db[pet_db[pet_id].class_].lv,//short
			pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate, 100,	//short
			0, 1,													//char
			pet_db[pet_id].jname);									//char*
	}

	return 0;
}
/*==========================================
 * NPCで経験値上げる
 *------------------------------------------
 */
int buildin_getexp(struct script_state &st)
{
	struct map_session_data *sd = script_rid2sd(st);
	int base=0,job=0;

	base=conv_num(st, (st.stack.stack_data[st.start+2]));
	job =conv_num(st, (st.stack.stack_data[st.start+3]));
	if(base<0 || job<0)
		return 0;
	if(sd)
		pc_gainexp(*sd,base,job);
	return 0;
}

/*==========================================
 * Gain guild exp [Celest]
 *------------------------------------------
 */
int buildin_guildgetexp(struct script_state &st)
{
	struct map_session_data *sd = script_rid2sd(st);
	int exp;

	exp = conv_num(st, (st.stack.stack_data[st.start+2]));
	if(exp < 0)
		return 0;
	if(sd && sd->status.guild_id > 0)
		guild_getexp (*sd, exp);

	return 0;
}

/*==========================================
 * モンスター発生
 *------------------------------------------
 */
int buildin_monster(struct script_state &st)
{
	int class_,amount,x,y;
	const char *str,*map,*event="";

	map	=conv_str(st,(st.stack.stack_data[st.start+2]));
	x	=conv_num(st, (st.stack.stack_data[st.start+3]));
	y	=conv_num(st, (st.stack.stack_data[st.start+4]));
	str	=conv_str(st,(st.stack.stack_data[st.start+5]));
	class_=conv_num(st, (st.stack.stack_data[st.start+6]));
	amount=conv_num(st, (st.stack.stack_data[st.start+7]));
	if( st.end>st.start+8 )
		event=conv_str(st,(st.stack.stack_data[st.start+8]));
//!! broadcast command if not on this mapserver
	map_session_data *sd = map_id2sd(st.rid);
	mob_once_spawn(sd,map,x,y,str,class_,amount,event);
	return 0;
}
/*==========================================
 * モンスター発生
 *------------------------------------------
 */
int buildin_areamonster(struct script_state &st)
{
	int class_,amount,x0,y0,x1,y1;
	const char *str,*map,*event="";

	map	=conv_str(st, (st.stack.stack_data[st.start+2]));
	x0	=conv_num(st, (st.stack.stack_data[st.start+3]));
	y0	=conv_num(st, (st.stack.stack_data[st.start+4]));
	x1	=conv_num(st, (st.stack.stack_data[st.start+5]));
	y1	=conv_num(st, (st.stack.stack_data[st.start+6]));
	str	=conv_str(st, (st.stack.stack_data[st.start+7]));
	class_=conv_num(st, (st.stack.stack_data[st.start+8]));
	amount=conv_num(st, (st.stack.stack_data[st.start+9]));
	if( st.end>st.start+10 )
		event=conv_str(st, (st.stack.stack_data[st.start+10]));
//!! broadcast command if not on this mapserver
	mob_once_spawn_area(map_id2sd(st.rid),map,x0,y0,x1,y1,str,class_,amount,event);
	return 0;
}
/*==========================================
 * モンスター削除
 *------------------------------------------
 */
int buildin_killmonster_sub(struct block_list &bl,va_list ap)
{
	struct mob_data &md =(struct mob_data&)bl;
	char *event=va_arg(ap,char *);
	int allflag=va_arg(ap,int);

	if(allflag)
	{	// delete all script-summoned mobs
		if( !md.cache  )
			mob_delete(md);
	}
	else
	{	// delete only mobs with same event name
		if(strcmp(event, md.npc_event)==0)
			mob_delete(md);
	}
	return 0;
}

int buildin_killmonster(struct script_state &st)
{
	const char *mapname,*event;
	int m,allflag=0;
	mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	event=conv_str(st, (st.stack.stack_data[st.start+3]));
	if(strcmp(event,"All")==0)
		allflag = 1;

	if( (m=map_mapname2mapid(mapname))<0 )
		return 0;
//!! broadcast command if not on this mapserver
	map_foreachinarea(buildin_killmonster_sub,
		m,0,0,map[m].xs,map[m].ys,BL_MOB, event ,allflag);
	return 0;
}

int buildin_killmonsterall_sub(struct block_list &bl,va_list ap)
{
	mob_delete((struct mob_data &)bl);
	return 0;
}
int buildin_killmonsterall(struct script_state &st)
{
	const char *mapname;
	int m;
	mapname=conv_str(st, (st.stack.stack_data[st.start+2]));

	if( (m=map_mapname2mapid(mapname))<0 )
		return 0;
//!! broadcast command if not on this mapserver
	map_foreachinarea(buildin_killmonsterall_sub,
		m,0,0,map[m].xs,map[m].ys,BL_MOB);
	return 0;
}

/*==========================================
 * イベント実行
 *------------------------------------------
 */
int buildin_doevent(struct script_state &st)
{
	const char *event;
	event=conv_str(st, (st.stack.stack_data[st.start+2]));
//!! broadcast command if not on this mapserver
	map_session_data *sd = map_id2sd(st.rid);
	if(sd) npc_event(*sd,event,0);
	return 0;
}
/*==========================================
 * NPC主体イベント実行
 *------------------------------------------
 */
int buildin_donpcevent(struct script_state &st)
{
	const char *event;
	event=conv_str(st, (st.stack.stack_data[st.start+2]));
	npc_event_do(event);
	return 0;
}
/*==========================================
 * イベントタイマー追加
 *------------------------------------------
 */
int buildin_addtimer(struct script_state &st)
{
	const char *event;
	unsigned long tick;
	tick=conv_num(st, (st.stack.stack_data[st.start+2]));
	event=conv_str(st, (st.stack.stack_data[st.start+3]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) pc_addeventtimer(*sd,tick,event);
	return 0;
}
/*==========================================
 * イベントタイマー削除
 *------------------------------------------
 */
int buildin_deltimer(struct script_state &st)
{
	const char *event;
	event=conv_str(st, (st.stack.stack_data[st.start+2]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) pc_deleventtimer(*sd,event);
	return 0;
}
/*==========================================
 * イベントタイマーのカウント値追加
 *------------------------------------------
 */
int buildin_addtimercount(struct script_state &st)
{
	const char *event;
	unsigned long tick;
	event=conv_str(st, (st.stack.stack_data[st.start+2]));
	tick=conv_num(st, (st.stack.stack_data[st.start+3]));
	map_session_data *sd = script_rid2sd(st); 
	if(sd) pc_addeventtimercount(*sd,event,tick);
	return 0;
}

/*==========================================
 * NPCタイマー初期化
 *------------------------------------------
 */
int buildin_initnpctimer(struct script_state &st)
{
	struct npc_data *nd;
	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd)
	{
		npc_settimerevent_tick(*nd,0);
		npc_timerevent_start(*nd, st.rid);
	}
	return 0;
}
/*==========================================
 * NPCタイマー開始
 *------------------------------------------
 */
int buildin_startnpctimer(struct script_state &st)
{
	struct npc_data *nd;
	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd)
	{
		npc_timerevent_start(*nd, st.rid);
	}
	return 0;
}
/*==========================================
 * NPCタイマー停止
 *------------------------------------------
 */
int buildin_stopnpctimer(struct script_state &st)
{
	struct npc_data *nd;
	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd) npc_timerevent_stop(*nd);
	return 0;
}
/*==========================================
 * NPCタイマー情報所得
 *------------------------------------------
 */
int buildin_getnpctimer(struct script_state &st)
{
	struct npc_data *nd;
	int type=conv_num(st, (st.stack.stack_data[st.start+2]));
	int val=0;
	if( st.end > st.start+3 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+3])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd)
	{
	switch(type){
		case 0: val=npc_gettimerevent_tick(*nd); break;
	case 1: val= (nd->u.scr.nexttimer>=0); break;
	case 2: val= nd->u.scr.timeramount; break;
	}
	}
	push_val(st.stack,C_INT,val);
	return 0;
}
/*==========================================
 * NPCタイマー値設定
 *------------------------------------------
 */
int buildin_setnpctimer(struct script_state &st)
{
	unsigned long tick;
	struct npc_data *nd;
	tick=conv_num(st, (st.stack.stack_data[st.start+2]));
	if( st.end > st.start+3 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+3])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd)
	{
		npc_settimerevent_tick(*nd,tick);
	}
	return 0;
}

/*==========================================
 * attaches the player rid to the timer [Celest]
 *------------------------------------------
 */
int buildin_attachnpctimer(struct script_state &st)
{
	struct map_session_data *sd;
	struct npc_data *nd;

	nd=(struct npc_data *)map_id2bl(st.oid);
	if( st.end > st.start+2 ) {
		const char *name = conv_str(st, (st.stack.stack_data[st.start+2]));
		sd=map_nick2sd(name);
	} else {
		sd = script_rid2sd(st);
	}
	if(sd) nd->u.scr.rid = sd->bl.id;
		return 0;
}

/*==========================================
 * detaches a player rid from the timer [Celest]
 *------------------------------------------
 */
int buildin_detachnpctimer(struct script_state &st)
{
	struct npc_data *nd;
	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	nd->u.scr.rid = 0;
	return 0;
}

/*==========================================
 * 天の声アナウンス
 *------------------------------------------
 */
int buildin_announce(struct script_state &st)
{
	const char *str;
	int flag;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	flag=conv_num(st, (st.stack.stack_data[st.start+3]));

	if(flag&0x0f){
		struct block_list *bl=(flag&0x08)? map_id2bl(st.oid) :
			(struct block_list *)script_rid2sd(st);
		clif_GMmessage(bl,str,flag);
	}else
		intif_GMmessage(str,flag);
	return 0;
}
/*==========================================
 * 天の声アナウンス（特定マップ）
 *------------------------------------------
 */
int buildin_mapannounce_sub(struct block_list &bl,va_list ap)
{
	char *str;
	int len,flag;
	str=va_arg(ap,char *);
	len=va_arg(ap,int);
	flag=va_arg(ap,int);
	clif_GMmessage(&bl,str,flag|3);
	return 0;
}
int buildin_mapannounce(struct script_state &st)
{
	const char *mapname,*str;
	int flag,m;

	mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	str=conv_str(st, (st.stack.stack_data[st.start+3]));
	flag=conv_num(st, (st.stack.stack_data[st.start+4]));

	if( (m=map_mapname2mapid(mapname))<0 )
		return 0;
//!! broadcast command if not on this mapserver
	map_foreachinarea(buildin_mapannounce_sub,
		m,0,0,map[m].xs,map[m].ys,BL_PC, str,strlen(str)+1,flag&0x10);
	return 0;
}
/*==========================================
 * 天の声アナウンス（特定エリア）
 *------------------------------------------
 */
int buildin_areaannounce(struct script_state &st)
{
	const char *map,*str;
	int flag,m;
	int x0,y0,x1,y1;

	map=conv_str(st, (st.stack.stack_data[st.start+2]));
	x0=conv_num(st, (st.stack.stack_data[st.start+3]));
	y0=conv_num(st, (st.stack.stack_data[st.start+4]));
	x1=conv_num(st, (st.stack.stack_data[st.start+5]));
	y1=conv_num(st, (st.stack.stack_data[st.start+6]));
	str=conv_str(st, (st.stack.stack_data[st.start+7]));
	flag=conv_num(st, (st.stack.stack_data[st.start+8]));

	if( (m=map_mapname2mapid(map))<0 )
		return 0;
//!! broadcast command if not on this mapserver
	map_foreachinarea(buildin_mapannounce_sub,
		m,x0,y0,x1,y1,BL_PC, str,strlen(str)+1,flag&0x10 );
	return 0;
}
/*==========================================
 * ユーザー数所得
 *------------------------------------------
 */
int buildin_getusers(struct script_state &st)
{
	int flag=conv_num(st, (st.stack.stack_data[st.start+2]));
	struct block_list *bl=map_id2bl((flag&0x08)?st.oid:st.rid);
	int val=0;
	switch(flag&0x07){
	case 0: val=map[bl->m].users; break;
	case 1: val=map_getusers(); break;
	}
	push_val(st.stack,C_INT,val);
	return 0;
}
/*==========================================
 * Works like @WHO - displays all online users names in window
 *------------------------------------------
 */
int buildin_getusersname(struct script_state &st)
{
	struct map_session_data *pl_sd = NULL;
	size_t i=0,disp_num=1;
	map_session_data *sd = script_rid2sd(st);
	if(sd) 
	for (i=0;i<fd_max;i++)
		if(session[i] && (pl_sd=(struct map_session_data *) session[i]->session_data) && pl_sd->state.auth){
			if( !(battle_config.hide_GM_session && pc_isGM(*pl_sd)) ){
				if((disp_num++)%10==0)
					clif_scriptnext(*sd,st.oid);
				clif_scriptmes(*sd,st.oid,pl_sd->status.name);
			}
		}
	return 0;
}
/*==========================================
 * マップ指定ユーザー数所得
 *------------------------------------------
 */
int buildin_getmapusers(struct script_state &st)
{
	const char *str;
	int m;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	if( (m=map_mapname2mapid(str))< 0){
		push_val(st.stack,C_INT,-1);
		return 0;
	}
//!! broadcast command if not on this mapserver
	push_val(st.stack,C_INT,map[m].users);
	return 0;
}
/*==========================================
 * エリア指定ユーザー数所得
 *------------------------------------------
 */
int buildin_getareausers_sub(struct block_list &bl,va_list ap)
{
	int *users=va_arg(ap,int *);
	(*users)++;
	return 0;
}
int buildin_getareausers(struct script_state &st)
{
	const char *str;
	int m,x0,y0,x1,y1,users=0;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	x0=conv_num(st, (st.stack.stack_data[st.start+3]));
	y0=conv_num(st, (st.stack.stack_data[st.start+4]));
	x1=conv_num(st, (st.stack.stack_data[st.start+5]));
	y1=conv_num(st, (st.stack.stack_data[st.start+6]));
	if( (m=map_mapname2mapid(str))< 0){
		push_val(st.stack,C_INT,-1);
		return 0;
	}
//!! broadcast command if not on this mapserver
	map_foreachinarea(buildin_getareausers_sub,
		m,x0,y0,x1,y1,BL_PC,&users);
	push_val(st.stack,C_INT,users);
	return 0;
}

/*==========================================
 * エリア指定ドロップアイテム数所得
 *------------------------------------------
 */
int buildin_getareadropitem_sub(struct block_list &bl,va_list ap)
{
	int item=va_arg(ap,int);
	int *amount=va_arg(ap,int *);
	struct flooritem_data &drop=(struct flooritem_data &)bl;

	if(drop.item_data.nameid==item)
		(*amount)+=drop.item_data.amount;

	return 0;
}
int buildin_getareadropitem(struct script_state &st)
{
	const char *str;
	int m,x0,y0,x1,y1,item,amount=0;
	struct script_data *data;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	x0=conv_num(st, (st.stack.stack_data[st.start+3]));
	y0=conv_num(st, (st.stack.stack_data[st.start+4]));
	x1=conv_num(st, (st.stack.stack_data[st.start+5]));
	y1=conv_num(st, (st.stack.stack_data[st.start+6]));

	data=&(st.stack.stack_data[st.start+7]);
	get_val(st,*data);
	if( data->type==C_STR || data->type==C_CONSTSTR ){
		const char *name=conv_str(st,*data);
		struct item_data *item_data = itemdb_searchname(name);
		item=512;
		if( item_data )
			item=item_data->nameid;
	}else
		item=conv_num(st,*data);

	if( (m=map_mapname2mapid(str))< 0){
		push_val(st.stack,C_INT,-1);
		return 0;
	}
//!! broadcast command if not on this mapserver
	map_foreachinarea(buildin_getareadropitem_sub,
		m,x0,y0,x1,y1,BL_ITEM,item,&amount);
	push_val(st.stack,C_INT,amount);
	return 0;
}
/*==========================================
 * NPCの有効化
 *------------------------------------------
 */
int buildin_enablenpc(struct script_state &st)
{
	const char *str;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	npc_enable(str,1);
	return 0;
}
/*==========================================
 * NPCの無効化
 *------------------------------------------
 */
int buildin_disablenpc(struct script_state &st)
{
	const char *str;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	npc_enable(str,0);
	return 0;
}

int buildin_enablearena(struct script_state &st)	// Added by RoVeRT
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	struct chat_data *cd;


	if(nd==NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id))==NULL)
		return 0;

	npc_enable(nd->name,1);
	nd->arenaflag=1;

	if(cd->users>=cd->trigger && cd->npc_event[0])
		npc_timer_event(cd->npc_event);

	return 0;
}
int buildin_disablearena(struct script_state &st)	// Added by RoVeRT
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	nd->arenaflag=0;

	return 0;
}
/*==========================================
 * 隠れているNPCの表示
 *------------------------------------------
 */
int buildin_hideoffnpc(struct script_state &st)
{
	const char *str;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	npc_enable(str,2);
	return 0;
}
/*==========================================
 * NPCをハイディング
 *------------------------------------------
 */
int buildin_hideonnpc(struct script_state &st)
{
	const char *str;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	npc_enable(str,4);
	return 0;
}
/*==========================================
 * 状態異常にかかる
 *------------------------------------------
 */
int buildin_sc_start(struct script_state &st)
{
	struct block_list *bl;
	int type;
	unsigned long tick;
	int val1;
	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	tick=conv_num(st, (st.stack.stack_data[st.start+3]));
	val1=conv_num(st, (st.stack.stack_data[st.start+4]));
	if( st.end>st.start+5 ) //指定したキャラを状態異常にする
		bl = map_id2bl(conv_num(st, (st.stack.stack_data[st.start+5])));
	else
	bl = map_id2bl(st.rid);

	if (bl != 0) {
		if(bl->type == BL_PC && ((struct map_session_data *)bl)->state.potion_flag==1)
			bl = map_id2bl(((struct map_session_data *)bl)->skilltarget);
		status_change_start(bl,type,val1,0,0,0,tick,0);
	}
	return 0;
}

/*==========================================
 * 状態異常にかかる(確率指定)
 *------------------------------------------
 */
int buildin_sc_start2(struct script_state &st)
{
	struct block_list *bl;
	int type;
	unsigned long tick;
	int val1,per;
	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	tick=conv_num(st, (st.stack.stack_data[st.start+3]));
	val1=conv_num(st, (st.stack.stack_data[st.start+4]));
	per=conv_num(st, (st.stack.stack_data[st.start+5]));
	if( st.end>st.start+6 ) //指定したキャラを状態異常にする
		bl = map_id2bl(conv_num(st, (st.stack.stack_data[st.start+6])));
	else
	bl = map_id2bl(st.rid);
	if(bl->type == BL_PC && ((struct map_session_data *)bl)->state.potion_flag==1)
		bl = map_id2bl(((struct map_session_data *)bl)->skilltarget);
	if(rand()%10000 < per)
	status_change_start(bl,type,val1,0,0,0,tick,0);
	return 0;
}

/*==========================================
 * 状態異常が直る
 *------------------------------------------
 */
int buildin_sc_end(struct script_state &st)
{
	struct block_list *bl;
	int type;
	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	bl = map_id2bl(st.rid);
	
	nullpo_retr(0,bl);  //Fixed null pointer right here [Kevin]

	if(bl->type == BL_PC && ((struct map_session_data *)bl)->state.potion_flag==1)
		bl = map_id2bl(((struct map_session_data *)bl)->skilltarget);
	status_change_end(bl,type,-1);
//	if(battle_config.etc_log)
//		ShowMessage("sc_end : %d %d\n",st.rid,type);
	return 0;
}
/*==========================================
 * 状態異常耐性を計算した確率を返す
 *------------------------------------------
 */
int buildin_getscrate(struct script_state &st)
{
	struct block_list *bl;
	int sc_def,type,rate;

	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	rate=conv_num(st, (st.stack.stack_data[st.start+3]));
	if( st.end>st.start+4 ) //指定したキャラの耐性を計算する
		bl = map_id2bl(conv_num(st, (st.stack.stack_data[st.start+6])));
	else
		bl = map_id2bl(st.rid);

	sc_def = status_get_sc_def(bl,type);

	rate = rate * sc_def / 100;
	push_val(st.stack,C_INT,rate);

	return 0;

}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_debugmes(struct script_state &st)
{
	conv_str(st, (st.stack.stack_data[st.start+2]));
	ShowMessage("script debug : %d %d : %s\n",st.rid,st.oid,st.stack.stack_data[st.start+2].u.str);
	return 0;
}

/*==========================================
 *捕獲アイテム使用
 *------------------------------------------
 */
int buildin_catchpet(struct script_state &st)
{
	int pet_id= conv_num(st, (st.stack.stack_data[st.start+2]));
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) pet_catch_process1(*sd,pet_id);
	return 0;
}

/*==========================================
 *携帯卵孵化機使用
 *------------------------------------------
 */
int buildin_birthpet(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) clif_sendegg(*sd);
	return 0;
}

/*==========================================
 * Added - AppleGirl For Advanced Classes, (Updated for Cleaner Script Purposes)
 *------------------------------------------
 */
int buildin_resetlvl(struct script_state &st)
{
	int type=conv_num(st, (st.stack.stack_data[st.start+2]));
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) pc_resetlvl(*sd,type);
	return 0;
}
/*==========================================
 * ステータスリセット
 *------------------------------------------
 */
int buildin_resetstatus(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) pc_resetstate(*sd);
	return 0;
}

/*==========================================
 * スキルリセット
 *------------------------------------------
 */
int buildin_resetskill(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd) pc_resetskill(*sd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int buildin_changebase(struct script_state &st)
{
	struct map_session_data *sd=NULL;
	int vclass;

	if( st.end>st.start+3 )
		sd=map_id2sd(conv_num(st, (st.stack.stack_data[st.start+3])));
	else
	sd=script_rid2sd(st);

	if(sd == NULL)
		return 0;

	vclass = conv_num(st, (st.stack.stack_data[st.start+2]));
	if(vclass == 22 && !battle_config.wedding_modifydisplay)
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
int buildin_changesex(struct script_state &st) {
	struct map_session_data *sd = NULL;
	sd = script_rid2sd(st);

	if (sd->status.sex == 0) {
		sd->status.sex = 1;
		if (sd->status.class_ == 20 || sd->status.class_ == 4021)
			sd->status.class_ -= 1;
	} else if (sd->status.sex == 1) {
		sd->status.sex = 0;
		if(sd->status.class_ == 19 || sd->status.class_ == 4020)
			sd->status.class_ += 1;
	}
	chrif_char_ask_name(-1, sd->status.name, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
	chrif_save(*sd);
	return 0;
}

/*==========================================
 * npcチャット作成
 *------------------------------------------
 */
int buildin_waitingroom(struct script_state &st)
{
	const char *name,*ev="";
	int limit, trigger = 0,pub=1;
	name=conv_str(st, (st.stack.stack_data[st.start+2]));
	limit= conv_num(st, (st.stack.stack_data[st.start+3]));
	if(limit==0)
		pub=3;

	if( (st.end > st.start+5) ){
		struct script_data* data=&(st.stack.stack_data[st.start+5]);
		get_val(st,*data);
		if(data->type==C_INT){
			// 新Athena仕様(旧Athena仕様と互換性あり)
			ev=conv_str(st, (st.stack.stack_data[st.start+4]));
			trigger=conv_num(st, (st.stack.stack_data[st.start+5]));
		}else{
			// eathena仕様
			trigger=conv_num(st, (st.stack.stack_data[st.start+4]));
			ev=conv_str(st, (st.stack.stack_data[st.start+5]));
		}
	}else{
		// 旧Athena仕様
		if( st.end > st.start+4 )
			ev=conv_str(st, (st.stack.stack_data[st.start+4]));
	}
	struct npc_data *nd = (struct npc_data *)map_id2bl(st.oid);
	if(nd) chat_createnpcchat(*nd,limit,pub,trigger,name,strlen(name)+1,ev);
	return 0;
}
/*==========================================
 * Works like 'announce' but outputs in the common chat window
 *------------------------------------------
 */
int buildin_globalmes(struct script_state &st)
{
	struct block_list *bl = map_id2bl(st.oid);
	struct npc_data *nd = (struct npc_data *)bl;
	const char *name=NULL,*mes;

	mes=conv_str(st, (st.stack.stack_data[st.start+2]));	// メッセージの取得
	if(mes==NULL) return 0;
	
	if(st.end>st.start+3){	// NPC名の取得(123#456)
		name=conv_str(st, (st.stack.stack_data[st.start+3]));
	} else {
		name=nd->name;
	}

	npc_globalmessage(name,mes);	// グローバルメッセージ送信

	return 0;
}
/*==========================================
 * npcチャット削除
 *------------------------------------------
 */
int buildin_delwaitingroom(struct script_state &st)
{
	struct npc_data *nd;
	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);
	if(nd) chat_deletenpcchat(*nd);
	return 0;
}
/*==========================================
 * npcチャット全員蹴り出す
 *------------------------------------------
 */
int buildin_waitingroomkickall(struct script_state &st)
{
	struct npc_data *nd;
	struct chat_data *cd;

	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd==NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id))==NULL )
		return 0;
	chat_npckickall(*cd);
	return 0;
}

/*==========================================
 * npcチャットイベント有効化
 *------------------------------------------
 */
int buildin_enablewaitingroomevent(struct script_state &st)
{
	struct npc_data *nd;
	struct chat_data *cd;

	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd==NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id))==NULL )
		return 0;
	chat_enableevent(*cd);
	return 0;
}

/*==========================================
 * npcチャットイベント無効化
 *------------------------------------------
 */
int buildin_disablewaitingroomevent(struct script_state &st)
{
	struct npc_data *nd;
	struct chat_data *cd;

	if( st.end > st.start+2 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+2])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd==NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id))==NULL )
		return 0;
	chat_disableevent(*cd);
	return 0;
}
/*==========================================
 * npcチャット状態所得
 *------------------------------------------
 */
int buildin_getwaitingroomstate(struct script_state &st)
{
	struct npc_data *nd;
	struct chat_data *cd;
	int val=0,type;
	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	if( st.end > st.start+3 )
		nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+3])));
	else
		nd=(struct npc_data *)map_id2bl(st.oid);

	if(nd==NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id))==NULL ){
		push_val(st.stack,C_INT,-1);
		return 0;
	}

	switch(type){
	case 0: val=cd->users; break;
	case 1: val=cd->limit; break;
	case 2: val=cd->trigger&0x7f; break;
	case 3: val=((cd->trigger&0x80)>0); break;
	case 32: val=(cd->users >= cd->limit); break;
	case 33: val=(cd->users >= cd->trigger); break;

	case 4:
		push_str(st.stack,C_CONSTSTR, cd->title);
		return 0;
	case 5:
		push_str(st.stack,C_CONSTSTR, cd->pass);
		return 0;
	case 16:
		push_str(st.stack,C_CONSTSTR, cd->npc_event);
		return 0;
	}
	push_val(st.stack,C_INT,val);
	return 0;
}

/*==========================================
 * チャットメンバー(規定人数)ワープ
 *------------------------------------------
 */
int buildin_warpwaitingpc(struct script_state &st)
{
	int x,y,i,n;
	const char *str;
	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	struct chat_data *cd;

	if(nd==NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id))==NULL )
		return 0;

	n=cd->trigger&0x7f;
	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	x=conv_num(st, (st.stack.stack_data[st.start+3]));
	y=conv_num(st, (st.stack.stack_data[st.start+4]));

	if( st.end > st.start+5 )
		n=conv_num(st, (st.stack.stack_data[st.start+5]));

	for(i=0;i<n;i++){
		struct map_session_data *sd=cd->usersd[0];	// リスト先頭のPCを次々に。

		mapreg_setreg(add_str( "$@warpwaitingpc")+(i<<24),sd->bl.id);

		if(strcmp(str,"Random")==0)
			pc_randomwarp(*sd,3);
		else if(strcmp(str,"SavePoint")==0){
			if(map[sd->bl.m].flag.noteleport)	// テレポ禁止
				return 0;

			pc_setpos(*sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
		}else
			pc_setpos(*sd,str,x,y,0);
	}
	mapreg_setreg(add_str( "$@warpwaitingpcnum"),n);
	return 0;
}
/*==========================================
 * RIDのアタッチ
 *------------------------------------------
 */
int buildin_attachrid(struct script_state &st)
{
	st.rid=conv_num(st, (st.stack.stack_data[st.start+2]));
	push_val(st.stack,C_INT, (map_id2sd(st.rid)!=NULL));
	return 0;
}
/*==========================================
 * RIDのデタッチ
 *------------------------------------------
 */
int buildin_detachrid(struct script_state &st)
{
	st.rid=0;
	return 0;
}
/*==========================================
 * 存在チェック
 *------------------------------------------
 */
int buildin_isloggedin(struct script_state &st)
{
	push_val(st.stack,C_INT, map_id2sd(
		conv_num(st, (st.stack.stack_data[st.start+2])) )!=NULL );
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
	MF_FIREWORKS, 		//24
	MF_GVG_DUNGEON		//25
};



int buildin_setmapflagnosave(struct script_state &st)
{
	int m,x,y;
	const char *str,*str2;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	str2=conv_str(st, (st.stack.stack_data[st.start+3]));
	x=conv_num(st, (st.stack.stack_data[st.start+4]));
	y=conv_num(st, (st.stack.stack_data[st.start+5]));
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0) {
		map[m].flag.nosave=1;
		memcpy(map[m].save.map,str2,16);
		map[m].save.x=x;
		map[m].save.y=y;
	}

	return 0;
}

int buildin_setmapflag(struct script_state &st)
{
	int m,i;
	const char *str;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	i=conv_num(st, (st.stack.stack_data[st.start+3]));
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:
				map[m].flag.nomemo=1;
				break;
			case MF_NOTELEPORT:
				map[m].flag.noteleport=1;
				break;
			case MF_NOBRANCH:
				map[m].flag.nobranch=1;
				break;
			case MF_NOPENALTY:
				map[m].flag.nopenalty=1;
				break;
			case MF_NOZENYPENALTY:
				map[m].flag.nozenypenalty=1;
				break;
			case MF_PVP:
				map[m].flag.pvp=1;
				break;
			case MF_PVP_NOPARTY:
				map[m].flag.pvp_noparty=1;
				break;
			case MF_PVP_NOGUILD:
				map[m].flag.pvp_noguild=1;
				break;
			case MF_GVG:
				map[m].flag.gvg=1;
				break;
			case MF_GVG_NOPARTY:
				map[m].flag.gvg_noparty=1;
				break;
			case MF_GVG_DUNGEON:
				map[m].flag.gvg_dungeon=1;
				break;
			case MF_NOTRADE:
				map[m].flag.notrade=1;
				break;
			case MF_NOSKILL:
				map[m].flag.noskill=1;
				break;
			case MF_NOWARP:
				map[m].flag.nowarp=1;
				break;
			case MF_NOPVP:
				map[m].flag.nopvp=1;
				break;
			case MF_NOICEWALL: // [Valaris]
				map[m].flag.noicewall=1;
				break;
			case MF_SNOW: // [Valaris]
				map[m].flag.snow=1;
				break;
			case MF_CLOUDS:
				map[m].flag.clouds=1;
				break;
			case MF_FOG: // [Valaris]
				map[m].flag.fog=1;
				break;
			case MF_FIREWORKS:
				map[m].flag.fireworks=1;
				break;
			case MF_SAKURA: // [Valaris]
				map[m].flag.sakura=1;
				break;
			case MF_LEAVES: // [Valaris]
				map[m].flag.leaves=1;
				break;
			case MF_RAIN: // [Valaris]
				map[m].flag.rain=1;
				break;
			case MF_INDOORS: // celest
				map[m].flag.indoors=1;
				break;
			case MF_NOGO: // celest
				map[m].flag.nogo=1;
				break;
		}
	}
	return 0;
}

int buildin_removemapflag(struct script_state &st)
{
	int m,i;
	const char *str;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	i=conv_num(st, (st.stack.stack_data[st.start+3]));
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:
				map[m].flag.nomemo=0;
				break;
			case MF_NOTELEPORT:
				map[m].flag.noteleport=0;
				break;
			case MF_NOSAVE:
				map[m].flag.nosave=0;
				break;
			case MF_NOBRANCH:
				map[m].flag.nobranch=0;
				break;
			case MF_NOPENALTY:
				map[m].flag.nopenalty=0;
				break;
			case MF_PVP:
				map[m].flag.pvp=0;
				break;
			case MF_PVP_NOPARTY:
				map[m].flag.pvp_noparty=0;
				break;
			case MF_PVP_NOGUILD:
				map[m].flag.pvp_noguild=0;
				break;
			case MF_GVG:
				map[m].flag.gvg=0;
				break;
			case MF_GVG_NOPARTY:
				map[m].flag.gvg_noparty=0;
				break;
			case MF_GVG_DUNGEON:
				map[m].flag.gvg_dungeon=0;
				break;
			case MF_NOZENYPENALTY:
				map[m].flag.nozenypenalty=0;
				break;
			case MF_NOSKILL:
				map[m].flag.noskill=0;
				break;
			case MF_NOWARP:
				map[m].flag.nowarp=0;
				break;
			case MF_NOPVP:
				map[m].flag.nopvp=0;
				break;
			case MF_NOICEWALL: // [Valaris]
				map[m].flag.noicewall=0;
				break;
			case MF_SNOW: // [Valaris]
				map[m].flag.snow=0;
				break;
			case MF_CLOUDS:
				map[m].flag.clouds=0;
				break;
			case MF_FOG: // [Valaris]
				map[m].flag.fog=0;
				break;
			case MF_FIREWORKS:
				map[m].flag.fireworks=0;
				break;
			case MF_SAKURA: // [Valaris]
				map[m].flag.sakura=0;
				break;
			case MF_LEAVES: // [Valaris]
				map[m].flag.leaves=0;
				break;
			case MF_RAIN: // [Valaris]
				map[m].flag.rain=0;
				break;
			case MF_INDOORS: // celest
				map[m].flag.indoors=0;
				break;
			case MF_NOGO: // celest
				map[m].flag.nogo=0;
				break;
		}
	}

	return 0;
}

int buildin_pvpon(struct script_state &st)
{
	size_t i;
	short m;
	const char *str;
	struct map_session_data *pl_sd=NULL;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && !map[m].flag.pvp && !map[m].flag.nopvp) {
		map[m].flag.pvp = 1;
		clif_send0199(m,1);

		if(battle_config.pk_mode) // disable ranking functions if pk_mode is on [Valaris]
			return 0;

		for(i=0;i<fd_max;i++){	//人数分ループ
			if(session[i] && (pl_sd=(struct map_session_data *) session[i]->session_data) && pl_sd->state.auth){
				if(m == pl_sd->bl.m && pl_sd->pvp_timer == -1) {
					pl_sd->pvp_timer=add_timer(gettick()+200,pc_calc_pvprank_timer,pl_sd->bl.id,0);
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

int buildin_pvpoff(struct script_state &st)
{
	size_t i;
	short m;
	const char *str;
	struct map_session_data *pl_sd=NULL;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && map[m].flag.pvp && map[m].flag.nopvp) {
		map[m].flag.pvp = 0;
		clif_send0199(m,0);

		if(battle_config.pk_mode) // disable ranking options if pk_mode is on [Valaris]
			return 0;

		for(i=0;i<fd_max;i++){	//人数分ループ
			if(session[i] && (pl_sd=(struct map_session_data *) session[i]->session_data) && pl_sd->state.auth){
				if(m == pl_sd->bl.m) {
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

int buildin_gvgon(struct script_state &st)
{
	int m;
	const char *str;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && !map[m].flag.gvg) {
		map[m].flag.gvg = 1;
		clif_send0199(m,3);
	}

	return 0;
}
int buildin_gvgoff(struct script_state &st)
{
	int m;
	const char *str;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	m = map_mapname2mapid(str);
//!! broadcast command if not on this mapserver
	if(m >= 0 && map[m].flag.gvg) {
		map[m].flag.gvg = 0;
		clif_send0199(m,0);
	}

	return 0;
}
/*==========================================
 *	NPCエモーション
 *------------------------------------------
 */

int buildin_emotion(struct script_state &st)
{
	int type;
	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	if(type < 0 || type > 100)
		return 0;
	block_list *bl = map_id2bl(st.oid);
	if(bl) clif_emotion(*bl,type);
	return 0;
}

int buildin_maprespawnguildid_sub(struct block_list &bl,va_list ap)
{
	unsigned long g_id=va_arg(ap,unsigned long);
	int flag=va_arg(ap,int);
	struct map_session_data *sd=NULL;
	struct mob_data *md=NULL;

	if(bl.type == BL_PC)
		sd=(struct map_session_data*)&bl;
	else if(bl.type == BL_MOB)
		md=(struct mob_data *)&bl;

	if(sd){
		if((sd->status.guild_id == g_id) && (flag&1))
			pc_setpos(*sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
		else if((sd->status.guild_id != g_id) && (flag&2))
			pc_setpos(*sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
		else if (sd->status.guild_id == 0)	// Warp out players not in guild [Valaris]
			pc_setpos(*sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);	// end addition [Valaris]
	}
	else if(md && flag&4){
		if(md->class_ < 1285 || md->class_ > 1288)
			mob_delete(*md);
	}
	return 0;
}
int buildin_maprespawnguildid(struct script_state &st)
{
	const char *mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	int g_id=conv_num(st, (st.stack.stack_data[st.start+3]));
	int flag=conv_num(st, (st.stack.stack_data[st.start+4]));

	int m=map_mapname2mapid(mapname);

	if(m) map_foreachinarea(buildin_maprespawnguildid_sub,m,0,0,map[m].xs-1,map[m].ys-1,BL_NUL,g_id,flag);
	return 0;
}

int buildin_agitstart(struct script_state &st)
{
	if(agit_flag==1) return 1;      // Agit already Start.
	agit_flag=1;
	guild_agit_start();
	return 0;
}

int buildin_agitend(struct script_state &st)
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
int buildin_agitcheck(struct script_state &st)
{
	int cond=conv_num(st, (st.stack.stack_data[st.start+2]));
	if(cond == 0) {
		if (agit_flag==1) push_val(st.stack,C_INT,1);
		if (agit_flag==0) push_val(st.stack,C_INT,0);
	} else {
		struct map_session_data *sd=script_rid2sd(st);
		if (agit_flag==1) pc_setreg(*sd,add_str( "@agit_flag"),1);
		if (agit_flag==0) pc_setreg(*sd,add_str( "@agit_flag"),0);
	}
	return 0;
}

int buildin_flagemblem(struct script_state &st)
{
	int g_id=conv_num(st, (st.stack.stack_data[st.start+2]));

	if(g_id < 0) return 0;

//	ShowMessage("Script.c: [FlagEmblem] GuildID=%d, Emblem=%d.\n", g->guild_id, g->emblem_id);
	((struct npc_data *)map_id2bl(st.oid))->u.scr.guild_id = g_id;
	return 1;
}

int buildin_getcastlename(struct script_state &st)
{
	const char *mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	struct guild_castle *gc;
	int i;
	char *buf=NULL;
	for(i=0;i<MAX_GUILDCASTLE;i++)
	{
		if( (gc=guild_castle_search(i)) != NULL )
		{
			if(strcmp(mapname,gc->map_name)==0)
			{
				buf=(char *)aMalloc(24*sizeof(char));
				memcpy(buf,gc->castle_name,24);//EOS included
				break;
			}
		}
	}
	if(buf)
		push_str(st.stack,C_STR, buf);
	else
		push_str(st.stack,C_CONSTSTR, "");
	return 0;
}

int buildin_getcastledata(struct script_state &st)
{
	const char *mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	int index=conv_num(st, (st.stack.stack_data[st.start+3]));
	struct guild_castle *gc;
	int i, val=0;

	for(i=0;i<MAX_GUILDCASTLE;i++)
	{
		if( (gc=guild_castle_search(i)) != NULL && 0==strcmp(mapname,gc->map_name) )
		{
			switch(index)
			{
			case  0:
				if(st.end>st.start+4)
				{
					const char *event = conv_str(st, (st.stack.stack_data[st.start+4]));
					guild_addcastleinfoevent(i,17,event);
				}
				for(i=1;i<26;i++) guild_castledataload(gc->castle_id,i);

				return 0;	// no idea why 
							// but don't push a value up to the stack in this case
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
			case 10: val = gc->visibleG0; break;
			case 11: val = gc->visibleG1; break;
			case 12: val = gc->visibleG2; break;
			case 13: val = gc->visibleG3; break;
			case 14: val = gc->visibleG4; break;
			case 15: val = gc->visibleG5; break;
			case 16: val = gc->visibleG6; break;
			case 17: val = gc->visibleG7; break;
			case 18: val = gc->Ghp0; break;
			case 19: val = gc->Ghp1; break;
			case 20: val = gc->Ghp2; break;
			case 21: val = gc->Ghp3; break;
			case 22: val = gc->Ghp4; break;
			case 23: val = gc->Ghp5; break;
			case 24: val = gc->Ghp6; break;
			case 25: val = gc->Ghp7; break;
				}
			break;
			}
		}
	push_val(st.stack,C_INT,val);
	return 0;
}

int buildin_setcastledata(struct script_state &st)
{
	const char *mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	int index=conv_num(st, (st.stack.stack_data[st.start+3]));
	int value=conv_num(st, (st.stack.stack_data[st.start+4]));
	struct guild_castle *gc;
	int i;

	for(i=0;i<MAX_GUILDCASTLE;i++){
		if( (gc=guild_castle_search(i)) != NULL ){
			if(strcmp(mapname,gc->map_name)==0){
				// Save Data byself First
				switch(index){
				case 1: gc->guild_id = value; break;
				case 2: gc->economy = value; break;
				case 3: gc->defense = value; break;
				case 4: gc->triggerE = value; break;
				case 5: gc->triggerD = value; break;
				case 6: gc->nextTime = value; break;
				case 7: gc->payTime = value; break;
				case 8: gc->createTime = value; break;
				case 9: gc->visibleC = value; break;
				case 10: gc->visibleG0 = value; break;
				case 11: gc->visibleG1 = value; break;
				case 12: gc->visibleG2 = value; break;
				case 13: gc->visibleG3 = value; break;
				case 14: gc->visibleG4 = value; break;
				case 15: gc->visibleG5 = value; break;
				case 16: gc->visibleG6 = value; break;
				case 17: gc->visibleG7 = value; break;
				case 18: gc->Ghp0 = value; break;
				case 19: gc->Ghp1 = value; break;
				case 20: gc->Ghp2 = value; break;
				case 21: gc->Ghp3 = value; break;
				case 22: gc->Ghp4 = value; break;
				case 23: gc->Ghp5 = value; break;
				case 24: gc->Ghp6 = value; break;
				case 25: gc->Ghp7 = value; break;
				default: return 0;
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
int buildin_requestguildinfo(struct script_state &st)
{
	int guild_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	const char *event=NULL;

	if( st.end>st.start+3 )
		event=conv_str(st, (st.stack.stack_data[st.start+3]));

	if(guild_id>0)
		guild_npc_request_info(guild_id,event);
	return 0;
}

/* =====================================================================
 * カードの数を得る
 * ---------------------------------------------------------------------
 */
int buildin_getequipcardcnt(struct script_state &st)
{
	int i,num;
	struct map_session_data *sd;
	int c=4;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	if(!sd)
	{
		push_val(st.stack,C_INT,0);
		return 0;
	}
	i=pc_checkequip(*sd,equip[num-1]);
	if(sd->status.inventory[i].card[0] == 0x00ff){ // 製造武器はカードなし
		push_val(st.stack,C_INT,0);
		return 0;
	}
	do{
		if( (sd->status.inventory[i].card[c-1] > 4000 &&
			sd->status.inventory[i].card[c-1] < 5000) ||
			itemdb_type(sd->status.inventory[i].card[c-1]) == 6){	// [Celest]
			push_val(st.stack,C_INT,(c));
			return 0;
		}
	}while(c--);
	push_val(st.stack,C_INT,0);
	return 0;
}

/* ================================================================
 * カード取り外し成功
 * ----------------------------------------------------------------
 */
int buildin_successremovecards(struct script_state &st)
{
	int i,num,cardflag=0,flag;
	struct map_session_data *sd;
	struct item item_tmp;
	int c=4;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	sd=script_rid2sd(st);
	i=pc_checkequip(*sd,equip[num-1]);
	if(sd->status.inventory[i].card[0]==0x00ff){ // 製造武器は処理しない
		return 0;
	}
	do{
		if( (sd->status.inventory[i].card[c-1] > 4000 &&
			sd->status.inventory[i].card[c-1] < 5000) ||
			itemdb_type(sd->status.inventory[i].card[c-1]) == 6){	// [Celest]

			cardflag = 1;
			item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].card[c-1];
			item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=0;
			item_tmp.attribute=0;
			item_tmp.card[0]=0,item_tmp.card[1]=0,item_tmp.card[2]=0,item_tmp.card[3]=0;

			if((flag=pc_additem(*sd,item_tmp,1))){	// 持てないならドロップ
				clif_additem(*sd,0,0,flag);
				map_addflooritem(item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,NULL,NULL,NULL,0);
			}
		}
	}while(c--);

	if(cardflag == 1){	// カードを取り除いたアイテム所得
		flag=0;
		item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].nameid;
		item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=sd->status.inventory[i].refine;
		item_tmp.attribute=sd->status.inventory[i].attribute;
		item_tmp.card[0]=0,item_tmp.card[1]=0,item_tmp.card[2]=0,item_tmp.card[3]=0;
		pc_delitem(*sd,i,1,0);
		if((flag=pc_additem(*sd,item_tmp,1))){	// もてないならドロップ
			clif_additem(*sd,0,0,flag);
			map_addflooritem(item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,NULL,NULL,NULL,0);
		}
		clif_misceffect(sd->bl,3);
		return 0;
	}
	return 0;
}

/* ================================================================
 * カード取り外し失敗 slot,type
 * type=0: 両方損失、1:カード損失、2:武具損失、3:損失無し
 * ----------------------------------------------------------------
 */
int buildin_failedremovecards(struct script_state &st)
{
	int i,num,cardflag=0,flag,typefail;
	struct map_session_data *sd;
	struct item item_tmp;
	int c=4;

	num=conv_num(st, (st.stack.stack_data[st.start+2]));
	typefail=conv_num(st, (st.stack.stack_data[st.start+3]));
	sd=script_rid2sd(st);
	i=pc_checkequip(*sd,equip[num-1]);
	if(sd->status.inventory[i].card[0]==0x00ff){ // 製造武器は処理しない
		return 0;
	}
	do{
		if( (sd->status.inventory[i].card[c-1] > 4000 &&
			sd->status.inventory[i].card[c-1] < 5000) ||
			itemdb_type(sd->status.inventory[i].card[c-1]) == 6){	// [Celest]

			cardflag = 1;

			if(typefail == 2){ // 武具のみ損失なら、カードは受け取らせる
				item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].card[c-1];
				item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=0;
				item_tmp.attribute=0;
				item_tmp.card[0]=0,item_tmp.card[1]=0,item_tmp.card[2]=0,item_tmp.card[3]=0;
				if((flag=pc_additem(*sd,item_tmp,1))){
					clif_additem(*sd,0,0,flag);
					map_addflooritem(item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,NULL,NULL,NULL,0);
				}
			}
		}
	}while(c--);

	if(cardflag == 1){

		if(typefail == 0 || typefail == 2){	// 武具損失
			pc_delitem(*sd,i,1,0);
			clif_misceffect(sd->bl,2);
			return 0;
		}
		if(typefail == 1){	// カードのみ損失（武具を返す）
			flag=0;
			item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].nameid;
			item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=sd->status.inventory[i].refine;
			item_tmp.attribute=sd->status.inventory[i].attribute;
			item_tmp.card[0]=0,item_tmp.card[1]=0,item_tmp.card[2]=0,item_tmp.card[3]=0;
			pc_delitem(*sd,i,1,0);
			if((flag=pc_additem(*sd,item_tmp,1))){
				clif_additem(*sd,0,0,flag);
				map_addflooritem(item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,NULL,NULL,NULL,0);
			}
		}
		clif_misceffect(sd->bl,2);
		return 0;
	}
	return 0;
}

int buildin_mapwarp(struct script_state &st)	// Added by RoVeRT
{
	int x,y,m;
	const char *str;
	const char *mapname;
	int x0,y0,x1,y1;

	mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	x0=0;
	y0=0;
	x1=map[map_mapname2mapid(mapname)].xs;
	y1=map[map_mapname2mapid(mapname)].ys;
	str=conv_str(st, (st.stack.stack_data[st.start+3]));
	x=conv_num(st, (st.stack.stack_data[st.start+4]));
	y=conv_num(st, (st.stack.stack_data[st.start+5]));

	if( (m=map_mapname2mapid(mapname))< 0)
		return 0;
//!! broadcast command if not on this mapserver
	map_foreachinarea(buildin_areawarp_sub,
		m,x0,y0,x1,y1,BL_PC,	str,x,y );
	return 0;
}

int buildin_cmdothernpc(struct script_state &st)	// Added by RoVeRT
{
	const char *npc,*command;

	npc=conv_str(st, (st.stack.stack_data[st.start+2]));
	command=conv_str(st, (st.stack.stack_data[st.start+3]));

	map_session_data *sd = map_id2sd(st.rid);
	if(sd) npc_command(*sd,npc,command);
	return 0;
}

int buildin_inittimer(struct script_state &st)	// Added by RoVeRT
{
//	struct npc_data *nd=(struct npc_data*)map_id2bl(st.oid);
//	nd->lastaction=nd->timer=gettick();

	map_session_data *sd = map_id2sd(st.rid);
	if(sd) npc_do_ontimer(st.oid, *sd, 1);
	return 0;
}

int buildin_stoptimer(struct script_state &st)	// Added by RoVeRT
{
//	struct npc_data *nd=(struct npc_data*)map_id2bl(st.oid);
//	nd->lastaction=nd->timer=-1;

	map_session_data *sd = map_id2sd(st.rid);
	if(sd) npc_do_ontimer(st.oid, *sd, 0);

	return 0;
}

int buildin_mobcount_sub(struct block_list &bl,va_list ap)	// Added by RoVeRT
{
	char *event=va_arg(ap,char *);
	int *c=va_arg(ap,int *);

	if(strcmp(event,((struct mob_data *)&bl)->npc_event)==0)
		(*c)++;
	return 0;
}

int buildin_mobcount(struct script_state &st)	// Added by RoVeRT
{
	const char *mapname,*event;
	int m,c=0;
	mapname=conv_str(st, (st.stack.stack_data[st.start+2]));
	event=conv_str(st, (st.stack.stack_data[st.start+3]));

	if( (m=map_mapname2mapid(mapname))<0 ) {
		push_val(st.stack,C_INT,-1);
		return 0;
	}
	map_foreachinarea(buildin_mobcount_sub,
		m,0,0,map[m].xs,map[m].ys,BL_MOB, event,&c );

	push_val(st.stack,C_INT, (c));

	return 0;
}
int buildin_marriage(struct script_state &st)
{
	const char *partner=conv_str(st, (st.stack.stack_data[st.start+2]));
	struct map_session_data *sd=script_rid2sd(st);
	struct map_session_data *p_sd=map_nick2sd(partner);

	push_val(st.stack,C_INT, (sd!=NULL && p_sd!=NULL && pc_marriage(*sd,*p_sd)) );

		return 0;
	}
int buildin_wedding_effect(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	struct block_list *bl;

	if(sd==NULL) {
		bl=map_id2bl(st.oid);
	} else
		bl=&sd->bl;
	if(bl) clif_wedding_effect(*bl);
	return 0;
}
int buildin_divorce(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	push_val(st.stack,C_INT, (sd && pc_divorce(*sd)) );
		return 0;
	}

int buildin_ispartneron(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	push_val(st.stack,C_INT,
		(sd && pc_ismarried(*sd) &&
		NULL!=map_nick2sd(map_charid2nick(sd->status.partner_id)))
		);
		return 0;
	}

int buildin_getpartnerid(struct script_state &st)
{
    struct map_session_data *sd=script_rid2sd(st);
    if (sd == NULL) {
        push_val(st.stack,C_INT,0);
        return 0;
    }
    push_val(st.stack,C_INT,sd->status.partner_id);
    return 0;
}

int buildin_warppartner(struct script_state &st)
{
	int x,y;
	const char *str;
	struct map_session_data *sd=script_rid2sd(st);
	struct map_session_data *p_sd=NULL;

	if( sd==NULL || !pc_ismarried(*sd) ||
		((p_sd=map_nick2sd(map_charid2nick(sd->status.partner_id))) == NULL))
	{
		push_val(st.stack,C_INT,0);
		return 0;
	}

	str=conv_str(st, (st.stack.stack_data[st.start+2]));
	x=conv_num(st, (st.stack.stack_data[st.start+3]));
	y=conv_num(st, (st.stack.stack_data[st.start+4]));

	pc_setpos(*p_sd,str,x,y,0);

	push_val(st.stack,C_INT,1);
	return 0;
}

/*================================================
 * Script for Displaying MOB Information [Valaris]
 *------------------------------------------------
 */
int buildin_strmobinfo(struct script_state &st)
{

	int num=conv_num(st, (st.stack.stack_data[st.start+2]));
	int class_=conv_num(st, (st.stack.stack_data[st.start+3]));

	if((class_>=0 && class_<=1000) || class_ >2000)
		return 0;

	switch (num) {
	case 1:
		{
			char *buf;
		buf = (char*)aMalloc(24*sizeof(char));
			strcpy(buf,mob_db[class_].name);
			push_str(st.stack,C_STR, buf);
			break;
		}
	case 2:
		{
			char *buf;
		buf=(char*)aMalloc(24*sizeof(char));
			strcpy(buf,mob_db[class_].jname);
			push_str(st.stack,C_STR, buf);
			break;
		}
	case 3:
		push_val(st.stack,C_INT,mob_db[class_].lv);
		break;
	case 4:
		push_val(st.stack,C_INT,mob_db[class_].max_hp);
		break;
	case 5:
		push_val(st.stack,C_INT,mob_db[class_].max_sp);
		break;
	case 6:
		push_val(st.stack,C_INT,mob_db[class_].base_exp);
		break;
	case 7:
		push_val(st.stack,C_INT,mob_db[class_].job_exp);
		break;
	}
	return 0;
}

/*==========================================
 * Summon guardians [Valaris]
 *------------------------------------------
 */
int buildin_guardian(struct script_state &st)
{
	int class_=0,amount=1,x=0,y=0,guardian=0;
	const char *str,*map,*event="";

	map	=conv_str(st, (st.stack.stack_data[st.start+2]));
	x	=conv_num(st, (st.stack.stack_data[st.start+3]));
	y	=conv_num(st, (st.stack.stack_data[st.start+4]));
	str	=conv_str(st, (st.stack.stack_data[st.start+5]));
	class_=conv_num(st, (st.stack.stack_data[st.start+6]));
	amount=conv_num(st, (st.stack.stack_data[st.start+7]));
	event=conv_str(st, (st.stack.stack_data[st.start+8]));
	if( st.end>st.start+9 )
		guardian=conv_num(st, (st.stack.stack_data[st.start+9]));

	mob_spawn_guardian(map_id2sd(st.rid),map,x,y,str,class_,amount,event,guardian);

	return 0;
}

/*================================================
 * Script for Displaying Guardian Info [Valaris]
 *------------------------------------------------
 */
int buildin_guardianinfo(struct script_state &st)
{
	int guardian=conv_num(st, (st.stack.stack_data[st.start+2]));
	struct map_session_data *sd=script_rid2sd(st);
	struct guild_castle *gc=guild_mapname2gc(map[sd->bl.m].mapname);

	if(guardian==0 && gc->visibleG0 == 1) push_val(st.stack,C_INT,gc->Ghp0);
	if(guardian==1 && gc->visibleG1 == 1) push_val(st.stack,C_INT,gc->Ghp1);
	if(guardian==2 && gc->visibleG2 == 1) push_val(st.stack,C_INT,gc->Ghp2);
	if(guardian==3 && gc->visibleG3 == 1) push_val(st.stack,C_INT,gc->Ghp3);
	if(guardian==4 && gc->visibleG4 == 1) push_val(st.stack,C_INT,gc->Ghp4);
	if(guardian==5 && gc->visibleG5 == 1) push_val(st.stack,C_INT,gc->Ghp5);
	if(guardian==6 && gc->visibleG6 == 1) push_val(st.stack,C_INT,gc->Ghp6);
	if(guardian==7 && gc->visibleG7 == 1) push_val(st.stack,C_INT,gc->Ghp7);
	else push_val(st.stack,C_INT,-1);

	return 0;
}
/*==========================================
 * IDからItem名
 *------------------------------------------
 */
int buildin_getitemname(struct script_state &st)
{
	int item_id=conv_num(st, (st.stack.stack_data[st.start+2]));
	struct item_data *i_data = itemdb_exists(item_id);

	if(i_data)
	{
		char *item_name;
		item_name=(char *)aMalloc(24*sizeof(char));
		memcpy(item_name,i_data->jname,24);//EOS included
		push_str(st.stack,C_STR, item_name);
	}
	else
		push_str(st.stack,C_CONSTSTR, "unknown");
	return 0;
}

/*==========================================
 * petskillbonus [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */

int buildin_petskillbonus(struct script_state &st)
{
	struct pet_data *pd;

	struct map_session_data *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->bonus)
	{ //Clear previous bonus
		if (pd->bonus->timer != -1)
			delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
	} else //init
		pd->bonus = (struct pet_data::pet_bonus *) aCalloc(1, sizeof(struct pet_data::pet_bonus));

	pd->bonus->type=conv_num(st, (st.stack.stack_data[st.start+2]));
	pd->bonus->val=conv_num(st, (st.stack.stack_data[st.start+3]));
	pd->bonus->duration=conv_num(st, (st.stack.stack_data[st.start+4]));
	pd->bonus->delay=conv_num(st, (st.stack.stack_data[st.start+5]));

	if (pd->state.skillbonus == -1)
		pd->state.skillbonus=0;	// waiting state

	// wait for timer to start
	if (battle_config.pet_equip_required && pd->equip_id == 0)
		pd->bonus->timer=-1;
	else
		pd->bonus->timer=add_timer(gettick()+pd->bonus->delay*1000, pet_skill_bonus_timer, sd->bl.id, 0);

	return 0;
}

/*==========================================
 * pet looting [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petloot(struct script_state &st)
{
	int max;
	struct pet_data *pd;
	struct map_session_data *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return 0;

	max=conv_num(st, (st.stack.stack_data[st.start+2]));

	if(max < 1)
		max = 1;	//Let'em loot at least 1 item.
	else if (max > MAX_PETLOOT_SIZE)
		max = MAX_PETLOOT_SIZE;
	
	pd = sd->pd;
	if(pd && pd->loot != NULL && pd->msd)
	{	//Release whatever was there already and reallocate memory
		pet_lootitem_drop(*pd, pd->msd);
		aFree(pd->loot->item);
	}
	else
		pd->loot = (struct pet_data::pet_loot *)aCalloc(1, sizeof(struct pet_data::pet_loot));

	pd->loot->item = (struct item *)aCalloc(max,sizeof(struct item));
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
int buildin_getinventorylist(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	int i,j=0;
	if(!sd) return 0;
	for(i=0;i<MAX_INVENTORY;i++){
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

int buildin_getskilllist(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	int i,j=0;
	if(!sd) return 0;
	for(i=0;i<MAX_SKILL;i++){
		if(sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0){
			pc_setreg(*sd,add_str( "@skilllist_id")+(j<<24),sd->status.skill[i].id);
			pc_setreg(*sd,add_str("@skilllist_lv")+(j<<24),sd->status.skill[i].lv);
			pc_setreg(*sd,add_str("@skilllist_flag")+(j<<24),sd->status.skill[i].flag);
			j++;
		}
	}
	pc_setreg(*sd,add_str( "@skilllist_count"),j);
	return 0;
}

int buildin_clearitem(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	int i;
	if(sd==NULL) return 0;
	for (i=0; i<MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount)
			pc_delitem(*sd, i, sd->status.inventory[i].amount, 0);
	}
	return 0;
}

/*==========================================
 * NPCクラスチェンジ
 * classは変わりたいclass
 * typeは通常0なのかな？
 *------------------------------------------
 */
int buildin_classchange(struct script_state &st)
{
	int class_,type;
	struct block_list *bl=map_id2bl(st.oid);

	if(bl)
	{
		class_=conv_num(st, (st.stack.stack_data[st.start+2]));
		type=conv_num(st, (st.stack.stack_data[st.start+3]));
		clif_class_change(*bl,class_,type);
	}
	return 0;
}

/*==========================================
 * NPCから発生するエフェクト
 *------------------------------------------
 */
int buildin_misceffect(struct script_state &st)
{
	int type;
	type=conv_num(st, (st.stack.stack_data[st.start+2]));
	if(st.oid)
	{	block_list *bl = map_id2bl(st.oid);
		if(bl) clif_misceffect2(*bl,type);
	}
	else
	{
		struct map_session_data *sd=script_rid2sd(st);
		if(sd) clif_misceffect2(sd->bl,type);
//!! broadcast command if not on this mapserver
	}
	return 0;
}
/*==========================================
 * サウンドエフェクト
 *------------------------------------------
 */
int buildin_soundeffect(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	const char *name;
	int type=0;
	name=conv_str(st, (st.stack.stack_data[st.start+2]));
	type=conv_num(st, (st.stack.stack_data[st.start+3]));
	if(sd)
	{
		if(st.oid)
		{	block_list *bl = map_id2bl(st.oid);
			if(bl) clif_soundeffect(*sd,*bl,name,type);
		}
		else
		{	clif_soundeffect(*sd,sd->bl,name,type);
	}
	}
	return 0;
}

int buildin_soundeffectall(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	block_list *bl;
	const char *name;
	int type=0;

	name=conv_str(st, (st.stack.stack_data[st.start+2]));
	type=conv_num(st, (st.stack.stack_data[st.start+3]));
	
	if(st.oid && (bl=map_id2bl(st.oid))!=NULL)
		clif_soundeffectall(*bl,name,type);
	else if(sd)
		clif_soundeffectall(sd->bl,name,type);
	return 0;
}
/*==========================================
 * pet status recovery [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petrecovery(struct script_state &st)
{
	struct pet_data *pd;
	struct map_session_data *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	
	if (pd->recovery)
	{ //Halt previous bonus
		if (pd->recovery->timer != -1)
			delete_timer(pd->recovery->timer, pet_recovery_timer);
	}
	else //Init
		pd->recovery = (struct pet_data::pet_recovery *)aCalloc(1, sizeof(struct pet_data::pet_recovery));
		
	pd->recovery->type=conv_num(st, (st.stack.stack_data[st.start+2]));
	pd->recovery->delay=conv_num(st, (st.stack.stack_data[st.start+3]));
	pd->recovery->timer=-1;

	return 0;
}

/*==========================================
 * pet healing [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petheal(struct script_state &st)
{
	struct pet_data *pd;
	struct map_session_data *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != -1)
			delete_timer(pd->s_skill->timer, pet_skill_support_timer);
	} else //init memory
		pd->s_skill = (struct pet_data::pet_skill_support *) aCalloc(1, sizeof(struct pet_data::pet_skill_support)); 
	
	pd->s_skill->id=0; //This id identifies that it IS petheal rather than pet_skillsupport
	//Use the lv as the amount to heal
	pd->s_skill->lv=conv_num(st, (st.stack.stack_data[st.start+2]));
	pd->s_skill->delay=conv_num(st, (st.stack.stack_data[st.start+3]));
	pd->s_skill->hp=conv_num(st, (st.stack.stack_data[st.start+4]));
	pd->s_skill->sp=conv_num(st, (st.stack.stack_data[st.start+5]));

	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->equip_id == 0)
		pd->s_skill->timer=-1;
	else
		pd->s_skill->timer=add_timer(gettick()+pd->s_skill->delay*1000,pet_heal_timer,sd->bl.id,0);

	return 0;
}

/*==========================================
 * pet magnificat [Valaris]
 *------------------------------------------
 */
/*
int buildin_petmag(struct script_state &st)
{
	struct pet_data *pd;
	struct map_session_data *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;

	if(pd==NULL)
		return 0;

	pd->skilltype=conv_num(st, (st.stack.stack_data[st.start+2]));
	pd->skillduration=conv_num(st, (st.stack.stack_data[st.start+3]));
	pd->skillval=conv_num(st, (st.stack.stack_data[st.start+4]));
	pd->skilltimer=conv_num(st, (st.stack.stack_data[st.start+5]));

	pd->skillbonustimer=add_timer(gettick()+pd->skilltimer*1000,pet_mag_timer,sd->bl.id,0);

	return 0;
}
*/
/*==========================================
 * pet attack skills [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------
 */
int buildin_petskillattack(struct script_state &st)
{
	struct pet_data *pd;
	struct map_session_data *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_data::pet_skill_attack *)aCalloc(1, sizeof(struct pet_data::pet_skill_attack));
				
	pd->a_skill->id=conv_num(st, (st.stack.stack_data[st.start+2]));
	pd->a_skill->lv=conv_num(st, (st.stack.stack_data[st.start+3]));
	pd->a_skill->div_ = 0;
	pd->a_skill->rate=conv_num(st, (st.stack.stack_data[st.start+4]));
	pd->a_skill->bonusrate=conv_num(st, (st.stack.stack_data[st.start+5]));

	return 0;
}

/*==========================================
 * pet attack skills [Valaris]
 *------------------------------------------
 */
int buildin_petskillattack2(struct script_state &st)
{
	struct pet_data *pd;
	struct map_session_data *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_data::pet_skill_attack *)aCalloc(1, sizeof(struct pet_data::pet_skill_attack));
				
	pd->a_skill->id=conv_num(st, (st.stack.stack_data[st.start+2]));
	pd->a_skill->lv=conv_num(st, (st.stack.stack_data[st.start+3]));
	pd->a_skill->div_ = conv_num(st, (st.stack.stack_data[st.start+4]));
	pd->a_skill->rate=conv_num(st, (st.stack.stack_data[st.start+5]));
	pd->a_skill->bonusrate=conv_num(st, (st.stack.stack_data[st.start+6]));

	return 0;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------
 */
int buildin_petskillsupport(struct script_state &st)
{
	struct pet_data *pd;
	struct map_session_data *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != -1)
			delete_timer(pd->s_skill->timer, pet_skill_support_timer);
	} else //init memory
		pd->s_skill = (struct pet_data::pet_skill_support *) aCalloc(1, sizeof(struct pet_data::pet_skill_support)); 
	
	pd->s_skill->id=conv_num(st, (st.stack.stack_data[st.start+2]));
	pd->s_skill->lv=conv_num(st, (st.stack.stack_data[st.start+3]));
	pd->s_skill->delay=conv_num(st, (st.stack.stack_data[st.start+4]));
	pd->s_skill->hp=conv_num(st, (st.stack.stack_data[st.start+5]));
	pd->s_skill->sp=conv_num(st, (st.stack.stack_data[st.start+6]));

	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->equip_id == 0)
		pd->s_skill->timer=-1;
	else
		pd->s_skill->timer=add_timer(gettick()+pd->s_skill->delay*1000,pet_skill_support_timer,sd->bl.id,0);

	return 0;
}

/*==========================================
 * Scripted skill effects [Celest]
 *------------------------------------------
 */
int buildin_skilleffect(struct script_state &st)
{
	struct map_session_data *sd;

	int skillid=conv_num(st, (st.stack.stack_data[st.start+2]));
	int skilllv=conv_num(st, (st.stack.stack_data[st.start+3]));
	sd=script_rid2sd(st);

	clif_skill_nodamage(sd->bl,sd->bl,skillid,skilllv,1);

	return 0;
}

/*==========================================
 * NPC skill effects [Valaris]
 *------------------------------------------
 */
int buildin_npcskilleffect(struct script_state &st)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);

	int skillid=conv_num(st, (st.stack.stack_data[st.start+2]));
	int skilllv=conv_num(st, (st.stack.stack_data[st.start+3]));
	int x=conv_num(st, (st.stack.stack_data[st.start+4]));
	int y=conv_num(st, (st.stack.stack_data[st.start+5]));

	clif_skill_poseffect(nd->bl,skillid,skilllv,x,y,gettick());

	return 0;
}

/*==========================================
 * Special effects [Valaris]
 *------------------------------------------
 */
int buildin_specialeffect(struct script_state &st)
{
	struct block_list *bl=map_id2bl(st.oid);

	if(bl) clif_specialeffect(*bl,conv_num(st, (st.stack.stack_data[st.start+2])), 0);

	return 0;
}

int buildin_specialeffect2(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);

	if(sd) clif_specialeffect(sd->bl,conv_num(st, (st.stack.stack_data[st.start+2])), 0);
		return 0;
}

/*==========================================
 * Nude [Valaris]
 *------------------------------------------
 */

int buildin_nude(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	if(sd)
	{
		size_t i;
		register bool calcflag=false;
		for(i=0;i<MAX_EQUIP;i++)
			if(sd->equip_index[i] >= 0)
			{
				pc_unequipitem(*sd,sd->equip_index[i],2);
				calcflag=true;
			}
		if(calcflag)
			status_calc_pc(*sd,1);
	}
	return 0;
}

/*==========================================
 * gmcommand [MouseJstr]
 *
 * suggested on the forums...
 * splitted into atcommand & charcommand by [Skotlex]
 *------------------------------------------
 */
int buildin_gmcommand(struct script_state &st)
{
	struct map_session_data *sd;
	const char *cmd;

	sd = script_rid2sd(st);
	if (!sd)
		return 0;
	cmd = conv_str(st, (st.stack.stack_data[st.start+2]));
	is_atcommand(sd->fd, *sd, cmd, 99);
	return 0;
}

int buildin_atcommand(struct script_state &st)
{
	struct map_session_data *sd;
	const char *cmd;

	sd = script_rid2sd(st);
	if (!sd)
		return 0;
	cmd = conv_str(st, (st.stack.stack_data[st.start+2]));
	is_atcommand(sd->fd, *sd, cmd, 99);
	return 0;
}

int buildin_charcommand(struct script_state &st)
{
	struct map_session_data *sd;
	const char *cmd;

	sd = script_rid2sd(st);
	if (!sd)
		return 0;
	cmd = conv_str(st, (st.stack.stack_data[st.start+2]));
	is_charcommand(sd->fd, *sd, cmd, 99);

	return 0;
}


/*==========================================
 * Displays a message for the player only (like system messages like "you got an apple" )
 *------------------------------------------
 */
int buildin_dispbottom(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	const char *message;
	message=conv_str(st, (st.stack.stack_data[st.start+2]));
	if(sd)
		clif_disp_onlyself(*sd,message);
	return 0;
}

/*==========================================
 * All The Players Full Recovery
   (HP/SP full restore and resurrect if need)
 *------------------------------------------
 */
int buildin_recovery(struct script_state &st)
{
	size_t i;
	for (i = 0; i < fd_max; i++) {
		if (session[i]){
			struct map_session_data *sd = (struct map_session_data *) session[i]->session_data;
			if (sd && sd->state.auth) {
				sd->status.hp = sd->status.max_hp;
				sd->status.sp = sd->status.max_sp;
				clif_updatestatus(*sd, SP_HP);
				clif_updatestatus(*sd, SP_SP);
				if(pc_isdead(*sd)){
					pc_setstand(*sd);
					clif_resurrection(sd->bl, 1);
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
int buildin_getpetinfo(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	int type=conv_num(st, (st.stack.stack_data[st.start+2]));

	if(sd && sd->status.pet_id){
		switch(type){
			case 0:
				push_val(st.stack,C_INT,sd->status.pet_id);
				break;
			case 1:
				if(sd->pet.class_)
					push_val(st.stack,C_INT,sd->pet.class_);
				else
					push_val(st.stack,C_INT,0);
				break;
			case 2:
				if(sd->pet.name)
				{	
					char *buf=(char *)aMalloc(24*sizeof(char));
					memcpy(buf,sd->pet.name, 24);//EOS included
					push_str(st.stack,C_STR, buf);
				}
				else
					push_str(st.stack,C_CONSTSTR, "");
				break;
			case 3:
				//if(sd->pet.intimate)
				push_val(st.stack,C_INT,sd->pet.intimate);
				break;
			case 4:
				//if(sd->pet.hungry)
				push_val(st.stack,C_INT,sd->pet.hungry);
				break;
			default:
				push_val(st.stack,C_INT,0);
				break;
		}
	}else{
		push_val(st.stack,C_INT,0);
	}
	return 0;
}
/*==========================================
 * Shows wether your inventory(and equips) contain
   selected card or not.
	checkequipedcard(4001);
 *------------------------------------------
 */
int buildin_checkequipedcard(struct script_state &st)
{
	struct map_session_data *sd=script_rid2sd(st);
	int n,i,c=0;
	c=conv_num(st, (st.stack.stack_data[st.start+2]));

	if(sd){
		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount){
				for(n=0;n<4;n++){
					if(sd->status.inventory[i].card[n]==c){
						push_val(st.stack,C_INT,1);
						return 0;
					}
				}
			}
		}
	}
	push_val(st.stack,C_INT,0);
	return 0;
}

int buildin_jump_zero(struct script_state &st) {
	int sel;
	sel=conv_num(st, (st.stack.stack_data[st.start+2]));
	if(!sel) {
		int pos;
		if( st.stack.stack_data[st.start+3].type!=C_POS ){
			ShowMessage("script: jump_zero: not label !\n");
			st.state=END;
			return 0;
		}

		pos=conv_num(st, (st.stack.stack_data[st.start+3]));
		st.pos=pos;
		st.state=GOTO;
		// ShowMessage("script: jump_zero: jumpto : %d\n",pos);
	} else {
		// ShowMessage("script: jump_zero: fail\n");
	}
	return 0;
}

int buildin_select(struct script_state &st)
{
	char *buf;
	size_t len,i;
	struct map_session_data *sd;

	sd=script_rid2sd(st);

	if(sd->state.menu_or_input==0){
		st.state=RERUNLINE;
		sd->state.menu_or_input=1;
		for(i=st.start+2,len=16;i<st.end;i++){
			conv_str(st, (st.stack.stack_data[i]));
			len+=strlen(st.stack.stack_data[i].u.str)+1;
		}
		buf=(char *)aMalloc((len+1)*sizeof(char));
		buf[0]=0;
		for(i=st.start+2,len=0;i<st.end;i++){
			strcat(buf,st.stack.stack_data[i].u.str);
			strcat(buf,":");
		}
		map_session_data *sd = script_rid2sd(st);
		if(sd) clif_scriptmenu(*sd,st.oid,buf);
		aFree(buf);
	} else if(sd->npc_menu==0xff){	// cansel
		sd->state.menu_or_input=0;
		st.state=END;
	} else {
		pc_setreg(*sd,add_str( "l15"),sd->npc_menu);
		pc_setreg(*sd,add_str( "@menu"),sd->npc_menu);
		sd->state.menu_or_input=0;
		push_val(st.stack,C_INT,sd->npc_menu);
	}
	return 0;
}

/*==========================================
 * GetMapMobs
	returns mob counts on a set map:
	e.g. GetMapMobs("prontera.gat")
	use "this" - for player's map
 *------------------------------------------
 */
int buildin_getmapmobs(struct script_state &st)
{
	const char *str;
	int m=-1,bx,by,i;
	int count=0,c;
	struct block_list *bl;

	str=conv_str(st, (st.stack.stack_data[st.start+2]));

	if(strcmp(str,"this")==0){
		struct map_session_data *sd=script_rid2sd(st);
		if(sd)
			m=sd->bl.m;
		else{
			push_val(st.stack,C_INT,-1);
			return 0;
		}
	}else
		m=map_mapname2mapid(str);

	if(m < 0){
		push_val(st.stack,C_INT,-1);
		return 0;
	}

	for(by=0;by<=(map[m].ys-1)/BLOCK_SIZE;by++){
		for(bx=0;bx<=(map[m].xs-1)/BLOCK_SIZE;bx++){
			bl = map[m].block_mob[bx+by*map[m].bxs];
			c = map[m].block_mob_count[bx+by*map[m].bxs];
			for(i=0;i<c && bl;i++,bl=bl->next){
				if(bl->x<map[m].xs && bl->y<map[m].ys)
					count++;
			}
		}
	}
	push_val(st.stack,C_INT,count);
	return 0;
}

/*==========================================
 * movenpc [MouseJstr]
 *------------------------------------------
 */

int buildin_movenpc(struct script_state &st)
{
	struct map_session_data *sd;
	const char *map,*npc;
	int x,y;

	sd = script_rid2sd(st);

	map = conv_str(st, (st.stack.stack_data[st.start+2]));
	x = conv_num(st, (st.stack.stack_data[st.start+3]));
	y = conv_num(st, (st.stack.stack_data[st.start+4]));
	npc = conv_str(st, (st.stack.stack_data[st.start+5]));

	return 0;
}

/*==========================================
 * message [MouseJstr]
 *------------------------------------------
 */

int buildin_message(struct script_state &st)
{
	struct map_session_data *sd;
	const char *msg,*player;
	struct map_session_data *pl_sd = NULL;

	sd = script_rid2sd(st);

	player = conv_str(st, (st.stack.stack_data[st.start+2]));
	msg = conv_str(st, (st.stack.stack_data[st.start+3]));

	if((pl_sd=map_nick2sd((char *) player)) == NULL)
             return 1;
	clif_displaymessage(pl_sd->fd, msg);

	return 0;
}

/*==========================================
 * npctalk (sends message to surrounding
 * area) [Valaris]
 *------------------------------------------
 */

int buildin_npctalk(struct script_state &st)
{
	const char *str;
	char message[255];

	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	str=conv_str(st, (st.stack.stack_data[st.start+2]));

	if(nd) {
		memcpy(message,nd->name,24);
		strcat(message," : ");
		strcat(message,str);
		clif_message(nd->bl, message);
	}

	return 0;
}

/*==========================================
 * hasitems (checks to see if player has any
 * items on them, if so will return a 1)
 * [Valaris]
 *------------------------------------------
 */

int buildin_hasitems(struct script_state &st)
{
	int i;
	struct map_session_data *sd;

	sd=script_rid2sd(st);

	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].amount && sd->status.inventory[i].nameid!=2364 && sd->status.inventory[i].nameid!=2365)
		{
			push_val(st.stack,C_INT,1);
			return 0;
		}
	}

	push_val(st.stack,C_INT,0);

	return 0;
}
// change npc walkspeed [Valaris]
int buildin_npcspeed(struct script_state &st)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	int x=0;

	x=conv_num(st, (st.stack.stack_data[st.start+2]));

	if(nd) {
		nd->speed=x;
	}

	return 0;
}
// make an npc walk to a position [Valaris]
int buildin_npcwalkto(struct script_state &st)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	int x=0,y=0;

	x=conv_num(st, (st.stack.stack_data[st.start+2]));
	y=conv_num(st, (st.stack.stack_data[st.start+3]));

	if(nd)
		npc_walktoxy(*nd,x,y,0);

	return 0;
}
// stop an npc's movement [Valaris]
int buildin_npcstop(struct script_state &st)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);

	if( (nd) && (nd->state.state==MS_WALK) )
		npc_stop_walking(*nd,1);
	return 0;
}


/*==========================================
  * getlook char info. getlook(arg)
  *------------------------------------------
  */
int buildin_getlook(struct script_state &st){
        int type,val;
        struct map_session_data *sd;
        sd=script_rid2sd(st);

        type=conv_num(st, (st.stack.stack_data[st.start+2]));
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

        push_val(st.stack,C_INT,val);
        return 0;
}

/*==========================================
  *     get char save point. argument: 0- map name, 1- x, 2- y
  *------------------------------------------
*/
int buildin_getsavepoint(struct script_state &st)
{
        int x,y,type;
        char *mapname;
        struct map_session_data *sd;

        sd=script_rid2sd(st);

        type=conv_num(st, (st.stack.stack_data[st.start+2]));
		mapname=(char*)aMalloc(24*sizeof(char));
		memcpy(mapname,sd->status.save_point.map,24);//EOS included

        x=sd->status.save_point.x;
        y=sd->status.save_point.y;
        switch(type){
            case 0:
                push_str(st.stack,C_STR,mapname);
                break;
            case 1:
                push_val(st.stack,C_INT,x);
                break;
            case 2:
                push_val(st.stack,C_INT,y);
                break;
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
int buildin_getmapxy(struct script_state &st)
{
	struct map_session_data *sd=NULL;
        struct npc_data *nd;
        struct pet_data *pd;
	int num;
	char *name;
	int x,y,type;
	char *mapname=NULL;

	if( st.stack.stack_data[st.start+2].type!=C_NAME )
	{
		ShowMessage("script: buildin_getmapxy: not mapname variable\n");
		push_val(st.stack,C_INT,-1);
                return 0;
        }
	if( st.stack.stack_data[st.start+3].type!=C_NAME )
	{
		ShowMessage("script: buildin_getmapxy: not mapx variable\n");
		push_val(st.stack,C_INT,-1);
                return 0;
        }
	if( st.stack.stack_data[st.start+4].type!=C_NAME )
	{
		ShowMessage("script: buildin_getmapxy: not mapy variable\n");
		push_val(st.stack,C_INT,-1);
                return 0;
        }


//??????????? >>>  Possible needly check function parameters on C_STR,C_INT,C_INT <<< ???????????//
	type=conv_num(st, (st.stack.stack_data[st.start+5]));

	switch(type)
	{
            case 0:                                             //Get Character Position
		if( st.end>st.start+6 )
			sd=map_nick2sd(conv_str(st, (st.stack.stack_data[st.start+6])));
                    else
                        sd=script_rid2sd(st);
		if( sd==NULL )
		{	//wrong char name or char offline
			push_val(st.stack,C_INT,-1);
                        return 0;
                    }
                    x=sd->bl.x;
                    y=sd->bl.y;
		mapname = sd->mapname;
		ShowMessage(">>>>%s %d %d\n",mapname,x,y);
                    break;
            case 1:                                             //Get NPC Position
		if( st.end > st.start+6 )
			nd=npc_name2id(conv_str(st, (st.stack.stack_data[st.start+6])));
                    else
			nd=(struct npc_data *)map_id2bl(st.oid);
		if( nd==NULL )
		{	//wrong npc name or char offline
			push_val(st.stack,C_INT,-1);
                        return 0;
                    }
                    x=nd->bl.x;
                    y=nd->bl.y;
		mapname=map[nd->bl.m].mapname;
		ShowMessage(">>>>%s %d %d\n",mapname,x,y);
                    break;
            case 2:                                             //Get Pet Position
		if( st.end>st.start+6 )
			sd=map_nick2sd(conv_str(st, (st.stack.stack_data[st.start+6])));
                    else
                        sd=script_rid2sd(st);
		if( sd==NULL )
		{	//wrong char name or char offline
			push_val(st.stack,C_INT,-1);
                        return 0;
                    }
                    pd=sd->pd;
		if(pd==NULL)
		{	//ped data not found
			push_val(st.stack,C_INT,-1);
                        return 0;
                    }
                    x=pd->bl.x;
                    y=pd->bl.y;
		mapname=map[pd->bl.m].mapname;
		ShowMessage(">>>>%s %d %d\n",mapname,x,y);
                    break;
            case 3:                                             //Get Mob Position
		push_val(st.stack,C_INT,-1);
                        return 0;
            default:                                            //Wrong type parameter
		push_val(st.stack,C_INT,-1);
                        return 0;
	}//end switch

	sd=script_rid2sd(st);
	if(sd)
	{
     //Set MapName$
		num=st.stack.stack_data[st.start+2].u.num;
        name=(char *)(str_buf+str_data[num&0x00ffffff].str);
		set_reg(st,num,name,mapname);

     //Set MapX
		num=st.stack.stack_data[st.start+3].u.num;
        name=(char *)(str_buf+str_data[num&0x00ffffff].str);
		set_reg(st,num,name,(void*)x);

     //Set MapY
		num=st.stack.stack_data[st.start+4].u.num;
        name=(char *)(str_buf+str_data[num&0x00ffffff].str);
		set_reg(st,num,name,(void*)y);

     //Return Success value
		push_val(st.stack,C_INT,0);
        return 0;
}
	push_val(st.stack,C_INT,-1);
	return 0;
}

/*=====================================================
 * Allows players to use a skill - by Qamera
 *-----------------------------------------------------
 */
int buildin_skilluseid (struct script_state &st)
{
   int skid,sklv;
   struct map_session_data *sd;
   skid=conv_num(st, (st.stack.stack_data[st.start+2]));
   sklv=conv_num(st, (st.stack.stack_data[st.start+3]));
   sd=script_rid2sd(st);
   if(sd) skill_use_id(sd,sd->status.account_id,skid,sklv);
   return 0;
}

/*=====================================================
 * Allows players to use a skill on a position [Celest]
 *-----------------------------------------------------
 */
int buildin_skillusepos(struct script_state &st)
{
   int skid,sklv,x,y;
   struct map_session_data *sd;
   skid=conv_num(st, (st.stack.stack_data[st.start+2]));
   sklv=conv_num(st, (st.stack.stack_data[st.start+3]));
   x=conv_num(st, (st.stack.stack_data[st.start+4]));
   y=conv_num(st, (st.stack.stack_data[st.start+5]));
   sd=script_rid2sd(st);
   if(sd) skill_use_pos(sd,x,y,skid,sklv);
   return 0;
}

/*==========================================
 * Allows player to write NPC logs (i.e. Bank NPC, etc) [Lupus]
 *------------------------------------------
 */
int buildin_logmes(struct script_state &st)
{
	if (log_config.npc <= 0 ) return 0;
	conv_str(st, (st.stack.stack_data[st.start+2]));
	map_session_data *sd = script_rid2sd(st);
	if(sd) log_npc(*sd,st.stack.stack_data[st.start+2].u.str);
	return 0;
}

int buildin_summon(struct script_state &st)
{
	int class_, id;
	const char *str,*event="";
	struct map_session_data *sd;
	struct mob_data *md;

	sd=script_rid2sd(st);
	if (sd) {
		unsigned long tick = gettick();
		str	=conv_str(st, (st.stack.stack_data[st.start+2]));
		class_=conv_num(st, (st.stack.stack_data[st.start+3]));
		if( st.end>st.start+4 )
			event=conv_str(st, (st.stack.stack_data[st.start+4]));

		id=mob_once_spawn(sd, "this", 0, 0, str,class_,1,event);
		if((md=(struct mob_data *)map_id2bl(id))){
			md->master_id=sd->bl.id;
			md->state.special_mob_ai=1;
			md->mode=mob_db[md->class_].mode|0x04;
			md->deletetimer=add_timer(tick+60000,mob_timer_delete,id,0);
			clif_misceffect2(md->bl,344);
		}
		clif_skill_poseffect(sd->bl,AM_CALLHOMUN,1,sd->bl.x,sd->bl.y,tick);
	}

	return 0;
}

/*==========================================
 * Checks whether it is daytime/nighttime
 *------------------------------------------
 */
int buildin_isnight(struct script_state &st)
{
	push_val(st.stack,C_INT, (night_flag == 1));
	return 0;
}

int buildin_isday(struct script_state &st)
{
	push_val(st.stack,C_INT, (night_flag == 0));
	return 0;
}

/*================================================
 * Check whether another item/card has been
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------
 */
// leave this here, just in case
#if 0
int buildin_isequipped(struct script_state &st)
{
	struct map_session_data *sd;
	int i, j, k, id = 1;
	int ret = -1;

	sd = script_rid2sd(st);
	if(sd)
	{
	for (i=0; id!=0; i++) {
		int flag = 0;
	
			if(st.end > st.start+i+2)
				id = conv_num(st,(st.stack.stack_data[st.start+i+2]));
			else 
				id = 0;

		if (id <= 0)
			continue;
		
		for (j=0; j<10; j++) {
			int index, type;
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == 9 && sd->equip_index[8] == index) continue;
			if(j == 5 && sd->equip_index[4] == index) continue;
			if(j == 6 && (sd->equip_index[5] == index || sd->equip_index[4] == index)) continue;
			type = itemdb_type(id);
			
			if(sd->inventory_data[index]) {
				if (type == 4 || type == 5) {
					if (sd->inventory_data[index]->nameid == id)
						flag = 1;
				} else if (type == 6) {
					for(k=0; k<sd->inventory_data[index]->slot; k++) {
						if (sd->status.inventory[index].card[0]!=0x00ff &&
							sd->status.inventory[index].card[0]!=0x00fe &&
								sd->status.inventory[index].card[0]!=0xff00 &&
							sd->status.inventory[index].card[k] == id) {
							flag = 1;
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
	push_val(st.stack,C_INT,ret);
	return 0;
}
#endif

/*================================================
 * Check how many items/cards in the list are
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------
 */
int buildin_isequippedcnt(struct script_state &st)
{
	struct map_session_data *sd;
	size_t i, j, k;
	unsigned long id = 1;
	int ret = 0;
	int index, type;

	sd = script_rid2sd(st);
	
	if(sd)
	{
	for (i=0; id!=0; i++) {

			if(st.end > st.start+i+2)
				id = conv_num(st,(st.stack.stack_data[st.start+i+2]));
			else 
				id = 0;

		if (id <= 0)
			continue;
		
		for (j=0; j<10; j++) {
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == 9 && sd->equip_index[8] == index) continue;
			if(j == 5 && sd->equip_index[4] == index) continue;
			if(j == 6 && (sd->equip_index[5] == index || sd->equip_index[4] == index)) continue;
			type = itemdb_type(id);
			
			if(sd->inventory_data[index]) {
				if (type == 4 || type == 5) {
					if (sd->inventory_data[index]->nameid == id)
						ret++; //[Lupus]
				} else if (type == 6) {
						for(k=0; k<sd->inventory_data[index]->flag.slot; k++) {
						if (sd->status.inventory[index].card[0]!=0x00ff &&
							sd->status.inventory[index].card[0]!=0x00fe &&
								sd->status.inventory[index].card[0]!=0xff00 &&
							sd->status.inventory[index].card[k] == id) {
							ret++; //[Lupus]
						}
					}
				}				
			}
		}
	}
	}
	push_val(st.stack,C_INT,ret);
	return 0;
}

/*================================================
 * Check whether another card has been
 * equipped - used for 2/15's cards patch [celest]
 * -- Items checked cannot be reused in another
 * card set to prevent exploits
 *------------------------------------------------
 */
int buildin_isequipped(struct script_state &st)
{
	struct map_session_data *sd;
	size_t i, j, k;
	unsigned short id = 1;
	int ret = -1;

	sd = script_rid2sd(st);
	
	if(sd)
	{
		for (i=0; id!=0; i++)
		{
		int flag = 0;
	
			if(st.end>st.start+(i+2))
				id=conv_num(st,(st.stack.stack_data[st.start+(i+2)]));
			else
				id = 0;
			
		if (id <= 0)
			continue;
		
			for (j=0; j<10; j++)
			{
			int index, type;
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == 9 && sd->equip_index[8] == index) continue;
			if(j == 5 && sd->equip_index[4] == index) continue;
			if(j == 6 && (sd->equip_index[5] == index || sd->equip_index[4] == index)) continue;
			type = itemdb_type(id);
			
				if(sd->inventory_data[index])
				{
					if (type == 4 || type == 5)
					{
					if (sd->inventory_data[index]->nameid == id)
						flag = 1;
					}
					else if (type == 6)
					{	// Item Hash format:
					// 1111 1111 1111 1111 1111 1111 1111 1111
					// [ left  ] [ right ] [ NA ] [  armor  ]
						for (k = 0; k < sd->inventory_data[index]->flag.slot; k++)
						{	// --- Calculate hash for current card ---
						// Defense equipment
						// They *usually* have only 1 slot, so we just assign 1 bit
						int hash = 0;
							if (sd->inventory_data[index]->type == 5)
							{
							hash = sd->inventory_data[index]->equip;
						}
						// Weapons
							// right hand: slot 1 - 0x0010000 ... slot 4 - 0x0080000
						// left hand: slot 1 - 0x1000000 ... slot 4 - 0x8000000
						// We can support up to 8 slots each, just in case
							else if (sd->inventory_data[index]->type == 4)
							{
							if (sd->inventory_data[index]->equip & 2)	// right hand
									hash = 0x00010000 * (1<<k);	// pow(2,k) x slot number
							else if (sd->inventory_data[index]->equip & 32)	// left hand
									hash = 0x01000000 * (1<<k);	// pow(2,k) x slot number
							}
							else
							continue;	// slotted item not armour nor weapon? we're not going to support it

						if (sd->setitem_hash & hash)	// check if card is already used by another set
							continue;	// this item is used, move on to next card

						if (sd->status.inventory[index].card[0] != 0x00ff &&
							sd->status.inventory[index].card[0] != 0x00fe &&
								sd->status.inventory[index].card[0] != 0xff00 &&
							sd->status.inventory[index].card[k] == id)
						{
							// We have found a match
							flag = 1;
							// Set hash so this card cannot be used by another
							sd->setitem_hash |= hash;
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
	push_val(st.stack,C_INT,ret);
	return 0;
}

/*================================================
 * Check how many given inserted cards in the CURRENT
 * weapon - used for 2/15's cards patch [Lupus]
 *------------------------------------------------
 */
int buildin_cardscnt(struct script_state &st)
{
	struct map_session_data *sd;
	size_t i, k;
	unsigned short id = 1;
	int ret = 0;
	int index, type;

	sd = script_rid2sd(st);
	
	if(sd)
	{
		for (i=0; id!=0; i++)
		{
			if(st.end>st.start+(i+2))
				id=conv_num(st,(st.stack.stack_data[st.start+(i+2)]));
			else 
				id = 0;
			
		if (id <= 0)
			continue;
		
		index = current_equip_item_index; //we get CURRENT WEAPON inventory index from status.c [Lupus]
		if(index < 0) continue;

		type = itemdb_type(id);
			
			if(sd->inventory_data[index])
			{
				if (type == 4 || type == 5)
				{
				if (sd->inventory_data[index]->nameid == id)
					ret++;
				}
				else if (type == 6)
				{
					for(k=0; k<sd->inventory_data[index]->flag.slot; k++)
					{
					if (sd->status.inventory[index].card[0]!=0x00ff &&
						sd->status.inventory[index].card[0]!=0x00fe &&
							sd->status.inventory[index].card[0]!=0xff00 &&
							sd->status.inventory[index].card[k] == id )
						{
						ret++;
					}
				}
			}				
		}
	}
	}
	push_val(st.stack,C_INT,ret);
//	push_val(st.stack,C_INT,current_equip_item_index);
	return 0;
}

/*=======================================================
 * Returns the refined number of the current item, or an
 * item with inventory index specified
 *-------------------------------------------------------
 */
int buildin_getrefine(struct script_state &st)
{
	struct map_session_data *sd;
	if ((sd = script_rid2sd(st))!= NULL)
		push_val(st.stack, C_INT, sd->status.inventory[current_equip_item_index].refine);
	return 0;
}

/*=======================================================
 * Allows 2 Parents to adopt a character as a Baby
 *-------------------------------------------------------
 */
int buildin_adopt(struct script_state &st)
{
	int ret=0;
	
	const char *parent1 = conv_str(st, (st.stack.stack_data[st.start+2]));
	const char *parent2 = conv_str(st, (st.stack.stack_data[st.start+3]));
	const char *child = conv_str(st, (st.stack.stack_data[st.start+4]));

	struct map_session_data *p1_sd = map_nick2sd(parent1);
	struct map_session_data *p2_sd = map_nick2sd(parent2);
	struct map_session_data *c_sd = map_nick2sd(child);

	if( p1_sd && p2_sd && c_sd &&
		p1_sd->status.base_level >= 70 &&
		p2_sd->status.base_level >= 70 )
		ret = pc_adoption(*p1_sd, *p2_sd, *c_sd);

	push_val(st.stack, C_INT, ret);
	return 0;
}

/*=======================================================
 * Day/Night controls
 *-------------------------------------------------------
 */
int buildin_night(struct script_state &st)
{
	if (night_flag != 1) map_night_timer(night_timer_tid, 0, 0, 1);
	return 0;
}
int buildin_day(struct script_state &st)
{
	if (night_flag != 0) map_day_timer(day_timer_tid, 0, 0, 1);
	return 0;
}

//=======================================================
// Unequip [Spectre]
//-------------------------------------------------------
int buildin_unequip(struct script_state &st)
{
	int i;
	size_t num;
	struct map_session_data *sd;
	num = conv_num(st, (st.stack.stack_data[st.start+2])) - 1;
	sd=script_rid2sd(st);
	if(sd!=NULL && num<10)
	{
		i=pc_checkequip(*sd,equip[num]);
		pc_unequipitem(*sd,i,2);
		return 0;
	}
	return 0;
}


int buildin_pcstrcharinfo(struct script_state &st)
{
	int aid,num;
	struct map_session_data *sd;
	
	aid=conv_num(st, (st.stack.stack_data[st.start+2]));
	num=conv_num(st, (st.stack.stack_data[st.start+3]));
	
	sd=map_id2sd(aid);
	if(sd==NULL){
		push_str(st.stack,C_CONSTSTR,"");
		num=-1;
	}
	if(num==0){
		char *buf;
		buf=(char*)aMalloc(24 * sizeof(char));
		safestrcpy(buf,sd->status.name, 24);
		push_str(st.stack,C_STR,buf);
	}
	else if(num==1){
		char *buf;
		buf=buildin_getpartyname_sub(sd->status.party_id);
		if(buf!=0)
			push_str(st.stack,C_STR,buf);
		else
			push_str(st.stack,C_CONSTSTR,"");
	}
	if(num==2){
		char *buf;
		buf=buildin_getguildname_sub(sd->status.guild_id);
		if(buf!=0)
			push_str(st.stack,C_STR,buf);
		else
			push_str(st.stack,C_CONSTSTR,"");
	}
	return 0;
}

//=======================================================
// strlen [Valaris]
//-------------------------------------------------------
int buildin_getstrlen(struct script_state &st)
{
	const char *str=conv_str(st, (st.stack.stack_data[st.start+2]));
	int len = (str) ? (int)strlen(str) : 0;
	push_val(st.stack,C_INT,len);
	return 0;
}

//=======================================================
// isalpha [Valaris]
//-------------------------------------------------------
int buildin_charisalpha(struct script_state &st)
{
	const char *str=conv_str(st, (st.stack.stack_data[st.start+2]));
	size_t pos =conv_num(st, (st.stack.stack_data[st.start+3]));

	int val = ( str && pos>0 && pos<strlen(str) ) ? isalpha( (int)((unsigned char)str[pos]) ) : 0;
	push_val(st.stack,C_INT, val);
	return 0;
}
//
// 実行部main
//
/*==========================================
 * コマンドの読み取り
 *------------------------------------------
 */
static int unget_com_data=-1;
int get_com(const char *script,unsigned int &pos)
{
	const unsigned char *s = (const unsigned char *)script;
	int i,j;
	if(unget_com_data>=0){
		i=unget_com_data;
		unget_com_data=-1;
		return i;
	}
	if(s[pos]>=0x80){
		return C_INT;
	}
	i=0; j=0;
	while(s[pos]>=0x40){
		i=s[pos++]<<j;
		j+=6;
	}
	return i+(s[pos++]<<j);
}

/*==========================================
 * コマンドのプッシュバック
 *------------------------------------------
 */
void unget_com(int c)
{
	if(unget_com_data!=-1){
		if(battle_config.error_log)
			ShowMessage("unget_com can back only 1 data\n");
	}
	unget_com_data=c;
}

/*==========================================
 * 数値の所得
 *------------------------------------------
 */
int get_num(const char *script,unsigned int &pos)
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
 * スタックから値を取り出す
 *------------------------------------------
 */
int pop_val(struct script_state& st)
{
	if(st.stack.sp <= 0)
		return 0;
	st.stack.sp--;
	get_val(st,(st.stack.stack_data[st.stack.sp]));
	if(st.stack.stack_data[st.stack.sp].type==C_INT)
		return st.stack.stack_data[st.stack.sp].u.num;
	return 0;
}

#define isstr(c) ((c).type==C_STR || (c).type==C_CONSTSTR)

/*==========================================
 * 加算演算子
 *------------------------------------------
 */
void op_add(struct script_state& st)
{
	st.stack.sp--;
	get_val(st,(st.stack.stack_data[st.stack.sp]));
	get_val(st,(st.stack.stack_data[st.stack.sp-1]));

	if(isstr(st.stack.stack_data[st.stack.sp]) || isstr(st.stack.stack_data[st.stack.sp-1])){
		conv_str(st,(st.stack.stack_data[st.stack.sp]));
		conv_str(st,(st.stack.stack_data[st.stack.sp-1]));
	}
	if(st.stack.stack_data[st.stack.sp].type==C_INT){ // ii
		st.stack.stack_data[st.stack.sp-1].u.num += st.stack.stack_data[st.stack.sp].u.num;
	} else { // ssの予定
		char *buf;
		buf=(char *)aCallocA(strlen(st.stack.stack_data[st.stack.sp-1].u.str)+
				strlen(st.stack.stack_data[st.stack.sp].u.str)+1,sizeof(char));
		strcpy(buf,st.stack.stack_data[st.stack.sp-1].u.str);
		strcat(buf,st.stack.stack_data[st.stack.sp].u.str);
		if(st.stack.stack_data[st.stack.sp-1].type==C_STR)
			aFree( (void*)(st.stack.stack_data[st.stack.sp-1].u.str) );
		if(st.stack.stack_data[st.stack.sp].type==C_STR)
			aFree((void*)(st.stack.stack_data[st.stack.sp].u.str) );
		st.stack.stack_data[st.stack.sp-1].type=C_STR;
		st.stack.stack_data[st.stack.sp-1].u.str=buf;
	}
}

/*==========================================
 * 二項演算子(文字列)
 *------------------------------------------
 */
void op_2str(struct script_state &st,int op,int sp1,int sp2)
{
	const char *s1=st.stack.stack_data[sp1].u.str;
	const char *s2=st.stack.stack_data[sp2].u.str;
	int a=0;

	switch(op){
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
		ShowMessage("illegal string operater\n");
		break;
	}

	push_val(st.stack,C_INT,a);

	if(st.stack.stack_data[sp1].type==C_STR)	aFree( (void*)(s1) );
	if(st.stack.stack_data[sp2].type==C_STR)	aFree( (void*)(s2) );
}
/*==========================================
 * 二項演算子(数値)
 *------------------------------------------
 */
void op_2num(struct script_state &st,int op,int i1,int i2)
{
	switch(op){
	case C_SUB:
		i1-=i2;
		break;
	case C_MUL:
		{
			int64 res = (int64)i1 * (int64)i2;
			if (res >  LLCONST(2147483647) )
				i1 = 0x7FFFFFFF;
			else if (res <  LLCONST(-2147483648) )
				i1 = 0x80000000;
		else
				i1 = (int)res;
		}
		break;
	case C_DIV:
		i1/=i2;
		break;
	case C_MOD:
		i1%=i2;
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
	push_val(st.stack,C_INT,i1);
}
/*==========================================
 * 二項演算子
 *------------------------------------------
 */
void op_2(struct script_state &st,int op)
{
	int i1,i2;
	const char *s1=NULL,*s2=NULL;

	i2=pop_val(st);
	if( isstr(st.stack.stack_data[st.stack.sp]) )
		s2=st.stack.stack_data[st.stack.sp].u.str;

	i1=pop_val(st);
	if( isstr(st.stack.stack_data[st.stack.sp]) )
		s1=st.stack.stack_data[st.stack.sp].u.str;

	if( s1!=NULL && s2!=NULL ){
		// ss => op_2str
		op_2str(st,op,st.stack.sp,st.stack.sp+1);
	}else if( s1==NULL && s2==NULL ){
		// ii => op_2num
		op_2num(st,op,i1,i2);
	}else{
		// si,is => error
		ShowMessage("script: op_2: int&str, str&int not allow.");
		push_val(st.stack,C_INT,0);
	}
}

/*==========================================
 * 単項演算子
 *------------------------------------------
 */
void op_1num(struct script_state &st,int op)
{
	int i1;
	i1=pop_val(st);
	switch(op){
	case C_NEG:
		i1=-i1;
		break;
	case C_NOT:
		i1=~i1;
		break;
	case C_LNOT:
		i1=!i1;
		break;
	}
	push_val(st.stack,C_INT,i1);
}


/*==========================================
 * 関数の実行
 *------------------------------------------
 */
int run_func(struct script_state &st)
{
	int i,start_sp,end_sp,func;

	end_sp=st.stack.sp;
	for(i=end_sp-1;i>=0 && st.stack.stack_data[i].type!=C_ARG;i--);
	if(i==0){
		if(battle_config.error_log)
			ShowMessage("function not found\n");
		st.state=END;
		return 0;
	}
	start_sp=i-1;
	st.start=i-1;
	st.end=end_sp;

	func=st.stack.stack_data[st.start].u.num;
	if( st.stack.stack_data[st.start].type!=C_NAME || str_data[func].type!=C_FUNC ){
		ShowMessage ("run_func: '"CL_WHITE"%s"CL_RESET"' (type %d) is not function and command!\n",
				str_buf + str_data[func].str, str_data[func].type);
		st.state=END;
		return 0;
	}
#ifdef DEBUG_RUN
	if(battle_config.etc_log) {
		ShowMessage("run_func : %s? (%d(%d))\n",str_buf+str_data[func].str,func,str_data[func].type);
		ShowMessage("stack dump :");
		for(i=0;i<end_sp;i++){
			switch(st.stack.stack_data[i].type){
			case C_INT:
				ShowMessage(" int(%d)",st.stack.stack_data[i].u.num);
				break;
			case C_NAME:
				ShowMessage(" name(%s)",str_buf+str_data[st.stack.stack_data[i].u.num].str);
				break;
			case C_ARG:
				ShowMessage(" arg");
				break;
			case C_POS:
				ShowMessage(" pos(%d)",st.stack.stack_data[i].u.num);
				break;
			default:
				ShowMessage(" %d,%d",st.stack.stack_data[i].type,st.stack.stack_data[i].u.num);
			}
		}
		ShowMessage("\n");
	}
#endif
	if(str_data[func].func){
		str_data[func].func(st);
	} else {
		if(battle_config.error_log)
			ShowMessage("run_func : %s? (%d(%d))\n",str_buf+str_data[func].str,func,str_data[func].type);
		push_val(st.stack,C_INT,0);
	}

	pop_stack(st.stack,start_sp,end_sp);

	if(st.state==RETFUNC){
		// ユーザー定義関数からの復帰
		int olddefsp=st.defsp;
		int i;

		pop_stack(st.stack,st.defsp,start_sp);	// 復帰に邪魔なスタック削除
		if(st.defsp<4 || st.stack.stack_data[st.defsp-1].type!=C_RETINFO){
			ShowMessage("script:run_func(return) return without callfunc or callsub!\n");
			st.state=END;
			return 0;
		}
		i = conv_num(st, (st.stack.stack_data[st.defsp-4]));				// 引数の数所得
		st.pos=conv_num(st, (st.stack.stack_data[st.defsp-1]));			// スクリプト位置の復元
		st.script=(char*)conv_num(st, (st.stack.stack_data[st.defsp-2]));	// スクリプトを復元
		st.defsp=conv_num(st, (st.stack.stack_data[st.defsp-3]));			// 基準スタックポインタを復元

		pop_stack(st.stack,olddefsp-4-i,olddefsp);		// 要らなくなったスタック(引数と復帰用データ)削除

		st.state=GOTO;
	}

	return 0;
}

/*==========================================
 * スクリプトの実行メイン部分
 *------------------------------------------
 */
bool run_script_main(struct script_state &st)
{
	int c=0, rerun_pos;
	int cmdcount=script_config.check_cmdcount;
	int gotocount=script_config.check_gotocount;

	if(!st.script) return 0;

	st.defsp = st.stack.sp;

	rerun_pos=st.pos;

	for(st.state=0;st.state==0;){
		c = get_com(st.script,st.pos);
		switch(c){
		case C_EOL:
			if(st.stack.sp!=st.defsp){
				if(battle_config.error_log)
					ShowMessage("stack.sp(%d) != default(%d)\n",st.stack.sp,st.defsp);
//!!
printf("(%d)\n", st.pos);
debug_script(st.script,((st.pos>32)?st.pos-32:0),((st.pos>32)?32:st.pos));

				st.stack.sp=st.defsp;
			}
			rerun_pos=st.pos;
			break;
		case C_INT:
			push_val(st.stack,C_INT,get_num(st.script,st.pos));
			break;
		case C_POS:
		case C_NAME:
		{
			unsigned long tmp;
			tmp = (0xFF&st.script[st.pos])
				| (0xFF&st.script[st.pos+1])<<8
				| (0xFF&st.script[st.pos+2])<<16;
			push_val(st.stack,c,tmp);
			st.pos+=3;
			break;
		}
		case C_ARG:
			push_val(st.stack,c,0);
			break;
		case C_STR:
			push_str(st.stack, C_CONSTSTR, (st.script+st.pos));
			while(st.script[st.pos++]);
			break;
		case C_FUNC:
			run_func(st);
			if(st.state==GOTO){
				rerun_pos=st.pos;
//				script=st.script;
				st.state=0;
				if( gotocount>0 && (--gotocount)<=0 ){
					ShowMessage("run_script: infinity loop !\n");
					st.state=END;
				}
			}
			break;

		case C_ADD:
			op_add(st);
			break;

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
			op_2(st,c);
			break;

		case C_NEG:
		case C_NOT:
		case C_LNOT:
			op_1num(st,c);
			break;

		case C_NOP:
			st.pos++;
			st.state=END;
			break;

		default:
			if(battle_config.error_log)
				ShowMessage("unknown command : %d @ %d\n",c,st.pos);
			st.state=END;
			break;
		}
		if( cmdcount>0 && (--cmdcount)<=0 ){
			ShowMessage("run_script: infinity loop !\n");
			st.state=END;
		}
	}
	switch(st.state){
	case STOP:
		break;
	case END:
		{
			struct map_session_data *sd=map_id2sd(st.rid);
			st.pos = 0xFFFFFFFF;
			if(sd && sd->npc_id==st.oid)
				npc_event_dequeue(*sd);
		}
		break;
	case RERUNLINE:
		{
			st.pos=rerun_pos;
		}
		break;
	}

	return (st.state!=END);
}

/*==========================================
 * スクリプトの実行
 *------------------------------------------
 */
int run_script(const char *rootscript,int pos,int rid,int oid)
{
	struct script_state st;
	struct map_session_data *sd=map_id2sd(rid);

	if(rootscript==NULL || pos<0)
		return -1;

	memset(&st,0,sizeof(st));

	if(sd && sd->npc_stackbuf && sd->npc_scriptroot==rootscript){
		// 前回のスタックを復帰
		// set scripts to continue from previous run
		st.script			= sd->npc_script;
		sd->npc_script      = NULL;
		sd->npc_scriptroot  = NULL;

		// transfer saved stack data to script stack
		st.stack.sp			= sd->npc_stack;
		st.stack.sp_max		= sd->npc_stackmax;		
		st.stack.stack_data = (struct script_data *)sd->npc_stackbuf;
		sd->npc_stackbuf=NULL;

	}else{
		// スタック初期化
		// set scripts to start a new script
		st.script			= rootscript;

		// create a new stack
		st.stack.sp			= 0;
		st.stack.sp_max		= 64;
		st.stack.stack_data = (struct script_data *)aCalloc(st.stack.sp_max,sizeof(st.stack.stack_data[0]));
	}
	st.pos=pos;
	st.rid=rid;
	st.oid=oid;

	if( run_script_main(st) && sd)
	{	// script is not finished, we save the stack
		// 再開するためにスタック情報を保存

		if( sd->npc_stackbuf )// should not happen
			aFree(sd->npc_stackbuf);

		// transfer stack data to npc
		sd->npc_stackbuf = (char *)st.stack.stack_data;
		st.stack.stack_data = NULL;

		sd->npc_stack     = st.stack.sp;
		sd->npc_stackmax  = st.stack.sp_max;
		sd->npc_script    = st.script;
		sd->npc_scriptroot= rootscript;
	}
	if(st.stack.stack_data)
	{
		for(int i = 0; i < st.stack.sp; i++)
			if (st.stack.stack_data[i].type == C_STR)
				aFree((void*)(st.stack.stack_data[i].u.str));
		aFree(st.stack.stack_data);
		st.stack.stack_data=NULL;
	}

	return st.pos;
}


/*==========================================
 * マップ変数の変更
 *------------------------------------------
 */
int mapreg_setreg(int num,int val)
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
int mapreg_setregstr(int num,const char *str)
{
	char *p;

	if( (p=(char *) numdb_search(mapregstr_db,num))!=NULL )
		aFree(p);

	if( str==NULL || *str==0 ){
		numdb_erase(mapregstr_db,num);
		mapreg_dirty=1;
		return 0;
	}
	p=(char *)aMalloc( (strlen(str)+1)*sizeof(char));
	strcpy(p,str);
	numdb_insert(mapregstr_db,num,p);
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

	if( (fp=savefopen(mapreg_txt,"rt"))==NULL )
		return -1;

	while(fgets(line,sizeof(line),fp)){
		char buf1[256],buf2[1024],*p;
		int n,v,s,i;
		if( sscanf(line,"%255[^,],%d\t%n",buf1,&i,&n)!=2 &&
			(i=0,sscanf(line,"%[^\t]\t%n",buf1,&n)!=1) )
			continue;
		if( buf1[strlen(buf1)-1]=='$' ){
			if( sscanf(line+n,"%[^\n\r]",buf2)!=1 ){
				ShowMessage("%s: %s broken data !\n",mapreg_txt,buf1);
				continue;
			}
			p=(char *)aMalloc( (strlen(buf2) + 1)*sizeof(char));
			strcpy(p,buf2);
			s= add_str( buf1);
			numdb_insert(mapregstr_db,(i<<24)|s,p);
		}else{
			if( sscanf(line+n,"%d",&v)!=1 ){
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
int script_save_mapreg_intsub(void *key,void *data,va_list ap)
{
	FILE *fp=va_arg(ap,FILE*);
	int num=((int)key)&0x00ffffff, i=((int)key)>>24;
	char *name=str_buf+str_data[num].str;
	if( name[1]!='@' ){
		if(i==0)
			fprintf(fp,"%s\t%d\n", name, (int)data);
		else
			fprintf(fp,"%s,%d\t%d\n", name, i, (int)data);
	}
	return 0;
}
int script_save_mapreg_strsub(void *key,void *data,va_list ap)
{
	FILE *fp=va_arg(ap,FILE*);
	int num=((int)key)&0x00ffffff, i=((int)key)>>24;
	char *name=str_buf+str_data[num].str;
	if( name[1]!='@' ){
		if(i==0)
			fprintf(fp,"%s\t%s\n", name, (char *)data);
		else
			fprintf(fp,"%s,%d\t%s\n", name, i, (char *)data);
	}
	return 0;
}
int script_save_mapreg()
{
	FILE *fp;
	int lock;

	if( (fp=lock_fopen(mapreg_txt,&lock))==NULL )
		return -1;
	numdb_foreach(mapreg_db,script_save_mapreg_intsub,fp);
	numdb_foreach(mapregstr_db,script_save_mapreg_strsub,fp);
	lock_fclose(fp,mapreg_txt,&lock);
	mapreg_dirty=0;
	return 0;
}
int script_autosave_mapreg(int tid,unsigned long tick,int id,int data)
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
	for(i=0;i<MAX_EQUIP;i++)
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
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	script_config.verbose_mode = 0;
	script_config.warn_func_no_comma = 1;
	script_config.warn_cmd_no_comma = 1;
	script_config.warn_func_mismatch_paramnum = 1;
	script_config.warn_cmd_mismatch_paramnum = 1;
	script_config.check_cmdcount=16384;
	script_config.check_gotocount=1024;


	script_config.event_script_type = 0;
	script_config.event_requires_trigger = 1;

	fp=savefopen(cfgName,"r");
	if (fp == NULL) {
		ShowMessage("file not found: %s\n",cfgName);
		return 1;
	}
	while (fgets(line, sizeof(line) - 1, fp)) {
		if( !skip_empty_line(line) )
			continue;
		i = sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if (i != 2)
			continue;
		if(strcasecmp(w1,"refine_posword")==0) {
			set_posword(w2);
		}
		else if(strcasecmp(w1,"verbose_mode")==0) {
			script_config.verbose_mode = config_switch(w2);
		}
		else if(strcasecmp(w1,"warn_func_no_comma")==0) {
			script_config.warn_func_no_comma = config_switch(w2);
		}
		else if(strcasecmp(w1,"warn_cmd_no_comma")==0) {
			script_config.warn_cmd_no_comma = config_switch(w2);
		}
		else if(strcasecmp(w1,"warn_func_mismatch_paramnum")==0) {
			script_config.warn_func_mismatch_paramnum = config_switch(w2);
		}
		else if(strcasecmp(w1,"warn_cmd_mismatch_paramnum")==0) {
			script_config.warn_cmd_mismatch_paramnum = config_switch(w2);
		}
		else if(strcasecmp(w1,"check_cmdcount")==0) {
			script_config.check_cmdcount = config_switch(w2);
		}
		else if(strcasecmp(w1,"check_gotocount")==0) {
			script_config.check_gotocount = config_switch(w2);
		}
		else if(strcasecmp(w1,"event_script_type")==0) {
			script_config.event_script_type = config_switch(w2);
		}
		else if(strcasecmp(w1,"die_event_name")==0) {			
			safestrcpy(script_config.die_event_name, w2,sizeof(script_config.die_event_name));
		}
		else if(strcasecmp(w1,"kill_event_name")==0) {
			safestrcpy(script_config.kill_event_name, w2,sizeof(script_config.kill_event_name));
		}
		else if(strcasecmp(w1,"login_event_name")==0) {
			safestrcpy(script_config.login_event_name, w2,sizeof(script_config.login_event_name));
		}
		else if(strcasecmp(w1,"logout_event_name")==0) {
			safestrcpy(script_config.logout_event_name, w2, sizeof(script_config.logout_event_name));
		}
		else if(strcasecmp(w1,"mapload_event_name")==0) {
			safestrcpy(script_config.mapload_event_name, w2, sizeof(script_config.mapload_event_name));
		}
		else if(strcasecmp(w1,"event_requires_trigger")==0) {
			script_config.event_requires_trigger = config_switch(w2);
		}
		else if(strcasecmp(w1,"import")==0){
			script_config_read(w2);
		}
	}
	fclose(fp);

	return 0;
}

/*==========================================
 * 終了
 *------------------------------------------
 */
int mapreg_db_final(void *key,void *data,va_list ap)
{
	return 0;
}
int mapregstr_db_final(void *key,void *data,va_list ap)
{
	aFree(data);
	return 0;
}
int scriptlabel_db_final(void *key,void *data,va_list ap)
{
	return 0;
}
int userfunc_db_final(void *key,void *data,va_list ap)
{
	aFree(key);
	aFree(data);
	return 0;
}
int do_final_script()
{
	if(mapreg_dirty>=0)
		script_save_mapreg();

	if(mapreg_db)
		numdb_final(mapreg_db,mapreg_db_final);
	if(mapregstr_db)
		strdb_final(mapregstr_db,mapregstr_db_final);
	if(scriptlabel_db)
		strdb_final(scriptlabel_db,scriptlabel_db_final);
	if(userfunc_db)
		strdb_final(userfunc_db,userfunc_db_final);

	if (str_data)	aFree(str_data);
	if (str_buf)	aFree(str_buf);

	return 0;
}
/*==========================================
 * 初期化
 *------------------------------------------
 */
int do_init_script()
{
	mapreg_db=numdb_init();
	mapregstr_db=numdb_init();
	script_load_mapreg();

	add_timer_func_list(script_autosave_mapreg,"script_autosave_mapreg");
	add_timer_interval(gettick()+MAPREG_AUTOSAVE_INTERVAL,MAPREG_AUTOSAVE_INTERVAL,
		script_autosave_mapreg,0,0);

	if (scriptlabel_db == NULL)
	  scriptlabel_db=strdb_init(50);
	return 0;
}
