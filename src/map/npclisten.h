// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef __NPCLISTEN__
#define __NPCLISTEN__


#ifdef WITH_PCRE

#include <pcre.h>

///////////////////////////////////////////////////////////////////////////////
// library includes
#ifdef _MSC_VER
#pragma comment(lib, "pcre.lib")
#endif


///////////////////////////////////////////////////////////////////////////////
/// info associated with a single pattern.
struct pcrematch_entry
{
    pcrematch_entry *cNext;
    char *cPattern;
	char *cLabel;
    pcre *cPcre;
    pcre_extra *cExtra;

	/////////////////////////////////////////////////////////////////
	/// constructor.
	pcrematch_entry(const char *pattern, const char *label);
	/////////////////////////////////////////////////////////////////
	/// destructor.
	~pcrematch_entry();
};

///////////////////////////////////////////////////////////////////////////////
/// set of patterns.
/// doubled linked list
struct pcrematch_set
{
	pcrematch_set *cPrev;
    pcrematch_set *cNext;
    pcrematch_entry *cEntry;
    int cSetID;

	/////////////////////////////////////////////////////////////////
	/// constructor.
	pcrematch_set(int id) : 
		cPrev(NULL),
		cNext(NULL), 
		cEntry(NULL),
		cSetID(id)
	{}
	/////////////////////////////////////////////////////////////////
	/// destructor.
	~pcrematch_set();


	///////////////////////////////////////////////////////////////////////////////
	// enqueue to a doubled linked list
	void pcrematch_set::link(pcrematch_set*& root);
	///////////////////////////////////////////////////////////////////////////////
	// dequeue from a doubled linked list
	void pcrematch_set::unlink(pcrematch_set*& root);
	///////////////////////////////////////////////////////////////////////////////
	// add a match entry
	void append(pcrematch_entry *e);
};

///////////////////////////////////////////////////////////////////////////////
/// access point of an NPC.
/// implements two roots points for active and inactive list
/// -> possibly reduce some efford with having
/// one single-linked list with a boolean active/inactive member
struct npc_parse
{
    struct pcrematch_set *cActive;
    struct pcrematch_set *cInactive;

	/////////////////////////////////////////////////////////////////
	/// constructor.
	npc_parse() :
		cActive(NULL), cInactive(NULL)
	{}
	/////////////////////////////////////////////////////////////////
	/// destructor.
	~npc_parse();

	/////////////////////////////////////////////////////////////////
	/// lookup a set.
	pcrematch_set& lookup(int sid);
};

#else

struct npc_parse
{};

#endif//WITH_PCRE


void npclisten_finalize(npcscript_data *nd);

class CNpcChat : public CMapProcessor
{
    const char *msg;
    size_t len;
    map_session_data &sd;
public:
	CNpcChat(const char *m, map_session_data &s)
		: msg(m), len(m?strlen(m):0), sd(s)
	{}
	~CNpcChat()	{}
	virtual int process(block_list& bl) const;
};

#endif//__NPCLISTEN__
