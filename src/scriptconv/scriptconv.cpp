// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "basesafeptr.h"
#include "basetime.h"
#include "baseparser.h"
#include "basefile.h"
#include "basestrsearch.h"

#include "scriptengine.h"
#include "oldeascriptconv.h"
#include "aegisscriptconv.h"
#include "dblookup.h"
#include "baseparam.h"

///////////////////////////////////////////////////////////////////////////////
// basic class for using the old way timers
///////////////////////////////////////////////////////////////////////////////
bool basics::CTimerBase::init(unsigned long interval)
{
	return false;
}

// external calling from external timer implementation
int basics::CTimerBase::timercallback(int timer, unsigned long tick, int id, basics::numptr data)
{
	return 0;
}
void basics::CTimerBase::timerfinalize()
{

}




void usage(const char*p)
{
	fprintf(stderr, "usage: %s [bptli] [-i <itemdb>] [-m <mobdb>] <input files/folder>\n", (p)?p:"<binary>");
	fprintf(stderr, "     option b: outputs beautified code\n");
	fprintf(stderr, "     option p: prints parse tree\n");
	fprintf(stderr, "     option t: prints transformation tree\n");
	fprintf(stderr, "     option l: enables logging for unknown function and variable names\n");
	fprintf(stderr, "[-i<itemdb>]: read itemdb from different place than ./db/item_db.txt\n");
	fprintf(stderr, " [-m<mobdb>]: read mobdb from different place than ./db/mob_db.txt\n");
	fprintf(stderr, " [-n<npcdb>]: read mobdb from different place than ./db/npc_db.txt\n");
	fprintf(stderr, "     default only checks syntax errors\n");
}

int get_option(const char* p)
{
	int option = OPT_PARSE;
	if(p)
	{
		// ignore leading minus
		if(*p=='-') ++p;
		for(; *p; ++p)
		{
			if(*p=='b')
				option |= OPT_BEAUTIFY;
			else if(*p=='p')
				option |= OPT_PRINTTREE;
			else if(*p=='t')
				option |= OPT_TRANSFORM;
			else if(*p=='l')
				option |= OPT_LOGGING;
			else
				fprintf(stderr, "unknown option %c\n", *p);
		}
	}
	return option;
}

// Accepts 3 arguments [engine file] [option(s)] <input file>
int main(int argc, char *argv[])
{
//	buildEngine();

	ulong tick = GetTickCount();
	bool ok=false;

	// parse commandline
	int i, c, option=OPT_PARSE;
	const char* itemdbpath = "./db/item_db.txt";
	const char* mobdbpath = "./db/mob_db.txt";
	const char* npcdbpath = "./db/npc_db.txt";

	for(c=0, i=1; i<argc; ++i)
	{
		if( basics::is_file(argv[i]) || basics::is_folder(argv[i]) )
		{
			++c;
		}
		else
		{	// test for option or overwrite
			if( argv[i][0] == '-' && argv[i][1] == 'i' 
				&& basics::is_file(argv[i]+2) )
			{	// overwrite mobdb
				itemdbpath = argv[i]+2;
			}
			else if( argv[i][0] == '-' && argv[i][1] == 'm' 
				&& basics::is_file(argv[i]+2) )
			{	// overwrite mobdb
				mobdbpath = argv[i]+2;
			}
			else if( argv[i][0] == '-' && argv[i][1] == 'n' 
				&& basics::is_file(argv[i]+2) )
			{	// overwrite npcdb
				npcdbpath = argv[i]+2;
			}
			else
			{	// option
				option = get_option(argv[i]);
			}
		}
	}
	if(!c)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	// load item and mobdb
	itemdb_entry::load(itemdbpath);
	mobdb_entry::load(mobdbpath);
	npcdb_entry::load(npcdbpath);


	oldeaparserstorage oldea;
	aegisparserstorage aegis;

	oldea.init(NULL);
	aegis.init(NULL);

	if( oldea.is_valid() || aegis.is_valid() )
	{
		oldeaParser ea(oldea, option);
		aegisParser ae(aegis, option);

		// do the arguments
		for(i=1; i<argc; ++i)
		{
			if( basics::is_folder( argv[i] ) )
			{	// folder
				ok = true;
				if(oldea) ok &= basics::findFiles(argv[i], "*.txt", ea);
				if(aegis) ok &= basics::findFiles(argv[i], "*.sc",  ae);
			}
			else if( basics::is_file( argv[i] ) )
			{	// single file
				if( aegis && basics::match_wildcard("*.sc", argv[i]) )
					ok=ae.process(argv[i]);
				else if(oldea)
					ok=ea.process(argv[i]);	
			}
		}
		fprintf(stderr, "\nready (%i)\n", ok);
		fprintf(stderr, "elapsed time: %li\n", (unsigned long)(GetTickCount()-tick));

		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
