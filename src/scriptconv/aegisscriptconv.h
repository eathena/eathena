// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _AEGISCONV_
#define _AEGISCONV_

#include "basefile.h"
#include "baseparser.h"
#include "scriptengine.h"


struct aegisparserstorage : public parserstorage
{
	aegisparserstorage()
	{}
	virtual ~aegisparserstorage()
	{}

	virtual bool getEngine(const unsigned char*& buf, unsigned long& sz);
};


struct aegisprinter : public printer
{
public:

	aegisprinter()
	{}
	virtual ~aegisprinter()
	{}

	virtual bool print_beautified(basics::CParser_CommentStore& parser, int rtpos);
};




class aegisParser : public basics::CFileProcessor
{
	basics::CParser_CommentStore*	parser;
	int								option;
	mutable aegisprinter			prn;

public:
	aegisParser(basics::CParser_CommentStore* p, int o) : parser(p), option(o)
	{}
	virtual bool process(const char*name) const;
};

///////////////////////////////////////////////////////////////////////////////
#endif//_AEGISCONV_

