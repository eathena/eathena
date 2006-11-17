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
	fprintf(stderr, "usage: %s [engine file] [bptl] <input file/folder>\n", (p)?p:"<binary>");
	fprintf(stderr, "     option b: outputs beautified code\n");
	fprintf(stderr, "     option p: prints parse tree\n");
	fprintf(stderr, "     option t: prints transformation tree\n");
	fprintf(stderr, "     option l: enables logging for function and variable names\n");
	fprintf(stderr, "     default only checks syntax errors\n");
}

int get_option(const char* p)
{
	int option = OPT_PARSE;
	if(p)
	{
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
	const char* enginefile=NULL;
	const char* inputfile=NULL;
	int i, option=OPT_PARSE;

	for(i=1; i<argc; ++i)
	{
		if( basics::is_file(argv[i]) )
		{
			if(!enginefile)
				enginefile = argv[i];
			else 
				inputfile = argv[i];
		}
		else if( basics::is_folder(argv[i]) )
		{
			inputfile = argv[i];
		}
		else
		{
			option = get_option(argv[i]);
		}
	}
	if(enginefile && !inputfile)
		basics::swap(enginefile, inputfile);

	if(!inputfile)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	oldeaparserstorage oldea;
	aegisparserstorage aegis;

	oldea.init(enginefile);
	aegis.init(enginefile);

	if( oldea.is_valid() || aegis.is_valid() )
	{
		oldeaParser ea(oldea, option);
		aegisParser ae(aegis, option);

		if( basics::is_folder( inputfile ) )
		{	// folder
			ok = true;
			if(oldea) ok &= basics::findFiles(inputfile, "*.txt", ea);
			if(aegis) ok &= basics::findFiles(inputfile, "*.sc",  ae);
		}
		else
		{	// single file
			if( aegis && basics::match_wildcard("*.sc", inputfile) )
				ok=ae.process(inputfile);
			else if(oldea)
				ok=ea.process(inputfile);	
		}
		fprintf(stderr, "\nready (%i)\n", ok);
		fprintf(stderr, "elapsed time: %li\n", (unsigned long)(GetTickCount()-tick));

		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
