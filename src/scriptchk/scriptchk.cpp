// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "basesafeptr.h"
#include "basetime.h"

#include "eaprogram.h"
#include "eaparser.h"
#include "eacompiler.h"
#include "eastorage.h"

void usage(const char*p)
{
	fprintf(stderr, "usage: %s [bptco] <input file>\n", (p)?p:"<binary>");
	fprintf(stderr, "     option b: outputs beautified code\n");
	fprintf(stderr, "     option p: prints parse tree\n");
	fprintf(stderr, "     option t: prints transformation tree\n");
	fprintf(stderr, "     option c: does compilation\n");
	fprintf(stderr, "     option o: prints compilation output\n");
}

int get_option(const char* p)
{
	int option = OPT_PARSE;
	if(p)
	{
		while(*p)
		{
			if(*p=='b')
				option |= OPT_BEAUTIFY;
			else if(*p=='p')
				option |= OPT_PRINTTREE;
			else if(*p=='t')
				option |= OPT_TRANSFORM;
			else if(*p=='c')
				option |= OPT_COMPILE;
			else if(*p=='o')
				option |= OPT_COMPILEOUTPUT;
			p++;
		}
	}
	return option;
}



int main(int argc, char *argv[])
{
//	buildEngine();
	ulong tick = GetTickCount();
	bool ok=false;

	// parse commandline
	int i, c, option=OPT_PARSE;

	for(c=0, i=1; i<argc; ++i)
	{
		if( basics::is_file(argv[i]) || basics::is_folder(argv[i]) )
		{
			++c;
		}
		else
		{	// test for option or overwrite
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

	eacompiler compiler;
	for(i=1; i<argc; ++i)
	{
		if( basics::is_file(argv[i]) )
		{
			if( !(ok=compiler.load_file(argv[i], option)) )
				break;
		}
	}

	fprintf(stderr, "\nready (%i)\n", ok);
	fprintf(stderr, "elapsed time: %lu\n", (unsigned long)(GetTickCount()-tick));
	return EXIT_SUCCESS;
}
