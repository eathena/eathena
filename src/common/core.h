// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// original : core.h 2003/03/14 11:55:25 Rev 1.4

#ifndef	_CORE_H_
#define	_CORE_H_

#include "basetypes.h"
#include "baseobjects.h"
#include "version.h"


extern int do_init(int,char**);
extern void do_final();
extern unsigned char getServerType();

void core_stoprunning();

const char* get_svn_revision();
void display_version(bool quit=false);
void display_title(void);

///////////////////////////////////////////////////////////////////////////////
/// uptime class.
class uptime : public basics::noncopyable
{
	static time_t starttime;
public:
	static const char *tostring(char *buffer=NULL);
	static void getvalues(unsigned long& days,unsigned long& hours,unsigned long& minutes,unsigned long& seconds);
	double gettime() const	{ return difftime(time(NULL),starttime); }
};


#endif	// _CORE_H_
