
#include "baseobjects.h"


#ifdef COUNT_GLOBALS
int global::sGlobalCount=0;
#ifdef DEBUG
global::_globalcount global::gc;
global::_globalcount::_globalcount()
{
	printf("init countint of global objects\n");
}
global::_globalcount::~_globalcount()
{
	printf("\nterminating global objects.\n");
#ifndef SINGLETHREAD
	sleep(1000); //give some time to clean up
#endif
	if( global::getcount() > 0 )
	{
		printf("global object count: %i\n", global::getcount());
		printf("still not dealloced everything, may be leaky or just static objects.\n");
	}
	else
	{
		printf("global objects clear.\n");
	}
}
#endif
#endif




