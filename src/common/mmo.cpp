

#include "base.h"
#include "mmo.h"
#include "showmsg.h"
#include "malloc.h"
#include "utils.h"


bool operator==(const struct item& a, const struct item& b)
{
	return ( (a.id == b.id) &&
			 (a.nameid == b.nameid) &&
			 (a.amount == b.amount) &&
			 (a.equip == b.equip) &&
			 (a.identify == b.identify) &&
			 (a.refine == b.refine) &&
			 (a.attribute == b.attribute) &&
			 (a.card[0] == b.card[0]) &&
			 (a.card[1] == b.card[1]) &&
			 (a.card[2] == b.card[2]) &&
			 (a.card[3] == b.card[3]) );
}


