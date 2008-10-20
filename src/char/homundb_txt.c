// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "homundb.h"
#include <string.h>

#define START_HOMUN_NUM 1

/// internal structure
typedef struct HomunDB_TXT
{
	HomunDB vtable;      // public interface

	DBMap* homuns;       // in-memory homun storage
	int next_homun_id;   // auto_increment

	char homun_db[1024]; // homun data storage file

} HomunDB_TXT;

/// internal functions
static bool homun_db_txt_init(HomunDB* self);
static void homun_db_txt_destroy(HomunDB* self);
static bool homun_db_txt_sync(HomunDB* self);
static bool homun_db_txt_create(HomunDB* self, struct s_homunculus* hd);
static bool homun_db_txt_remove(HomunDB* self, const int homun_id);
static bool homun_db_txt_save(HomunDB* self, const struct s_homunculus* hd);
static bool homun_db_txt_load_num(HomunDB* self, struct s_homunculus* hd, int homun_id);

static bool mmo_homun_fromstr(struct s_homunculus* hd, char* str);
static bool mmo_homun_tostr(const struct s_homunculus* hd, char* str);
static bool mmo_homun_sync(HomunDB_TXT* db);

/// public constructor
HomunDB* homun_db_txt(void)
{
	HomunDB_TXT* db = (HomunDB_TXT*)aCalloc(1, sizeof(HomunDB_TXT));

	// set up the vtable
	db->vtable.init      = &homun_db_txt_init;
	db->vtable.destroy   = &homun_db_txt_destroy;
	db->vtable.sync      = &homun_db_txt_sync;
	db->vtable.create    = &homun_db_txt_create;
	db->vtable.remove    = &homun_db_txt_remove;
	db->vtable.save      = &homun_db_txt_save;
	db->vtable.load_num  = &homun_db_txt_load_num;

	// initialize to default values
	db->homuns = NULL;
	db->next_homun_id = START_HOMUN_NUM;
	// other settings
	safestrncpy(db->homun_db, "save/homun.txt", sizeof(db->homun_db));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


static bool homun_db_txt_init(HomunDB* self)
{
/*
	char line[8192];
	struct s_homunculus *p;
	FILE *fp;
	int c=0;

	homun_db= idb_alloc(DB_OPT_RELEASE_DATA);

	if( (fp=fopen(homun_txt,"r"))==NULL )
		return 1;
	while(fgets(line, sizeof(line), fp))
	{
		p = (struct s_homunculus*)aCalloc(sizeof(struct s_homunculus), 1);
		if(p==NULL){
			ShowFatalError("int_homun: out of memory!\n");
			exit(EXIT_FAILURE);
		}
		if(inter_homun_fromstr(line,p)==0 && p->hom_id>0){
			if( p->hom_id >= homun_newid)
				homun_newid=p->hom_id+1;
			idb_put(homun_db,p->hom_id,p);
		}else{
			ShowError("int_homun: broken data [%s] line %d\n",homun_txt,c);
			aFree(p);
		}
		c++;
	}
	fclose(fp);
	return 0;
*/
}

static void homun_db_txt_destroy(HomunDB* self)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;

	// write data
	mmo_homun_sync(db);

	// delete homun database
	homuns->destroy(homuns, NULL);
	db->homuns = NULL;

	// delete entire structure
	aFree(db);
}

static bool homun_db_txt_sync(HomunDB* self)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	return mmo_homun_sync(db);
}

static bool homun_db_txt_create(HomunDB* self, struct s_homunculus* hd)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;
	struct s_homunculus* tmp;

	// decide on the homun id to assign
	int homun_id = ( hd->hom_id != -1 ) ? hd->hom_id : db->next_homun_id;

	// check if the homun is free
	tmp = idb_get(homuns, homun_id);
	if( tmp != NULL )
	{// error condition - entry already present
		ShowError("homun_db_txt_create: cannot create homunculus %d:'%s', this id is already occupied by %d:'%s'!\n", homun_id, hd->name, homun_id, tmp->name);
		return false;
	}

	// copy the data and store it in the db
	CREATE(tmp, struct s_homunculus, 1);
	memcpy(tmp, hd, sizeof(struct s_homunculus));
	tmp->hom_id = homun_id;
	idb_put(homuns, homun_id, tmp);

	// increment the auto_increment value
	if( homun_id >= db->next_homun_id )
		db->next_homun_id = homun_id + 1;

	// flush data
	mmo_homun_sync(db);

	// write output
	hd->hom_id = homun_id;

	return true;
}

static bool homun_db_txt_remove(HomunDB* self, const int homun_id)
{
	HomunDB_TXT* db = (HomunDB_TXT*)self;
	DBMap* homuns = db->homuns;

	struct s_homunculus* tmp = (struct s_homunculus*)idb_remove(homuns, homun_id);
	if( tmp == NULL )
	{// error condition - entry not present
		ShowError("homun_db_txt_remove: no such homunculus with id %d\n", homun_id);
		return false;
	}

	return true;
}

static bool homun_db_txt_save(HomunDB* self, const struct s_homunculus* hd)
{
	/*
	FILE *fp;
	int lock;
	if( (fp=lock_fopen(homun_txt,&lock))==NULL ){
		ShowError("int_homun: can't write [%s] !!! data is lost !!!\n",homun_txt);
		return 1;
	}
	homun_db->foreach(homun_db,inter_homun_save_sub,fp);
	lock_fclose(fp,homun_txt,&lock);
	return 0;

	char line[8192];
	FILE *fp;
	inter_homun_tostr(line,(struct s_homunculus *)data);
	fp=va_arg(ap,FILE *);
	fprintf(fp,"%s\n",line);
	return 0;
*/
}

static bool homun_db_txt_load_num(HomunDB* self, struct s_homunculus* hd, int homun_id)
{
}


static bool mmo_homun_fromstr(struct s_homunculus* hd, char* str)
{
/*
	int i, next, len;
	int tmp_int[25];
	unsigned int tmp_uint[5];
	char tmp_str[256];

	memset(p,0,sizeof(struct s_homunculus));

	i=sscanf(str,"%d,%d\t%127[^\t]\t%d,%d,%d,%d,%d,"
		"%u,%d,%d,%d,"
		"%u,%d,%d,"
		"%d,%d,%d,%d,%d,%d\t%n",
		&tmp_int[0],&tmp_int[1],tmp_str,
		&tmp_int[2],&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],
		&tmp_uint[0],&tmp_int[7],&tmp_int[8],&tmp_int[9],
		&tmp_uint[1],&tmp_int[10],&tmp_int[11],
		&tmp_int[12],&tmp_int[13],&tmp_int[14],&tmp_int[15],&tmp_int[16],&tmp_int[17],
		&next);

	if( i != 21 )
		return 1;

	p->hom_id = tmp_int[0];
	p->class_ = tmp_int[1];
	memcpy(p->name, tmp_str, NAME_LENGTH);

	p->char_id = tmp_int[2];
  	p->hp = tmp_int[3];
	p->max_hp = tmp_int[4];
	p->sp = tmp_int[5];
	p->max_sp = tmp_int[6];

	p->intimacy = tmp_uint[0];
	p->hunger = tmp_int[7];
	p->skillpts = tmp_int[8];
	p->level = tmp_int[9];

	p->exp = tmp_uint[1];
	p->rename_flag = tmp_int[10];
	p->vaporize = tmp_int[11];

	p->str = tmp_int[12];
	p->agi = tmp_int[13];
	p->vit = tmp_int[14];
	p->int_= tmp_int[15];
	p->dex = tmp_int[16];
	p->luk = tmp_int[17];

	//Read skills.
	while(str[next] && str[next] != '\n' && str[next] != '\r') {
		if (sscanf(str+next, "%d,%d,%n", &tmp_int[0], &tmp_int[1], &len) != 2)
			return 2;

		if (tmp_int[0] >= HM_SKILLBASE && tmp_int[0] < HM_SKILLBASE+MAX_HOMUNSKILL)
		{
			i = tmp_int[0] - HM_SKILLBASE;
			p->hskill[i].id = tmp_int[0];
			p->hskill[i].lv = tmp_int[1];
		} else
			ShowError("Read Homun: Unsupported Skill ID %d for homunculus (Homun ID=%d)\n", tmp_int[0], p->hom_id);
		next += len;
		if (str[next] == ' ')
			next++;
	}
	return 0;
*/
}

static bool mmo_homun_tostr(const struct s_homunculus* hd, char* str)
{
/*
	int i;

	str+=sprintf(str,"%d,%d\t%s\t%d,%d,%d,%d,%d,"
		"%u,%d,%d,%d,"
		"%u,%d,%d,"
		"%d,%d,%d,%d,%d,%d\t",
		p->hom_id, p->class_, p->name,
		p->char_id, p->hp, p->max_hp, p->sp, p->max_sp,
	  	p->intimacy, p->hunger, p->skillpts, p->level,
		p->exp, p->rename_flag, p->vaporize,
		p->str, p->agi, p->vit, p->int_, p->dex, p->luk);

	for (i = 0; i < MAX_HOMUNSKILL; i++)
	{
		if (p->hskill[i].id && !p->hskill[i].flag)
			str+=sprintf(str,"%d,%d,", p->hskill[i].id, p->hskill[i].lv);
	}

	return 0;
*/
}

static bool mmo_homun_sync(HomunDB_TXT* db)
{
}
