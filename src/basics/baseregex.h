
#ifndef __BASEREGEX__
#define __BASEREGEX__

#include "basetypes.h"
#include "basestring.h"
#include "basearray.h"

//////////////////////////////////////////////////////////////////////////
// switch to change pattern recognition bahaviour
#define REGEX_PCRECONFORM
// perl regular expressions only accept the last match in a recursive pattern
// such as 
// "([abc])*d" used on the string "abbbcd" would result in $0 = "abbbcd" and $1 = "c"
// even if "a","b","b","b" would also match to the first parentheses pattern
// without conformity finding all of them is possible, resulting in
// $0 = "abbbcd", $1 = "a", $2 = "b", $3 = "b", $4 = "b", $5 = "c"
// but would loose track of the $1..$n correlation

//!! TODO: modifiy the result to prepare arrays at $n with default returns conform to the standard



//////////////////////////////////////////////////////////////////////////
// test function
void test_regex();



///////////////////////////////////////////////////////////////////////////////
// Regular expression class
class CRegExp
{
	///////////////////////////////////////////////////////////////////////////
	// friends


	///////////////////////////////////////////////////////////////////////////
	// constants


	///////////////////////////////////////////////////////////////////////////
	// class declarations
public:
	///////////////////////////////////////////////////////////////////////////
	// an find element; consists actually of a start and end pointer 
	// inside the string element from the match query, which is internally copied
	class CFinds
	{
	public:
		///////////////////////////////////////////////////////////////////////
		// class data
		const char* startp;
		const char* endp;

		///////////////////////////////////////////////////////////////////////
		// construction
		CFinds(const char* s=NULL, const char* e=NULL) : startp(s), endp(e)
		{}
		///////////////////////////////////////////////////////////////////////
		// members
		bool operator==(const CFinds&) const {return false;}
	};
private:
	///////////////////////////////////////////////////////////////////////////
	// error handler
	class CRegErrorHandler
	{
		///////////////////////////////////////////////////////////////////////
		// friends
		friend class CRegExp;
		///////////////////////////////////////////////////////////////////////
		// class data
		mutable const char* m_szError;

	protected:
		///////////////////////////////////////////////////////////////////////
		// members
		void regerror( int id ) const
		{
			regerror( FindErr( id ) );
		}
		void regerror( const char* s ) const
		{
#ifdef DEBUG
			printf( "Error in RegularExpression: %s\n", s );
#endif
			m_szError = s;
		}
		CRegErrorHandler() : m_szError(NULL) { }
		CRegErrorHandler( const CRegErrorHandler & reh ) : m_szError( reh.m_szError ) {}

		const char* errmsg() const
		{	
			return m_szError;
		}
		void clearerror() const
		{
			m_szError=NULL;
		}
		static const char* FindErr( int id );
	};

	///////////////////////////////////////////////////////////////////////////
	// the actual regular expressio engine
	// contains/builds/executes the automat for running the query
	class CRegProgram : public CRegErrorHandler
	{

		///////////////////////////////////////////////////////////////////////
		// friends
		friend class CRegExp;

		///////////////////////////////////////////////////////////////////////
		// class data
		TArrayDST<char> cProgramm;	// the Programm
		char	cStartChar;			// internal
		char	cAnchor;			// internal
		size_t	cMust;				// internal
		size_t	cMustLen;			// internal
		bool	cStatus;
#ifdef DEBUG
		string<> cExpression;		// the regex itself
#endif
	public:
		///////////////////////////////////////////////////////////////////////
		// construction
		CRegProgram() :
			cStartChar(0),
			cAnchor(0),
			cMust(0),
			cMustLen(0),
			cStatus(false)
		{}
		CRegProgram(const char *exp, bool ignorecase) :
			cStartChar(0),
			cAnchor(0),
			cMust(0),
			cMustLen(0)
		{	//
			Compile( exp, ignorecase );
		}
		///////////////////////////////////////////////////////////////////////
		// copy/assignment
		CRegProgram(const CRegProgram& a) :
			cProgramm(a.cProgramm),
			cStartChar(a.cStartChar),
			cAnchor(a.cAnchor),
			cMust(a.cMust),
			cMustLen(a.cMustLen),
			cStatus(a.cStatus)
	#ifdef DEBUG
			,
			cExpression(a.cExpression)
	#endif

		{}
		const CRegProgram& operator=(const CRegProgram& a)
		{
			cProgramm	= a.cProgramm;
			cStartChar	= a.cStartChar;
			cAnchor		= a.cAnchor;
			cMust		= a.cMust;
			cMustLen	= a.cMustLen;
	#ifdef DEBUG
			cExpression = a.cExpression;
	#endif
			return *this;
		}
	private:
		///////////////////////////////////////////////////////////////////////
		// set query to case-ignoring
		void ignoreCase(const char * in, char * out) const;

		///////////////////////////////////////////////////////////////////////
		// returns the command at pos
		char command( size_t pos ) const
		{
			return cProgramm[pos];
		}
		///////////////////////////////////////////////////////////////////////
		// returns the operand at pos
		const char* operand( size_t pos ) const
		{
			return &(cProgramm[pos]) + 3;
		}
		///////////////////////////////////////////////////////////////////////
		// steps to the next command
		size_t nextcommand( size_t pos ) const;
		///////////////////////////////////////////////////////////////////////
		// AddValue - emit (if appropriate) a byte of code
		void AddValue(int b)
		{
			cProgramm.append( (char)b );
		}
		///////////////////////////////////////////////////////////////////////
		// AddNode - emit a node
		size_t AddNode(char op)
		{
			char buf[3] = {op,0,0};
			size_t const ret = cProgramm.size();
			cProgramm.append( buf, 3 );
			return ret;
		}
		///////////////////////////////////////////////////////////////////////
		// InsertNode - insert an operator in front of already-emitted operand
		// Means relocating the operand.
		void InsertNode(char op, size_t opnd)
		{
			char buf[3] = {op,0,0};
			cProgramm.insert(buf,3, opnd);
		}
		///////////////////////////////////////////////////////////////////////
		// AddTail - set the next-pointer at the end of a node chain
		void AddTail(size_t p, size_t val);
		///////////////////////////////////////////////////////////////////////
		// AddCmdTail - AddTail on operand of first argument; nop if operandless
		void AddCmdTail(size_t p, size_t val);
		///////////////////////////////////////////////////////////////////////
		// ParseExp - regular expression, i.e. main body or parenthesized thing
		// Caller must absorb opening parenthesis.
		// Combining parenthesis handling with the base level of regular expression
		// is a trifle forced, but the need to tie the tails of the branches to what
		// follows makes it hard to avoid.
		size_t ParseExp(const char*&parsestr, bool paren, int &parcnt, int &flag);
		///////////////////////////////////////////////////////////////////////
		// ParseBranch - one alternative of an | operator
		// Implements the concatenation operator.
		size_t ParseBranch(const char*&parsestr, int &parcnt, int &flag);
		///////////////////////////////////////////////////////////////////////
		// ParsePiece - something followed by possible [*+?]
		// Note that the branching code sequences used for ? and the general cases
		// of * and + are somewhat optimized:  they use the same NOTHING node as
		// both the endmarker for their branch list and the body of the last branch.
		// It might seem that this node could be dispensed with entirely, but the
		// endmarker role is not redundant.
		size_t ParsePiece(const char*&parsestr, int &parcnt, int &flag);
		///////////////////////////////////////////////////////////////////////
		// ParseAtom - the lowest level
		// Optimization:  gobbles an entire sequence of ordinary characters so that
		// it can turn them into a single node, which is smaller to store and
		// faster to run.  Backslashed characters are exceptions, each becoming a
		// separate node; the code is simpler that way and it's not worth fixing.
		size_t ParseAtom(const char*&parsestr, int &parcnt, int &flag);

		///////////////////////////////////////////////////////////////////////
		//
		inline bool isRepeat( char c ) const
		{
			return ((c) == '*' || (c) == '+' || (c) == '?'); 
		}

		///////////////////////////////////////////////////////////////////////
		// calling the match query
		bool MatchTry(const char* base, const char* str, TArrayDCT<CFinds>& finds) const;

		///////////////////////////////////////////////////////////////////////
		// MatchMain - main matching routine
		// Conceptually the strategy is simple:  check to see whether the current
		// node matches, call self recursively to see whether the rest matches,
		// and then act accordingly.  In practice we make some effort to avoid
		// recursion, in particular by going through "ordinary" nodes (that don't
		// need to know whether the rest of the match failed) by a loop instead of
		// by recursion.
		bool MatchMain(const char* base, const char* str, TArrayDCT<CFinds>& finds, size_t progstart, const char*&reginput) const;

		///////////////////////////////////////////////////////////////////////
		// MatchRepeat - report how many times something simple would match
		size_t MatchRepeat( size_t node, const char*&reginput ) const;


#ifdef DEBUG
		///////////////////////////////////////////////////////////////////////
		// debug functions
	private:
		// GetPrintCmd - printable representation of opcode
		const char * GetPrintCmd( size_t pos ) const;
	public:
		void Print() const;
		void Dump() const;
		///////////////////////////////////////////////////////////////////////
#endif

	public:
		///////////////////////////////////////////////////////////////////////
		// returns the internal status
		bool Status() const { return cStatus; }
		///////////////////////////////////////////////////////////////////////
		// compiles an expression
		bool Compile(const char* exp, bool ignorecase);
		///////////////////////////////////////////////////////////////////////
		// regexec - match a CRegExpWorker against a string
		bool Execute( const char* string, TArrayDCT<CFinds>& finds ) const;
	};
	///////////////////////////////////////////////////////////////////////////

private:
	///////////////////////////////////////////////////////////////////////////
	// class data
	mutable string<>			cQueryString;	// the string which we match with
	mutable TArrayDCT<CFinds>	cFinds;			// the finds in that string

	///////////////////////////////////////////////////////////////////////////
	// reference to the expression parser
	TPtrAutoRef<CRegProgram> cProg;

	///////////////////////////////////////////////////////////////////////////
	// clears the errormessage
	void clearerror() const;

	///////////////////////////////////////////////////////////////////////////
public:

	///////////////////////////////////////////////////////////////////////////
	// constructor/destructor
	CRegExp()	{}
	CRegExp( const char* exp, bool iCase = false ) : cProg( exp, iCase )	{}
	~CRegExp()	{}

	///////////////////////////////////////////////////////////////////////////
	// copy/assign
	CRegExp( const CRegExp &r )	: cProg( r.cProg )	{}
	const CRegExp & operator=( const CRegExp & r );

	const CRegExp & operator=( const char* exp );
	const CRegExp & assign( const char* exp, bool iCase = false );
	const CRegExp & assign( const CRegExp & r );

	///////////////////////////////////////////////////////////////////////////
	// search
	bool Match( const char *s ) const;
	bool Match( const string<> &s ) const;

	bool operator==( const char *s ) const { return Match(s); }
	bool operator==( const string<> &s ) const { return Match(s); }

	friend bool operator==( const char *s, const CRegExp& reg ) { return reg.Match(s); }
	friend bool operator==( const string<> &s, const CRegExp& reg ) { return reg.Match(s); }

	///////////////////////////////////////////////////////////////////////////
	// substring access
	int SubStrings() const;
	const string<> operator[]( unsigned int i ) const;
	int SubStart( unsigned int i ) const;
	int SubLength( unsigned int i ) const;

	///////////////////////////////////////////////////////////////////////////
	//
	string<> GetReplaceString(const char* sReplaceExp) const;

	
	string<> sub(const char * replacement, const string<> str, size_t count=~0, size_t which=0)
	{	// str is not string reference, need a real copy
		clearerror();
		if ( !cProg.get() )
		{
			return str;
		}
		size_t no=0;
		const char *s = str;
		string<> replacestr;

		bool ret;

		while( no < count )
		{
			ret = Match( s );
			if( !ret || which>cFinds.size()  )
				break;
			// append from str to beginning of sFind
			replacestr.append( s, cFinds[which].startp-s );
			replacestr.append( replacement );
			s = cFinds[which].endp;
			no++;
		}
		// append the rest
		replacestr += s;

		return replacestr;
	}

	string<> subn(const char * replacement, const string<> str, size_t &count, size_t which=0)
	{	// str is not string reference, need a real copy
		clearerror();
		if ( !cProg.get() )
		{
			return str;
		}
		
		const char *s = str;
		string<> replacestr;
		bool ret;

		count=0;
		while( 1 )
		{
			ret = Match( s );
			if( !ret || which>cFinds.size() )
				break;
			// append from str to beginning of sFind
			replacestr.append( s, cFinds[which].startp-s );
			replacestr.append( replacement );
			s = cFinds[which].endp;
			count++;
		}
		// append the rest
		replacestr += s;

		return replacestr;
	}

	///////////////////////////////////////////////////////////////////////////
	// correctly compiled 
	bool isOK() const;
	///////////////////////////////////////////////////////////////////////////
	// last run error message, NULL when none
	const char* errmsg() const;
	
#ifdef DEBUG
	void Dump() const;
	void Print() const;
#endif
};







#endif//__BASEREGEX__

