// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _OLDEASCRIPTCONV_
#define _OLDEASCRIPTCONV_

#include "basefile.h"
#include "baseparser.h"
#include "scriptengine.h"



struct oldeaparserstorage : public parserstorage
{
	oldeaparserstorage()
	{}
	virtual ~oldeaparserstorage()
	{}

	virtual bool getEngine(const unsigned char*& buf, unsigned long& sz);
};


struct oldeaprinter : public printer
{
public:

	oldeaprinter()
	{}
	virtual ~oldeaprinter()
	{}

	void print_newnpchead(const char*name, const char*map=NULL, int x=0, int y=0, int d=0, int s=0, int tx=0, int ty=0);
	void print_oldscripthead(const char* str);
	void print_oldminscripthead( const char* str );
	void print_oldfunctionhead( const char* str );
	void print_oldmonsterhead( const char* str );
	void print_oldwarphead( const char* str );
	void print_oldmapflaghead( const char* str );
	void print_oldduphead( const char* str );
	void print_oldshophead( const char* str );
	void print_oldmobdbhead( const char* str );
	void print_oldmobdbheadea( const char* str );
	int print_olditemdbhead( const char* str );
	int print_olditemdbheadea( const char* str );

	
	virtual bool print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent);


	bool transform_print_unprocessed(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool transform_cmd_set(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool transform_callstm(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool transform_identifier(basics::CParser_CommentStore& parser, int rtpos, short parent);
};




class oldeaParser : public basics::CFileProcessor
{
	basics::CParser_CommentStore*	parser;
	int								option;
	mutable oldeaprinter			prn;

public:
	oldeaParser(basics::CParser_CommentStore* p, int o) : parser(p), option(o)
	{
		if(option & OPT_LOGGING)
			prn.open_log();
	}
	virtual bool process(const char*name) const;
};


///////////////////////////////////////////////////////////////////////////////
#endif//_OLDEASCRIPTCONV_


