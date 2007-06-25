// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "eaengine.h"
#include "eacompiler.h"
#include "eastorage.h"
#include "eabuildin.h"


// for debug purpose only
basics::map<basics::string<>, basics::variant> tempvalues;


///////////////////////////////////////////////////////////////////////////////
CStackEngine::global_variables CStackEngine::sGlobalVariables;


///////////////////////////////////////////////////////////////////////////////
/// get a variable.
// sample implementation for debugging
bool CStackEngine::global_variables::get_variable(const basics::string<>& name, basics::variant& target)
{
/*
	// current structure is incomplete
	// scope::type::name
	const char* scope_s = name.c_str();
	const char* scope_e = strstr(scope_s,"::");
	if(!scope_e)
		return false;
	const char* type_s = scope_e+2;
	const char* type_e = strstr(type_s,"::");
	if(!type_e)
		return false;
	const char* id_s = type_e+2;
	const char* id_e = strstr(id_s,"::");
	if(!id_e)
		return false;
	const char* name_s = id_e+2;
	const char* name_e = strstr(name_s,"::");
	if(!name_e)
	{
		name_e = name_s;
		while(*name_s)
			++name_s;
	}
	else
	{
		//##TODO add checking for subscopes
		// eg. perm::player::playerid::party::... -> perm::party::playerpartyid::...

		return false;
	}
*/


	basics::variant& val = tempvalues[name];
	target = val;
//	printf("reading '%s'=%s\n", name.c_str(), basics::tostring(target).c_str());

	return true;
}
///////////////////////////////////////////////////////////////////////////////
/// set a variable
// sample implementation for debugging
bool CStackEngine::global_variables::set_variable(const basics::string<>& name, const basics::variant& target)
{

	// sample code just uses the debug 

	tempvalues[name] = target;
//	printf("writing '%s'=%s\n", name.c_str(), basics::tostring(target).c_str());

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// get a variable.
bool CStackEngine::get_variable(const char* name, bool as_value)
{
	// last two values on the stack contain scope and type of the variable
	basics::variant *stack = this->pCall->cStack.end()-1;
	basics::string<> varname;
	
	//##TODO: add variable type decoding here
	// this one only concats the strings together
	varname << stack[-1].get_string() << "::"	// scope 
			<< stack[ 0].get_string() << "::"	// type
			<< 0 << "::"						// id
			<< name;							// name

	// prepare the stack
	this->pCall->cStack.strip(1);
	this->pCall->cStack.last().clear();

	const bool ret = this->sGlobalVariables.get_variable(varname, this->pCall->cStack.last());

	if(as_value)
	{
		this->pCall->cStack.last().evaluate();
	}
	else
	{
		this->cExternal.insert(varname, this->pCall->cStack.last());
	}
	return ret; 
}


///////////////////////////////////////////////////////////////////////////////

void CStackEngine::array_select(basics::variant& arr, const basics::variant* selectlist, int elems)
{
	// save the value
	basics::variant tmp(*arr);
	
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
	// save the value
	basics::variant  tmp(*arr);

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
	arr.create_array(cnt*(1+abs(end-start)/ofs));
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


CStackEngine::state_t CStackEngine::process()
{
	if( !this->pCall || !this->pCall->cProg.exists() )
		return OFFLINE;

	bool run = true;
	scriptprog::CCommand ccmd;

	basics::variant *stack;

	while(run && this->pCall->cProg->getCommand(this->pCall->cPC, ccmd) )
	{
		if( this->cDebugMode )
		{		
			size_t i;
			for(i=0; i<this->pCall->cStack.size(); ++i)
				printf("stack %2i: %s,%s - %s\n", (int)i, 
				this->pCall->cStack[i].is_reference()?"ref":"val",
					basics::variant::type2name(this->pCall->cStack[i].type()), 
					basics::tostring(this->pCall->cStack[i]).c_str());
			printf("\n");
			printf("-------\npc=%i: ", (int)this->pCall->cPC);
			this->pCall->cProg->printCommand(ccmd);
			printf("\n");
		}

		const ssize_t stacksize = this->pCall->cStack.end()-this->pCall->cStack.begin();
		stack = this->pCall->cStack.end()-1;

		if( stacksize < (int)ccmd.cCount )
		{	// something wrong
			fprintf(stderr, "stack underflow, %u values requited, have %i\n", (uint)ccmd.cCount, (int)stacksize);
			return OFFLINE;
		}
		switch( ccmd.cCommand )
		{
		case OP_START:
		{
			if( this->cDebugMode )
			{
				printf("-------\n");
				printf("starting function %s, parameter:\n", this->pCall->cProg->cName.c_str());
				size_t i;
				for(i=0; i<this->pCall->cPara.size(); ++i)
					printf("%i: %s - %s\n", (int)i, basics::variant::type2name(this->pCall->cPara[i].type()), basics::tostring(this->pCall->cPara[i]).c_str());
				printf("-------\n");
			}
		}
		case OP_NOP:
		{
			break;
		}
		/////////////////////////////////////////////////////////////////
		// assignment operations
		// take two stack values and push up one
		case OP_ASSIGN:	// <Op If> '='   <Op>
		{
			*stack[0] = stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ARRAY_ASSIGN:
		{
			stack[0].operator_assign(stack[-1]);
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ADD_ASSIGN:
		{
			stack[0] += stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_SUB_ASSIGN:
		{
			stack[0] -= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_MUL_ASSIGN:
		{
			stack[0] *= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_DIV_ASSIGN:
		{
			stack[0] /= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_MOD_ASSIGN:
		{
			stack[0] %= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_BIN_XOR_ASSIGN:
		{
			stack[0] ^= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_BIN_AND_ASSIGN:
		{
			stack[0] &= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_BIN_OR_ASSIGN:
		{
			stack[0] |= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_RSHIFT_ASSIGN:
		{
			stack[0] >>= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_LSHIFT_ASSIGN:
		{
			stack[0] <<= stack[-1];
			stack[-1] = stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ADD:		// <Op AddSub> '+' <Op MultDiv>
		{
			stack[-1] += stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_SUB:		// <Op AddSub> '-' <Op MultDiv>
		{
			stack[-1] -= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_MUL:		// <Op MultDiv> '*' <Op Unary>
		{
			stack[-1] *= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_DIV:		// <Op MultDiv> '/' <Op Unary>
		{
			stack[-1] /= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_MOD:		// <Op MultDiv> '%' <Op Unary>
		{
			stack[-1] %= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_BIN_XOR:	// <Op BinXOR> '^' <Op BinAND>
		{
			stack[-1] ^= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_BIN_AND:		// <Op BinAND> '&' <Op Equate>
		{
			stack[-1] &= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_BIN_OR:		// <Op BinOr> '|' <Op BinXOR>
		{
			stack[-1] |= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_RSHIFT:		// <Op Shift> '>>' <Op AddSub>
		{
			stack[-1] >>= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_LSHIFT:		// <Op Shift> '<<' <Op AddSub>
		{
			stack[-1] <<= stack[0];
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_EQUATE:		// <Op Equate> '==' <Op Compare>
		{
			stack[-1] = (stack[-1]==stack[0]);
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_UNEQUATE:	// <Op Equate> '!=' <Op Compare>
		{
			stack[-1] = (stack[-1]!=stack[0]);
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ISGT:		// <Op Compare> '>'  <Op Shift>
		{
			stack[-1] = (stack[-1]> stack[0]);
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ISGTEQ:		// <Op Compare> '>=' <Op Shift>
		{
			stack[-1] = (stack[-1]>=stack[0]);
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ISLT:		// <Op Compare> '<'  <Op Shift>
		{
			stack[-1] = (stack[-1]< stack[0]);
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ISLTEQ:		// <Op Compare> '<=' <Op Shift>
		{
			stack[-1] = (stack[-1]<=stack[0]);
			stack[-1].evaluate();
			this->pCall->cStack.strip(1);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// unary operations
		// take one stack values and push a value
		case OP_NOT:	// '!'    <Op Unary>
		{
			stack[0].evaluate();
			stack[0].lognot();
			break;
		}
		case OP_INVERT:	// '~'    <Op Unary>
		{
			stack[0].evaluate();
			stack[0].invert();
			break;
		}
		case OP_NEGATE:	// '-'    <Op Unary>
		{
			stack[0].evaluate();
			stack[0].negate();
			break;
		}

		/////////////////////////////////////////////////////////////////
		// sizeof operations
		// take one stack values and push the result
							// sizeof '(' <Type> ')' // replaces with OP_PUSH_INT on compile time
		case OP_SIZEOF:		// sizeof '(' Id ')'
		{
			stack[0].evaluate();
			stack[0] = stack[0].size();
			break;
		}

		/////////////////////////////////////////////////////////////////
		// cast operations
		// take one stack values and push the result
		case OP_CAST_INTEGER:	// '(' <Type> ')' <Op Unary>
		{	//
			stack[0].evaluate();
			stack[0].cast(basics::VAR_INTEGER);
			break;
		}
		case OP_CAST_STRING:	// '(' <Type> ')' <Op Unary>
		{	//
			stack[0].evaluate();
			stack[0].cast(basics::VAR_STRING);
			break;
		}
		case OP_CAST_FLOAT:	// '(' <Type> ')' <Op Unary>
		{	//
			stack[0].evaluate();
			stack[0].cast(basics::VAR_FLOAT);
			break;
		}

		/////////////////////////////////////////////////////////////////
		// Pre operations
		// take one stack variable and push a value
		case OP_PREADD:		// '++'   <Op Unary>
		{	
			++stack[0];
			stack[0].evaluate();
			break;
		}
		case OP_PRESUB:		// '--'   <Op Unary>
		{	
			--stack[0];
			stack[0].evaluate();
			break;
		}
		/////////////////////////////////////////////////////////////////
		// Post operations
		// take one stack variable and push a value
		case OP_POSTADD:	// <Op Pointer> '++'
		{	
			basics::variant temp(stack[0]);
			stack[0].evaluate();
			++temp;
			break;
		}
		case OP_POSTSUB:	// <Op Pointer> '--'
		{	
			basics::variant temp(stack[0]);
			stack[0].evaluate();
			--temp;
			break;
		}

		/////////////////////////////////////////////////////////////////
		// Member Access
		// take a variable and a value from stack and push a variable
		case OP_MEMBER:		// <Op Pointer> '.' <Value>
		case OP_SCOPE:		// <Op Pointer> '::' <Value>
		{
			if( !stack[0].access_member(ccmd.cString) )
			{	// remove any content when variable access has failed
				fprintf(stderr, "failed accessing member '%s'\n", ccmd.cString);
				return OFFLINE;
			}
			break;
		}
		/////////////////////////////////////////////////////////////////
		// Array
		// take a variable and some values from stack and push a variable
		case OP_ARRAY:		// <Op Pointer> '[' <Expr> ']'
		{
			const size_t inx = stack[0].get_int();
			if( inx < stack[-1].size() )
			{
				stack[-1] = stack[-1][inx];
			}
			else
			{
				fprintf(stderr, "runtime error: array out of bound (defined as [%i], access on %i)\n", (int)stack[-1].size(), (int)inx);
				return OFFLINE;
			}
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_ARRAYSEL:	// <Op Pointer> '[' <Expr List> ']'
		{
			if( stacksize < ccmd.cParam )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", (int)ccmd.cParam, (int)stacksize);
				return OFFLINE;
			}
			stack[-ccmd.cParam].evaluate();
			array_select(stack[-ccmd.cParam],
						 &stack[-ccmd.cParam+1],
						 ccmd.cParam);
			this->pCall->cStack.strip(ccmd.cParam-1);
			break;
		}
		case OP_RANGE:		// <Op Pointer> '[' <Expr>':'<Expr> ']'
		{
			stack[-2].evaluate();
			array_splice(stack[-2],
						 stack[-1].get_int(),
						 stack[ 0].get_int());
			this->pCall->cStack.strip(2);
			break;
		}
		case OP_SPLICE:		// <Op Pointer> '[' <Expr>':'<Expr>':'<Expr> ']'
		{
			stack[-3].evaluate();
			array_splice(stack[-3],
						 stack[-2].get_int(),
						 stack[-1].get_int(),
						 stack[ 0].get_int());
			this->pCall->cStack.strip(3);
			break;
		}
		case OP_DULICATE:		// <Op Pointer> '[' <Expr>':'<Expr>':*'<Expr> ']'
		{
			stack[-3].evaluate();
			array_splice(stack[-3],
						 stack[-2].get_int(),
						 stack[-1].get_int(),
						 1,
						 stack[ 0].get_int());
			this->pCall->cStack.strip(3);
			break;
		}
		case OP_CREATEARRAY:
		{
			//this->logging("array resize (%i dimension(s))", ccmd.cParam);
			// there are (ccmd.cParam) elements on stack containing the dimensions of a multi-dimensional array
			const int dim = ccmd.cParam;
			int i;
			if( stacksize <= dim )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", dim, (int)stacksize);
				return OFFLINE;
			}
			for(i=1; i<=dim; ++i)
			{	// number of elements in this dimension
				stack[-dim].append_array( stack[i-dim].get_int() );
			}
			this->pCall->cStack.strip(dim);
			break;
		}
		case OP_CONCAT:
		{
			//this->logging("vectorize '%i' elements", ccmd.cParam); break;
			const int cnt = ccmd.cParam;
			int i;
			basics::variant temp;
			if( stacksize < cnt )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", cnt, (int)stacksize);
				return OFFLINE;
			}
			temp.create_array( cnt );
			for(i=0; i<cnt; ++i)
			{	// put the elements into the array
				temp[i] = stack[1+i-cnt];				
			}
			this->pCall->cStack.strip(cnt-1);
			this->pCall->cStack.last() = *temp;	// assign a new value
			break;
		}
		/////////////////////////////////////////////////////////////////
		// conditional branch
		// take one value from stack 
		// and add 1 or the branch offset to the programm counter
		case OP_NIF:
		{
			if( stack[0] == 0 )
				this->pCall->cPC = ccmd.cParam;
			this->pCall->cStack.strip(1);
			break;
		}
		case OP_IF:		// if '(' <Expr> ')' <Normal Stm>
		{
			if( stack[0] != 0 )
				this->pCall->cPC = ccmd.cParam;
			this->pCall->cStack.strip(1);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// conditional branch or Pop
		case OP_NIF_POP:
		{
			if( stack[0] == 0 )
				this->pCall->cPC = ccmd.cParam;
			else
				this->pCall->cStack.strip(1);
			break;
		}
		case OP_IF_POP:		// if '(' <Expr> ')' <Normal Stm>
		{
			if( stack[0] != 0 )
				this->pCall->cPC = ccmd.cParam;
			else
				this->pCall->cStack.strip(1);
			break;
		}
		/////////////////////////////////////////////////////////////////
		// unconditional branch
		// add the branch offset to the programm counter
		case OP_GOTO:	// goto position
		{
			this->pCall->cPC = ccmd.cParam;
			break;
		}
		/////////////////////////////////////////////////////////////////
		// clear element content
		case OP_EMPTY:
		{
			stack[0].make_empty();
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
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().clear();
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
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().evaluate();
			this->pCall->cStack.last() = ccmd.cCommand-OP_PUSH_ZERO;
			break;
		}
		case OP_PUSH_INT:	// followed by an integer
		{
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().evaluate();
			this->pCall->cStack.last() = ccmd.cParam;
			break;
		}
		case OP_PUSH_STRING:	// followed by a string address
		{
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().evaluate();
			this->pCall->cStack.last() = ccmd.cString;
			break;
		}
		case OP_PUSH_FLOAT:	// followed by a float
		{
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().evaluate();
			this->pCall->cStack.last() = scriptprog::int2float(ccmd.cParam);
			break;
		}
		case OP_PUSH_VAR:	// followed by a string containing a variable name
		{
			this->get_variable(ccmd.cString, false);
			break;			
		}
		case OP_PUSH_VAL:
		{
			this->get_variable(ccmd.cString, true);	
			break;			
		}
		case OP_PUSH_PARAVAR:
		{	// push a reference
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().clear();
			this->pCall->cStack.last() = ((size_t)ccmd.cParam<this->pCall->cPara.size())?this->pCall->cPara[ccmd.cParam]:basics::variant();
			break;
		}
		case OP_PUSH_PARAVAL:
		{	// push a value
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().clear();
			this->pCall->cStack.last() = ((size_t)ccmd.cParam<this->pCall->cPara.size())?this->pCall->cPara[ccmd.cParam]:basics::variant();
			this->pCall->cStack.last().evaluate();
			break;
		}
		case OP_PUSH_TEMPVAR:
		{	// push a reference
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().clear();
			this->pCall->cStack.last() = ((size_t)ccmd.cParam<this->pCall->cTemp.size())?this->pCall->cTemp[ccmd.cParam]:basics::variant();
			break;
		}
		case OP_PUSH_TEMPVAL:
		{	// push a value
			this->pCall->cStack.resize(this->pCall->cStack.size()+1);
			this->pCall->cStack.last().clear();
			this->pCall->cStack.last() = ((size_t)ccmd.cParam<this->pCall->cTemp.size())?this->pCall->cTemp[ccmd.cParam]:basics::variant();
			this->pCall->cStack.last().evaluate();
			break;
		}
		case OP_POP:	// decrements the stack
		{	
			this->pCall->cStack.resize(this->pCall->cCC);
			this->clear_externals();
			break;
		}
		case OP_EVAL:
		{	//evaluate
			stack[0].evaluate();
			break;
		}
		case OP_REDUCE:
		{	//reduce
			const int cnt = ccmd.cParam;
			basics::variant temp;
			if( stacksize < cnt )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", cnt, (int)stacksize);
				return OFFLINE;
			}
			if(cnt>1)
			{
				stack[1-cnt] = stack[0];
				this->pCall->cStack.strip(cnt-1);
			}
			break;
		}		
		case OP_BOOLEAN:
		{	//evaluate
			stack[0].evaluate();
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
			if( is_frame_limit() )
				return OFFLINE;
			const ssize_t cnt = stack[0].get_int();
			const ssize_t ofs = 1+cnt;
			if( stacksize < ofs )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", (int)ofs, (int)stacksize);
				return OFFLINE;
			}
			if( ( ccmd.cCommand!=OP_BLDFUNCTION || !this->call_buildin(ccmd.cString, ofs, cnt) ) && 
				!this->call_function(ccmd.cString, "main", ofs, cnt) )
			{
				fprintf(stderr, "function '%s' not found assuming zero return\n", ccmd.cString);
				this->pCall->cStack.strip(ofs);
				// push the return value
				this->pCall->cStack.resize(this->pCall->cStack.size()+1);
				this->pCall->cStack.last().evaluate();
				this->pCall->cStack.last() = 0;
			}
			break;
		}
		case OP_SUBFUNCTION:
		{	// save the current stack/programm/counters
			// prepare a new stack
			// get the programm and the function entry
			// set initial programm counter
			if( is_frame_limit() )
				return OFFLINE;
			const ssize_t cnt = stack[0].get_int();
			const ssize_t ofs = 2+cnt;
			if( stacksize < ofs )
			{	// something wrong
				fprintf(stderr, "stack underflow, %i values requited, have %i\n", (int)ofs, (int)stacksize);
				return OFFLINE;
			}
			if( !this->call_function(ccmd.cString, stack[-1].get_string(), ofs, cnt) )
			{
				fprintf(stderr, "function '%s' not found assuming zero return\n", ccmd.cString);
				this->pCall->cStack.strip(ofs);
				// push the return value
				this->pCall->cStack.resize(this->pCall->cStack.size()+1);
				this->pCall->cStack.last().evaluate();
				this->pCall->cStack.last() = 0;
			}
			break;
		}
		case OP_GOSUB:
		{	
			if( is_frame_limit() )
				return OFFLINE;
			this->call_gosub(ccmd.cParam);
			break;
		}
		case OP_RETURN:
		{	// restore the previous stack/programm/counters
			// if stacksize>0
			if( return_script(stack[0]) )
				break;
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
	return OFFLINE;
}

///////////////////////////////////////////////////////////////////////////
/// start a programm
bool CStackEngine::start(const scriptprog::script& prog, const basics::string<>& startlabel)
{
	if( prog.exists() )
	{
		// prepare new run parameter
		callframe* newcall = new callframe;
		newcall->cProg = prog;
		newcall->cDecl = prog->get_declaration("main");
		newcall->cPC   = prog->get_labeltarget(startlabel);
		newcall->cCC   = 0;
		newcall->cTemp.resize(newcall->cDecl.cVarCnt);
		newcall->cStack.resize(newcall->cCC);
		
		this->cMtx.lock();

		if( this->is_offline() )
		{	// run the programm
			this->pCall = newcall;
			this->run_script();
		}
		else
		{	// queue the programm
			this->cCallQueue.append(newcall);
		}
		
		this->cMtx.unlock();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
/// check if can further increase stackframe.
bool CStackEngine::is_frame_limit() const
{
	if( this->cCallStack.size() < max_frame )
		return false;
	fprintf(stderr, "max stackframe reached, abort execution\n");
	return true;
}
///////////////////////////////////////////////////////////////////////////
/// run the currently selected script.
/// only call with locked mutex and placed cCall
bool CStackEngine::run_script()
{
	for(;;)
	{
		this->cState = RUNNING;
		// run the currently selected script
		this->cMtx.unlock();
		const state_t ret = this->process();
		this->cMtx.lock();

		if( ret == PAUSED )
		{	// pause the current script
			this->cState = PAUSED;
			// break execution, need to wait for the continue
			break;
		}
		// otherwise the current program has been terminated
		delete this->pCall;
		this->pCall=NULL;

		if( this->cCallStack.size()>0 )
		{	// select the next script in queue
			this->pCall = this->cCallStack[0];
			this->cCallStack.removeindex(0);
		}
		else
		{	// no further script queued, can finish
			this->cState = OFFLINE;
			break;
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////
/// coming back from a callback with return value
bool CStackEngine::cont(const basics::variant& retvalue)
{
	bool ret = false;
	this->cMtx.lock();
	if( this->is_paused() )
	{	// push the return value
		this->pCall->cStack.resize(this->pCall->cStack.size()+1);
		this->pCall->cStack.last().evaluate();
		this->pCall->cStack.last() = *retvalue;

		run_script();

		ret = true;
	}
	else
	{	// error
		fprintf(stderr, "continue nonpaused script\n");

		// otherwise the current program has been terminated
		if( this->pCall )
		{
			delete this->pCall;
			this->pCall=NULL;
		}
		if( this->cCallStack.size()>0 )
		{	// select the next script in queue
			this->pCall = this->cCallStack[0];
			this->cCallStack.removeindex(0);
			this->run_script();
		}
		else
		{	// no further script queued, can finish
			this->cState = OFFLINE;
		}
	}
	this->cMtx.unlock();
	return ret;
}



///////////////////////////////////////////////////////////////////////////
/// gosub to a label inside the current script.
bool CStackEngine::call_gosub(size_t startpos)
{
	this->cMtx.lock();
	// prepare new run parameter
	callframe* newcall = new callframe(*this->pCall);
	// set the start position
	newcall->cPC = startpos;

	// save the run parameter
	this->pCall->cStack.append( 0 );
	this->cCallStack.append(this->pCall);

	// set the new run parameter
	this->pCall = newcall;
	this->cMtx.unlock();
	return true;
}

///////////////////////////////////////////////////////////////////////////
/// calls a script function from the current script (function call)
bool CStackEngine::call_buildin(const basics::string<>& name, const uint paramstart, const uint paramcnt)
{
	const buildin::declaration* ptr = buildin::get(name);
	if( ptr && ptr->function )
	{
		basics::variant retvalue = ptr->function( callparameter(*this, this->pCall->cStack.end()-paramstart, paramcnt) );

		this->pCall->cStack.strip(paramstart);
		// push the return value
		this->pCall->cStack.resize(this->pCall->cStack.size()+1);
		this->pCall->cStack.last().evaluate();
		this->pCall->cStack.last() = *retvalue;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
/// calls a script function from the current script (function call)
bool CStackEngine::call_function(const basics::string<>& name, const basics::string<>& entry, const uint paramstart, const uint paramcnt)
{
	scriptprog::script prog = scriptprog::get_script(name);
	if( prog.exists() )
	{
		// prepare new run parameter
		callframe* newcall = new callframe;
		newcall->cProg = prog;
		newcall->cDecl = prog->get_declaration(entry);
		newcall->cPC   = this->pCall->cDecl.cEntry;
		newcall->cCC   = 0;
		// copy parameters
		{
			size_t sz;
			newcall->cPara.clear();
			newcall->cPara.append( this->pCall->cStack.end()-paramstart, paramcnt );
			for(sz=0; sz<paramcnt; ++sz)
			{	// set refernce mode for non-const parameters
				if( newcall->cDecl.cParam[sz].cConst )
					newcall->cPara[sz].evaluate();
			}
			if( newcall->cDecl.cParam.size() > paramcnt )
			{	// add missing default parameter
				sz= newcall->cDecl.cParam.size()-paramcnt;
				const scriptdecl::parameter* sp = newcall->cDecl.cParam.begin()+paramcnt;
				for(; sz; --sz, ++sp)
				{
					newcall->cPara.append( sp->cValue );
				}
			}
		}
		newcall->cTemp.resize(newcall->cDecl.cVarCnt);
		newcall->cStack.resize(newcall->cCC);

		// save the run parameter
		this->cMtx.lock();
		this->pCall->cStack.append( paramstart );
		this->cCallStack.append(this->pCall);
		this->cMtx.unlock();

		// set the new run parameter
		this->pCall = newcall;
		return true;
	}
	else
	{	// try buildin function, ignore entry here
		return this->call_buildin(name, paramstart, paramcnt);
	}
}

///////////////////////////////////////////////////////////////////////////
/// return from a script
bool CStackEngine::return_script(basics::variant retvalue)
{
	if( this->cCallStack.size() )
	{	// restore the run parameter
		if( this->cDebugMode )
		{
			printf("-------\n");
			printf("return from function %s\n", this->pCall->cProg->cName.c_str());
			printf("return value: %s - %s\n", basics::variant::type2name(retvalue.type()), basics::tostring(retvalue).c_str());
			printf("-------\n");
		}

		this->cMtx.lock();
		delete this->pCall;
		this->pCall = this->cCallStack.last();
		this->cCallStack.strip(1);
		this->cMtx.unlock();

		size_t sz = 1+this->pCall->cStack.last().get_int();
		this->pCall->cStack.strip(sz);

		// push the return value
		this->pCall->cStack.resize(this->pCall->cStack.size()+1);
		this->pCall->cStack.last().evaluate();
		this->pCall->cStack.last() = *retvalue;
		return true;
	}
	else
	{	// return from the first level
		if( this->cDebugMode )
		{
			printf("return from the first level\n");
		}
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////
/// clear the engine
void CStackEngine::clear()
{
	this->cMtx.lock();
	this->clear_externals();
	this->clear_stack();

	if( this->cCallQueue.size() )
	{
		size_t i;
		for(i=0; i<this->cCallQueue.size(); ++i)
		{
			delete this->cCallQueue[i];
			this->cCallQueue[i]=NULL;
		}
		this->cCallQueue.clear();
	}
	this->cMtx.unlock();
}
///////////////////////////////////////////////////////////////////////////
/// clear external variables.
void CStackEngine::clear_externals()
{
	external_map_t::iterator iter(this->cExternal);
	for(; iter; ++iter)
	{
		this->sGlobalVariables.set_variable(iter->key, iter->data);
	}
	this->cExternal.clear();
}
///////////////////////////////////////////////////////////////////////////
/// clear current callstack only.
void CStackEngine::clear_stack()
{
	this->cMtx.lock();
	if(this->pCall)
	{
		delete this->pCall;
		this->pCall=NULL;
	}
	if( this->cCallStack.size() )
	{
		size_t i;
		for(i=0; i<this->cCallStack.size(); ++i)
		{
			delete this->cCallStack[i];
			this->cCallStack[i]=NULL;
		}
		this->cCallStack.clear();
	}
	this->cMtx.unlock();
}

///////////////////////////////////////////////////////////////////////////
/// 
void CStackEngine::self_test(const char* name, bool debug)
{
	CStackEngine e(debug);
	const char*pp = strstr(name,"::");
	basics::string<> progname = pp?basics::string<>(name, pp-name):basics::string<>(name);
	if(pp)
		pp+=2;
	else
		pp= "main";

	
	fprintf(stdout,"\n\n");

	scriptprog::script prog = scriptprog::get_script(progname);
	if( prog.exists() )
	{
		fprintf(stdout, "define variables (<name>=<value>, empty input to finish)\n");
		char buffer[1024];
		while( fgets(buffer, sizeof(buffer), stdin) )
		{
			buffer[sizeof(buffer)-1] = 0;
			const char* ip = strchr(buffer, '=');
			if( ip )
			{
				basics::string<> name(buffer, ip);
				basics::string<> value(ip+1);
				name.trim(), value.trim();
				tempvalues.insert(name, value);
			}
			else
				break;
		}

		fprintf(stdout, "executing programm '%s':\n", name);
		e.start(prog, pp);
		fprintf(stdout, "\nprogramm '%s' finished\n", name);
	}
	else
	{
		fprintf(stderr, "programm '%s' not found\n", name);
	}

	{
		basics::map<basics::string<>, basics::variant>::iterator iter(tempvalues);
		for(;iter; ++iter)
		{
			fprintf(stdout, "var '%s' (%s) = %s\n", 
				iter->key.c_str(),
				basics::variant::type2name(iter->data.type()),
				basics::tostring(iter->data).c_str() );
		}
	}
}



