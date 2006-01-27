#include "baseregex.h"
#include "basesafeptr.h"

// code derived from implementation of Guy Gascoigne - Piggford (ggp@bigfoot.com)
// which is a port of originally code written by Henry Spencer
// the original author wants his copyright notice intact, so I follow his wish.
///////////////////////////////////////////////////////////////////////////////
// original notice:
///////////////////////////////////////////////////////////////////////////////
// regcomp and regexec -- regsub and regerror are elsewhere
// @(#)regexp.c	1.3 of 18 April 87
//
//	Copyright (c) 1986 by University of Toronto.
//	Written by Henry Spencer.  Not derived from licensed software.
//
//	Permission is granted to anyone to use this software for any
//	purpose on any computer system, and to redistribute it freely,
//	subject to the following restrictions:
//
//	1. The author is not responsible for the consequences of use of
//		this software, no matter how awful, even if they arise
//		from defects in it.
//
//	2. The origin of this software must not be misrepresented, either
//		by explicit claim or by omission.
//
//	3. Altered versions must be plainly marked as such, and must not
//		be misrepresented as being the original software.
// *** THIS IS AN ALTERED VERSION.  It was altered by John Gilmore,
// *** hoptoad!gnu, on 27 Dec 1986, to add \< and \> for word-matching
// *** as in BSD grep and ex.
// *** THIS IS AN ALTERED VERSION.  It was altered by John Gilmore,
// *** hoptoad!gnu, on 28 Dec 1986, to optimize characters quoted with \.
// *** THIS IS AN ALTERED VERSION.  It was altered by James A. Woods,
// *** ames!jaw, on 19 June 1987, to quash a regcomp() redundancy.
// *** THIS IS AN ALTERED VERSION.  It was altered by Geoffrey Noer,
// *** THIS IS AN ALTERED VERSION.  It was altered by Guy Gascoigne - Piggford
// *** guy@wyrdrune.com, on 15 March 1998, porting it to C++ and converting
// *** it to be the engine for the Regexp class
// *** THIS IS AN ALTERED VERSION.  It was altered by Shinomori Aoshi
// *** in March 2000, restructuring the classes and adding some more stuff.
// *** constantly keep up with changing environments.
//
//  4. This notice must not be removed or altered.
//
// Beware that some of this code is subtly aware of the way operator
// precedence is structured in regular expressions.  Serious changes in
// regular-expression syntax might require a total rethink.
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// The "internal use only" fields in the programm are present to pass info from
// compile to execute that permits the execute phase to run lots faster on
// simple cases.  They are:
//
// cStartChar	char that must begin a match; '\0' if none obvious
// cAnchor		is the match anchored (at beginning-of-line only)?
// cMust		offset to the string inside the program that match must include, or 0
// cMustLen		length of cMust string
//
// cStartChar and cAnchor permit very fast decisions on suitable starting
// points for a match, cutting down the work a lot.  cMust permits fast
// rejection of lines that cannot possibly match.  The cMust tests are
// costly enough that execution supplies a cMust only if the regular expression
// contains something potentially expensive (at present, the only
// such thing detected is * or + at the start of the r.e., which can
// involve a lot of backup).  cMustLen is supplied because the test in
// needs it and on compiling it is computed anyway.

// Structure for the program.  This is essentially a linear encoding
// of a nondeterministic finite-state machine (aka syntax charts or
// "railroad normal form" in parsing technology).  Each node is an opcode
// plus a "next" pointer, possibly plus an operand.  "Next" pointers of
// all nodes except BRANCH implement concatenation; a "next" pointer with
// a BRANCH on both ends of it is connecting two alternatives.  (Here we
// have one of the subtle syntax dependencies: an individual BRANCH (as
// opposed to a collection of them) is never concatenated with anything
// because of operator precedence.)  The operand of some types of node is
// a literal string; for others, it is a node leading into a sub-FSM.  In
// particular, the operand of a BRANCH node is the first node of the
// branch.  (NB this is *not* a tree structure: the tail of the branch
// connects to the thing following the set of BRANCHes.)  The opcodes
// are:

enum {
//  definition		number		opnd?			meaning
	REGEX_END		=	0,		//	no			End of program.
	REGEX_BOL		=	1,		//	no			Match beginning of line.
	REGEX_EOL		=	2,		//	no			Match end of line.
	REGEX_ANY		=	3,		//	no			Match any character.
	REGEX_ANYOF		=	4,		//	str	+\0		Match any of these.
	REGEX_ANYBUT	=	5,		//	str	+\0		Match any but one of these.
	REGEX_BRANCH	=	6,		//	node		Match this, or the next..\&.
	REGEX_BACK		=	7,		//	no			"next" ptr points backward.
	REGEX_EXACTLY	=	8,		//	str	+\0		Match this string.
	REGEX_NOTHING	=	9,		//	no			Match empty string.
	REGEX_STAR		=	10,		//	node		Match this 0 or more times.
	REGEX_PLUS		=	11,		//	node		Match this 1 or more times.
	REGEX_WORDA		=	12,		//	no			Match "" at wordchar, where prev is nonword
	REGEX_WORDZ		=	13,		//	no			Match "" at nonwordchar, where prev is word
	REGEX_OPEN		=	14,		//	level(2byte)	Sub-RE starts here.
	REGEX_CLOSE		=	15,		//	level(2byte)	Analogous to OPEN.

	// posix commands
	REGEX_SPACE		=	16,		//	no			[:space:]
	REGEX_XDIGIT	=	17,		//	no			[:xdigit:]
	REGEX_DIGIT		=	18,		//	no			[:digit:]
	REGEX_UPPER		=	19,		//	no			[:upper:]
	REGEX_LOWER		=	20,		//	no			[:lower:]
	REGEX_RWORD		=	21,		//	no			[:word:]
	REGEX_GRAPH		=	22,		//	no			[:graph:]
	REGEX_PRINT		=	23,		//	no			[:print:]
	REGEX_PUNCT		=	24,		//	no			[:punct:]
	REGEX_CNTRL		=	25		//	no			[:cntrl:]

};

///////////////////////////////////////////////////////////////////////////////
// Opcode notes:
//
// REGEX_BRANCH	The set of branches constituting a single choice are hooked
//		together with their "next" pointers, since precedence prevents
//		anything being concatenated to any individual branch.  The
//		"next" pointer of the last BRANCH in a choice points to the
//		thing following the whole choice.  This is also where the
//		final "next" pointer of each individual branch points; each
//		branch starts with the operand node of a BRANCH node.
//
// REGEX_BACK		Normal "next" pointers all implicitly point forward; BACK
//		exists to make loop structures possible.
//
// REGEX_STAR,REGEX_PLUS	'?', and complex '*' and '+', are implemented 
//		as circular BRANCH structures using BACK.  Simple cases 
//		(one character per match) are implemented with STAR and PLUS 
//		for speed and to minimize recursive plunges.
//
// REGEX_OPEN,REGEX_CLOSE count parenthesise 

// A node is one char of opcode followed by two chars of "next" pointer.
// "Next" pointers are stored as two 8-bit pieces, high order first.  The
// value is a positive offset from the opcode of the node containing it.
// An operand, if any, simply follows the node.  (Note that much of the
// code generation knows about this implicit relationship.)
//
// Using two bytes for the "next" pointer is vast overkill for most things,
// but allows patterns to get big without disasters.

///////////////////////////////////////////////////////////////////////////////
enum
{
	REGERR_SENTINEL_VALUE = 0,
	REGERR_NULLARG = 1, 
	REGERR_CORRUPTED, 
	REGERR_CORRUPTION, 
	REGERR_CORRUPTED_POINTERS,
	REGERR_BAD_REGREPEAT, 
	REGERR_CORRUPTED_OPCODE, 
	REGERR_NULL_TO_REGSUB,
	REGERR_DAMAGED_REGEXP_REGSUB, 
	REGERR_DAMAGED_MATCH_STRING, 
	REGERR_NULL_TO_REGCOMP,
	REGERR_TO_BIG, 
	REGERR_TO_MANY_PAREN, 
	REGERR_UNTERMINATED_PAREN, 
	REGERR_UNMATCHED_PAREN,
	REGERR_INTERNAL_ERROR_JUNK, 
	REGERR_OP_COULD_BE_EMPTY, 
	REGERR_NESTED_OP, 
	REGERR_INVALID_RANGE,
	REGERR_UNMATCHED_BRACE, 
	REGERR_INTERNAL_UNEXPECTED_CHAR, 
	REGERR_OP_FOLLOWS_NOTHING,
	REGERR_TRAILING_ESC, 
	REGERR_INTERNAL_STRSCSPN, 
	REGERR_NO_REGEXP
};


#ifdef DEBUG
int regnarrate = 0;
#endif

///////////////////////////////////////////////////////////////////////////////
// The first byte of the CRegExpWorker internal "program" is actually this magic
// number; the start node begins in the second byte.
const char	REGEX_MAGIC = ((char)'\234');

///////////////////////////////////////////////////////////////////////////////
struct regErr
{
	int m_id;
	const char* m_err;
} errors[] = {
	{ REGERR_NULLARG,					( "NULL argument to regexec" ) },
	{ REGERR_CORRUPTED,					( "corrupted CRegExpWorker" ) },
	{ REGERR_CORRUPTION,				( "CRegExpWorker corruption" ) },
	{ REGERR_CORRUPTED_POINTERS,		( "corrupted pointers" ) },
	{ REGERR_BAD_REGREPEAT,				( "internal error: bad call of regrepeat" ) },
	{ REGERR_CORRUPTED_OPCODE,			( "corrupted opcode" ) },
	{ REGERR_NULL_TO_REGSUB,			( "NULL parm to regsub" ) },
	{ REGERR_DAMAGED_REGEXP_REGSUB,		( "damaged CRegExpWorker fed to regsub" ) },
	{ REGERR_DAMAGED_MATCH_STRING,		( "damaged match string" ) },
	{ REGERR_NULL_TO_REGCOMP,			( "NULL argument to regcomp" ) },
	{ REGERR_TO_BIG,					( "CRegExpWorker too big" ) },
	{ REGERR_TO_MANY_PAREN,				( "too many ()" ) },
	{ REGERR_UNTERMINATED_PAREN,		( "unterminated ()" ) },
	{ REGERR_UNMATCHED_PAREN,			( "unmatched ()" ) },
	{ REGERR_INTERNAL_ERROR_JUNK,		( "internal error: junk on end" ) },
	{ REGERR_OP_COULD_BE_EMPTY,			( "*+ operand could be empty" ) },
	{ REGERR_NESTED_OP,					( "nested *?+" ) },
	{ REGERR_INVALID_RANGE,				( "invalid [] range" ) },
	{ REGERR_UNMATCHED_BRACE,			( "unmatched []" ) },
	{ REGERR_INTERNAL_UNEXPECTED_CHAR,	( "internal error: \\0|) unexpected" ) },
	{ REGERR_OP_FOLLOWS_NOTHING,		( "?+* follows nothing" ) },
	{ REGERR_TRAILING_ESC,				( "trailing \\" ) },
	{ REGERR_INTERNAL_STRSCSPN,			( "internal error: strcspn 0" ) },
	{ REGERR_NO_REGEXP,					( "NULL CRegExpWorker" ) },
	{ REGERR_SENTINEL_VALUE,			( "Unknown error") }							// must be last value
};


///////////////////////////////////////////////////////////////////////////////
// Flags to be passed up and down.

enum {
	REGEX_HASWIDTH	=	01,	// Known never to match null string.
	REGEX_SIMPLE	=	02,	// Simple enough to be STAR/PLUS operand.
	REGEX_SPSTART	=	04,	// Starts with * or +.
	REGEX_WORST		=	0	// Worst case.
};


///////////////////////////////////////////////////////////////////////////////
// a simple loop, is not costly here
const char* CRegExp::CRegErrorHandler::FindErr( int id )
{
	struct regErr * pperr;
	for ( pperr = errors; pperr->m_id != REGERR_SENTINEL_VALUE; pperr++ )
		if( pperr->m_id == id )
			return pperr->m_err;
	return pperr->m_err;
	// since we've fallen off the array, perr->m_id == 0
}


///////////////////////////////////////////////////////////////////////////////
// CRegProgram functions
void CRegExp::CRegProgram::ignoreCase(const char * in, char * out) const
{
	// copy in to out making every top level character a [Aa] set
	bool inRange = false;
	while( *in )
	{
		if( *in == '[' && in[-1] !='\\')
			inRange = true;

		if( *in == ']'  && in[-1] !='\\')
			inRange = false;
		if( !inRange && stringcheck::isalpha( *in ) )
		{
			*out++ = '[';
			*out++ = stringcheck::toupper( *in );
			*out++ = stringcheck::tolower( *in );
			*out++ = ']';
		}
		else
			*out++ = *in;
		in++;
	}
	*out = 0;
}
///////////////////////////////////////////////////////////////////////////////
//
size_t CRegExp::CRegProgram::nextcommand( size_t pos ) const
{
	const ushort offset =(((ushort)((uchar)cProgramm[pos+2]))     ) |
						 (((ushort)((uchar)cProgramm[pos+1])) << 8);
	if(offset == 0)
		return 0;
	return ((this->command(pos) == REGEX_BACK) ? pos-offset : pos+offset);
}
///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::CRegProgram::AddTail(size_t p, size_t val)
{
	size_t scan;
	size_t temp;

	// Find last node. 
	for(scan = p; (temp = this->nextcommand(scan)) > 0; scan = temp)
		continue;

	short value = (short)((this->command(scan) == REGEX_BACK) ? scan - val : val - scan);
	cProgramm[scan+2] = 0xFF & (value     );
	cProgramm[scan+1] = 0xFF & (value >> 8);
}
///////////////////////////////////////////////////////////////////////////////
// AddCmdTail - AddTail on operand of first argument; nop if operandless
void CRegExp::CRegProgram::AddCmdTail(size_t p, size_t val)
{
	// "Operandless" and "op != BRANCH" are synonymous in practice. 
	if( this->command(p) == REGEX_BRANCH)
		AddTail(p+3, val);
}
///////////////////////////////////////////////////////////////////////////////
// ParseExp - regular expression, i.e. main body or parenthesized thing
// Caller must absorb opening parenthesis.
// Combining parenthesis handling with the base level of regular expression
// is a trifle forced, but the need to tie the tails of the branches to what
// follows makes it hard to avoid.
size_t CRegExp::CRegProgram::ParseExp(const char*&parsestr, bool paren, int &parcnt, int &flag)
{
	size_t ret=0;
	size_t br;
	size_t ender;
	int parno = 0;
	int flags;

	flag = REGEX_HASWIDTH;	// Tentatively. 

	if( paren)
	{
		// Make an OPEN node. 
		parno = parcnt;
		parcnt++;
		ret = AddNode(REGEX_OPEN);
		AddValue( (parno    )&0xFF );
		AddValue( (parno>> 8)&0xFF );
	}

	// Pick up the branches, linking them together. 
	br = ParseBranch(parsestr, parcnt, flags);
	if( br == 0)
		return 0;
	if( paren)
		AddTail(ret, br);	// OPEN -> first. 
	else
		ret = br;
	flag &= ~(~flags&REGEX_HASWIDTH);	// Clear bit if bit 0. 
	flag |= flags&REGEX_SPSTART;
	while (*parsestr == '|')
	{
		parsestr++;
		br = ParseBranch(parsestr, parcnt, flags);
		if( br == 0)
			return 0;
		AddTail(ret, br);	// BRANCH -> BRANCH. 
		flag &= ~(~flags&REGEX_HASWIDTH);
		flag |= flags&REGEX_SPSTART;
	}

	// Make a closing node, and hook it on the end. 
	if( paren)
	{
		ender = AddNode(REGEX_CLOSE);
		AddValue( (parno    )&0xFF );
		AddValue( (parno>> 8)&0xFF );
	}
	else
		ender = AddNode(REGEX_END);

	AddTail(ret, ender);

	// Hook the tails of the branches to the closing node. 
	for (br = ret; br != 0; br = this->nextcommand(br))
		AddCmdTail(br, ender);

	// Check for proper termination. 
	if( paren && *parsestr++ != ')')
	{
		regerror( REGERR_UNTERMINATED_PAREN );
		return 0;
	}
	else if( !paren && *parsestr != '\0')
	{
		if( *parsestr == ')')
		{
			regerror( REGERR_UNMATCHED_PAREN );
			return 0;
		}
		else
		{
			regerror( REGERR_INTERNAL_ERROR_JUNK );
			return 0;
		}
		// NOTREACHED 
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
// ParseBranch - one alternative of an | operator
// Implements the concatenation operator.
size_t CRegExp::CRegProgram::ParseBranch(const char*&parsestr, int &parcnt, int &flag)
{
	size_t ret;
	size_t chain;
	size_t latest;
	int flags;
	int c;

	flag = REGEX_WORST;				// Tentatively. 

	ret = AddNode(REGEX_BRANCH);
	chain = 0;
	while ((c = *parsestr) != '\0' && c != '|' && c != ')')
	{
		latest = ParsePiece(parsestr, parcnt, flags);
		if( latest == 0)
			return 0;
		flag |= flags&REGEX_HASWIDTH;
		if( chain == 0)		// First piece. 
			flag |= flags&REGEX_SPSTART;
		else
			AddTail(chain, latest);
		chain = latest;
	}
	if( chain == 0)			// Loop ran zero times. 
		AddNode(REGEX_NOTHING);

	return ret;
}
///////////////////////////////////////////////////////////////////////////////
// ParsePiece - something followed by possible [*+?]
// Note that the branching code sequences used for ? and the general cases
// of * and + are somewhat optimized:  they use the same NOTHING node as
// both the endmarker for their branch list and the body of the last branch.
// It might seem that this node could be dispensed with entirely, but the
// endmarker role is not redundant.
size_t CRegExp::CRegProgram::ParsePiece(const char*&parsestr, int &parcnt, int &flag)
{
	size_t ret;
	char op;
	size_t next;
	int flags;

	ret = ParseAtom(parsestr, parcnt, flags);
	if( ret == 0)
		return 0;

	op = *parsestr;
	if( !isRepeat(op) )
	{
		flag = flags;
		return ret;
	}

	if( !(flags&REGEX_HASWIDTH) && op != ( '?' ))
	{
		regerror( REGERR_OP_COULD_BE_EMPTY );
		return 0;
	}
	switch (op)
	{
		case ( '*' ): flag = REGEX_WORST|REGEX_SPSTART;	break;
		case ( '+' ): flag = REGEX_WORST|REGEX_SPSTART|REGEX_HASWIDTH;	break;
		case ( '?' ): flag = REGEX_WORST; break;
	}

	if( op == ( '*' ) && (flags&REGEX_SIMPLE))
		InsertNode(REGEX_STAR, ret);
	else if( op == ( '*' ))
	{
		// Emit x* as (x&|), where & means "self". 
		InsertNode(REGEX_BRANCH, ret);			// Either x 
		AddCmdTail(ret, AddNode(REGEX_BACK));	// and loop 
		AddCmdTail(ret, ret);					// back 
		AddTail(ret, AddNode(REGEX_BRANCH));	// or 
		AddTail(ret, AddNode(REGEX_NOTHING));	// null. 
	}
	else if( op == ( '+' ) && (flags&REGEX_SIMPLE))
		InsertNode(REGEX_PLUS, ret);
	else if( op == ( '+' ))
	{
		// Emit x+ as x(&|), where & means "self". 
		next = AddNode(REGEX_BRANCH);			// Either 
		AddTail(ret, next);
		AddTail(AddNode(REGEX_BACK), ret);		// loop back 
		AddTail(next, AddNode(REGEX_BRANCH));	// or 
		AddTail(ret, AddNode(REGEX_NOTHING));	// null. 
	}
	else if( op == ( '?' ))
	{
		// Emit x? as (x|) 
		InsertNode(REGEX_BRANCH, ret);			// Either x 
		AddTail(ret, AddNode(REGEX_BRANCH));	// or 
		next = AddNode(REGEX_NOTHING);			// null. 
		AddTail(ret, next);
		AddCmdTail(ret, next);
	}
	parsestr++;
	if( isRepeat(*parsestr) )
	{
		regerror( REGERR_NESTED_OP );
		return 0;
	}

	return ret;
}
///////////////////////////////////////////////////////////////////////////////
// ParseAtom - the lowest level
// Optimization:  gobbles an entire sequence of ordinary characters so that
// it can turn them into a single node, which is smaller to store and
// faster to run.  Backslashed characters are exceptions, each becoming a
// separate node; the code is simpler that way and it's not worth fixing.
size_t CRegExp::CRegProgram::ParseAtom(const char*&parsestr, int &parcnt, int &flag)
{
	size_t ret;
	int flags;

	flag = REGEX_WORST;		// Tentatively. 

	switch ( *parsestr++ )
	{
		// FIXME: these chars only have meaning at beg/end of pat?
		case '^':
			ret = AddNode(REGEX_BOL);
			break;
		case '$':
			ret = AddNode(REGEX_EOL);
			break;
		case '.':
			ret = AddNode(REGEX_ANY);
			flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
			break;
		case '[':
		{
			int range;
			int rangeend;
			int c;

			if( 0==memcmp(parsestr, ":space:", 7) )
			{
				ret = AddNode(REGEX_SPACE);
				parsestr+=7;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":xdigit:", 8) )
			{
				ret = AddNode(REGEX_XDIGIT);
				parsestr+=8;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":digit:", 7) )
			{
				ret = AddNode(REGEX_DIGIT);
				parsestr+=7;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":upper:", 7) )
			{
				ret = AddNode(REGEX_UPPER);
				parsestr+=7;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":lower:", 7) )
			{
				ret = AddNode(REGEX_LOWER);
				parsestr+=7;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":word:", 6) )
			{
				ret = AddNode(REGEX_RWORD);
				parsestr+=6;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":graph:", 7) )
			{
				ret = AddNode(REGEX_GRAPH);
				parsestr+=7;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":print:", 7) )
			{
				ret = AddNode(REGEX_PRINT);
				parsestr+=7;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":punct:", 7) )
			{
				ret = AddNode(REGEX_PUNCT);
				parsestr+=7;
				c=*parsestr++;
			}
			else if( 0==memcmp(parsestr, ":cntrl:", 7) )
			{
				ret = AddNode(REGEX_CNTRL);
				parsestr+=7;
				c=*parsestr++;
			}
			else
			{
				if( *parsestr == '^')
				{	// Complement of range. 
					ret = AddNode(REGEX_ANYBUT);
					parsestr++;
				}
				else
					ret = AddNode(REGEX_ANYOF);

				if( (c = *parsestr) == ']' || c == '-')
				{
					AddValue(c);
					parsestr++;
				}
				while ((c = *parsestr++ ) != '\0' && c != ']')
				{
					if( c != '-')
						AddValue(c);
					else if( (c = *parsestr) == ']' || c == '\0')
						AddValue('-');
					else
					{
						range = *(parsestr-2);
						rangeend = (char)c;
						if( range > rangeend)
						{
							regerror( REGERR_INVALID_RANGE );
							return 0;
						}
						for (range++; range <= rangeend; range++)
							AddValue(range);
						parsestr++;
					}
				}
				AddValue('\0');
			}
			if( c != ']')
			{
				regerror( REGERR_UNMATCHED_BRACE );
				return 0;
			}
			flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
			break;
		}
		case '(':
			ret = ParseExp(parsestr, true, parcnt, flags);
			if( ret == 0)
				return 0;
			flag |= flags&(REGEX_HASWIDTH|REGEX_SPSTART);
			break;
		case '\0':
		case '|':
		case ')':
			// supposed to be caught earlier 
			regerror( REGERR_INTERNAL_UNEXPECTED_CHAR );
			return 0;
		case '?':
		case '+':
		case '*':
			{
				regerror( REGERR_OP_FOLLOWS_NOTHING );
				return 0;
			}
		case '\\':
			switch (*parsestr++)
			{
				case '\0':
				{
					regerror( REGERR_TRAILING_ESC );
					return 0;
				}
				case '<':
					ret = AddNode(REGEX_WORDA);
					break;
				case '>':
					ret = AddNode(REGEX_WORDZ);
					break;
				case 's':
					ret = AddNode(REGEX_SPACE);
					break;
				case 'd':
					ret = AddNode(REGEX_DIGIT);
					break;
				case 'w':
					ret = AddNode(REGEX_RWORD);
					break;
				/* FIXME: Someday handle \1, \2, ... */
				default:
					/* Handle general quoted chars in exact-match routine */
					goto de_fault;
			}
			break;
	de_fault:
	default:
		// Encode a string of characters to be matched exactly.
		//
		// This is a bit tricky due to quoted chars and due to
		// '*', '+', and '?' taking the SINGLE char previous
		// as their operand.
		//
		// On entry, the char at regparse[-1] is going to go
		// into the string, no matter what it is.  (It could be
		// following a \ if we are entered from the '\' case.)
		// 
		// Basic idea is to pick up a good char in  ch  and
		// examine the next char.  If it's *+? then we twiddle.
		// If it's \ then we frozzle.  If it's other magic char
		// we push  ch  and terminate the string.  If none of the
		// above, we push  ch  on the string and go around again.
		//
		//  regprev  is used to remember where "the current char"
		// starts in the string, if due to a *+? we need to back
		// up and put the current char in a separate, 1-char, string.
		// When  regprev  is NULL,  ch  is the only char in the
		// string; this is used in *+? handling, and in setting
		// flags |= SIMPLE at the end.
		{
			const char *regprev;
			register char ch;

			parsestr--; // Look at cur char
			ret = AddNode(REGEX_EXACTLY);
			for ( regprev = 0 ; ; ) {
				ch = *parsestr++;	// Get current char
				switch (*parsestr)
				{	// look at next one
				default:
					AddValue(ch);	// Add cur to string
					break;
				case '.': case '[': case '(':
				case ')': case '|': case '\n':
				case '$': case '^':
				case '\0':
				// FIXME, $ and ^ should not always be magic
				magic:
					AddValue(ch);	// dump cur char
					goto done;	// and we are done

				case '?': case '+': case '*':
					if( !regprev) 	// If just ch in str,
						goto magic;	// use it
					// End mult-char string one early
					parsestr = regprev; // Back up parse
					goto done;

				case '\\':
					AddValue(ch);	// Cur char OK
					switch( parsestr[1] ){ // Look after '\\' 
					case '\0':
					case '<':
					case '>':
					case 's':
					case 'd':
					case 'w':
					// FIXME: Someday handle \1, \2, ...
						goto done; // Not quoted
					default:
						// Backup point is \, scan	* point is after it.
						regprev = parsestr;
						parsestr++; 
						continue;	// NOT break;
					}
				}
				regprev = parsestr;	// Set backup point
			}
		done:
			AddValue('\0');
			flag |= REGEX_HASWIDTH;
			if( !regprev)		// One char?
				flag |= REGEX_SIMPLE;
		}
		break;
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::CRegProgram::MatchTry(const char* base, const char* str, TArrayDCT<CRegExp::CFinds>& finds) const
{
	size_t i;

	const char* reginput = str;

	if( finds.size()<1 ) finds.resize(1);

	for (i = 1; i<finds.size(); i--)
	{
		finds[i].startp= NULL;
		finds[i].endp  = NULL;
	}
	if( MatchMain(base, str, finds, 1, reginput) )
	{
		finds[0].startp = str;
		finds[0].endp   = reginput;
		return true;
	}
	else
	{
		finds.resize(0);
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////
// MatchMain - main matching routine
// Conceptually the strategy is simple:  check to see whether the current
// node matches, call self recursively to see whether the rest matches,
// and then act accordingly.  In practice we make some effort to avoid
// recursion, in particular by going through "ordinary" nodes (that don't
// need to know whether the rest of the match failed) by a loop instead of
// by recursion.
bool CRegExp::CRegProgram::MatchMain(const char* base, const char* str, TArrayDCT<CRegExp::CFinds>& finds, size_t progstart, const char*&reginput) const
{
	size_t scan;	// Current node. 
	size_t next=0;	// Next node. 

#ifdef DEBUG
	if( progstart != 0 && regnarrate)
		printf("%s(\n", GetPrintCmd(progstart));
#endif
	for(scan=progstart; scan != 0; scan = next)
	{
#ifdef DEBUG
		if( regnarrate)
			printf("%s...\n", GetPrintCmd(scan));
#endif
		next = this->nextcommand(scan);

		switch( this->command(scan) )
		{
			case REGEX_BOL:
				if( (reginput) != str)
					return false;
				break;
			case REGEX_EOL:
				if( *reginput != '\0')
					return false;
				break;
			case REGEX_RWORD:
				// Prev must be BOL or nonword 
				if( reginput > base &&
					(stringcheck::isalnum(reginput[-1]) || reginput[-1] == '_'))
					return false;
				// Must be a word containing letter, digit, or _ 
				if( !stringcheck::isalnum(*reginput) && *reginput != '_')
					return false;
				// go through the whole word
				do
				{
					reginput++;
				}
				while( stringcheck::isalnum(*reginput) || *reginput == '_' );
				// now looking at non letter, digit, or _ 
				break;
			case REGEX_WORDA:
				// Must be looking at a letter, digit, or _ 
				if( (!stringcheck::isalnum(*reginput)) && *reginput != '_')
					return false;
				// Prev must be BOL or nonword 
				if( reginput > base &&
					(stringcheck::isalnum(reginput[-1]) || reginput[-1] == '_'))
					return false;
				break;
			case REGEX_WORDZ:
				// Must be looking at non letter, digit, or _ 
				if( stringcheck::isalnum(*reginput) || *reginput == '_')
					return false;
				// We don't care what the previous char was 
				break;
			case REGEX_SPACE:
				if( !stringcheck::isspace(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_XDIGIT:
				if( !stringcheck::isxdigit(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_DIGIT:
				if( !stringcheck::isdigit(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_UPPER:
				if( !stringcheck::isupper(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_LOWER:
				if( !stringcheck::islower(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_GRAPH:
				if( !stringcheck::isgraph(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_PRINT:
				if( !stringcheck::isprint(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_PUNCT:
				if( !stringcheck::ispunct(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_CNTRL:
				if( !stringcheck::iscntrl(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_ANY:
				if( *reginput == '\0')
					return false;
				reginput++;
				break;
			case REGEX_EXACTLY:
			{
				size_t len;
				const char* opnd = this->operand(scan);
				// Inline the first character for speed. 
				if( *opnd != *reginput)
					return false;
				len = strlen(opnd);
				if( len > 1 && strncmp(opnd, reginput, len) != 0)
					return false;
				reginput += len;

				break;
			}
			case REGEX_ANYOF:
				if( *reginput == '\0' ||
						strchr(this->operand(scan), *reginput) == NULL)
					return false;
				reginput++;
				break;
			case REGEX_ANYBUT:
				if( *reginput == '\0' ||
						strchr(this->operand(scan), *reginput) != NULL)
					return false;
				reginput++;
				break;
			case REGEX_NOTHING:
				break;
			case REGEX_BACK:
				break;
			case REGEX_OPEN:
			{
				const char * const input = reginput;
				unsigned char *ip = (unsigned char *)this->operand(scan);
				size_t no = (((ushort)ip[0])    )
							| (((ushort)ip[1])<< 8);

				if( MatchMain(base, str, finds, next, reginput) )
				{
					if( finds.size() <= (size_t)no) finds.resize(no+1);
					// Don't set startp if some later
					// invocation of the same parentheses
					// already has.
					if( finds[no].startp == NULL)
						finds[no].startp = input;
					return true;
				}
				else
					return false;
				break;
			}
			case REGEX_CLOSE:
			{
				const char * const input = reginput;
				unsigned char *ip = (unsigned char *)this->operand(scan);
				size_t no = (((ushort)ip[0])    )
							| (((ushort)ip[1])<< 8);

				// tell the pos we have a finished find
				//if( finds.size() <= (size_t)no) finds.resize(no+1);
				//	finds[no].cnt++;

				if( MatchMain(base, str, finds, next, reginput) )
				{
					if( finds.size() <= (size_t)no) finds.resize(no+1);

					// Don't set endp if some later
					// invocation of the same parentheses
					// already has.
					if( finds[no].endp == NULL)
						finds[no].endp = input;
// perl regular expressions only accept the last match in a recursive pattern
// we here could find all of them
#ifndef REGEX_PCRECONFORM
					else
						finds.insert( CRegExp::CFinds(NULL,input), 1, no );
#endif
					return true;
				}
				else
					return false;
				break;
			}
			case REGEX_BRANCH:
			{
				const char* save = reginput;

				if( this->command(next) != REGEX_BRANCH)	// No choice. 
					next = scan+3;							// Avoid recursion. 
				else
				{
					while(this->command(scan) == REGEX_BRANCH)
					{
						if( MatchMain(base, str, finds, scan+3, reginput) )
							return true;
						reginput = (char*)save;
						scan = this->nextcommand(scan);
					}
					return false;
					// NOTREACHED 
				}
				break;
			}
			case REGEX_STAR: 
			case REGEX_PLUS:
			{
				const char nextch = (this->command(next) == REGEX_EXACTLY) ? *this->operand(next) : '\0';
				size_t no;
				const char* save = reginput;
				const size_t min = (this->command(scan) == REGEX_STAR) ? 0 : 1;

				for(no=MatchRepeat(scan+3,reginput) + 1; no>min; no--)
				{
					reginput = save + no - 1;
					// If it could work, try it. 
					if( nextch == '\0' || *reginput == nextch)
						if( MatchMain(base, str, finds, next, reginput) )
							return true;
				}
				return false;
				break;
			}
			case REGEX_END:
				return true;	// Success! 
				break;
			default:
				regerror( REGERR_CORRUPTION );
				return false;
				break;
		}
	}

	// We get here only if there's trouble -- normally "case END" is
	// the terminating point.
	regerror( REGERR_CORRUPTED_POINTERS );
	return false;
}
///////////////////////////////////////////////////////////////////////////////
// MatchRepeat - report how many times something simple would match
size_t CRegExp::CRegProgram::MatchRepeat( size_t node, const char*&reginput ) const
{
	size_t count;
	const char* scan;
	char ch;

	switch( this->command(node) )
	{
		case REGEX_ANY:
			return(strlen(reginput));
			break;
		case REGEX_EXACTLY:
			ch = *this->operand(node);
			count = 0;
			for (scan = reginput; *scan == ch; scan++)
				count++;
			return(count);
			break;
		case REGEX_ANYOF:
			return(strspn(reginput, this->operand(node)));
			break;
		case REGEX_ANYBUT:
			return(strcspn(reginput, this->operand(node)));
			break;
		default:		// Oh dear.  Called inappropriately. 
			regerror( REGERR_BAD_REGREPEAT );
			return(0);	// Best compromise. 
			break;
	}
	// NOTREACHED 
}


#ifdef DEBUG
///////////////////////////////////////////////////////////////////////////////
// GetPrintCmd - printable representation of opcode
const char * CRegExp::CRegProgram::GetPrintCmd( size_t pos ) const
{
	static char buf[64];
	const char *p="";

	strcpy(buf, ( ":" ));

	switch (this->command(pos))
	{
	case REGEX_BOL:		p= "bol"; break;
	case REGEX_EOL:		p= "eol"; break;
	case REGEX_ANY:		p= "any"; break;
	case REGEX_ANYOF:	p= "anyof"; break;
	case REGEX_ANYBUT:	p= "anybut"; break;
	case REGEX_BRANCH:	p= "branch"; break;
	case REGEX_EXACTLY:	p= "exact"; break;
	case REGEX_NOTHING:	p= "nothing"; break;
	case REGEX_BACK:	p= "back"; break;
	case REGEX_END:		p= "end"; break;
	case REGEX_STAR:	p= "star"; break;
	case REGEX_PLUS:	p= "plus"; break;
	case REGEX_WORDA:	p= "worda"; break;
	case REGEX_WORDZ:	p= "wordz"; break;

	case REGEX_SPACE:	p= "space"; break;
	case REGEX_XDIGIT:	p= "xdigit"; break;
	case REGEX_DIGIT:	p= "digit"; break;
	case REGEX_UPPER:	p= "upper"; break;
	case REGEX_LOWER:	p= "lower"; break;
	case REGEX_RWORD:	p= "rword"; break;
	case REGEX_GRAPH:	p= "graph"; break;
	case REGEX_PRINT:	p= "print"; break;
	case REGEX_PUNCT:	p= "punct"; break;
	case REGEX_CNTRL:	p= "cntrl"; break;

	case REGEX_OPEN:
	{
		unsigned char *ip = (unsigned char *)this->operand(pos);
		const int no = (ip[0]    )
					 | (ip[1]<< 8);

		sprintf(buf+strlen(buf), ( "open %d" ), no);
		p = NULL;
		break;
	}
	case REGEX_CLOSE:
	{
		unsigned char *ip = (unsigned char *)this->operand(pos);
		const int no = (ip[0]    )
					 | (ip[1]<< 8);

		sprintf(buf+strlen(buf), ( "close %d" ), no);
		p = NULL;
		break;
	}
	default:
		p = "corrupted opcode";
		break;
	}
	if( p != NULL)
		strcat(buf, p);
	return (buf);
}
///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::CRegProgram::Print() const
{
	char op = REGEX_EXACTLY;	// Arbitrary non-END op. 
	size_t next;
	size_t s = 1;
	while (op != REGEX_END)
	{	// While that wasn't END last time... 
		op = this->command(s);

		printf(( "%2d %s" ), s, GetPrintCmd(s));	// Where, what. 
		
		next = this->nextcommand(s);
		if( next == NULL)		// Next ptr. 
			printf("(0)");
		else 
			printf("(%d)", next);
		s += 3;
		if( op == REGEX_ANYOF || op == REGEX_ANYBUT || op == REGEX_EXACTLY)
		{
			// Literal string, where present. 
			while( cProgramm[s] != '\0')
			{
				putchar( cProgramm[s] );
				s++;
			}
			s++;
		}
		else if( op == REGEX_OPEN || op == REGEX_CLOSE)
		{
			s+=2;
		}

		
		putchar('\n');
	}

	// Header fields of interest. 
	if( cStartChar != '\0')
		printf( "start with `%c' ", cStartChar);
	if( cAnchor)
		printf( "anchored" );
	if( cMust > 0)
		printf( "must have \"%s\"", this->operand(cMust) );
	printf(( "\n" ));
}
///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::CRegProgram::Dump() const
{
	for(size_t i=0; i<cProgramm.size(); i++)
		printf("%i ", cProgramm[i]);
	printf("\n"); 
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::CRegProgram::Compile(const char* exp, bool ignorecase)
{
	int flags;

	cStartChar	= 0;
	cAnchor		= 0;
	cMust		= 0;
	cMustLen	= 0;
	cStatus		= false;

	if( exp == NULL)
	{
		regerror( REGERR_NULL_TO_REGCOMP );
		return false;
	}

	cProgramm.clear();
	AddValue(REGEX_MAGIC);

	int parcnt=1;
	const char *parse = exp;

	if( ignorecase )
	{
		parse = new char[(strlen(exp) * 4) + 1];
		ignoreCase( exp, (char*)parse );
	}

#ifdef DEBUG
	cExpression = parse;
#endif

	if( 0 != this->ParseExp(parse, false, parcnt, flags)  )
	{
		size_t scan = 1;	// First BRANCH. 
		if( this->command(this->nextcommand(scan)) == REGEX_END )
		{	// Only one top-level choice. 
			scan += 3;

			// Starting-point info. 
			if( this->command(scan) == REGEX_EXACTLY)
				cStartChar = *this->operand(scan);
			else if( this->command(scan) == REGEX_BOL)
				cAnchor = 1;

			// If there's something expensive in the r.e., find the
			// longest literal string that must appear and make it the
			// cMust.  Resolve ties in favor of later strings, since
			// the cStartChar check works with the beginning of the r.e.
			// and avoiding duplication strengthens checking.  Not a
			// strong reason, but sufficient in the absence of others.
			 
			if( flags&REGEX_SPSTART) 
			{
				size_t longest = 0;
				size_t len = 0;

				while(scan != 0)
				{
					if( (this->command(scan) == REGEX_EXACTLY) && (len < strlen(this->operand(scan))) )
					{
						longest = scan;
						len = strlen(this->operand(scan));
					}
					scan = this->nextcommand(scan);
				}
				cMust = longest;
				cMustLen = (int)len;
			}
		}
		cStatus = true;
	}
	if( ignorecase && parse )
		delete[] (char*)parse;

	return cStatus;
}
///////////////////////////////////////////////////////////////////////////////
// regexec - match a CRegExpWorker against a string
bool CRegExp::CRegProgram::Execute( const char* str, TArrayDCT<CRegExp::CFinds>& finds ) const
{
	finds.resize(0);

	// Be paranoid. 
	if( str == NULL )
	{
		regerror( REGERR_NULLARG );
		return false;
	}
	if( cProgramm.size()<1 )
	{
		regerror( REGERR_NO_REGEXP );
		return false;
	}

	// Check validity of program. 
	if( cProgramm[0] != REGEX_MAGIC)
	{
		regerror( REGERR_CORRUPTED );
		return false;
	}

	// If there is a "must appear" string, look for it. 
	if( cMust > 0 && strstr( str, this->operand(cMust) ) == NULL )
		return false;

	// Simplest case:  anchored match need be tried only once. 
	if( cAnchor )
		return( MatchTry(str, str, finds ) );

	// Messy cases:  unanchored match. 
	if( cStartChar != '\0' )
	{	// We know what char it must start with. 
		for( const char *s = str; s != NULL; s = strchr( s+1 , cStartChar ) )
			if( MatchTry(str, s, finds) )
				return true;
		return false;
	}
	else
	{	// We don't -- general case. 
		for ( const char * s = str; !MatchTry(str, s, finds); s++ )
			if( *s == '\0')
				return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// CRegExp functions

///////////////////////////////////////////////////////////////////////////////
//
const CRegExp & CRegExp::operator=( const CRegExp & r )
{
	if( this != &r )
		cProg = r.cProg;
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
//
const CRegExp & CRegExp::operator=( const char* exp )
{
	cProg->Compile(exp,false);
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
//
const CRegExp & CRegExp::assign( const char* exp, bool iCase )
{
	cProg->Compile(exp,iCase);
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
//
const CRegExp & CRegExp::assign( const CRegExp & r )
{
	if( this != &r )
		cProg = r.cProg;
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::Match( const char * s ) const
{
	clearerror();
	cQueryString = s;
	bool ret = false;
	if( cProg.get() )
	{	// access to cProg is read-only so there will be no automatic copy
		ret  = cProg->Execute(cQueryString, cFinds);
		//ret = ((const TPtrAutoRef<CRegProgram>)cProg)->Execute(cQueryString, cFinds);
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::Match( const string<> &s ) const
{
	clearerror();
	cQueryString = s;
	bool ret = false;
	if( cProg.get() )
	{	// access to cProg is read-only so there will be no automatic copy
		ret  = cProg->Execute(cQueryString, cFinds);
		//ret = ((const TPtrAutoRef<CRegProgram>)cProg)->Execute(cQueryString, cFinds);
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
int CRegExp::SubStrings() const
{
	clearerror();
	int ret = -1;
	if( cProg.get() )
		ret = cFinds.size()-1;
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
int CRegExp::SubStart( unsigned int i ) const
{
	clearerror();
	int ret = -1;
	if( cProg.get() && i<cFinds.size() )
		ret = cFinds[i].startp - (const char*)cQueryString;
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
int CRegExp::SubLength( unsigned int i ) const
{
	clearerror();
	int ret = -1;
	if( cProg.get() && i<cFinds.size() )
	{
		ret = cFinds[i].endp - cFinds[i].startp;
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::isOK() const
{
	return cProg.get() ? cProg->Status() : false;
}

#ifdef DEBUG
///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::Dump() const
{
	if( cProg.get() )
		cProg->Dump();
	else
		printf("No CRegExpWorker to dump out\n");
}
///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::Print() const
{
	if( cProg.get() )
		cProg->Print();
	else
		printf("No CRegExpWorker to print out\n");
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
const string<> CRegExp::operator[]( unsigned int i ) const
{
	clearerror();
	if( cProg.get() && i<cFinds.size() )
	{
		return string<>( cFinds[i].startp, SubLength(i) );
	}
	else
	{
		return "";
	}
}
///////////////////////////////////////////////////////////////////////////////
// GetReplaceString	- Converts a replace expression to a string
//					- perform substitutions after a match
// Returns			- The resultant string
string<> CRegExp::GetReplaceString( const char* sReplaceExp ) const
{
	clearerror();
	if( !cProg.get() )
		return ( "" );

	char *src = (char *)sReplaceExp;
	char c;
	int no;
	size_t len;
	string<> ret;

	if( sReplaceExp == NULL )
	{
		cProg->regerror( REGERR_NULL_TO_REGSUB  );
	}
	else if( cProg->cProgramm.size()<=0 || cProg->cProgramm[0] != REGEX_MAGIC )
	{
		cProg->regerror( REGERR_DAMAGED_REGEXP_REGSUB );
	}
	else
	{	// create the string
		src = (char *)sReplaceExp;
		while ((c = *src++) != ('\0')) 
		{
			if( c == ('&'))
				no = 0;
			else if( c == ('\\') && stringcheck::isdigit(*src))
			{
				no = *src++ - ('0');
				while( stringcheck::isdigit(*src) )
					no = 10*no + *src++ - ('0');				
			}
			else
				no = -1;

			if( no < 0) 
			{	
				// Ordinary character. 
				if( c == ('\\') && (*src == ('\\') || *src == ('&')))
					c = *src++;
				ret.append( c );
			} 
			else if( cFinds.size()>(size_t)no &&
					 cFinds[no].startp != NULL && 
					 cFinds[no].endp  != NULL && 
					 cFinds[no].endp > cFinds[no].startp ) 
			{
				// Get tagged expression
				len = cFinds[no].endp - cFinds[no].startp;
				if( len != 0 && strlen( cFinds[no].startp ) < len )
				{	/* strncpy hit NUL. */
					cProg->regerror( REGERR_DAMAGED_MATCH_STRING );
					return "";
				}
				else
				{
					ret.append( cFinds[no].startp, len );
				}
			}
		}
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
const char* CRegExp::errmsg() const
{
	return cProg.get() ? cProg->errmsg() : CRegErrorHandler::FindErr(REGERR_NO_REGEXP);
}
///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::clearerror() const
{	
	if( cProg.get() ) cProg->clearerror();
}










///////////////////////////////////////////////////////////////////////////////
// test function
void test_regex()
{
#ifdef DEBUG
	const char *a;
	string<> ret;
	CRegExp exp("(.*ab)([^a]*)a");
	CRegExp xx = exp;

	CRegExp yy("(([^,]*),)*([^,]*)");
	yy.Dump();
	yy.Print();
	yy.Match("ab1ba, ab2ba, ab3ba, ab4ba, ab5ba, ab6ab");
	{
		int i;
		for(i=0; i<=yy.SubStrings(); i++)
			printf("%2i: %s\n", i, (const char*)yy[i]);
	}

	yy = "([abc])*d";
	yy.Match("abbbcd");
	{
		int i;
		for(i=0; i<=yy.SubStrings(); i++)
			printf("%2i: %s\n", i, (const char*)yy[i]);
	}


	exp = "(blue|white|red)";
	exp.Dump();
	exp.Print();

	a = "aaababb1bbbbaaaaaabbb ff white bbbaaabb2bbbbaaaaaaaa	aaaaaab3bbbbaxx";
	exp.Match(a);
	a = ret = exp[0];
	a = ret = exp[1];
	a = ret = exp[2];

	a = "aaababb1bbbbaaaaaabbb ff white bbbaaabb2bbbbaaaaaaaa	aaaaaab3bbbbaxx";
	xx.Match(a);
	a = ret = xx[0];
	a = ret = xx[1];
	a = ret = xx[2];


	CRegExp x2("([^,]*),");
	a = "ab1ba, ab2ba, ab3ba, ab4ba, ab5ba, ab6ab";

	x2.Match(a);
	a = ret = x2[0];
	a = ret = x2[1];
	a = ret = x2[2];
	a = ret = x2[3];
	a = ret = x2[4];
	a = ret = x2[5];


	const char *fields[][5] = 
	{
	  {"abc","abc","y","&","abc"},
	  {"abc","xbc","n","-","-"},
	  {"abc","axc","n","-","-"},
	  {"abc","abx","n","-","-"},
	  {"abc","xabcy","y","&","abc"},
	  {"abc","ababc","y","&","abc"},
	  {"ab*c","abc","y","&","abc"},
	  {"ab*bc","abc","y","&","abc"},
	  {"ab*bc","abbc","y","&","abbc"},
	  {"ab*bc","abbbbc","y","&","abbbbc"},
	  {"ab+bc","abbc","y","&","abbc"},
	  {"ab+bc","abc","n","-","-"},
	  {"ab+bc","abq","n","-","-"},
	  {"ab+bc","abbbbc","y","&","abbbbc"},
	  {"ab?bc","abbc","y","&","abbc"},
	  {"ab?bc","abc","y","&","abc"},
	  {"ab?bc","abbbbc","n","-","-"},
	  {"ab?c","abc","y","&","abc"},
	  {"^abc$","abc","y","&","abc"},
	  {"^abc$","abcc","n","-","-"},
	  {"^abc","abcc","y","&","abc"},
	  {"^abc$","aabc","n","-","-"},
	  {"abc$","aabc","y","&","abc"},
	  {"^","abc","y","&",""},
	  {"$","abc","y","&",""},
	  {"a.c","abc","y","&","abc"},
	  {"a.c","axc","y","&","axc"},
	  {"a.*c","axyzc","y","&","axyzc"},
	  {"a.*c","axyzd","n","-","-"},
	  {"a[bc]d","abc","n","-","-"},
	  {"a[bc]d","abd","y","&","abd"},
	  {"a[b-d]e","abd","n","-","-"},
	  {"a[b-d]e","ace","y","&","ace"},
	  {"a[b-d]","aac","y","&","ac"},
	  {"a[-b]","a-","y","&","a-"},
	  {"a[b-]","a-","y","&","a-"},
	  {"[k]","ab","n","-","-"},
	  {"a[b-a]","-","c","-","-"},
	  {"a[]b","-","c","-","-"},
	  {"a[","-","c","-","-"},
	  {"a]","a]","y","&","a]"},
	  {"a[]]b","a]b","y","&","a]b"},
	  {"a[^bc]d","aed","y","&","aed"},
	  {"a[^bc]d","abd","n","-","-"},
	  {"a[^-b]c","adc","y","&","adc"},
	  {"a[^-b]c","a-c","n","-","-"},
	  {"a[^]b]c","a]c","n","-","-"},
	  {"a[^]b]c","adc","y","&","adc"},
	  {"ab|cd","abc","y","&","ab"},
	  {"ab|cd","abcd","y","&","ab"},
	  {"()ef","def","y","&-\\1","ef-"},
	  {"()*","-","c","-","-"},
	  {"*a","-","c","-","-"},
	  {"^*","-","c","-","-"},
	  {"$*","-","c","-","-"},
	  {"(*)b","-","c","-","-"},
	  {"$b","b","n","-","-"},
	  {"a\\","-","c","-","-"},
	  {"a\\(b","a(b","y","&-\\1","a(b-"},
	  {"a\\(*b","ab","y","&","ab"},
	  {"a\\(*b","a((b","y","&","a((b"},
	  {"a\\\\b","a\\b","y","&","a\\b"},
	  {"abc)","-","c","-","-"},
	  {"(abc","-","c","-","-"},
	  {"((a))","abc","y","&-\\1-\\2","a-a-a"},
	  {"(a)b(c)","abc","y","&-\\1-\\2","abc-a-c"},
	  {"a+b+c","aabbabc","y","&","abc"},
	  {"a**","-","c","-","-"},
	  {"a*?","-","c","-","-"},
	  {"(a*)*","-","c","-","-"},
	  {"(a*)+","-","c","-","-"},
	  {"(a|)*","-","c","-","-"},
	  {"(a*|b)*","-","c","-","-"},
	  {"(a+|b)*","ab","y","&-\\1","ab-b"},
	  {"(a+|b)+","ab","y","&-\\1","ab-b"},
	  {"(a+|b)?","ab","y","&-\\1","a-a"},
	  {"[^ab]*","cde","y","&","cde"},
	  {"(^)*","-","c","-","-"},
	  {"(ab|)*","-","c","-","-"},
	  {")(","-","c","-","-"},
	  {"","abc","y","&",""},
	  {"abc","","n","-","-"},
	  {"a*","","y","&",""},
	  {"abcd","abcd","y","&-\\&-\\\\&","abcd-&-\\abcd"},
	  {"a(bc)d","abcd","y","\\1-\\\\1-\\\\\\1","bc-\\1-\\bc"},
	  {"([abc])*d","abbbcd","y","&-\\1","abbbcd-c"},
	  {"([abc])*bcd","abcd","y","&-\\1","abcd-a"},
	  {"a|b|c|d|e","e","y","&","e"},
	  {"(a|b|c|d|e)f","ef","y","&-\\1","ef-e"},
	  {"((a*|b))*","-","c","-","-"},
	  {"abcd*efg","abcdefg","y","&","abcdefg"},
	  {"ab*","xabyabbbz","y","&","ab"},
	  {"ab*","xayabbbz","y","&","a"},
	  {"(ab|cd)e","abcde","y","&-\\1","cde-cd"},
	  {"[abhgefdc]ij","hij","y","&","hij"},
	  {"^(ab|cd)e","abcde","n","x\\1y","xy"},
	  {"(abc|)ef","abcdef","y","&-\\1","ef-"},
	  {"(a|b)c*d","abcd","y","&-\\1","bcd-b"},
	  {"(ab|ab*)bc","abc","y","&-\\1","abc-a"},
	  {"a([bc]*)c*","abc","y","&-\\1","abc-bc"},
	  {"a([bc]*)(c*d)","abcd","y","&-\\1-\\2","abcd-bc-d"},
	  {"a([bc]+)(c*d)","abcd","y","&-\\1-\\2","abcd-bc-d"},
	  {"a([bc]*)(c+d)","abcd","y","&-\\1-\\2","abcd-b-cd"},
	  {"a[bcd]*dcdcde","adcdcde","y","&","adcdcde"},
	  {"a[bcd]+dcdcde","adcdcde","n","-","-"},
	  {"(ab|a)b*c","abc","y","&-\\1","abc-ab"},
	  {"((a)(b)c)(d)","abcd","y","\\1-\\2-\\3-\\4","abc-a-b-d"},
	  {"[ -~]*","abc","y","&","abc"},
	  {"[ -~ -~]*","abc","y","&","abc"},
	  {"[ -~ -~ -~]*","abc","y","&","abc"},
	  {"[ -~ -~ -~ -~]*","abc","y","&","abc"},
	  {"[ -~ -~ -~ -~ -~]*","abc","y","&","abc"},
	  {"[ -~ -~ -~ -~ -~ -~]*","abc","y","&","abc"},
	  {"[ -~ -~ -~ -~ -~ -~ -~]*","abc","y","&","abc"},
	  {"[a-zA-Z_][a-zA-Z0-9_]*","alpha","y","&","alpha"},
	  {"^a(bc+|b[eh])g|.h$","abh","y","&-\\1","bh-"},
	  {"(bc+d$|ef*g.|h?i(j|k))","effgz","y","&-\\1-\\2","effgz-effgz-"},
	  {"(bc+d$|ef*g.|h?i(j|k))","ij","y","&-\\1-\\2","ij-ij-j"},
	  {"(bc+d$|ef*g.|h?i(j|k))","effg","n","-","-"},
	  {"(bc+d$|ef*g.|h?i(j|k))","bcdd","n","-","-"},
	  {"(bc+d$|ef*g.|h?i(j|k))","reffgz","y","&-\\1-\\2","effgz-effgz-"},
	  {"(((((((((a)))))))))","a","y","&","a"},
	  {"multiple words of text","uh-uh","n","-","-"},
	  {"multiple words","multiple words, yeah","y","&","multiple words"},
	  {"(.*)c(.*)","abcde","y","&-\\1-\\2","abcde-ab-de"},
	  {"\\((.*), (.*)\\)","(a, b)","y","(\\2, \\1)","(b, a)"},
	  {"\\<abc\\>","d abc f","y","&","abc"},
	  {"\\<abc\\>","dabc f","n","-","-"},
	  {"\\<abc\\>","d abcf","n","-","-"},
	  {"\\<abc\\>","d abc_f","n","-","-"},
	};

	size_t i;
	printf("\n");
	for(i=0; i<sizeof(fields)/sizeof(fields[0]); i++)
	{
		CRegExp r(fields[i][0]);
		if( !r.isOK() )
		{
			if( *fields[i][2] != 'c' )
				printf("%i: unexpected comp failure in '%s' (%s)\n", i, fields[i][0], r.errmsg());
			continue;
		}
		else if( *fields[i][2] == 'c')
		{
			printf("%i: unexpected comp success in '%s'\n", i, fields[i][0]);
			continue;
		}

		if( !r.Match(fields[i][1]))
		{
			if( *fields[i][2] != 'n' )
				printf("%i: unexpected match failure in '%s' / '%s' (%s)\n", i, fields[i][0],fields[i][1], r.errmsg());
			continue;
		}
		if( *fields[i][2] == 'n' )
		{
			printf("%i: unexpected match success in '%s' / '%s'\n", i, fields[i][0],fields[i][1]);
			continue;
		}
		string<> repl = r.GetReplaceString( fields[i][3] );
		if( r.errmsg() )
		{
			printf("%i: GetReplaceString complaint in '%s'\n", i, fields[i][0], r.errmsg());
			continue;
		}
		if ( repl != fields[i][4] )
			printf("%i: regsub result in '%s' wrong (is: '%s' != should: '%s')\n", i, fields[i][0], (const char*)repl, fields[i][4]);
	}


	{
		CRegExp badregexp;
		CRegExp r;
	
		r = CRegExp(NULL);
		if( r.isOK() || !r.errmsg() )
			printf("regcomp(NULL) doesn't complain\n");

		r = CRegExp();		// clean it out
		if( r.Match("foo") || !r.errmsg() )
			printf("regexec(NULL, ...) doesn't complain\n");

		r = CRegExp("foo");
		if( !r.isOK() )
			printf("regcomp(\"foo\") fails");

		if ( r.Match(NULL) || !r.errmsg() )
			printf("regexec(..., NULL) doesn't complain\n");

		r = CRegExp();
		string<> repl = r.GetReplaceString( "foo" );
		if ( !r.errmsg() )
			printf("GetReplaceString(NULL, ..., ...) doesn't complain\n");


		repl = r.GetReplaceString((TCHAR *)NULL );
		if ( !r.errmsg() )
			printf("GetReplaceString(..., NULL, ...) doesn't complain\n");
		

		if( badregexp.Match( "foo" ) || !badregexp.errmsg() )
			printf("regexec(nonsense, ...) doesn't complain\n");

		repl = badregexp.GetReplaceString( "foo" );
		if( !badregexp.errmsg() )
			printf("GetReplaceString(nonsense, ..., ...) doesn't complain\n");

		CRegExp reg1( "[abc]+" );

		if ( !reg1.isOK() )
			printf("regcomp(\"[abc]+\") fails\n");

		if ( !reg1.Match( "test string 1 - aaa") || reg1.errmsg() )
			printf("Match failed\n");

		if ( reg1[0] != "aaa" )
			printf("Match failed %s!=%s\n", (const char*)reg1[0], "aaa");


		CRegExp reg2( reg1 );

		if ( !reg2.Match("test string 2 - bbb") || reg2.errmsg() )
			printf("Match failed\n");

		repl = reg2.GetReplaceString( "&" );
		if ( reg2[0] != "bbb" )
			printf("Match failed %s!=%s\n", (const char*)reg2[0], "bbb");


		CRegExp reg3 ( reg1 );

		if ( !reg3.Match( "test string 3 - ccc") || reg3.errmsg() )
			printf("Match failed\n");

		if ( reg3[0] != "ccc" )
			printf("Match failed %s!=%s\n", (const char*)reg3[0], "ccc");

	}
#endif
}


