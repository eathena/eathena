// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "mercenary.h"
#include "basetypes.h"


/// level depending factor.
/// for ie. exponential increase/decrease of costs
double level_factor(unsigned short wishlv, unsigned short baselv)
{
	const double lvl_diff = 10.0;
	return pow(2.0, ((double)(wishlv-baselv))/lvl_diff);
}



int do_init_merc (void)
{
	// read DB's
	return 0;
}

int do_final_merc (void)
{
	// clean up
	return 0;
}
