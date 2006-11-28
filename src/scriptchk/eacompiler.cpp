// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "eacompiler.h"


class Variable : public basics::string<>
{
	Variable(const Variable&);					// no copy
	const Variable& operator=(const Variable&);	// no assign

public:
	basics::Variant		cValue;
	///////////////////////////////////////////////////////////////////////////
	// Construct/Destruct
	Variable(const char* n) : basics::string<>(n)				{  }
	Variable(const basics::string<>& n) : basics::string<>(n)	{  }
	~Variable()	{  }

	///////////////////////////////////////////////////////////////////////////
	// PreInitialize Array
	bool setsize(size_t cnt)
	{
	//	cValue.setarray(cnt);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// Set Values
	void set(int val)						{ cValue = val; }
	void set(double val)					{ cValue = val; }
	void set(const char* val)				{ cValue = val; }
	void set(const basics::Variant & val)	{ cValue = val; }

	///////////////////////////////////////////////////////////////////////////
	// read/set Value
	operator const basics::Variant&() const				{ return cValue; }
	const Variable& operator=(const basics::Variant& v)	{ cValue=v; return *this; }
};




///////////////////////////////////////////////////////////////////////////////
// the user stack
///////////////////////////////////////////////////////////////////////////////
class UserStack : private basics::noncopyable
{
	///////////////////////////////////////////////////////////////////////////
	basics::stack<basics::Variant>			cStack;		// the stack
	size_t					cSC;		// stack counter
	size_t					cSB;		// initial stack start index
	size_t					cParaBase;	// function parameter start index
	size_t					cTempBase;	// TempVar start index

	size_t					cPC;		// Programm Counter
	CProgramm*				cProg;

	bool process()
	{
		bool run = true;
		CProgramm::CCommand ccmd;

		while(cProg && run && cProg->getCommand(cPC, ccmd) )
		{
			switch( ccmd.cCommand )
			{
			case OP_START:
			case OP_NOP:
			{
				break;
			}
			/////////////////////////////////////////////////////////////////
			// assignment operations
			// take two stack values and push up one
			case OP_ASSIGN:	// <Op If> '='   <Op>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_ADD:		// <Op AddSub> '+' <Op MultDiv>
			case OP_ASSIGN_ADD:	// <Op If> '+='  <Op>
			{
				cSC--;
				cStack[cSC-1] += cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_SUB:		// <Op AddSub> '-' <Op MultDiv>
			case OP_ASSIGN_SUB:	// <Op If> '-='  <Op>
			{
				cSC--;
				cStack[cSC-1] -= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_MUL:		// <Op MultDiv> '*' <Op Unary>
			case OP_ASSIGN_MUL:	// <Op If> '*='  <Op>
			{
				cSC--;
				cStack[cSC-1] *= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_DIV:		// <Op MultDiv> '/' <Op Unary>
			case OP_ASSIGN_DIV:	// <Op If> '/='  <Op>
			{
				cSC--;
				cStack[cSC-1] /= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_MOD:		// <Op MultDiv> '%' <Op Unary>
			{
				cSC--;
				cStack[cSC-1] /= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_BIN_XOR:	// <Op BinXOR> '^' <Op BinAND>
			case OP_ASSIGN_XOR:	// <Op If> '^='  <Op>
			{
				cSC--;
				cStack[cSC-1] ^= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_BIN_AND:		// <Op BinAND> '&' <Op Equate>
			case OP_ASSIGN_AND:	// <Op If> '&='  <Op>
			{
				cSC--;
				cStack[cSC-1] &= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_BIN_OR:		// <Op BinOr> '|' <Op BinXOR>
			case OP_ASSIGN_OR:	// <Op If> '|='  <Op>
			{
				cSC--;
				cStack[cSC-1] |= cStack[cSC];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_LOG_AND:	// <Op And> '&&' <Op BinOR>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] && cStack[cSC];
				break;
			}
			case OP_LOG_OR:		// <Op Or> '||' <Op And>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] || cStack[cSC];
				break;
			}
			case OP_RSHIFT:		// <Op Shift> '>>' <Op AddSub>
			case OP_ASSIGN_RSH:	// <Op If> '>>='  <Op>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] >> cStack[cSC];
				break;
			}
			case OP_LSHIFT:		// <Op Shift> '<<' <Op AddSub>
			case OP_ASSIGN_LSH:	// <Op If> '<<='  <Op>
			{
				cSC--;
				cStack[cSC-1] = cStack[cSC-1] << cStack[cSC];
				break;
			}
			case OP_EQUATE:		// <Op Equate> '==' <Op Compare>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]==cStack[cSC]);
				break;
			}
			case OP_UNEQUATE:	// <Op Equate> '!=' <Op Compare>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]!=cStack[cSC]);
				break;
			}
			case OP_ISGT:		// <Op Compare> '>'  <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]>cStack[cSC]);
				break;
			}
			case OP_ISGTEQ:		// <Op Compare> '>=' <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]>=cStack[cSC]);
				break;
			}
			case OP_ISLT:		// <Op Compare> '<'  <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]<cStack[cSC]);
				break;
			}
			case OP_ISLTEQ:		// <Op Compare> '<=' <Op Shift>
			{
				cSC--;
				cStack[cSC-1] = (cStack[cSC-1]<=cStack[cSC]);
				break;
			}
			/////////////////////////////////////////////////////////////////
			// select operation
			// take three stack values and push the second or third depending on the first
			case OP_SELECT:	// <Op Or> '?' <Op If> ':' <Op If>
			{
				cSC -= 2;
				cStack[cSC-1] = (cStack[cSC-1]!=0) ? cStack[cSC] : cStack[cSC+1];
				break;
			}

			/////////////////////////////////////////////////////////////////
			// unary operations
			// take one stack values and push a value
			case OP_NOT:	// '!'    <Op Unary>
			{
				cStack[cSC-1] = !(cStack[cSC-1]);
				break;
			}
			case OP_INVERT:	// '~'    <Op Unary>
			{
				cStack[cSC-1] = ~(cStack[cSC-1]);
				break;
			}
			case OP_NEGATE:	// '-'    <Op Unary>
			{
				cStack[cSC-1] = -(cStack[cSC-1]);
				break;
			}

			/////////////////////////////////////////////////////////////////
			// sizeof operations
			// take one stack values and push the result
								// sizeof '(' <Type> ')' // replaces with OP_PUSH_INT on compile time
			case OP_SIZEOF:		// sizeof '(' Id ')'
			{
				cStack[cSC-1].makeValue();
				cStack[cSC-1] = (int)cStack[cSC-1].size();
				break;
			}

			/////////////////////////////////////////////////////////////////
			// cast operations
			// take one stack values and push the result
			case OP_CAST:	// '(' <Type> ')' <Op Unary>   !CAST
			{	// <Op Unary> is first on the stack, <Type> is second
				cStack[cSC-1].cast(ccmd.cParam1);
				break;
			}

			/////////////////////////////////////////////////////////////////
			// Pre operations
			// take one stack variable and push a value
			case OP_PREADD:		// '++'   <Op Unary>
			{	
				++cStack[cSC-1];
				cStack[cSC-1].makeValue();
				break;
			}
			case OP_PRESUB:		// '--'   <Op Unary>
			{	
				--cStack[cSC-1];
				cStack[cSC-1].makeValue();
				break;
			}
			/////////////////////////////////////////////////////////////////
			// Post operations
			// take one stack variable and push a value
			case OP_POSTADD:	// <Op Pointer> '++'
			{	
				basics::Variant temp = cStack[cSC-1];
				temp.makeValue();
				cStack[cSC-1]++;
				cStack[cSC-1].makeValue();
				cStack[cSC-1] = temp;
				break;
			}
			case OP_POSTSUB:	// <Op Pointer> '--'
			{	
				basics::Variant temp(cStack[cSC-1]);
				temp.makeValue();
				cStack[cSC-1]--;
				cStack[cSC-1].makeValue();
				cStack[cSC-1] = temp;
				break;
			}

			/////////////////////////////////////////////////////////////////
			// Member Access
			// take a variable and a value from stack and push a varible
			case OP_MEMBER:		// <Op Pointer> '.' <Value>     ! member
			{
				printf("not implemented yet\n");

				cSC--;
				break;
			}
			/////////////////////////////////////////////////////////////////
			// Array
			// take a variable and a value from stack and push a varible
			case OP_ARRAY:		// <Op Pointer> '[' <Expr> ']'  ! array
			{
				cSC--;
				cStack[cSC-1].assign( cStack[cSC-1][ cStack[cSC].getInt() ], true);
				break;
			}
			/////////////////////////////////////////////////////////////////
			// standard function calls
			// check the values on stack before or inside the call of function
			case OP_CALLSCRIPT1:
			case OP_CALLSCRIPT2:
			case OP_CALLSCRIPT3:
			case OP_CALLSCRIPT4:
							// Id '(' <Expr> ')'
							// Id '(' ')'
							// Id <Call List> ';'
							// Id ';'
			{

				printf("not implemented yet\n");
				cSC--;
				break;
			}
			/////////////////////////////////////////////////////////////////
			// standard function calls
			// check the values on stack before or inside the call of function
			case OP_CALLBUILDIN1:
			case OP_CALLBUILDIN2:
			case OP_CALLBUILDIN3:
			case OP_CALLBUILDIN4:
							// Id '(' <Expr> ')'
							// Id '(' ')'
							// Id <Call List> ';'
							// Id ';'
			{
				printf("not implemented yet\n");
				cSC--;
				break;
			}

			/////////////////////////////////////////////////////////////////
			// conditional branch
			// take one value from stack 
			// and add 1 or the branch offset to the programm counter
			case OP_NIF:
			{
				cSC--;
				if( cStack[cSC]==0 )
					cPC = ccmd.cParam1;

				break;
			}
			case OP_IF:		// if '(' <Expr> ')' <Normal Stm>
			{
				if( cStack[cSC]!=0 )
					cPC = ccmd.cParam1;

				break;
			}
			/////////////////////////////////////////////////////////////////
			// unconditional branch
			// add the branch offset to the programm counter
			case OP_GOTO:	// goto position
			{
				cPC = ccmd.cParam1;
				break;
			}

			case OP_CLEAR:
			{
				cStack[cSC-1].clear();
				break;
			}
			case OP_RESIZE:
			{	//this->logging("array resize (%i dimension(s))", ccmd.cParam1); break;
				// there are (ccmd.cParam1) elements on stack containing the dimemsions oth the multi-array
				size_t i, dim = ccmd.cParam1;
				cSC -= dim;
				cStack[cSC-1].clear();
				for(i=0; i<dim; ++i)
				{	// number of elements in this dimension
					cStack[cSC-1].addarray( cStack[cSC+i].getInt() );
				}
				break;
			}
			case OP_VECTORIZE1:
			case OP_VECTORIZE2:
			case OP_VECTORIZE3:
			case OP_VECTORIZE4:
			{	//this->logging("vectorize '%i' elements", ccmd.cParam1); break;
				size_t i, cnt = ccmd.cParam1;
				cSC -= cnt;
				cStack[cSC].setarray( cnt );
				for(i=1; i<cnt; ++i)
				{	// put the elements into the array
					cStack[cSC][i] = cStack[cSC+i];				
				}
				// and virtually push the vectorized element
				cSC ++;
				break;
			}

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

			case OP_PUSH_ADDR:
			case OP_PUSH_INT1:	// followed by an integer
			case OP_PUSH_INT2:
			case OP_PUSH_INT3:
			case OP_PUSH_INT4:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = ccmd.cParam1;
				cSC++;
				break;
			}
			case OP_PUSH_STRING:	// followed by a string (pointer)
			{
				cStack[cSC].makeValue();
				cStack[cSC] = ccmd.cString;
				cSC++;
				break;
			}
			case OP_PUSH_FLOAT:	// followed by a float
			{
				cStack[cSC].makeValue();
				cStack[cSC] = (double)CProgramm::int2float(ccmd.cParam1);
				cSC++;
				break;
			}
			case OP_PUSH_VAR:	// followed by a string containing a variable name
			{
				cStack[cSC].makeValue();
//				cStack[cSC] = findvariable(ccmd.cString);
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_VALUE:
			{
				cStack[cSC].makeValue();
//				cStack[cSC] = findvariable(ccmd.cString);
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_PARAM:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = cStack[cParaBase+ccmd.cParam1];
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_TEMPVAR1:
			case OP_PUSH_TEMPVAR2:
			case OP_PUSH_TEMPVAR3:
			case OP_PUSH_TEMPVAR4:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = cStack[cSB+ccmd.cParam1];
				cStack[cSC].makeVariable();
				cSC++;
				break;
			}
			case OP_PUSH_TEMPVALUE1:
			case OP_PUSH_TEMPVALUE2:
			case OP_PUSH_TEMPVALUE3:
			case OP_PUSH_TEMPVALUE4:
			{
				cStack[cSC].makeValue();
				cStack[cSC] = cStack[cSB+ccmd.cParam1];
				cSC++;
				break;
			}

			case OP_POP:	// decrements the stack by one
			{	// maybe better reset the stack
				cSC--;
				break;
			}


			case VX_LABEL:
			case VX_BREAK:
			case VX_CONT:
			case VX_GOTO:
				printf("non-converted temporal opcodes\n");

			case OP_RETURN:
			case OP_END:
			default:
			{
				run=false;
				break;
			}
			}// end switch
		}// end while
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	UserStack() : cSC(0), cSB(0), cParaBase(0), cPC(0), cProg(NULL)
	{}
	~UserStack()
	{}

	///////////////////////////////////////////////////////////////////////////
	// start a programm
	bool Call(size_t programm_id)
	{		

	
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// comming back from a callback
	void Continue(basics::Variant& retvalue)
	{


	}
};



struct aaa
{
	char str[24];
	int  val;
};

void vartest()
{

	aaa xxa = {"hallo",1};
	aaa xxb = {"xx",2};

	aaa xxc;

	xxc = xxa;
	xxc = xxb;


	/////////////////////////
	basics::Variant a,b,c;
	a = 8.6;

	basics::Variant ref(a,true);
	ref += 2;

	printf("%s\n", (const char*)a.getString());
	printf("%s\n", (const char*)ref.getString());

	b = "hallo";

	ref = a+b;

	printf("%s\n", (const char*)a.getString());
	printf("%s\n", (const char*)ref.getString());

	ref++;
	printf("%s\n", (const char*)a.getString());
	printf("%s\n", (const char*)ref.getString());

	b.setarray(3);
	b[1] = 1;
	b[2] = 2.2;

	c = a;
	c = b;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	c = a+b;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	b = c++;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	printf("%s\n", (const char*)b[0].getString());
	printf("%s\n", (const char*)b[1].getString());
	printf("%s\n\n", (const char*)b[2].getString());

	c[1] += c[0];
	c[2] =  4;

	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	c += a;
	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
	
	c[2] /= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] &= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] |= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] ^= a;			printf("%s\n", (const char*)c[2].getString());
	c[2] = a && c[2];	printf("%s\n", (const char*)c[2].getString());
	c[2] = a || c[2];	printf("%s\n\n", (const char*)c[2].getString());

	printf("%s\n", (const char*)c[0].getString());
	printf("%s\n", (const char*)c[1].getString());
	printf("%s\n\n", (const char*)c[2].getString());
}






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



scriptstorage::scriptfile* scriptstorage::insert(const basics::string<>& filename)
{
	size_t pos;
	if( !this->files.find(filename, 0, pos) )
	{
		this->files.insert(filename);
	}
	return &(this->files[pos]);
}

void scriptstorage::info()
{
	 basics::slist<scriptfile>::iterator iter(this->files);
	for(; iter; ++iter)
	{
		printf("%s: %i scripts\n", 
			(const char*)(*iter),
			(int)iter->scripts.size());
	}
	printf("%i files\n", (int)this->files.size());
}

