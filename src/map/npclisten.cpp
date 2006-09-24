// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#include "timer.h"
#include "malloc.h"
#include "version.h"
#include "nullpo.h"
#include "showmsg.h"


#include "npc.h"
#include "npclisten.h"
#include "status.h"
#include "chat.h"
#include "script.h"
#include "battle.h"


#ifdef WITH_PCRE

/**
 *  Written by MouseJstr in a vision... (2/21/2005)
 *
 *  This allows you to make npc listen for spoken text (global
 *  messages) and pattern match against that spoken text using perl
 *  regular expressions.
 *
 *  Please feel free to copy this code into your own personal ragnarok
 *  servers or distributions but please leave my name.  Also, please
 *  wait until I've put it into the main eA branch which means I
 *  believe it is ready for distribution.
 *
 *  So, how do people use this?
 *
 *  The first and most important function is defpattern
 *
 *    defpattern 1, "[^:]+: (.*) loves (.*)", "label";
 *
 *  this defines a new pattern in set 1 using perl syntax 
 *    (http://www.troubleshooters.com/codecorn/littperl/perlreg.htm)
 *  and tells it to jump to the supplied label when the pattern
 *  is matched.
 *
 *  each of the matched Groups will result in a variable being
 *  set ($p1$ through $p9$  with $p0$ being the entire string)
 *  before the script gets executed.
 *
 *    activatepset 1;
 * 
 *  This activates a set of patterns.. You can have many pattern
 *  sets defined and many active all at once.  This feature allows
 *  you to set up "conversations" and ever changing expectations of
 *  the pattern matcher
 *
 *    deactivatepset 1;
 *
 *  turns off a pattern set;
 *
 *    deactivatepset -1;
 *
 *  turns off ALL pattern sets;
 *
 *    deletepset 1;
 *
 *  deletes a pset
 */


///////////////////////////////////////////////////////////////////////////////
/// constructor.
pcrematch_entry::pcrematch_entry(const char *pattern, const char *label) : 
	cNext(NULL),
	cPattern(strdup(pattern)),
	cLabel(strdup(label)),
	cPcre(NULL),
	cExtra(NULL)
{
    const char *err;
    int erroff;
    this->cPcre  = pcre_compile(pattern, PCRE_CASELESS, &err, &erroff, NULL);
    this->cExtra = pcre_study(this->cPcre, 0, &err);
}


///////////////////////////////////////////////////////////////////////////////
/// destructor.
/// delete everythign associated with a entry
pcrematch_entry::~pcrematch_entry()
{
    if(this->cPcre)		{ pcre_free(this->cPcre); this->cPcre=NULL; }
	if(this->cExtra)	{ pcre_free(this->cExtra); this->cExtra=NULL; }
	if(this->cPattern)	{ free(this->cPattern); this->cPattern=NULL; }
	if(this->cLabel)	{ free(this->cLabel); this->cLabel=NULL; }

	if(this->cNext)
	{
		delete this->cNext;
		this->cNext = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// destructor.
pcrematch_set::~pcrematch_set()
{
    pcrematch_set *a = this->cNext;

	// unlink
	if(this->cNext)
		this->cNext->cPrev = this->cPrev;
	if(this->cPrev)
		this->cPrev->cNext = this->cNext;
	this->cNext = this->cPrev = NULL;

	// be sure to have a start node
	while(a->cPrev)
		a = a->cPrev;
	// and delete the whole chain
	delete a;

	// delete the entry
	if(this->cEntry)
	{
		delete this->cEntry;
		this->cEntry = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////
// enqueue to a doubled linked list
void pcrematch_set::link(pcrematch_set*& root)
{
	this->cPrev = NULL;
	this->cNext = root;
	if( root )
		this->cNext->cPrev = this;
	root = this;
}

///////////////////////////////////////////////////////////////////////////////
// dequeue from a doubled linked list
void pcrematch_set::unlink(pcrematch_set*& root)
{
	if( this->cNext )
		this->cNext->cPrev = this->cPrev;
	if( this->cPrev)
		this->cPrev->cNext = this->cNext;
	else 
		root = this->cNext;
	this->cPrev = NULL;
	this->cNext = NULL;
}


void pcrematch_set::append(pcrematch_entry *e)
{
	if(e)
	{
		if( this->cEntry == NULL )
		{	// first pattern
			this->cEntry = e;
		}
		else
		{	// add to chain
			pcrematch_entry * last = this->cEntry;
			// look for the last entry
			while(last->cNext) last = last->cNext;
			last->cNext = e;
		}
		e->cNext=NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// destructor.
/// deletes the whole pattern match
npc_parse::~npc_parse()
{
	if(this->cActive)
	{
		delete this->cActive;
		this->cActive = NULL;
	}
	if(this->cInactive)
	{
		delete this->cInactive;
		this->cInactive = NULL;
	}
}



///////////////////////////////////////////////////////////////////////////////
/// lookup a set. and possibly create a new set of patterns by the set id
pcrematch_set& npc_parse::lookup(int setid) 
{
	pcrematch_set *set;
	// search in active sets
	for(set=this->cActive; set; set=set->cNext)
	{
		if( set->cSetID == setid )
			return *set;
	}
	// search in inactive sets
	for(set=this->cInactive; set; set=set->cNext)
	{
		if( set->cSetID == setid )
			return *set;
	}
	// create new and add to inactive sets
	set = new pcrematch_set(setid);

	set->cNext=this->cInactive;
	if(set->cNext)
		set->cNext->cPrev = set;
	this->cInactive = set;

    return *set;
}














///////////////////////////////////////////////////////////////////////////////
/// Handler called whenever a global message is spoken in a NPC's area
int CNpcChat::process(block_list &bl) const
{
    npcscript_data *pnd = bl.get_script();
    npc_parse *npcParse = (pnd)?pnd->listendb:NULL;

    // Not interested in anything you might have to say...
    if(npcParse == NULL || npcParse->cActive == NULL)
        return 0;

	npcscript_data &nd = *pnd;
    // grab the active list
    pcrematch_set *pcreset = npcParse->cActive;
	pcrematch_entry *e;

    for(; pcreset; pcreset=pcreset->cNext)	// interate across all active sets
    for(e=pcreset->cEntry; e; e=e->cNext)	// interate across all patterns in that set
	{
		if( nd.ref && nd.ref->script )
		{
			size_t pos = nd.ref->get_labelpos(e->cLabel);
			if( pos==0 )
			{	// unable to find label... do something..
				ShowWarning("Unable to find label: %s", e->cLabel);
			}
			else
			{
				int offsets[30];
				char buf[256];

				// perform pattern match
				int r = pcre_exec(e->cPcre, e->cExtra, msg, len, 0, 
					0, offsets, sizeof(offsets) / sizeof(offsets[0]));

				// PCRE Documentation:
				// Captured substrings are returned to the caller via a vector of integer offsets 
				// whose address is passed in ovector. The number of elements in the vector is passed 
				// in ovecsize, which must be a non-negative number. 
				// Note: this argument is NOT the size of ovector in bytes.
				// The first two-thirds of the vector is used to pass back captured substrings, 
				// each substring using a pair of integers. The remaining third of the vector is used as 
				// workspace by pcre_exec() while matching capturing subpatterns, and is not available 
				// for passing back information. The length passed in ovecsize should always be a multiple of three. 
				// If it is not, it is rounded down.
				// When a match is successful, information about captured substrings is returned in pairs of integers, 
				// starting at the beginning of ovector, and continuing up to two-thirds of its length at the most. 
				// The first element of a pair is set to the offset of the first character in a substring, 
				// and the second is set to the offset of the first character after the end of a substring. 
				// The first pair, ovector[0] and ovector[1], identify the portion of the subject string matched by 
				// the entire pattern. The next pair is used for the first capturing subpattern, and so on. 
				// The value returned by pcre_exec() is the number of pairs that have been set. 
				// If there are no capturing subpatterns, the return value from a successful match is 1, 
				// indicating that just the first pair of offsets has been set.
				if (r >= 0)
				{
					size_t cnt;

					// clear previous variables
					set_var("$p9$", "");
					set_var("$p8$", "");
					set_var("$p7$", "");
					set_var("$p6$", "");
					set_var("$p5$", "");
					set_var("$p4$", "");
					set_var("$p3$", "");
					set_var("$p2$", "");
					set_var("$p1$", "");
					set_var("$p0$", "");

					// save out the matched strings
					switch (r)
					{
					case 10:
						cnt = (offsets[19]>offsets[18])?( (offsets[19]<offsets[18]+(int)sizeof(buf))?(offsets[19]-offsets[18]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[18], cnt);
						buf[cnt] = '\0';
						set_var("$p9$", buf);
					case 9:
						cnt = (offsets[17]>offsets[16])?( (offsets[17]<offsets[16]+(int)sizeof(buf))?(offsets[17]-offsets[16]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[16], cnt);
						buf[cnt] = '\0';
						set_var("$p8$", buf);
					case 8:
						cnt = (offsets[15]>offsets[14])?( (offsets[15]<offsets[14]+(int)sizeof(buf))?(offsets[15]-offsets[14]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[14], cnt);
						buf[cnt] = '\0';
						set_var("$p7$", buf);
					case 7:
						cnt = (offsets[13]>offsets[12])?( (offsets[13]<offsets[12]+(int)sizeof(buf))?(offsets[13]-offsets[12]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[12], cnt);
						buf[cnt] = '\0';
						set_var("$p6$", buf);
					case 6:
						cnt = (offsets[11]>offsets[10])?( (offsets[11]<offsets[10]+(int)sizeof(buf))?(offsets[11]-offsets[10]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[10], cnt);
						buf[cnt] = '\0';
						set_var("$p5$", buf);
					case 5:
						cnt = (offsets[ 9]>offsets[ 8])?( (offsets[ 9]<offsets[ 8]+(int)sizeof(buf))?(offsets[ 9]-offsets[ 8]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[ 8], cnt);
						buf[cnt] = '\0';
						set_var("$p4$", buf);
					case 4:
						cnt = (offsets[ 7]>offsets[ 6])?( (offsets[ 7]<offsets[ 6]+(int)sizeof(buf))?(offsets[ 7]-offsets[ 6]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[ 6], cnt);
						buf[cnt] = '\0';
						set_var("$p3$", buf);
					case 3:
						cnt = (offsets[ 5]>offsets[ 4])?( (offsets[ 5]<offsets[ 4]+(int)sizeof(buf))?(offsets[ 5]-offsets[ 4]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[ 4], cnt);
						buf[cnt] = '\0';

						set_var("$p2$", buf);
					case 2:
						cnt = (offsets[ 3]>offsets[ 2])?( (offsets[ 3]<offsets[ 2]+(int)sizeof(buf))?(offsets[ 3]-offsets[ 2]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[ 2], cnt);
						buf[cnt] = '\0';

						set_var("$p1$", buf);
					case 1:
						cnt = (offsets[ 1]>offsets[ 0])?( (offsets[ 1]<offsets[ 0]+(int)sizeof(buf))?(offsets[ 1]-offsets[ 0]):sizeof(buf)):0;
						memcpy(buf, msg+offsets[ 0], cnt);
						buf[cnt] = '\0';
						set_var("$p0$", buf);
					}

					// run the npc script
					CScriptEngine::run(nd.ref->script, pos,sd.block_list::id,nd.block_list::id);
					// and return
					return 0;
				}
				// otherwise no match
			}
		}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// define/compile a new pattern
int buildin_defpattern(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    const char *pattern=st.GetString(st[3]);
    const char *label=st.GetString(st[4]);
    npcscript_data *nd= npcscript_data::from_blid(st.oid);
	if(nd) 
	{
		// create a new pattern storage if not exists
		if( nd->listendb == NULL ) 
			nd->listendb = new npc_parse;

		// append a new entry
		nd->listendb->lookup(setid).append( new pcrematch_entry(pattern, label) );
	}
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
/// activate a set of patterns.
/// if the setid does not exist, this will silently return
int buildin_activatepset(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    npcscript_data *nd= npcscript_data::from_blid(st.oid);
	if(nd && nd->listendb && nd->listendb->cInactive)
	{
		if( setid == -1 )
		{	// activate all
			if(nd->listendb->cActive)
			{	
				pcrematch_set *set = nd->listendb->cActive;
				for(; set->cNext; set=set->cNext);

				set->cNext = nd->listendb->cInactive;
				nd->listendb->cInactive->cPrev = set;
			}
			else
				nd->listendb->cActive = nd->listendb->cInactive;
			nd->listendb->cInactive = NULL;
		}
		else
		{
			pcrematch_set *set= nd->listendb->cInactive;
			for( ; set; set = set->cNext)
			{
				if(set->cSetID == setid)
				{
					// dequeue from inactive
					set->unlink(nd->listendb->cInactive);
					// enqueue to active
					set->link(nd->listendb->cActive);
					
					break;
				}
			}
		}
	}
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// deactivate a set of patterns.
/// if the setid does not exist, this will silently return
int buildin_deactivatepset(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    npcscript_data *nd=npcscript_data::from_blid(st.oid);
	if(nd && nd->listendb && nd->listendb->cActive) 
	{
		if( setid == -1 )
		{	// deactivate all
			if(nd->listendb->cInactive)
			{	
				pcrematch_set *set = nd->listendb->cInactive;
				for(; set->cNext; set=set->cNext);

				set->cNext = nd->listendb->cActive;
				nd->listendb->cActive->cPrev = set;
			}
			else
				nd->listendb->cInactive = nd->listendb->cActive;
			nd->listendb->cActive = NULL;
		}
		else
		{
			pcrematch_set *set = nd->listendb->cActive;
			for( ; set; set = set->cNext)
			{
				if(set->cSetID == setid)
				{
					// dequeue from active
					set->unlink(nd->listendb->cActive);
					// enqueue to inactive
					set->link(nd->listendb->cInactive);

					break;
				}
			}
		}
	}
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// delete a set of patterns.
int buildin_deletepset(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    npcscript_data *nd= npcscript_data::from_blid(st.oid);
	if(nd && nd->listendb)
	{
		pcrematch_set *set;
			
		for(set=nd->listendb->cActive; set; set=set->cNext)
		{
			if (set->cSetID == setid)
			{
				set->unlink(nd->listendb->cActive);
				delete set;
				return 0;
			}
		}
		for(set=nd->listendb->cInactive; set; set=set->cNext)
		{
			if (set->cSetID == setid)
			{
				set->unlink(nd->listendb->cInactive);
				delete set;
				return 0;
			}
		}
	}
    return 0;
}

#else

int CNpcChat::process(block_list &bl) const			{ return 0; }
int buildin_defpattern(CScriptEngine &st)			{ return 0; }
int buildin_activatepset(CScriptEngine &st)			{ return 0; }
int buildin_deactivatepset(CScriptEngine &st)		{ return 0; }
int buildin_deletepset(CScriptEngine &st)			{ return 0; }


#endif
