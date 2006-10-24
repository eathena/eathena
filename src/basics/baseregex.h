
#ifndef __BASEREGEX__
#define __BASEREGEX__

#include "basetypes.h"
#include "basestring.h"
#include "basearray.h"


//////////////////////////////////////////////////////////////////////////
/// Regualar Expression Library


NAMESPACE_BEGIN(basics)


//////////////////////////////////////////////////////////////////////////
/// test function
void test_regex(void);



///////////////////////////////////////////////////////////////////////////////
/// Regular expression class
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
	/// find element. consists actually of a start and end pointer 
	/// inside the string element from the match query, which is internally copied
	class CFinds : public defaultcmp
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
	};
private:
	///////////////////////////////////////////////////////////////////////////
	/// error handler.
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
			fprintf(stderr, "Error in RegularExpression: %s\n", s );
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
	/// the actual regular expression engine.
	/// contains/builds/executes the automat for running the query
	class CRegProgram : public CRegErrorHandler
	{

		///////////////////////////////////////////////////////////////////////
		// friends
		friend class CRegExp;

		///////////////////////////////////////////////////////////////////////
		// class data
		vector<char> cProgramm;		// the Programm
		size_t	cMust;				// internal
		size_t	cMustLen;			// internal
		char	cStartChar;			// internal
		char	cAnchor;			// internal


		/// regex configuration.
		/// cPCREconform:
		/// perl regular expressions only accept the last match in a recursive pattern
		/// such as 
		/// "([abc])*d" used on the string "abbbcd" would result in $0 = "abbbcd" and $1 = "c"
		/// even if "a","b","b","b" would also match to the first parentheses pattern
		/// with cPCREconform=false the result of each parentheses is stored
		/// as vector so each match can be obtained
		struct _config
		{
			uint	cStatus : 1;
			uint	cCaseInSensitive : 1;
			uint	cMultiLineDT : 1;
			uint	cMultiLineSE : 1;
			uint	cPCREconform : 1;
			uint	cParenDisable : 1;
			uint	_unused : 10;
			/// constructor
			_config(bool stat=true, bool icase=true, bool multidt=true, bool multise=true, bool conform=true)
				: cStatus(stat),cCaseInSensitive(icase),cMultiLineDT(multidt),cMultiLineSE(multise),cPCREconform(conform),
				cParenDisable(false)
			{}
			/// config data to integer conversion.
			ushort get() const
			{
				return	(0x0001 * cStatus) |
						(0x0002 * cCaseInSensitive) |
						(0x0004 * cMultiLineDT) |
						(0x0008 * cMultiLineSE) |
						(0x0010 * cPCREconform) |
						(0x0020 * cParenDisable);
			}
			operator ushort() const
			{
				return this->get();
			}
			/// integer to config data conversion.
			void set(ushort v)
			{
				cStatus				= (v&0x0001)==0x0001;
				cCaseInSensitive	= (v&0x0002)==0x0002;
				cMultiLineDT		= (v&0x0004)==0x0004;
				cMultiLineSE		= (v&0x0008)==0x0008;
				cPCREconform		= (v&0x0010)==0x0010;
				cParenDisable		= (v&0x0020)==0x0020;
			}
			const _config& operator =(ushort v)
			{
				this->set(v);
				return *this;
			}
		} cConfig;
#ifdef DEBUG
		string<> cExpression;		///< the regex itself
#endif
	public:
		///////////////////////////////////////////////////////////////////////
		/// construction
		CRegProgram() :
			cMust(0),
			cMustLen(0),
			cStartChar(0),
			cAnchor(0),
			cConfig(false,false,true,true,true)
		{}
		CRegProgram(const char *exp, bool iCase=true, bool multi=true, bool conform=true) :
			cMust(0),
			cMustLen(0),
			cStartChar(0),
			cAnchor(0),
			cConfig(false,iCase,multi,multi,conform)
		{	//
			Compile( exp );
		}
		///////////////////////////////////////////////////////////////////////
		/// copy/assignment
		CRegProgram(const CRegProgram& a) :
			cProgramm(a.cProgramm),
			cMust(a.cMust),
			cMustLen(a.cMustLen),
			cStartChar(a.cStartChar),
			cAnchor(a.cAnchor),
			cConfig(a.cConfig)

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
			cConfig		= a.cConfig;
	#ifdef DEBUG
			cExpression = a.cExpression;
	#endif
			return *this;
		}
	private:
		///////////////////////////////////////////////////////////////////////
		/// set query to case-ignoring.
		void ignoreCase(const char * in, char * out) const;

		///////////////////////////////////////////////////////////////////////
		/// returns the command at pos.
		char command( size_t pos ) const
		{
			return cProgramm[pos];
		}
		///////////////////////////////////////////////////////////////////////
		/// returns the operand at pos.
		const char* operand( size_t pos ) const
		{
			return &(cProgramm[pos]) + 3;
		}
		///////////////////////////////////////////////////////////////////////
		/// steps to the next command
		size_t nextcommand( size_t pos ) const;
		///////////////////////////////////////////////////////////////////////
		/// emit (if appropriate) a byte of code.
		void AddValue(char b)
		{
			cProgramm.append( b );
		}
		void AddtoSet(size_t &pos, char b);
		bool ComparetoSet(size_t node, char b) const;
		///////////////////////////////////////////////////////////////////////
		/// emit a node.
		size_t AddNode(char op)
		{
			char buf[3] = {op,0,0};
			size_t const ret = cProgramm.size();
			cProgramm.append( buf, 3 );
			return ret;
		}
		///////////////////////////////////////////////////////////////////////
		/// InsertNode - insert an operator in front of already-emitted operand
		/// Means relocating the operand.
		void InsertNode(char op, size_t opnd)
		{
			char buf[3] = {op,0,0};
			cProgramm.insert(buf,3, opnd);
		}
		///////////////////////////////////////////////////////////////////////
		/// AddTail - set the next-pointer at the end of a node chain
		void AddTail(size_t p, size_t val);
		///////////////////////////////////////////////////////////////////////
		// AddCmdTail - AddTail on operand of first argument; nop if operandless
		void AddCmdTail(size_t p, size_t val);
		///////////////////////////////////////////////////////////////////////
		/// parses regular expression. i.e. main body or parenthesized thing
		/// Caller must absorb opening parenthesis.
		/// Combining parenthesis handling with the base level of regular expression
		/// is a trifle forced, but the need to tie the tails of the branches to what
		/// follows makes it hard to avoid.
		size_t ParseExp(const char*&parsestr, int paren, int &parcnt, int &flag);
		///////////////////////////////////////////////////////////////////////
		/// parses one alternative of an | operator.
		/// Implements the concatenation operator.
		size_t ParseBranch(const char*&parsestr, int &parcnt, int &flag);
		///////////////////////////////////////////////////////////////////////
		/// parses a piece - something followed by possible [*+?].
		/// Note that the branching code sequences used for ? and the general cases
		/// of * and + are somewhat optimized:  they use the same NOTHING node as
		/// both the endmarker for their branch list and the body of the last branch.
		/// It might seem that this node could be dispensed with entirely, but the
		/// endmarker role is not redundant.
		size_t ParsePiece(const char*&parsestr, int &parcnt, int &flag);
		///////////////////////////////////////////////////////////////////////
		/// parses atoms - the lowest level.
		/// Optimization:
		/// - gobbles an entire sequence of ordinary characters so that
		///   it can turn them into a single node, which is smaller to store and
		///   faster to run.
		/// - Backslashed characters are exceptions, each becoming a separate node; 
		///   the code is simpler that way and it's not worth fixing.
		size_t ParseAtom(const char*&parsestr, int &parcnt, int &flag);

		///////////////////////////////////////////////////////////////////////
		//
		inline bool isRepeat( char c ) const
		{
			return ((c) == '*' || (c) == '+' || (c) == '?' || (c) == '{'); 
		}

		///////////////////////////////////////////////////////////////////////
		// calling the match query.
		bool MatchTry(const char* base, const char* str, vector< vector<CFinds> >& finds) const;

		///////////////////////////////////////////////////////////////////////
		/// MatchMain - main matching routine.
		/// Conceptually the strategy is simple:
		/// - check to see whether the current node matches, 
		///   call self recursively to see whether the rest matches, 
		///   and then act accordingly. 
		/// - In practice we make some effort to avoid recursion, 
		///   in particular by going through "ordinary" nodes (that don't
		///   need to know whether the rest of the match failed) by a loop 
		///   instead of by recursion.
		bool MatchMain(const char* base, const char* str, vector< vector<CFinds> >& finds, size_t progstart, const char*&reginput, map<size_t, size_t>& imap) const;

		///////////////////////////////////////////////////////////////////////
		/// MatchRepeat. - report how many times something simple would match
		size_t MatchRepeat(const char* base, size_t node, const char*&reginput ) const;


#ifdef DEBUG
		///////////////////////////////////////////////////////////////////////
		// debug functions.
	private:
		/// GetPrintCmd - printable representation of opcode.
		const char * GetPrintCmd( size_t pos ) const;
	public:
		void Print() const;
		void Dump() const;
		///////////////////////////////////////////////////////////////////////
#endif

	public:
		///////////////////////////////////////////////////////////////////////
		/// returns the internal status.
		bool Status() const { return cConfig.cStatus; }
		///////////////////////////////////////////////////////////////////////
		/// compiles an expression.
		bool Compile(const char* exp);
		///////////////////////////////////////////////////////////////////////
		/// match a CRegExpWorker against a string.
		bool Execute( const char* string, vector< vector<CFinds> >& finds ) const;
	};
	///////////////////////////////////////////////////////////////////////////

private:
	///////////////////////////////////////////////////////////////////////////
	// class data
	mutable string<>					cQueryString;	///< the string which we match with
	mutable vector< vector<CFinds> >	cFinds;			///< the finds in that string

	///////////////////////////////////////////////////////////////////////////
	/// reference to the expression parser.
	TPtrAutoRef<CRegProgram> cProg;

	///////////////////////////////////////////////////////////////////////////
	/// clears the errormessage.
	void clearerror() const;

	///////////////////////////////////////////////////////////////////////////
public:

	///////////////////////////////////////////////////////////////////////////
	/// constructor/destructor
	CRegExp()	{}
	CRegExp( const char* exp, bool icase = false, bool conf=true )
		: cProg( exp, icase, conf )	{}
	~CRegExp()	{}

	///////////////////////////////////////////////////////////////////////////
	/// copy/assign
	CRegExp( const CRegExp &r )	: cProg( r.cProg )	{}
	const CRegExp & operator=( const CRegExp & r );

	const CRegExp & operator=( const char* exp );
	const CRegExp & assign( const char* exp, bool icase = false, bool multi=true, bool conf=true );
	const CRegExp & assign( const CRegExp & r );

	///////////////////////////////////////////////////////////////////////////
	/// search
	bool match( const char *s ) const;
	bool match( const string<> &s ) const;

	bool operator==( const char *s ) const { return match(s); }
	bool operator==( const string<> &s ) const { return match(s); }

	friend bool operator==( const char *s, const CRegExp& reg ) { return reg.match(s); }
	friend bool operator==( const string<> &s, const CRegExp& reg ) { return reg.match(s); }

	///////////////////////////////////////////////////////////////////////////
	/// substring access.
	/// number of finds/number of finds in a parenthesis
	unsigned int sub_count() const;
	unsigned int sub_count(size_t i) const;
	/// last find at position i (PCRE conform)
	const string<> operator[]( size_t i ) const;
	/// access to all finds (default to the last == PCRE conform)
	const string<> operator()(size_t i=0, size_t k=~0) const;
	/// start and length of the substring according to the matchstring
	unsigned int sub_start(size_t i=0, size_t k=~0) const;
	unsigned int sub_length(size_t i=0, size_t k=~0) const;

	///////////////////////////////////////////////////////////////////////////
	/// replacements.
	string<> replacestring(const char* sReplaceExp) const;
	string<> sub(const char * replacement, const string<> str, size_t count=~0, size_t which=0);
	string<> subn(const char * replacement, const string<> str, size_t &count, size_t which=0);


	///////////////////////////////////////////////////////////////////////////
	/// correctly compiled.
	bool is_valid() const;

	///////////////////////////////////////////////////////////////////////////
	/// run options
	void setCaseSensitive(bool v);
	bool isCaseSensitive() const;
	void setMultiLine(bool v);
	bool isMultiLine() const;
	void setPCREconform(bool v);
	bool isPCREconform() const;

	///////////////////////////////////////////////////////////////////////////
	/// last run error message. NULL when none
	const char* errmsg() const;
	
#ifdef DEBUG
	void Dump() const;
	void Print() const;
#endif
};



NAMESPACE_END(basics)


#endif//__BASEREGEX__

