// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#ifndef _EACOMPILER_
#define _EACOMPILER_

#include "basesync.h"
#include "baseparser.h"



///////////////////////////////////////////////////////////////////////////////
// terminal definitions from parse tree
#include "eascript.h"

USING_NAMESPACE(basics)


///////////////////////////////////////////////////////////////////////////////
// command opcodes for stack interpreter
typedef enum
{
	/////////////////////////////////////////////////////////////////
	// "no operation" as usual
	OP_NOP				=  0,	

	/////////////////////////////////////////////////////////////////
	// assignment operaations
	// take two stack values and push up one
	OP_ASSIGN			=  1,	// <Op If> '='   <Op>
	OP_ASSIGN_ADD		=  2,	// <Op If> '+='  <Op>
	OP_ASSIGN_SUB		=  3,	// <Op If> '-='  <Op>
	OP_ASSIGN_MUL		=  4,	// <Op If> '*='  <Op>
	OP_ASSIGN_DIV		=  5,	// <Op If> '/='  <Op>
	OP_ASSIGN_XOR		=  6,	// <Op If> '^='  <Op>
	OP_ASSIGN_AND		=  7,	// <Op If> '&='  <Op>
	OP_ASSIGN_OR		=  8,	// <Op If> '|='  <Op>
	OP_ASSIGN_RSH		=  9,	// <Op If> '>>=' <Op>
	OP_ASSIGN_LSH		= 10,	// <Op If> '<<=' <Op>

	/////////////////////////////////////////////////////////////////
	// select operation
	// take three stack values and push the second or third depending on the first
	OP_SELECT			= 11,	// <Op Or> '?' <Op If> ':' <Op If>

	/////////////////////////////////////////////////////////////////
	// logic operations
	// take two stack values and push a value
	OP_LOG_OR			= 12,	// <Op Or> '||' <Op And>
	OP_LOG_AND			= 13,	// <Op And> '&&' <Op BinOR>
	OP_BIN_OR			= 14,	// <Op BinOr> '|' <Op BinXOR>
	OP_BIN_XOR			= 15,	// <Op BinXOR> '^' <Op BinAND>
	OP_BIN_AND			= 16,	// <Op BinAND> '&' <Op Equate>

	/////////////////////////////////////////////////////////////////
	// compare operations
	// take two stack values and push a boolean value
	OP_EQUATE			= 17,	// <Op Equate> '==' <Op Compare>
	OP_UNEQUATE			= 18,	// <Op Equate> '!=' <Op Compare>
	OP_ISGT				= 19,	// <Op Compare> '>'  <Op Shift>
	OP_ISGTEQ			= 20,	// <Op Compare> '>=' <Op Shift>
	OP_ISLT				= 21,	// <Op Compare> '<'  <Op Shift>
	OP_ISLTEQ			= 22,	// <Op Compare> '<=' <Op Shift>

	/////////////////////////////////////////////////////////////////
	// shift operations
	// take two stack values and push a value
	OP_LSHIFT			= 23,	// <Op Shift> '<<' <Op AddSub>
	OP_RSHIFT			= 24,	// <Op Shift> '>>' <Op AddSub>

	/////////////////////////////////////////////////////////////////
	// add/sub operations
	// take two stack values and push a value
	OP_ADD				= 25,	// <Op AddSub> '+' <Op MultDiv>
	OP_SUB				= 26,	// <Op AddSub> '-' <Op MultDiv>

	/////////////////////////////////////////////////////////////////
	// mult/div/modulo operations
	// take two stack values and push a value
	OP_MUL				= 27,	// <Op MultDiv> '*' <Op Unary>
	OP_DIV				= 28,	// <Op MultDiv> '/' <Op Unary>
	OP_MOD				= 29,	// <Op MultDiv> '%' <Op Unary>

	/////////////////////////////////////////////////////////////////
	// unary operations
	// take one stack values and push a value
	OP_NOT				= 30,	// '!'    <Op Unary>
	OP_INVERT			= 31,	// '~'    <Op Unary>
	OP_NEGATE			= 32,	// '-'    <Op Unary>

	/////////////////////////////////////////////////////////////////
	// sizeof operations
	// take one stack values and push the result
								// sizeof '(' <Type> ')' // replaces with OP_PUSH_INT on compile time
	OP_SIZEOF			= 33,	// sizeof '(' Id ')'

	/////////////////////////////////////////////////////////////////
	// cast operations
	// take one stack values and push the result
	OP_CAST				= 34,	// '(' <Type> ')' <Op Unary>   !CAST

	/////////////////////////////////////////////////////////////////
	// Pre operations
	// take one stack variable and push a value
	OP_PREADD			= 35,	// '++'   <Op Pointer>
	OP_PRESUB			= 36,	// '--'   <Op Pointer>

	/////////////////////////////////////////////////////////////////
	// Post operations
	// take one stack variable and push a value
	OP_POSTADD			= 37,	// <Op Pointer> '++'
	OP_POSTSUB			= 38,	// <Op Pointer> '--'

	/////////////////////////////////////////////////////////////////
	// Member Access
	// take a variable and a value from stack and push a varible
	OP_MEMBER			= 39,	// <Op Pointer> '.' <Value>     ! member


	/////////////////////////////////////////////////////////////////
	// Array
	// take a variable and a value from stack and push a varible
	OP_ARRAY			= 40,	// <Op Pointer> '[' <Expr> ']'  ! array


	/////////////////////////////////////////////////////////////////
	// standard function calls
	// check the parameters on stack before or inside the call of function
	OP_CALLSCRIPT1		= 41,	// followed by the number on the script 1byte
	OP_CALLSCRIPT2		= 42,	// followed by the number on the script 2byte
	OP_CALLSCRIPT3		= 43,	// followed by the number on the script 3byte
	OP_CALLSCRIPT4		= 44,	// followed by the number on the script 4byte
	OP_CALLBUILDIN1		= 45,	// followed by the number on the function 1byte
	OP_CALLBUILDIN2		= 46,	// followed by the number on the function 2byte
	OP_CALLBUILDIN3		= 47,	// followed by the number on the function 3byte
	OP_CALLBUILDIN4		= 48,	// followed by the number on the function 4byte
								// Id '(' <Expr> ')'
								// Id '(' ')'
								// Id <Call List> ';'
								// Id ';'
	// followed by the function id
	// followed by the number of valid parameters

	/////////////////////////////////////////////////////////////////
	// explicit stack pushes
	// Values pushed on stack directly
								// HexLiteral
								// DecLiteral
								// StringLiteral
								// CharLiteral
								// FloatLiteral
								// Id
								// <Call Arg>  ::= '-'

	OP_PUSH_ADDR		=49,	// followed by an address
	OP_PUSH_INT1		=50,	// followed by an integer 1byte
	OP_PUSH_INT2		=51,	// followed by an integer 2byte
	OP_PUSH_INT3		=52,	// followed by an integer 3byte
	OP_PUSH_INT4		=53,	// followed by an integer 4byte
	OP_PUSH_STRING		=54,	// followed by a string strlen() bytes + EOS
	OP_PUSH_FLOAT		=55,	// followed by a float 4 byte
	OP_PUSH_VAR			=56,	// followed by a string containing the variable name strlen() bytes + EOS
	OP_PUSH_VALUE		=57,	// followed by a string containing the variable name strlen() bytes + EOS
	OP_PUSH_PARAM		=58,	// followed by the number of the parameter 1byte
	OP_PUSH_TEMPVAR1	=59,	// followed by the number of the temp variable 1byte
	OP_PUSH_TEMPVAR2	=60,	// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVAR3	=61,	// followed by the number of the temp variable 3byte
	OP_PUSH_TEMPVAR4	=62,	// followed by the number of the temp variable 4byte
	OP_PUSH_TEMPVALUE1	=63,	// followed by the number of the temp variable 1byte 
	OP_PUSH_TEMPVALUE2	=64,	// followed by the number of the temp variable 2byte
	OP_PUSH_TEMPVALUE3	=65,	// followed by the number of the temp variable 3byte
	OP_PUSH_TEMPVALUE4	=66,	// followed by the number of the temp variable 4byte
	OP_VECTORIZE1		=67,	// followed by the number of array elements 1byte 
	OP_VECTORIZE2		=68,	// followed by the number of array elements 2byte
	OP_VECTORIZE3		=69,	// followed by the number of array elements 3byte
	OP_VECTORIZE4		=70,	// followed by the number of array elements 4byte
	OP_RESIZE			=71,	// followed by dimension (char), resize a var array, remove elements or pad variables of type NONE

	OP_CLEAR			=72,	// clear a variable
	OP_POP				=73,	// clear the stack

	OP_START			=74,	// Program Start followed by 3byte Programm length
	OP_END				=75,	// Quit the interpreter immediately
	OP_RETURN			=76,	// return, quit if last scope	


	// Jumps
	/////////////////////////////////////////////////////////////////
	// conditional branch
	OP_NIF				=77,	// if '(' <Expr> ')' <Normal Stm> followed by the target address
	OP_IF				=78,	// if '(' <Expr> ')' <Normal Stm> followed by the target address
	/////////////////////////////////////////////////////////////////
	// unconditional branch
	OP_GOTO				=79,	// goto Id ';' followed by the target address

	VX_LABEL			=80,	// temporary node followed by a temporary target address 0
	VX_BREAK			=81,	// temporary node followed by a temporary target address 0
	VX_CONT				=82,	// temporary node followed by a temporary target address 0
	VX_GOTO				=83,	// direct jump node followed by the target address

};







///////////////////////////////////////////////////////////////////////////////
/*
	necessary code transformations on control structures

	break;	  => ignored by default (warn)
	continue; => ignored by default (warn)

	<Label Stm>     ::= Id ':'
	=> 
	register in label list


	if '(' <Expr> ')' <Normal Stm>
	=>

	<Expr>				// puts the result on the stack
	_opnif_ label1		// take one value from stack returning the offset (1 or the branch offset)
	<Normal Stm1>		// 
	label1				// branch offset is leading here


	if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
	=>

	<Expr>				// puts the result on the stack
	_opnif_ label1		// take one value from stack returning the offset (1 or the branch offset)
	<Normal Stm1>		// 
	goto label2			// jump over Normal Stm2
	label1				// branch offset1 is leading here
	<Normal Stm2>		// 
	label2				// branch offset2 is leading here


	while '(' <Expr> ')' <Normal Stm>
	=>

	label1				// branch offset1 is leading here
	<Expr>				// puts the result on the stack
	_opnif_ label2		// take one value from stack returning the offset (1 or the branch offset)
	<Normal Stm> 
	goto label1			// jump offset1
	label2				// branch offset2 is leading here

	break;	  => goto label2
	continue; => goto label1



	for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
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

	
	do <Normal Stm> while '(' <Expr> ')' ';'
	=>
	
	label1
	<Normal Stm>
	<Expr> -> _opif_ label1
	label 2

	break;	  => goto label2
	continue; => goto label1



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

*/













///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern inline uint32 axtoi(const char *hexStg)
{
	uint32 ret = 0;
	size_t cnt = 0;
	if( hexStg && hexStg[0]=='0' && hexStg[1]=='x' )
	{	
		hexStg+=2;
		while(*hexStg && (cnt++ < 2*sizeof(uint32)) )
		{
			if( (*hexStg>='0' && *hexStg<='9') )
				ret = (ret<<4) | 0x0F & (*hexStg - '0');
			else if( (*hexStg>='A' && *hexStg<='F') )
				ret = (ret<<4) | 0x0F & (*hexStg - 'A' + 10);
			else if( (*hexStg>='a' && *hexStg<='f') )
				ret = (ret<<4) | 0x0F & (*hexStg - 'a' + 10);
			else
				break;
			hexStg++;
		}
	}
	return ret;
}



///////////////////////////////////////////////////////////////////////////////
// this->logging output collector/rerouter
///////////////////////////////////////////////////////////////////////////////
class CLogger
{
	int enable;
	int do_print(const char *fmt, va_list& argptr)
	{
		int ret=0;
		static char		tempbuf[4096]; // initially using a static fixed buffer size 
		static Mutex	mtx;
		ScopeLock		sl(mtx);
		size_t sz  = 4096; // initial buffer size
		char *ibuf = tempbuf;


		if(fmt)
		{
			if(argptr)
			{
				do
				{	// print
					if( vsnprintf(ibuf, sz, fmt, argptr) >=0 ) // returns -1 in case of error
						break; // print ok, can break
					// otherwise
					// free the memory if it was dynamically alloced
					if(ibuf!=tempbuf) delete[] ibuf;
					// double the size of the buffer
					sz *= 2;
					ibuf = new char[sz];
					// and loop in again
				}while(1); 
				// ibuf contains the printed string
				ret = output(ibuf);
			}
			else
			{	// thust the format string, no parameter
				ret = output(fmt);
			}
		}
		if(ibuf!=tempbuf) delete[] ibuf;
		return ret;
	}
public:
	CLogger(int e=2) : enable(e)		{}
	virtual ~CLogger()					{}

	int logging(const char *fmt, ...)
	{
		int ret = 0;
		if(enable>=2)
		{
			va_list argptr;
			va_start(argptr, fmt);
			ret = do_print(fmt, argptr);
			va_end(argptr);

		}
		return ret;
	}
	int warning(const char *fmt, ...)
	{
		int ret = 0;
		if(enable>=1)
		{
			va_list argptr;
			va_start(argptr, fmt);
			ret = do_print(fmt, argptr);
			va_end(argptr);

		}
		return ret;
	}
	int error(const char *fmt, ...)
	{
		int ret = 0;
		if(enable>=0)
		{
			va_list argptr;
			va_start(argptr, fmt);
			ret = do_print(fmt, argptr);
			va_end(argptr);

		}
		return ret;
	}
	virtual int output(const char* str)
	{
		int ret = printf(str);
		fflush(stdout);
		return ret;
	}
};






///////////////////////////////////////////////////////////////////////////////
//
// Parse Tree Transformation
//  * Simplification 
//    - node reduction (ok)
//    - list unrolling (ok)
//  * Optimisation
//    - combining constants ( )
//    - removing unreachable nodes ( )
//    - reordering integer multiplication/division ( )
//
///////////////////////////////////////////////////////////////////////////////

class parsenode : public global, public CLogger
{
	///////////////////////////////////////////////////////////////////////////
	// types
	typedef parsenode* parsenodep;

	///////////////////////////////////////////////////////////////////////////
	// class data
	parsenodep*				cList;
	size_t					cCount;
	unsigned short			cType;
	unsigned short			cSymbol;
	string<>				cSymbolName;
	string<>				cLexeme;
	unsigned short			cLine;
	unsigned short			cColumn;


	void insertnode(unsigned short t, unsigned short s, const string<>& n, const string<>& l, unsigned short line, unsigned short col)
	{
		// add element to List
		parsenodep* temp = new parsenodep[cCount+1];
		if(cList)
		{
			memcpy(temp,cList,cCount*sizeof(parsenodep));
			delete[] cList;
		}
		cList = temp;
		cList[cCount] = new parsenode(t,s,n,l,line,col);
		cCount++;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	parsenode(): cList(NULL),cCount(0),cType(0),cSymbol(0)	{}
	parsenode(unsigned short t, unsigned short s, const string<>& n, const string<>& l, unsigned short line, unsigned short col)
		:  cList(NULL),cCount(0),cType(t),cSymbol(s),cSymbolName(n),cLexeme(l),cLine(line),cColumn(col)
	{}

	parsenode(const CParser& parser) :  cList(NULL),cCount(0),cType(0),cSymbol(0)
	{
		parsenode temp;
		temp.reduce_tree(parser,0, 0);
		if(temp.cCount >=1 && temp.cList && temp.cList[0])
		{	// take the first child node out
			cList		= temp.cList[0]->cList;
			cCount		= temp.cList[0]->cCount;
			cType		= temp.cList[0]->cType;
			cSymbol		= temp.cList[0]->cSymbol;
			cSymbolName	= temp.cList[0]->cSymbolName;
			cLexeme		= temp.cList[0]->cLexeme;
			cLine		= temp.cList[0]->cLine;
			cColumn		= temp.cList[0]->cColumn;
			// and put a dummy parsenode as list element in
			temp.cList[0]->cList = NULL;
		}
	}
	~parsenode()
	{	
		if(cList)
		{	// clear childs
			for(size_t i=0; i<cCount; ++i)
				if(cList[i]) delete cList[i];
			// clear list
			delete [] cList;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// element access
	const char* Lexeme()	const	{ return cLexeme; }
	const string<>& LexemeObj() const { return cLexeme; }
	const char*SymbolName()	const	{ return cSymbolName; }
	unsigned short Symbol()	const	{ return cSymbol; }
	unsigned short Type()	const	{ return cType; }
	size_t count()	const			{ return cCount; }
	unsigned short Line() const		{ return cLine; }
	unsigned short Column() const	{ return cColumn; }

	const parsenode& operator[](size_t inx) const	{ return (inx<cCount)?(*(cList[inx])):(*this); }

	///////////////////////////////////////////////////////////////////////////
	// parse tree functions
	void print_tree(size_t level=0)
	{
		size_t i;
		for(i=0; i<level; ++i) this->logging("| ");
		this->logging("%c-<%s>(%i)[%i] ::= '%s'\n", 
			(cType)?'T':'N', 
			(const char*)cSymbolName,
			cSymbol,
			cCount, 
			(const char*)cLexeme );

		for(i=0; i<cCount; ++i)
			if(cList[i]) cList[i]->print_tree(level+1);
	}

	void reduce_tree(const CParser& parser, int rtpos, int flat)
	{
		size_t j, k;
		const CStackElement* se = &parser.rt[rtpos];
		const CStackElement* child;
		if( se->cChildNum==1 )
		{
			this->reduce_tree(parser, se->cChildPos, 0);
		}
		else
		{
			parsenodep newlist = this;
			if( flat==0)
			{
				this->insertnode(se->symbol.Type, se->symbol.idx, se->symbol.Name, se->cToken.cLexeme, se->cToken.line, se->cToken.column);
				newlist = this->cList[this->cCount-1];
			}
			k = se->cChildPos+se->cChildNum;
			for(j=se->cChildPos; j<k; ++j)
			{
				child = &parser.rt[j];
				if(child->symbol.Type != 1) // non terminal
				{
					flat = 0;
					// list layout
					if( se->cChildNum==2 &&
						child->symbol.idx == se->symbol.idx )
						flat = 1;
					else if( se->cChildNum==3 &&
						child->symbol.idx == se->symbol.idx &&
						PT_COMMA==(parser.rt[se->cChildPos+1]).symbol.idx )
						flat = 1;
					newlist->reduce_tree(parser, j, flat);
				}
				else
				{
					newlist->insertnode(child->symbol.Type, child->symbol.idx, child->symbol.Name, child->cToken.cLexeme, se->cToken.line, se->cToken.column);
				}
			}
		}
	}
};







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CProgramm : public CLogger
{
	friend class UserStack;

protected:
	typedef struct
	{
	public:
		unsigned char	cCommand;
		int				cParam1;
		int				cParam2;
		const char*		cString;
	} CCommand;
	class CLabel : public string<>
	{	
	public:
		uint valid : 1;		// correct address is set
		uint _dummy : 7;	// unused bits to fill the 32bit gap
		uint pos :24;		// 24bit address inside the script
		uint use;			// usage counter of the label

		CLabel(const char* n=NULL, int p=-1) : string<>(n), valid(0), pos(p),use(0)	{}
		virtual ~CLabel()	{}
	};

	slist<CLabel>			cLabelList;	// label list

	vector<uchar>			cProgramm;	// the stack programm
	size_t					cVarCnt;	// number of temporal variables
public:
	///////////////////////////////////////////////////////////////////////////
	// construct destruct
	CProgramm() : cVarCnt(0)
	{}
	virtual ~CProgramm()
	{}

	///////////////////////////////////////////////////////////////////////////
	// compares
	virtual bool operator==(const CProgramm& p) const	{ return this==&p; }
	virtual bool operator!=(const CProgramm& p) const	{ return this!=&p; }


	///////////////////////////////////////////////////////////////////////////
	// 
	size_t size()				{ return cProgramm.size(); }
	size_t getCurrentPosition()	{ return cProgramm.size(); }

	///////////////////////////////////////////////////////////////////////////
	// fetch command and parameters; and go to next command
	bool getCommand(size_t &inx, CCommand& cmd)
	{
		cmd.cCommand = getCommand(inx);
		switch( cmd.cCommand )
		{
			// commands followed by an int (1 byte)
			case OP_PUSH_PARAM:
			case OP_PUSH_INT1:
			case OP_PUSH_TEMPVAR1:
			case OP_PUSH_TEMPVALUE1:
			case OP_VECTORIZE1:
			case OP_RESIZE:
			case OP_CAST:
				cmd.cParam1 = getChar(inx);
				break;
			// commands followed by an int (2 byte)
			case OP_PUSH_INT2:
			case OP_PUSH_TEMPVAR2:
			case OP_PUSH_TEMPVALUE2:
			case OP_VECTORIZE2:
				cmd.cParam1 = getShort(inx);
				break;
			// commands followed by an int (3 byte)
			case OP_PUSH_INT3:
			case OP_PUSH_TEMPVAR3:
			case OP_PUSH_TEMPVALUE3:
			case OP_VECTORIZE3:
				cmd.cParam1 = getAddr(inx);
				break;
			// commands followed by an int or float (4 byte)
			case OP_PUSH_INT4:
			case OP_PUSH_TEMPVAR4:
			case OP_PUSH_TEMPVALUE4:
			case OP_VECTORIZE4:
			case OP_PUSH_FLOAT:
			case OP_START:
				cmd.cParam1 = getInt(inx);
				break;
			// commands followed by an address
			case OP_NIF:
			case OP_IF:
			case VX_BREAK:
			case VX_CONT:
			case VX_GOTO:
			case OP_GOTO:
			case OP_PUSH_ADDR:
				cmd.cParam1 = getAddr(inx);
				break;

			// commands followed by an int (1 byte) and a second unsigned char param
			case OP_CALLBUILDIN1:
			case OP_CALLSCRIPT1:
				cmd.cParam1 = getChar(inx);
				cmd.cParam2 = getChar(inx);
				break;

			// commands followed by an int (2 byte) and a second unsigned char param
			case OP_CALLBUILDIN2:
			case OP_CALLSCRIPT2:
				cmd.cParam1 = (unsigned short)getShort(inx);
				cmd.cParam2 = getChar(inx);
				break;
			// commands followed by an int (3 byte) and a second unsigned char param
			case OP_CALLBUILDIN3:
			case OP_CALLSCRIPT3:
				cmd.cParam1 = getAddr(inx);
				cmd.cParam2 = getChar(inx);
				break;

			// commands followed by an int (4 byte) and a second unsigned char param
			case OP_CALLBUILDIN4:
			case OP_CALLSCRIPT4:
				cmd.cParam1 = getInt(inx);
				cmd.cParam2 = getChar(inx);
				break;

			// commands followed by a string
			case OP_PUSH_VAR:
			case OP_PUSH_VALUE:
			case OP_PUSH_STRING:
				cmd.cString = getString(inx);
				break;
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	size_t nextCommand(size_t pos)
	{
		switch( getCommand(pos) )
		{
		// commands with no parameters
		case OP_NOP:
		case OP_ASSIGN:
		case OP_ASSIGN_ADD:
		case OP_ASSIGN_SUB:
		case OP_ASSIGN_MUL:
		case OP_ASSIGN_DIV:
		case OP_ASSIGN_XOR:
		case OP_ASSIGN_AND:
		case OP_ASSIGN_OR:
		case OP_ASSIGN_RSH:
		case OP_ASSIGN_LSH:
		case OP_SELECT:
		case OP_LOG_OR:
		case OP_LOG_AND:
		case OP_BIN_OR:
		case OP_BIN_XOR:
		case OP_BIN_AND:
		case OP_EQUATE:
		case OP_UNEQUATE:
		case OP_ISGT:
		case OP_ISGTEQ:
		case OP_ISLT:
		case OP_ISLTEQ:
		case OP_LSHIFT:
		case OP_RSHIFT:
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_MOD:
		case OP_NOT:
		case OP_INVERT:
		case OP_NEGATE:
		case OP_SIZEOF:
		case OP_PREADD:
		case OP_PRESUB:
		case OP_POSTADD:
		case OP_POSTSUB:
		case OP_MEMBER:
		case OP_ARRAY:
		case OP_CLEAR:
		case OP_POP:
		case OP_END:
		case OP_RETURN:
			return pos;

		// commands with int parameters (1 bytes)
		case OP_PUSH_PARAM:
		case OP_PUSH_INT1:
		case OP_PUSH_TEMPVAR1:
		case OP_PUSH_TEMPVALUE1:
		case OP_VECTORIZE1:
		case OP_RESIZE:
		case OP_CAST:
			return pos+1;

		// commands with int parameters (2 bytes)
		case OP_PUSH_INT2:
		case OP_PUSH_TEMPVAR2:
		case OP_PUSH_TEMPVALUE2:
		case OP_VECTORIZE2:
			return pos+2;

		// commands with int parameters (3 bytes)
		case OP_PUSH_INT3:
		case OP_PUSH_TEMPVAR3:
		case OP_PUSH_TEMPVALUE3:
		case OP_VECTORIZE3:
			return pos+3;

		// commands with int or float parameters (4 bytes)
		case OP_START:
		case OP_PUSH_FLOAT:
		case OP_PUSH_INT4:
		case OP_PUSH_TEMPVAR4:
		case OP_PUSH_TEMPVALUE4:
		case OP_VECTORIZE4:
			return pos+4;

		// commands with addr parameters
		case OP_NIF:
		case OP_IF:
		case VX_BREAK:
		case VX_CONT:
		case VX_GOTO:
		case OP_GOTO:
		case OP_PUSH_ADDR:
			return pos+4;

		// commands with addr (1 bytes)and char parameters 
		case OP_CALLSCRIPT1:
		case OP_CALLBUILDIN1:
			return pos+2;

		// commands with addr (2 bytes)and char parameters 
		case OP_CALLSCRIPT2:
		case OP_CALLBUILDIN2:
			return pos+3;

		// commands with addr (3 bytes)and char parameters 
		case OP_CALLSCRIPT3:
		case OP_CALLBUILDIN3:
			return pos+4;
		
		// commands with addr (4 bytes)and char parameters 
		case OP_CALLSCRIPT4:
		case OP_CALLBUILDIN4:
			return pos+5;

		// commands with string parameters (sizeof(pointer) bytes)
		case OP_PUSH_STRING:
		case OP_PUSH_VAR:
		case OP_PUSH_VALUE:
			getString(pos);
			return pos;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// label functions
	bool createLabel(const char* name)
	{	// add a label in labellist, update the target position in programm if given
		return cLabelList.insert(name);
	}
	bool appendLabel(const char* name)
	{	// appends the address of a label
		size_t inx;
		if( cLabelList.find(name,0, inx) )
		{	
			// check if label has been set
			if( cLabelList[inx].valid )
			{	// can just push the address
				appendAddr( cLabelList[inx].pos );
			}
			else
			{	// otherwise build a linked list of unset jump address fields
				size_t newstart = cProgramm.size();
				appendAddr( cLabelList[inx].pos );
				cLabelList[inx].pos = newstart;
			}
			cLabelList[inx].use++;
			return true;
		}
		return false;
	}
	bool correctLabel(const char* name)
	{	// we now have the correct position of the labels
		size_t inx;
		if( cLabelList.find(name,0, inx) && !cLabelList[inx].valid )
		{
			size_t pos = cProgramm.size();
			size_t tmp1,tmp2, addr=cLabelList[inx].pos;
			while( addr<pos )
			{
				tmp1 = addr;			// save the address since it is moving
				tmp2 = getAddr(tmp1);	// read the next address in the chain
				replaceAddr(pos, addr);	// update the address
				addr = tmp2;
			}
			cLabelList[inx].valid=1;
			cLabelList[inx].pos=pos;
			return true;
		}
		return false;
	}


	int useLabel(const char* name, int pos=-1)
	{	// add a label in labellist, update the target position in programm if given
		size_t inx;
		if( cLabelList.find(name,0, inx) )
		{	// tryed to reposition a label with already given position
			// should actually not happen since detected on label pre-run
			if(pos>=0 && cLabelList[inx].valid)	
				return -1;
			else if(pos>=0)					// have a position now
			{
				cLabelList[inx].pos = pos;
				cLabelList[inx].valid=1;
			}
			else							// otherwise just a usage of the label	
			cLabelList[inx].use++;
			return inx;
		}
		return -1;
	}
	bool isLabel(const char* name, size_t &inx)
	{	// check if name is a label and return the label identifier
		return cLabelList.find(name,0, inx);
	}
	const char *getLabelName(size_t inx)
	{	// return the name of a label for a given identifier
		if( inx<cLabelList.size() )
			return cLabelList[inx];
		return "";
	}
	int getLabelTarget(const char* name)
	{	// get the target position in programm of label by name
		size_t inx;
		if( cLabelList.find(name,0, inx) )
		{
			return cLabelList[inx].pos;
		}
		return -1;
	}
	int getLabelTarget(size_t inx)
	{	// get the target position in programm of label by identifier
		if( inx<cLabelList.size() )
			return cLabelList[inx].pos;
		return (cProgramm.size()>0) ? cProgramm.size()-1 : 0;	
		// in error case; return the last active program position (usually the end marker)
	}
	bool ConvertLabels()
	{	// convert labels into goto commands or push_addr commands
		CCommand ccmd;
		size_t i, pos, tmp;
		int inx;
		i=0;

		while( i<cProgramm.size() )
		{	// need to copy the position, 
			// it gets internally incremented on accesses
			pos=i;
			getCommand(i, ccmd);
			if( ccmd.cCommand==VX_GOTO )
			{	
				tmp = pos;
				// read the old command sequence
				getCommand(pos);
				inx = getAddr(pos);
				// recalculate
				inx = getLabelTarget( inx );
				// write the new command sequence
				replaceCommand(OP_GOTO,tmp);
				replaceAddr(inx,tmp);
			}
			else if( ccmd.cCommand==VX_LABEL )
			{	
				tmp = pos;
				// read the old command sequence
				getCommand(pos);
				inx = getAddr(pos);
				// recalculate
				inx = getLabelTarget( inx );
				// write the new command sequence
				replaceCommand(OP_PUSH_ADDR,tmp);
				replaceAddr(inx,tmp);
			}
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// replacing temporary jump targets
	bool replaceJumps(size_t start, size_t end, uchar cmd, int val)
	{	// convert a specific temporary jump command into goto commands
		CCommand ccmd;
		size_t pos, tmp;
		while( start<end && start<cProgramm.size() )
		{	// need to copy the position, 
			// it gets incremented on access internally
			pos=start;
			getCommand(start, ccmd);
			if( ccmd.cCommand==cmd )
			{	// just replace the command
				replaceCommand(OP_GOTO,pos);
				// replace the jump taget if not set already
				tmp=pos;
				if( 0==getAddr(pos) )
					replaceAddr(val,tmp);
			}
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// merging
	size_t append(const CProgramm& p)
	{
		size_t pos = this->getCurrentPosition();
		this->append(p);
		return pos;
	}
	
	///////////////////////////////////////////////////////////////////////////
	// access functions
	unsigned char getCommand(size_t &inx)
	{
		if( inx < cProgramm.size() )
			return cProgramm[inx++];
		return OP_END;
	}
	size_t insertCommand(unsigned char val, size_t &inx)
	{
		size_t pos = inx;
		cProgramm.insert(val, 1, inx++);
		return pos;
	}
	size_t replaceCommand(unsigned char val, size_t &inx)
	{
		size_t pos = inx;
		cProgramm[inx++] = val;
		return pos;
	}
	size_t appendCommand(unsigned char val)
	{
		size_t pos = cProgramm.size();
		cProgramm.append(val);
		return pos;
	}
	///////////////////////////////////////////////////////////////////////////
	unsigned char getChar(size_t &inx)
	{
		if( inx < cProgramm.size() )
			return cProgramm[inx++];
		return 0;
	}
	size_t insertChar(unsigned char val, size_t &inx)
	{
		size_t pos = inx;
		cProgramm.insert(val, 1, inx++);
		return pos;
	}
	size_t replaceChar(unsigned char val, size_t &inx)
	{
		size_t pos = inx;
		cProgramm[inx++] = val;
		return pos;
	}
	size_t appendChar(unsigned char val)
	{
		size_t pos = cProgramm.size();
		cProgramm.append(val);
		return pos;
	}
	///////////////////////////////////////////////////////////////////////////
	short getShort(size_t &inx)
	{	// getting a 16bit integer
		if( inx+1 < cProgramm.size() )
		{	
			return 	  ((unsigned short)cProgramm[inx++])
					| ((unsigned short)cProgramm[inx++]<<0x08);
		}
		return 0;
	}
	size_t insertShort(short val, size_t &inx)
	{	// setting a 16bit integer
		size_t pos = inx;
		cProgramm.insert(GetByte(val,0), 1, inx++);
		cProgramm.insert(GetByte(val,1), 1, inx++);
		return pos;
	}
	size_t replaceShort(short val, size_t &inx)
	{	// setting a 16bit integer
		size_t pos = inx;
		cProgramm[inx++] = GetByte(val,0);
		cProgramm[inx++] = GetByte(val,1);
		return pos;
	}
	size_t appendShort(int val)
	{	// setting a 16bit integer
		size_t pos = cProgramm.size();
		cProgramm.append(GetByte(val,0));
		cProgramm.append(GetByte(val,1));
		return pos;
	}
	///////////////////////////////////////////////////////////////////////////
	int getAddr(size_t &inx)
	{	// getting a 24bit integer
		if( inx+2 < cProgramm.size() )
		{	
			return 	  ((unsigned long)cProgramm[inx++])
					| ((unsigned long)cProgramm[inx++]<<0x08)
					| ((unsigned long)cProgramm[inx++]<<0x10);
		}
		return 0;
	}
	size_t insertAddr(int val, size_t &inx)
	{	// setting a 24bit integer
		size_t pos = inx;
		cProgramm.insert(GetByte(val,0), 1, inx++);
		cProgramm.insert(GetByte(val,1), 1, inx++);
		cProgramm.insert(GetByte(val,2), 1, inx++);
		return pos;
	}
	size_t replaceAddr(int val, size_t &inx)
	{	// setting a 24bit integer
		size_t pos = inx;
		cProgramm[inx++] = GetByte(val,0);
		cProgramm[inx++] = GetByte(val,1);
		cProgramm[inx++] = GetByte(val,2);
		return pos;
	}
	size_t appendAddr(int val)
	{	// setting a 24bit integer
		size_t pos = cProgramm.size();
		cProgramm.append(GetByte(val,0));
		cProgramm.append(GetByte(val,1));
		cProgramm.append(GetByte(val,2));
		return pos;
	}
	///////////////////////////////////////////////////////////////////////////
	int getInt(size_t &inx)
	{	// getting a 32bit integer
		if( inx+3 < cProgramm.size() )
		{	
			return 	  ((unsigned long)cProgramm[inx++])
					| ((unsigned long)cProgramm[inx++]<<0x08)
					| ((unsigned long)cProgramm[inx++]<<0x10)
					| ((unsigned long)cProgramm[inx++]<<0x18);
		}
		return 0;
	}
	size_t insertInt(int val, size_t &inx)
	{	// setting a 32bit integer
		size_t pos = inx;
		cProgramm.insert(GetByte(val,0), 1, inx++);
		cProgramm.insert(GetByte(val,1), 1, inx++);
		cProgramm.insert(GetByte(val,2), 1, inx++);
		cProgramm.insert(GetByte(val,3), 1, inx++);
		return pos;
	}
	size_t replaceInt(int val, size_t &inx)
	{	// setting a 32bit integer
		size_t pos = inx;
		cProgramm[inx++] = GetByte(val,0);
		cProgramm[inx++] = GetByte(val,1);
		cProgramm[inx++] = GetByte(val,2);
		cProgramm[inx++] = GetByte(val,3);
		return pos;
	}
	size_t appendInt(int val)
	{	// setting a 32bit integer
		size_t pos = cProgramm.size();
		cProgramm.append(GetByte(val,0));
		cProgramm.append(GetByte(val,1));
		cProgramm.append(GetByte(val,2));
		cProgramm.append(GetByte(val,3));
		return pos;
	}
	///////////////////////////////////////////////////////////////////////////
	// converting float via union, !!not portable!!
	static float int2float(int num)
	{	// getting a 32bit float
		union
		{
			float valf;
			unsigned char buf[4];
		}storage;
		storage.buf[0] = (unsigned char)(0xFF & (num));
		storage.buf[1] = (unsigned char)(0xFF & (num>>0x08));
		storage.buf[2] = (unsigned char)(0xFF & (num>>0x10));
		storage.buf[3] = (unsigned char)(0xFF & (num>>0x18));
		return storage.valf;
	}
	static int float2int(float num)
	{	// getting a 32bit float
		union
		{
			float valf;
			unsigned char buf[4];
		}storage;
		storage.valf = num;

		return	  (((unsigned int)storage.buf[0]) )
				| (((unsigned int)storage.buf[1]) << 0x08 )
				| (((unsigned int)storage.buf[2]) << 0x10)
				| (((unsigned int)storage.buf[3]) << 0x18);

	}
	///////////////////////////////////////////////////////////////////////////
/*
!! inserting pointers is unused
	const char* getTableString(size_t &inx)
	{	// getting a pointer, 64bit ready
		// msb to lsb order for faster reading
		if( inx+sizeof(size_t) <= cProgramm.size() )
		{
			size_t i, vali = 0;
			for(i=0; i<sizeof(size_t); ++i)
				vali = vali<<8 | ((size_t)cProgramm[inx++]);
			return (const char*)vali;
		}
		return "";
	}
	size_t insertTableString(const char*val, size_t &inx)
	{	// setting a pointer, 64bit ready
		// msb to lsb order for faster reading
		size_t i, vali = (size_t)val;
		size_t pos = inx;
		for(i=0; i<sizeof(size_t); ++i)
		{
			cProgramm.insert( vali&0xFF, 1, inx+sizeof(size_t)-1-i );
			vali >>= 8;
		}
		inx += sizeof(size_t);
		return pos;
	}
	size_t replaceTableString(const char*val, size_t &inx)
	{	// setting a pointer, 64bit ready
		// msb to lsb order for faster reading
		size_t i, vali = (size_t)val;
		size_t pos = inx;
		for(i=0; i<sizeof(size_t); ++i)
		{
			cProgramm[inx+sizeof(size_t)-1-i] = ( vali&0xFF );
			vali >>= 8;
		}
		inx += sizeof(size_t);
		return pos;
	}
	size_t appendTableString(const char*val)
	{	// setting a pointer, 64bit ready
		// msb to lsb order for faster reading
		size_t i, vali = (size_t)val;
		size_t pos = cProgramm.size();
		for(i=0; i<sizeof(size_t); ++i)
		{
			cProgramm.append( (vali>>(8*(sizeof(size_t)-1-i)))&0xFF);
		}
		return pos;
	}
*/
	///////////////////////////////////////////////////////////////////////////
	const char* getString(size_t &inx)
	{	
		register size_t i=inx;
		const char *str = (const char *)&(cProgramm[i]);
		// search for the EOS marker
		while( cProgramm[i] && i<cProgramm.size() )
			i++;
		if( i>=cProgramm.size() ) 
		{	// did not found a marker, there is some serious error
			inx++;
			return "";
		}
		else
		{
			inx=i+1;
			return str;
		}
	}
	size_t insertString(const char*val, size_t &inx)
	{	
		size_t pos = inx;
		size_t sz = 0;
		// insert the EOS marker
		cProgramm.insert( (unsigned char)0, 1, inx );
		if(val)
		{
			sz = strlen(val);
			cProgramm.insert( (unsigned char*)val, sz, inx);
		}
		inx += 1+sz;
		return pos;
	}
	size_t appendString(const char*val)
	{
		size_t pos = cProgramm.size();
		if(val)
		{
			cProgramm.append( (unsigned char*)val, 1+strlen(val) );
		}
		else
		{	// just append a EOS marker
			cProgramm.append( 0 );
		}
		return pos;
	}
	//!! no replacestring !!

	///////////////////////////////////////////////////////////////////////////
	// append variable size command, 
	// assuming command numbers for cmd1,2,3,4 are following directly
	size_t appendVarCommand(unsigned char cmd, int val)
	{
		size_t pos = cProgramm.size();
		// chars           0 ...        255
		// shorts     -32768 ...      32767
		// addr            0 ...   16777216
		// int   -2147483648 ... 2147483647
		if(val>= 0     && val<=255)
		{
			this->appendCommand(cmd+0);
			this->appendChar(val);
		}
		else if(val>=-32768 && val<=32767)
		{
			this->appendCommand(cmd+1);
			this->appendShort(val);
		}
		else if(val>= 0     && val<=16777216)
		{
			this->appendCommand(cmd+2);
			this->appendAddr(val);
		}
		else // -2147483648 ... 2147483647
		{
			this->appendCommand(cmd+3);
			this->appendInt(val);
		}
		return pos;
	}

	///////////////////////////////////////////////////////////////////////////
	// debug
	void dump()
	{
		size_t i;

		this->logging("binary output:\n");

		for(i=0; i<cProgramm.size(); ++i)
		{
			this->logging("%3i  ", cProgramm[i]);
			if(15==i%16) this->logging("\n");
		}
		this->logging("\n\n");
		this->logging("command sequence:\n");
		for(i=0; i<cProgramm.size(); printCommand(i),this->logging("\n"));

		this->logging("\n");
		this->logging("labels:\n");
		for(i=0; i<cLabelList.size(); ++i)
		{
			this->logging("'%s' jump to %i (used: %i)\n", (const char*)cLabelList[i], cLabelList[i].pos, cLabelList[i].use);
		}
		this->logging("\n");
	}
	void printCommand(size_t &pos)
	{
		this->logging("%4i: ",pos);
		CCommand ccmd;
		if( getCommand(pos, ccmd) )
		{
			switch( ccmd.cCommand )
			{
			// commands with no parameters
			case OP_NOP:
				this->logging("nop"); break;
			case OP_ASSIGN:
				this->logging("assign"); break;
			case OP_ASSIGN_ADD:
				this->logging("assign add"); break;
			case OP_ASSIGN_SUB:
				this->logging("assign sub"); break;
			case OP_ASSIGN_MUL:
				this->logging("assign mul"); break;
			case OP_ASSIGN_DIV:
				this->logging("assign div"); break;
			case OP_ASSIGN_XOR:
				this->logging("assign xor"); break;
			case OP_ASSIGN_AND:
				this->logging("assign and"); break;
			case OP_ASSIGN_OR:
				this->logging("assign or"); break;
			case OP_ASSIGN_RSH:
				this->logging("assign rightshift"); break;
			case OP_ASSIGN_LSH:
				this->logging("assign leftshift"); break;
			case OP_SELECT:
				this->logging("select"); break;
			case OP_LOG_OR:
				this->logging("logic or"); break;
			case OP_LOG_AND:
				this->logging("logic and"); break;
			case OP_BIN_OR:
				this->logging("binary or"); break;
			case OP_BIN_XOR:
				this->logging("binary xor"); break;
			case OP_BIN_AND:
				this->logging("binary and"); break;
			case OP_EQUATE:
				this->logging("equal"); break;
			case OP_UNEQUATE:
				this->logging("uneqal"); break;
			case OP_ISGT:
				this->logging("compare greater then"); break;
			case OP_ISGTEQ:
				this->logging("compare greater/equal then"); break;
			case OP_ISLT:
				this->logging("compare less then"); break;
			case OP_ISLTEQ:
				this->logging("compare less/equal then"); break;
			case OP_LSHIFT:
				this->logging("leftshift"); break;
			case OP_RSHIFT:
				this->logging("rightshift"); break;
			case OP_ADD:
				this->logging("add"); break;
			case OP_SUB:
				this->logging("sub"); break;
			case OP_MUL:
				this->logging("mul"); break;
			case OP_DIV:
				this->logging("div"); break;
			case OP_MOD:
				this->logging("modulo"); break;
			case OP_NOT:
				this->logging("logic not"); break;
			case OP_INVERT:
				this->logging("binary invert"); break;
			case OP_NEGATE:
				this->logging("arithmetic negate"); break;
			case OP_SIZEOF:
				this->logging("sizeof"); break;
			case OP_PREADD:
				this->logging("preop add"); break;
			case OP_PRESUB:
				this->logging("preop sub"); break;
			case OP_POSTADD:
				this->logging("postop add"); break;
			case OP_POSTSUB:
				this->logging("postop sub"); break;
			case OP_MEMBER:
				this->logging("member access"); break;
			case OP_ARRAY:
				this->logging("array access"); break;
			case OP_CLEAR:
				this->logging("clear variable"); break;
			case OP_POP:
				this->logging("pop stack"); break;
			case OP_END:
				this->logging("quit"); break;
			case OP_RETURN:
				this->logging("return"); break;

			// commands with int (float) parameters (4 bytes)
			case OP_START:
				this->logging("start (progsize=%i)", ccmd.cParam1); break;
			case OP_PUSH_ADDR:
				this->logging("push addr '%i'", ccmd.cParam1); break;
			case OP_PUSH_INT1:
				this->logging("push uchar '%i'", ccmd.cParam1); break;
			case OP_PUSH_INT2:
				this->logging("push short '%i'", ccmd.cParam1); break;
			case OP_PUSH_INT3:
				this->logging("push int3 '%i'", ccmd.cParam1); break;
			case OP_PUSH_INT4:
				this->logging("push int '%i'", ccmd.cParam1); break;
			case OP_PUSH_FLOAT:
				this->logging("push float '%f'", int2float(ccmd.cParam1) ); break;
			case OP_PUSH_PARAM:
				this->logging("push function parameter '%i'", ccmd.cParam1); break;
			case OP_PUSH_TEMPVAR1:
			case OP_PUSH_TEMPVAR2:
			case OP_PUSH_TEMPVAR3:
			case OP_PUSH_TEMPVAR4:
				this->logging("push temp variable '%i'", ccmd.cParam1); break;
			case OP_PUSH_TEMPVALUE1:
			case OP_PUSH_TEMPVALUE2:
			case OP_PUSH_TEMPVALUE3:
			case OP_PUSH_TEMPVALUE4:
				this->logging("push value from temp variable '%i'", ccmd.cParam1); break;
			case OP_RESIZE:
				this->logging("array resize (%i dimension(s))", ccmd.cParam1); break;
			case OP_CAST:
				this->logging("cast to %i", ccmd.cParam1); break;
			case OP_VECTORIZE1:
			case OP_VECTORIZE2:
			case OP_VECTORIZE3:
			case OP_VECTORIZE4:
				this->logging("vectorize '%i' elements", ccmd.cParam1); break;

			case OP_NIF:
				this->logging("conditional jump on false to '%i'", ccmd.cParam1); break;
			case OP_IF:
				this->logging("conditional jump on true to '%i'", ccmd.cParam1); break;
			case OP_GOTO:
				this->logging("jump to '%i'", ccmd.cParam1); break;
			case VX_BREAK:
				this->logging("break (error: not converted) '%i'", ccmd.cParam1); break;
			case VX_CONT:
				this->logging("continue (error: not converted) '%i'", ccmd.cParam1); break;
			case VX_GOTO:
				this->logging("jump to '%i' (error: not converted)", ccmd.cParam1); break;
			case VX_LABEL:
			{
				size_t inx = ccmd.cParam1;
				this->logging("label '%s' jump to %i (error: not converted)", getLabelName(inx), getLabelTarget(inx)); break;
			}
				

			// commands with int and char parameters
			case OP_CALLSCRIPT1:
			case OP_CALLSCRIPT2:
			case OP_CALLSCRIPT3:
			case OP_CALLSCRIPT4:
				this->logging("call script '%i' (%i args)", ccmd.cParam1, ccmd.cParam2); break;
			case OP_CALLBUILDIN1:
			case OP_CALLBUILDIN2:
			case OP_CALLBUILDIN3:
			case OP_CALLBUILDIN4:
				this->logging("call buildin '%i' (%i args)", ccmd.cParam1, ccmd.cParam2); break;


			// commands with string parameters (sizeof(pointer) bytes)
			case OP_PUSH_STRING:
				this->logging("push string '%s'", ccmd.cString); break;
			case OP_PUSH_VAR:
				this->logging("push global variable '%s'", ccmd.cString); break;
			case OP_PUSH_VALUE:
				this->logging("push value from global variable '%s'", ccmd.cString); break;
			}

		}
	}
};


class CBuildin
{
	string<>	cID;
	size_t		cParaCnt;
public:
	///////////////////////////////////////////////////////////////////////////
	// construct destruct
	CBuildin() : cID((const char*)NULL),cParaCnt(0)
	{}
	CBuildin(const char* n, size_t p=0) : cID(n),cParaCnt(p)
	{}
	virtual ~CBuildin()
	{}
	///////////////////////////////////////////////////////////////////////////
	// compares
	virtual bool operator==(const char *name) const	{ return cID==name; }
	virtual bool operator!=(const char *name) const	{ return cID!=name; }
	virtual bool operator==(const CBuildin& p) const	{ return this==&p; }
	virtual bool operator!=(const CBuildin& p) const	{ return this!=&p; }
	///////////////////////////////////////////////////////////////////////////
	// accesses
	size_t Param()		{ return cParaCnt; }
	const char* Name()	{ return cID; }

	///////////////////////////////////////////////////////////////////////////
	// function call entrance
	int entrypoint()
	{
		// get/check arguments

		int ret = function();

		// return to script interpreter

		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// user function which is called on activation
	// overload with your function
	virtual int function()	{ return 0; }
};






class CFunction : public CProgramm
{
	// <Func Decl>  ::= <Scalar> Id '(' <Params>  ')' <Block>	// fixed parameter count
    //                | 'function' 'script' <Name Id> <Block>	// unknown parameter
	string<>	cID;
	size_t		cParaCnt;
public:
	///////////////////////////////////////////////////////////////////////////
	// construct destruct
	CFunction() : cID((const char*)NULL),cParaCnt(0)
	{}
	CFunction(const char* n, size_t p=0) : cID(n),cParaCnt(p)
	{}
	virtual ~CFunction()
	{}
	///////////////////////////////////////////////////////////////////////////
	// compares
	virtual bool operator==(const char *name) const	{ return 0==strcasecmp(cID,name); }
	virtual bool operator!=(const char *name) const	{ return 0!=strcasecmp(cID,name); }
	virtual bool operator==(const CFunction& p) const	{ return this==&p; }
	virtual bool operator!=(const CFunction& p) const	{ return this!=&p; }
	///////////////////////////////////////////////////////////////////////////
	// accesses
	size_t Param()		{ return cParaCnt; }
	const char* Name()	{ return cID; }
};



class CScript : public CProgramm
{
	//	<Script Decl> ::= '-' 'script' <Name Id> <Sprite Id> ',' <Block>
    //                  | <File Id> ',' DecLiteral ',' DecLiteral ',' DecLiteral 'script' <Name Id> <Sprite Id> ',' <TouchUp> <Block>
	string<> cID;
	string<> cName;
	string<> cMap;
	unsigned short cX;
	unsigned short cY;
	unsigned short cXs;
	unsigned short cYs;
	unsigned char cDir;
	unsigned short cSprite;
public:
	///////////////////////////////////////////////////////////////////////////
	// construct destruct
	CScript() : cID((const char*)NULL),cName((const char*)NULL),cMap((const char*)NULL),cX(0),cY(0),cXs(0),cYs(0),cDir(0),cSprite(0)
	{}
	CScript(const char* id, const char* name, const char* map, unsigned short x,unsigned short y, unsigned short xs,unsigned short ys, unsigned char dir, unsigned short sprite)
		: cID(id),cName(name),cMap(map),cX(x),cY(y),cXs(xs),cYs(ys),cDir(dir),cSprite(sprite)
	{}
	virtual ~CScript()
	{}
	///////////////////////////////////////////////////////////////////////////
	// compares
	virtual bool operator==(const char *name) const	{ return cID==name; }
	virtual bool operator!=(const char *name) const	{ return cID!=name; }
	virtual bool operator==(const CScript& p) const	{ return this==&p; }
	virtual bool operator!=(const CScript& p) const	{ return this!=&p; }

};


class CScriptEnvironment
{
	class CConstant : public string<>
	{	
	public:
		int		cValue;	// should be a variant

		CConstant(const char* n=NULL, int v=0) : string<>(n), cValue(v)	{}
		virtual ~CConstant()	{}
	};

	class CParameter : public string<>
	{	
	public:
		size_t		cID;

		CParameter(const char* n=NULL, size_t i=0) : string<>(n), cID(i)	{}
		virtual ~CParameter()	{}
	};


	TArrayDPT<CBuildin>		cBuildinTable;		// table of buildin functions
	TArrayDPT<CFunction>	cFunctionTable;		// table of script functions
	TArrayDPT<CScript>		cScriptTable;		// table of scripts

	// compile time only
	slist<CConstant>		cConstTable;		// table of constants
	slist<CParameter>		cParamTable;		// table of parameter keywords
	
	// actually of no real use
	slist< string<> >		cStringTable;		// table of strings

public:
	CScriptEnvironment()	{}
	~CScriptEnvironment()	{}

	const char* insert2Stringtable(const char*str)
	{
		size_t pos;
		cStringTable.insert( str );
		if( cStringTable.find(str,0,pos) )
		{	// look up the sting to get it's index
			return ((const char*)cStringTable[pos]) ? ((const char*)cStringTable[pos]) : "";
		}
		return NULL;
	}

	int getFunctionID(const char* name) const
	{
		for(size_t i=0; i<cFunctionTable.size(); ++i)
		{
			if( cFunctionTable[i] == name )
				return i;
		}
		return -1;
	}
	int getScriptID(const char* name)
	{
		for(size_t i=0; i<cScriptTable.size(); ++i)
		{
			if( cScriptTable[i] == name )
				return i;
		}
		return -1;
	}
	int addFunction(const char *name, size_t param)
	{
		size_t pos;
		for(pos=0; pos<cFunctionTable.size(); pos++)
		{
			if( cFunctionTable[pos] == name )
				return pos;
		}
		if( cFunctionTable.append( CFunction(name,param) ) )
			return pos;
		return -1;
	}
	int addScript(const char* id, const char* name, const char* map, unsigned short x,unsigned short y, unsigned short xs,unsigned short ys, unsigned char dir, unsigned short sprite)
	{
		size_t pos;
		for(pos=0; pos<cScriptTable.size(); pos++)
		{
			if( cScriptTable[pos] == name )
				return pos;
		}

		if( cScriptTable.append( CScript(id, name, map, x, y, xs,ys,dir, sprite) ) )
			return pos;
		return -1;
	}
	CFunction	&function(size_t pos)
	{	// will automatically throw on out-of-bound
		return cFunctionTable[pos];	
	}
	CScript		&script(size_t pos)
	{	// will automatically throw on out-of-bound
		return cScriptTable[pos];	
	}
};





///////////////////////////////////////////////////////////////////////////////
// compiler
///////////////////////////////////////////////////////////////////////////////
class CScriptCompiler : public CLogger
{
	///////////////////////////////////////////////////////////////////////////
	// class types

	///////////////////////////////////////////////////////
	// compile flags
	enum
	{
		CFLAG_NONE		= 0x00000000,	// no restrictions
		CFLAG_LVALUE	= 0x00000001,	// a variable is required
		CFLAG_GLOBAL	= 0x00000002,	// a global variable/value is required
		CFLAG_USE_BREAK	= 0x00000004,	// allow break + jump offset 
		CFLAG_USE_CONT	= 0x00000008	// allow continue + jump offset 
	};
	///////////////////////////////////////////////////////
	// variable types
	typedef enum
	{
		VAR_PARAM,		// function parameter
		VAR_TEMP,		// temp variable
		VAR_GACCOUNT,	// global account variable
		VAR_GCHAR,		// global character variable
		VAR_GGUILD,		// global guild variable
		VAR_GPARTY,		// global party variable
		VAR_GSERVER		// global server variable
	} vartype;
	///////////////////////////////////////////////////////
	// simple temporary list element
	class _stmlist
	{
	public:
		const parsenode*	value;
		const parsenode*	stm;
		size_t			gotomarker;

		_stmlist(const parsenode *v=NULL, const parsenode *s=NULL)
			: value(v), stm(s), gotomarker(0)
		{}
		_stmlist(const parsenode &v, const parsenode &s)
			: value(&v), stm(&s), gotomarker(0)
		{}
		_stmlist(const parsenode &s)
			: value(NULL), stm(&s), gotomarker(0)
		{}
		~_stmlist()	{}

		bool operator==(const _stmlist &v) const { return this==&v; }
		bool operator< (const _stmlist &v) const { return this< &v; }
	};

	///////////////////////////////////////////////////////
	// variable name storage
	class CVariable : public string<>
	{	
	public:
		size_t		id;
		vartype		type;
		size_t		use;

		CVariable(const char* n=NULL, size_t id=0xFFFFFFFF, vartype t=VAR_TEMP)
			: string<>(n), id(id), type(t), use(0)
		{}
		virtual ~CVariable()	{}
	};


	///////////////////////////////////////////////////////////////////////////
	// class data
	slist<CVariable>		cTempVar;		// variable list
	slist<CVariable>		cParaVar;		// parameter list
	CScriptEnvironment		&cEnv;			// the current script environment



	///////////////////////////////////////////////////////////////////////////
	// variable functions
	int insertVariable(const char* name, vartype t)
	{
		size_t inx;
		size_t id = cTempVar.size();
		cTempVar.insert( CVariable(name,id,t) );
		if( cTempVar.find(name, 0, inx) )
			return inx;
		return -1;
	}
	bool isVariable(const char* name, size_t &inx)
	{
		return cTempVar.find(name,0, inx);
	}
	int insertParameter(const char* name, vartype t)
	{
		size_t inx;
		size_t id = cTempVar.size();
		cParaVar.insert( CVariable(name,id,t) );
		if( cParaVar.find(name, 0, inx) )
			return inx;
		return -1;
	}
	bool isParameter(const char* name, size_t &inx)
	{
		return cParaVar.find(name,0, inx);
	}

	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
public:
	CScriptCompiler(CScriptEnvironment &e, int log=2): CLogger(log), cEnv(e) {}
	~CScriptCompiler()	{}

private:

	///////////////////////////////////////////////////////////////////////////
	// terminal checking functions
	bool CheckTerminal(const parsenode &node, int idx)
	{
		return ( node.Type()==1 && node.Symbol()==idx );
	}
	bool CheckNonTerminal(const parsenode &node, int idx)
	{
		return ( node.Type()==0 && node.Symbol()==idx );
	}
	///////////////////////////////////////////////////////////////////////////
	// debug functions
	void PrintTerminal(const parsenode &node)
	{
		const char *lp = (node.Type()==1)?node.Lexeme():node.SymbolName();
		this->logging("%s", (lp)?lp:"");
	}
	void PrintChildTerminals(const parsenode &node, bool line_number=false)
	{
		const char *lp;
		size_t i;
		for(i=0; i<node.count(); ++i)
		{
			lp = (node[i].Type()==1)?node[i].Lexeme():node[i].SymbolName();
			this->logging("%s ", (lp)?lp:"");
		}
		if(line_number)
			this->logging("(line %i, column %i)", node.Line(), node.Column());
	}
	//////////////////////////////////////////////////////////////////////////
	// main compile loop, is called recursively with all parse tree nodes
	bool CompileMain(const parsenode &node, size_t level, unsigned long flags, CProgramm& prog, int& userval)
	{
		size_t i;
		bool accept = false;
		///////////////////////////////////////////////////////////////////////
		
		if( node.Type()==1 )
		{	// terminals
			
			switch(node.Symbol())
			{
			case PT_LBRACE:
				this->logging("PT_LBRACE - ");
				this->logging("increment skope counter\n");
				accept = true;
				break;
			case PT_RBRACE:
				this->logging("PT_RBRACE - ");
				this->logging("decrement skope counter, clear local variables\n");
				accept = true;
				break;
			case PT_SEMI:
				this->logging("PT_SEMI - ");
				this->logging("clear stack\n");
				prog.appendCommand(OP_POP);
				accept = true;
				break;

			case PT_HEXLITERAL:
				this->logging("PT_HEXLITERAL - ");
				this->logging("%s - %s ", node.SymbolName(), node.Lexeme());
				if( 0 == (flags&CFLAG_LVALUE) )
				{

					prog.appendVarCommand( OP_PUSH_INT1, axtoi(node.Lexeme()) );
					this->logging("accepted\n");
					accept = true;
				}	
				else
				{
					this->logging("left hand assignment, not accepted\n");
				}	

				break;

			case PT_DECLITERAL:
				this->logging("PT_DECLITERAL - ");
				this->logging("%s - %s ", node.SymbolName(), node.Lexeme());
				if( 0 == (flags&CFLAG_LVALUE) )
				{
					prog.appendVarCommand( OP_PUSH_INT1, atoi(node.Lexeme()) );
					this->logging("accepted\n");
					accept = true;
				}	
				else
				{
					this->logging("left hand assignment, not accepted\n");
				}	

				break;

			case PT_CHARLITERAL:
				this->logging("PT_CHARLITERAL - ");
				this->logging("%s - %s ", node.SymbolName(), node.Lexeme());
				if( 0 == (flags&CFLAG_LVALUE) )
				{
					prog.appendVarCommand( OP_PUSH_INT1, (node.Lexeme())? node.Lexeme()[1]:0 );
					this->logging("accepted\n");
					accept = true;
				}	
				else
				{
					this->logging("left hand assignment, not accepted\n");
				}	

				break;

			case PT_FLOATLITERAL:
				this->logging("PT_FLOATLITERAL - ");
				this->logging("%s - %s ", node.SymbolName(), node.Lexeme());
				if( 0 == (flags&CFLAG_LVALUE) )
				{
					prog.appendCommand(OP_PUSH_FLOAT);
					prog.appendInt( CProgramm::float2int( (float)atof(node.Lexeme()) ) );

					this->logging("accepted\n");
					accept = true;
				}	
				else
				{
					this->logging("left hand assignment, not accepted\n");
				}	

				break;

			case PT_STRINGLITERAL:
				this->logging("PT_STRINGLITERAL - ");
				this->logging("%s - %s ", node.SymbolName(), node.Lexeme());
				if( 0 == (flags&CFLAG_LVALUE) )
				{
					// get the string without leading quotation mark
					const char *ip=NULL;
					if(node.LexemeObj().length()>=2)
					{
						char *str=(char*)(node.Lexeme()+1);
						size_t endpos = strlen(str)-1;
						str[endpos]=0; // cut off the trailing quotation mark
						ip = cEnv.insert2Stringtable( str );
						str[endpos]='"'; // put back the trailing quotation mark
					}
					if( ip )
					{	
						prog.appendCommand(OP_PUSH_STRING);
						prog.appendString( ip );

						this->logging("accepted\n");
						accept = true;
					}
					else
					{
						this->logging("error at string literal '%s'\n", node.Lexeme() );
					}
				}	
				else
				{
					this->logging("left hand assignment, not accepted\n");
				}	
				break;

			case PT_MINUS:
				this->logging("PT_MINUS - ");
				if( 0 == (flags&CFLAG_LVALUE) )
				{
					prog.appendVarCommand( OP_PUSH_INT1, 0 );
					this->logging(" menu element accepted\n");
					accept = true;
				}	
				else
				{
					this->logging("left hand assignment, not accepted\n");
				}	
				break;
				

			case PT_ID:
				this->logging("PT_ID - ");
				if( node.Lexeme() )
				{
					size_t inx;
					////////////////////////////
					// check for labels (determined in pre-run)
					////////////////////////////
					if( prog.isLabel(node.Lexeme(), inx) )
					{	// a label

//						prog.appendCommand(VX_LABEL);
//						prog.appendAddr( inx );
//						accept = true;

						prog.appendCommand(OP_PUSH_ADDR);
						prog.appendLabel(node[1].Lexeme());
						accept = true;


						this->logging("Label accepted: %s - %s (%i)\n", node.SymbolName(), node.Lexeme(), inx);
					}
					////////////////////////////
					// else check for constant keyword
					////////////////////////////
					// else check for global parameter keyword
					////////////////////////////
					////////////////////////////
					// it is a variable
					////////////////////////////
					else 
					{	// a variable name can be 
						// function parameter, temp variable or global storage
						size_t inx;
						accept = isParameter(node.Lexeme(), inx);
						if( accept )
						{	// a function parameter
							prog.appendCommand(OP_PUSH_PARAM);
							prog.appendChar(inx);

							this->logging("Local Function Parameter accepted: %s - %s (%i)\n", node.SymbolName(), node.Lexeme(), inx);
						}
						else
						{	// a variable
							bool first = false;
							accept = isVariable(node.Lexeme(),inx);
							if( !accept )
							{	// first encounter on an undeclared variable
								// assume declaration as "auto temp"
								this->logging("first encounter of variable '%s', assuming type of \"auto temp\"\n", node.Lexeme());
								first = true;

								insertVariable( node.Lexeme(), VAR_TEMP);
								accept = isVariable(node.Lexeme(),inx);
							}

							if(accept)
							{
								if( 0!=(flags&CFLAG_GLOBAL) && cTempVar[inx].type == VAR_TEMP)
								{	// only a temp variable but have been asked for a global one

									this->logging("Global Variable required: %s - %s\n", node.SymbolName(), node.Lexeme());
									accept = false;
								}
							}

							if(accept)
							{
								cTempVar[inx].use++;
								if( cTempVar[inx].type == VAR_TEMP )
								{	// a local temp variable 
									if( first || 0 != (flags&CFLAG_LVALUE) )
									{
										prog.appendVarCommand( OP_PUSH_TEMPVAR1, cTempVar[inx].id );

										this->logging("Local Variable Name accepted: %s - %s (%i)\n", node.SymbolName(), node.Lexeme(), cTempVar[inx].id);
										if(first)
										{
											prog.appendCommand(OP_CLEAR);
											this->logging("initialisation of local variable\n");
										}
									}
									else
									{
										prog.appendVarCommand( OP_PUSH_TEMPVALUE1, cTempVar[inx].id );
										this->logging("Local Variable Value accepted: %s - %s (%i)\n", node.SymbolName(), node.Lexeme(), cTempVar[inx].id);
									}
								}
								else
								{	// a global variable
									if( 0 != (flags&CFLAG_LVALUE) )
									{
										prog.appendCommand(OP_PUSH_VAR);
										prog.appendString( node.Lexeme() );
										this->logging("Global Variable Name accepted: %s - %s\n", node.SymbolName(), node.Lexeme());
									}
									else
									{
										prog.appendCommand(OP_PUSH_VALUE);
										prog.appendString( node.Lexeme() );
										this->logging("Global Variable Value accepted: %s - %s\n", node.SymbolName(), node.Lexeme());
									}
								}
							}
						}
						if(!accept)
						{
							this->logging("Error in variable statement: %s - %s\n", node.SymbolName(), node.Lexeme());
						}
					}

				}
				break;

			default:
				
				this->logging("Term  - ");
				this->logging("%s - %s\n", node.SymbolName(), node.Lexeme());

				// accept any terminal for debug only
				//accept = true;

				break;
			}
		}
		///////////////////////////////////////////////////////////////////////
		else
		{	// nonterminals

			if( node.count()==1 )
			{	// only one child, just go down
				accept = CompileMain(node[0], level+1, flags, prog, userval);
			}
			else
			// process the childs depending on this node
			switch(node.Symbol())
			{
			///////////////////////////////////////////////////////////////////
			// a variable declaration
			///////////////////////////////////////////////////////////////////
			case PT_VARDECL:
			{
				// <Var Decl> ::= <Type> <Var List>  ';'
				// <Type>     ::= <Mod> <Scalar>
				//              | <Mod> 
				//              |       <Scalar>
				// <Mod>      ::= global
				//              | temp
				// <Scalar>   ::= string
				//              | double
				//              | int
				//              | auto

				// compile the type
				vartype type = VAR_TEMP;
				int num=0;	// variable storage type, ignored for now
				// find all subterminals in first subtree and compile the variable list
				if( !CompileVarType(node[0], type, num) ||
					!CompileVarList(node[1], level+1, flags, prog, userval, type) )
					break;
				
				// new compile the variable list
				int varcount; // not necessary though but maybe nice to have
				if( node[1].Symbol() == PT_VARLIST || node[1].Symbol() == PT_VAR )
					accept = CompileMain(node[1], level+1, flags, prog, varcount);
				else
				{	// <Type> Id ;
					size_t inx;
					accept = isVariable(node[1].Lexeme(),inx);
					if( accept && cTempVar[inx].type==VAR_TEMP )
					{						
						this->logging("Local Variable Name accepted: %s - %s (%i)\n", node[1].SymbolName(), node[1].Lexeme(), cTempVar[inx].id);
						prog.appendVarCommand( OP_PUSH_TEMPVAR1, cTempVar[inx].id );
						this->logging("clear variable\n");
						prog.appendCommand(OP_CLEAR);
					}
				}
				this->logging("clear stack\n");
				prog.appendCommand(OP_POP);
				break;
			}
			case PT_VAR:
			{	//	<Var>      ::= Id <Array>
				//				 | Id <Array> '=' <MultiList>
				//				 | Id 
				//				 | Id         '=' <Op If>
				switch( node.count() )
				{
				case 1:	// Id
				{
					size_t inx;
					accept = isVariable(node[0].Lexeme(),inx);
					if(accept && cTempVar[inx].type==VAR_TEMP)
					{					
						this->logging("Local Variable Name accepted: %s - %s (%i)\n", node.SymbolName(), node.Lexeme(), cTempVar[inx].id);
						prog.appendVarCommand( OP_PUSH_TEMPVAR1, cTempVar[inx].id );
						this->logging("clear variable\n");
						prog.appendCommand(OP_CLEAR);
					}
					break;
				}
				case 3:	// Id '=' <Op If>
				{
					// put the target as variable, this should check for L-Values
					accept  = CompileMain(node[0], level+1, flags | CFLAG_LVALUE, prog, userval);

					// the result will be as single value (int, string or float) on stack
					accept &= CompileMain(node[2], level+1, flags & ~CFLAG_LVALUE, prog, userval);
					this->logging("PT_OPASSIGN\n");
					prog.appendCommand(OP_ASSIGN);
					break;
				}
				case 2:	// Id <Array>
				{
					// put the target as variable, this should check for L-Values
					accept  = CompileMain(node[0], level+1, flags | CFLAG_LVALUE, prog, userval);

					// compile the array resizer
					// put the size argument (inside node 1 is a "[ size ]" structure
					if( node[1].Symbol()==PT_ARRAY )
					{
						int dimension=0;
						// compile size arguments
						accept &= CompileMain(node[1], level+1, flags & ~CFLAG_LVALUE, prog, dimension);
						if(dimension<=255)
						{
							this->logging("PT_OPRESIZE\n");
							prog.appendCommand(OP_RESIZE);
							prog.appendChar(dimension);
						}
						else
						{
							this->logging("only 255 dimensions supported\n");
							accept=false;
						}
					}
					else
					{	// no array to init
						accept = false;
					}
					break;
				}
				case 4:	// Id <Array> '=' <MultiList>
				{	
					// put the target as variable, this should check for L-Values
					accept  = CompileMain(node[0], level+1, flags | CFLAG_LVALUE, prog, userval);

					if( node[1].Symbol()==PT_ARRAY )
					{
						// ignore the array resizer
						// since the dimension is given by the data
					}
					else
					{	// no array to init
						accept = false;
					}
					if(accept)
					{	/////////////////////////////////////////////
						// get number of initialisation elements
						accept &= CompileMain(node[3], level+1, flags & ~CFLAG_LVALUE, prog, userval);
						// assign the initial
						this->logging("PT_OPASSIGN - \n");
						prog.appendCommand(OP_ASSIGN);
					}
					break;
				}
				}//end switch
				break;
			}
			case PT_ARRAY:
			{	// '[' val ']' <Array>
				int elem;
				accept  = CompileMain(node[1], level+1, flags & ~CFLAG_LVALUE, prog, elem);
				userval++;
				if(node.count()==4)// next dimension
					accept &= CompileMain(node[3], level+1, flags & ~CFLAG_LVALUE, prog, userval);
				break;
			}
			case PT_MULTILIST:
			{	// '{' <InitList> '}'
				int dimension = 0;		
				accept  = CompileMain(node[1], level+1, flags & ~CFLAG_LVALUE, prog, dimension);
				if(dimension==0 && accept) dimension++; // single element (not a initlist)
				// vectorize the elements on the stack
				prog.appendVarCommand( OP_VECTORIZE1, dimension );
				break;
			}
			case PT_INITLIST:
			{	// ::= <Op If> ',' <InitList>
				// 	 | <MultiList> ',' <InitList>
				// 	 | <Op If>
				// 	 | <MultiList>
				accept = true;
				for(i=0; i<node.count(); ++i)
				{
					if(node[i].Symbol()==PT_INITLIST)
						accept &= CompileMain(node[i], level+1, flags & ~CFLAG_LVALUE, prog, userval);
					else if(node[i].Symbol()!=PT_COMMA)
					{
						userval++;
						accept &= CompileMain(node[i], level+1, flags & ~CFLAG_LVALUE, prog, userval);
					}
				}
				break;
			}


			///////////////////////////////////////////////////////////////////
			// a label
			case PT_LABELSTM:
			{	// expecting 2 terminals in here, the first is the labelname, the second a ":"
				this->logging("PT_LABELSTM - ");

//				size_t pos = prog.getCurrentPosition();
//				int inx = prog.useLabel( node[0].Lexeme(), pos );
//				if(inx>=0)
				if( prog.correctLabel( node[0].Lexeme() ) )
				{
					accept = true;
					this->logging("accepting label: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				else
				{
					this->logging("error in label statement: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// goto control statements
			case PT_GOTOSTMS:
			{	// <Goto Stms>  ::= goto Id ';'

//				prog.appendCommand(VX_GOTO);
//				int inx = prog.useLabel( node[1].Lexeme() );
//				if(inx>=0)
//				{
//					prog.appendAddr( inx );
//					accept = true;
//				}

				prog.appendCommand(OP_GOTO);
				prog.appendLabel(node[1].Lexeme());
				accept = true;
				break;
			}
			///////////////////////////////////////////////////////////////////
			// assignment operators
			case PT_OPASSIGN:
			{	// expecting 3 terminals in here, the first and third can be terminals or nonterminales
				// the second desides which assignment is to choose
				// check terminal count and operation type
				
				// put the target as variable, 
				// this should check for L-Values
				accept  = CompileMain(node[0], level+1, flags | CFLAG_LVALUE, prog, userval);

				// put the source
				// the result will be as single value (int, string or float) on stack
				accept &= CompileMain(node[2], level+1, flags & ~CFLAG_LVALUE, prog, userval);

				// push the command
				switch( node[1].Symbol() )
				{
				case PT_EQ:			prog.appendCommand(OP_ASSIGN);     break;
				case PT_PLUSEQ:		prog.appendCommand(OP_ASSIGN_ADD); break;
				case PT_MINUSEQ:	prog.appendCommand(OP_ASSIGN_SUB); break;
				case PT_TIMESEQ:	prog.appendCommand(OP_ASSIGN_MUL); break;
				case PT_DIVEQ:		prog.appendCommand(OP_ASSIGN_DIV); break;
				case PT_CARETEQ:	prog.appendCommand(OP_ASSIGN_XOR); break;
				case PT_AMPEQ:		prog.appendCommand(OP_ASSIGN_AND); break;
				case PT_PIPEEQ:		prog.appendCommand(OP_ASSIGN_OR);  break;
				case PT_GTGTEQ:		prog.appendCommand(OP_ASSIGN_RSH); break;
				case PT_LTLTEQ:		prog.appendCommand(OP_ASSIGN_LSH); break;
				}

				this->logging("PT_OPASSIGN - ");
				if(accept)
				{
					this->logging("accepting assignment %s: ", node[1].SymbolName());
					PrintChildTerminals(node);
					this->logging("\n");
					accept = true;
				}
				else
				{
					this->logging("error in assignment %s: ", node[1].SymbolName());
					PrintChildTerminals(node);
					this->logging("\n");
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// call statement eA style without brackets
			case PT_CALLSTM:
			{	// call contains at least one parameter
				// the first must be terminal PT_ID
				if( node.count() >0 && 
					CheckTerminal(node[0], PT_ID) )
				{	// first terminal is ok
					unsigned char command = OP_NOP;
					int id = -1;
					int paramcnt=0;

					this->logging("PT_CALLSTM - ");

					// go through childs now
					if( node.count() == 3 )
					{	// ID <something> ';'
						// process the <something>, need values
						accept = CompileMain(node[1], level+1,flags & ~CFLAG_LVALUE, prog, paramcnt); // need values

						// if not a call list as parameter it is a single parameter
						if( !CheckNonTerminal(node[1], PT_CALLLIST) )
							paramcnt++;

					}
					else if( node.count() == 2 )
					{	// ID ';'
						accept = true;
					}
					if(paramcnt<0 || paramcnt>255)
					{	
						this->logging("number of parameters invalid (%i) - ", paramcnt);
						accept=false;
					}
					if(accept)
					{
						id = cEnv.getFunctionID( node[0].Lexeme() );
						if( id>=0 )
						{	// found in function table
							command = OP_CALLBUILDIN1;
							if( (size_t)paramcnt < cEnv.function(id).Param() )
							{
								this->logging("number of parameters insufficient (%i given, %i needed) - ",paramcnt,cEnv.function(id).Param());
								accept = false;
							}
						}
						else
						{
							id = cEnv.getScriptID( node[0].Lexeme() );
							if( id>=0 )
							{
								command = OP_CALLSCRIPT1;
							}
							else
							{
								this->logging("Function does not exist - ");
								accept=false;
							}
						}
					}

					if(accept)
					{
						// push the command and function ID
						prog.appendVarCommand( command, id );
						prog.appendChar(paramcnt);
						this->logging("accepting call statement: ");
						PrintChildTerminals(node);
						this->logging(" - %i arguments", paramcnt);
						this->logging("\n");
					}
					else
					{
						this->logging("error in call statement: ");
						PrintChildTerminals(node);
						this->logging("\n");
					}

					// process the final ';' if exist
					if( CheckTerminal(node[node.count()-1], PT_SEMI) )
						accept &= CompileMain(node[node.count()-1], level+1,flags, prog, userval);
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// function calls
			case PT_RETVALUES:
			{
				if( node.count() >0 && 
					CheckTerminal(node[0], PT_ID) )
				{	// first terminal is ok
					unsigned char command = OP_NOP;
					int id = -1;
					int paramcnt=0;

					this->logging("PT_RETVALUES - ");

					if( node.count() == 4 )
					{	// ID '(' <Call Liste> ')'
						// process the <something>, need values
						accept = CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, paramcnt); // need values
						// if not a expression list as parameter it is a single parameter
						if( !CheckNonTerminal(node[2], PT_EXPRLIST) )
							paramcnt++;
					}
					if(paramcnt<0 || paramcnt>255)
					{	
						this->logging("number of parameters not allowed (%i)\n", paramcnt);
						accept=false;
					}

					if(accept)
					{
						id = cEnv.getFunctionID( node[0].Lexeme() );
						if( id>=0 )
						{	// found in function table
							command = OP_CALLBUILDIN1;
							if( (size_t)paramcnt < cEnv.function(id).Param() )
							{
								this->logging("number of parameters insufficient (%i given, %i needed) - ",paramcnt,cEnv.function(id).Param());
								accept = false;
							}
						}
						else
						{
							id = cEnv.getScriptID( node[0].Lexeme() );
							if( id>=0 )
							{
								command = OP_CALLSCRIPT1;
							}
							else
							{
								accept=false;
							}
						}
					}
					if(accept)
					{
						// push the command and function ID
						prog.appendVarCommand( command, id );
						prog.appendChar(paramcnt);

						this->logging("accepting call statement: ");
						PrintChildTerminals(node);
						this->logging(" - %i arguments", paramcnt);
						this->logging("\n");
					}
					else
					{
						this->logging("error in call statement: ");
						PrintChildTerminals(node);
						if( OP_NOP==command )
							this->logging(" - Function does not exist");
						this->logging("\n");
					}
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// comma seperated operands
			case PT_CALLLIST:
			case PT_EXPRLIST:
			case PT_VARLIST:
			{	// go through childs, spare the comma
				accept = true;
				for(i=0; i<node.count() && accept; ++i)
				{
					if( !CheckTerminal(node[i], PT_COMMA) )
					{
						accept = CompileMain(node[i], level+1,flags, prog, userval);

						if( !CheckNonTerminal(node[i], PT_CALLLIST) &&
							!CheckNonTerminal(node[i], PT_EXPRLIST) &&
							!CheckNonTerminal(node[i], PT_VARLIST) )
							userval++;
					}
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// return;
			case PT_RETURNSTMS:
			{	// <Return Stms>::= return <ArgC> ';'
				//                | end ';'

				if( CheckTerminal(node[0], PT_RETURN) )
				{
					accept = CompileMain(node[1], level+1,flags, prog, userval);
					prog.appendCommand(OP_RETURN);
				}
				else
				{
					accept = true;
					prog.appendCommand(OP_END);
				}

				if(accept)
				{
					this->logging("accept return statement: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				else
				{
					this->logging("error in return statement: ");
					PrintChildTerminals(node);
					this->logging("\n");			
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// call statement
			case PT_NORMALSTM:
			{	// can be:
				// if '(' <Expr> ')' <Normal Stm>
				// if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
				// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
				// while '(' <Expr> ')' <Normal Stm>
				// do <Normal Stm> while '(' <Expr> ')' ';'
				// switch '(' <Expr> ')' '{' <Case Stms> '}'
				// <ExprList> ';'
				// ';'              !Null statement
				if( node.count() <=2  )
				{	// <ExprList> ';' or ';', just call the childs
					accept = true;
					for(i=0; i<node.count(); ++i)
					{
						if( !CheckTerminal(node[i], PT_COMMA) )
						{
							accept = CompileMain(node[i], level+1,flags, prog, userval);
							if( !accept ) break;
						}
					}
				}
				else if( node.count() ==5 && CheckTerminal(node[0], PT_IF) )
				{	// if '(' <Expr> ')' <Normal Stm>
					// put in <Expr>
					accept = CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval); // need the result on the stack
					this->logging("Conditional Jump False -> Label1\n");
					prog.appendCommand(OP_NIF);
					size_t inspos1 = prog.appendAddr(0);	// placeholder
					// put in <Normal Stm>
					accept &= CompileMain(node[4], level+1,flags, prog, userval);
					this->logging("Label1\n");
					// calculate and insert the correct jump 
					prog.replaceAddr( prog.getCurrentPosition() ,inspos1);
				}
				else if( node.count() ==7 && CheckTerminal(node[0], PT_IF) )
				{	// if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
					// put in <Expr>
					accept = CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval); // need the result on the stack
					this->logging("Conditional Jump False -> Label1\n");
					prog.appendCommand(OP_NIF);
					size_t inspos1 = prog.appendAddr(0);	// placeholder
					// put in <Normal Stm>
					accept &= CompileMain(node[4], level+1,flags, prog, userval);
					this->logging("Goto -> Label2\n");
					prog.appendCommand(OP_GOTO);
					size_t inspos2 = prog.appendAddr(0);	// placeholder
					this->logging("Label1\n");
					// calculate and insert the correct jump 
					prog.replaceAddr( prog.getCurrentPosition() ,inspos1);
					// put in <Normal Stm2>
					accept &= CompileMain(node[6], level+1,flags, prog, userval);
					this->logging("Label2\n");
					prog.replaceAddr( prog.getCurrentPosition(), inspos2);			
				}
				else if( node.count() ==9 && CheckTerminal(node[0], PT_FOR) )
				{	// for '(' <Arg1> ';' <Arg2> ';' <Arg3> ')' <Normal Stm>
					// execute <Arg1>
					accept  = CompileMain(node[2], level+1,flags, prog, userval);
					// execute ";" to clear the stack;
					accept &= CompileMain(node[3], level+1,flags, prog, userval);
					this->logging("Label1\n");
					size_t tarpos1 = prog.getCurrentPosition();// position marker
					// execute <Arg2>, need a value
					accept &= CompileMain(node[4], level+1,flags & ~CFLAG_LVALUE, prog, userval);

					this->logging("Conditional Jump False -> Label2\n");
					prog.appendCommand(OP_NIF);
					size_t inspos2 = prog.appendAddr(0);	// placeholder

					// execute the loop body
					size_t rstart = prog.getCurrentPosition();
					accept &= CompileMain(node[8], level+1, flags | CFLAG_USE_BREAK | CFLAG_USE_CONT, prog, userval);
					size_t rend   = prog.getCurrentPosition();

					size_t tarpos3 = prog.getCurrentPosition();// position marker
					// execute the incrementor <Arg3>
					accept &= CompileMain(node[6], level+1,flags, prog, userval);
					// execute ";" to clear the stack;
					accept &= CompileMain(node[5], level+1,flags, prog, userval);

					this->logging("Goto -> Label1\n");
					prog.appendCommand(OP_GOTO);
					prog.appendAddr(tarpos1);

					this->logging("Label2\n");
					size_t tarpos2 = prog.getCurrentPosition();
					prog.replaceAddr( tarpos2 ,inspos2);

					// convert break -> goto Label2
					// convert continue -> goto Label1
					prog.replaceJumps(rstart,rend,VX_BREAK,tarpos2);
					prog.replaceJumps(rstart,rend,VX_CONT,tarpos3);
				}
				else if( node.count() ==5 && CheckTerminal(node[0], PT_WHILE) )
				{	// while '(' <Expr> ')' <Normal Stm>

					this->logging("Label1\n");
					size_t tarpos1 = prog.getCurrentPosition();// position marker

					// execute <Expr>
					accept  = CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

					this->logging("Conditional Jump False -> Label2\n");
					prog.appendCommand(OP_NIF);
					size_t inspos2 = prog.appendAddr(0);	// placeholder offset

					// execute <Normal Stm>
					size_t rstart = prog.getCurrentPosition();
					accept &= CompileMain(node[4], level+1,flags | CFLAG_USE_BREAK | CFLAG_USE_CONT, prog, userval);
					size_t rend = prog.getCurrentPosition();

					this->logging("Goto -> Label1\n");
					prog.appendCommand(OP_GOTO);
					prog.appendAddr(tarpos1);

					this->logging("Label2\n");
					size_t tarpos2 = prog.getCurrentPosition();
					prog.replaceAddr( tarpos2 ,inspos2);
					// convert break -> goto Label2
					// convert continue -> goto Label1
					prog.replaceJumps(rstart,rend,VX_BREAK,tarpos2);
					prog.replaceJumps(rstart,rend,VX_CONT,tarpos1);

				}
				else if( node.count() ==7 && CheckTerminal(node[0], PT_DO) )
				{	// do <Normal Stm> while '(' <Expr> ')' ';'

					this->logging("Label1\n");
					size_t tarpos1 = prog.getCurrentPosition();// position marker

					// execute <Normal Stm>
					size_t rstart = prog.getCurrentPosition();
					accept  = CompileMain(node[1], level+1,flags | CFLAG_USE_BREAK | CFLAG_USE_CONT, prog, userval);
					size_t rend = prog.getCurrentPosition();

					// execute <Expr>
					accept &= CompileMain(node[4], level+1,flags & ~CFLAG_LVALUE, prog, userval);
					this->logging("Conditional Jump True -> Label1\n");
					prog.appendCommand(OP_IF);
					prog.appendAddr(tarpos1);

					size_t tarpos2 = prog.getCurrentPosition();
					accept &= CompileMain(node[6], level+1,flags & ~CFLAG_LVALUE, prog, userval);

					// convert break -> goto Label2
					// convert continue -> goto Label1
					prog.replaceJumps(rstart,rend,VX_BREAK,tarpos2);
					prog.replaceJumps(rstart,rend,VX_CONT,tarpos1);

				}
				else if( node.count() ==7 && CheckTerminal(node[0], PT_SWITCH) )
				{	// switch '(' <Expr> ')' '{' <Case Stms> '}'
					// <Case Stms> ::= case <Value> ':' <Stm List> <Case Stms>
					//               | default ':' <Stm List> <Case Stms>

					char varname[128];
					size_t inx;
					snprintf(varname, 128,"_#casetmp%04li", (unsigned long)prog.getCurrentPosition());
					insertVariable( varname, VAR_TEMP);
					accept = isVariable(varname,inx);
					if( accept )
					{
						inx = cTempVar[inx].id;

						this->logging("create temporary variable %s (%i)\n", varname, inx);

						prog.appendVarCommand( OP_PUSH_TEMPVAR1, inx );
						this->logging("push temporary variable %s (%i)\n", varname, inx);

						// compile <Expr>
						accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);
						prog.appendCommand(OP_ASSIGN);
						this->logging("OP_ASSIGN\n");
						// save the result from <Expr> into the temp variable
					}

					// process the case statements
					vector<_stmlist> stmlist;
					int hasdefault=-1;

					if( accept )
					{	// use a if-else-if struture to process the case; 
						// but first reorder the tree to a list
						const parsenode *casestm = &(node[5]);
						while(casestm && casestm->Symbol()==PT_CASESTMS)
						{
							if( casestm->count()==5 )
							{	// case <Value> ':' <Stm List> <Case Stms>
								stmlist.append( _stmlist(casestm->operator[](1), casestm->operator[](3)) );
								casestm = &(casestm->operator[](4));
							}
							else if( casestm->count()==4 )
							{	//default ':' <Stm List> <Case Stms>

								if(hasdefault>=0)
								{
									this->logging("error: multiple case marker specified\n");
									accept = false;
									break;
								}
								hasdefault=stmlist.size();

								stmlist.append( _stmlist(casestm->operator[](2)) );
								casestm = &(casestm->operator[](3));
							}
							else
								casestm = NULL;					
						}
					}

					size_t defpos=0;
					if( accept )
					{	// go through the list and build the if-else-if

						for(i=0; i<stmlist.size()&& accept; ++i)
						{
							if( stmlist[i].value )
							{	// normal case statements, not the default case

								// push the temprary variable
								prog.appendVarCommand( OP_PUSH_TEMPVALUE1, inx );
								this->logging("push temporary variable %s (%i)\n", varname, inx);

								// push the case value
								accept = CompileMain(*stmlist[i].value, level+1,flags, prog, userval);

								this->logging("PT_EQUAL\n");
								prog.appendCommand(OP_EQUATE);
				
								this->logging("Conditional Jump True -> CaseLabel%i\n", i);
								prog.appendCommand(OP_IF);
								stmlist[i].gotomarker = prog.appendAddr(0);	// placeholder
							}
						}
						this->logging("Goto -> LabelEnd/Default\n");
						prog.appendCommand(OP_GOTO);
						defpos = prog.appendAddr(0);	// placeholder
						if(hasdefault>=0)
							stmlist[hasdefault].gotomarker = defpos;
					}
					if(accept)
					{	// build the body of the case statements
						size_t address;
						size_t rstart = prog.getCurrentPosition();
						for(i=0; i<stmlist.size()&& accept; ++i)
						{
							if( stmlist[i].stm )
							{	
								// fill in the target address
								address = prog.getCurrentPosition();
								if( stmlist[i].gotomarker>0 )
									prog.replaceAddr(address, stmlist[i].gotomarker);

								// compile the statement
								accept = CompileMain(*stmlist[i].stm, level+1,flags | CFLAG_USE_BREAK, prog, userval);
							}
						}
						size_t rend = prog.getCurrentPosition();

						if(hasdefault<0)
						{	// no default case, so redirect the last goto to the end
							prog.replaceAddr(rend, defpos);
						}
						// convert break -> goto REND
						prog.replaceJumps(rstart,rend,VX_BREAK, rend );
					}
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// break/continue control statements
			case PT_LCTRSTMS:
			{	// 'break' ';'
				// 'continue' ';'
				if( (node.count()>0 && node[0].Symbol() == PT_CONTINUE) && (0!=(flags & CFLAG_USE_CONT)) )
				{
					prog.appendCommand(VX_CONT);
					prog.appendAddr( 0 );
					accept = true;
				}
				else if( (node.count()>0 && node[0].Symbol() == PT_BREAK) && (0!=(flags & CFLAG_USE_BREAK)) )
				{
					prog.appendCommand(VX_BREAK);
					prog.appendAddr( 0 );
					accept = true;
				}
				else if( (node.count()>0 && node[0].Symbol() == PT_BREAK) )
				{	// accept it as call to end
					prog.appendCommand(OP_RETURN);
					accept = true;
				}
				if(!accept)
					this->logging("keyword '%s' not allowed in this scope\n", node[0].Lexeme());
				break;
			}

			///////////////////////////////////////////////////////////////////
			// operands
			case PT_OPADDSUB:
			{	// <Op AddSub> '+' <Op MultDiv>
				// <Op AddSub> '-' <Op MultDiv>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				switch( node[1].Symbol() )
				{
				case PT_PLUS:
					this->logging("PT_PLUS\n");
					prog.appendCommand(OP_ADD);
					break;
				case PT_MINUS:
					this->logging("PT_MINUS\n");
					prog.appendCommand(OP_SUB);
					break;
				}// end switch
				break;
			}
			case PT_OPMULTDIV:
			{	// <Op MultDiv> '*' <Op Unary>
				// <Op MultDiv> '/' <Op Unary>
				// <Op MultDiv> '%' <Op Unary>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				switch( node[1].Symbol() )
				{
				case PT_TIMES:
					this->logging("PT_TIMES\n");
					prog.appendCommand(OP_MUL);
					break;
				case PT_DIV:
					this->logging("PT_DIV\n");
					prog.appendCommand(OP_DIV);
					break;
				case PT_PERCENT:
					this->logging("PT_PERCENT\n");
					prog.appendCommand(OP_MOD);
					break;
				}// end switch
				break;
			}
			case PT_OPAND:
			{	// <Op And> '&&' <Op BinOR>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				if( node[1].Symbol() == PT_AMPAMP )
				{
					this->logging("PT_OPAND\n");
					prog.appendCommand(OP_LOG_AND);
				}
				break;
			}
			case PT_OPOR:
			{	// <Op Or> '||' <Op And>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				if( node[1].Symbol() == PT_PIPEPIPE )
				{
					this->logging("PT_PIPEPIPE\n");
					prog.appendCommand(OP_LOG_OR);
				}
				break;
			}

			case PT_OPBINAND:
			{	// <Op BinAND> '&' <Op Equate>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				if( node[1].Symbol() == PT_AMP )
				{
					this->logging("PT_OPBINAND\n");
					prog.appendCommand(OP_BIN_AND);
				}
				break;
			}
			case PT_OPBINOR:
			{	// <Op BinOr> '|' <Op BinXOR>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				if( node[1].Symbol() == PT_PIPE )
				{
					this->logging("PT_OPBINOR\n");
					prog.appendCommand(OP_BIN_OR);
				}
				break;
			}
			case PT_OPBINXOR:
			{	// <Op BinXOR> '^' <Op BinAND>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				if( node[1].Symbol() == PT_CARET )
				{
					this->logging("PT_OPBINOR\n");
					prog.appendCommand(OP_BIN_XOR);
				}
				break;
			}
			case PT_OPCAST:
			{	// '(' <Scalar> ')' <Op Unary>

				// put the operands on stack, first the value then the target type
				accept  = CompileMain(node[3], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				if( PT_AUTO != node[1].Symbol() )
				{	// don't need to cast to auto type
					if( PT_INT    == node[1].Symbol() ||
						PT_STRING == node[1].Symbol() ||
						PT_DOUBLE == node[1].Symbol() )
					{						
						prog.appendCommand(OP_CAST);
						prog.appendChar( node[1].Symbol() );
						this->logging("PT_OPCAST\n");
					}
					else
					{
						accept = false;
						this->logging("wrong cast target\n");
					}
				}
				break;
			}
			case PT_OPCOMPARE:
			{	// <Op Compare> '>'  <Op Shift>
				// <Op Compare> '>=' <Op Shift>
				// <Op Compare> '<'  <Op Shift>
				// <Op Compare> '<=' <Op Shift>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				switch( node[1].Symbol() )
				{
				case PT_GT:
					this->logging("PT_GT\n");
					prog.appendCommand(OP_ISGT);
					break;
				case PT_GTEQ:
					this->logging("PT_GTEQ\n");
					prog.appendCommand(OP_ISGTEQ);
					break;
				case PT_LT:
					this->logging("PT_LT\n");
					prog.appendCommand(OP_ISLT);
					break;
				case PT_LTEQ:
					this->logging("PT_LTEQ\n");
					prog.appendCommand(OP_ISLTEQ);
					break;
				}// end switch
				break;
			}
			
			case PT_OPEQUATE:
			{	// <Op Equate> '==' <Op Compare>
				// <Op Equate> '!=' <Op Compare>

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				switch( node[1].Symbol() )
				{
				case PT_EQEQ:
					this->logging("PT_EQUAL\n");
					prog.appendCommand(OP_EQUATE);
					break;
				case PT_EXCLAMEQ:
					this->logging("PT_UNEQUAL\n");
					prog.appendCommand(OP_UNEQUATE);
					break;
				}// end switch
				break;
			}
			case PT_OPIF:
			{	// <Op Or> '?' <Op If> ':' <Op If>
				// => if( or ) opif1 else opif2
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				this->logging("Conditional Jump False -> Label1\n");
				prog.appendCommand(OP_NIF);
				size_t inspos1 = prog.appendAddr(0);	// placeholder
				// put in <Op If1>
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				this->logging("Goto -> Label2\n");
				prog.appendCommand(OP_GOTO);
				size_t inspos2 = prog.appendAddr(0);	// placeholder
				this->logging("Label1\n");
				// calculate and insert the correct jump offset
				prog.replaceAddr( prog.getCurrentPosition() ,inspos1);
				// put in <Op If2>
				accept &= CompileMain(node[4], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				this->logging("Label2\n");
				prog.replaceAddr( prog.getCurrentPosition() ,inspos2);

				this->logging("PT_SELECT\n");
				break;
			}
			case PT_OPPOINTER:
			{	// <Op Pointer> '.' Id             ! member variable
				// <Op Pointer> '.' <RetValues>    ! member function
				// <Op Pointer> '[' <Expr> ']'     ! array

				switch( node[1].Symbol() )
				{
				case PT_DOT:
					// put the operands on stack
					accept  = CompileMain(node[0], level+1,flags | CFLAG_LVALUE|CFLAG_GLOBAL, prog, userval);	// base variable
					
					if( CheckTerminal(node[2], PT_ID) )
					{	// have to select the correct variable inside base according to the member name
						// and put this variable on the stack

						// need to check if member variable is valid
						accept &= CompileMain(node[2], level+1,flags | CFLAG_LVALUE|CFLAG_GLOBAL, prog, userval);	// member variable

						prog.appendCommand(OP_MEMBER);
						this->logging("PT_MEMBERACCESS variable %s\n", accept?"ok":"failed");
					}
					else if( CheckNonTerminal(node[2], PT_RETVALUES) )
					{
						accept &= CompileMain(node[2], level+1,flags, prog, userval);	// member function
						this->logging("PT_MEMBERACCESS function %s\n", accept?"ok":"failed");
					}
					else
					{
						this->logging("PT_MEMBERACCESS failed\n");
					}
					
					break;
				case PT_LBRACKET:
					// put the operands on stack
					accept  = CompileMain(node[0], level+1,flags | CFLAG_LVALUE, prog, userval);	// variable
					accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);	// index
					prog.appendCommand(OP_ARRAY);
					this->logging("PT_ARRAYACCESS %s %s\n", (flags)?"variable":"value", (accept)?"successful":"failed");
					break;
				}// end switch
				break;
			}
			case PT_OPPOST:
			{	// <Op Pointer> '++'
				// <Op Pointer> '--'

				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags | CFLAG_LVALUE, prog, userval); // put the variable itself on stack

				switch( node[1].Symbol() )
				{
				case PT_PLUSPLUS:
					this->logging("PT_OPPOST_PLUSPLUS, \n");
					prog.appendCommand(OP_POSTADD);
					break;
				case PT_MINUSMINUS:
					this->logging("PT_OPPOST_MINUSMINUS\n");
					prog.appendCommand(OP_POSTSUB);
					break;
				}// end switch

				break;
			}
			case PT_OPPRE:
			{	// '++'   <Op Unary>
				// '--'   <Op Unary>

				// put the operands on stack
				accept  = CompileMain(node[1], level+1,flags | CFLAG_LVALUE, prog, userval); // put the variable itself on stack

				switch( node[0].Symbol() )
				{
				case PT_PLUSPLUS:
					this->logging("PT_OPPRE_PLUSPLUS, \n");
					prog.appendCommand(OP_PREADD);
					break;
				case PT_MINUSMINUS:
					this->logging("PT_OPPRE_MINUSMINUS\n");
					prog.appendCommand(OP_PRESUB);
					break;
				}// end switch

				break;
			}
			case PT_OPSHIFT:
			{	// <Op Shift> '<<' <Op AddSub>
				// <Op Shift> '>>' <Op AddSub>
				// put the operands on stack
				accept  = CompileMain(node[0], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				accept &= CompileMain(node[2], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				switch( node[1].Symbol() )
				{
				case PT_GTGT:
					this->logging("PT_GTGT, \n");
					prog.appendCommand(OP_RSHIFT);
					break;
				case PT_LTLT:
					this->logging("PT_LTLT\n");
					prog.appendCommand(OP_LSHIFT);
					break;
				}// end switch

				break;
			}
			case PT_OPSIZEOF:
			{	// sizeof '(' <Type> ')'
				// sizeof '(' Id ')'

				// put the operands on stack
				switch( node[2].Symbol() )
				{
				case PT_ID:
					accept  = CompileMain(node[0], level+1,flags | CFLAG_LVALUE, prog, userval); // put the variable itself on stack
					prog.appendCommand(OP_SIZEOF);
					this->logging("PT_OPSIZEOF ID, \n");
					break;
				default:
					prog.appendVarCommand( OP_PUSH_INT1, 1 );
					this->logging("PT_OPSIZEOF TYPE, put the corrosponding value on stack\n");
					break;
				}// end switch

				break;
			}
			case PT_OPUNARY:
			{	// '!'    <Op Unary>
				// '~'    <Op Unary>
				//'-'    <Op Unary>
				//'+'    <Op Unary>

				accept  = CompileMain(node[1], level+1,flags & ~CFLAG_LVALUE, prog, userval);

				// put the operands on stack
				switch( node[0].Symbol() )
				{
				case PT_EXCLAM:
					this->logging("PT_OPUNARY_NOT\n");
					prog.appendCommand(OP_NOT);
					break;
				case PT_TILDE:
					this->logging("PT_OPUNARY_INVERT\n");
					prog.appendCommand(OP_INVERT);
					break;
				case PT_MINUS:
					this->logging("PT_OPUNARY_NEGATE\n");
					prog.appendCommand(OP_NEGATE);
					break;
				//case PT_PLUS:	// can just ignore
				//	break;		
				}// end switch
				break;
			}
			case PT_VALUE:
			{	// '(' <Expr> ')'
				accept  = CompileMain(node[1], level+1,flags & ~CFLAG_LVALUE, prog, userval);
				break;
			}

			///////////////////////////////////////////////////////////////////
			// statements with no own handler that just go through their childs
			case PT_STMLIST:
			case PT_BLOCK:
			{	// check if childs are accepted
				accept = true;
				for(i=0; i<node.count(); ++i)
				{
					accept = CompileMain(node[i], level+1,flags, prog, userval);
					if( !accept ) break;
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// default prints unhandled statements
			default:
			{
				this->logging("NTerm - ");
				this->logging("%s - %s\n", node.SymbolName(), node.Lexeme());

				// accept non-terminal but go through their childs
				accept = true;
				for(i=0; i<node.count(); ++i)
				{
					accept = CompileMain(node[i], level+1,flags, prog, userval);
					if( !accept ) break;
				}
				break;
			}
			}// switch
		}
		///////////////////////////////////////////////////////////////////////
		return accept;
	}
	///////////////////////////////////////////////////////////////////////////
	// compile variable type declaration
	bool CompileVarType(const parsenode &node, vartype &type, int &num)
	{
		size_t i;
		bool accept = false;
		if( node.Type()==1 )
		{	// terminals
			switch( node.Symbol() )
			{
			case PT_AUTO:
			case PT_DOUBLE:
			case PT_STRING:
			case PT_INT:
				num = node.Symbol();
				accept = true;
				break;
			case PT_TEMP:
				type = VAR_TEMP;
				accept = true;
				break;
			case PT_GLOBAL:
				type = VAR_GSERVER;
				accept = true;
				break;
			}
		}
		else
		{	// nonterminal
			accept=true;
			for(i=0; i<node.count() && accept; ++i)
				accept &= CompileVarType(node[i], type, num);
		}
		return accept;
	}

	///////////////////////////////////////////////////////////////////////////
	// compile the variable declaration
	bool CompileVarList(const parsenode &node, size_t level, unsigned long flags, CProgramm& prog, int& userval, vartype type)
	{	// <Var List> ::= <Var> ',' <Var List>
		//              | <Var>

		bool accept = false;
		if( node.Symbol()==PT_VARLIST )
		{	// <Var> , <VarList>
			// <Var> , <Var> , ...
			size_t i;
			accept = true;
			for(i=0; i<node.count(); ++i)
			{
				if( node[i].Symbol() == PT_VARLIST ||
					node[i].Symbol() == PT_VAR || 
					node[i].Symbol() == PT_ID )
				{
					accept &= CompileVarList(node[i], level, flags, prog, userval, type);
				}
			}
		}
		else if( node.Symbol() == PT_VAR )
		{	// <Var>  ::= Id 
			//			| Id '=' <Op If>
			// <Var>  ::= Id <Array>
			//			| Id <Array> '=' { <InitList> }
			// only do the id here
			accept = CompileVarList(node[0], level+1, flags, prog, userval, type);
		}
		else if( node.Symbol()==PT_ID )
		{
			size_t id;
			if( isVariable(node.Lexeme(), id) )
			{
				this->logging( "variable '%s'already defined\n", node.Lexeme());
			}
			else
			{
				insertVariable( node.Lexeme(), type);
				accept = isVariable(node.Lexeme(), id);
				if( accept )
					this->logging( "accepting variable '%s' of type %i\n", node.Lexeme(), type);
				else
					this->logging( "error when accepting variable '%s' of type %i\n", node.Lexeme(), type);
			}
		}
		return accept;
	}
	///////////////////////////////////////////////////////////////////////////
	// initial pre-run through the tree to find necessary elements
	// currently only labels are done here, 
	//!! check if merging with main compile loop would be suitable
	bool CompileLabels(const parsenode &node, size_t level, unsigned long flags, CProgramm& prog, int& userval)
	{
		size_t i;
		bool accept = false;
		///////////////////////////////////////////////////////////////////
		if( node.Type()==1 )
		{	// terminals
			accept = true;
		}
		///////////////////////////////////////////////////////////////////
		else
		{	// nonterminals

			if( node.count()==1 )
			{	// only one child, just go down
				accept = CompileLabels(node[0], level+1, flags, prog, userval);
			}
			else
			// check the childs
			switch(node.Symbol())
			{
			///////////////////////////////////////////////////////////////////
			// a label
			case PT_LABELSTM:
			{	// expecting 2 terminals in here, the first is the labelname, the second a ":"
				size_t inx;
				if( !prog.isLabel(node[0].Lexeme(), inx) )
				{
					if( prog.createLabel( node[0].Lexeme() ) )
					{
						accept = true;
						this->logging("accepting label: ");
						PrintChildTerminals(node);
						this->logging("\n");
					}
					else
					{
						this->logging("error processing in label statement: ");
						PrintChildTerminals(node);
						this->logging("\n");
					}
				}
				else
				{
					this->logging("error: label already defined: ");
					PrintChildTerminals(node, true);
					this->logging("\n");
				}
				break;
			}
			///////////////////////////////////////////////////////////////////
			// default prints unhandled statements
			default:
			{
				// accept non-terminal but go through their childs
				accept = true;
				for(i=0; i<node.count() && accept; ++i)
					accept = CompileLabels(node[i], level+1,flags, prog, userval);
				break;
			}
			}// switch
		}
		///////////////////////////////////////////////////////////////////////
		return accept;
	}
	///////////////////////////////////////////////////////////////////////////
	// builds name and id from a ExName node
	bool CompileScriptExNameID(const parsenode& nameid, char name[])
	{
		bool ret = false;
		if( nameid.Symbol()==PT_NAMEID)
		{	// inside a <exName Id>
			size_t i;
			for(i=0; i<nameid.count(); ++i)
			{
				if( nameid[i].Symbol()==PT_ID || nameid[i].Symbol()==PT_ID)
				{
					strcat(name, nameid[i].Lexeme() );
					strcat(name, " ");
					ret = true;
				}
				else if( nameid.count() > 1 && nameid[i].Symbol()==PT_NAMEID)
					ret &= CompileScriptExNameID(nameid[i], name);
				else
				{
					ret = false;
					break;
				}
			}
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// generate name and id from a NameId node
	bool CompileScriptNameID(const parsenode &node, char id[], char name[])
	{
		bool ret=false;
		name[0]=0;
		id[0]=0;

		if(node.Symbol()== PT_NAMEID)
		{
			const parsenode *nameid=&node;
			// go into the first
			if( nameid && nameid->count()==3 && nameid->Symbol()==PT_NAMEID )
			{
				if( nameid->operator[](2).Symbol()==PT_ID )
					strcpy(id,nameid->operator[](2).Lexeme());	// terminal ID

				if( nameid->operator[](0).Symbol()==PT_NAMEID)
					nameid = &(nameid->operator[](0));			// nonterminal <exName Id>
				else
					nameid=NULL;
			}
			else if(nameid->count()==1 && nameid->Symbol()==PT_NAMEID)
			{	// non-collapsed parse tree
				nameid = &(nameid->operator[](0));
			}
			
			CompileScriptExNameID(*nameid,name);

			if(strlen(name)>0)
				name[strlen(name)-1]=0;

			if( node.Symbol() != PT_NAMEID || node.count()!=3 )
			{	// identical name and id 
				strcpy(id,name);
			}
		}
		else if( node.Symbol()== PT_ID )
		{	// just a simple id
			strcpy(name, node.Lexeme() );
			strcpy(id, node.Lexeme() );
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// generate the sprite number from a SpriteId node
	unsigned short CompileScriptSpriteID(const parsenode &spriteid)
	{
		if( spriteid.count()==1 && spriteid.Symbol()==PT_SPRITEID)
			return atoi(spriteid[0].Lexeme() );
		else if( spriteid.Symbol()==PT_DECLITERAL)
			return atoi(spriteid.Lexeme());
		else
			return 0xFFFF;
			
	}
	///////////////////////////////////////////////////////////////////////////
	// generate a filename from a FileId node
	bool CompileScriptFileID(const parsenode &fileid, char map[])
	{
		if( fileid.count()==3 && fileid.Symbol()==PT_ID )
		{
			strcpy(map, fileid[0].Lexeme() );
			strcat(map, ".");
			strcat(map, fileid[2].Lexeme() );
			return true;
		}
		else
		{
			map[0]=0;
			return false;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// compile headers of ea script syntax
	bool CompileScriptHeader(const parsenode &node, char id[], char name[], char map[], unsigned short &x, unsigned short &y, unsigned short &xs, unsigned short &ys, unsigned char &dir, unsigned short &sprite)
	{	// '-' 'script' <Name Id> <Sprite Id> ',' <Block>
		// 'function' 'script' <Name Id> <Block>
		// <File Id> ',' DecLiteral ',' DecLiteral ',' DecLiteral 'script' <Name Id> <Sprite Id> ',' <Touchup> <Block>
		// 
		// <Name Id>   ::= <exName Id>
		//               | <exName Id> '::' Id
		// 
		// <exName Id> ::= Id
		//               | ApoId
		//               | Id <exName Id> 
		//               | ApoId <exName Id> 
		// <Sprite Id> ::= DecLiteral
		//               | '-' DecLiteral
		// 
		// <File Id>   ::= Id '.' Id

		bool accept = false;
		if( node.count()==6  && CheckNonTerminal(node[1], PT_SCRIPT) )
		{	// version 1
			map[0]=0;
			x=y=0xFFFF;
			dir=0;
			sprite = CompileScriptSpriteID(node[3]);
			accept = CompileScriptNameID( node[2], id, name);
		}
		else if( node.count()==4  && CheckNonTerminal(node[1], PT_SCRIPT) )
		{	// version 2
			map[0]=0;
			x=y=0xFFFF;
			dir=0;
			sprite = 0xFFFF;
			accept = CompileScriptNameID( node[2], id, name);
		}
		else if(node.count()==13 && CheckNonTerminal(node[7], PT_SCRIPT) )
		{	// version 3
			accept  = CompileScriptFileID( node[0], map);
			accept &= CompileScriptNameID( node[8], id, name);
			x		= atoi( node[2].Lexeme() );
			y		= atoi( node[4].Lexeme() );
			dir		= atoi( node[6].Lexeme() );
			sprite	= CompileScriptSpriteID(node[9]);
			if( node[10].count()==4 ) // Touchup == <decimal> ',' <decimal> ','
			{
				xs	= atoi( node[10][0].Lexeme() );
				ys	= atoi( node[10][1].Lexeme() );
			}
			else
				xs=ys=0;
		}
		return accept;
	}
	///////////////////////////////////////////////////////////////////////////
	// counting number of parameters in nonterminal DParams and Params
	size_t CompileParams(const parsenode &node, bool insert=false)
	{	
		size_t cnt = 0, i;

		if( CheckNonTerminal(node, PT_PARAMS) )
		{	// go through childs, spare the comma
			for(i=0; i<node.count(); ++i)
			{
				if( !CheckTerminal(node[i], PT_COMMA) )
				{
					cnt += CompileParams(node[i]);

					if( !CheckNonTerminal(node[i], PT_PARAMS) )
					{
						cnt++;

						if( CheckNonTerminal(node[i], PT_PARAM) )
						{	// register the function parameters like variables
							//	<Param>      ::= const <Scalar> Id
							//				   |       <Scalar> Id
							//				   | const          Id
							//				   |                Id
							insertParameter(node[i][node[i].count()-1].Lexeme(), VAR_PARAM);
						}
					}

				}
			}
		}
		return cnt;
	}

	///////////////////////////////////////////////////////////////////////////
	// compiler entry point
public:
	bool CompileTree(const parsenode &node)
	{
		size_t i;
		bool accept = false;
		///////////////////////////////////////////////////////////////////////
		
//		if( node.Type()==1 )
//		{	// terminals
//			
//			// generally not accepted here
//		}
//		///////////////////////////////////////////////////////////////////////
//		else
// some headers come in as terminals right now,
// will be changed, when grammar is fished
		{	// nonterminals
			switch(node.Symbol())
			{
			///////////////////////////////////////////////////////////////////
			// 
			case PT_BLOCK:
			{	// single <Block> without header (for small scripts)
				int userval;
				CProgramm prog;

				cTempVar.clear();

				prog.appendCommand(OP_START);
				size_t sizepos=prog.appendInt(0);
				if( CompileLabels(node, 0, 0, prog, userval) )
					accept = CompileMain(node, 0, 0, prog, userval);
				prog.appendCommand(OP_END);
				prog.replaceInt(prog.getCurrentPosition(),sizepos);
				prog.ConvertLabels();

				this->logging("\n");
				if( accept )
				{
					this->logging("accept block: ");
					PrintChildTerminals(node);
					this->logging("\n");

					prog.dump();
					this->logging("variables:\n");
					for(size_t i=0;i<cTempVar.size(); ++i)
					{
						this->logging("(%i) '%s' type=%i, id=%i, use=%i\n",
							i,(const char*)cTempVar[i],
							cTempVar[i].type,cTempVar[i].id,cTempVar[i].use);
					}
					this->logging("\n");
				}
				else
				{
					this->error("failed\n");
					this->logging("failed block: ");
					PrintChildTerminals(node);
					this->logging("\nuse detailed outputs for debugging\n");

				}
				break;

			}
			case PT_FUNC:
			{	// <Scalar> Id '(' <Paramse>  ')' <Block>
				// <Scalar> Id '(' <Paramse>  ')' ';'
				if( CheckTerminal(node[1], PT_ID) )
				{
					cTempVar.clear(); // clear tempvars here
					// count parameters (will return 0 when using an incorrect nonterminal, which is correct)
					size_t cnt = CompileParams(node[3], true);
					// generate a new function script
					int pos = cEnv.addFunction( node[1].Lexeme(), cnt );
					if( pos>=0 )
					{	
						if( CheckTerminal(node[5], PT_SEMI) )
						{	// a function declaration
							this->logging("function declaration: ");
							PrintChildTerminals(node);
							this->logging(" (id=%i)\n",pos);
							accept = true;
						}
						else // <Block>
						{	// compile the <block>
							int userval;
							CProgramm &prog = cEnv.function(pos);

							if( prog.size() >0 )
							{
								this->logging("error: Function/Script already exists\n");
							}
							else
							{
								prog.appendCommand(OP_START);
								size_t sizepos=prog.appendInt(0);
								if( CompileLabels(node, 0, 0, prog, userval) )
									accept = CompileMain(node[node.count()-1], 0, 0, prog, userval);
								prog.appendCommand(OP_END);
								prog.replaceInt(prog.getCurrentPosition(),sizepos);
								prog.ConvertLabels();


								this->logging("\n");
								prog.dump();
								this->logging("variables:\n");
								for(size_t i=0;i<cTempVar.size(); ++i)
								{
									this->logging("(%i) '%s' type=%i, id=%i, use=%i\n",
										i,(const char*)cTempVar[i],
										cTempVar[i].type,cTempVar[i].id,cTempVar[i].use);
								}
								this->logging("\n");
							}
						}
					}
				}
				if( accept )
				{
					this->logging("accept function: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				else
				{
					this->error("failed\n");
					this->logging("failed function: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				break;

			}
/*			case PT_OLDFUNC:
			case PT_OLDSCRIPT:
			{	// '-' 'script' <Name Id> <Sprite Id> ',' <Block>
				// 'function' 'script' <Name Id> <Block>


				if( (node.count()==6  && CheckNonTerminal(node[1], PT_SCRIPT)) ||
					(node.count()==4  && CheckNonTerminal(node[1], PT_SCRIPT))  )
				{
					char name[128], id[128], map[128];
					unsigned short x,y,xs,ys,sprite;
					unsigned char dir;
					CompileScriptHeader(node,id, name, map, x, y, xs,ys, dir, sprite);

					// generate a new script
					int pos= cEnv.addScript(id, name, map, x, y, xs, ys, dir, sprite);
					if( pos>=0 )
					{	// compile the <block>

						int userval;
						CProgramm &prog = cEnv.script(pos);

						if( prog.size() >0 )
						{
							this->logging("error: Script already exists\n");
						}
						else
						{
							cTempVar.clear();

							prog.appendCommand(OP_START);
							size_t sizepos=prog.appendInt(0);
							if( CompileLabels(node, 0, 0, prog, userval) )
								accept = CompileMain(node[node.count()-1], 0, 0, prog, userval);
							prog.appendCommand(OP_END);
							prog.replaceInt(prog.getCurrentPosition(),sizepos);
							prog.ConvertLabels();


							this->logging("\n");
							prog.dump();
							this->logging("variables:\n");
							for(size_t i=0;i<cTempVar.size(); ++i)
							{
								this->logging("(%i) '%s' type=%i, id=%i, use=%i\n",
									i,(const char*)cTempVar[i],
									cTempVar[i].type,cTempVar[i].id,cTempVar[i].use);
							}
							this->logging("\n");
						}
					}
				}

				if( accept )
				{
					this->logging("accept script: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				else
				{
					this->logging("failed script: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				break;
			}
*/
			case PT_OLDMAPFLAG:
			case PT_OLDDUP:
			case PT_OLDMOB:
			case PT_OLDSHOP:
			case PT_OLDWARP:
			case PT_SHOP:
			case PT_WARP:
			case PT_OLDMAPFLAGHEAD:
			case PT_OLDDUPHEAD:
			case PT_OLDSHOPHEAD:
			case PT_OLDWARPHEAD:
			case PT_OLDMONSTERHEAD:
			{
				this->logging("not yet supported %s functional, skip compiling\n", node.SymbolName());
				accept = true;
				break;
			}

			case PT_OLDFUNC:
			case PT_OLDSCRIPT:
			case PT_OLDNPC:
			case PT_NPC:
			case PT_MOB:
			{
				this->logging("not fully supported %s functional, compiling only contained blocks\n", node.SymbolName());
				accept = true;
				const parsenode* pn =  &node[ node.count()-1];
				if( CheckNonTerminal( *pn, PT_EVENT) )
					pn = &pn->operator[](0);

				if( CheckNonTerminal(*pn, PT_BLOCK) )
				{	// compile block if exist
					int userval;
					CProgramm prog;

					cTempVar.clear();

					prog.appendCommand(OP_START);
					size_t sizepos=prog.appendInt(0);
					if( CompileLabels(*pn, 0, 0, prog, userval) )
						accept = CompileMain(*pn, 0, 0, prog, userval);
					prog.appendCommand(OP_END);
					prog.replaceInt(prog.getCurrentPosition(),sizepos);
					prog.ConvertLabels();


					this->logging("\n");
					prog.dump();
					this->logging("variables:\n");
					for(size_t i=0;i<cTempVar.size(); ++i)
					{
						this->logging("(%i) '%s' type=%i, id=%i, use=%i\n",
							i,(const char*)cTempVar[i],
							cTempVar[i].type,cTempVar[i].id,cTempVar[i].use);
					}
					this->logging("\n");
				}
				if( accept )
				{
					this->logging("accept script: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				else
				{
					this->error("failed\n");
					this->logging("failed script: ");
					PrintChildTerminals(node);
					this->logging("\n");
				}
				break;
			}

			case PT_DECLS:
			{	// go through all declarations
				for(i=0; i<node.count(); ++i)
				{
					accept = CompileTree(node[i]);
					if( !accept ) break;
				}
				break;
			}
			default:
			{	
				PrintChildTerminals(node); this->logging(" not allowed on this scope\n");
				break;
			}
			}// switch
		}
		///////////////////////////////////////////////////////////////////////
		return accept;
	}

};







const unsigned char* getEngine(ulong &sz);
void buildEngine();
///////////////////////////////////////////////////////////////////////////////
#endif//_EACOMPILER_
