// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EACOMPILER_
#define _EACOMPILER_

#include "basefile.h"
#include "basestrsearch.h"
#include "baseparser.h"
#include "basesync.h"

#include "scriptprn.h"



#include "eaopcode.h"
#include "eaprogram.h"
#include "eadefine.h"
#include "eainstance.h"



///////////////////////////////////////////////////////////////////////////////
// terminal definitions from parse tree
#include "eascript.h"

// predeclare
struct scriptfile;



///////////////////////////////////////////////////////////////////////////
/// some simplifying interface to the parser reduction tree.
//##TODO: get it templatified to host the compiler selection code directly
struct parse_node
{
	const basics::CParser*			parser;
	const basics::CStackElement*	se;
	
	parse_node() : parser(NULL), se(NULL)
	{}
	// default copy/assignment
	parse_node(const basics::CParser& p, size_t rtpos) : parser(&p), se(&p.rt[rtpos])
	{
		advance();
	}
	parse_node(const basics::CParser& p) : parser(&p), se(&p.rt[0])
	{
		advance();
	}
	void advance()
	{	// truncate single node reductions
		while( this->se && this->se->cChildNum==1 )
			this->se = &this->parser->rt[this->se->cChildPos];
	}
	///////////////////////////////////////////////////////////////////////////
	// element access
	const char* c_str() const				{ return (se)? se->cToken.cLexeme:""; }
	const basics::string<> string() const	{ return (se)? se->cToken.cLexeme:""; }
	const char*name() const					{ return (se)? se->symbol.Name:""; }
	unsigned short symbol() const			{ return (se)? se->symbol.idx :0; }
	unsigned short type() const				{ return (se)? se->symbol.Type:0; }
	size_t childs()	const					{ return (se)? se->cChildNum:0; }
	unsigned int line() const				{ return (se)? se->cToken.line:0; }
	unsigned int column() const				{ return (se)? se->cToken.column:0; }

	///////////////////////////////////////////////////////////////////////////
	// check for terminal
	bool is_terminal(int idx) const
	{
		return ( se && this->type()==1 && this->symbol()==idx );
	}
	///////////////////////////////////////////////////////////////////////////
	// check for nonterminal
	bool is_nonterminal(int idx) const
	{
		return ( se && this->type()==0 && this->symbol()==idx );
	}
	///////////////////////////////////////////////////////////////////////////
	// get a child
	parse_node operator[](size_t inx) const	{ return (se && inx<se->cChildNum)?(parse_node(*this->parser, se->cChildPos+inx)):(*this); }

	///////////////////////////////////////////////////////////////////////////
	void print(bool line_number=true, bool first=true) const
	{
		if( first && !this->type()==1 )
		{
			printf("<%s>: ", this->name());
		}
		if(first) printf("'");
		if( this->type()==1 )
		{	// terminals
			printf("%s", this->c_str());
		}
		else
		{
			size_t i;
			for(i=0; i<this->childs(); ++i)
			{
				this->operator[](i).print(false, false);
			}
		}
		if(first) printf("'");
		if(line_number)
			printf(" (line %u, column %u)", this->line(), this->column());
	}
};


///////////////////////////////////////////////////////////////////////////////
/// compiler.
struct eacompiler
{
	int logging(const char *fmt, ...) const
	{
		int ret = 0;
		va_list argptr;
		va_start(argptr, fmt);
		ret = vfprintf(stdout,fmt, argptr);
		va_end(argptr);
		return ret;
	}
	int warning(const char *fmt, ...) const
	{
		int ret = 0;
		va_list argptr;
		va_start(argptr, fmt);
		ret = vfprintf(stderr,fmt, argptr);
		va_end(argptr);
		return ret;
	}


	///////////////////////////////////////////////////////
	// compile flags
	enum
	{
		CFLAG_NONE		= 0x00000000,	// no restrictions
		CFLAG_LVALUE	= 0x00000001,	// a variable is required
		CFLAG_RVALUE	= 0x00000002,	// a value is required
		CFLAG_USE_BREAK	= 0x00000004,	// allow break
		CFLAG_USE_CONT	= 0x00000008,	// allow continue
		CFLAG_CONST		= 0x00000010,	// nodes have to be const expressions
		CFLAG_MEMBER	= 0x00000020,	// look up variable members
		CFLAG_GLOBAL	= 0x00000040,	// a global variable/value is required
		CFLAG_VARCREATE	= 0x00000080,	// create the variable
		CFLAG_VARSILENT	= 0x00000100,	// don't push the variable
		CFLAG_NOASSIGN	= 0x00000200,	// lvalue operation without assign
		CFLAG_ASSIGN	= 0x00000400,	// lvalue operation with forced assign
		CFLAG_DECL		= 0x00000800,	// compiling a declaration
		CFLAG_VARSTRING	= 0x00010000,	// variable type
		CFLAG_VARFLOAT	= 0x00020000,
		CFLAG_VARINT	= 0x00040000,
		CFLAG_VARMASK	= 0x00070000,
		CFLAG_SUBFUNC	= 0x00080000,	// subfunction
		CFLAG_PERM		= 0x00100000,
		CFLAG_TEMP		= 0x00200000,
		CFLAG_PARAM		= 0x00400000,
		CFLAG_SCPMASK	= 0x00700000,
		CFLAG_PLY		= 0x01000000,	// variable storage
		CFLAG_ACC		= 0x02000000,
		CFLAG_LOG		= 0x03000000,
		CFLAG_NPC		= 0x04000000,
		CFLAG_GLB		= 0x05000000,
		CFLAG_PRT		= 0x06000000,
		CFLAG_GLD		= 0x07000000,
		CFLAG_STORMASK	= 0x07000000
	};
	///////////////////////////////////////////////////////
	// variable name storage
	// var {id, type, declare_scope, use}
	struct CVar
	{
		typedef basics::TObjPtrCount<CVar>		variable;

		size_t			id;		// number for temp/para, unused otherwise
		uint			luse;	// usage counter lefthand
		uint			ruse;	// usage counter righthand
		uint			xuse;	// usage counter unspecified
		uint			scope;	// scope counter
		uint			cline;	// line of first occurance
		uint			uline;	// line of last usage
		basics::var_t	type;	// type of content
		bool			isconst;// as name says
		bool			declared;// as name says

		CVar(size_t i=static_cast<size_t>(-1), uint s=0, basics::var_t t=basics::VAR_AUTO, bool c=false, bool p=false, bool d=false, uint l=0)
			: id(i), luse(p), ruse(0), xuse(0), scope(s), cline(l), uline(l), type(t),isconst(c),declared(d)
		{}
		~CVar()	{}
	};

	///////////////////////////////////////////////////////////////////////////
	// variable scope
	struct CVariableScope : public basics::smap<basics::string<>,basics::smap<uint,CVar::variable> >
	{
		typedef basics::smap<basics::string<>,basics::smap<uint,CVar::variable> > base;
		typedef base::iterator iterator;

		size_t cnt_tempvar;
		size_t cnt_paramvar;

		CVariableScope() : cnt_tempvar(0),cnt_paramvar(0)
		{}
		void clear()
		{
			cnt_tempvar=0;
			cnt_paramvar=0;
			this->base::clear();
		}
	};
	///////////////////////////////////////////////////////
	// variable name storage
	struct CLabelPos
	{
		size_t use;
		size_t pos;
		uint createline;
		uint useline;
		bool valid;
		
		CLabelPos(): use(0), pos(0), createline(0), useline(0), valid(false)
		{}
	};
	struct CContBreak
	{
		size_t									continue_target;///< target for continue
		size_t									break_target;	///< target for break
		bool									has_continue;	///< when continue is valid
		CContBreak() : continue_target(0),break_target(0),has_continue(false)
		{}
		CContBreak(size_t cnt) : continue_target(cnt),break_target(0),has_continue(true)
		{}
	};

	CVariableScope								cVariable;		///< current variable set
	scriptdefines								cDefines_;		///< current define set
	basics::smap<basics::string<>,uint32>		cStrTable;		///< temporary string table
	basics::smap<basics::string<>,CLabelPos>	cLabels;		///< temporary label list
	basics::vector<basics::variant>				cConstvalues;	///< const value stack
	CContBreak									cControl;		///< current break/continue controls

	scriptprog::script							prog;			///< current programm
	basics::vector<scriptinstance::instance>	inst;			///< current instances
	scriptdecl*									funcdecl;		///< current declaration
	basics::TObjPtrCount<scriptfile>			cFile;			///< current file

	basics::slist< basics::string<> >			loadingfiles;
	basics::CParseConfig						cParseConfig;
	int											cCompileOptions;

	///////////////////////////////////////////////////////////////////////////
	// support functions
	void replace_jumps(size_t startaddr, size_t with);

	bool is_defined(const basics::string<>& name) const;
	bool is_defined(const basics::string<>& name, basics::variant& def) const;
	basics::variant get_define(const basics::string<>& name) const;

	bool create_label(const basics::string<>& name, uint line);
	bool check_labels();

	/// correct the variable access identifier.
	static uint variable_correct_access(uint access);
	bool variable_scopename2id(const basics::string<>& name, int& uservalue) const;

	static const char* variable_getaccess(uint access);
	static const char* variable_getscope(uint access);
	static const char* variable_getstore(uint access);
	
	/// put a variable with known access to the program.
	/// create and initialite the var entry if not exists,
	/// cannot fail
	bool put_knownvariable(const parse_node &node, const basics::string<>& name, ulong flags, uint scope, uint access);
	/// put a variable with unknown access to the program.
	/// create pure temp variables as default,
	/// fail when multiple variables of same name exist
	bool put_variable(const parse_node &node, const basics::string<>& name, ulong flags, uint scope);
	/// create a variable.
	/// fails when variable already exists
	bool create_variable(const parse_node &node, const basics::string<>& name, uint access, uint scope, basics::var_t type, bool isconst, bool declared);
	bool exists_variable(const basics::string<>& name, uint access);
	bool check_variable(uint scope, bool final=true) const;
	bool export_variable(const parse_node &node, uint scope, CVariableScope& target) const;

	bool put_function_call(const parse_node &node, uint scope, const basics::string<>& name, bool membercall, bool global);
	bool put_subfunction_call(const parse_node &node, uint scope, const basics::string<>& host, const basics::string<>& name, bool membercall);


	///////////////////////////////////////////////////////////////////////////
	// define functions
	bool compile_define(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// include functions
	bool compile_include(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// label functions
	bool compile_label(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_goto(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_gosub(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// script variables
	bool compile_vardecl(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_variable(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// function declaration
	bool compare_declarations(const parse_node &namenode, const scriptdecl&a, const scriptdecl& b, const bool b_has_values);
	bool compile_funcdecl(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_subfuncdecl(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_parameter(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// function call
	bool build_function_parameter(const scriptdecl& decl, const parse_node &node, uint scope, int& paramcount);
	bool compile_function_call(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_subfunction_call(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_globalfunction_call(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// literals
	bool compile_identifier(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_numberliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_floatliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_charliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_stringliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_emptyliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_semiliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// statements
	bool compile_switchstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_ifstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_whilestm(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_forstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_dostm(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_lctrlstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_returnstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_regexpr(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opassign(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opif(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opor(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opand(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opbinary(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opunary(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_oppost(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_oppre(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opcast(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_opsizeof(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_memberfunc(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_membervariable(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_scopevariable(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_array(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_range(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_rangemod(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_eval(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_concat(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	bool compile_commalist(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_statements(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_declarations(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	///////////////////////////////////////////////////////////////////////////
	// objects
	bool compile_object(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_objlist(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_speclist(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_itemlist(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_item(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_itemoverwrite(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_instance(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_ordernpc(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_ordertouch(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_ordermob(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_orderwarp(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_orderscr(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_specitem(const parse_node &node, uint scope, unsigned long flags, int& uservalue);
	bool compile_block(const parse_node &node, uint scope, unsigned long flags, int& uservalue);

	//////////////////////////////////////////////////////////////////////////
	// main compile loop, is called recursively with all parse tree nodes
	bool compile_main(const parse_node &node, uint scope, unsigned long flags, int& uservalue);



	
	bool is_const() const;
	void put_nonconst();
	void put_command(command_t command);
	void put_intcommand(command_t command, int p);
	void put_strcommand(command_t command, const basics::string<>& s);
	void put_varcommand(command_t command, int64 i);


	void put_value_unchecked(double d);
	void put_value_unchecked(int64 i);
	void put_value_unchecked(const basics::string<>& s);
	void put_value_unchecked(const basics::variant& v);
	void put_value(double d, bool constval=true);
	void put_value(int64 i, bool constval=true);
	void put_value(const basics::string<>& s, bool constval=true);
	void put_value(const basics::variant& v, bool constval=true);
	bool is_nonconst_operation(const parse_node &node) const;


public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	eacompiler();
	~eacompiler();

	///////////////////////////////////////////////////////////////////////////
	/// compiler entry point.
	bool compile(const parse_node &node);

	bool compile_file(const basics::string<>& filename, int options, bool forced=false);
};







///////////////////////////////////////////////////////////////////////////////
/*	code transformations on control structures
	///////////////////////////////////////////////////////
	break;	  => ignored by default (warn)
	///////////////////////////////////////////////////////
	continue; => ignored by default (warn)
	///////////////////////////////////////////////////////
	<Label Stm>     ::= Id ':'
	=> 
	register in label list
	///////////////////////////////////////////////////////
	if '(' <Expr> ')' <Normal Stm>
	=>
	<Expr>				// puts the result on the stack
	_opnif_ label1		// take one value from stack returning the offset
						// (1 or the branch offset)
	<Normal Stm1>		// 
	label1				// branch offset is leading here
	///////////////////////////////////////////////////////
	if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
	=>
	<Expr>				// puts the result on the stack
	_opnif_ label1		// take one value from stack returning the offset
						// (1 or the branch offset)
	<Normal Stm1>		// 
	goto label2			// jump over Normal Stm2
	label1				// branch offset1 is leading here
	<Normal Stm2>		// 
	label2				// branch offset2 is leading here
	///////////////////////////////////////////////////////
	while '(' <Expr> ')' <Normal Stm>
	=>
	label1				// branch offset1 is leading here
	<Expr>				// puts the result on the stack
	_opnif_ label2		// take one value from stack returning the offset
						// (1 or the branch offset)
	<Normal Stm> 
	goto label1			// jump offset1
	label2				// branch offset2 is leading here
	
	break;	  => goto label2
	continue; => goto label1
	///////////////////////////////////////////////////////
	for '(' <Arg1> ';' <Arg2> ';' <Arg3> ')' <Normal Stm>
	=>
	<Arg1>
	label1
	<Arg2>
	_opif!_ label3
	<Normal Stm>
	label2
	<Arg3>
	goto label1
	label3

	break;	  => goto label3
	continue; => goto label2
	///////////////////////////////////////////////////////
	do <Normal Stm> while '(' <Expr> ')' ';'
	=>
	label1
	<Normal Stm>
	<Expr> -> _opif_ label1
	label 2

	break;	  => goto label2
	continue; => goto label1
	///////////////////////////////////////////////////////
	switch '(' <Expr> ')' '{' <Case Stms> '}'
	<Case Stms>  ::= case <Value> ':' <Stm List> <Case Stms>
               | default ':' <Stm List>
               |
	=>
	<Exprtemp> = <Expr>
	<Exprtemp> == <Case Stms1>.<Value> -> _opif_ label1
	<Exprtemp> == <Case Stms2>.<Value> -> _opif_ label2
	<Exprtemp> == <Case Stms3>.<Value> -> _opif_ label3
	goto label_default
	label1
	<Stm List1>
	label2
	<Stm List2>
	label3
	<Stm List3>
	label_default
	<Stm Listdef>
	label_end

	break;	  => goto label_end
	continue  => warn/ignored
	///////////////////////////////////////////////////////
	<a> || <b>
	=>
	<a>
	_opifpop_ label1
	<b>
	label1
	///////////////////////////////////////////////////////
	<a> && <b>
	=>
	<a>
	_opnifpop_ label1
	pop
	<b>
	label1
	///////////////////////////////////////////////////////
	<a>?<b>:<c>
	=>
	<a>
	_opnif_ label1
	<b>
	goto labelend
	label1
	<c>
	labelend
	///////////////////////////////////////////////////////
*/
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
#endif//_EACOMPILER_
