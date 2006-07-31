#include "timer.h"
#include "malloc.h"
#include "version.h"
#include "nullpo.h"
#include "showmsg.h"

#include "npc.h"
#include "status.h"
#include "chat.h"
#include "script.h"
#include "battle.h"


#ifdef WITH_PCRE

#include <pcre.h>

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

/* Structure containing all info associated with a single pattern
   block */

struct pcrematch_entry
{
    struct pcrematch_entry *next_;
    char *pattern_;
    pcre *pcre_;
    pcre_extra *pcre_extra_;
    char *label_;

	pcrematch_entry() : 
		next_(NULL),
		pattern_(NULL),
		pcre_(NULL),
		pcre_extra_(NULL),
		label_(NULL)
	{}
};

/* A set of patterns that can be activated and deactived with a single
   command */

struct pcrematch_set
{
    struct pcrematch_set *next_, *prev_;
    struct pcrematch_entry *head_;
    int setid_;


	pcrematch_set(int id) : 
		next_(NULL), prev_(NULL),head_(NULL),
		setid_(id)
	{}

};

/* 
 * Entire data structure hung off a NPC
 *
 * The reason I have done it this way (a void * in npc_data and then
 * this) was to reduce the number of patches that needed to be applied
 * to a ragnarok distribution to bring this code online.  I
 * also wanted people to be able to grab this one file to get updates
 * without having to do a large number of changes.
 */

struct npc_parse
{
    struct pcrematch_set *active_;
    struct pcrematch_set *inactive_;

	npc_parse() :
		active_(NULL), inactive_(NULL)
	{}
};


/**
 * delete everythign associated with a entry
 *
 * This does NOT do the list management
 */

void finalize_pcrematch_entry(struct pcrematch_entry *e)
{
    if(e->pcre_)		{ pcre_free(e->pcre_); e->pcre_=NULL; }
	if(e->pcre_extra_)	{ pcre_free(e->pcre_extra_); e->pcre_extra_=NULL; }
	if(e->pattern_)		{ free(e->pattern_); e->pattern_=NULL; }
	if(e->label_)		{ free(e->label_); e->label_=NULL; }
}

/**
 * Lookup (and possibly create) a new set of patterns by the set id
 */
struct pcrematch_set * lookup_pcreset(struct npc_data *nd,int setid) 
{
    struct pcrematch_set *pcreset;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL) 
        nd->chatdb = npcParse = new struct npc_parse;

    pcreset = npcParse->active_;

    while (pcreset != NULL) 
	{
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }
    if (pcreset == NULL) 
        pcreset = npcParse->inactive_;

    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }

    if (pcreset == NULL) 
	{
        pcreset = new struct pcrematch_set(setid);
        pcreset->next_ = npcParse->inactive_;
        if (pcreset->next_ != NULL)
            pcreset->next_->prev_ = pcreset;
        pcreset->prev_ = 0;
        npcParse->inactive_ = pcreset;
    }
    return pcreset;
}

/**
 * activate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */

void activate_pcreset(struct npc_data *nd,int setid)
{
    struct pcrematch_set *pcreset;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;

    if (npcParse == NULL) 
        return; // Nothing to activate...

    pcreset = npcParse->inactive_;
    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }
    if (pcreset == NULL)
        return; // not in inactive list

    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset->prev_;
    if (pcreset->prev_ != NULL)
        pcreset->prev_->next_ = pcreset->next_;
    else 
        npcParse->inactive_ = pcreset->next_;

    pcreset->prev_ = NULL;
    pcreset->next_ = npcParse->active_;
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset;
    npcParse->active_ = pcreset;
}

/**
 * deactivate a set of patterns.
 *
 * if the setid does not exist, this will silently return
 */

void deactivate_pcreset(struct npc_data *nd,int setid)
{
    struct pcrematch_set *pcreset;
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL) 
        return; // Nothing to deactivate...

    if (setid == -1) {
		while(npcParse->active_ != NULL)
			deactivate_pcreset(nd, npcParse->active_->setid_);
		return;
    }
    pcreset = npcParse->active_;
    while (pcreset != NULL) {
        if (pcreset->setid_ == setid)
            break;
        pcreset = pcreset->next_;
    }
    if (pcreset == NULL)
        return; // not in active list
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset->prev_;
    if (pcreset->prev_ != NULL)
        pcreset->prev_->next_ = pcreset->next_;
    else 
        npcParse->active_ = pcreset->next_;

    pcreset->prev_ = NULL;
    pcreset->next_ = npcParse->inactive_;
    if (pcreset->next_ != NULL)
        pcreset->next_->prev_ = pcreset;
    npcParse->inactive_ = pcreset;
}

/**
 * delete a set of patterns.
 */
void delete_pcreset(struct npc_data *nd,int setid)
{
	int active = 1;
	struct pcrematch_set *pcreset;
	struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
	if (npcParse == NULL)
		return; // Nothing to deactivate...
	
	pcreset = npcParse->active_;
	npcParse->active_ = NULL;
	while (pcreset != NULL)
	{
		if (pcreset->setid_ == setid)
			break;
		pcreset = pcreset->next_;
	}
	if (pcreset == NULL)
	{
		active = 0;
		pcreset = npcParse->inactive_;
		while (pcreset != NULL) {
			if (pcreset->setid_ == setid)
				break;
			pcreset = pcreset->next_;
		}
	}
	if (pcreset == NULL)
		return;
	
	if (pcreset->next_ != NULL)
		pcreset->next_->prev_ = pcreset->prev_;
	
	if (pcreset->prev_ != NULL)
		pcreset->prev_->next_ = pcreset->next_;
	else // we have been the first
	{	// also update the root
		if(active == 1)
			npcParse->active_ = pcreset->next_;
		else
			npcParse->inactive_ = pcreset->next_;
	}

	pcreset->prev_ = NULL;
	pcreset->next_ = NULL;
	
	while (pcreset->head_)
	{
		struct pcrematch_entry *n = pcreset->head_->next_;
		finalize_pcrematch_entry(pcreset->head_);
		pcreset->head_ = n;
	}
	delete pcreset;
}

/**
 * create a new pattern entry 
 */
struct pcrematch_entry *create_pcrematch_entry(struct pcrematch_set * set)
{
	if(!set)
		return NULL;

    struct pcrematch_entry * e =   new struct pcrematch_entry;
    struct pcrematch_entry * last = set->head_;

    // Normally we would have just stuck it at the end of the list but
    // this doesn't sink up with peoples usage pattern.  They wanted
    // the items defined first to have a higher priority then the
    // items defined later.. as a result, we have to do some work up
    // front..

    /*  if we are the first pattern, stick us at the end */
    if (last == NULL) {
        set->head_ = e;
        return e;
    }

    /* Look for the last entry */
    while (last->next_ != NULL)
        last = last->next_;

    last->next_ = e;
    e->next_ = NULL;

    return e;
}

/**
 * define/compile a new pattern
 */

void npc_chat_def_pattern(struct npc_data *nd, int setid, 
    const char *pattern, const char *label)
{
    const char *err;
    int erroff;

    struct pcrematch_set * s = lookup_pcreset(nd, setid);
    struct pcrematch_entry *e = create_pcrematch_entry(s);
    e->pattern_ = strdup(pattern);
    e->label_ = strdup(label);
    e->pcre_ = pcre_compile(pattern, PCRE_CASELESS, &err, &erroff, NULL);
    e->pcre_extra_ = pcre_study(e->pcre_, 0, &err);
}

/**
 * Delete everything associated with a NPC concerning the pattern
 * matching code 
 *
 * this could be more efficent but.. how often do you do this?
 */
void npc_chat_finalize(struct npc_data *nd)
{
    struct npc_parse *npcParse = (struct npc_parse *) nd->chatdb;
    if (npcParse == NULL)
        return;
	nd->chatdb=NULL;

    while(npcParse->active_)
      delete_pcreset(nd, npcParse->active_->setid_);

    while(npcParse->inactive_)
      delete_pcreset(nd, npcParse->inactive_->setid_);
	delete npcParse;
	
}

/**
 * Handler called whenever a global message is spoken in a NPC's area
 */
int CNpcChat::process(block_list &bl) const
{
    struct npc_data &nd = (struct npc_data &)bl;
    struct npc_parse *npcParse = (struct npc_parse *)nd.chatdb;
    struct pcrematch_set *pcreset;

    // Not interested in anything you might have to say...
    if (npcParse == NULL || npcParse->active_ == NULL)
        return 0;

    // grab the active list
    pcreset = npcParse->active_;

    // interate across all active sets
    while (pcreset != NULL)
	{
        struct pcrematch_entry *e = pcreset->head_;
        // interate across all patterns in that set
        while (e != NULL)
		{
			if( nd.u.scr.ref && nd.u.scr.ref->script )
			{
				size_t pos = nd.u.scr.ref->get_labelpos(e->label_);
				if( pos==0 )
				{	// unable to find label... do something..
					ShowMessage("Unable to find label: %s", e->label_);
				}
				else
				{
					int offsets[30];
					char buf[255];

					// perform pattern match
					int r = pcre_exec(e->pcre_, e->pcre_extra_, msg, len, 0, 
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
						CScriptEngine::run(nd.u.scr.ref->script, pos,sd.block_list::id,nd.block_list::id);
						// and return
						return 0;
					}
					// otherwise no match
				}
			}
			// next pattern in set
            e = e->next_;
        }
		// next set
        pcreset = pcreset->next_;
    }
    return 0;
}

// Various script builtins used to support these functions
int buildin_defpattern(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    const char *pattern=st.GetString(st[3]);
    const char *label=st.GetString(st[4]);
    struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	if(nd) 
		npc_chat_def_pattern(nd, setid, pattern, label);
    return 0;
}
int buildin_activatepset(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	if(nd)
		activate_pcreset(nd, setid);

    return 0;
}
int buildin_deactivatepset(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	if(nd)
		deactivate_pcreset(nd, setid);

    return 0;
}
int buildin_deletepset(CScriptEngine &st)
{
    int setid=st.GetInt(st[2]);
    struct npc_data *nd=(struct npc_data *)map_id2bl(st.oid);
	if(nd)
		delete_pcreset(nd, setid);
    return 0;
}

#else

void npc_chat_finalize(struct npc_data *nd)			{}

//int npc_chat_sub(struct block_list &bl, va_list &ap){ return 0; }
int CNpcChat::process(block_list &bl) const			{ return 0; }
int buildin_defpattern(CScriptEngine &st)			{ return 0; }
int buildin_activatepset(CScriptEngine &st)			{ return 0; }
int buildin_deactivatepset(CScriptEngine &st)		{ return 0; }
int buildin_deletepset(CScriptEngine &st)			{ return 0; }


#endif
