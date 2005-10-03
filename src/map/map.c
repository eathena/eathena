// $Id: map.c,v 1.6 2004/09/25 17:37:01 MouseJstr Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <netdb.h>
#include <unistd.h>
#endif
#include <math.h>

#include "../common/core.h"
#include "../common/timer.h"
#include "../common/db.h"
#include "../common/grfio.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/nullpo.h"

#include "map.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "status.h"
#include "mob.h"
#include "chat.h"
#include "itemdb.h"
#include "storage.h"
#include "skill.h"
#include "trade.h"
#include "party.h"
#include "battle.h"
#include "script.h"
#include "guild.h"
#include "pet.h"
#include "atcommand.h"
#include "charcommand.h"

#include "log.h"

#include "charsave.h"


// maybe put basic macros to somewhere else
#define swap(a,b) ((a == b) || ((a ^= b), (b ^= a), (a ^= b)))

#ifndef TXT_ONLY

#include "mail.h" // mail system [Valaris]

MYSQL mmysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;
char tmp_sql[65535]="";

MYSQL lmysql_handle;
MYSQL_RES* lsql_res ;
MYSQL_ROW  lsql_row ;

MYSQL logmysql_handle; //For the log database - fix by [Maeki]
MYSQL_RES* logsql_res ;
MYSQL_ROW  logsql_row ;

MYSQL mail_handle; // mail system [Valaris]
MYSQL_RES* 	mail_res ;
MYSQL_ROW	mail_row ;

int map_server_port = 3306;
char map_server_ip[16] = "127.0.0.1";
char map_server_id[32] = "ragnarok";
char map_server_pw[32] = "ragnarok";
char map_server_db[32] = "ragnarok";
int db_use_sqldbs = 0;

int login_server_port = 3306;
char login_server_ip[16] = "127.0.0.1";
char login_server_id[32] = "ragnarok";
char login_server_pw[32] = "ragnarok";
char login_server_db[32] = "ragnarok";

char item_db_db[32] = "item_db";
char item_db2_db[32] = "item_db2";
char mob_db_db[32] = "mob_db";
char mob_db2_db[32] = "mob_db2";
char login_db[32] = "login";
char login_db_level[32] = "level";
char login_db_account_id[32] = "account_id";

char log_db[32] = "log";
char log_db_ip[16] = "127.0.0.1";
char log_db_id[32] = "ragnarok";
char log_db_pw[32] = "ragnarok";
int log_db_port = 3306;

char gm_db[32] = "login";
char gm_db_level[32] = "level";
char gm_db_account_id[32] = "account_id";

int lowest_gm_level = 1;
int read_gm_interval = 600000;

char char_db[32] = "char";

char charsql_host[40] = "localhost";
int charsql_port = 3306;
char charsql_user[32] = "ragnarok";
char charsql_pass[32] = "eAthena";
char charsql_db[40] = "ragnarok";
MYSQL charsql_handle;
MYSQL_RES* charsql_res;
MYSQL_ROW charsql_row;

#endif /* not TXT_ONLY */

static int online_timer(int,unsigned int,int,int);
int CHECK_INTERVAL = 3600000; // [Valaris]

char *INTER_CONF_NAME;
char *LOG_CONF_NAME;
char *MAP_CONF_NAME;
char *BATTLE_CONF_FILENAME;
char *ATCOMMAND_CONF_FILENAME;
char *CHARCOMMAND_CONF_FILENAME;
char *SCRIPT_CONF_NAME;
char *MSG_CONF_NAME;
char *GRF_PATH_FILENAME;

#define USE_AFM
#define USE_AF2

// ‹É—Í static‚Åƒ?ƒJƒ‹‚É?‚ß‚é
static struct dbt * id_db=NULL;
static struct dbt * pc_db=NULL;
static struct dbt * map_db=NULL;
static struct dbt * nick_db=NULL;
static struct dbt * charid_db=NULL;

static int map_users=0;
static struct block_list *objects[MAX_FLOORITEM];
static int first_free_object_id=0,last_object_id=0;

#define block_free_max 1048576
struct block_list *block_free[block_free_max];
static int block_free_count = 0, block_free_lock = 0;

#define BL_LIST_MAX 1048576
static struct block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;

static char afm_dir[1024] = ""; // [Valaris]

struct map_data map[MAX_MAP_PER_SERVER];
int map_num = 0;

int map_port=0;

int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int charsave_method = 0; //Default 'OLD' Save method (SQL ONLY!) [Sirius]
int agit_flag = 0;
int night_flag = 0; // 0=day, 1=night [Yor]
int kick_on_disconnect = 1;

struct charid2nick {
	char nick[NAME_LENGTH];
	int req_id;
};

// «Ş«Ã«×«­«ã«Ã«·«å××éÄ«Õ«é«°(map_athana.conf?ªÎread_map_from_cacheªÇò¦E)
// 0:××éÄª·ªÊª¤ 1:Şª?õêÜÁğí 2:?õêÜÁğí
int  map_read_flag = READ_FROM_GAT;
char map_cache_file[256]="db/map.info"; // «Ş«Ã«×«­«ã«Ã«·«å«Õ«¡«¤«E£

char motd_txt[256] = "conf/motd.txt";
char help_txt[256] = "conf/help.txt";

char wisp_server_name[NAME_LENGTH] = "Server"; // can be modified in char-server configuration file

int console = 0;

/*==========================================
 * ‘SmapI?Œv‚Å‚ÌÚ??İ’è
 * (charI‚©‚ç‘—‚ç‚ê‚Ä‚­‚é)
 *------------------------------------------
 */
void map_setusers(int fd)
{
	map_users = RFIFOL(fd,2);
	// send some anser
	WFIFOW(fd,0) = 0x2718;
	WFIFOSET(fd,2);
}

/*==========================================
 * ‘SmapI?Œv‚Å‚ÌÚ??æ“¾ (/w‚Ö‚Ì?“š—p)
 *------------------------------------------
 */
int map_getusers(void) {
	return map_users;
}

//
// blockíœ‚ÌˆÀ‘S«Šm•Û?—
//

/*==========================================
 * block‚ğfree‚·‚é‚Æ‚«free‚Ì?‚í‚è‚ÉŒÄ‚Ô
 * ƒƒbƒN‚³‚ê‚Ä‚¢‚é‚Æ‚«‚Íƒoƒbƒtƒ@‚É‚½‚ß‚é
 *------------------------------------------
 */
int map_freeblock (struct block_list *bl)
{
	if (block_free_lock == 0 || block_free_count >= block_free_max)
	{	//Directly calling aFree shouldn't be a leak, as Free remembers the size the original pointed to memory was allocated with? [Skotlex]
		aFree(bl);
		bl = NULL;
		if (block_free_count >= block_free_max)
			if (battle_config.error_log)
				ShowWarning("map_freeblock: too many free block! %d %d\n",
					block_free_count, block_free_lock);
	} else
		block_free[block_free_count++] = bl;

	return block_free_lock;
}
/*==========================================
 * block‚Ìfree‚ğˆêsI‚É‹Ö~‚·‚é
 *------------------------------------------
 */
int map_freeblock_lock (void)
{
	return ++block_free_lock;
}

/*==========================================
 * block‚Ìfree‚ÌƒƒbƒN‚ğ‰ğœ‚·‚é
 * ‚±‚Ì‚Æ‚«AƒƒbƒN‚ªŠ®‘S‚É‚È‚­‚È‚é‚Æ
 * ƒoƒbƒtƒ@‚É‚½‚Ü‚Á‚Ä‚¢‚½block‚ğ‘S•”íœ
 *------------------------------------------
 */
int map_freeblock_unlock (void)
{
	if ((--block_free_lock) == 0) {
		int i;
		for (i = 0; i < block_free_count; i++)
		{	//Directly calling aFree shouldn't be a leak, as Free remembers the size the original pointed to memory was allocated with? [Skotlex]
			aFree(block_free[i]);
			block_free[i] = NULL;
		}
		block_free_count = 0;
	} else if (block_free_lock < 0) {
		if (battle_config.error_log)
			ShowError("map_freeblock_unlock: lock count < 0 !\n");
		block_free_lock = 0; // Ÿ‰ñˆÈ~‚ÌƒƒbƒN‚Éxá‚ªo‚Ä‚­‚é‚Ì‚ÅƒŠƒZƒbƒg
	}

	return block_free_lock;
}

// map_freeblock_lock() ‚ğŒÄ‚ñ‚Å map_freeblock_unlock() ‚ğŒÄ‚Î‚È‚¢
// ŠÖ”‚ª‚ ‚Á‚½‚Ì‚ÅA’èŠú“I‚Éblock_free_lock‚ğƒŠƒZƒbƒg‚·‚é‚æ‚¤‚É‚·‚éB
// ‚±‚ÌŠÖ”‚ÍAdo_timer() ‚ÌƒgƒbƒvƒŒƒxƒ‹‚©‚çŒÄ‚Î‚ê‚é‚Ì‚ÅA
// block_free_lock ‚ğ’¼Ú‚¢‚¶‚Á‚Ä‚àxá–³‚¢‚Í‚¸B

int map_freeblock_timer (int tid, unsigned int tick, int id, int data)
{
	if (block_free_lock > 0) {
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", block_free_lock);
		block_free_lock = 1;
		map_freeblock_unlock();
	}

	return 0;
}

//
// block‰»?—
//
/*==========================================
 * map[]‚Ìblock_list‚©‚ç?‚ª‚Á‚Ä‚¢‚éê‡‚É
 * bl->prev‚Ébl_head‚ÌƒAƒhƒŒƒX‚ğ“ü‚ê‚Ä‚¨‚­
 *------------------------------------------
 */
static struct block_list bl_head;

/*==========================================
 * map[]‚Ìblock_list‚É’Ç‰Á
 * mob‚Í?‚ª‘½‚¢‚Ì‚Å•ÊƒŠƒXƒg
 *
 * ?‚Élink?‚İ‚©‚ÌŠm”F‚ª–³‚¢BŠë?‚©‚à
 *------------------------------------------
 */
int map_addblock (struct block_list *bl)
{
	int m, x, y, pos;

	nullpo_retr(0, bl);

	if (bl->prev != NULL) {
		if(battle_config.error_log)
			ShowError("map_addblock error : bl->prev != NULL\n");
		return 0;
	}

	m = bl->m;
	x = bl->x;
	y = bl->y;
	if (m < 0 || m >= map_num ||
		x < 0 || x >= map[m].xs ||
		y < 0 || y >= map[m].ys)
		return 1;

	pos = x/BLOCK_SIZE+(y/BLOCK_SIZE)*map[m].bxs;
	if (bl->type == BL_MOB) {
		bl->next = map[m].block_mob[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		map[m].block_mob[pos] = bl;
		map[m].block_mob_count[pos]++;
	} else {
		bl->next = map[m].block[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		map[m].block[pos] = bl;
		map[m].block_count[pos]++;
		if (bl->type == BL_PC)
		{
			struct map_session_data* sd;
			if (map[m].users++ == 0 && battle_config.dynamic_mobs)	//Skotlex
				map_spawnmobs(m);
			sd = (struct map_session_data*)bl;
			if (agit_flag && battle_config.pet_no_gvg && map[m].flag.gvg && sd->pd)
			{	//Return the pet to egg. [Skotlex]
				clif_displaymessage(sd->fd, "Pets are not allowed in Guild Wars.");
				pet_menu(sd, 3); //Option 3 is return to egg.
			}
		}
	}

	return 0;
}

/*==========================================
 * map[]‚Ìblock_list‚©‚çŠO‚·
 * prev‚ªNULL‚Ìê‡list‚É?‚ª‚Á‚Ä‚È‚¢
 *------------------------------------------
 */
int map_delblock (struct block_list *bl)
{
	int b;
	nullpo_retr(0, bl);

	// ?‚Éblocklist‚©‚ç?‚¯‚Ä‚¢‚é
	if (bl->prev == NULL) {
		if (bl->next != NULL) {
			// prev‚ªNULL‚Ånext‚ªNULL‚Å‚È‚¢‚Ì‚Í—L‚Á‚Ä‚Í‚È‚ç‚È‚¢
			if(battle_config.error_log)
				ShowError("map_delblock error : bl->next!=NULL\n");
		}
		return 0;
	}

	b = bl->x/BLOCK_SIZE+(bl->y/BLOCK_SIZE)*map[bl->m].bxs;

	if (bl->type == BL_PC)
		if (--map[bl->m].users == 0 && battle_config.dynamic_mobs)	//[Skotlex]
			map_removemobs(bl->m);

	if (bl->next)
		bl->next->prev = bl->prev;
	if (bl->prev == &bl_head) {
		// ƒŠƒXƒg‚Ì“ª‚È‚Ì‚ÅAmap[]‚Ìblock_list‚ğXV‚·‚é
		if (bl->type == BL_MOB) {
			map[bl->m].block_mob[b] = bl->next;
			if ((map[bl->m].block_mob_count[b]--) < 0)
				map[bl->m].block_mob_count[b] = 0;
		} else {
			map[bl->m].block[b] = bl->next;
			if((map[bl->m].block_count[b]--) < 0)
				map[bl->m].block_count[b] = 0;
		}
	} else {
		bl->prev->next = bl->next;
	}
	bl->next = NULL;
	bl->prev = NULL;

	return 0;
}

/*==========================================
 * ü?‚ÌPCl?‚ğ?‚¦‚é (unused)
 *------------------------------------------
 */
int map_countnearpc (int m, int x, int y)
{
	int bx, by, c = 0;
	struct block_list *bl=NULL;

	if (map[m].users == 0)
		return 0;
	for (by = y/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1; by<=y/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1; by++) {
		if (by < 0 || by >= map[m].bys)
			continue;
		for (bx = x/BLOCK_SIZE-AREA_SIZE/BLOCK_SIZE-1; bx <= x/BLOCK_SIZE+AREA_SIZE/BLOCK_SIZE+1; bx++) {
			if (bx < 0 || bx >= map[m].bxs)
				continue;
			bl = map[m].block[bx+by*map[m].bxs];
			while(bl) {
				if (bl->type == BL_PC)
					c++;
				bl = bl->next;
			}
		}
	}

	return c;
}

/*==========================================
 * Counts specified number of objects on given cell.
 *------------------------------------------
 */
int map_count_oncell(int m, int x, int y, int type) {
	int bx,by;
	struct block_list *bl=NULL;
	int i,c;
	int count = 0;

	if (x < 0 || y < 0 || (x >= map[m].xs) || (y >= map[m].ys))
		return 0;
	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	if (type == 0 || type != BL_MOB)
	{
		bl = map[m].block[bx+by*map[m].bxs];
		c = map[m].block_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next){
			if(bl->x == x && bl->y == y && bl->type == BL_PC) count++;
		}
	}
	
	if (type == 0 || type == BL_MOB)
	{
		bl = map[m].block_mob[bx+by*map[m].bxs];
		c = map[m].block_mob_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next){
			if(bl->x == x && bl->y == y) count++;
		}
	}
	return count;
}
/*
 * «»«E¾ªÎõÌôøªËÌ¸ªÄª±ª¿«¹«­«Eæ«Ë«Ã«ÈªòÚ÷ª¹
 */
struct skill_unit *map_find_skill_unit_oncell(struct block_list *target,int x,int y,int skill_id,struct skill_unit *out_unit)
{
	int m,bx,by;
	struct block_list *bl;
	int i,c;
	struct skill_unit *unit;
	m = target->m;

	if (x < 0 || y < 0 || (x >= map[m].xs) || (y >= map[m].ys))
		return NULL;
	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	bl = map[m].block[bx+by*map[m].bxs];
	c = map[m].block_count[bx+by*map[m].bxs];
	for(i=0;i<c && bl;i++,bl=bl->next){
		if (bl->x != x || bl->y != y || bl->type != BL_SKILL)
			continue;
		unit = (struct skill_unit *) bl;
		if (unit==out_unit || !unit->alive ||
				!unit->group || unit->group->skill_id!=skill_id)
			continue;
		if (battle_check_target(&unit->bl,target,unit->group->target_flag)>0)
			return unit;
	}
	return NULL;
}

/*==========================================
 * map m (x0,y0)-(x1,y1)?‚Ì‘Sobj‚É?‚µ‚Ä
 * func‚ğŒÄ‚Ô
 * type!=0 ‚È‚ç‚»‚Ìí—Ş‚Ì‚İ
 *------------------------------------------
 */
int map_foreachinarea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int type,...) {
	va_list ap;
	int bx,by;
	int returnCount =0;	//total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	int blockcount=bl_list_count,i,c;

	if (m < 0)
		return 0;
	va_start(ap,type);
	if (x1 < x0)
	{	//Swap range
		bx = x0;
		x0 = x1;
		x1 = bx;
	}
	if (y1 < y0)
	{
		bx = y0;
		y0 = y1;
		y1 = bx;
	}
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;
	
	if (type == 0 || type != BL_MOB)
		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	if(type==0 || type==BL_MOB)
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinarea: block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]
}

/*==========================================
 * ‹éŒ`(x0,y0)-(x1,y1)‚ª(dx,dy)ˆÚ“®‚µ‚½bÌ
 * —ÌˆæŠO‚É‚È‚é—Ìˆæ(‹éŒ`‚©LšŒ`)?‚Ìobj‚É
 * ?‚µ‚Äfunc‚ğŒÄ‚Ô
 *
 * dx,dy‚Í-1,0,1‚Ì‚İ‚Æ‚·‚éi‚Ç‚ñ‚È’l‚Å‚à‚¢‚¢‚Á‚Û‚¢Hj
 *------------------------------------------
 */
int map_foreachinmovearea(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int dx,int dy,int type,...) {
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);
	if (x1 < x0)
	{	//Swap range
		bx = x0;
		x0 = x1;
		x1 = bx;
	}
	if (y1 < y0)
	{
		bx = y0;
		y0 = y1;
		y1 = bx;
	}
	if(dx==0 || dy==0){
		// ‹éŒ`—Ìˆæ‚Ìê‡
		if(dx==0){
			if(dy<0){
				y0=y1+dy+1;
			} else {
				y1=y0+dy-1;
			}
		} else if(dy==0){
			if(dx<0){
				x0=x1+dx+1;
			} else {
				x1=x0+dx-1;
			}
		}
		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=map[m].xs) x1=map[m].xs-1;
		if(y1>=map[m].ys) y1=map[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1 && bl_list_count<BL_LIST_MAX)
						bl_list[bl_list_count++]=bl;
				}
			}
		}
	}else{
		// Lš—Ìˆæ‚Ìê‡

		if(x0<0) x0=0;
		if(y0<0) y0=0;
		if(x1>=map[m].xs) x1=map[m].xs-1;
		if(y1>=map[m].ys) y1=map[m].ys-1;
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if((bl) && !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
						continue;
					if((bl) && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
						(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
						bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
				}
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if((bl) && !(bl->x>=x0 && bl->x<=x1 && bl->y>=y0 && bl->y<=y1))
						continue;
					if((bl) && ((dx>0 && bl->x<x0+dx) || (dx<0 && bl->x>x1+dx) ||
						(dy>0 && bl->y<y0+dy) || (dy<0 && bl->y>y1+dy)) &&
						bl_list_count<BL_LIST_MAX)
							bl_list[bl_list_count++]=bl;
				}
			}
		}

	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinarea: block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev) {	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			if (bl_list[i]->type == BL_PC
			  && session[((struct map_session_data *) bl_list[i])->fd] == NULL)
				continue;
			returnCount += func(bl_list[i],ap);
		}

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;
}

// -- moonsoul	(added map_foreachincell which is a rework of map_foreachinarea but
//			 which only checks the exact single x/y passed to it rather than an
//			 area radius - may be more useful in some instances)
//
int map_foreachincell(int (*func)(struct block_list*,va_list),int m,int x,int y,int type,...) {
	int bx,by;
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
	struct block_list *bl=NULL;
	va_list ap;
	int blockcount=bl_list_count,i,c;

	va_start(ap,type);

	by=y/BLOCK_SIZE;
	bx=x/BLOCK_SIZE;

	if(type==0 || type!=BL_MOB)
	{
		bl = map[m].block[bx+by*map[m].bxs];
		c = map[m].block_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(type && bl && bl->type!=type)
				continue;
			if(bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
				bl_list[bl_list_count++]=bl;
		}
	}

	if(type==0 || type==BL_MOB)
	{
		bl = map[m].block_mob[bx+by*map[m].bxs];
		c = map[m].block_mob_count[bx+by*map[m].bxs];
		for(i=0;i<c && bl;i++,bl=bl->next)
		{
			if(bl && bl->x==x && bl->y==y && bl_list_count<BL_LIST_MAX)
				bl_list[bl_list_count++]=bl;
		}
	}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachincell: block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;
}

/*============================================================
* For checking a path between two points (x0, y0) and (x1, y1)
*------------------------------------------------------------
 */
int map_foreachinpath(int (*func)(struct block_list*,va_list),int m,int x0,int y0,int x1,int y1,int range,int type,...)
{
	int returnCount =0;  //total sum of returned values of func() [Skotlex]
/*	va_list ap;
	double deltax = 0.0;
	double deltay = 0.0;
	int t, bx, by;
	int *xs, *ys;
	int blockcount = bl_list_count, i, c;
	struct block_list *bl = NULL;

	if(m < 0)
		return;
	va_start(ap,type);
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;

	// I'm not finished thinking on it
	// but first it might better use a parameter equation
	// x=(x1-x0)*t+x0; y=(y1-y0)*t+y0; t=[0,1]
	// would not need special case aproximating for infinity/zero slope
	// so maybe this way:

	// find maximum runindex
	int tmax = abs(y1-y0);
	if(tmax < abs(x1-x0))
		tmax = abs(x1-x0);

	xs = (int *)aCallocA(tmax + 1, sizeof(int));
	ys = (int *)aCallocA(tmax + 1, sizeof(int));

	// pre-calculate delta values for x and y destination
	// should speed up cause you don't need to divide in the loop
	if(tmax>0)
	{
		deltax = ((double)(x1-x0)) / ((double)tmax);
		deltay = ((double)(y1-y0)) / ((double)tmax);
	}
	// go along the index
	for(t=0; t<=tmax; t++)
	{
		int x = (int)floor(deltax * (double)t +0.5)+x0;
		int y = (int)floor(deltay * (double)t +0.5)+y0;
		// the xy pairs of points in line between x0y0 and x1y1
		// including start and end point
		xs[t] = x;
		ys[t] = y;
	}

	if (type == 0 || type != BL_MOB)


this here is  wrong,
there is no check if x0<x1 and y0<y1
but this is not valid in 3 of 4 cases,
so in this case here you check only blocks when shooting to a positive direction
shooting in other directions just do nothing like the skill has failed
if you want to keep this that way then check and swap x0,y0 with x1,y1

		for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl) {
						if (type && bl->type!=type)
							continue;
						for(t=0; t<=tmax; t++)
							if(bl->x==xs[t] && bl->y==ys[t] && bl_list_count<BL_LIST_MAX)
								bl_list[bl_list_count++]=bl;
					}
				}
			}
		}
	if(type==0 || type==BL_MOB)
		for(by=y0/BLOCK_SIZE;by<=y1/BLOCK_SIZE;by++){
			for(bx=x0/BLOCK_SIZE;bx<=x1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl) {
						for(t=0; t<=tmax; t++)
							if(bl->x==xs[t] && bl->y==ys[t] && bl_list_count<BL_LIST_MAX)
								bl_list[bl_list_count++]=bl;
					}
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			printf("map_foreachinarea: *WARNING* block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	bl_list_count = blockcount;
	aFree (xs);
	aFree (ys);
 	va_end(ap);

*/

//////////////////////////////////////////////////////////////
//
// sharp shooting 1
//
//////////////////////////////////////////////////////////////
// problem:
// finding targets standing on and within some range of a line
// (t1,t2 t3 and t4 get hit)
//
//     target 1
//      x t4
//     t2
// t3 x
//   x
//  S
//////////////////////////////////////////////////////////////
// solution 1 (straight forward, but a bit calculation expensive)
// calculating perpendiculars from quesionable mobs to the straight line
// if the mob is hit then depends on the distance to the line
//
// solution 2 (complex, need to handle many cases, but maybe faster)
// make a formula to deside if a given (x,y) is within a shooting area
// the shape can be ie. rectangular or triangular
// if the mob is hit then depends on if the mob is inside or outside the area
// I'm not going to implement this, but if somebody is interested
// in vector algebra, it might be some fun

//////////////////////////////////////////////////////////////
// possible shooting ranges (I prefer the second one)
//////////////////////////////////////////////////////////////
//
//  ----------------                     ------
//  ----------------               ------------
// Sxxxxxxxxxxxxxxxxtarget    Sxxxxxxxxxxxxxxxxtarget
//  ----------------               ------------
//  ----------------                      -----
//
// the original code implemented the left structure
// might be not that realistic, so I changed to the other one
// I take "range" as max distance from the line
//////////////////////////////////////////////////////////////
/*
	va_list ap;
	int i, blockcount = bl_list_count;
	struct block_list *bl;
	int c1,c2;

///////////
	double deltax,deltay;
	double k,kfact,knorm;
	double v1,v2,distance;
	double xm,ym,rd;
	int bx,by,bx0,bx1,by0,by1;
//////////////
	// no map
	if(m < 0) return 0;

	// xy out of range
	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x1 >= map[m].xs) x1 = map[m].xs-1;
	if (y1 >= map[m].ys) y1 = map[m].ys-1;

	///////////////////////////////
	// stuff for a linear equation in xy coord to calculate
	// the perpendicular from a block xy to the straight line
	deltax = (x1-x0);
	deltay = (y1-y0);
	kfact = (deltax*deltax+deltay*deltay);	// the sqare length of the line
	knorm = -deltax*x0-deltay*y0;			// the offset vector param

//printf("(%i,%i)(%i,%i) range: %i\n",x0,y0,x1,y1,range);

	if(kfact==0) return 0; // shooting at the standing position should not happen
	kfact = 1/kfact; // divide here and multiply in the loop

	range *= range; // compare with range^2 so we can skip a sqrt and signs

	///////////////////////////////
	// prepare shooting area check
	xm = (x1+x0)/2.0;
	ym = (y1+y0)/2.0;// middle point on the shooting line
	// the sqared radius of a circle around the shooting range
	// plus the sqared radius of a block
	rd = (x0-xm)*(x0-xm) + (y0-ym)*(y0-ym) + (range*range)
					+BLOCK_SIZE*BLOCK_SIZE/2;
	// so whenever a block midpoint is within this circle
	// some of the block area is possibly within the shooting range

	///////////////////////////////
	// what blocks we need to test
	// blocks covered by the xy position of begin and end of the line
	bx0 = x0/BLOCK_SIZE;
	bx1 = x1/BLOCK_SIZE;
	by0 = y0/BLOCK_SIZE;
	by1 = y1/BLOCK_SIZE;
	// swap'em for a smallest-to-biggest run
	if(bx0>bx1)	swap(bx0,bx1);
	if(by0>by1)	swap(by0,by1);

	// enlarge the block area by a range value and 1
	// so we can be sure to process all blocks that might touch the shooting area
	// in this case here with BLOCK_SIZE=8 and range=2 it will be only enlarged by 1
	// but I implement it anyway just in case that ranges will be larger
	// or BLOCK_SIZE smaller in future
	i = (range/BLOCK_SIZE+1);//temp value
	if(bx0>i)				bx0 -=i; else bx0=0;
	if(by0>i)				by0 -=i; else by0=0;
	if(bx1+i<map[m].bxs)	bx1 +=i; else bx1=map[m].bxs-1;
	if(by1+i<map[m].bys)	by1 +=i; else by1=map[m].bys-1;


//printf("run for (%i,%i)(%i,%i)\n",bx0,by0,bx1,by1);
	for(bx=bx0; bx<=bx1; bx++)
	for(by=by0; by<=by1; by++)
	{	// block xy
		c1  = map[m].block_count[bx+by*map[m].bxs];		// number of elements in the block
		c2  = map[m].block_mob_count[bx+by*map[m].bxs];	// number of mobs in the mob block
		if( (c1==0) && (c2==0) ) continue;				// skip if nothing in the blocks

//printf("block(%i,%i) %i %i\n",bx,by,c1,c2);fflush(stdout);
		// test if the mid-point of the block is too far away
		// so we could skip the whole block in this case
		v1 = (bx*BLOCK_SIZE+BLOCK_SIZE/2-xm)*(bx*BLOCK_SIZE+BLOCK_SIZE/2-xm)
			+(by*BLOCK_SIZE+BLOCK_SIZE/2-ym)*(by*BLOCK_SIZE+BLOCK_SIZE/2-ym);
//printf("block(%i,%i) v1=%f rd=%f\n",bx,by,v1,rd);fflush(stdout);
		// check for the worst case scenario
		if(v1 > rd)	continue;

		// it seems that the block is at least partially covered by the shooting range
		// so we go into it
		if(type==0 || type!=BL_MOB) {
  			bl = map[m].block[bx+by*map[m].bxs];		// a block with the elements
			for(i=0;i<c1 && bl;i++,bl=bl->next){		// go through all elements
				if( bl && ( !type || bl->type==type ) && bl_list_count<BL_LIST_MAX )
				{
					// calculate the perpendicular from block xy to the straight line
					k = kfact*(deltax*bl->x + deltay*bl->y + knorm);
					// check if the perpendicular is within start and end of our line
					if(k>=0 && k<=1)
					{	// calculate the distance
						v1 = deltax*k+x0 - bl->x;
						v2 = deltay*k+y0 - bl->y;
						distance = v1*v1+v2*v2;
						// triangular shooting range
						if( distance <= range*k )
							bl_list[bl_list_count++]=bl;
					}
				}
			}//end for elements
		}

		if(type==0 || type==BL_MOB) {
			bl = map[m].block_mob[bx+by*map[m].bxs];	// and the mob block
			for(i=0;i<c2 && bl;i++,bl=bl->next){
				if(bl && bl_list_count<BL_LIST_MAX) {
					// calculate the perpendicular from block xy to the straight line
					k = kfact*(deltax*bl->x + deltay*bl->y + knorm);
//printf("mob: (%i,%i) k=%f ",bl->x,bl->y, k);
					// check if the perpendicular is within start and end of our line
					if(k>=0 && k<=1)
					{
						 v1 = deltax*k+x0 - bl->x;
						 v2 = deltay*k+y0 - bl->y;
						 distance = v1*v1+v2*v2;
//printf("dist: %f",distance);
						 // triangular shooting range
						 if( distance <= range*k )
						 {
//printf("  hit");
							bl_list[bl_list_count++]=bl;
						 }
					}
//printf("\n");
				}
			}//end for mobs
		}
	}//end for(bx,by)


	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			printf("map_foreachinpath: *WARNING* block count too many!\n");
	}

	va_start(ap,type);
	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	va_end(ap);

	bl_list_count = blockcount;

*/

//////////////////////////////////////////////////////////////
//
// sharp shooting 2
//
//////////////////////////////////////////////////////////////
// problem:
// finding targets standing exactly on a line
// (only t1 and t2 get hit)
//
//     target 1
//      x t4
//     t2
// t3 x
//   x
//  S
//////////////////////////////////////////////////////////////
/*
	va_list ap;
	int i, blockcount = bl_list_count;
	struct block_list *bl;
	int c1,c2;

	//////////////////////////////////////////////////////////////
	// linear parametric equation
	// x=(x1-x0)*t+x0; y=(y1-y0)*t+y0; t=[0,1]
	//////////////////////////////////////////////////////////////
	// linear equation for finding a single line between (x0,y0)->(x1,y1)
	// independent of the given xy-values
	double dx = 0.0;
	double dy = 0.0;
	int bx=-1;	// initialize block coords to some impossible value
	int by=-1;

	int t;
	///////////////////////////////
	// find maximum runindex
	int tmax = abs(y1-y0);
	if(tmax  < abs(x1-x0))
		tmax = abs(x1-x0);
	// pre-calculate delta values for x and y destination
	// should speed up cause you don't need to divide in the loop
	if(tmax>0)
	{
		dx = ((double)(x1-x0)) / ((double)tmax);
		dy = ((double)(y1-y0)) / ((double)tmax);
	}
	// go along the index
	for(t=0; t<=tmax; t++)
	{	// xy-values of the line including start and end point
		int x = (int)floor(dx * (double)t +0.5)+x0;
		int y = (int)floor(dy * (double)t +0.5)+y0;

		// check the block index of the calculated xy
		if( (bx!=x/BLOCK_SIZE) || (by!=y/BLOCK_SIZE) )
		{	// we have reached a new block
			// so we store the current block coordinates
			bx = x/BLOCK_SIZE;
			by = y/BLOCK_SIZE;

			// and process the data
			c1  = map[m].block_count[bx+by*map[m].bxs];		// number of elements in the block
			c2  = map[m].block_mob_count[bx+by*map[m].bxs];	// number of mobs in the mob block
			if( (c1==0) && (c2==0) ) continue;				// skip if nothing in the block

			if(type==0 || type!=BL_MOB) {
				bl = map[m].block[bx+by*map[m].bxs];		// a block with the elements
				for(i=0;i<c1 && bl;i++,bl=bl->next){		// go through all elements
					if( bl && ( !type || bl->type==type ) && bl_list_count<BL_LIST_MAX )
					{
						// check if block xy is on the line
						if( (bl->x-x0)*(y1-y0) == (bl->y-y0)*(x1-x0) )
						// and if it is within start and end point
						if( (((x0<=x1)&&(x0<=bl->x)&&(bl->x<=x1)) || ((x0>=x1)&&(x0>=bl->x)&&(bl->x>=x1))) &&
							(((y0<=y1)&&(y0<=bl->y)&&(bl->y<=y1)) || ((y0>=y1)&&(y0>=bl->y)&&(bl->y>=y1))) )
							bl_list[bl_list_count++]=bl;
					}
				}//end for elements
			}

			if(type==0 || type==BL_MOB) {
				bl = map[m].block_mob[bx+by*map[m].bxs];	// and the mob block
				for(i=0;i<c2 && bl;i++,bl=bl->next){
					if(bl && bl_list_count<BL_LIST_MAX) {
						// check if mob xy is on the line
						if( (bl->x-x0)*(y1-y0) == (bl->y-y0)*(x1-x0) )
						// and if it is within start and end point
						if( (((x0<=x1)&&(x0<=bl->x)&&(bl->x<=x1)) || ((x0>=x1)&&(x0>=bl->x)&&(bl->x>=x1))) &&
							(((y0<=y1)&&(y0<=bl->y)&&(bl->y<=y1)) || ((y0>=y1)&&(y0>=bl->y)&&(bl->y>=y1))) )
							bl_list[bl_list_count++]=bl;
					}
				}//end for mobs
			}
		}
	}//end for index

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinarea: block count too many!\n");
	}

	va_start(ap,type);
	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é
	va_end(ap);

	bl_list_count = blockcount;

	return returnCount;
*/
//////////////////////////////////////////////////////////////
//
// sharp shooting 3 [Skotlex]
//
//////////////////////////////////////////////////////////////
// problem:
// Same as Sharp Shooting 1. Hits all targets within range of
// the line.
// (t1,t2 t3 and t4 get hit)
//
//     target 1
//      x t4
//     t2
// t3 x
//   x
//  S
//////////////////////////////////////////////////////////////
// Methodology: 
// My trigonometrics and math is a little rusty... so the approach I am writing 
// here is basicly do a double for to check for all targets in the square that 
// contains the initial and final positions (area range increased to match the 
// radius given), then for each object to test, calculate the distance to the 
// path and include it if the range fits and the target is in the line (0<k<1,
// as they call it).
// The implementation I took as reference is found at 
// http://astronomy.swin.edu.au/~pbourke/geometry/pointline/ 
// (they have a link to a C implementation, too)
// This approach is a lot like #2 commented on this function, which I have no 
// idea why it was commented. I won't use doubles/floats, but pure int math for speed purposes
// The range considered is always the same no matter how close/far the target is because that's
// how SharpShooting works currently in kRO.

	//Generic map_foreach* variables.
	va_list ap;
	int i, blockcount = bl_list_count;
	struct block_list *bl;
	int c, bx, by;
	//method specific variables
	int magnitude2; //The square of the magnitude
	int k, xi, yi, xu, yu;
	int mx0 = x0, mx1 = x1, my0 = y0, my1 = y1;
	
	//Avoid needless calculations by not getting the sqrt right away.
	#define MAGNITUDE2(x0, y0, x1, y1) (((x1)-(x0))*((x1)-(x0)) + ((y1)-(y0))*((y1)-(y0)))
	
	if (m < 0)
		return 0;
		
	va_start(ap,type);

	//Expand target area to cover range.
	if (mx0 > mx1)
	{
		mx0+=range;
		mx1-=range;
	} else {
		mx0-=range;
		mx1+=range;
	}
	if (my0 > my1)
	{
		my0+=range;
		my1-=range;
	} else {
		my0-=range;
		my1+=range;
	}

	//The two fors assume mx0 < mx1 && my0 < my1
	if (mx0 > mx1)
	{
		k = mx1;
		mx1 = mx0;
		mx0 = k;
	}
	if (my0 > my1)
	{
		k = my1;
		my1 = my0;
		my0 = k;
	}
	
	if (mx0 < 0) mx0 = 0;
	if (my0 < 0) my0 = 0;
	if (mx1 >= map[m].xs) mx1 = map[m].xs-1;
	if (my1 >= map[m].ys) my1 = map[m].ys-1;
	
	range*=range<<8; //Values are shifted later on for higher precision using int math.
	magnitude2 = MAGNITUDE2(x0,y0, x1,y1);
	if (magnitude2 < 1) //Same begin and ending point, can't trace path.
		return 0;
	
	if (type == 0 || type != BL_MOB)
		for (by = my0 / BLOCK_SIZE; by <= my1 / BLOCK_SIZE; by++) {
			for(bx=mx0/BLOCK_SIZE;bx<=mx1/BLOCK_SIZE;bx++){
				bl = map[m].block[bx+by*map[m].bxs];
				c = map[m].block_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && type && bl->type!=type)
						continue;
					if(bl && bl_list_count<BL_LIST_MAX)
					{
						xi = bl->x;
						yi = bl->y;
					
						k = (xi-x0)*(x1-x0) + (yi-y0)*(y1-y0);
						if (k < 0)// || k > magnitude2) //No check to see if it lies after the target's point.
							continue;
					
						//All these shifts are to increase the precision of the intersection point and distance considering how it's
						//int math.
						k = (k<<4)/magnitude2; //k will be between 1~16 instead of 0~1
						xi<<=4;
						yi<<=4;
						xu= (x0<<4) +k*(x1-x0);
						yu= (y0<<4) +k*(y1-y0);
						k = MAGNITUDE2(xi, yi, xu, yu);
						
						//If all dot coordinates were <<4 the square of the magnitude is <<8
						if (k > range)
							continue;

						bl_list[bl_list_count++]=bl;
					}
				}
			}
		}
	if(type==0 || type==BL_MOB)
		for(by=my0/BLOCK_SIZE;by<=my1/BLOCK_SIZE;by++){
			for(bx=mx0/BLOCK_SIZE;bx<=mx1/BLOCK_SIZE;bx++){
				bl = map[m].block_mob[bx+by*map[m].bxs];
				c = map[m].block_mob_count[bx+by*map[m].bxs];
				for(i=0;i<c && bl;i++,bl=bl->next){
					if(bl && bl_list_count<BL_LIST_MAX)
					{
						xi = bl->x;
						yi = bl->y;
						k = (xi-x0)*(x1-x0) + (yi-y0)*(y1-y0);
						if (k < 0)// || k > magnitude2) //No check to see if it lies after the target's point.
							continue;
					
						k = (k<<4)/magnitude2; //k will be between 1~16 instead of 0~1
						xi<<=4;
						yi<<=4;
						xu= (x0<<4) +k*(x1-x0);
						yu= (y0<<4) +k*(y1-y0);
						k = MAGNITUDE2(xi, yi, xu, yu);
						
						//If all dot coordinates were <<4 the square of the magnitude is <<8
						if (k > range)
							continue;

						bl_list[bl_list_count++]=bl;
					}
				}
			}
		}

	if(bl_list_count>=BL_LIST_MAX) {
		if(battle_config.error_log)
			ShowWarning("map_foreachinpath: block count too many!\n");
	}

	map_freeblock_lock();	// ƒƒ‚ƒŠ‚©‚ç‚Ì‰ğ•ú‚ğ‹Ö~‚·‚é

	for(i=blockcount;i<bl_list_count;i++)
		if(bl_list[i]->prev)	// —L?‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN
			returnCount += func(bl_list[i],ap);

	map_freeblock_unlock();	// ‰ğ•ú‚ğ‹–‰Â‚·‚é

	va_end(ap);
	bl_list_count = blockcount;
	return returnCount;	//[Skotlex]

}

/*==========================================
 * °ƒAƒCƒeƒ€‚âƒGƒtƒFƒNƒg—p‚ÌˆêObjŠ„‚è?‚Ä
 * object[]‚Ö‚Ì•Û‘¶‚Æid_db“o?‚Ü‚Å
 *
 * bl->id‚à‚±‚Ì’†‚Åİ’è‚µ‚Ä–â‘è–³‚¢?
 *------------------------------------------
 */
int map_addobject(struct block_list *bl) {
	int i;
	if( bl == NULL ){
		ShowWarning("map_addobject nullpo?\n");
		return 0;
	}
	if(first_free_object_id<2 || first_free_object_id>=MAX_FLOORITEM)
		first_free_object_id=2;
	for(i=first_free_object_id;i<MAX_FLOORITEM;i++)
		if(objects[i]==NULL)
			break;
	if(i>=MAX_FLOORITEM){
		if(battle_config.error_log)
			ShowWarning("no free object id\n");
		return 0;
	}
	first_free_object_id=i;
	if(last_object_id<i)
		last_object_id=i;
	objects[i]=bl;
	numdb_insert(id_db,i,bl);
	return i;
}

/*==========================================
 * ˆêObject‚Ì‰ğ•ú
 *	map_delobject‚Ìfree‚µ‚È‚¢ƒo?ƒWƒ‡ƒ“
 *------------------------------------------
 */
int map_delobjectnofree(int id) {
	if(objects[id]==NULL)
		return 0;

	map_delblock(objects[id]);
	numdb_erase(id_db,id);
	objects[id]=NULL;

	if(first_free_object_id>id)
		first_free_object_id=id;

	while(last_object_id>2 && objects[last_object_id]==NULL)
		last_object_id--;

	return 0;
}

/*==========================================
 * ˆêObject‚Ì‰ğ•ú
 * block_list‚©‚ç‚ÌíœAid_db‚©‚ç‚Ìíœ
 * object data‚ÌfreeAobject[]‚Ö‚ÌNULL‘ã“ü
 *
 * add‚Æ‚Ì??«‚ª–³‚¢‚Ì‚ª?‚É‚È‚é
 *------------------------------------------
 */
int map_delobject(int id) {
	struct block_list *obj = objects[id];

	if(obj==NULL)
		return 0;

	map_delobjectnofree(id);
	map_freeblock(obj);

	return 0;
}

/*==========================================
 * ‘SˆêObj‘Šè‚Éfunc‚ğŒÄ‚Ô
 *
 *------------------------------------------
 */
void map_foreachobject(int (*func)(struct block_list*,va_list),int type,...) {
	int i;
	int blockcount=bl_list_count;
	va_list ap;

	va_start(ap,type);

	for(i=2;i<=last_object_id;i++){
		if(objects[i]){
			if(type && objects[i]->type!=type)
				continue;
			if(bl_list_count>=BL_LIST_MAX) {
				if(battle_config.error_log)
					ShowWarning("map_foreachobject: too many blocks !\n");
			}
			else
				bl_list[bl_list_count++]=objects[i];
		}
	}

	map_freeblock_lock();

	for(i=blockcount;i<bl_list_count;i++)
		if( bl_list[i]->prev || bl_list[i]->next )
			func(bl_list[i],ap);

	map_freeblock_unlock();

	va_end(ap);
	bl_list_count = blockcount;
}

/*==========================================
 * °ƒAƒCƒeƒ€‚ğÁ‚·
 *
 * data==0‚ÌbÍtimer‚ÅÁ‚¦‚½ê * data!=0‚ÌbÍE‚¤“™‚ÅÁ‚¦‚½bÆ‚µ‚Ä“®ì
 *
 * ŒãÒ‚ÍAmap_clearflooritem(id)‚Ö
 * map.h?‚Å#define‚µ‚Ä‚ ‚é
 *------------------------------------------
 */
int map_clearflooritem_timer(int tid,unsigned int tick,int id,int data) {
	struct flooritem_data *fitem=NULL;

	fitem = (struct flooritem_data *)objects[id];
	if(fitem==NULL || fitem->bl.type!=BL_ITEM || (!data && fitem->cleartimer != tid)){
		if(battle_config.error_log)
			ShowError("map_clearflooritem_timer : error\n");
		return 1;
	}
	if(data)
		delete_timer(fitem->cleartimer,map_clearflooritem_timer);
	else if(fitem->item_data.card[0] == (short)0xff00)
		intif_delete_petdata( MakeDWord(fitem->item_data.card[1],fitem->item_data.card[2]) );
	clif_clearflooritem(fitem,0);
	map_delobject(fitem->bl.id);

	return 0;
}

/*==========================================
 * (m,x,y)‚Ìü?rangeƒ}ƒX?‚Ì‹ó‚«(=N“ü‰Â”\)cell‚Ì
 * ?‚©‚ç“K?‚Èƒ}ƒX–Ú‚ÌÀ•W‚ğx+(y<<16)‚Å•Ô‚·
 *
 * Œ»?range=1‚ÅƒAƒCƒeƒ€ƒhƒƒbƒv—p“r‚Ì‚İ
 *------------------------------------------
 */
int map_searchrandfreecell(int m,int x,int y,int range) {
	int free_cell,i,j;
	int* free_cells;

	if (range < 0)
		return -1;
	
	//FIXME: Would it be quicker to hardcode an array of 9 since this function is always called with range 1?
	free_cells = aCalloc((2*range+1)*(2*range+1), sizeof(int)); //better use more memory than having to wipe twice the cells. [Skotlex]
	
	for(free_cell=0,i=-range;i<=range;i++){
		if(i+y<0 || i+y>=map[m].ys)
			continue;
		for(j=-range;j<=range;j++){
			if(j+x<0 || j+x>=map[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			if(map_count_oncell(m,j+x,i+y, BL_ITEM) > 1) //Avoid item stacking to prevent against exploits. [Skotlex]
				continue;
			free_cells[free_cell++] = j+x+((i+y)<<16);
		}
	}
	if(free_cell==0)
	{
		aFree(free_cells);
		return -1;
	}
	free_cell=free_cells[rand()%free_cell];
	aFree(free_cells);
	return free_cell;
/*
	for(i=-range;i<=range;i++){
		if(i+y<0 || i+y>=map[m].ys)
			continue;
		for(j=-range;j<=range;j++){
			if(j+x<0 || j+x>=map[m].xs)
				continue;
			if(map_getcell(m,j+x,i+y,CELL_CHKNOPASS))
				continue;
			if(map_count_oncell(m,j+x,i+y, BL_ITEM) > 1) //Avoid item stacking to prevent against exploits. [Skotlex]
				continue;
			if(free_cell==0){
				x+=j;
				y+=i;
				i=range+1;
				break;
			}
			free_cell--;
		}
	}

	return x+(y<<16);
*/
}

/*==========================================
 * (m,x,y)‚ğ’†S‚É3x3ˆÈ?‚É°ƒAƒCƒeƒ€İ’u
 *
 * item_data‚ÍamountˆÈŠO‚ğcopy‚·‚é
 *------------------------------------------
 */
int map_addflooritem(struct item *item_data,int amount,int m,int x,int y,struct map_session_data *first_sd,
    struct map_session_data *second_sd,struct map_session_data *third_sd,int type) {
	int xy,r;
	unsigned int tick;
	struct flooritem_data *fitem=NULL;

	nullpo_retr(0, item_data);

	if((xy=map_searchrandfreecell(m,x,y,1))<0)
		return 0;
	r=rand();

	fitem = (struct flooritem_data *)aCalloc(1,sizeof(*fitem));
	fitem->bl.type=BL_ITEM;
	fitem->bl.prev = fitem->bl.next = NULL;
	fitem->bl.m=m;
	fitem->bl.x=xy&0xffff;
	fitem->bl.y=(xy>>16)&0xffff;
/* Unneeded because of aCalloc [Skotlex]
	fitem->first_get_id = 0;
	fitem->first_get_tick = 0;
	fitem->second_get_id = 0;
	fitem->second_get_tick = 0;
	fitem->third_get_id = 0;
	fitem->third_get_tick = 0;
*/
	fitem->bl.id = map_addobject(&fitem->bl);
	if(fitem->bl.id==0){
		aFree(fitem);
		return 0;
	}

	tick = gettick();
	if(first_sd) {
		fitem->first_get_id = first_sd->bl.id;
		if(type)
			fitem->first_get_tick = tick + battle_config.mvp_item_first_get_time;
		else
			fitem->first_get_tick = tick + battle_config.item_first_get_time;
	}
	if(second_sd) {
		fitem->second_get_id = second_sd->bl.id;
		if(type)
			fitem->second_get_tick = tick + battle_config.mvp_item_first_get_time + battle_config.mvp_item_second_get_time;
		else
			fitem->second_get_tick = tick + battle_config.item_first_get_time + battle_config.item_second_get_time;
	}
	if(third_sd) {
		fitem->third_get_id = third_sd->bl.id;
		if(type)
			fitem->third_get_tick = tick + battle_config.mvp_item_first_get_time + battle_config.mvp_item_second_get_time + battle_config.mvp_item_third_get_time;
		else
			fitem->third_get_tick = tick + battle_config.item_first_get_time + battle_config.item_second_get_time + battle_config.item_third_get_time;
	}

	memcpy(&fitem->item_data,item_data,sizeof(*item_data));
	fitem->item_data.amount=amount;
	fitem->subx=(r&3)*3+3;
	fitem->suby=((r>>2)&3)*3+3;
	fitem->cleartimer=add_timer(gettick()+battle_config.flooritem_lifetime,map_clearflooritem_timer,fitem->bl.id,0);

	map_addblock(&fitem->bl);
	clif_dropflooritem(fitem);

	return fitem->bl.id;
}

/*==========================================
 * charid_db‚Ö’Ç‰Á(•ÔM‘Ò‚¿‚ª‚ ‚ê‚Î•ÔM)
 *------------------------------------------
 */
void map_addchariddb(int charid, char *name) {
	struct charid2nick *p;
	int req = 0;

	p = (struct charid2nick*)numdb_search(charid_db,charid);
	if (p == NULL){	// ƒf?ƒ^ƒx?ƒX‚É‚È‚¢
		p = (struct charid2nick *)aCallocA(1, sizeof(struct charid2nick));
	} else {
		numdb_erase(charid_db, charid);
		req = p->req_id;
	}

	p->req_id = 0;
	memcpy(p->nick, name, NAME_LENGTH-1);
	numdb_insert(charid_db, charid, p);

	if (req) {	// •ÔM‘Ò‚¿‚ª‚ ‚ê‚Î•ÔM
		struct map_session_data *sd = map_id2sd(req);
		if (sd != NULL)
			clif_solved_charname(sd,charid);
	}
}

/*==========================================
 * charid_db‚Ö’Ç‰Ái•ÔM—v‹‚Ì‚İj
 *------------------------------------------
 */
int map_reqchariddb(struct map_session_data * sd,int charid) {
	struct charid2nick *p=NULL;

	nullpo_retr(0, sd);

	p = (struct charid2nick*)numdb_search(charid_db,charid);
	if(p!=NULL)	// ƒf?ƒ^ƒx?ƒX‚É‚·‚Å‚É‚ ‚é
		return 0;
	p = (struct charid2nick *)aCalloc(1,sizeof(struct charid2nick));
	p->req_id=sd->bl.id;
	numdb_insert(charid_db,charid,p);
	return 0;
}

/*==========================================
 * id_db‚Öbl‚ğ’Ç‰Á
 *------------------------------------------
 */
void map_addiddb(struct block_list *bl) {
	nullpo_retv(bl);

	if (bl->type == BL_PC)
		numdb_insert(pc_db,bl->id,bl);
	numdb_insert(id_db,bl->id,bl);
}

/*==========================================
 * id_db‚©‚çbl‚ğíœ
 *------------------------------------------
 */
void map_deliddb(struct block_list *bl) {
	nullpo_retv(bl);

	numdb_erase(id_db,bl->id);
	if (bl->type == BL_PC)
		numdb_erase(pc_db,bl->id);
}

/*==========================================
 * nick_db‚Ösd‚ğ’Ç‰Á
 *------------------------------------------
 */
void map_addnickdb(struct map_session_data *sd) {
	nullpo_retv(sd);

	strdb_insert(nick_db,sd->status.name,sd);
}

/*==========================================
 * PC‚Ìquit?— map.c?•ª
 *
 * quit?—‚Ìå?‚ªˆá‚¤‚æ‚¤‚È?‚à‚µ‚Ä‚«‚½
 *------------------------------------------
 */
int map_quit(struct map_session_data *sd) {
	nullpo_retr(0, sd);

	if(!sd->state.waitingdisconnect) {
		if (sd->state.event_disconnect) {
			if (script_config.event_script_type == 0) {
				struct npc_data *npc;
				if ((npc = npc_name2id(script_config.logout_event_name))) {
					run_script(npc->u.scr.script,0,sd->bl.id,npc->bl.id); // PCLogoutNPC
					ShowStatus("Event '"CL_WHITE"%s"CL_RESET"' executed.\n", script_config.logout_event_name);
				}
			} else {
				ShowStatus("%d '"CL_WHITE"%s"CL_RESET"' events executed.\n",
					npc_event_doall_id(script_config.logout_event_name, sd->bl.id), script_config.logout_event_name);
			}
		}

		if(sd->chatID)	// ƒ`ƒƒƒbƒg‚©‚ço‚é
			chat_leavechat(sd);

		if(sd->trade_partner)	// æˆø‚ğ’†?‚·‚é
			trade_tradecancel(sd);

		if(sd->party_invite>0)	// ƒp?ƒeƒB?—U‚ğ‹‘”Û‚·‚é
			party_reply_invite(sd,sd->party_invite_account,0);

		if(sd->guild_invite>0)	// ƒMƒ‹ƒh?—U‚ğ‹‘”Û‚·‚é
			guild_reply_invite(sd,sd->guild_invite,0);
		if(sd->guild_alliance>0)	// ƒMƒ‹ƒh“¯–¿?—U‚ğ‹‘”Û‚·‚é
			guild_reply_reqalliance(sd,sd->guild_alliance_account,0);

		party_send_logout(sd);	// ƒp?ƒeƒB‚ÌƒƒOƒAƒEƒgƒƒbƒZ?ƒW‘—M

		party_send_dot_remove(sd);//minimap dot fix [Kevin]

		guild_send_memberinfoshort(sd,0);	// ƒMƒ‹ƒh‚ÌƒƒOƒAƒEƒgƒƒbƒZ?ƒW‘—M

		guild_send_dot_remove(sd);
		 
		pc_cleareventtimer(sd);	// ƒCƒxƒ“ƒgƒ^ƒCƒ}‚ğ”jŠü‚·‚é

		if(sd->state.storage_flag == 1)
			storage_storage_quit(sd);	// ‘qŒÉ‚ğŠJ‚¢‚Ä‚é‚È‚ç•Û‘¶‚·‚é
		else if(sd->state.storage_flag == 2)
			storage_guild_storage_quit(sd,0);

		// check if we've been authenticated [celest]
		if (sd->state.auth)
			skill_castcancel(&sd->bl,0);	// ‰r¥‚ğ’†?‚·‚é

		skill_stop_dancing(&sd->bl);// ƒ_ƒ“ƒX/‰‰‘t’†?

		if(sd->sc_data && sd->sc_data[SC_BERSERK].timer!=-1) //ƒo?ƒT?ƒN’†‚ÌI—¹‚ÍHP‚ğ100‚É
			sd->status.hp = 100;

//		status_change_clear(&sd->bl,1);	// ƒXƒe?ƒ^ƒXˆÙí‚ğ‰ğœ‚·‚é
		skill_clear_unitgroup(&sd->bl);	// ƒXƒLƒ‹ƒ†ƒjƒbƒgƒOƒ‹?ƒv‚Ìíœ

		// check if we've been authenticated [celest]
		if (sd->state.auth) {
			skill_cleartimerskill(&sd->bl);
			pc_stop_walking(sd,0);
			pc_stopattack(sd);
			pc_stop_following(sd);
			pc_delinvincibletimer(sd);
		}
		pc_delspiritball(sd,sd->spiritball,1);
		skill_gangsterparadise(sd,0);
		skill_unit_move(&sd->bl,gettick(),4);

		if (sd->state.auth)
			status_calc_pc(sd,4);
	//	skill_clear_unitgroup(&sd->bl);	// [Sara-chan]

		if (!(sd->status.option & OPTION_HIDE))
			clif_clearchar_area(&sd->bl,2);

		chrif_save_scdata(sd); //Save status changes, then clear'em out from memory. [Skotlex]
		status_change_clear(&sd->bl,1);
		
		if(sd->status.pet_id && sd->pd) {
			pet_lootitem_drop(sd->pd,sd);
			pet_remove_map(sd);
			if(sd->pet.intimate <= 0) {
				intif_delete_petdata(sd->status.pet_id);
				sd->status.pet_id = 0;
				sd->pd = NULL;
				sd->petDB = NULL;
			}
			else
				intif_save_petdata(sd->status.account_id,&sd->pet);
		}

		if(pc_isdead(sd))
			pc_setrestartvalue(sd,2);

		pc_clean_skilltree(sd);

		//The storage closing routines will save the char if needed. [Skotlex]
		if (!sd->state.storage_flag)
			chrif_save(sd);
		else if (sd->state.storage_flag == 1)
			storage_storageclose(sd);
		else if (sd->state.storage_flag == 2)
			storage_guild_storageclose(sd);

		map_delblock(&sd->bl);
	}

	if (sd->stack) {
		script_free_stack(sd->stack);
		sd->stack= NULL;
	}
	
	chrif_char_offline(sd);

	{
		void *p = numdb_search(charid_db,sd->status.char_id);
		if(p) {
			numdb_erase(charid_db,sd->status.char_id);
			aFree(p);
		}
	}
	strdb_erase(nick_db,sd->status.name);
	numdb_erase(charid_db,sd->status.char_id);
	numdb_erase(id_db,sd->bl.id);
	numdb_erase(pc_db,sd->bl.id);

	// Notify friends that this char logged out. [Skotlex]
	clif_foreachclient(clif_friendslist_toggle_sub, sd->status.account_id, sd->status.char_id, 0);
	
	if(sd->reg)
	{	//Double logout already freed pointer fix... [Skotlex]
		aFree(sd->reg);
		sd->reg = NULL;
		sd->reg_num = 0;
	}

	if(sd->regstr)
	{
		aFree(sd->regstr);
		sd->regstr = NULL;
		sd->regstr_num = 0;
	}

	if(!sd->fd) //There is no session connected, and as such socket.c won't free the data, we must do it. [Skotlex]
		aFree(sd);
	return 0;
}

/*==========================================
 * id”Ô?‚ÌPC‚ğ’T‚·B‹‚È‚¯‚ê‚ÎNULL
 *------------------------------------------
 */
struct map_session_data * map_id2sd(int id) {
// Now using pc_db to handle all players, should be quicker than both previous methods at a small expense of more memory. [Skotlex]
	if (id <= 0) return NULL;
	return (struct map_session_data*)numdb_search(pc_db,id);
}

/*==========================================
 * char_id”Ô?‚Ì–¼‘O‚ğ’T‚·
 *------------------------------------------
 */
char * map_charid2nick(int id) {
	struct charid2nick *p = (struct charid2nick*)numdb_search(charid_db,id);

	if(p==NULL)
		return NULL;
	if(p->req_id!=0)
		return NULL;
	return p->nick;
}

struct map_session_data * map_charid2sd(int id) {
	int i, users;
	struct map_session_data **all_sd;

	if (id <= 0) return 0;

	all_sd = map_getallusers(&users);
	for(i = 0; i < users; i++)
		if (all_sd[i] && all_sd[i]->status.char_id == id)
			return all_sd[i];

	return NULL;
}

/*==========================================
 * Search session data from a nick name
 * (without sensitive case if necessary)
 * return map_session_data pointer or NULL
 *------------------------------------------
 */
struct map_session_data * map_nick2sd(char *nick) {
	int i, quantity=0, nicklen, users;
	struct map_session_data *sd = NULL;
	struct map_session_data *pl_sd = NULL, **pl_allsd;

	if (nick == NULL)
		return NULL;

    nicklen = strlen(nick);

	 pl_allsd = map_getallusers(&users);
	 
	for (i = 0; i < users; i++) {
		pl_sd = pl_allsd[i];
		// Without case sensitive check (increase the number of similar character names found)
		if (strnicmp(pl_sd->status.name, nick, nicklen) == 0) {
			// Strict comparison (if found, we finish the function immediatly with correct value)
			if (strcmp(pl_sd->status.name, nick) == 0)
				return pl_sd;
			quantity++;
			sd = pl_sd;
		}
	}
	// Here, the exact character name is not found
	// We return the found index of a similar account ONLY if there is 1 similar character
	if (quantity == 1)
		return sd;

	// Exact character name is not found and 0 or more than 1 similar characters have been found ==> we say not found
	return NULL;
}

/*==========================================
 * id”Ô?‚Ì•¨‚ğ’T‚·
 * ˆêObject‚Ìê‡‚Í”z—ñ‚ğˆø‚­‚Ì‚İ
 *------------------------------------------
 */
struct block_list * map_id2bl(int id)
{
	struct block_list *bl=NULL;
	if(id >= 0 && id < sizeof(objects)/sizeof(objects[0]))
		bl = objects[id];
	else
		bl = (struct block_list*)numdb_search(id_db,id);

	return bl;
}

static int map_foreachpc_sub(void * key,void * data,va_list ap)
{
	struct map_session_data *sd = (struct map_session_data*) data;
	struct map_session_data ***total_sd = va_arg(ap, struct map_session_data***);
	int *count = va_arg(ap, int*);
	
	if (!sd->state.auth)
		return 0; //Do not count in not-yet authenticated characters.

	(*total_sd)[(*count)++] = sd;
	return 0;
}

/*==========================================
 * Returns an array of all players in the server (includes non connected ones) [Skotlex]
 * The int pointer given returns the count of elements in the array.
 * If null is passed, it is requested that the memory be freed (for shutdown), and null is returned.
 *------------------------------------------
 */
struct map_session_data** map_getallusers(int *users) {
	static struct map_session_data **all_sd=NULL;
	static int all_count = 0;
	
	if (users == NULL)
	{	//Free up data
		if (all_sd) aFree(all_sd);
		all_sd = NULL;
		return NULL;
	}

	if (all_sd == NULL)
	{	//Init data
		all_count = pc_db->item_count; //This is the real number of chars in the db, better use this than the actual "online" count.
		if (all_count < 1)
			all_count = 10; //Allow room for at least 10 chars.
		all_sd = aCalloc(all_count, sizeof(struct map_session_data*)); //it's actually just the size of a pointer.
	}

	if (all_count < pc_db->item_count)
	{
		all_count = pc_db->item_count;
		all_sd = aRealloc(all_sd, all_count*sizeof(struct map_session_data*));
	}
	*users = 0;
	numdb_foreach(pc_db,map_foreachpc_sub,&all_sd, users);
	return all_sd;
}

/*==========================================
 * id_db?‚Ì‘S‚Ä‚Éfunc‚ğ?s
 *------------------------------------------
 */
int map_foreachiddb(int (*func)(void*,void*,va_list),...) {
	va_list ap;

	va_start(ap,func);
	numdb_foreach(id_db,func,ap);
	va_end(ap);
	return 0;
}

/*==========================================
 * map.npc‚Ö’Ç‰Á (warp“™‚Ì—Ìˆæ‚¿‚Ì‚İ)
 *------------------------------------------
 */
int map_addnpc(int m,struct npc_data *nd) {
	int i;
	if(m<0 || m>=map_num)
		return -1;
	for(i=0;i<map[m].npc_num && i<MAX_NPC_PER_MAP;i++)
		if(map[m].npc[i]==NULL)
			break;
	if(i==MAX_NPC_PER_MAP){
		if(battle_config.error_log)
			ShowWarning("too many NPCs in one map %s\n",map[m].name);
		return -1;
	}
	if(i==map[m].npc_num){
		map[m].npc_num++;
	}

	nullpo_retr(0, nd);

	map[m].npc[i]=nd;
	nd->n = i;
	numdb_insert(id_db,nd->bl.id,nd);

	return i;
}

void map_removenpc(void) {
	int i,m,n=0;

	for(m=0;m<map_num;m++) {
		for(i=0;i<map[m].npc_num && i<MAX_NPC_PER_MAP;i++) {
			if(map[m].npc[i]!=NULL) {
				clif_clearchar_area(&map[m].npc[i]->bl,2);
				map_delblock(&map[m].npc[i]->bl);
				numdb_erase(id_db,map[m].npc[i]->bl.id);
				if(map[m].npc[i]->bl.subtype==SCRIPT) {
					aFree(map[m].npc[i]->u.scr.script);
					aFree(map[m].npc[i]->u.scr.label_list);
				}
				aFree(map[m].npc[i]);
				map[m].npc[i] = NULL;
				n++;
			}
		}
	}
	
	ShowStatus("Successfully removed and freed from memory '"CL_WHITE"%d"CL_RESET"' NPCs.\n",n);
}

/*=========================================
 * Dynamic Mobs [Wizputer]
 *-----------------------------------------
 */

// allocates a struct when it there is place free in the cache,
// and returns NULL otherwise
// -- i'll just leave the old code in case it's needed ^^;
struct mob_list* map_addmobtolist(unsigned short m)
{
	size_t i;
	for (i = 0; i < MAX_MOB_LIST_PER_MAP; i++) {
		if (map[m].moblist[i] == NULL) {
			map[m].moblist[i] = (struct mob_list *) aMalloc (sizeof(struct mob_list));
			return map[m].moblist[i];
		}
	}
	return NULL;
}

/*void map_addmobtolist(struct mob_list *mob)
{
	int i;

	for (i = 0; i < MAX_MOB_LIST_PER_MAP; i++)
		if (map[mob->m].moblist[i] == NULL)
			break;
	// moblist is full, so don't add it?
	if (i == MAX_MOB_LIST_PER_MAP) {
		aFree(mob);
		return;
	}

	map[mob->m].moblist[i] = mob;
}*/

void map_spawnmobs(int m)
{
	int i, k=0;
	if (map[m].mob_delete_timer != -1)
	{	//Mobs have not been removed yet [Skotlex]
		delete_timer(map[m].mob_delete_timer, map_removemobs_timer);
		map[m].mob_delete_timer = -1;
		return;
	}
	for(i=0; i<MAX_MOB_LIST_PER_MAP; i++)
		if(map[m].moblist[i]!=NULL)
		{
			k+=map[m].moblist[i]->num;
			npc_parse_mob2(map[m].moblist[i],1);
		}

	if (battle_config.etc_log && k > 0)
	{
		ShowStatus("Map %s: Spawned '"CL_WHITE"%d"CL_RESET"' mobs.\n",map[m].name, k);
	}
}

int mob_cache_cleanup_sub(struct block_list *bl, va_list ap) {
	struct mob_data *md = (struct mob_data *)bl;
	nullpo_retr(0, md);

	//When not to remove:
	//Mob has the cached flag on 0
	if (!md->cached)
		return 0;

	mob_remove_map(md, 0);
	map_deliddb(&md->bl);
	aFree(md);
	md = NULL;

	return 1;
}

int map_removemobs_timer(int tid, unsigned int tick, int id, int data)
{
	int k;
	if (id < 0 || id >= MAX_MAP_PER_SERVER)
	{	//Incorrect map id!
		if (battle_config.error_log)
			ShowError("map_removemobs_timer error: timer %d points to invalid map %d\n",tid, id);
		return 0;
	}
	if (map[id].mob_delete_timer != tid)
	{	//Incorrect timer call!
		if (battle_config.error_log)
			ShowError("map_removemobs_timer mismatch: %d != %d (map %s)\n",map[id].mob_delete_timer, tid, map[id].name);
		return 0;
	}
	map[id].mob_delete_timer = -1;
	if (map[id].users > 0) //Map not empty!
		return 1;
	k = map_foreachinarea(mob_cache_cleanup_sub, id, 0, 0, map[id].xs, map[id].ys, BL_MOB);

	if (battle_config.etc_log && k > 0)
		ShowStatus("Map %s: Removed '"CL_WHITE"%d"CL_RESET"' mobs.\n",map[id].name, k);
	
	return 1;
}

void map_removemobs(int m)
{
	if (map[m].mob_delete_timer != -1)
		return; //Mobs are already scheduled for removal

	map[m].mob_delete_timer = add_timer(gettick()+battle_config.mob_remove_delay, map_removemobs_timer, m, 0);
}

/*==========================================
 * map–¼‚©‚çmap”Ô?‚Ö?Š·
 *------------------------------------------
 */
int map_mapname2mapid(char *name) {
	struct map_data *md=NULL;

	md = (struct map_data*)strdb_search(map_db,name);

#ifdef USE_AFM
	// If we can't find the .gat map try .afm instead [celest]
	if(md==NULL && strstr(name,".gat")) {
	  char *afm_name = aStrdup(name);
	  strcpy(&afm_name[strlen(name) - 3], "afm");
	  md = (struct map_data*)strdb_search(map_db,afm_name);
	  aFree(afm_name);
	}
#endif

	if(md==NULL || md->gat==NULL)
		return -1;
	return md->m;
}

/*==========================================
 * ‘¼Imap–¼‚©‚çip,port?Š·
 *------------------------------------------
 */
int map_mapname2ipport(char *name,int *ip,int *port) {
	struct map_data_other_server *mdos=NULL;

	mdos = (struct map_data_other_server*)strdb_search(map_db,name);
	if(mdos==NULL || mdos->gat)
		return -1;
	*ip=mdos->ip;
	*port=mdos->port;
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int map_check_dir(int s_dir,int t_dir) {
	if(s_dir == t_dir)
		return 0;
	switch(s_dir) {
		case 0:
			if(t_dir == 7 || t_dir == 1 || t_dir == 0)
				return 0;
			break;
		case 1:
			if(t_dir == 0 || t_dir == 2 || t_dir == 1)
				return 0;
			break;
		case 2:
			if(t_dir == 1 || t_dir == 3 || t_dir == 2)
				return 0;
			break;
		case 3:
			if(t_dir == 2 || t_dir == 4 || t_dir == 3)
				return 0;
			break;
		case 4:
			if(t_dir == 3 || t_dir == 5 || t_dir == 4)
				return 0;
			break;
		case 5:
			if(t_dir == 4 || t_dir == 6 || t_dir == 5)
				return 0;
			break;
		case 6:
			if(t_dir == 5 || t_dir == 7 || t_dir == 6)
				return 0;
			break;
		case 7:
			if(t_dir == 6 || t_dir == 0 || t_dir == 7)
				return 0;
			break;
	}
	return 1;
}

/*==========================================
 * ”Ş‰ä‚Ì•ûŒü‚ğŒvZ
 *------------------------------------------
 */
int map_calc_dir( struct block_list *src,int x,int y) {
	int dir=0;
	int dx,dy;

	nullpo_retr(0, src);

	dx=x-src->x;
	dy=y-src->y;
	if( dx==0 && dy==0 ){	// ”Ş‰ä‚ÌêŠˆê’v
		dir=0;	// ã
	}else if( dx>=0 && dy>=0 ){	// •ûŒü“I‚É‰Eã
		dir=7;						// ‰Eã
		if( dx*2-1<dy ) dir=0;		// ã
		if( dx>dy*2 ) dir=6;		// ‰E
	}else if( dx>=0 && dy<=0 ){	// •ûŒü“I‚É‰E‰º
		dir=5;						// ‰E‰º
		if( dx*2-1<-dy ) dir=4;		// ‰º
		if( dx>-dy*2 ) dir=6;		// ‰E
	}else if( dx<=0 && dy<=0 ){ // •ûŒü“I‚É¶‰º
		dir=3;						// ¶‰º
		if( dx*2+1>dy ) dir=4;		// ‰º
		if( dx<dy*2 ) dir=2;		// ¶
	}else{						// •ûŒü“I‚É¶ã
		dir=1;						// ¶ã
		if( -dx*2-1<dy ) dir=0;		// ã
		if( -dx>dy*2 ) dir=2;		// ¶
	}
	return dir;
}

// gatŒn
/*==========================================
 * (m,x,y)‚Ìó‘Ô‚ğ’²‚×‚é
 *------------------------------------------
 */

int map_getcell(int m,int x,int y,cell_t cellchk)
{
	return (m < 0 || m >= MAX_MAP_PER_SERVER) ? 0 : map_getcellp(&map[m],x,y,cellchk);
}

int map_getcellp(struct map_data* m,int x,int y,cell_t cellchk)
{
	int type;
	nullpo_ret(m);

	if(x<0 || x>=m->xs-1 || y<0 || y>=m->ys-1)
	{
		if(cellchk==CELL_CHKNOPASS) return 1;
		return 0;
	}
	type = m->gat[x+y*m->xs];
	if (cellchk<0x10)
		type &= CELL_MASK;

	switch(cellchk)
	{
		case CELL_CHKPASS:
			return (type!=1 && type!=5);
		case CELL_CHKNOPASS:
			return (type==1 || type==5);
		case CELL_CHKWALL:
			return (type==1);
		case CELL_CHKWATER:
			return (type==3);
		case CELL_CHKGROUND:
			return (type==5);
		case CELL_GETTYPE:
			return type;
		case CELL_CHKNPC:
			return (type&CELL_NPC);
		case CELL_CHKBASILICA:
			return (type&CELL_BASILICA);
		case CELL_CHKREGEN:
			return (type&CELL_REGEN);
		default:
			return 0;
	}
}

/*==========================================
 * (m,x,y)‚Ìó‘Ô‚ğİ’è‚·‚é
 *------------------------------------------
 */
void map_setcell(int m,int x,int y,int cell)
{
	int j;
	if(x<0 || x>=map[m].xs || y<0 || y>=map[m].ys)
		return;
	j=x+y*map[m].xs;

	switch (cell) {
		case CELL_SETNPC:
			map[m].gat[j] |= CELL_NPC;
			break;
		case CELL_SETBASILICA:
			map[m].gat[j] |= CELL_BASILICA;
			break;
		case CELL_CLRBASILICA:
			map[m].gat[j] &= ~CELL_BASILICA;
			break;
		case CELL_SETREGEN:
			map[m].gat[j] |= CELL_REGEN;
			break;
		default:
			map[m].gat[j] = (map[m].gat[j]&~CELL_MASK) + cell;
			break;
	}
}

/*==========================================
 * ‘¼IŠÇ—‚Ìƒ}ƒbƒv‚ğdb‚É’Ç‰Á
 *------------------------------------------
 */
int map_setipport(char *name,unsigned long ip,int port) {
	struct map_data *md=NULL;
	struct map_data_other_server *mdos=NULL;

	md = (struct map_data*)strdb_search(map_db,name);
	if(md==NULL){ // not exist -> add new data
		mdos=(struct map_data_other_server *)aCalloc(1,sizeof(struct map_data_other_server));
		memcpy(mdos->name, name, NAME_LENGTH-1);
//		mdos->gat  = NULL;
		mdos->ip   = ip;
		mdos->port = port;
//		mdos->map  = NULL;
		strdb_insert(map_db,mdos->name,mdos);
	} else if(md->gat){
		if(ip!=clif_getip() || port!=clif_getport()){
			// “Ç‚İbñ‚Å‚¢‚½‚¯‚ÇA’S“–ŠO‚É‚È‚Á‚½ƒ}ƒbƒv
			mdos=(struct map_data_other_server *)aCalloc(1,sizeof(struct map_data_other_server));
			memcpy(mdos->name, name, NAME_LENGTH-1);
//			mdos->gat  = NULL;
			mdos->ip   = ip;
			mdos->port = port;
			mdos->map  = md;
			strdb_insert(map_db,mdos->name,mdos);
			// printf("from char server : %s -> %08lx:%d\n",name,ip,port);
		} else {
			// “Ç‚İbñ‚Å‚¢‚ÄA’S“–‚É‚È‚Á‚½ƒ}ƒbƒvi‰½‚à‚µ‚È‚¢j
			;
		}
	} else {
		mdos=(struct map_data_other_server *)md;
		if(ip == clif_getip() && port == clif_getport()) {
			// ©•ª‚Ì’S“–‚É‚È‚Á‚½ƒ}ƒbƒv
			if(mdos->map == NULL) {
				// “Ç‚İbñ‚Å‚¢‚È‚¢‚Ì‚ÅI—¹‚·‚é
				ShowFatalError("map_setipport : %s is not loaded.\n",name);
				exit(1);
			} else {
				// “Ç‚İbñ‚Å‚¢‚é‚Ì‚Å’u‚«Š·‚¦‚é
				md = mdos->map;
				aFree(mdos);
				strdb_insert(map_db,md->name,md);
			}
		} else {
			// ‘¼‚ÌI‚Ì’S“–ƒ}ƒbƒv‚È‚Ì‚Å’u‚«Š·‚¦‚é‚¾‚¯
			mdos->ip   = ip;
			mdos->port = port;
		}
	}
	return 0;
}

/*==========================================
 * ‘¼IŠÇ—‚Ìƒ}ƒbƒv‚ğ‘S‚Äíœ
 *------------------------------------------
 */
int map_eraseallipport_sub(void *key,void *data,va_list va) {
	struct map_data_other_server *mdos = (struct map_data_other_server*)data;
	if(mdos->gat == NULL && mdos->map == NULL) {
		strdb_erase(map_db,key);
		aFree(mdos);
	}
	return 0;
}

int map_eraseallipport(void) {
	strdb_foreach(map_db,map_eraseallipport_sub);
	return 1;
}

/*==========================================
 * ‘¼IŠÇ—‚Ìƒ}ƒbƒv‚ğdb‚©‚çíœ
 *------------------------------------------
 */
int map_eraseipport(char *name,unsigned long ip,int port)
{
	struct map_data *md;
	struct map_data_other_server *mdos;
//	unsigned char *p=(unsigned char *)&ip;

	md=(struct map_data *) strdb_search(map_db,name);
	if(md){
		if(md->gat) // local -> check data
			return 0;
		else {
			mdos=(struct map_data_other_server *)md;
			if(mdos->ip==ip && mdos->port == port) {
				if(mdos->map) {
					// ‚±‚Ìƒ}ƒbƒvI‚Å‚à“Ç‚İbñ‚Å‚¢‚é‚Ì‚ÅˆÚ“®‚Å‚«‚é
					return 1; // ŒÄ‚Ño‚µŒ³‚Å chrif_sendmap() ‚ğ‚·‚é
				} else {
					strdb_erase(map_db,name);
					aFree(mdos);
				}
//				if(battle_config.etc_log)
//					printf("erase map %s %d.%d.%d.%d:%d\n",name,p[0],p[1],p[2],p[3],port);
			}
		}
	}
	return 0;
}

// ‰Šú‰»ü‚è
/*==========================================
 * …ê‚‚³İ’è
 *------------------------------------------
 */
static struct waterlist_ {
	char mapname[MAP_NAME_LENGTH];
	int waterheight;
} *waterlist=NULL;

#define NO_WATER 1000000

static int map_waterheight(char *mapname) {
	if(waterlist){
		int i;
		for(i=0;waterlist[i].mapname[0] && i < MAX_MAP_PER_SERVER;i++)
			if(strcmp(waterlist[i].mapname,mapname)==0)
				return waterlist[i].waterheight;
	}
	return NO_WATER;
}

static void map_readwater(char *watertxt) {
	char line[1024],w1[1024];
	FILE *fp=NULL;
	int n=0;

	fp=fopen(watertxt,"r");
	if(fp==NULL){
		ShowError("file not found: %s\n",watertxt);
		return;
	}
	if(waterlist==NULL)
		waterlist = (struct waterlist_*)aCallocA(MAX_MAP_PER_SERVER,sizeof(*waterlist));
	while(fgets(line,1020,fp) && n < MAX_MAP_PER_SERVER){
		int wh,count;
		if(line[0] == '/' && line[1] == '/')
			continue;
		if((count=sscanf(line,"%s%d",w1,&wh)) < 1){
			continue;
		}
		memcpy(waterlist[n].mapname,w1, MAP_NAME_LENGTH-1);
		if(count >= 2)
			waterlist[n].waterheight = wh;
		else
			waterlist[n].waterheight = 3;
		n++;
	}
	fclose(fp);
}
/*==========================================
* ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚É’Ç‰Á‚·‚é
*===========================================*/

// ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚ÌÅ‘å’l
#define MAX_MAP_CACHE 768

//Šeƒ}ƒbƒv‚²‚Æ‚ÌÅ¬ŒÀî•ñ‚ğ“ü‚ê‚é‚à‚ÌAREAD_FROM_BITMAP—p
struct map_cache_info {
	char fn[32];//ƒtƒ@ƒCƒ‹–¼
	int xs,ys; //•‚Æ‚‚³
	int water_height;
	int pos;  // ƒf[ƒ^‚ª“ü‚ê‚Ä‚ ‚éêŠ
	int compressed;     // zilb’Ê‚¹‚é‚æ‚¤‚É‚·‚éˆ×‚Ì—\–ñ
	int compressed_len; // zilb’Ê‚¹‚é‚æ‚¤‚É‚·‚éˆ×‚Ì—\–ñ
}; // 56 byte

struct map_cache_head {
	int sizeof_header;
	int sizeof_map;
	// ã‚Ì‚Q‚Â‰ü•Ï•s‰Â
	int nmaps; // ƒ}ƒbƒv‚ÌŒÂ”
	int filesize;
};

struct {
	struct map_cache_head head;
	struct map_cache_info *map;
	FILE *fp;
	int dirty;
} map_cache;

static int map_cache_open(char *fn);
static void map_cache_close(void);
static int map_cache_read(struct map_data *m);
static int map_cache_write(struct map_data *m);

static int map_cache_open(char *fn)
{
	if(map_cache.fp) {
		map_cache_close();
	}
	map_cache.fp = fopen(fn,"r+b");
	if(map_cache.fp) {
		fread(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fseek(map_cache.fp,0,SEEK_END);
		if(
			map_cache.head.sizeof_header == sizeof(struct map_cache_head) &&
			map_cache.head.sizeof_map    == sizeof(struct map_cache_info) &&
			map_cache.head.filesize      == ftell(map_cache.fp)
		) {
			// ƒLƒƒƒbƒVƒ…“Ç‚İbİ¬Œ÷
			map_cache.map = (struct map_cache_info *) aMalloc(sizeof(struct map_cache_info) * map_cache.head.nmaps);
			fseek(map_cache.fp,sizeof(struct map_cache_head),SEEK_SET);
			fread(map_cache.map,sizeof(struct map_cache_info),map_cache.head.nmaps,map_cache.fp);
			return 1;
		}
		fclose(map_cache.fp);
	}
	// “Ç‚İbİ‚É¸”s‚µ‚½‚Ì‚ÅV‹K‚Éì¬‚·‚é
	map_cache.fp = fopen(fn,"wb");
	if(map_cache.fp) {
		memset(&map_cache.head,0,sizeof(struct map_cache_head));
		map_cache.map   = (struct map_cache_info *) aCalloc(sizeof(struct map_cache_info),MAX_MAP_CACHE);
		map_cache.head.nmaps         = MAX_MAP_CACHE;
		map_cache.head.sizeof_header = sizeof(struct map_cache_head);
		map_cache.head.sizeof_map    = sizeof(struct map_cache_info);

		map_cache.head.filesize  = sizeof(struct map_cache_head);
		map_cache.head.filesize += sizeof(struct map_cache_info) * map_cache.head.nmaps;

		map_cache.dirty = 1;
		return 1;
	}
	return 0;
}

static void map_cache_close(void)
{
	if(!map_cache.fp) { return; }
	if(map_cache.dirty) {
		fseek(map_cache.fp,0,SEEK_SET);
		fwrite(&map_cache.head,1,sizeof(struct map_cache_head),map_cache.fp);
		fwrite(map_cache.map,map_cache.head.nmaps,sizeof(struct map_cache_info),map_cache.fp);
	}
	fclose(map_cache.fp);
	aFree(map_cache.map);
	map_cache.fp = NULL;
	return;
}

int map_cache_read(struct map_data *m)
{
	int i;
	if(!map_cache.fp) { return 0; }
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(!strcmp(m->name,map_cache.map[i].fn)) {
			if(map_cache.map[i].water_height != map_waterheight(m->name)) {
				// …ê‚Ì‚‚³‚ªˆá‚¤‚Ì‚Å“Ç‚İ’¼‚µ
				return 0;
			} else if(map_cache.map[i].compressed == 0) {
				// ”ñˆ³kƒtƒ@ƒCƒ‹
				int size = map_cache.map[i].xs * map_cache.map[i].ys;
				m->xs = map_cache.map[i].xs;
				m->ys = map_cache.map[i].ys;
				m->gat = (unsigned char *)aCalloc(m->xs * m->ys,sizeof(unsigned char));
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				if(fread(m->gat,1,size,map_cache.fp) == size) {
					// ¬Œ÷
					return 1;
				} else {
					// ‚È‚º‚©ƒtƒ@ƒCƒ‹Œã”¼‚ªŒ‡‚¯‚Ä‚é‚Ì‚Å“Ç‚İ’¼‚µ
					m->xs = 0; m->ys = 0; aFree(m->gat); m->gat = NULL;
					return 0;
				}
			} else if(map_cache.map[i].compressed == 1) {
				// ˆ³kƒtƒ‰ƒO=1 : zlib
				unsigned char *buf;
				unsigned long dest_len;
				int size_compress = map_cache.map[i].compressed_len;
				m->xs = map_cache.map[i].xs;
				m->ys = map_cache.map[i].ys;
				m->gat = (unsigned char *)aMalloc(m->xs * m->ys * sizeof(unsigned char));
				buf = (unsigned char*)aMalloc(size_compress);
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				if(fread(buf,1,size_compress,map_cache.fp) != size_compress) {
					// ‚È‚º‚©ƒtƒ@ƒCƒ‹Œã”¼‚ªŒ‡‚¯‚Ä‚é‚Ì‚Å“Ç‚İ’¼‚µ
					ShowError("fread error\n");
					aFree(m->gat); m->xs = 0; m->ys = 0; m->gat = NULL;
					aFree(buf);
					return 0;
				}
				dest_len = m->xs * m->ys;
				decode_zip(m->gat,&dest_len,buf,size_compress);
				if(dest_len != map_cache.map[i].xs * map_cache.map[i].ys) {
					// ³í‚É‰ğ“€‚ªo—ˆ‚Ä‚È‚¢
					aFree(m->gat); m->xs = 0; m->ys = 0; m->gat = NULL;
					aFree(buf);
					return 0;
				}
				aFree(buf);
				return 1;
			}
		}
	}
	return 0;
}

static int map_cache_write(struct map_data *m)
{
	int i;
	unsigned long len_new , len_old;
	char *write_buf;
	if(!map_cache.fp) { return 0; }
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(!strcmp(m->name,map_cache.map[i].fn)) {
			// “¯‚¶ƒGƒ“ƒgƒŠ[‚ª‚ ‚ê‚Îã‘‚«
			if(map_cache.map[i].compressed == 0) {
				len_old = map_cache.map[i].xs * map_cache.map[i].ys;
			} else if(map_cache.map[i].compressed == 1) {
				len_old = map_cache.map[i].compressed_len;
			} else {
				// ƒTƒ|[ƒg‚³‚ê‚Ä‚È‚¢Œ`®‚È‚Ì‚Å’·‚³‚O
				len_old = 0;
			}
			if(map_read_flag == 2) {
				// ˆ³k•Û‘¶
				// ‚³‚·‚ª‚É‚Q”{‚É–c‚ê‚é–‚Í‚È‚¢‚Æ‚¢‚¤–‚Å
				write_buf = (char *) aMalloc(m->xs * m->ys * 2);
				len_new = m->xs * m->ys * 2;
				encode_zip((unsigned char *) write_buf,&len_new,m->gat,m->xs * m->ys);
				map_cache.map[i].compressed     = 1;
				map_cache.map[i].compressed_len = len_new;
			} else {
				len_new = m->xs * m->ys;
				write_buf = (char *) m->gat;
				map_cache.map[i].compressed     = 0;
				map_cache.map[i].compressed_len = 0;
			}
			if(len_new <= len_old) {
				// ƒTƒCƒY‚ª“¯‚¶‚©¬‚³‚­‚È‚Á‚½‚Ì‚ÅêŠ‚Í•Ï‚í‚ç‚È‚¢
				fseek(map_cache.fp,map_cache.map[i].pos,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
			} else {
				// V‚µ‚¢êŠ‚É“o˜^
				fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
				fwrite(write_buf,1,len_new,map_cache.fp);
				map_cache.map[i].pos = map_cache.head.filesize;
				map_cache.head.filesize += len_new;
			}
			map_cache.map[i].xs  = m->xs;
			map_cache.map[i].ys  = m->ys;
			map_cache.map[i].water_height = map_waterheight(m->name);
			map_cache.dirty = 1;
			if(map_read_flag == 2) {
				aFree(write_buf);
			}
			return 0;
		}
	}
	// “¯‚¶ƒGƒ“ƒgƒŠ‚ª–³‚¯‚ê‚Î‘‚«bß‚éêŠ‚ğ’T‚·
	for(i = 0;i < map_cache.head.nmaps ; i++) {
		if(map_cache.map[i].fn[0] == 0) {
			// V‚µ‚¢êŠ‚É“o˜^
			if(map_read_flag == 2) {
				write_buf = (char *) aMalloc(m->xs * m->ys * 2);
				len_new = m->xs * m->ys * 2;
				encode_zip((unsigned char *) write_buf,&len_new,m->gat,m->xs * m->ys);
				map_cache.map[i].compressed     = 1;
				map_cache.map[i].compressed_len = len_new;
			} else {
				len_new = m->xs * m->ys;
				write_buf = (char *) m->gat;
				map_cache.map[i].compressed     = 0;
				map_cache.map[i].compressed_len = 0;
			}
			strncpy(map_cache.map[i].fn,m->name,sizeof(map_cache.map[0].fn));
			fseek(map_cache.fp,map_cache.head.filesize,SEEK_SET);
			fwrite(write_buf,1,len_new,map_cache.fp);
			map_cache.map[i].pos = map_cache.head.filesize;
			map_cache.map[i].xs  = m->xs;
			map_cache.map[i].ys  = m->ys;
			map_cache.map[i].water_height = map_waterheight(m->name);
			map_cache.head.filesize += len_new;
			map_cache.dirty = 1;
			if(map_read_flag == 2) {
				aFree(write_buf);
			}
			return 0;
		}
	}
	// ‘‚«bß‚È‚©‚Á‚½
	return 1;
}

#ifdef USE_AFM
static int map_readafm(int m,char *fn) {

	/*
	Advanced Fusion Maps Support
	(c) 2003-2004, The Fusion Project
	- AlexKreuz

	The following code has been provided by me for eAthena
	under the GNU GPL.  It provides Advanced Fusion
	Map, the map format desgined by me for Fusion, support
	for the eAthena emulator.

	I understand that because it is under the GPL
	that other emulators may very well use this code in their
	GNU project as well.

	The AFM map format was not originally a part of the GNU
	GPL. It originated from scratch by my own hand.  I understand
	that distributing this code to read the AFM maps with eAthena
	causes the GPL to apply to this code.  But the actual AFM
	maps are STILL copyrighted to the Fusion Project.  By choosing

	In exchange for that 'act of faith' I ask for the following.

	A) Give credit where it is due.  If you use this code, do not
	   place your name on the changelog.  Credit should be given
	   to AlexKreuz.
	B) As an act of courtesy, ask me and let me know that you are putting
	   AFM support in your project.  You will have my blessings if you do.
	C) Use the code in its entirety INCLUDING the copyright message.
	   Although the code provided may now be GPL, the AFM maps are not
	   and so I ask you to display the copyright message on the STARTUP
	   SCREEN as I have done here. (refer to core.c)
	   "Advanced Fusion Maps (c) 2003-2004 The Fusion Project"

	Without this copyright, you are NOT entitled to bundle or distribute
	the AFM maps at all.  On top of that, your "support" for AFM maps
	becomes just as shady as your "support" for Gravity GRF files.

	The bottom line is this.  I know that there are those of you who
	would like to use this code but aren't going to want to provide the
	proper credit.  I know this because I speak frome experience.  If
	you are one of those people who is going to try to get around my
	requests, then save your breath because I don't want to hear it.

	I have zero faith in GPL and I know and accept that if you choose to
	not display the copyright for the AFMs then there is absolutely nothing
	I can do about it.  I am not about to start a legal battle over something
	this silly.

	Provide the proper credit because you believe in the GPL.  If you choose
	not to and would rather argue about it, consider the GPL failed.

	October 18th, 2004
	- AlexKreuz
	- The Fusion Project
	*/


	int s;
	int x,y,xs,ys;
	size_t size;

	char afm_line[65535];
	int afm_size[2];
	FILE *afm_file;
	char *str;

	afm_file = fopen(fn, "r");
	if (afm_file != NULL) {

//		printf("\rLoading Maps [%d/%d]: %-50s  ",m,map_num,fn);
//		fflush(stdout);

		str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
		str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
		str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
		sscanf(str , "%d%d", &afm_size[0], &afm_size[1]);

		map[m].m = m;
		xs = map[m].xs = afm_size[0];
		ys = map[m].ys = afm_size[1];
		// check this, unsigned where it might not need to be
		map[m].gat = (unsigned char*)aCallocA(s = map[m].xs * map[m].ys, 1);

		if(map[m].gat==NULL){
			ShowFatalError("out of memory : map_readmap gat\n");
			exit(1);
		}

		map[m].npc_num=0;
		map[m].users=0;
		memset(&map[m].flag,0,sizeof(map[m].flag));

		if(battle_config.pk_mode) map[m].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]

		for (y = 0; y < ys; y++) {
			str=fgets(afm_line, sizeof(afm_line)-1, afm_file);
			for (x = 0; x < xs; x++) {
				map[m].gat[x+y*xs] = str[x]-48;
			}
		}

		map[m].bxs=(xs+BLOCK_SIZE-1)/BLOCK_SIZE;
		map[m].bys=(ys+BLOCK_SIZE-1)/BLOCK_SIZE;
		size = map[m].bxs * map[m].bys * sizeof(struct block_list*);
		map[m].block = (struct block_list**)aCalloc(size, 1);

		if(map[m].block == NULL){
			ShowFatalError("out of memory : map_readmap block\n");
			exit(1);
		}

		map[m].block_mob = (struct block_list**)aCalloc(size, 1);
		if (map[m].block_mob == NULL) {
			ShowFatalError("out of memory : map_readmap block_mob\n");
			exit(1);
		}

		size = map[m].bxs*map[m].bys*sizeof(int);

		map[m].block_count = (int*)aCallocA(size, 1);
		if(map[m].block_count==NULL){
			ShowFatalError("out of memory : map_readmap block\n");
			exit(1);
		}
		memset(map[m].block_count,0,size);

		map[m].block_mob_count = (int*)aCallocA(size, 1);
		if(map[m].block_mob_count==NULL){
			ShowFatalError("out of memory : map_readmap block_mob\n");
			exit(1);
		}
		memset(map[m].block_mob_count,0,size);

		strdb_insert(map_db,map[m].name,&map[m]);

		fclose(afm_file);

	}

	return 0;
}

#ifdef USE_AF2
static int map_readaf2(int m, char *fn)
{
	char out_file[256];
	char *p, *out;

	p = out = fn;
	while ((p = strchr(p, '/')) != NULL)
		out = ++p;
	
	strncpy (out_file, out, strlen(out));
	p = strrchr (out_file, '.');
	if (p) *p++ = 0;
	strcat(out_file, ".out");

	if (deflate_file(fn, out_file)) {
		map_readafm(m, out_file);
		unlink (out_file);
		return 1;
	}

	return 0;
}
#endif
#endif

/*==========================================
 * ƒ}ƒbƒv1–‡“Ç‚İbİ
 * ===================================================*/
static int map_readmap(int m,char *fn, char *alias, int *map_cache, int maxmap) {
	char *gat="";
	size_t size;

	int i = 0;
	int e = 0;
	char progress[21] = "                    ";

	//printf("\rLoading Maps [%d/%d]: %-50s  ",m,map_num,fn);
	if (maxmap) { //avoid map-server crashing if there are 0 maps
		char c = '-';
		static int lasti = -1;
		static int last_time = -1;
		i=m*20/maxmap;
		if ((i != lasti) || (last_time != time(0))) {
			lasti = i;
			printf("\r");
			ShowStatus("Progress: ");
			printf("[");
			for (e=0;e<i;e++) progress[e] = '#';
			printf(progress);
			printf("] Working: [");
			last_time = time(0);
			switch(last_time % 4) {
				case 0: c='\\'; break;
				case 1: c='|'; break;
				case 2: c='/'; break;
				case 3: c='-'; break;
			}
			printf("%c]",c);
			fflush(stdout);
		}
	}

	if(map_cache_read(&map[m])) {
		// ƒLƒƒƒbƒVƒ…‚©‚ç“Ç‚İbß‚½
		(*map_cache)++;
	} else {
		int s;
		int wh;
		int x,y,xs,ys;
		struct gat_1cell {float high[4]; int type;} *p=NULL;
		// read & convert fn
		// again, might not need to be unsigned char
		gat = (char*)grfio_read(fn);
		if(gat==NULL) {
			return -1;
			// ‚³‚·‚ª‚Éƒ}ƒbƒv‚ª“Ç‚ß‚È‚¢‚Ì‚Í‚Ü‚¸‚¢‚Ì‚ÅI—¹‚·‚é
			//printf("Can't load map %s\n",fn);
			//exit(1);
		}

		xs=map[m].xs=*(int*)(gat+6);
		ys=map[m].ys=*(int*)(gat+10);
		map[m].gat = (unsigned char *)aCallocA(s = map[m].xs * map[m].ys,sizeof(unsigned char));
		wh=map_waterheight(map[m].name);
		for(y=0;y<ys;y++){
			p=(struct gat_1cell*)(gat+y*xs*20+14);
			for(x=0;x<xs;x++){
				if(wh!=NO_WATER && p->type==0){
					// …ê”»’è
					map[m].gat[x+y*xs]=(p->high[0]>wh || p->high[1]>wh || p->high[2]>wh || p->high[3]>wh) ? 3 : 0;
				} else {
					map[m].gat[x+y*xs]=p->type;
				}
				p++;
			}
		}
		map_cache_write(&map[m]);
		aFree(gat);
	}

	map[m].m=m;
	map[m].npc_num=0;
	map[m].users=0;
	memset(&map[m].flag,0,sizeof(map[m].flag));
	if(battle_config.pk_mode)
		map[m].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]
	map[m].bxs=(map[m].xs+BLOCK_SIZE-1)/BLOCK_SIZE;
	map[m].bys=(map[m].ys+BLOCK_SIZE-1)/BLOCK_SIZE;
	size = map[m].bxs * map[m].bys * sizeof(struct block_list*);
	map[m].block = (struct block_list **)aCalloc(1,size);
	map[m].block_mob = (struct block_list **)aCalloc(1,size);
	size = map[m].bxs*map[m].bys*sizeof(int);
	map[m].block_count = (int *)aCallocA(1,size);
	map[m].block_mob_count=(int *)aCallocA(1,size);
	if (alias)
           strdb_insert(map_db,alias,&map[m]);
        else
           strdb_insert(map_db,map[m].name,&map[m]);

//	printf("%s read done\n",fn);

	return 0;
}

/*==========================================
 * Removes the map in the index passed.
 *------------------------------------------
 */
static void map_delmapid(int id)
{
	ShowNotice("Removing map [ %s ] from maplist\n",map[id].name);
	memmove(map+id, map+id+1, sizeof(map[0])*(map_num-id-1));
	map_num--;
}

/*==========================================
 * ‘S‚Ä‚Ìmapƒf?ƒ^‚ğ?‚İ?‚Ş
 *------------------------------------------
 */
int map_readallmap(void) {
	int i,maps_removed=0;
	char fn[256], *p;
#ifdef USE_AFM
	FILE *afm_file;
#endif
	int map_cache = 0;

	// ƒ}ƒbƒvƒLƒƒƒbƒVƒ…‚ğŠJ‚­
	if(map_read_flag >= READ_FROM_BITMAP) {
		map_cache_open(map_cache_file);
	}

	ShowStatus("Loading Maps%s...\n",
		(map_read_flag == CREATE_BITMAP_COMPRESSED ? " (Generating Map Cache w/ Compression)" :
		map_read_flag == CREATE_BITMAP ? " (Generating Map Cache)" :
		map_read_flag >= READ_FROM_BITMAP ? " (w/ Map Cache)" :
		map_read_flag == READ_FROM_AFM ? " (w/ AFM)" : ""));

	// æ‚É‘S•”‚Ìƒƒbƒv‚Ì‘¶İ‚ğŠm”F
	for (i = 0; i < map_num; i++)
	{
#ifdef USE_AFM
		char afm_name[256] = "";
#endif
		// set it by default first
		map[i].alias = NULL;
		memset(map[i].moblist, 0, sizeof(map[i].moblist));	//Initialize moblist [Skotlex]
		map[i].mob_delete_timer = -1;	//Initialize timer [Skotlex]

#ifdef USE_AFM
		if(!strstr(map[i].name, ".afm")) {
		// check if it's necessary to replace the extension - speeds up loading abit
			strncpy(afm_name, map[i].name, strlen(map[i].name) - 4);
			strcat(afm_name, ".afm");
		}
		sprintf(fn,"%s\\%s", afm_dir, afm_name);
		for (p = &fn[0]; *p != 0; p++)
			if (*p == '\\') *p = '/';	// * At the time of Unix

		afm_file = fopen(fn, "r");
		if (afm_file != NULL) {
			fclose(afm_file);
			map_readafm(i,fn);
			continue;
		}

	#ifdef USE_AF2
		// try with *.af2
		fn[strlen(fn)-1] = '2';
		afm_file = fopen(fn, "r");
		if (afm_file != NULL) {
			fclose(afm_file);
			if (map_readaf2(i,fn) != 0)
				continue;
		}
	#endif
#endif
		if (strstr(map[i].name,".gat") != NULL) {
			p = strstr(map[i].name, "<"); // [MouseJstr]
			if (p != NULL) {
				char buf[64];
				*p++ = '\0';
				sprintf(buf,"data\\%s", p);
				map[i].alias = aStrdup(buf);
			} else
				map[i].alias = NULL;

			sprintf(fn,"data\\%s",map[i].name);
			if(map_readmap(i,fn, p, &map_cache, map_num) != -1)
				continue;
		}
		//If the code reached this point, the map couldn't be loaded.
		map_delmapid(i);
		maps_removed++;
		i--;
	}

	aFree(waterlist);
	printf("\r");
	ShowInfo("Successfully loaded '"CL_WHITE"%d"CL_RESET"' maps.%30s\n",map_num,"");

	map_cache_close();
	if(map_read_flag == CREATE_BITMAP || map_read_flag == CREATE_BITMAP_COMPRESSED) {
		--map_read_flag;
	}

	if (maps_removed) {
		ShowNotice("Maps Removed: '"CL_WHITE"%d"CL_RESET"'\n",maps_removed);
	}
	return 0;
}

/*==========================================
 * ?‚İ?‚Şmap‚ğ’Ç‰Á‚·‚é
 *------------------------------------------
 */
int map_addmap(char *mapname) {
	if (strcmpi(mapname,"clear")==0) {
		map_num=0;
		return 0;
	}

	if (map_num >= MAX_MAP_PER_SERVER - 1) {
		ShowError("Could not add map '"
		CL_WHITE"%s"CL_RESET"', the limit of maps has been reached.\n",mapname);
		return 1;
	}
	memcpy(map[map_num].name, mapname, MAP_NAME_LENGTH-1);
	map_num++;
	return 0;
}

/*==========================================
 * ?‚İ?‚Şmap‚ğíœ‚·‚é
 *------------------------------------------
 */
int map_delmap(char *mapname) {

	int i;

	if (strcmpi(mapname, "all") == 0) {
		map_num = 0;
		return 0;
	}

	for(i = 0; i < map_num; i++) {
		if (strcmp(map[i].name, mapname) == 0) {
			map_delmapid(i);
			return 1;
		}
	}
	return 0;
}

static int map_ip_set_ = 0;
static int char_ip_set_ = 0;
//static int bind_ip_set_ = 0;

/*==========================================
 * Console Command Parser [Wizputer]
 *------------------------------------------
 */
int parse_console(char *buf) {
	char *type,*command,*map, *buf2;
	int x = 0, y = 0;
	int m, n;
	struct map_session_data *sd;

	sd = (struct map_session_data*)aCalloc(sizeof(*sd), 1);

	sd->fd = 0;
	strcpy( sd->status.name , "console");

	type = (char *)aMallocA(64);
	command = (char *)aMallocA(64);
	map = (char *)aMallocA(64);
	buf2 = (char *)aMallocA(72);

	memset(type,0,64);
	memset(command,0,64);
	memset(map,0,64);
	memset(buf2,0,72);

	if ( ( n = sscanf(buf, "%[^:]:%[^:]:%99s %d %d[^\n]", type , command , map , &x , &y )) < 5 )
		if ( ( n = sscanf(buf, "%[^:]:%[^\n]", type , command )) < 2 )
			n = sscanf(buf,"%[^\n]",type);

	if ( n == 5 ) {
		if (x <= 0) {
			x = rand() % 399 + 1;
			sd->bl.x = x;
		} else {
			sd->bl.x = x;
		}

		if (y <= 0) {
			y = rand() % 399 + 1;
			sd->bl.y = y;
		} else {
			sd->bl.y = y;
		}

		m = map_mapname2mapid(map);
		if ( m >= 0 )
			sd->bl.m = m;
		else {
			ShowWarning("Console: Unknown map\n");
			goto end;
		}
	}

	ShowInfo("Type of command: %s || Command: %s || Map: %s Coords: %d %d\n",type,command,map,x,y);

	if ( strcmpi("admin",type) == 0 && n == 5 ) {
		sprintf(buf2,"console: %s",command);
		if( is_atcommand(sd->fd,sd,buf2,99) == AtCommand_None )
			printf("Console: not atcommand\n");
	} else if ( strcmpi("server",type) == 0 && n == 2 ) {
		if ( strcmpi("shutdown", command) == 0 || strcmpi("exit",command) == 0 || strcmpi("quit",command) == 0 ) {
			runflag = 0;
		}
	} else if ( strcmpi("help",type) == 0 ) {
		ShowNotice("To use GM commands:\n");
		printf("admin:<gm command>:<map of \"gm\"> <x> <y>\n");
		printf("You can use any GM command that doesn't require the GM.\n");
		printf("No using @item or @warp however you can use @charwarp\n");
		printf("The <map of \"gm\"> <x> <y> is for commands that need coords of the GM\n");
		printf("IE: @spawn\n");
		printf("To shutdown the server:\n");
		printf("server:shutdown\n");
	}

	end:
	aFree(buf);
	aFree(type);
	aFree(command);
	aFree(map);
	aFree(buf2);
	aFree(sd);

	return 0;
}

/*==========================================
 * İ’èƒtƒ@ƒCƒ‹‚ğ?‚İ?‚Ş
 *------------------------------------------
 */
int map_config_read(char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	struct hostent *h = NULL;

	fp = fopen(cfgName,"r");
	if (fp == NULL) {
		ShowFatalError("Map configuration file not found at: %s\n", cfgName);
		exit(1);
	}
	while(fgets(line, sizeof(line) -1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			if(strcmpi(w1,"timestamp_format")==0){
				strncpy(timestamp_format, w2, 20);
			} else if (strcmpi(w1, "userid")==0){
				chrif_setuserid(w2);
			} else if (strcmpi(w1, "passwd") == 0) {
				chrif_setpasswd(w2);
			} else if (strcmpi(w1, "char_ip") == 0) {
				char_ip_set_ = 1;
				h = gethostbyname (w2);
				if(h != NULL) {
					ShowInfo("Char Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%d.%d.%d.%d"CL_RESET"'.\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
					sprintf(w2,"%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				}
				chrif_setip(w2);
			} else if (strcmpi(w1, "char_port") == 0) {
				chrif_setport(atoi(w2));
			} else if (strcmpi(w1, "map_ip") == 0) {
				map_ip_set_ = 1;
				h = gethostbyname (w2);
				if (h != NULL) {
					ShowInfo("Map Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%d.%d.%d.%d"CL_RESET"'.\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
					sprintf(w2, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				}
				clif_setip(w2);
			} else if (strcmpi(w1, "bind_ip") == 0) {
				//bind_ip_set_ = 1;
				h = gethostbyname (w2);
				if (h != NULL) {
					ShowInfo("Map Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%d.%d.%d.%d"CL_RESET"'.\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
					sprintf(w2, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				}
				clif_setbindip(w2);
			} else if (strcmpi(w1, "map_port") == 0) {
				clif_setport(atoi(w2));
				map_port = (atoi(w2));
			} else if (strcmpi(w1, "water_height") == 0) {
				map_readwater(w2);
			} else if (strcmpi(w1, "map") == 0) {
				map_addmap(w2);
			} else if (strcmpi(w1, "delmap") == 0) {
				map_delmap(w2);
			} else if (strcmpi(w1, "npc") == 0) {
				npc_addsrcfile(w2);
			} else if (strcmpi(w1, "delnpc") == 0) {
				npc_delsrcfile(w2);
			} else if (strcmpi(w1, "autosave_time") == 0) {
				autosave_interval = atoi(w2) * 1000;
				if (autosave_interval <= 0)
					autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			} else if (strcmpi(w1, "motd_txt") == 0) {
				strcpy(motd_txt, w2);
			} else if (strcmpi(w1, "help_txt") == 0) {
				strcpy(help_txt, w2);
			} else if (strcmpi(w1, "mapreg_txt") == 0) {
				strcpy(mapreg_txt, w2);
			} else if(strcmpi(w1,"read_map_from_cache") == 0){
				if (atoi(w2) == 2)
					map_read_flag = READ_FROM_BITMAP_COMPRESSED;
				else if (atoi(w2) == 1)
					map_read_flag = READ_FROM_BITMAP;
				else
					map_read_flag = READ_FROM_GAT;
			} else if(strcmpi(w1,"map_cache_file") == 0) {
				strncpy(map_cache_file,w2,255);
			} else if(strcmpi(w1,"afm_dir") == 0) {
				strcpy(afm_dir, w2);
			} else if (strcmpi(w1, "console") == 0) {
				if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 ) {
					console = 1;
					ShowNotice("Console Commands is enabled.\n");
				}
			} else if (strcmpi(w1, "import") == 0) {
				map_config_read(w2);
			}
		}
	}
	fclose(fp);

	return 0;
}

int inter_config_read(char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: '%s'.\n",cfgName);
		return 1;
	}
	while(fgets(line,1020,fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"kick_on_disconnect")==0){
			kick_on_disconnect = battle_config_switch(w2);
	#ifndef TXT_ONLY
		} else if(strcmpi(w1,"charsave_method")==0){
			charsave_method = atoi(w2); //New char saving method.
		} else if(strcmpi(w1,"item_db_db")==0){
			strcpy(item_db_db,w2);
		} else if(strcmpi(w1,"mob_db_db")==0){
			strcpy(mob_db_db,w2);
		} else if(strcmpi(w1,"item_db2_db")==0){
			strcpy(item_db2_db,w2);
		} else if(strcmpi(w1,"mob_db2_db")==0){
			strcpy(mob_db2_db,w2);
		} else if(strcmpi(w1,"login_db_level")==0){
			strcpy(login_db_level,w2);
		} else if(strcmpi(w1,"login_db_account_id")==0){
		    strcpy(login_db_account_id,w2);
		} else if(strcmpi(w1,"login_db")==0){
			strcpy(login_db,w2);
		} else if (strcmpi(w1, "char_db") == 0) {
			strcpy(char_db, w2);
		} else if(strcmpi(w1,"gm_db_level")==0){
			strcpy(gm_db_level,w2);
		} else if(strcmpi(w1,"gm_db_account_id")==0){
		    strcpy(gm_db_account_id,w2);
		} else if(strcmpi(w1,"gm_db")==0){
			strcpy(gm_db,w2);
		//Map Server SQL DB
		} else if(strcmpi(w1,"map_server_ip")==0){
			strcpy(map_server_ip, w2);
		} else if(strcmpi(w1,"map_server_port")==0){
			map_server_port=atoi(w2);
		} else if(strcmpi(w1,"map_server_id")==0){
			strcpy(map_server_id, w2);
		} else if(strcmpi(w1,"map_server_pw")==0){
			strcpy(map_server_pw, w2);
		} else if(strcmpi(w1,"map_server_db")==0){
			strcpy(map_server_db, w2);
		} else if(strcmpi(w1,"use_sql_db")==0){
			db_use_sqldbs = battle_config_switch(w2);
			ShowStatus ("Using SQL dbs: %s\n",w2);
		//Login Server SQL DB
		} else if(strcmpi(w1,"login_server_ip")==0){
			strcpy(login_server_ip, w2);
        } else if(strcmpi(w1,"login_server_port")==0){
			login_server_port = atoi(w2);
		} else if(strcmpi(w1,"login_server_id")==0){
			strcpy(login_server_id, w2);
		} else if(strcmpi(w1,"login_server_pw")==0){
			strcpy(login_server_pw, w2);
		} else if(strcmpi(w1,"login_server_db")==0){
			strcpy(login_server_db, w2);
		} else if(strcmpi(w1,"lowest_gm_level")==0){
			lowest_gm_level = atoi(w2);
		} else if(strcmpi(w1,"read_gm_interval")==0){
			read_gm_interval = ( atoi(w2) * 60 * 1000 ); // Minutes multiplied by 60 secs per min by 1000 milliseconds per second
		}else if(strcmpi(w1, "char_server_ip") == 0){
			strcpy(charsql_host, w2);
		}else if(strcmpi(w1, "char_server_port") == 0){
			charsql_port = atoi(w2);
		}else if(strcmpi(w1, "char_server_id") == 0){
			strcpy(charsql_user, w2);
		}else if(strcmpi(w1, "char_server_pw") == 0){
			strcpy(charsql_pass, w2);
		}else if(strcmpi(w1, "char_server_db") == 0){
			strcpy(charsql_db, w2);
		} else if(strcmpi(w1,"log_db")==0) {
			strcpy(log_db, w2);
		} else if(strcmpi(w1,"log_db_ip")==0) {
			strcpy(log_db_ip, w2);
		} else if(strcmpi(w1,"log_db")==0) {
			strcpy(log_db, w2);
		} else if(strcmpi(w1,"log_db_id")==0) {
			strcpy(log_db_id, w2);
		} else if(strcmpi(w1,"log_db_pw")==0) {
			strcpy(log_db_pw, w2);
		} else if(strcmpi(w1,"log_db_port")==0) {
			log_db_port = atoi(w2);
	#endif
		//support the import command, just like any other config
		} else if(strcmpi(w1,"import")==0){
			inter_config_read(w2);
		}
	}
	fclose(fp);

	return 0;
}

#ifndef TXT_ONLY
/*=======================================
 *  MySQL Init
 *---------------------------------------
 */

int map_sql_init(void){

    mysql_init(&mmysql_handle);

	//DB connection start
	ShowInfo("Connect Map DB Server....\n");
	if(!mysql_real_connect(&mmysql_handle, map_server_ip, map_server_id, map_server_pw,
		map_server_db ,map_server_port, (char *)NULL, 0)) {
			//pointer check
			ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
			exit(1);
	}
	else {
		ShowStatus("connect success! (Map Server Connection)\n");
	}

    mysql_init(&lmysql_handle);

    //DB connection start
    ShowInfo("Connect Login DB Server....\n");
    if(!mysql_real_connect(&lmysql_handle, login_server_ip, login_server_id, login_server_pw,
        login_server_db ,login_server_port, (char *)NULL, 0)) {
	        //pointer check
			ShowSQL("DB error - %s\n",mysql_error(&lmysql_handle));
			exit(1);
	}
	 else {
		ShowStatus ("connect success! (Login Server Connection)\n");
	 }

	if(battle_config.mail_system) { // mail system [Valaris]
		mysql_init(&mail_handle);
		if(!mysql_real_connect(&mail_handle, map_server_ip, map_server_id, map_server_pw,
			map_server_db ,map_server_port, (char *)NULL, 0)) {
				ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
				exit(1);
		}
	}

	return 0;
}

int map_sql_close(void){
	mysql_close(&mmysql_handle);
	ShowStatus("Close Map DB Connection....\n");

	mysql_close(&lmysql_handle);
	ShowStatus("Close Login DB Connection....\n");

	if (log_config.sql_logs)
//Updating this if each time there's a log_config addition is too much of a hassle.	[Skotlex]
		/*&& (log_config.branch || log_config.drop || log_config.mvpdrop ||
		log_config.present || log_config.produce || log_config.refine || log_config.trade))*/
	{
		mysql_close(&logmysql_handle);
		ShowStatus("Close Log DB Connection....\n");
	}

	return 0;
}

int log_sql_init(void){

    mysql_init(&logmysql_handle);

	//DB connection start
	ShowInfo(""CL_WHITE"[SQL]"CL_RESET": Connecting to Log Database "CL_WHITE"%s"CL_RESET" At "CL_WHITE"%s"CL_RESET"...\n",log_db,log_db_ip);
	if(!mysql_real_connect(&logmysql_handle, log_db_ip, log_db_id, log_db_pw,
		log_db ,log_db_port, (char *)NULL, 0)) {
			//pointer check
			ShowSQL("DB error - %s\n",mysql_error(&logmysql_handle));
			exit(1);
	} else {
		ShowStatus(""CL_WHITE"[SQL]"CL_RESET": Successfully '"CL_GREEN"connected"CL_RESET"' to Database '"CL_WHITE"%s"CL_RESET"'.\n", log_db);
	}

	return 0;
}
#endif /* not TXT_ONLY */

void char_online_check(void)
{
	int i, users;
	struct map_session_data *sd, **all_sd;

	chrif_char_reset_offline();

	all_sd = map_getallusers(&users);

	for (i = 0; i < users; i++) {
		if ((sd = all_sd[i]) && !(battle_config.hide_GM_session && pc_isGM(sd)))
			 chrif_char_online(sd);
	}
}
int online_timer (int tid,unsigned int tick,int id,int data)
{
	char_online_check();
	return 0;
}

int id_db_final(void *k,void *d,va_list ap) { return 0; }
int pc_db_final(void *k,void *d,va_list ap) { return 0; }
int map_db_final(void *k,void *d,va_list ap) { return 0; }
int nick_db_final(void *k,void *d,va_list ap)
{
	char *p = (char *) d;
	if (p) aFree(p);
	return 0;
}
int charid_db_final(void *k,void *d,va_list ap)
{
	struct charid2nick *p = (struct charid2nick *) d;
	if (p) aFree(p);
	return 0;
}

int cleanup_sub(struct block_list *bl, va_list ap) {
	nullpo_retr(0, bl);

	switch(bl->type) {
		case BL_PC:
			map_quit((struct map_session_data *) bl);
			break;
		case BL_NPC:
			npc_unload((struct npc_data *)bl);
			break;
		case BL_MOB:
			mob_unload((struct mob_data *)bl);
			break;
		case BL_PET:
			pet_remove_map((struct map_session_data *)bl);
			break;
		case BL_ITEM:
			map_clearflooritem(bl->id);
			break;
		case BL_SKILL:
			skill_delunit((struct skill_unit *) bl);
			break;
	}

	return 0;
}

/*==========================================
 * mapII—¹—
 *------------------------------------------
 */
void do_final(void) {
	int i, j;
	ShowStatus("Terminating...\n");

	map_cache_close();
	grfio_final();

	for (i = 0; i < map_num; i++)
		if (map[i].m >= 0)
			map_foreachinarea(cleanup_sub, i, 0, 0, map[i].xs, map[i].ys, 0);

	chrif_char_reset_offline();
	chrif_flush_fifo();

//#if 0	// why is this here? >_>
	do_final_chrif(); // ‚±‚Ì“à•”‚ÅƒLƒƒƒ‰‚ğ‘S‚ÄØ’f‚·‚é
	do_final_npc();
//	map_removenpc();
	do_final_script();
	do_final_itemdb();
	do_final_storage();
	do_final_guild();
	do_final_party();
	do_final_pc();
	do_final_pet();
	do_final_mob();
	do_final_msg();

	map_getallusers(NULL); //Clear the memory allocated for this array.
	
	for (i=0; i<map_num; i++) {
		if(map[i].gat) aFree(map[i].gat);
		if(map[i].block) aFree(map[i].block);
		if(map[i].block_mob) aFree(map[i].block_mob);
		if(map[i].block_count) aFree(map[i].block_count);
		if(map[i].block_mob_count) aFree(map[i].block_mob_count);
		if(battle_config.dynamic_mobs) { //Dynamic mobs flag by [random]
			for (j=0; j<MAX_MOB_LIST_PER_MAP; j++)
				if (map[i].moblist[j]) aFree(map[i].moblist[j]);
		}
	}

	numdb_final(id_db, id_db_final);
	numdb_final(pc_db, pc_db_final);
	strdb_final(map_db, map_db_final);
	strdb_final(nick_db, nick_db_final);
	numdb_final(charid_db, charid_db_final);

//#endif

#ifndef TXT_ONLY
    map_sql_close();
	if(charsave_method)
		charsql_db_init(0); //Connecting to chardb
#endif /* not TXT_ONLY */
	ShowStatus("Successfully terminated.\n");
}

/*======================================================
 * Map-Server Version Screen [MC Cameri]
 *------------------------------------------------------
 */
void map_helpscreen(int flag) { // by MC Cameri
	puts("Usage: map-server [options]");
	puts("Options:");
	puts(CL_WHITE"  Commands\t\t\tDescription"CL_RESET);
	puts("-----------------------------------------------------------------------------");
	puts("  --help, --h, --?, /?		Displays this help screen");
	puts("  --map-config <file>		Load map-server configuration from <file>");
	puts("  --battle-config <file>	Load battle configuration from <file>");
	puts("  --atcommand-config <file>	Load atcommand configuration from <file>");
	puts("  --charcommand-config <file>	Load charcommand configuration from <file>");
	puts("  --script-config <file>	Load script configuration from <file>");
	puts("  --msg-config <file>		Load message configuration from <file>");
	puts("  --grf-path-file <file>	Load grf path file configuration from <file>");
	puts("  --sql-config <file>		Load inter-server configuration from <file>");
	puts("				(SQL Only)");
	puts("  --log-config <file>		Load logging configuration from <file>");
	puts("				(SQL Only)");
	puts("  --version, --v, -v, /v	Displays the server's version");
	puts("\n");
	if (flag) exit(1);
}

/*======================================================
 * Map-Server Version Screen [MC Cameri]
 *------------------------------------------------------
 */
void map_versionscreen(int flag) {
	printf("CL_WHITE" "eAthena version %d.%02d.%02d, Athena Mod version %d" CL_RESET"\n",
		ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION,
		ATHENA_MOD_VERSION);
	puts(CL_GREEN "Website/Forum:" CL_RESET "\thttp://eathena.deltaanime.net/");
	puts(CL_GREEN "Download URL:" CL_RESET "\thttp://eathena.systeminplace.net/");
	puts(CL_GREEN "IRC Channel:" CL_RESET "\tirc://irc.deltaanime.net/#athena");
	puts("\nOpen " CL_WHITE "readme.html" CL_RESET " for more information.");
	if (ATHENA_RELEASE_FLAG) ShowNotice("This version is not for release.\n");
	if (flag) exit(1);
}

/*======================================================
 * Map-Server Init and Command-line Arguments [Valaris]
 *------------------------------------------------------
 */
void set_server_type(void)
{
	SERVER_TYPE = ATHENA_SERVER_MAP;
}
int do_init(int argc, char *argv[]) {
	int i;

#ifdef GCOLLECT
	GC_enable_incremental();
#endif

	INTER_CONF_NAME="conf/inter_athena.conf";
	LOG_CONF_NAME="conf/log_athena.conf";
	MAP_CONF_NAME = "conf/map_athena.conf";
	BATTLE_CONF_FILENAME = "conf/battle_athena.conf";
	ATCOMMAND_CONF_FILENAME = "conf/atcommand_athena.conf";
	CHARCOMMAND_CONF_FILENAME = "conf/charcommand_athena.conf";
	SCRIPT_CONF_NAME = "conf/script_athena.conf";
	MSG_CONF_NAME = "conf/msg_athena.conf";
	GRF_PATH_FILENAME = "conf/grf-files.txt";

	chrif_connected = 0;

	srand(gettick());

	for (i = 1; i < argc ; i++) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "--h") == 0 || strcmp(argv[i], "--?") == 0 || strcmp(argv[i], "/?") == 0)
			map_helpscreen(1);
		else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "--v") == 0 || strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "/v") == 0)
			map_versionscreen(1);
		else if (strcmp(argv[i], "--map_config") == 0 || strcmp(argv[i], "--map-config") == 0)
			MAP_CONF_NAME=argv[i+1];
		else if (strcmp(argv[i],"--battle_config") == 0 || strcmp(argv[i],"--battle-config") == 0)
			BATTLE_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--atcommand_config") == 0 || strcmp(argv[i],"--atcommand-config") == 0)
			ATCOMMAND_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--charcommand_config") == 0 || strcmp(argv[i],"--charcommand-config") == 0)
			CHARCOMMAND_CONF_FILENAME = argv[i+1];
		else if (strcmp(argv[i],"--script_config") == 0 || strcmp(argv[i],"--script-config") == 0)
			SCRIPT_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--msg_config") == 0 || strcmp(argv[i],"--msg-config") == 0)
			MSG_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--grf_path_file") == 0 || strcmp(argv[i],"--grf-path-file") == 0)
			GRF_PATH_FILENAME = argv[i+1];
#ifndef TXT_ONLY
		else if (strcmp(argv[i],"--inter_config") == 0 || strcmp(argv[i],"--inter-config") == 0)
			INTER_CONF_NAME = argv[i+1];
#endif
		else if (strcmp(argv[i],"--log_config") == 0 || strcmp(argv[i],"--log-config") == 0)
			LOG_CONF_NAME = argv[i+1];
		else if (strcmp(argv[i],"--run_once") == 0)	// close the map-server as soon as its done.. for testing [Celest]
			runflag = 0;
	}

	map_config_read(MAP_CONF_NAME);

	if ((naddr_ == 0) && (map_ip_set_ == 0 || char_ip_set_ == 0)) {
		ShowError("\nUnable to determine your IP address... please edit the map_athena.conf file and set it.\n");
		ShowError("(127.0.0.1 is valid if you have no network interface)\n");
	}

	if (map_ip_set_ == 0 || char_ip_set_ == 0) {
		// The map server should know what IP address it is running on
		//   - MouseJstr
		int localaddr = ntohl(addr_[0]);
		unsigned char *ptr = (unsigned char *) &localaddr;
		char buf[16];
		sprintf(buf, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);;
		if (naddr_ != 1)
			ShowNotice("Multiple interfaces detected..  using %s as our IP address\n", buf);
		else
			ShowInfo("Defaulting to %s as our IP address\n", buf);
		if (map_ip_set_ == 0)
			clif_setip(buf);
		if (char_ip_set_ == 0)
				chrif_setip(buf);
		if (ptr[0] == 192 && ptr[1] == 168)
			ShowError("\nFirewall detected.. \n    edit lan_support.conf and map_athena.conf\n\n");
	}

	if (SHOW_DEBUG_MSG)
		ShowNotice("Server running in '"CL_WHITE"Debug Mode"CL_RESET"'.\n");

	battle_config_read(BATTLE_CONF_FILENAME);
	msg_config_read(MSG_CONF_NAME);
	atcommand_config_read(ATCOMMAND_CONF_FILENAME);
	charcommand_config_read(CHARCOMMAND_CONF_FILENAME);
	script_config_read(SCRIPT_CONF_NAME);
	inter_config_read(INTER_CONF_NAME);
	log_config_read(LOG_CONF_NAME);

	id_db = numdb_init();
	pc_db = numdb_init();	//Added for reliable map_id2sd() use. [Skotlex]
	map_db = strdb_init(MAP_NAME_LENGTH);
	nick_db = strdb_init(NAME_LENGTH);
	charid_db = numdb_init();
#ifndef TXT_ONLY
	map_sql_init();
	if(charsave_method)
		charsql_db_init(1); //Connecting to chardb
#endif /* not TXT_ONLY */

	grfio_init(GRF_PATH_FILENAME);

	map_readallmap();

	add_timer_func_list(map_freeblock_timer, "map_freeblock_timer");
	add_timer_func_list(map_clearflooritem_timer, "map_clearflooritem_timer");
	add_timer_func_list(map_removemobs_timer, "map_removemobs_timer");
	add_timer_interval(gettick()+1000, map_freeblock_timer, 0, 0, 60*1000);

	// online status timer, checks every hour [Valaris]
	add_timer_func_list(online_timer, "online_timer");
	add_timer_interval(gettick()+1000, online_timer, 0, 0, CHECK_INTERVAL);

	do_init_chrif();
	do_init_clif();
	do_init_itemdb();
	do_init_mob();	// npc‚Ì‰Šú‰»‚Åmob_spawn‚µ‚ÄAmob_db‚ğ?Æ‚·‚é‚Ì‚Åinit_npc‚æ‚èæ
	do_init_script();
	do_init_pc();
	do_init_status();
	do_init_party();
	do_init_guild();
	do_init_storage();
	do_init_skill();
	do_init_pet();
	do_init_npc();

#ifndef TXT_ONLY /* mail system [Valaris] */
	if(battle_config.mail_system)
		do_init_mail();

	if (log_config.sql_logs)
	{
		log_sql_init();
	}
#endif /* not TXT_ONLY */

	npc_event_do_oninit();	// npc‚ÌOnInitƒCƒxƒ“ƒg?s

	if ( console ) {
		set_defaultconsoleparse(parse_console);
		start_console();
	}

	if (battle_config.pk_mode == 1)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	ShowStatus("Server is '"CL_GREEN"ready"CL_RESET"' and listening on port '"CL_WHITE"%d"CL_RESET"'.\n\n", map_port);

	return 0;
}

int compare_item(struct item *a, struct item *b) {

	if (a->nameid == b->nameid &&
		a->identify == b->identify &&
		a->refine == b->refine &&
		a->attribute == b->attribute)
	{
		int i;
		for (i = 0; i < MAX_SLOTS && (a->card[i] == b->card[i]); i++);
		return (i == MAX_SLOTS);
	}
	return 0;
}

#ifndef TXT_ONLY
int charsql_db_init(int method){


    	if(method == 1){ //'INIT / START'
                  ShowInfo("Connecting to 'character' Database... ");
	         mysql_init(&charsql_handle);

	         if(!mysql_real_connect(&charsql_handle, charsql_host, charsql_user, charsql_pass, charsql_db, charsql_port, (char *)NULL, 0)){
							ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
	                 exit(1);
	         }else{
	                 printf("success.\n");
	         }
         }else if(method == 0){ //'FINAL' / Shutdown
         	ShowInfo("Closing 'character' Database connection ... ");
                 mysql_close(&charsql_handle);
                 printf("done.\n");
         }

    return 0;
}
#endif
