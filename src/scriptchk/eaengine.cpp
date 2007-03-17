// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "eaengine.h"
#include "eacompiler.h"
#include "eastorage.h"



void CStackEngine::array_select(basics::variant& arr, const basics::variant* selectlist, int elems)
{
	basics::variant  tmp;
	// save the value
	tmp.assign(arr, false);
	// set the new size
	arr.resize_array((elems<0)?0:elems);
	int i;
	size_t p;
	const size_t sz = tmp.size();
	for(i=0; i<elems; ++i)
	{
		p = selectlist[i].get_int();
		if(p<sz)
			arr[i] = tmp[p];
		else
			arr[i].clear();
	}
}

void CStackEngine::array_splice(basics::variant& arr, int start, int end, int ofs, int cnt)
{
	basics::variant  tmp;
	// save the value
	tmp.assign(arr, false); 
	// check the run parameters
	if(ofs==0)
		ofs=1;
	else if(ofs<0)
	{
		basics::swap(start,end);
		ofs = -ofs;
	}
	if(cnt<=0)
		cnt=1;
	// create a new array
	//##TODO: add size checks here
	arr.clear();
	arr.create_array(1+abs(end-start)/ofs);

	//##TODO: add array manipulation to variant
	int i,k,n, sz=tmp.size();
	if( start<=end )
	{
		for(i=0, n=0; n<cnt; ++n)
		{
			for(k=start; k<0  && k<=end; ++i, k+=ofs) {}
			for(       ; k<sz && k<=end; ++i, k+=ofs) arr[i] = tmp[k];
			for(       ;         k<=end; ++i, k+=ofs) {}
		}
	}
	else
	{	// also reverse the elements
		for(i=0, n=0; n<cnt; ++n)
		{
			for(k=start; k>=sz && k>end; ++i, k-=ofs) {}
			for(       ; k>=0  && k>end; ++i, k-=ofs) arr[i] = tmp[k];
			for(       ;          k>end; ++i, k-=ofs) {}
		}
	}
}

bool CStackEngine::process()
{
	if( !this->cProg.exists() )
		return false;

	bool run = true;
	scriptprog::CCommand ccmd;
	scriptprog& prog = *this->cProg;

	basics::variant *stack;

	while(run && prog.getCommand(this->cPC, ccmd) )
	{
		const ssize_t stacksize = this->cStack.end()-this->cStack.begin();
		stack = this->cStack.end()-1;

		if( stacksize < (int)ccmd.cCount )
		{	// soemthing wrong
			fprintf(stderr, "stack underflow, %u values requited, have %i\n", (uint)ccmd.cCount, (int)stacksize);
			return false;
		}
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
			stack[0] = stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_ARRAY_ASSIGN:
		{
			stack[0].operator_assign(stack[-1]);
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_ADD_ASSIGN:
		{
			stack[0] += stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_SUB_ASSIGN:
		{
			stack[0] -= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_MUL_ASSIGN:
		{
			stack[0] *= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_DIV_ASSIGN:
		{
			stack[0] /= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_MOD_ASSIGN:
		{
			stack[0] %= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_BIN_XOR_ASSIGN:
		{
			stack[0] ^= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_BIN_AND_ASSIGN:
		{
			stack[0] &= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_BIN_OR_ASSIGN:
		{
			stack[0] |= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_RSHIFT_ASSIGN:
		{
			stack[0] >>= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_LSHIFT_ASSIGN:
		{
			stack[0] <<= stack[-1];
			stack[-1].assign(stack[0], false);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_ADD:		// <Op AddSub> '+' <Op MultDiv>
		{
			stack[-1] += stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_SUB:		// <Op AddSub> '-' <Op MultDiv>
		{
			stack[-1] -= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_MUL:		// <Op MultDiv> '*' <Op Unary>
		{
			stack[-1] *= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_DIV:		// <Op MultDiv> '/' <Op Unary>
		{
			stack[-1] /= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_MOD:		// <Op MultDiv> '%' <Op Unary>
		{
			stack[-1] %= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_BIN_XOR:	// <Op BinXOR> '^' <Op BinAND>
		{
			stack[-1] ^= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_BIN_AND:		// <Op BinAND> '&' <Op Equate>
		{
			stack[-1] &= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_BIN_OR:		// <Op BinOr> '|' <Op BinXOR>
		{
			stack[-1] |= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_RSHIFT:		// <Op Shift> '>>' <Op AddSub>
		{
			stack[-1] >>= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_LSHIFT:		// <Op Shift> '<<' <Op AddSub>
		{
			stack[-1] <<= stack[0];
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_EQUATE:		// <Op Equate> '==' <Op Compare>
		{
			stack[-1] = (stack[-1]==stack[0]);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_UNEQUATE:	// <Op Equate> '!=' <Op Compare>
		{
			stack[-1] = (stack[-1]!=stack[0]);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_ISGT:		// <Op Compare> '>'  <Op Shift>
		{
			stack[-1] = (stack[-1]> stack[0]);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_ISGTEQ:		// <Op Compare> '>=' <Op Shift>
		{
			stack[-1] = (stack[-1]>=stack[0]);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_ISLT:		// <Op Compare> '<'  <Op Shift>
		{
			stack[-1] = (stack[-1]<=stack[0]);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		case OP_ISLTEQ:		// <Op Compare> '<=' <Op Shift>
		{
			stack[-1] = (stack[-1]<=stack[0]);
			stack[-1].make_value();
			this->cStack.strip(1);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// unary operations
		// take one stack values and push a value
		case OP_NOT:	// '!'    <Op Unary>
		{
			stack[0].make_value();
			stack[0].lognot();
			break;
		}
		case OP_INVERT:	// '~'    <Op Unary>
		{
			stack[0].make_value();
			stack[0].invert();
			break;
		}
		case OP_NEGATE:	// '-'    <Op Unary>
		{
			stack[0].make_value();
			stack[0].negate();
			break;
		}

		/////////////////////////////////////////////////////////////////
		// sizeof operations
		// take one stack values and push the result
							// sizeof '(' <Type> ')' // replaces with OP_PUSH_INT on compile time
		case OP_SIZEOF:		// sizeof '(' Id ')'
		{
			stack[0].make_value();
			stack[0] = stack[0].size();
			break;
		}

		/////////////////////////////////////////////////////////////////
		// cast operations
		// take one stack values and push the result
		case OP_CAST_INTEGER:	// '(' <Type> ')' <Op Unary>
		{	//
			stack[0].make_value();
			stack[0].cast(basics::VAR_INTEGER);
			break;
		}
		case OP_CAST_STRING:	// '(' <Type> ')' <Op Unary>
		{	//
			stack[0].make_value();
			stack[0].cast(basics::VAR_STRING);
			break;
		}
		case OP_CAST_FLOAT:	// '(' <Type> ')' <Op Unary>
		{	//
			stack[0].make_value();
			stack[0].cast(basics::VAR_FLOAT);
			break;
		}

		/////////////////////////////////////////////////////////////////
		// Pre operations
		// take one stack variable and push a value
		case OP_PREADD:		// '++'   <Op Unary>
		{	
			++stack[0];
			stack[0].make_value();
			break;
		}
		case OP_PRESUB:		// '--'   <Op Unary>
		{	
			--stack[0];
			stack[0].make_value();
			break;
		}
		/////////////////////////////////////////////////////////////////
		// Post operations
		// take one stack variable and push a value
		case OP_POSTADD:	// <Op Pointer> '++'
		{	
			basics::variant temp(stack[0]);
			stack[0].make_value();
			++temp;
			break;
		}
		case OP_POSTSUB:	// <Op Pointer> '--'
		{	
			basics::variant temp(stack[0]);
			stack[0].make_value();
			--temp;
			break;
		}

		/////////////////////////////////////////////////////////////////
		// Member Access
		// take a variable and a value from stack and push a variable
		case OP_MEMBER:		// <Op Pointer> '.' <Value>
		case OP_SCOPE:		// <Op Pointer> '::' <Value>
		{
			stack[0].access_member(ccmd.cString);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// Array
		// take a variable and some values from stack and push a variable
		case OP_ARRAY:		// <Op Pointer> '[' <Expr> ']'
		{
			stack[-1].assign( stack[-1][ stack[0].get_int() ], true);
			this->cStack.strip(1);
			break;
		}
		case OP_ARRAYSEL:	// <Op Pointer> '[' <Expr List> ']'
		{
			if( stacksize < ccmd.cParam )
			{	// soemthing wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", (int)ccmd.cParam, (int)stacksize);
				return false;
			}
			stack[-ccmd.cParam].make_value();
			array_select(stack[-ccmd.cParam],
						 &stack[-ccmd.cParam+1],
						 ccmd.cParam);
			this->cStack.strip(ccmd.cParam-1);
			break;
		}
		case OP_RANGE:		// <Op Pointer> '[' <Expr>':'<Expr> ']'
		{
			stack[-2].make_value();
			array_splice(stack[-2],
						 stack[-1].get_int(),
						 stack[ 0].get_int());
			this->cStack.strip(2);
			break;
		}
		case OP_SPLICE:		// <Op Pointer> '[' <Expr>':'<Expr>':'<Expr> ']'
		{
			stack[-3].make_value();
			array_splice(stack[-3],
						 stack[-2].get_int(),
						 stack[-1].get_int(),
						 stack[ 0].get_int());
			this->cStack.strip(3);
			break;
		}
		case OP_DULICATE:		// <Op Pointer> '[' <Expr>':'<Expr>':*'<Expr> ']'
		{
			stack[-3].make_value();
			array_splice(stack[-3],
						 stack[-2].get_int(),
						 stack[-1].get_int(),
						 1,
						 stack[ 0].get_int());
			this->cStack.strip(3);
			break;
		}
		case OP_CREATEARRAY:
		{
			//this->logging("array resize (%i dimension(s))", ccmd.cParam);
			// there are (ccmd.cParam) elements on stack containing the dimensions of a multi-dimensional array
			const int dim = ccmd.cParam;
			int i;
			if( stacksize < dim )
			{	// soemthing wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", dim, (int)stacksize);
				return false;
			}
			for(i=1; i<=dim; ++i)
			{	// number of elements in this dimension
				stack[-dim].append_array( stack[i-dim].get_int() );
			}
			this->cStack.strip(dim);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// conditional branch
		// take one value from stack 
		// and add 1 or the branch offset to the programm counter
		case OP_NIF:
		{
			if( stack[0] == 0 )
				this->cPC = ccmd.cParam;
			this->cStack.strip(1);
			break;
		}
		case OP_IF:		// if '(' <Expr> ')' <Normal Stm>
		{
			if( stack[0] != 0 )
				this->cPC = ccmd.cParam;
			this->cStack.strip(1);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// conditional branch or Pop
		case OP_NIF_POP:
		{
			if( stack[0] == 0 )
				this->cPC = ccmd.cParam;
			else
				this->cStack.strip(1);
			break;
		}
		case OP_IF_POP:		// if '(' <Expr> ')' <Normal Stm>
		{
			if( stack[0] != 0 )
				this->cPC = ccmd.cParam;
			else
				this->cStack.strip(1);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// unconditional branch
		// add the branch offset to the programm counter
		case OP_GOTO:	// goto position
		{
			this->cPC = ccmd.cParam;
			break;
		}
		case OP_EMPTY:
		{
			stack[0].empty();
			break;
		}
		case OP_CONCAT:
		{
			//this->logging("vectorize '%i' elements", ccmd.cParam); break;
			const int cnt = ccmd.cParam;
			int i;
			basics::variant temp;
			if( stacksize < cnt )
			{	// soemthing wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", cnt, (int)stacksize);
				return false;
			}
			temp.create_array( cnt );
			for(i=0; i<cnt; ++i)
			{	// put the elements into the array
				temp[i] = stack[i-cnt];				
			}
			this->cStack.strip(cnt-1);
			this->cStack.last().assign(temp, false);	// assign a new value
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
		case OP_PUSH_NONE:
		{
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last().clear();
			break;
		}
		case OP_PUSH_ZERO:
		case OP_PUSH_ONE:
		case OP_PUSH_TWO:
		case OP_PUSH_THREE:
		case OP_PUSH_FOUR:
		case OP_PUSH_FIVE:
		case OP_PUSH_SIX:
		case OP_PUSH_SEVEN:
		case OP_PUSH_EIGHT:
		case OP_PUSH_NINE:
		case OP_PUSH_TEN:
		{
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last() = ccmd.cCommand-OP_PUSH_ZERO;
			break;
		}
		case OP_PUSH_INT:	// followed by an integer
		{
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last() = ccmd.cParam;
			break;
		}
		case OP_PUSH_STRING:	// followed by a string address
		{
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last() = ccmd.cString;
			break;
		}
		case OP_PUSH_FLOAT:	// followed by a float
		{
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last() = scriptprog::int2float(ccmd.cParam);
			break;
		}
		case OP_PUSH_VAR:	// followed by a string containing a variable name
		{
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last().clear();
			this->cStack.last().access_member(ccmd.cString);
			// but also could decode the variable string directly
			this->cStack.last().make_reference();
			break;			
		}
		case OP_PUSH_VAL:
		{
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last().clear();
			this->cStack.last().access_member(ccmd.cString);
			// but also could decode the variable string directly
			this->cStack.last().make_value();
			break;			
		}
		case OP_PUSH_PARAVAR:
		{	// push a reference
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last().assign(((size_t)ccmd.cParam<this->cPara.size())?this->cPara[ccmd.cParam]:basics::variant(), true);
			break;
		}
		case OP_PUSH_PARAVAL:
		{	// push a reference/value
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last().assign(((size_t)ccmd.cParam<this->cPara.size())?this->cPara[ccmd.cParam]:basics::variant(), false);
			break;
		}
		case OP_PUSH_TEMPVAR:
		{	// push a reference
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last().assign(((size_t)ccmd.cParam<this->cTemp.size())?this->cTemp[ccmd.cParam]:basics::variant(), true);
			break;
		}
		case OP_PUSH_TEMPVAL:
		{	// push a value
			this->cStack.resize(this->cStack.size()+1);
			this->cStack.last().make_value();
			this->cStack.last().assign(((size_t)ccmd.cParam<this->cTemp.size())?this->cTemp[ccmd.cParam]:basics::variant(), false);
			break;
		}
		case OP_POP:	// decrements the stack
		{	
			this->cStack.resize(this->cCC);
			break;
		}
		case OP_EVAL:
		{	//evaluate
			stack[0].make_value();
			break;
		}
		case OP_BOOLEAN:
		{	//evaluate
			stack[0].make_value();
			stack[0] = stack[0].get_bool();
			break;
		}

		/////////////////////////////////////////////////////////////////
		// standard function calls
		// check the values on stack before or inside the call of function
		case OP_BLDFUNCTION:
		case OP_FUNCTION:
		{	// save the current stack/programm/counters
			// prepare a new stack
			// get the programm and the function entry
			// set initial programm counter
			const ssize_t sz = 1+stack[0].get_int();
			if( stacksize < sz )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", (int)sz, (int)stacksize);
				return false;
			}
			basics::variant result = this->call_function(ccmd.cParam);
			this->cStack.strip(sz-1);
			this->cStack.last() = result;
			break;
		}
		case OP_SUBFUNCTION:
		{	// save the current stack/programm/counters
			// prepare a new stack
			// get the programm and the function entry
			// set initial programm counter
			const ssize_t sz = 2+stack[0].get_int();
			if( stacksize < sz )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", (int)sz, (int)stacksize);
				return false;
			}
			basics::variant result = this->call_function(ccmd.cParam);
			this->cStack.strip(sz-1);
			this->cStack.last() = result;
			break;
		}
		case OP_GOSUB:
		{	// save the current stack/programm/counters
			// get the programm and the label entry
			// set initial programm counter
			break;
		}
		case OP_RETURN:
		{	// restore the previous stack/programm/counters
			// if stacksize>0
			
			// fall through if no program on the stack
		}
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

///////////////////////////////////////////////////////////////////////////
/// start a programm
bool CStackEngine::start(const scriptprog::script& prog, const basics::string<>& startlabel)
{
	if( prog.exists() )
	{
		uint startpos = prog->get_labeltarget(startlabel);
		scriptdecl decl = prog->get_declaration("main");
		if( this->cProg.exists() )
		{	// queue the script in

			return true;
		}
		else
		{
			// start a new program
			this->cStack.clear();


			// set the new run parameter
			this->cCC       = 0;
			this->cPC       = startpos;
			this->cDecl     = decl;
			this->cProg     = prog;

			this->cStack.resize(this->cCC);

			// run it
			return run_script();
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
/// coming back from a callback with return value
bool CStackEngine::cont(const basics::variant& retvalue)
{
	if( this->is_paused() )
	{	// push the return value
		this->cStack.resize(this->cStack.size()+1);
		this->cStack.last().make_value();
		this->cStack.last() = retvalue;

		// run the script
		return run_script();
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////
/// run the currently selected script
bool CStackEngine::run_script()
{
	// process the current script
	const bool ret=this->process();
	// finalize
	if( !ret || this->cState == OFFLINE || this->cState == RUNNING )
	{	// error or not paused
		this->cState = OFFLINE;
		this->cStack.clear();
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////
/// calls a script from the current script (gosub).
/// format is "<script name>::<start label>", label can be omitted
bool CStackEngine::call_script(const char* name)
{
	if( name )
	{
		script prog;
		scriptdecl decl;
		uint startpos = 0;
		const char*pp = strstr(name,"::");

		// find the script


		// get the start position
		if(pp)
		{
			startpos = prog->get_labeltarget(pp+2);
		}
		// save the run parameter

		// set the new run parameter
		this->cCC       = 0;
		this->cPC       = startpos;
		this->cDecl     = decl;
		this->cProg     = prog;
		
		this->cStack.resize(this->cCC);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
/// calls a script function from the current script (function call)
bool CStackEngine::call_function(const uint param)
{

	script prog;
	scriptdecl decl;

	uint startpos = 0;
	// find the function
	// get the start position

	// save the run parameter

	// set the new run parameter
	this->cCC       = 0;
	this->cPC       = startpos;
	this->cDecl     = decl;
	this->cProg     = prog;

	this->cStack.resize(this->cCC);
	return true;
}


///////////////////////////////////////////////////////////////////////////
/// return from a script
bool CStackEngine::return_script(const basics::variant& retvalue)
{
	// dequeue the old script
	void *tmp=NULL;
	if(tmp)
	{	// restore the run parameter

		// push the return value
		this->cStack.resize(this->cStack.size()+1);
		this->cStack.last().make_value();
		this->cStack.last() = retvalue;
		return true;
	}
	else
	{	// return from the first level
		return false;
	}
}

