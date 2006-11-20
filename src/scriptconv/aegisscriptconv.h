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
	basics::string<>	cAegisName;	///< name of currently processed npc
	basics::string<>	cEAName;	///< name of currently processed npc (ea format)
	basics::string<>	cMapName;	///< map name of currently processed npc
	bool cHasTmpstr;				///< has an instance of tempstr
	bool cHasTmpval;				///< has an instance of tempval
	bool cHasDefault;				///< has a default case

public:
	aegisprinter()
	{}
	virtual ~aegisprinter()
	{
		if(logfile) fclose(logfile);
	}
	virtual bool print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool print_callstm(basics::CParser_CommentStore& parser, int namepos, int parapos);
	bool print_varray(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool print_npcvarray(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool print_opand(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool print_idstring(basics::string<>& origstr);
	bool print_identifier(basics::CParser_CommentStore& parser, int rtpos, short parent);
	bool print_npchead(const char* head, const basics::string<>& name, const basics::string<>& map, const basics::string<>& xpos, const basics::string<>& ypos, const char* dir, const char* sprite, const basics::string<>& touchx, const basics::string<>& touchy);
};




class aegisParser : public basics::CFileProcessor
{
	basics::CParser_CommentStore*	parser;
	int								option;
	mutable aegisprinter			prn;

public:
	aegisParser(basics::CParser_CommentStore* p, int o) : parser(p), option(o)
	{
		if(option & OPT_LOGGING)
			prn.open_log();	
	}
	virtual bool process(const char*name) const;
};

///////////////////////////////////////////////////////////////////////////////
#endif//_AEGISCONV_

