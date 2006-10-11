// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "scriptengine.h"

/// array for compile-emedded script engine
const unsigned char engine[] = 
{
#include "oldeascript.engine.txt"
};



#include "basezlib.h"


const uchar* getEngine(unsigned long &sz)
{
	static unsigned char buffer1[128*1024];
	basics::CZlib zlib;

	sz = sizeof(buffer1);
	if( 0==zlib.decode(buffer1, sz, engine, sizeof(engine)) )
	{
		printf("loading engine...(%lu)\n", sz);
		return buffer1;
	}
	return NULL;
}

void buildEngine()
{
	static unsigned char buffer1[128*1024];
	static unsigned char buffer2[128*1024];
	FILE *fp = fopen("oldeascript.cgt", "rb");
	ulong sz=fread(buffer1, 1, sizeof(buffer1), fp);

	ulong i, sz2=sizeof(buffer2);
	basics::CZlib zlib;
	zlib.encode(buffer2, sz2, buffer1, sz);

	for(i=0; i<sz2; ++i)
	{
		if(i%8==0) printf("\n");
		printf("0x%02X, ", buffer2[i]);
	}
	exit(0);
}

