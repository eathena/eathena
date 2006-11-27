// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "scriptengine.h"
#include "basezlib.h"




const unsigned char* extractEngine(const unsigned char*buffer, unsigned long sourcesz, unsigned long &sz)
{
	static unsigned char buffer1[128*1024];
	basics::CZlib zlib;

	sz = sizeof(buffer1);
	if( 0==zlib.decode(buffer1, sz, buffer, sourcesz) )
	{
		fprintf(stderr, "loading engine...(%lu)\n", sz);
		return buffer1;
	}
	return NULL;
}

void buildEngine(const char*filename)
{
	static unsigned char buffer1[128*1024];
	static unsigned char buffer2[128*1024];
	FILE *fp = fopen(filename, "rb");
	ulong sz=fread(buffer1, 1, sizeof(buffer1), fp);

	ulong i, sz2=sizeof(buffer2);
	basics::CZlib zlib;
	zlib.encode(buffer2, sz2, buffer1, sz);

	for(i=0; i<sz2; ++i)
	{
		if(i%8==0) printf("\n");
		fprintf(stdout, "0x%02X, ", buffer2[i]);
	}
	exit(0);
}

bool parserstorage::init(const char* enginefile)
{
	if(parser_config) delete parser_config;
	if(parser) delete parser;
	if(enginefile)
	{
		try
		{
			this->parser_config = new basics::CParseConfig( enginefile );
		}
		catch(...)
		{
			this->parser_config=NULL;
		}
		if (!this->parser_config)
		{
			fprintf(stderr, "Could not open engine file %s\n", enginefile);
			return false;
		}
	}
	else
	{
		const unsigned char *buffer;
		ulong sz;
		this->getEngine(buffer, sz);
		const unsigned char *e = extractEngine(buffer, sz, sz);
		if(!e)
		{
			fprintf(stderr, "Error creating parser\n");
			return EXIT_FAILURE;
		}
		this->parser_config = new basics::CParseConfig(e, sz);
		if (!this->parser_config)
		{
			fprintf(stderr, "Could not load engine\n");
			return false;
		}
	}

	this->parser = new basics::CParser_CommentStore(parser_config);
	if (!this->parser)
	{
		fprintf(stderr, "Error creating parser\n");
		return false;
	}
	return true;
}
