#include "basetypes.h"
#include "baseregex.h"
#include "basesafeptr.h"

NAMESPACE_BEGIN(basics)

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
// *** THIS IS AN ALTERED VERSION.  It was altered by Atsutazaka Hinoko
// *** in June 2003, adding more regex syntax
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
//  definition		number			opnd?			meaning
	REGEX_END		=	0,			//	no			End of program.

	///////////////////////////////////////////////////////////////////////////
	// controls for simple element matches
	REGEX_N_STAR	=	1,			//	node		Match this 0 or more times.
	REGEX_N_PLUS	=	2,			//	node		Match this 1 or more times.
	REGEX_L_STAR	=	3,			//	node		Match this 0 or more times.
	REGEX_L_PLUS	=	4,			//	node		Match this 1 or more times.

	REGEX_N_RANGE	=	7,			//	2x2bytes	upper/lower limit
	REGEX_N_RANGEMIN=	8,			//	2bytes		lower limit
	REGEX_N_RANGEMAX=	9,			//	2bytes		upper limit

	REGEX_L_RANGE	=	10,			//	2x2bytes	upper/lower limit
	REGEX_L_RANGEMIN=	11,			//	2bytes		lower limit
	REGEX_L_RANGEMAX=	12,			//	2bytes		upper limit



	///////////////////////////////////////////////////////////////////////////
	// controls for complex matches
	REGEX_N_BRANCH	=	16,			//	node		greedy Match this, or the next..\&.
	REGEX_L_BRANCH	=	17,			//	node		lazy Match this, or the next..\&.
	//
	REGEX_N_BRRANGE	=	19,			//	2x2bytes	upper/lower limit
	REGEX_N_BRRGMIN	=	20,			//	2bytes		lower limit
	REGEX_N_BRRGMAX	=	21,			//	2bytes		upper limit
	
	REGEX_L_BRRANGE	=	22,			//	2x2bytes	lazy upper/lower limit
	REGEX_L_BRRGMIN	=	23,			//	2bytes		lazy lower limit
	REGEX_L_BRRGMAX	=	24,			//	2bytes		lazy upper limit


	// UNUSED		=	28-31



	///////////////////////////////////////////////////////////////////////////
	// control bitmasks
//	REGEX_CONTROL	=	0x1F,		// control mask
//	REGEX_OPERATION	=	0x0F,		// operation mask
//	REGEX_COMPLEX	=	0x10,		// complex mask
//	REGEX_LAZY		=	0x08,		// lazy mask
//	REGEX_RANGED	=	0x04,		// ranged mask
//	REGEX_MINMAX	=	0x02,		// min/max vs. range mask
//	REGEX_POSSESSIVE

	// detect a branch with					if( 0==(c&~REGEX_CONTROL) )
	// detect a simple operation with		if( (branch) && 0==(c&REGEX_COMPLEX) )
	// detect a complex operation with		if( (branch) && 0!=(c&REGEX_COMPLEX) )
	// detect a lazy operation with			if( (branch) && 0!=(c&REGEX_LAZY) )
	// detect a non-ranged op				if( (branch) && 0==(c&REGEX_RANGED) )
	// detect a ranged op					if( (branch) && 0!=(c&REGEX_RANGED) )

	///////////////////////////////////////////////////////////////////////////
	// zero length match nodes
	REGEX_BOL		=	32,			//	no			Match beginning of line.
	REGEX_EOL		=	33,			//	no			Match end of line.
	REGEX_BOS		=	34,			//	no			\A, matches beginning of string, does not match begin of line
	REGEX_EOS2		=	35,			//	no			\Z, matches end of string, does not match end of line, exect when the EOS is after the EOL
	REGEX_EOS		=	36,			//	no			\z, matches end of string, does not match end of line
	REGEX_WCHAR		=	58,			//	no			/b
	REGEX_NOWCHAR	=	59,			//	no			/B

	///////////////////////////////////////////////////////////////////////////
	// length matching nodes
	REGEX_ANY		=	37,			//	no			Match any character.
	REGEX_ANYOF		=	38,			//	sets +\0	Match any of these.
	REGEX_ANYBUT	=	39,			//	sets +\0	Match any but one of these.
	REGEX_EXACTLY	=	40,			//	str	+\0		Match this string.
	REGEX_WORDA		=	41,			//	no			Match "" at wordchar, where prev is nonword
	REGEX_WORDZ		=	42,			//	no			Match "" at nonwordchar, where prev is word
	// posix commands
	REGEX_SPACE		=	43,			//	no			[:space:]	-> \s,
	REGEX_NOSPACE	=	44,			//	no			^[:space:]	-> \S,
	REGEX_XDIGIT	=	45,			//	no			[:xdigit:]	-> \h
	REGEX_NOXDIGIT	=	46,			//	no			^[:xdigit:]	-> \H
	REGEX_DIGIT		=	47,			//	no			[:digit:]	-> \d
	REGEX_NODIGIT	=	48,			//	no			^[:digit:]	-> \D
	REGEX_UPPER		=	49,			//	no			[:upper:]	-> \U ??
	REGEX_LOWER		=	50,			//	no			[:lower:]	-> \u ??
	REGEX_WORD		=	51,			//	no			[:word:]	-> \w,
	REGEX_NOWORD	=	52,			//	no			^[:word:]	-> \W,
	REGEX_GRAPH		=	53,			//	no			[:graph:]
	REGEX_PRINT		=	54,			//	no			[:print:]
	REGEX_PUNCT		=	55,			//	no			[:punct:]	-> \p ??
	REGEX_NOPUNCT	=	56,			//	no			^[:punct:]	-> \P ??
	REGEX_CNTRL		=	57,			//	no			[:cntrl:]
	REGEX_LETTER	=	60,			//	no			\l
	REGEX_NOLETTER	=	61,			//	no			\L

//			\X					matches a single unicode char
//			\uFFFF				Matches a specific Unicode code point (4 hexadecimal)
//			\p{L} or \p{Letter}	Matches a single Unicode code point that has the property "letter"
//			\P{L} or \P{Letter}	Matches a single Unicode code point that has not the property "letter"

	REGEX_SETRANGE	=	62,			//	2byte		lower/upper, range of chars
	// anyof/anybut opcodes
	REGEX_SET		=	63,			//	str + \0	self defined set
	// back reference
	REGEX_BACKREF	=	64,			//	level(2byte)	backreference to searchstring, does "REGEX_EXACTLY \<number>"

	///////////////////////////////////////////////////////////////////////////
	// parenthesis
	REGEX_OPEN		=	65,			//	2byte		Sub-RE starts here.
	REGEX_CLOSE		=	66,			//	2byte		Analogous to OPEN.

	///////////////////////////////////////////////////////////////////////////
	// controls
	REGEX_NOTHING	=	67,			//	no			nop
	REGEX_BACK		=	68,			//	no			"next" ptr points backward.

	REGEX_LAHEADT	=	69,			//	node		positive lookahead
	REGEX_LAHEADF	=	70,			//	node		negative lookahead
	REGEX_LBEHINDT	=	71,			//	str+\0		positive lookbehind
	REGEX_LBEHINDF	=	72,			//	str+\0		negative lookbehind

	REGEX_POSSESS	=	73,			//	no			subRE with possession

	///////////////////////////////////////////////////////////////////////////
	// config
	REGEX_CONFIG	=	74,			//	1byte		set configs for the rest of the RE (saves current state in imap)
	REGEX_RESTORE	=	75,			//	2byte		restore config for the sub RE and restore after
	// config bits
	REGEX_CONF_CASE	=	0x01,	// case insensitive (!! true when case INSENSITIVE !!)
	REGEX_MASK_CASE	=	0x02,
	REGEX_CONF_NLDT	=	0x04,	// dot matches newline
	REGEX_MASK_NLDT	=	0x08,
	REGEX_CONF_NLSE	=	0x10,	// caret/dollar match newline
	REGEX_MASK_NLSE	=	0x20
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
	REGERR_INVALID_BRACKET,
	REGERR_INTERNAL_ERROR_JUNK, 
	REGERR_OP_COULD_BE_EMPTY, 
	REGERR_NESTED_OP, 
	REGERR_INVALID_RANGE,
	REGERR_UNMATCHED_BRACE, 
	REGERR_INTERNAL_UNEXPECTED_CHAR, 
	REGERR_OP_FOLLOWS_NOTHING,
	REGERR_TRAILING_ESC, 
	REGERR_INVALID_BACKREF,
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
	{ REGERR_CORRUPTED,					( "internal error: corrupted programm" ) },
	{ REGERR_CORRUPTION,				( "internal error: CRegExpWorker corruption" ) },
	{ REGERR_CORRUPTED_POINTERS,		( "internal error: corrupted pointers" ) },
	{ REGERR_BAD_REGREPEAT,				( "internal error: bad call of regrepeat" ) },
	{ REGERR_CORRUPTED_OPCODE,			( "internal error: corrupted opcode" ) },
	{ REGERR_NULL_TO_REGSUB,			( "NULL argument to regsub" ) },
	{ REGERR_DAMAGED_REGEXP_REGSUB,		( "empty regular expression fed to regsub" ) },
	{ REGERR_DAMAGED_MATCH_STRING,		( "damaged match string" ) },
	{ REGERR_NULL_TO_REGCOMP,			( "NULL argument to regcomp" ) },
	{ REGERR_TO_BIG,					( "internal error: CRegExpWorker too big" ) },
	{ REGERR_TO_MANY_PAREN,				( "too many ()" ) },
	{ REGERR_UNTERMINATED_PAREN,		( "unterminated ()" ) },
	{ REGERR_UNMATCHED_PAREN,			( "unmatched ()" ) },
	{ REGERR_INVALID_BRACKET,			( "invalid range in {}" ) },
	{ REGERR_INTERNAL_ERROR_JUNK,		( "internal error: junk on end" ) },
	{ REGERR_OP_COULD_BE_EMPTY,			( "*+ operand could be empty" ) },
	{ REGERR_NESTED_OP,					( "nested *?+" ) },
	{ REGERR_INVALID_RANGE,				( "invalid [] range" ) },
	{ REGERR_UNMATCHED_BRACE,			( "unmatched []" ) },
	{ REGERR_INTERNAL_UNEXPECTED_CHAR,	( "internal error: \\0|) unexpected" ) },
	{ REGERR_OP_FOLLOWS_NOTHING,		( "?+* follows nothing" ) },
	{ REGERR_TRAILING_ESC,				( "trailing \\" ) },
	{ REGERR_INVALID_BACKREF,			( "invalid backreference count" ) },
	{ REGERR_NO_REGEXP,					( "no regular expression" ) },
	// must be last value
	{ REGERR_SENTINEL_VALUE,			( "unknown error") }	
};


///////////////////////////////////////////////////////////////////////////////
// Flags to be passed up and down.

enum {
	REGEX_WORST		=	0,	// Worst case.
	REGEX_HASWIDTH	=	01,	// Known never to match null string.
	REGEX_SIMPLE	=	02,	// Simple enough to be STAR/PLUS operand.
	REGEX_SPSTART	=	04,	// Starts with * or +.

	REGEX_NOPAREN	=	0,
	REGEX_PAREN		=	01,
	REGEX_OFFPAREN	=	02
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
// not needed anymore
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
	if( this->command(p) == REGEX_N_BRANCH ||
		this->command(p) == REGEX_L_BRANCH )
		AddTail(p+3, val);
	else if( this->command(p) == REGEX_N_BRRGMIN ||
			 this->command(p) == REGEX_N_BRRGMAX ||
			 this->command(p) == REGEX_L_BRRGMIN ||
			 this->command(p) == REGEX_L_BRRGMAX )
		AddTail(p+5, val);
	else if( this->command(p) == REGEX_N_BRRANGE ||
			 this->command(p) == REGEX_L_BRRANGE )
		AddTail(p+7, val);
}
///////////////////////////////////////////////////////////////////////////////
// ParseExp - regular expression, i.e. main body or parenthesized thing
// Caller must absorb opening parenthesis.
// Combining parenthesis handling with the base level of regular expression
// is a trifle forced, but the need to tie the tails of the branches to what
// follows makes it hard to avoid.
size_t CRegExp::CRegProgram::ParseExp(const char*&parsestr, int paren, int &parcnt, int &flag)
{
	size_t ret=0;
	size_t br;
	size_t ender;
	int parno = 0;
	int flags;

	flag = REGEX_HASWIDTH;	// Tentatively. 

	if(paren)
	{	// Make an OPEN node. 
		if( paren==REGEX_PAREN )
		{	// only when really counting it
			parno = parcnt;
			parcnt++;
		}
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
			regerror( REGERR_UNMATCHED_PAREN );
		else
			regerror( REGERR_INTERNAL_ERROR_JUNK );
		return 0;
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
	int internflag;
	int c;

	flag = REGEX_WORST;				// Tentatively. 

	ret = AddNode(REGEX_N_BRANCH);
	chain = 0;
	while ((c = *parsestr) != '\0' && c != '|' && c != ')')
	{
		latest = ParsePiece(parsestr, parcnt, internflag);
		if( latest == 0)
			return 0;
		flag |= internflag&REGEX_HASWIDTH;
		if( chain == 0)		// First piece. 
			flag |= internflag&REGEX_SPSTART;
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

	// check for recursions
	int lobound=0, hibound=0, run=0;
	if( op == ( '{' ))
	{	// {recursion} or {min,max}  or {min,}  or {,max}
		// not really implemented, just projected to ?,* or +
		parsestr++;
		while( *parsestr!='}' )
		{
			if(*parsestr == ',')
			{
				if(run)
				{
					regerror( REGERR_INVALID_BRACKET );
					return 0;
				}
				else
					run++;
			}
			else if(*parsestr >= '0' || *parsestr <= '9')
			{
				if(run)
					hibound = 10*hibound + *parsestr - '0';
				else
					lobound = 10*lobound + *parsestr - '0';
			}
			else
			{
				regerror( REGERR_INVALID_BRACKET );
				return 0;
			}
			parsestr++;
		}
		if(!run) hibound=lobound;	// only one number given

		// both numbers given but not in correct order
		if( hibound && lobound>hibound ) swap(lobound,hibound);

		// strip out the simple cases
		if(lobound==0 && hibound==1)
			op = '?';
		else if(lobound==0 && hibound==0)
			op = '*';
		else if(lobound==1 && hibound==0)
			op = '+';
	}
	parsestr++;


	// ??, *?, +? and {m,n}?
	// Lazy quantifiers 

	// ?+, *+, ++ and {m,n}+
	// Possessive quantifiers are a limited yet syntactically cleaner alternative to atomic grouping. 
	// They behave as normal greedy quantifiers, 
	// except that they will not give up part of their match for backtracking.
	bool lazy   = ( *parsestr == ( '?' ));
	bool possess= ( *parsestr == ( '+' ));

	if(lazy || possess)
		parsestr++;

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
		case ( '{' ):
		{
			if(lobound==0)				// => *
				flag = REGEX_WORST|REGEX_SPSTART;
			else							// => +
				flag = REGEX_WORST|REGEX_SPSTART|REGEX_HASWIDTH;
		}
	}


	if( op == ( '{' ) )
	{
		if( flags&REGEX_SIMPLE )
		{
			if(lobound==0)	// but hibound is defined
			{
				InsertNode(lazy?REGEX_L_RANGEMAX:REGEX_N_RANGEMAX, ret);
				cProgramm.insert( (char)((hibound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((hibound   )&0xFF), 1, ret+3);
			}
			else if(hibound==0)// but lobound is defined
			{
				InsertNode(lazy?REGEX_L_RANGEMIN:REGEX_N_RANGEMIN, ret);
				cProgramm.insert( (char)((lobound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((lobound   )&0xFF), 1, ret+3);
			}
			else	// lobound and hibound is defined
			{
				InsertNode(lazy?REGEX_L_RANGE:REGEX_N_RANGE, ret);
				cProgramm.insert( (char)((hibound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((hibound   )&0xFF), 1, ret+3);
				cProgramm.insert( (char)((lobound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((lobound   )&0xFF), 1, ret+3);
			}
		}
		else
		{
			if(lobound==0)	// but hibound is defined
			{
				InsertNode(lazy?REGEX_L_BRRGMAX:REGEX_N_BRRGMAX, ret);			// Either x 
				cProgramm.insert( (char)((hibound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((hibound   )&0xFF), 1, ret+3);
				AddCmdTail(ret, AddNode(REGEX_BACK));	// and loop 
				AddCmdTail(ret, ret);					// back 
				AddTail(ret, AddNode(REGEX_N_BRANCH));	// or 
				AddTail(ret, AddNode(REGEX_NOTHING));	// null. 
			}
			else if(hibound==0)// but lobound is defined
			{
				InsertNode(lazy?REGEX_L_BRRGMIN:REGEX_N_BRRGMIN, ret);			// Either x 
				cProgramm.insert( (char)((lobound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((lobound   )&0xFF), 1, ret+3);
				AddCmdTail(ret, AddNode(REGEX_BACK));	// and loop 
				AddCmdTail(ret, ret);					// back 
				AddTail(ret, AddNode(REGEX_N_BRANCH));	// or 
				AddTail(ret, AddNode(REGEX_NOTHING));	// null. 
			}
			else	// lobound and hibound is defined
			{
				InsertNode(lazy?REGEX_L_BRRANGE:REGEX_N_BRRANGE, ret);			// Either x 
				cProgramm.insert( (char)((hibound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((hibound   )&0xFF), 1, ret+3);
				cProgramm.insert( (char)((lobound>>8)&0xFF), 1, ret+3);
				cProgramm.insert( (char)((lobound   )&0xFF), 1, ret+3);
				AddCmdTail(ret, AddNode(REGEX_BACK));	// and loop 
				AddCmdTail(ret, ret);					// back 
				AddTail(ret, AddNode(REGEX_N_BRANCH));	// or 
				AddTail(ret, AddNode(REGEX_NOTHING));	// null. 
			}		
		}
	}
	else if( op == ( '*' ) )
	{
		if( flags&REGEX_SIMPLE )
		{
			InsertNode(lazy?REGEX_L_STAR:REGEX_N_STAR, ret);
		}
		else
		{	// Emit x* as (x&|), where & means "self". 
			InsertNode(lazy?REGEX_L_BRANCH:REGEX_N_BRANCH, ret);			// Either x 
			AddCmdTail(ret, AddNode(REGEX_BACK));	// and loop 
			AddCmdTail(ret, ret);					// back 
			AddTail(ret, AddNode(REGEX_N_BRANCH));	// or 
			AddTail(ret, AddNode(REGEX_NOTHING));	// null. 
		}
	}
	else if( op == ( '+' ) )
	{
		if( flags&REGEX_SIMPLE )
		{
			InsertNode(lazy?REGEX_L_PLUS:REGEX_N_PLUS, ret);
		}
		else
		{	// Emit x+ as x(&|), where & means "self". 
			size_t next = AddNode(lazy?REGEX_L_BRANCH:REGEX_N_BRANCH);	// Either 
			AddTail(ret, next);
			AddTail(AddNode(REGEX_BACK), ret);		// loop back 
			AddTail(next, AddNode(REGEX_N_BRANCH));	// or 
			AddTail(ret, AddNode(REGEX_NOTHING));	// null. 
		}
	}
	else if( op == ( '?' ) )
	{
		// Emit x? as (x|) 
		InsertNode(lazy?REGEX_L_BRANCH:REGEX_N_BRANCH, ret);			// Either x 
		AddTail(ret, AddNode(REGEX_N_BRANCH));	// or 
		size_t next = AddNode(REGEX_NOTHING);	// null. 
		AddTail(ret, next);
		AddCmdTail(ret, next);
	}

	if( possess )
	{
		InsertNode( REGEX_POSSESS, ret );
	}

	if( isRepeat(*parsestr) )
	{
		regerror( REGERR_NESTED_OP );
		return 0;
	}

	return ret;
}
///////////////////////////////////////////////////////////////////////////////
// adds chars to a set / creates it when not exist
// only possible before the assiciated node gets tailed
void CRegExp::CRegProgram::AddtoSet(size_t &pos, char b)
{
	if( pos==0 || pos>cProgramm.size() )
	{	// start a new set
		pos = cProgramm.size();
		cProgramm.append( (char)REGEX_SET );
		cProgramm.append( (char)b );
		cProgramm.append( (char)'\0' );
	}
	else
	{	// append to the set
		// check if b already exists in the set
		// rely on the set beeing terminated properly
		// also sort the set, but maybe not that important
		size_t i;
		for(i=pos+1; cProgramm[i] && cProgramm[i]<b; ++i);
		if( cProgramm[i]!=b )
			cProgramm.insert( (char)b, 1, i );

		// just insert without any checking
		//cProgramm.insert( (char)b, 1, pos+1 );
	}
}
bool CRegExp::CRegProgram::ComparetoSet(size_t node, char b) const
{
	const char *opsc = this->operand(node);
	bool found = false;
	while( *opsc )
	{
		switch(*opsc)
		{
		case REGEX_SPACE:
			found = ( stringcheck::isspace(b) );
			break;
		case REGEX_NOSPACE:
			found = ( !stringcheck::isspace(b) );
			break;
		case REGEX_XDIGIT:
			found = ( stringcheck::isxdigit(b) );
			break;
		case REGEX_DIGIT:
			found = ( stringcheck::isdigit(b) );
			break;
		case REGEX_NODIGIT:
			found = ( !stringcheck::isdigit(b) );
			break;
		case REGEX_UPPER:
			found = ( stringcheck::isupper(b) );
			break;
		case REGEX_WORD:
			found = ( stringcheck::isalnum(b) || b == '_');
			break;
		case REGEX_NOWORD:
			found = ( !stringcheck::isalnum(b) );
			break;
		case REGEX_LOWER:
			found = ( stringcheck::islower(b) );
			break;
		case REGEX_GRAPH:
			found = ( stringcheck::isgraph(b) );
			break;
		case REGEX_PRINT:
			found = ( stringcheck::isprint(b) );
			break;
		case REGEX_PUNCT:
			found = ( stringcheck::ispunct(b) );
			break;
		case REGEX_CNTRL:
			found = ( stringcheck::iscntrl(b) );
			break;
		case REGEX_SET:
		{	// start of the charset string
			++opsc;
			if( this->cConfig.cCaseInSensitive )
			{
				while( *opsc && (locase(*opsc)!=locase(b)) ) ++opsc;
				found = ( 0!=*opsc );
			}
			else
			{
				while( *opsc && (*opsc!=b) ) ++opsc;
				found = ( 0!=*opsc );
			}
			while(*opsc) opsc++;
			break;
		}
		case REGEX_SETRANGE:
		{	// get range limits
			char lo = *(++opsc);
			char hi = *(++opsc);
			if( this->cConfig.cCaseInSensitive )
				found = ( locase(b)>=locase(lo) && locase(b)<=locase(hi) );
			else
				found = ( b>=lo && b<=hi );
			break;
		}
		default:	// wrong opcode
			return false;
		}//end switch
		
		if(found)
		{	// predecision possible
			break;
		}
		// go to the next check
		opsc++;
	}//end while
	return found;
}

///////////////////////////////////////////////////////////////////////////////
// ParseAtom - the lowest level
// Optimization:  gobbles an entire sequence of ordinary characters so that
// it can turn them into a single node, which is smaller to store and
// faster to run.  Backslashed characters are exceptions, each becoming a
// separate node; the code is simpler that way and it's not worth fixing.
size_t CRegExp::CRegProgram::ParseAtom(const char*&parsestr, int &parcnt, int &flag)
{
	size_t ret=0;
	int flags=0;
	bool run;

	flag = REGEX_WORST;		// Tentatively. 
	do
	{
		// try to find a single atom, 
		// so in general we run through this only once 
		run = false;

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
				// direct use of posix charsets
				if( 0==memcmp(parsestr, ":space:]", 8) )
				{
					ret = AddNode(REGEX_SPACE);
					parsestr+=8;
				}
				else if( 0==memcmp(parsestr, ":xdigit:]", 9) )
				{
					ret = AddNode(REGEX_XDIGIT);
					parsestr+=9;
				}
				else if( 0==memcmp(parsestr, ":digit:]", 8) )
				{
					ret = AddNode(REGEX_DIGIT);
					parsestr+=8;
				}
				else if( 0==memcmp(parsestr, ":upper:]", 8) )
				{
					ret = AddNode(REGEX_UPPER);
					parsestr+=8;
				}
				else if( 0==memcmp(parsestr, ":lower:]", 8) )
				{
					ret = AddNode(REGEX_LOWER);
					parsestr+=8;
				}
				else if( 0==memcmp(parsestr, ":word:]", 7) )
				{
					ret = AddNode(REGEX_WORD);
					parsestr+=7;
				}
				else if( 0==memcmp(parsestr, ":graph:]", 8) )
				{
					ret = AddNode(REGEX_GRAPH);
					parsestr+=8;
				}
				else if( 0==memcmp(parsestr, ":print:]", 8) )
				{
					ret = AddNode(REGEX_PRINT);
					parsestr+=8;
				}
				else if( 0==memcmp(parsestr, ":punct:]", 8) )
				{
					ret = AddNode(REGEX_PUNCT);
					parsestr+=8;
				}
				else if( 0==memcmp(parsestr, ":cntrl:]", 8) )
				{
					ret = AddNode(REGEX_CNTRL);
					parsestr+=8;
				}
				
				else
				{
					// self defined charset
					int c;
					size_t setpos=0;

					// check for set complement
					if( *parsestr == '^')
					{	
						ret = AddNode(REGEX_ANYBUT);
						parsestr++;
					}
					else
						ret = AddNode(REGEX_ANYOF);

					// exceptions in a set
					if( (c=*parsestr) == '-' || c==']')	
					{	
						AddtoSet(setpos,c);
						parsestr++;
					}

					while( (c = *parsestr++ ) != '\0' && c != ']' )
					{
						if(c=='[')
						{	// using posix charsets inside a set
							if( 0==memcmp(parsestr, ":space:]", 8) )
							{
								AddValue(REGEX_SPACE);
								parsestr+=8;
								continue;
							}
							else if( 0==memcmp(parsestr, ":xdigit:]", 9) )
							{
								AddValue(REGEX_XDIGIT);
								parsestr+=9;
								continue;
							}
							else if( 0==memcmp(parsestr, ":digit:]", 8) )
							{
								AddValue(REGEX_DIGIT);
								parsestr+=8;
								continue;
							}
							else if( 0==memcmp(parsestr, ":upper:]", 8) )
							{
								AddValue(REGEX_UPPER);
								parsestr+=8;
								continue;
							}
							else if( 0==memcmp(parsestr, ":lower:]", 8) )
							{
								AddValue(REGEX_LOWER);
								parsestr+=8;
								continue;
							}
							else if( 0==memcmp(parsestr, ":word:]", 7) )
							{
								AddValue(REGEX_WORD);
								parsestr+=7;
								continue;
							}
							else if( 0==memcmp(parsestr, ":graph:]", 8) )
							{
								AddValue(REGEX_GRAPH);
								parsestr+=8;
								continue;
							}
							else if( 0==memcmp(parsestr, ":print:]", 8) )
							{
								AddValue(REGEX_PRINT);
								parsestr+=8;
								continue;
							}
							else if( 0==memcmp(parsestr, ":punct:]", 8) )
							{
								AddValue(REGEX_PUNCT);
								parsestr+=8;
								continue;
							}
							else if( 0==memcmp(parsestr, ":cntrl:]", 8) )
							{
								AddValue(REGEX_CNTRL);
								parsestr+=8;
								continue;
							}
							// otherwise run through the normal test
						}
						if( c == '\\')
						{	
							if( *parsestr == '\0')
							{	// escape at string end
								AddtoSet(setpos,c);
							}
							else
							{	// normal escaped character
								c =*parsestr++;
								switch(c)
								{
								case 't':
									AddtoSet(setpos,0x09);	// horizontal tab
									break;
								case 'n':
									AddtoSet(setpos,0x0A);	// line feed
									break;
								case 'r':
									AddtoSet(setpos,0x0D);	// carriage return
									break;
								case 'f':
									AddtoSet(setpos,0x0C);	// form feed
									break;
								case 'v':
									AddtoSet(setpos,0x0B);	// vertical space
									break;
								case 'e':
									AddtoSet(setpos,0x1B);	// escape character
									break;
								case 'a':
									AddtoSet(setpos,0x07);	// bell
									break;
								// whitespace
								case 's':
									AddValue(REGEX_SPACE);
									break;
								case 'S':
									AddValue(REGEX_NOSPACE);
									break;
								// digit
								case 'd':
									AddValue(REGEX_DIGIT);
									break;
								case 'D':
									AddValue(REGEX_NODIGIT);
									break;
								// letters and digits
								case 'w':
									AddValue(REGEX_WORD);
									break;
								case 'W':
									AddValue(REGEX_NOWORD);
									break;
								case 'b':
									AddValue(REGEX_WCHAR);
									break;
								case 'B':
									AddValue(REGEX_NOWCHAR);
									break;
								// char as hexcode
								case 'x':
								{	// assume usual case of two digit hexcode
									char buf[3] = { parsestr[0], parsestr[1], 0};
									if( !stringcheck::isxdigit(parsestr[1]) )
									{	// but check if it's really two digits
										buf[1] = 0;
										parsestr++;
									}
									else
										parsestr+=2;
									char val = stringtoue<char>(buf,16);
									if(val)	// dont accept EOS
										AddtoSet(setpos,val);
									break;
								}
								// otherwise just add the escaped char to the set
								default:
									AddtoSet(setpos,c);
									break;
								}
							}
						}
						else if( c!='-' && *parsestr != '-' )
						{	// normal character but not a range
							AddtoSet(setpos,c);
						}
						else
						{	// a range
							if( c!='-' ) // then *parsestr was '-'
								parsestr++;
							// parsestr is now pointing after the '-'
							// parsestr-2 is pointing before
							if( *parsestr == ']' || *parsestr == '\0')
							{	// minus at the end
								AddtoSet(setpos,'-');
							}
							else
							{	// normal minus
								char range_st = *(parsestr-2);
								char range_ed = *(parsestr);

								// swapping is not standard conform but more confortable
								if( range_st > range_ed )
									swap(range_st,range_ed);

								AddValue(REGEX_SETRANGE);
								AddValue(range_st);
								AddValue(range_ed);
								parsestr++;
							}
						}
					}
					AddValue('\0');

					if( c != ']')
					{
						regerror( REGERR_UNMATCHED_BRACE );
						return 0;
					}
				}
				flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
				break;
			}


			case '(':
			{
				if( parsestr[0] == '?' && parsestr[1] == '#' )
				{	// (?#comment)
					// skip the comment, also finish on the end of the RE
					char c=0;
					while( c!=')' && (c=*parsestr) ) *parsestr++;
					run = true;
				}
				else if(parsestr[0] == '?' && 
					(parsestr[1] == 'i' || parsestr[1] == 's' || parsestr[1] == 'm' || parsestr[1] == '-') )
				{
					// (?i) 	Turn on case insensitivity for the remainder of the regular expression.
					// (?-i) 	Turn off case insensitivity for the remainder of the regular expression.
					// (?s) 	Turn on "dot matches newline" for the remainder of the regular expression.
					// (?-s) 	Turn off "dot matches newline" for the remainder of the regular expression.
					// (?m) 	Caret and dollar match after and before newlines for the remainder of the regular expression.
					// (?-m) 	Caret and dollar only match at the start and end of the string for the remainder of the regular expression.
					// (?i-sm) 	Turns on the options "i" and "m", and turns off "s" for the remainder of the regular expression.
					// (?i-sm:regex)
					parsestr++;
					char config=0;
					while( *parsestr == 'i' || *parsestr == 's' || *parsestr == 'm' || *parsestr == '-' )
					{
						if( *parsestr == '-' )
						{
							if( parsestr[1] == 'i' )
							{
								config |=  REGEX_MASK_CASE;
								config &= ~REGEX_CONF_CASE;
							}
							else if( parsestr[1] == 's' )
							{
								config |=  REGEX_MASK_NLDT;
								config &= ~REGEX_CONF_NLDT;
							}
							else if( parsestr[1] == 'm' )
							{
								config |=  REGEX_MASK_NLSE;
								config &= ~REGEX_CONF_NLSE;
							}
							else
							{	// ignore
								continue;
								// might be better to report the error
							}
							parsestr+=2;
						}
						else
						{
							if( parsestr[0] == 'i' )
							{
								config |=  REGEX_MASK_CASE;
								config |=  REGEX_CONF_CASE;
							}
							else if( parsestr[0] == 's' )
							{
								config |=  REGEX_MASK_NLDT;
								config |=  REGEX_CONF_NLDT;
							}
							else if( parsestr[0] == 'm' )
							{
								config |=  REGEX_MASK_NLSE;
								config |=  REGEX_CONF_NLSE;
							}
							parsestr++;
						}
						
					}
					ret = AddNode(REGEX_CONFIG);
					AddValue( config );

					if(*parsestr==':')
					{	// sub-RE
						parsestr++;
						size_t repos = ParseExp(parsestr, REGEX_OFFPAREN, parcnt, flags);
						if( repos == 0)
							return 0;

						AddTail(ret, repos);		// config -> sub-RE
						
						size_t restpos = AddNode(REGEX_RESTORE);
						// relative offset to the associated config
						AddValue( ((restpos-ret)    )&0xFF );
						AddValue( ((restpos-ret)>> 8)&0xFF );

						AddTail(repos, restpos);	// sub-RE -> restore

					}
					else if( *parsestr!=')' )
					{	// config not properly closed

						return 0;
					}
					else
					{	// skip the trailing parenthesis
						parsestr++;
					}

				}
				else if(parsestr[0] == '?' &&  parsestr[1] == '>')
				{	// (?>regex) Atomic groups prevent the regex engine from backtracking back into the group 
					// (forcing the group to discard part of its match) after a match has been found for the group. 
					// Backtracking can occur inside the group before it has matched completely, 
					// and the engine can backtrack past the entire group, discarding its match entirely. 
					// Eliminating needless backtracking provides a speed increase. 
					// Atomic grouping is often indispensable when nesting quantifiers to prevent a catastrophic amount 
					// of backtracking as the engine needlessly tries pointless permutations of the nested quantifiers.

					ret = AddNode( REGEX_POSSESS );
					parsestr+=2;
					// sub-RE
					size_t repos = ParseExp(parsestr, REGEX_OFFPAREN, parcnt, flags);
					if( repos == 0)
						return 0;

					// ?+, *+, ++ and {m,n}+
					// Possessive quantifiers are a limited yet syntactically cleaner alternative to atomic grouping. 
					// They behave as normal greedy quantifiers, 
					// except that they will not give up part of their match for backtracking.

				}
				else if(parsestr[0] == '?' &&  ( parsestr[1] == '=' || parsestr[1] == '!') )
				{	///////////////////////////////////////////////////////////
					// (?=regex) Zero-width positive lookahead. 
					// Matches at a position where the pattern inside the lookahead can be matched. 
					// Matches only the position. It does not consume any characters or expand the match. 
					///////////////////////////////////////////////////////////
					// (?!regex) Zero-width negative lookahead.
					// Identical to positive lookahead, except that the overall match will only succeed 
					// if the regex inside the lookahead fails to match.
					///////////////////////////////////////////////////////////

					ret = AddNode( (parsestr[1] == '=')?REGEX_LAHEADT:REGEX_LAHEADF );

					parsestr+=2;
													
					// disable parenthesis memory for all parenthesis in the sub-RE
					cConfig.cParenDisable = true;

					// sub-RE
					size_t repos = ParseExp(parsestr, REGEX_OFFPAREN, parcnt, flags);
					if( repos == 0)
						return 0;

					cConfig.cParenDisable = false;
				}
				else if(parsestr[0] == '?' &&  parsestr[1] == '<' && ( parsestr[2] == '=' || parsestr[2] == '!') )
				{	///////////////////////////////////////////////////////////
					// (?<=text) Zero-width positive lookbehind.
					// Matches at a position to the left of which text appears. 
					// Since regular expressions cannot be applied backwards, 
					// the test inside the lookbehind can only be plain text.
					///////////////////////////////////////////////////////////
					// (?<!text) Zero-width negative lookbehind. 
					// Matches at a position if the text does not appear to the left of that position.
					///////////////////////////////////////////////////////////

					ret = AddNode( (parsestr[2] == '=')?REGEX_LBEHINDT:REGEX_LBEHINDF );

					parsestr+=3;
					// add EOS
					cProgramm.insert( '\0', 1, ret+3);
					while( *parsestr && *parsestr!= ')' )
					{	// add the string in inverse order
						cProgramm.insert(*parsestr++, 1, ret+3);
					}
					if( *parsestr!= ')' )
					{	// error incorrect parenthesis

						return 0;
					}
					else
						parsestr++;
				}
				else
				{	//(?:non storing RE)
					int localflag = ( parsestr[0] == '?' && parsestr[1] == ':' );
					if(localflag) parsestr+=2;
					// diable parenthesis memory on nonstoring parenthesis and in lookahead sub-RE's
					ret = ParseExp(parsestr, (localflag || cConfig.cParenDisable)?REGEX_OFFPAREN:REGEX_PAREN, parcnt, flags);
					if( ret == 0)
						return 0;
					flag |= flags&(REGEX_HASWIDTH|REGEX_SPSTART);
				}
				break;
			}
			case '\0':
			case '|':
			case ')':
				// supposed to be caught earlier 
				regerror( REGERR_INTERNAL_UNEXPECTED_CHAR );
				return 0;
			case '?':
			case '+':
			case '*':
			case '{':
				regerror( REGERR_OP_FOLLOWS_NOTHING );
				return 0;
			case '\\':
			{
				char c = *parsestr++;
				if( stringcheck::isdigit(c) )
				{	
					int level = c - '0';

					while( stringcheck::isdigit(*parsestr) )
					{
						int tmp = 10*level + *parsestr -'0'; // also add the following number
						if( tmp > parcnt)
						{	// seems to be too big
							break;
						}
						else
						{	// take the number and test the next for a digit
							level = tmp;
							parsestr++;
						}
					}
					if( level > parcnt)
					{	// only accept backreferences number smaller than current acceptec parenthesis count
						// otherwise just ignore it
						run = true;
						break;
					}
					else
					{
						ret = AddNode(REGEX_BACKREF);
						AddValue( (level    )&0xFF );
						AddValue( (level>> 8)&0xFF );

						flag |= REGEX_HASWIDTH;
					}
				}
				else
				{
					switch( c )
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
						case 'A':
							ret = AddNode(REGEX_BOS);
							break;
						case 'z':
							ret = AddNode(REGEX_EOS);
							break;
						case 'Z':
							ret = AddNode(REGEX_EOS2);
							break;
						case 'b':
							ret = AddNode(REGEX_WCHAR);
							break;
						case 'B':
							ret = AddNode(REGEX_NOWCHAR);
							break;
						case 's':
							ret = AddNode(REGEX_SPACE);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'S':
							ret = AddNode(REGEX_NOSPACE);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'd':
							ret = AddNode(REGEX_DIGIT);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'D':
							ret = AddNode(REGEX_NODIGIT);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'w':
							ret = AddNode(REGEX_WORD);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'W':
							ret = AddNode(REGEX_NOWORD);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'h':
							ret = AddNode(REGEX_XDIGIT);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'H':
							ret = AddNode(REGEX_NOXDIGIT);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'l':
							ret = AddNode(REGEX_LETTER);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'L':
							ret = AddNode(REGEX_NOLETTER);
							flag |= REGEX_HASWIDTH|REGEX_SIMPLE;
							break;
						case 'o': // octal 3 digits
						{
							int result=0;
							const char *p = parsestr;
							do
							{
								int parsechar = *p;
								if( parsechar<'0' || parsechar > '7' )
									break;
								parsechar -= '0';
								result = (result<<3) | (parsechar&0x07);
								p++;
							} while( *p && p<parsestr+3);
							if(result)
							{
								ret = AddNode(REGEX_EXACTLY);
								AddValue( result&0xFF );
								AddValue('\0');
							}
							parsestr = p;
							break;
						}
						case 'x': // hex
						{
							int result=0;
							const char *p = parsestr;
							do
							{
								int parsechar = *p;
								if(parsechar>='0' && parsechar<='9')
									parsechar -= '0';
								else if(parsechar>='a' && parsechar<='f')
									parsechar -= 'a'-10;
								else if(parsechar>='A' && parsechar<='F')
									parsechar -= 'A'-10;
								else
									break;

								result = (result<<4) | (parsechar&0x0F);
								p++;
							} while( *p && p<parsestr+2);
							if(result)
							{
								ret = AddNode(REGEX_EXACTLY);
								AddValue( result&0xFF );
								AddValue('\0');
							}
							parsestr = p;
							break;
						}						
						default:
							// Handle general quoted chars in exact-match routine
							goto de_fault;
					}
				}
				break;
			}
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
			de_fault:
			default:
			{
				const char *regprev;
				register char ch;

				parsestr--; // Look at cur char
				ret = AddNode(REGEX_EXACTLY);
				for ( regprev = 0 ; ; )
				{
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

					case '?': case '+': case '*': case '{':
						if( !regprev) 	// If just ch in str,
							goto magic;	// use it
						// End mult-char string one early
						parsestr = regprev; // Back up parse
						goto done;

					case '\\':
						AddValue(ch);	// Cur char OK
						switch( parsestr[1] )
						{	// Look after '\\' 
						case 't':
							AddValue(0x09);	// horizontal tab
							parsestr+=2;
							break;
						case 'n':
							AddValue(0x0A);	// line feed
							parsestr+=2;
							break;
						case 'r':
							AddValue(0x0D);	// carriage return
							parsestr+=2;
							break;
						case 'f':
							AddValue(0x0C);	// form feed
							parsestr+=2;
							break;
						case 'v':
							AddValue(0x0B);	// vertical space
							parsestr+=2;
							break;
						case 'e':
							AddValue(0x1B);	// escape character
							parsestr+=2;
							break;
						case 'a':
							AddValue(0x07);	// bell
							parsestr+=2;
							break;
						case '\0':
						case '<':
						case '>':
						case 'A':
						case 'z':
						case 'Z':
						case 's':
						case 'S':
						case 'd':
						case 'D':
						case 'w':
						case 'W':
						case 'b':
						case 'B':
						case 'h':
						case 'H':
						case 'l':
						case 'L':
						case 'o':
						case 'x':
						// backref
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
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
		}// end switch
	}while(run);
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::CRegProgram::MatchTry(const char* base, const char* str, vector< vector<CRegExp::CFinds> >& finds) const
{
	size_t i;
	const char* reginput = str;
	map<size_t, size_t> imap;

	if( finds.size()<1 ) finds.resize(1);

	for (i = 1; i<finds.size(); i--)
	{
		finds[i].clear();
	}
	if( MatchMain(base, str, finds, 1, reginput, imap) )
	{
		finds[0].clear();
		finds[0].append( CRegExp::CFinds(str,reginput) );
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
bool CRegExp::CRegProgram::MatchMain(const char* base, const char* str, vector< vector<CRegExp::CFinds> >& finds, size_t progstart, const char*&reginput, map<size_t, size_t>& imap) const
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
			///////////////////////////////////////////////////////////////////
			// check positions
			// those elements do not have a length so don't increment the counter
			case REGEX_BOL:
				if( reginput != base  && (!this->cConfig.cMultiLineSE || (*reginput != '\r' && *reginput != '\n')) )
					return false;
				break;
			case REGEX_EOL:
				if( *reginput != '\0' && (!this->cConfig.cMultiLineSE || (*reginput != '\r' && *reginput != '\n')) )
					return false;
				break;
			case REGEX_WORDA:
				// current must be a letter, digit, or _ 
				if( !stringcheck::isalnum(*reginput) && *reginput != '_' )
					return false;
				// previous must be BOL or nonword
				if( reginput != base && reginput[-1] != '\r' && reginput[-1] != '\n' &&
					(stringcheck::isalnum(reginput[-1]) || reginput[-1] == '_') )
					return false;
				break;
			case REGEX_WORDZ:
				// current must be EOL or nonword
				if( reginput[0] != '\0' && reginput[0] != '\r' && reginput[0] != '\n' &&
					(stringcheck::isalnum(reginput[0]) || reginput[0] == '_') )
					return false;
				// previous must be a letter, digit, or _ 
				if( !stringcheck::isalnum(reginput[1]) && reginput[1] != '_' )
					return false;
				break;
			case REGEX_BOS:
				// begin of string only
				if( reginput != base )
					return false;
				break;
			case REGEX_EOS:
				// end of string only
				if( *reginput != '\0' )
					return false;
				break;
			case REGEX_EOS2:
			{	// end of string or end of line when eos is after that eol
				if( *reginput != '\0' )
				{	// do the eos after the eol check
					const char *ptr = reginput;
					while( *ptr == '\r' || *ptr == '\n' ) // skip both linefeed and return
						ptr++;
					if( *ptr != '\0' )
						return false;
				}
				break;
			}
			case REGEX_WCHAR:
			{	// Matches the position between a word character (anything matched by \w) 
				// and a non-word character (only one direction )

				// false when BOL or nonword
				bool prev = ( reginput != base     && reginput[-1] != '\r' && reginput[-1] != '\n' &&
					(stringcheck::isalnum(reginput[-1]) || reginput[-1] == '_') );

				// false when EOL  or nonword
				bool curr = ( reginput[ 0] != '\0' && reginput[ 0] != '\r' && reginput[ 0] != '\n' &&
					(stringcheck::isalnum(reginput[ 0]) || reginput[ 0] == '_') );


				if( !prev ^ curr )
					return false;
								
				break;
			}
			case REGEX_NOWCHAR:
			{
				// Matches at the position between two word characters (i.e the position between \w\w) 
				// as well as at the position between two non-word characters

				// false when BOL or nonword
				bool prev = ( reginput != base     && reginput[-1] != '\r' && reginput[-1] != '\n' &&
					(stringcheck::isalnum(reginput[-1]) || reginput[-1] == '_') );

				// false when EOL  or nonword
				bool curr = ( reginput[ 0] != '\0' && reginput[ 0] != '\r' && reginput[ 0] != '\n' &&
					(stringcheck::isalnum(reginput[ 0]) || reginput[ 0] == '_') );

				if( prev^curr )
					return false;

				break;
			}
			///////////////////////////////////////////////////////////////////
			// check on single character base
			// increment position by each char
			case REGEX_SPACE:
				if( !stringcheck::isspace(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_NOSPACE:
				if( stringcheck::isspace(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_XDIGIT:
				if( !stringcheck::isxdigit(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_NOXDIGIT:
				if( stringcheck::isxdigit(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_DIGIT:
				if( !stringcheck::isdigit(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_NODIGIT:
				if( stringcheck::isdigit(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_WORD:
				if( !stringcheck::isalnum(*reginput) && *reginput != '_')
					return false;
				reginput++;
				break;
			case REGEX_NOWORD:
				if( stringcheck::isalnum(*reginput) || *reginput == '_')
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
			case REGEX_LETTER:
				if( !stringcheck::isalpha(*reginput) )
					return false;
				reginput++;
				break;
			case REGEX_NOLETTER:
				if( stringcheck::isalpha(*reginput) )
					return false;
				reginput++;
				break;

			case REGEX_ANY:
				if( *reginput == '\0'  || (this->cConfig.cMultiLineDT && (*reginput == '\r' || *reginput == '\n')) )
					return false;
				reginput++;
				break;
			///////////////////////////////////////////////////////////////////
			// check on match base
			case REGEX_EXACTLY:
			{
				const char* prun = this->operand(scan);
				const char* opsc = reginput;

				// Inline the first character for speed. 
				if( (!this->cConfig.cCaseInSensitive &&        *prun         != *opsc ) ||
					( this->cConfig.cCaseInSensitive && locase(*prun) != locase(*opsc)) )
					return false;

				if(this->cConfig.cCaseInSensitive)
				{
					while( *prun && locase(*prun) == locase(*opsc) ) ++prun,++opsc;
				}
				else
				{
					while( *prun && *prun==*opsc ) ++prun,++opsc;
				}
				if( *prun )	// found a mismatch
					return false;
				reginput = opsc;

				break;
			}
			case REGEX_BACKREF:
			{
				unsigned char *ip = (unsigned char *)this->operand(scan);
				size_t no = (((ushort)ip[0])    )
							| (((ushort)ip[1])<< 8);

				if( finds.size() > no && finds[no].size() > 0 &&
					finds[no].last().startp!=NULL && finds[no].last().endp!=NULL )
				{
					const char* prun = finds[no].last().startp;
					const char* pend = finds[no].last().endp;
					const char* opsc = reginput;
					if(this->cConfig.cCaseInSensitive)
					{
						while( prun<pend && locase(*prun) == locase(*opsc) ) ++prun,++opsc;
					}
					else
					{
						while( prun<pend && *prun==*opsc ) ++prun,++opsc;
					}
					if( prun<pend )	// found a mismatch
						return false;
					reginput = opsc;
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// lookahead/lookbehind
			case REGEX_LAHEADT:
			case REGEX_LAHEADF:
			{
				const char* input = reginput;
				bool ret = MatchMain(base, str, finds, scan+3, input, imap);
				if( ret ^ (REGEX_LAHEADT==this->command(scan)) )
					return false;
				break;
			}
			case REGEX_LBEHINDT:
			case REGEX_LBEHINDF:
			{
				const char* opsc = reginput-1;
				const char* prun = this->operand(scan);

				if(this->cConfig.cCaseInSensitive)
				{
					while( *prun && opsc>=base && locase(*prun) == locase(*opsc) ) ++prun,--opsc;
				}
				else
				{
					while( *prun && opsc>=base && *prun==*opsc ) ++prun,--opsc;
				}
				if( (*prun=='\0') ^ (REGEX_LBEHINDT==this->command(scan)) )	// found a mismatch
					return false;
				break;
			}
			///////////////////////////////////////////////////////////////////
			// check on single character on a set
			case REGEX_POSSESS:
			{
				const char* input = reginput;
				if( !MatchMain(base, str, finds, scan+3, reginput, imap) )
				{
					reginput = input;
					return false;
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// check on single character on a set
			case REGEX_ANYOF:
			case REGEX_ANYBUT:
			{
				bool compare = (REGEX_ANYOF==this->command(scan));
				bool found = ComparetoSet(scan, *reginput);

				// found && !compare || !found && compare -> matching error
				if( compare ^ found )
					return false;

				reginput++;
				break;
			}
			///////////////////////////////////////////////////////////////////
			// parenthesis
			case REGEX_OPEN:
			{
				unsigned char *ip = (unsigned char *)this->operand(scan);
				size_t no =   (((ushort)ip[0])    )
							| (((ushort)ip[1])<< 8);

				if(no!=0)
				{	// only store !=0 brackets 

					if( finds.size() <= no) finds.resize(no+1);
					// add a new (possible) find
					finds[no].append( CRegExp::CFinds(reginput,NULL) );
				}

				if( MatchMain(base, str, finds, next, reginput, imap) )
				{
					return true;
				}
				else
				{
					if(no!=0)
					{	// remove the last find on error
						finds[no].strip(1);
						// remove the whole array if empty
						if( finds[no].size() == 0 )
							finds.resize(no);
					}
					return false;
				}
				//break;
			}
			case REGEX_CLOSE:
			{
				unsigned char *ip = (unsigned char *)this->operand(scan);
				size_t no =   (((ushort)ip[0])    )
							| (((ushort)ip[1])<< 8);

				if(no!=0 && finds.size() > no && finds[no].size()>0)
				{	// only store !=0 brackets
					// and if there was an open before
					// that has stored the start pointer (paranoia check)
					finds[no].last().endp = reginput;
				}
				return MatchMain(base, str, finds, next, reginput, imap);

				// don't need to remove the find on error
				// since it will get cleaned by the OPEN 
				// or overwritten when returning here again
				//break;
			}
			///////////////////////////////////////////////////////////////////
			// config
			case REGEX_CONFIG:
			{
				// safe the current config
				imap[scan] = this->cConfig.get();
				
				// set the new config
				char c = *this->operand(scan);

				if( c & REGEX_MASK_CASE )
					const_cast<CRegProgram*>(this)->cConfig.cCaseInSensitive = (c & REGEX_CONF_CASE)!=0;
				if( c & REGEX_MASK_NLDT )
					const_cast<CRegProgram*>(this)->cConfig.cMultiLineDT = (c & REGEX_CONF_NLDT)!=0;
				if( c & REGEX_MASK_NLSE )
					const_cast<CRegProgram*>(this)->cConfig.cMultiLineSE = (c & REGEX_CONF_NLSE)!=0;

				bool ret = MatchMain(base, str, finds, next, reginput, imap);

				// restore the former config
				const_cast<CRegExp::CRegProgram*>(this)->cConfig = imap[scan];
				return ret;
			}
			case REGEX_RESTORE:
			{
				unsigned char *ip = (unsigned char *)this->operand(scan);
				size_t no = (((ushort)ip[0])    )
							| (((ushort)ip[1])<< 8);
				// offset is backwards so the position of the associated config is:
				no = scan-no;
				if( imap.exists(no) )
				{	// restore the config
					const_cast<CRegExp::CRegProgram*>(this)->cConfig = imap[no];
				}
				// otherwise something went wrong

				break;
			}

			///////////////////////////////////////////////////////////////////
			// control nodes
			case REGEX_NOTHING:
				break;
			case REGEX_BACK:
				break;

			case REGEX_N_BRANCH:
			case REGEX_L_BRANCH:
			case REGEX_N_BRRANGE:
			case REGEX_N_BRRGMIN:
			case REGEX_N_BRRGMAX:
			case REGEX_L_BRRANGE:
			case REGEX_L_BRRGMIN:
			case REGEX_L_BRRGMAX:
			{
				if( this->command(next) != REGEX_N_BRANCH &&
					this->command(next) != REGEX_L_BRANCH &&
					this->command(next) != REGEX_N_BRRANGE &&
					this->command(next) != REGEX_N_BRRGMIN &&
					this->command(next) != REGEX_N_BRRGMAX &&
					this->command(next) != REGEX_L_BRRANGE &&
					this->command(next) != REGEX_L_BRRGMIN &&
					this->command(next) != REGEX_L_BRRGMAX
				  )
				{	// Avoid recursion.

					size_t ofs = 3;
					if( this->command(scan) == REGEX_N_BRRGMIN ||
						this->command(scan) == REGEX_N_BRRGMAX ||
						this->command(scan) == REGEX_L_BRRGMIN ||
						this->command(scan) == REGEX_L_BRRGMAX  )
						ofs = 5;
					else if( this->command(scan) == REGEX_N_BRRANGE ||
							 this->command(scan) == REGEX_L_BRRANGE  )
						ofs = 7;
					
					next = scan+ofs;							
				}
				else
				{

					while( this->command(scan) == REGEX_N_BRANCH ||
						   this->command(scan) == REGEX_L_BRANCH ||
						   this->command(scan) == REGEX_N_BRRANGE ||
						   this->command(scan) == REGEX_N_BRRGMIN ||
						   this->command(scan) == REGEX_N_BRRGMAX ||
						   this->command(scan) == REGEX_L_BRRANGE ||
						   this->command(scan) == REGEX_L_BRRGMIN ||
						   this->command(scan) == REGEX_L_BRRGMAX
						)
					{

						size_t min = 0;
						size_t max = 0;
						size_t ofs = 3;

						if( this->command(scan) == REGEX_N_BRRGMIN ||
							this->command(scan) == REGEX_N_BRRGMAX ||
							this->command(scan) == REGEX_L_BRRGMIN ||
							this->command(scan) == REGEX_L_BRRGMAX  )
							ofs = 5;
						else if( this->command(scan) == REGEX_N_BRRANGE ||
								 this->command(scan) == REGEX_L_BRRANGE  )
							ofs = 7;

						if( this->command(scan) == REGEX_N_BRRANGE ||
							this->command(scan) == REGEX_L_BRRANGE )
						{
							const uchar*ptr = (const uchar*)this->operand(scan);
							min= (((ushort)ptr[0])) | (((ushort)ptr[1])<<8);
							max= (((ushort)ptr[2])) | (((ushort)ptr[3])<<8);
						}
						else if( this->command(scan) == REGEX_N_BRRGMIN ||
								 this->command(scan) == REGEX_L_BRRGMIN )
						{
							const uchar*ptr = (const uchar*)this->operand(scan);
							min= (((ushort)ptr[0])) | (((ushort)ptr[1])<<8);
						}
						else if( this->command(scan) == REGEX_N_BRRGMAX || 
								 this->command(scan) == REGEX_L_BRRGMAX )
						{
							const uchar*ptr = (const uchar*)this->operand(scan);
							max= (((ushort)ptr[0])) | (((ushort)ptr[1])<<8);
						}

						if( this->command(scan) == REGEX_N_BRANCH )
						{	// normal branch or
							// gready star/plus
							// maximize the count on this slot and then try the rest
							// reduce the count successively (by returning) if rest fails

							// safe the pointer in case of an error
							const char* save = reginput;

							if( MatchMain(base, str, finds, scan+ofs, reginput, imap) )
								return true;

							// restore the last valid pointer
							reginput = save;

							scan = this->nextcommand(scan);

							// stay in the while loop
						}
						else if( this->command(scan) == REGEX_L_BRANCH )
						{
							// lazy star/plus
							// have minimum count on the slot to try the rest
							// increase successively if rest fails
							// REGEX_BRPLUS already has matched once when reaching this
							// while REGEX_BRSTAR starts the first match

							const size_t nextcommand = this->nextcommand(scan);

							// safe the pointer in case of an error
							const char* save = reginput;

							if( MatchMain(base, str, finds, nextcommand, reginput, imap) )
								return true;
							// otherwise the rest of the RE has failed
							reginput = save;
							// so we try to match with the variable expression once again
							// which will loop us back to the beginning.
							// can break the whole thing when this fails
							if( MatchMain(base, str, finds, scan+ofs, reginput, imap) )
								return true;
							else
							{	
								reginput = save;
								return false;
							}
							// always return here
						}
						else if( this->command(scan) == REGEX_N_BRRANGE ||
								 this->command(scan) == REGEX_N_BRRGMIN ||
								 this->command(scan) == REGEX_N_BRRGMAX )
						{
							// gready range
							// maximize the count on this slot and then try the rest
							// reduce the count successively (by returning) if rest fails

							// safe the pointer in case of an error
							const char* save = reginput;

							// count the runs
							if( imap.exists(scan) )
								imap[scan]++;
							else
								imap.insert(scan, 1);


							if( !max || imap[scan] <= max )
							{
								if( MatchMain(base, str, finds, scan+ofs, reginput, imap) )
									return true;
								else
								{
									reginput = save;
									imap[scan]--;
									if( imap[scan] < min )
										return false;
								}
							}
							const size_t nextcommand = this->nextcommand(scan);

							// try the following RE
							if( MatchMain(base, str, finds, nextcommand, reginput, imap) )
								return true;
							else
								return false;
							// stay return here
						}
						else if( this->command(scan) == REGEX_L_BRRANGE ||
								 this->command(scan) == REGEX_L_BRRGMIN ||
								 this->command(scan) == REGEX_L_BRRGMAX )
						{
							// lazy range
							// have minimum count on the slot to try the rest
							// increase successively if rest fails
							// REGEX_BRPLUS already has matched once when reaching this
							// while REGEX_BRSTAR starts the first match

							// safe the pointer in case of an error
							const char* save = reginput;
							// count the runs
							if( imap.exists(scan) )
								imap[scan]++;
							else
								imap.insert(scan, 1);


							if( imap[scan] <= min )
							{
								if( MatchMain(base, str, finds, scan+ofs, reginput, imap) )
									return true;
								else
								{
									reginput = save;
									imap[scan]--;
									return false;
								}
							}

							reginput = (char*)save;
							const size_t nextcommand = this->nextcommand(scan);

							if( MatchMain(base, str, finds, nextcommand, reginput, imap) )
								return true;
							// otherwise the rest of the RE has failed
							reginput = save;

							// so we try to match with the variable expression once again
							// which will loop us back to the beginning
							// can break the whole thing when this fails
							if( MatchMain(base, str, finds, scan+ofs, reginput, imap) )
								return true;
							else
							{
								reginput = save;
								imap[scan]--;
								return false;
							}
							/////////////////
						}
					}
					return false;
					// NOTREACHED 
				}
				break;
			}
/*			case REGEX_BRANCH:
			{
				const char* save = reginput;
				if( this->command(next) != REGEX_BRANCH)	// No choice. 
					next = scan+3;							// Avoid recursion. 
				else
				{
					while(this->command(scan) == REGEX_BRANCH)
					{
						if( imap.exists(scan) )
							imap[scan]++;
						else
							imap[scan]=1;

printf("imap s %i = %i\n", scan, imap[scan]);

						if( MatchMain(base, str, finds, scan+3, reginput, imap) )
							return true;

						imap[scan]--;
printf("imap r %i = %i\n", scan, imap[scan]);

						reginput = (char*)save;
						scan = this->nextcommand(scan);
					}
					return false;
					// NOTREACHED 
				}
				break;
			}
*/			///////////////////////////////////////////////////////////////////
			// simple variable size nodes
			// only generated for single character of variable number
			case REGEX_N_STAR:
			case REGEX_N_PLUS:
			case REGEX_N_RANGE:
			case REGEX_N_RANGEMIN:
			case REGEX_N_RANGEMAX:
			case REGEX_L_STAR: 
			case REGEX_L_PLUS:
			case REGEX_L_RANGE:
			case REGEX_L_RANGEMIN:
			case REGEX_L_RANGEMAX:
			{
				const char nextch = (this->command(next) == REGEX_EXACTLY) ? *this->operand(next) : '\0';
				size_t no;
				const char* save = reginput;
				size_t min = 0;
				size_t max = 0;
				size_t ofs = 3;
				//if(this->command(scan) == REGEX_STAR)	min=0, max=0, ofs=3;
				if( this->command(scan) == REGEX_N_PLUS || 
					this->command(scan) == REGEX_L_PLUS )
					min=1;//, max=0, ofs=3;
				else if( this->command(scan) == REGEX_N_RANGE ||
						 this->command(scan) == REGEX_L_RANGE )
				{
					const uchar*ptr = (const uchar*)this->operand(scan);
					min= (((ushort)ptr[0])) | (((ushort)ptr[1])<<8);
					max= (((ushort)ptr[2])) | (((ushort)ptr[3])<<8);
					ofs=7;
				}
				else if( this->command(scan) == REGEX_N_RANGEMIN ||
						 this->command(scan) == REGEX_L_RANGEMIN )
				{
					const uchar*ptr = (const uchar*)this->operand(scan);
					min= (((ushort)ptr[0])) | (((ushort)ptr[1])<<8);
					ofs=5;
				}
				else if( this->command(scan) == REGEX_N_RANGEMAX ||
						 this->command(scan) == REGEX_L_RANGEMAX )
				{
					const uchar*ptr = (const uchar*)this->operand(scan);
					max= (((ushort)ptr[0])) | (((ushort)ptr[1])<<8);
					ofs=5;
				}

				if( this->command(scan) == REGEX_L_STAR ||
					this->command(scan) == REGEX_L_PLUS ||
					this->command(scan) == REGEX_L_RANGE ||
					this->command(scan) == REGEX_L_RANGEMIN ||
					this->command(scan) == REGEX_L_RANGEMAX )
				{	// lazy operator
					// start with min count and go up to max when fails
					no=MatchRepeat(base, scan+ofs,reginput);
					if(no>=min)
					{
						if( !max || no<max ) max = no;
						no = min;
						for(; no<max ; ++no)
						{
							reginput = save + no;
							// If it could work, try it. 
							if( nextch == '\0' || *reginput == nextch)
								if( MatchMain(base, str, finds, next, reginput, imap) )
									return true;
						}
					}
				}
				else
				{	// gready 
					// start with max count and go down to min when fails
					no=MatchRepeat(base, scan+ofs,reginput);
					if(max && no>max) no = max;
					for(no++; no>min ; --no)	// find another way to detect the unsigned overflow
					{
						reginput = save + no - 1;
						// If it could work, try it. 
						if( nextch == '\0' || *reginput == nextch)
							if( MatchMain(base, str, finds, next, reginput, imap) )
								return true;
					}
				}
				return false;
				//break;
			}
			///////////////////////////////////////////////////////////////////
			// 
			case REGEX_END:
				return true;	// Success! 
				break;
			default:
				regerror( REGERR_CORRUPTION );
				return false;
				break;
		}
	}

	return true;
	// the rest is actually not needed, since we fall through here on success also
	// but I leave the rest as reference
	// We get here only if there's trouble -- normally "case END" is
	// the terminating point.
//	regerror( REGERR_CORRUPTED_POINTERS );
//	return false;
}
///////////////////////////////////////////////////////////////////////////////
// MatchRepeat - report how many times something simple would match
size_t CRegExp::CRegProgram::MatchRepeat(const char* base, size_t node, const char*&reginput ) const
{
	const char* scan = reginput;
	char ch;

	switch( this->command(node) )
	{
		case REGEX_ANY:
			while( *scan != '\0'  && (!this->cConfig.cMultiLineDT || (*scan != '\r' || *scan != '\n')) )
				++scan;
			break;
		case REGEX_EXACTLY:
			ch = *this->operand(node);
			if(this->cConfig.cCaseInSensitive)
				while( locase(*scan) == locase(ch) ) ++scan;
			else
				while( *scan == ch ) ++scan;
				
			break;
		case REGEX_ANYBUT:
		case REGEX_ANYOF:
		{
			bool compare = (REGEX_ANYOF!=this->command(node));
			for( ; *scan && (compare^ComparetoSet(node, *scan)); ++scan);
			break;
		}
		case REGEX_SPACE:
			while( stringcheck::isspace(*scan) ) ++scan;
			break;
		case REGEX_NOSPACE:
			while( !stringcheck::isspace(*scan) ) ++scan;
			break;
		case REGEX_XDIGIT:
			while( stringcheck::isxdigit(*scan) ) ++scan;
			break;
		case REGEX_NOXDIGIT:
			while( !stringcheck::isxdigit(*scan) ) ++scan;
			break;
		case REGEX_DIGIT:
			while( stringcheck::isdigit(*scan) ) ++scan;
			break;
		case REGEX_NODIGIT:
			while( !stringcheck::isdigit(*scan) ) ++scan;
			break;
		case REGEX_WORD:
			while( stringcheck::isalnum(*scan) || *scan == '_') ++scan;
			break;
		case REGEX_NOWORD:
			while( !stringcheck::isalnum(*scan) && *scan != '_') ++scan;
			break;
		case REGEX_UPPER:
			while( stringcheck::isupper(*scan) ) ++scan;
			break;
		case REGEX_LOWER:
			while( stringcheck::islower(*scan) ) ++scan;
			break;
		case REGEX_GRAPH:
			while( stringcheck::isgraph(*scan) ) ++scan;
			break;
		case REGEX_PRINT:
			while( stringcheck::isprint(*scan) ) ++scan;
			break;
		case REGEX_PUNCT:
			while( stringcheck::ispunct(*scan) ) ++scan;
			break;
		case REGEX_CNTRL:
			while( stringcheck::iscntrl(*scan) ) ++scan;
			break;
		case REGEX_LETTER:
			while( stringcheck::isalpha(*scan) ) ++scan;
			break;
		case REGEX_NOLETTER:
			while( !stringcheck::isalpha(*scan) ) ++scan;
			break;
		default:		// Oh dear.  Called inappropriately. 
			regerror( REGERR_BAD_REGREPEAT );
			return(0);	// Best compromise. 
			break;
	}
	return(scan-reginput);
}


#ifdef DEBUG
///////////////////////////////////////////////////////////////////////////////
// GetPrintCmd - printable representation of opcode
const char * CRegExp::CRegProgram::GetPrintCmd( size_t pos ) const
{
	const char *p=":corrupted opcode";

	switch (this->command(pos))
	{
	case REGEX_BOL:		p= ":bol"; break;
	case REGEX_EOL:		p= ":eol"; break;
	case REGEX_ANY:		p= ":any"; break;
	case REGEX_ANYOF:	p= ":anyof"; break;
	case REGEX_ANYBUT:	p= ":anybut"; break;
	case REGEX_EXACTLY:	p= ":exact"; break;
	case REGEX_NOTHING:	p= ":nothing"; break;
	case REGEX_BACK:	p= ":back"; break;
	case REGEX_END:		p= ":end"; break;
	case REGEX_WORDA:	p= ":worda"; break;
	case REGEX_WORDZ:	p= ":wordz"; break;

	case REGEX_SPACE:	p= ":space"; break;
	case REGEX_NOSPACE:	p= ":nospace"; break;
	case REGEX_XDIGIT:	p= ":xdigit"; break;
	case REGEX_NOXDIGIT:p= ":no xdigit"; break;
	case REGEX_DIGIT:	p= ":digit"; break;
	case REGEX_NODIGIT:	p= ":nodigit"; break;
	case REGEX_UPPER:	p= ":upper"; break;
	case REGEX_LOWER:	p= ":lower"; break;
	case REGEX_WORD:	p= ":word"; break;
	case REGEX_NOWORD:	p= ":noword"; break;
	case REGEX_GRAPH:	p= ":graph"; break;
	case REGEX_PRINT:	p= ":print"; break;
	case REGEX_PUNCT:	p= ":punct"; break;
	case REGEX_CNTRL:	p= ":cntrl"; break;
	case REGEX_WCHAR:	p= ":wordchar"; break;
	case REGEX_NOWCHAR:	p= ":no wordchar"; break;
	case REGEX_LETTER:	p= ":letter"; break;
	case REGEX_NOLETTER:p= ":no letter"; break;


	case REGEX_SET:		p= ":set"; break;
	case REGEX_SETRANGE:p= ":range"; break;

	case REGEX_BOS:		p= ":bos"; break;
	case REGEX_EOS:		p= ":eos"; break;
	case REGEX_EOS2:	p= ":weak eos"; break;

	case REGEX_BACKREF:		p= ":backreference"; break;

	case REGEX_CONFIG:		p= ":config"; break;
	case REGEX_RESTORE:		p= ":config restore"; break;

	case REGEX_OPEN:	p= ":open"; break;
	case REGEX_CLOSE:	p= ":close"; break;


	case REGEX_N_STAR:	p= ":star"; break;
	case REGEX_N_PLUS:	p= ":plus"; break;
	case REGEX_L_STAR:	p= ":lazy star"; break;
	case REGEX_L_PLUS:	p= ":lazy plus"; break;

	case REGEX_N_RANGE:		p= ":range"; break;
	case REGEX_N_RANGEMIN:	p= ":range min"; break;
	case REGEX_N_RANGEMAX:	p= ":range max"; break;

	case REGEX_L_RANGE:		p= ":lazy range"; break;
	case REGEX_L_RANGEMIN:	p= ":lazy range min"; break;
	case REGEX_L_RANGEMAX:	p= ":lazy range max"; break;


	case REGEX_N_BRANCH:	p= ":branch"; break;
	case REGEX_L_BRANCH:	p= ":lazy branch"; break;

	case REGEX_N_BRRANGE:	p= ":complex range"; break;
	case REGEX_N_BRRGMIN:	p= ":complex range min"; break;
	case REGEX_N_BRRGMAX:	p= ":complex range max"; break;

	case REGEX_L_BRRANGE:	p= ":lazy complex range"; break;
	case REGEX_L_BRRGMIN:	p= ":lazy complex range min"; break;
	case REGEX_L_BRRGMAX:	p= ":lazy complex range max"; break;

	case REGEX_LAHEADT:		p= ":positive lookahead"; break;
	case REGEX_LAHEADF:		p= ":negative lookahead"; break;
	case REGEX_LBEHINDT:	p= ":positive lookbehind"; break;
	case REGEX_LBEHINDF:	p= ":negative lookbehind"; break;

	case REGEX_POSSESS:		p= ":possessive subRE"; break;

	}
	return p;
}
///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::CRegProgram::Print() const
{
	char op = REGEX_EXACTLY;	// Arbitrary non-END op. 
	size_t next;
	size_t s = 1;
	while( op != REGEX_END && s<cProgramm.size() )
	{	// While that wasn't END last time... 
		op = this->command(s);
		next = this->nextcommand(s);

		printf(( "%3d (->%3d) %s" ), (int)s, (int)next, GetPrintCmd(s));	// Where, what. 

		s += 3;// operand
		if( op == REGEX_EXACTLY )
		{
			// Literal string, where present. 
			printf(" '");
			while( cProgramm[s] != '\0')
			{
				if( stringcheck::iscntrl( cProgramm[s] ) )
					printf("{0x%X}", cProgramm[s] );

				else
					putchar( cProgramm[s] );
				s++;
			}
			printf("'");
			++s;
		}
		else if( op == REGEX_ANYOF || op == REGEX_ANYBUT )
		{
			printf(" ");
			while( cProgramm[s] != '\0')
			{
				switch( cProgramm[s] )
				{
				case REGEX_SPACE: case REGEX_NOSPACE:
				case REGEX_XDIGIT:
				case REGEX_DIGIT: case REGEX_NODIGIT:
				case REGEX_UPPER:
				case REGEX_WORD: case REGEX_NOWORD:
				case REGEX_LOWER:
				case REGEX_GRAPH:
				case REGEX_PRINT:
				case REGEX_PUNCT:
				case REGEX_CNTRL:
					printf(( "%s" ), GetPrintCmd(s));
					++s;
					break;
				case REGEX_SET:
				{	
					printf(( "%s '" ), GetPrintCmd(s));
					++s;
					while( cProgramm[s] != '\0')
					{
						putchar( cProgramm[s] );
						s++;
					}
					printf("'");
					++s;
					break;
				}
				case REGEX_SETRANGE:
				{
					printf(( "%s '%c'-'%c'" ), GetPrintCmd(s), cProgramm[s+1], cProgramm[s+2]);
					s+=3;
					break;
				}
				}//end switch
			}
			++s;
		}
		else if( op == REGEX_OPEN || op == REGEX_CLOSE || op == REGEX_BACKREF )
		{
			ushort no = ((ushort)cProgramm[s]) | ((ushort)(cProgramm[s+1])<<8);
			printf( "(%i)", no);
			s+=2;
		}
		else if( op == REGEX_N_RANGE || op == REGEX_N_BRRANGE ||
				 op == REGEX_L_RANGE || op == REGEX_L_BRRANGE )
		{
			ushort no = ((ushort)cProgramm[s]) | ((ushort)(cProgramm[s+1])<<8);
			printf( "(%i)", no);
			s+=2;
			no = ((ushort)cProgramm[s]) | ((ushort)(cProgramm[s+1])<<8);
			printf( "(%i)", no);
			s+=2;
		}
		else if( op == REGEX_N_RANGEMIN || op == REGEX_N_RANGEMAX ||
				 op == REGEX_L_RANGEMIN || op == REGEX_L_RANGEMAX ||
				 
				 op == REGEX_N_BRRGMIN || op == REGEX_N_BRRGMAX ||
				 op == REGEX_L_BRRGMIN || op == REGEX_L_BRRGMAX )
		{
			ushort no = ((ushort)cProgramm[s]) | ((ushort)(cProgramm[s+1])<<8);
			printf( "(%i)", no);
			s+=2;
		}
		else if( op==REGEX_CONFIG )
		{	// one byte
			printf(":%s%s%s",
				(cProgramm[s] & REGEX_MASK_NLDT)? ((cProgramm[s] & REGEX_CONF_NLDT)?" nldt on": " nldt off"):"",
				(cProgramm[s] & REGEX_MASK_NLSE)? ((cProgramm[s] & REGEX_CONF_NLSE)?" nlse on": " nlse off"):"",
				(cProgramm[s] & REGEX_MASK_CASE)? ((cProgramm[s] & REGEX_CONF_CASE)?" icase on": " icase off"):"");
			s+=1;
		}
		else if( op==REGEX_RESTORE )
		{// one byte
			ushort no = ((ushort)cProgramm[s]) | ((ushort)(cProgramm[s+1])<<8);
			printf( "(%i)", (int)(s-no-3));// -3 since s already has been increased
			s+=2;
		}
		else if( op==REGEX_LBEHINDT || op==REGEX_LBEHINDF )
		{
			// Literal string, where present. 
			printf(" '");
			size_t run, end = s;
			while( cProgramm[s] != '\0') s++;
			run = s-1;
			while( run>=end )
			{
				if( stringcheck::iscntrl( cProgramm[run] ) )
					printf("{0x%X}", cProgramm[run] );

				else
					putchar( cProgramm[run] );
				--run;
			}
			printf("'");
			++s;
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
	for(size_t i=0; i<cProgramm.size(); ++i)
		printf("%i ", cProgramm[i]);
	printf("\n"); 
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::CRegProgram::Compile(const char* exp)
{
	int flags;

	cStartChar	= 0;
	cAnchor		= 0;
	cMust		= 0;
	cMustLen	= 0;
	cConfig.cStatus = false;

	if( exp == NULL)
	{
		regerror( REGERR_NULL_TO_REGCOMP );
		return false;
	}

	cProgramm.clear();
	AddValue(REGEX_MAGIC);

	int parcnt=1;
	const char *parse = exp;

#ifdef DEBUG
	cExpression = parse;
#endif

	if( 0 != this->ParseExp(parse, REGEX_NOPAREN, parcnt, flags)  )
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
		cConfig.cStatus = true;
	}
	else
	{	// error
		cProgramm.clear();
	}
	return cConfig.cStatus;
}
///////////////////////////////////////////////////////////////////////////////
// regexec - match a CRegExpWorker against a string
bool CRegExp::CRegProgram::Execute( const char* str, vector< vector<CRegExp::CFinds> >& finds ) const
{
	finds.resize(0);

	// Be paranoid. 
	if( str == NULL )
	{
		regerror( REGERR_NULLARG );
		return false;
	}
	if( !cConfig.cStatus || cProgramm.size()<1 )
	{
		regerror( REGERR_NO_REGEXP );
		return false;
	}

	// Check validity of program. 
	if( cProgramm[0] != REGEX_MAGIC )
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
	cProg->cConfig.cCaseInSensitive=false;
	cProg->cConfig.cMultiLineDT=true;
	cProg->cConfig.cMultiLineSE=true;
	cProg->cConfig.cPCREconform=true;
	cProg->Compile(exp);
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
//
const CRegExp & CRegExp::assign( const char* exp, bool icase, bool multi, bool conf)
{
	cProg->cConfig.cCaseInSensitive=icase;
	cProg->cConfig.cMultiLineDT=multi;
	cProg->cConfig.cMultiLineSE=multi;
	cProg->cConfig.cPCREconform=conf;
	cProg->Compile(exp);
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
bool CRegExp::match( const char * s ) const
{
	if(s)
	{
		clearerror();
		cQueryString = s;
		bool ret = false;
		if( cProg.get() )
		{	// access to cProg is read-only so there will be no automatic copy
			ret  = cProg->Execute(cQueryString, cFinds);
		}
		return ret;
	}
	else
	{
		cProg->regerror( REGERR_NULLARG );
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::match( const string<> &s ) const
{
	clearerror();
	cQueryString = s;
	bool ret = false;
	if( cProg.get() )
	{	// access to cProg is read-only so there will be no automatic copy
		ret  = cProg->Execute(cQueryString, cFinds);
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
// number of finds
unsigned int CRegExp::sub_count() const
{
	clearerror();
	return ( cProg.get() && cFinds.size()>0 ) ? cFinds.size()-1 : 0;
}
unsigned int CRegExp::sub_count(size_t i) const
{
	clearerror();
	if( cProg.get() && i<cFinds.size() )
		return cFinds[i].size();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// start and length of the substring according to the matchstring
unsigned int CRegExp::sub_start(size_t i, size_t k) const
{
	clearerror();
	unsigned int ret = 0;
	if( cProg.get() && i<cFinds.size() && cFinds[i].size()>0 )
	{
		if(k>cFinds[i].size()) k = cFinds[i].size()-1;
		ret = cFinds[i][k].startp - (const char*)cQueryString;
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
//
unsigned int CRegExp::sub_length(size_t i, size_t k) const
{
	clearerror();
	int ret = -1;
	if( cProg.get() && i<cFinds.size() && cFinds[i].size()>0 )
	{
		if(k>cFinds[i].size()) k = cFinds[i].size()-1;
		ret = cFinds[i][k].endp - cFinds[i][k].startp;
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////
// last find at position i (PCRE conform)
const string<> CRegExp::operator[]( size_t i ) const
{
	string<> str;
	clearerror();
	if( cProg.get() && i<cFinds.size() && cFinds[i].size()>0 )
		str.append( cFinds[i].last().startp, cFinds[i].last().endp-cFinds[i].last().startp );
	return str;
}

///////////////////////////////////////////////////////////////////////////////
// access to all finds (default to the last == PCRE conform)
const string<> CRegExp::operator()(size_t i, size_t k) const
{
	string<> str;
	clearerror();
	if( cProg.get() && i<cFinds.size() && cFinds[i].size()>0 )
	{
		if(k>cFinds[i].size()) k = cFinds[i].size()-1;
		str.append( cFinds[i][k].startp, cFinds[i][k].endp-cFinds[i][k].startp );
	}
	return str;
}

///////////////////////////////////////////////////////////////////////////////
//
bool CRegExp::is_valid() const
{
	return cProg.get() ? cProg->Status() : false;
}

///////////////////////////////////////////////////////////////////////////////
//
void CRegExp::setCaseSensitive(bool v)
{
	this->cProg->cConfig.cCaseInSensitive = !v;
}
bool CRegExp::isCaseSensitive() const
{
	return cProg.get() ? !cProg->cConfig.cCaseInSensitive : false;
}
void CRegExp::setMultiLine(bool v)
{
	this->cProg->cConfig.cMultiLineDT=this->cProg->cConfig.cMultiLineSE=v;
}
bool CRegExp::isMultiLine() const
{
	return cProg.get() ? cProg->cConfig.cMultiLineDT : true;
}
void CRegExp::setPCREconform(bool v)
{
	this->cProg->cConfig.cPCREconform=v;
}
bool CRegExp::isPCREconform() const
{
	return cProg.get() ? cProg->cConfig.cPCREconform : true;
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
// replacestring	- Converts a replace expression to a string
//					- perform substitutions after a match
// Returns			- The resultant string
string<> CRegExp::replacestring( const char* sReplaceExp ) const
{
	string<> ret;

	clearerror();
	if( !cProg.get() )
	{
		cProg->regerror( REGERR_NO_REGEXP );
	}
	else if( sReplaceExp == NULL )
	{
		cProg->regerror( REGERR_NULL_TO_REGSUB  );
	}
	else if( !cProg->cConfig.cStatus || cProg->cProgramm.size()<=0 || cProg->cProgramm[0] != REGEX_MAGIC )
	{
		cProg->regerror( REGERR_DAMAGED_REGEXP_REGSUB );
	}
	else
	{	// create the string
		char c;
		int no;
		size_t len;
		const char *src = sReplaceExp;

		while( (c = *src++) != ('\0') ) 
		{
			if( c == ('&'))
				no = 0;
			else if( c == ('\\') && stringcheck::isdigit(*src) )
			{
				no = *src++ - ('0');
				while( stringcheck::isdigit(*src) )
					no = 10*no + *src++ - ('0');
			}
			else
				no = -1;

			if( no < 0) 
			{	// Ordinary character. 
				if( c == ('\\') && (*src == ('\\') || *src == ('&')))
					c = *src++;
				ret.append( c );
			} 
			else if( cFinds.size()>(size_t)no &&
					 cFinds[no].size() > 0 &&
					 cFinds[no].last().startp != NULL && 
					 cFinds[no].last().endp  != NULL && 
					 cFinds[no].last().endp >= cFinds[no].last().startp ) 
			{
				// Get tagged expression
				len = cFinds[no].last().endp - cFinds[no].last().startp;
				if( len != 0 && hstrlen( cFinds[no].last().startp ) < len )
				{	// strncpy hit NULL
					cProg->regerror( REGERR_DAMAGED_MATCH_STRING );
					ret.clear();
					break;
				}
				else
				{
					ret.append( cFinds[no].last().startp, len );
				}
			}
		}
	}
	return ret;
}
string<> CRegExp::sub(const char * replacement, const string<> str, size_t count, size_t which)
{
	clearerror();
	if ( !cProg.get() )
		return str;

	size_t no=0;
	const char *s = str;
	string<> replacestr;

	bool ret;

	while( no < count )
	{
		ret = this->match( s );
		if( !ret || which>cFinds.size()  )
			break;
		// append from str to beginning of sFind
		replacestr.append( s, cFinds[which].last().startp-s );
		replacestr.append( replacement );
		s = cFinds[which].last().endp;
		no++;
	}
	// append the rest
	replacestr += s;

	return replacestr;
}

string<> CRegExp::subn(const char * replacement, const string<> str, size_t &count, size_t which)
{
	clearerror();
	if ( !cProg.get() )
		return str;
	
	const char *s = str;
	string<> replacestr;
	bool ret;

	count=0;
	while( 1 )
	{
		ret = this->match( s );
		if( !ret || which>cFinds.size() )
			break;
		// append from str to beginning of sFind
		replacestr.append( s, cFinds[which].last().startp-s );
		replacestr.append( replacement );
		s = cFinds[which].last().endp;
		count++;
	}
	// append the rest
	replacestr += s;

	return replacestr;
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
void test_regex(void)
{
#if defined(DEBUG)
	{
//		CRegExp re("aaa(?i-m-ss:AAA)b");
		CRegExp re("(...)*(a+)(abc)");

		re.Dump();
		re.Print();

		re.match("123123123aaabc");


		printf("----\n%s\n", (const char*)re[0]);
		uint i,k;
		for(i=1; i<=re.sub_count(); ++i)	// finds count from 1
		{
			printf("%2u: ", i);
			for(k=0; k<re.sub_count(i); ++k)	// inside finds count from 0
				printf("%s, ", (const char*)re(i,k));

			printf("\n");
		}

		printf("----\n");
	}

	{
		const char *a;
		string<> ret;
		CRegExp exp("(.*ab)([^a]*)a");
		CRegExp xx = exp;

		CRegExp yy("(?:([^,]*),)*([^,]*)");
		yy.Dump();
		yy.Print();
		yy.match("ab1ba, ab2ba, ab3ba, ab4ba, ab5ba, ab6ab");
		{
			uint i;
			for(i=0; i<=yy.sub_count(); ++i)
				printf("%2u: %s\n", i, (const char*)yy[i]);
		}

		yy = "([abc])*d";
		yy.match("abbbcd");
		{
			uint i;
			for(i=0; i<=yy.sub_count(); ++i)
				printf("%2u: %s\n", i, (const char*)yy[i]);
		}


		exp = "(blue|white|red)";
		exp.Dump();
		exp.Print();

		a = "aaababb1bbbbaaaaaabbb ff white bbbaaabb2bbbbaaaaaaaa	aaaaaab3bbbbaxx";
		exp.match(a);
		a = ret = exp[0];
		a = ret = exp[1];
		a = ret = exp[2];

		a = "aaababb1bbbbaaaaaabbb ff white bbbaaabb2bbbbaaaaaaaa	aaaaaab3bbbbaxx";
		xx.match(a);
		a = ret = xx[0];
		a = ret = xx[1];
		a = ret = xx[2];


		CRegExp x2("([^,]*),");
		a = "ab1ba, ab2ba, ab3ba, ab4ba, ab5ba, ab6ab";

		x2.match(a);
		a = ret = x2[0];
		a = ret = x2[1];
		a = ret = x2[2];
		a = ret = x2[3];
		a = ret = x2[4];
		a = ret = x2[5];
	}

	{
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
//			{"a[b-a]","-","c","-","-"},		// can do this -> range swapping
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
//			{"a*?","-","c","-","-"},	// can do this -> lazy operators
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
		for(i=0; i<sizeof(fields)/sizeof(fields[0]); ++i)
		{
			CRegExp r(fields[i][0]);
			if( !r.is_valid() )
			{
				if( *fields[i][2] != 'c' )
					printf("%i: unexpected comp failure in '%s' (%s)\n", (int)i, fields[i][0], r.errmsg());
				continue;
			}
			else if( *fields[i][2] == 'c')
			{
				printf("%i: unexpected comp success in '%s'\n", (int)i, fields[i][0]);
				continue;
			}

			if( !r.match(fields[i][1]))
			{
				if( *fields[i][2] != 'n' )
					printf("%i: unexpected match failure in '%s' / '%s' (%s)\n", (int)i, fields[i][0],fields[i][1], r.errmsg());
				continue;
			}
			if( *fields[i][2] == 'n' )
			{
				printf("%i: unexpected match success in '%s' / '%s'\n", (int)i, fields[i][0],fields[i][1]);
				continue;
			}
			string<> repl = r.replacestring( fields[i][3] );
			if( r.errmsg() )
			{
				printf("%i: GetReplaceString complaint in '%s' / '%s'\n", (int)i, fields[i][0], r.errmsg());
				continue;
			}
			if ( repl != fields[i][4] )
				printf("%i: regsub result in '%s' wrong (is: '%s' != should: '%s')\n", (int)i, fields[i][0], (const char*)repl, fields[i][4]);
		}
	}


	{
		CRegExp badregexp;
		CRegExp r;
	
		r = CRegExp(NULL);
		if( r.is_valid() || !r.errmsg() )
			printf("regcomp(NULL) doesn't complain\n");

		r = CRegExp();		// clean it out
		if( r.match("foo") || !r.errmsg() )
			printf("regexec(NULL, ...) doesn't complain\n");

		r = CRegExp("foo");
		if( !r.is_valid() )
			printf("regcomp(\"foo\") fails");

		if ( r.match(NULL) || !r.errmsg() )
			printf("regexec(..., NULL) doesn't complain\n");

		r = CRegExp();
		string<> repl = r.replacestring( "foo" );
		if ( !r.errmsg() )
			printf("replacestring(NULL, ..., ...) doesn't complain\n");


		repl = r.replacestring( NULL );
		if ( !r.errmsg() )
			printf("replacestring(..., NULL, ...) doesn't complain\n");
		

		if( badregexp.match( "foo" ) || !badregexp.errmsg() )
			printf("regexec(nonsense, ...) doesn't complain\n");

		repl = badregexp.replacestring( "foo" );
		if( !badregexp.errmsg() )
			printf("replacestring(nonsense, ..., ...) doesn't complain\n");

		CRegExp reg1( "[abc]+" );

		if ( !reg1.is_valid() )
			printf("regcomp(\"[abc]+\") fails\n");

		if ( !reg1.match( "test string 1 - aaa") || reg1.errmsg() )
			printf("Match failed\n");

		if ( reg1[0] != "aaa" )
			printf("Match failed %s!=%s\n", (const char*)reg1[0], "aaa");


		CRegExp reg2( reg1 );

		if ( !reg2.match("test string 2 - bbb") || reg2.errmsg() )
			printf("Match failed\n");

		repl = reg2.replacestring( "&" );
		if ( reg2[0] != "bbb" )
			printf("Match failed %s!=%s\n", (const char*)reg2[0], "bbb");


		CRegExp reg3 ( reg1 );

		if ( !reg3.match( "test string 3 - ccc") || reg3.errmsg() )
			printf("Match failed\n");

		if ( reg3[0] != "ccc" )
			printf("Match failed %s!=%s\n", (const char*)reg3[0], "ccc");

	}
#endif//DEBUG
}


NAMESPACE_END(basics)
