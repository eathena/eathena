#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/strlib.h"
#include "chardb.h"
#include "int_fame.h"
#include <string.h>

//temporary imports
extern CharDB* chars;


void char_read_fame_list(void)
{
	struct fame_list* list;
	int size;
	CharDBIterator* iter;
	struct mmo_charstatus cd;
	int i;

	get_fame_list(FAME_SMITH, &list, &size);
	memset(list, 0, MAX_FAME_LIST*sizeof(struct fame_list));
	get_fame_list(FAME_CHEMIST, &list, &size);
	memset(list, 0, MAX_FAME_LIST*sizeof(struct fame_list));
	get_fame_list(FAME_TAEKWON, &list, &size);
	memset(list, 0, MAX_FAME_LIST*sizeof(struct fame_list));
	
	//FIXME: horribly expensive processing
	iter = chars->iterator(chars);
	while( iter->next(iter, &cd) )
	{
		if( cd.fame == 0 )
			continue;

		list = NULL;
		if( cd.class_ == JOB_BLACKSMITH || cd.class_ == JOB_WHITESMITH || cd.class_ == JOB_BABY_BLACKSMITH )
			get_fame_list(FAME_SMITH, &list, &size);
		else
		if( cd.class_ == JOB_ALCHEMIST || cd.class_ == JOB_CREATOR || cd.class_ == JOB_BABY_ALCHEMIST )
			get_fame_list(FAME_CHEMIST, &list, &size);
		else
		if( cd.class_ == JOB_TAEKWON )
			get_fame_list(FAME_TAEKWON, &list, &size);

		if( list != NULL )
		{
			ARR_FIND( 0, size, i, cd.fame > list[i].fame );
			if( i < size )
			{// insert new entry
				ARR_MOVELEFT(size-1, i, list, struct fame_list);
				list[i].id = cd.char_id;
				list[i].fame = cd.fame;
				safestrncpy(list[i].name, cd.name, NAME_LENGTH);
			}
		}
	}
	iter->destroy(iter);
}
