// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////


#include "eaparser.h"
#include "eacompiler.h"
#include "eabuildin.h"
#include "eastorage.h"
#include "scriptengine.h"


///////////////////////////////////////////////////////////////////////////////
// compiler

eacompiler::eacompiler()
	: funcdecl(NULL)
{
	ulong sz;
	const unsigned char *e = getEngine(sz);
	if( !e )
	{
		fprintf(stderr, "Error creating parser\n");
	}
	else if( !this->cParseConfig.create(e, sz) )
	{
		fprintf(stderr, "Could not load engine\n");
	}

	// set some default defines
	this->cDefines_.insert("north",0);
	this->cDefines_.insert("northeast",1);
	this->cDefines_.insert("east",2);
	this->cDefines_.insert("southeast",3);
	this->cDefines_.insert("south",4);
	this->cDefines_.insert("southwest",5);
	this->cDefines_.insert("west",6);
	this->cDefines_.insert("northwest",7);
}

eacompiler::~eacompiler()
{
}



///////////////////////////////////////////////////////////////////////////
// support functions
void eacompiler::replace_jumps(size_t startaddr, size_t with)
{	// replace with real target address
	// go recusively through all connected addresses
	size_t hlp;
	while( startaddr!= 0 )
	{
		hlp = startaddr;					// save the address since it is moving
		hlp = prog->getAddr(hlp);			// read the next address in the chain
		prog->replaceAddr(with, startaddr);	// update the address
		startaddr = hlp;					// go on with the next
	}
}


// defines

///////////////////////////////////////////////////////////////////////////
// define functions
bool eacompiler::is_defined(const basics::string<>& name) const
{
	return this->cDefines_.exists(name);
}
bool eacompiler::is_defined(const basics::string<>& name, basics::variant& def) const
{	// check for define and return the value
	if( this->cDefines_.exists(name) )
		return def = this->cDefines_.get(name), true;
	return false;
}
basics::variant eacompiler::get_define(const basics::string<>& name) const
{
	if( this->cDefines_.exists(name) )
		return this->cDefines_.get(name);
	return basics::variant();
}

bool eacompiler::create_label(const basics::string<>& name, uint line)
{
	CLabelPos& label = this->cLabels[name];
	if( !label.valid )
	{	// correct the existing labels
		size_t newpos = prog->getCurrentPosition();
		this->replace_jumps(label.pos, newpos);
		// set the entry
		label.pos = newpos;
		label.valid = true;
		label.createline = line;

		// register the label
		//if( name[0]=='O' || name[1]=='n' )
		{	// register 'On...'-labels
			prog->create_label(name, newpos);
		}
		return true;
	}
	this->warning("multiple definition of label: %s (line %u)\n", name.c_str(), line);
	return false;
}

bool eacompiler::check_labels()
{
	int ret=0;
	basics::smap<basics::string<>,CLabelPos>::iterator iter(this->cLabels);
	for(; iter; ++iter)
	{
		if( !iter->data.valid )
		{
			this->warning("undefined label for 'goto %s' (line %u)\n", (const char*)iter->key, iter->data.useline);
			++ret;
		}
		else if( !iter->data.use && (iter->key[0]!='O' || iter->key[1]!='n') )
		{
			this->warning("unused label '%s' (line %u)\n", (const char*)iter->key, iter->data.createline);
		}
	}
	return (ret==0);
}








bool eacompiler::compile_define(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'define' identifier '=' <Op If> ';'
	const size_t i = cConstvalues.size();
	const bool accept = compile_main(node[3], scope, flags | CFLAG_CONST, uservalue);
	if( accept )
	{
		this->cDefines_.insert( node[1].string(), cConstvalues.last() );
		if( flags&CFLAG_DECL )
			this->cFile->definitions.insert( node[1].string(), cConstvalues.last() );
	}
	cConstvalues.resize(i);
	return accept;
}

///////////////////////////////////////////////////////////////////////////
// include functions
bool eacompiler::compile_include(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{
	basics::string<> name = basics::create_filespec(basics::folder_part(this->cFile->c_str()), basics::string<>(node[1].string().c_str()+1,node[1].string().size()-2));
	if( !this->compile_file(name, this->cCompileOptions) )
	{
		this->warning("included in '%s' at line %u\n", this->cFile->c_str(), node[1].line());
	}
	else
	{	// connect the two files
		scriptfile::scriptfile_ptr ptr = scriptfile::get_scriptfile(name);
		if( ptr.exists() )
		{
			this->cFile->parents.push(name);
			ptr->childs.push(*this->cFile);
			// get the defines from the parent file
			ptr->get_defines(this->cDefines_);
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
// label functions
bool eacompiler::compile_label(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// identifier ':'
	return create_label(node[0].string(), node.line());
}
bool eacompiler::compile_goto(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'goto' identifier ';'
	// jump inside the current programm
	// does not alter stack nor the current programm
	const basics::string<>& name = node[1].string();
	CLabelPos& label = this->cLabels[name];
	++label.use;
	label.useline = node.line();
	if( label.valid )
	{
		this->put_varcommand(OP_GOTO, label.pos);
	}
	else
	{	// default size
		this->put_command(OP_GOTO3);
		label.pos = prog->appendAddr( label.pos );
	}
	return true;
}

bool eacompiler::compile_gosub(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'gosub' identifier ';'
	// pushes current programm to program stack and jumps to location (internal/external)
	// does not alter the stack
	const basics::string<>& name = node[1].string();
	CLabelPos& label = this->cLabels[name];
	++label.use;
	label.useline = node.line();
	if( label.valid )
	{
		this->put_varcommand(OP_GOSUB, label.pos);
	}
	else
	{	// default size
		this->put_command(OP_GOSUB3);
		label.pos = prog->appendAddr( label.pos );
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
// script variables
bool eacompiler::compile_vardecl(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <VarType> <VarList>  ';'
	// 'string'
    // 'double'
    // 'int'
    // 'auto'
    // 'var'
	// process the type
	const basics::string<>& type = node[0].string();
	if( type == "string" )
		flags |= CFLAG_VARSTRING;
	else if( type == "float" )
		flags |= CFLAG_VARFLOAT;
	else if( type == "int" )
		flags |= CFLAG_VARINT;
	else  if( type != "var" && type != "auto" )
	{	// not recognized
		this->warning("unrecognized type in variable declararation (line %u)\n", node.line());
		return false;
	}
	// compile the var list
	return compile_main(node[1], scope, flags, uservalue);
}

bool eacompiler::compile_variable(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Variable>  ::= <Op Pointer> <VarAssign>
	// <VarAssign> ::= '=' <Expr> or empty
	const parse_node varname = node[0];
	const parse_node &varassign = node[1];
	const bool has_assign = 0!=varassign.childs();
	bool accept = false;
	
	// <Array> := <Op Pointer> '[' <Range> ']'
//	if( varname.symbol() == PT_ARRAY )
//	{
//	}
// ignore the array for the moment

	if( varname.symbol() != PT_SCOPEVAR && varname.symbol() != PT_ARRAY &&varname.symbol() != PT_IDENTIFIER )
	{
		this->warning("variable type '%s' is not allowed (line %u)\n", varname.name(), node.line());
	}
	else
	{
		// varassign not empty -> '=' <Expr>
		const int varflags = (flags&~(CFLAG_RVALUE|CFLAG_ASSIGN))|CFLAG_VARCREATE|(has_assign?(CFLAG_ASSIGN|CFLAG_LVALUE):(CFLAG_NOASSIGN|CFLAG_VARSILENT));
		accept= (!has_assign || compile_main(varassign[1], scope, (flags&~CFLAG_LVALUE)|CFLAG_RVALUE, uservalue)) &&
				compile_main(varname, scope, varflags, uservalue);
		if( accept && has_assign )
		{	//## TODO: add check for proper dimensions of assignment values
			this->put_command( (varname.symbol()!=PT_ARRAY)?OP_ASSIGN:OP_ARRAY_ASSIGN );
			this->put_command(OP_POP);
		}
	}
	return accept;
}
///////////////////////////////////////////////////////////////////////////
// compare function declaration
bool eacompiler::compare_declarations(const parse_node &namenode, const scriptdecl&a, const scriptdecl& b, const bool b_has_values)
{
	if( a.cReturn != b.cReturn )
	{
		this->warning("function '%s' redefined with different return type (line %u)\n", (const char*)namenode.string(), namenode.line());
		return false;
	}
	if(a.cParam.size()!=b.cParam.size())
	{
		this->warning("function '%s' redefined with different number of arguments (line %u)\n", (const char*)namenode.string(), namenode.line());
		return false;
	}
	{
		basics::vector<scriptdecl::parameter>::iterator a1(a.cParam);
		basics::vector<scriptdecl::parameter>::iterator a2(b.cParam);
		for(; a1; ++a1,++a2)
		{
			if( a1->cType != a2->cType )
			{
				this->warning("function '%s' redefined with different parameter types (line %u)\n", (const char*)namenode.string(), namenode.line());
				this->warning("was %s, redefined as %s\n", basics::variant::type2name(a1->cType), basics::variant::type2name(a2->cType));
				return false;
			}
			if( a1->cConst != a2->cConst )
			{
				this->warning("function '%s' redefined with different access type (line %u)\n", (const char*)namenode.string(), namenode.line());
				this->warning(a1->cConst?"was const, redefined as non-const\n":"was non-const, redefined as const\n");
				return false;
			}
			if( b_has_values )
			{	// a and b have default values, so check for equivalence
				if( a1->cValue != a2->cValue )
				{
					this->warning("function '%s' redefined with different default value (line %u)\n", (const char*)namenode.string(), namenode.line());
					this->warning("was %s, redefined as %s\n", a1->cValue.get_arraystring().c_str(), a2->cValue.get_arraystring().c_str());
					return false;
				}
			}
			else
			{	// only a has default values, b has to be empty
				if( !a2->cValue.is_empty() )
				{
					this->warning("function '%s', definition of default value only allowed at predeclaration (line %u)\n", (const char*)namenode.string(), namenode.line());
					return false;
				}
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
// function declaration
bool eacompiler::compile_funcdecl(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <VarType> identifier '(' <Paramse> ')' ';'
	// <VarType> identifier '(' <Paramse> ')' <Block>

	const parse_node &namenode = node[1];

	// check for a possible existing declaration
	scriptprog::script temp  = scriptprog::get_script( namenode.string() );
	if( temp.exists() && temp->cName.size() )
	{
		if( temp->size() )
		{	// redefinition
			this->warning("function '%s' already defined (line %u)\nignoring additional definition.\n", (const char*)namenode.string(), namenode.line());
			// return true here as we want to skip the whole thing
			return true;
		}
		// otherwise there is a predeclaration only
	}
	else
	{
		temp.clear();
	}

	prog.clear();
	inst.clear();
	cVariable.clear();
	cStrTable.clear();
	cLabels.clear();
	cConstvalues.clear();
	scriptdefines tmpdefines = this->cDefines_;
	
	prog->cName = node[1].string();
	const basics::string<> main_node("main");
	this->funcdecl = prog->cHeader.search(main_node);
	if( !this->funcdecl )
	{	// create new
		prog->cHeader[main_node].cName = main_node;
		this->funcdecl = prog->cHeader.search(main_node);
		this->funcdecl->cScript = &(*prog);
	}
	this->funcdecl->cReturn = basics::variant::name2type(node[0].string());

	// parameter
	const parse_node &param = node[3];
	if( param.symbol()!=PT_PARAMSE && !compile_main(param, scope, 0, uservalue) )
		return false;

	if( temp.exists() )
	{	// compare with the predeclaration
		scriptdecl decl = temp->get_declaration(main_node);
		if( !compare_declarations(namenode, decl, *this->funcdecl, true) )
			return false;
		// and delete the temporary, so it does unregister itself
		temp.clear();
	}

	
	
	bool accept;
	const parse_node &body = node[5];
	if( body.symbol()==PT_BLOCK )
	{	// '{' <Stm List> '}'
		// object with attached script
		// setup
		prog->appendCommand(OP_START);
		size_t sizepos=prog->appendInt(0);

		// compile
		accept = compile_main(body[1], scope, 0, uservalue) &&
				 check_labels() && check_variable(scope);
		// cleanup
		if( prog->cProgramm.last() != OP_END )
			this->put_command(OP_END);
		prog->replaceInt(prog->getCurrentPosition(), sizepos);
	}
	else
	{	// declaration only
		accept = true;
	}

	///////////////////////////////////////////////////////////////////
	this->funcdecl = NULL;
	// register script
	if(accept)
	{
		prog->unregist();
		accept = scriptprog::regist(prog, node.line());
		if( accept )
		{
			if( cCompileOptions&OPT_COMPILEOUTPUT )
				prog->dump();
			this->cFile->scripts.push_back(prog->cName);
		}
	}

	prog.clear();
	inst.clear();
	cVariable.clear();
	cStrTable.clear();
	cLabels.clear();
	cConstvalues.clear();
	this->cDefines_ = tmpdefines;

	return accept;
}
///////////////////////////////////////////////////////////////////////////
// subfunction declaration
bool eacompiler::compile_subfuncdecl(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'function' <Func Decl>
	// <VarType> identifier '(' <Paramse> ')' ';'
	// <VarType> identifier '(' <Paramse> ')' <Block>
	const parse_node &funcnode = node[1];
	const parse_node &returnval = funcnode[0];
	const parse_node &name = funcnode[1];
	const parse_node &parameter = funcnode[3];
	const parse_node &body = funcnode[5];
	const bool declare_only = (body.symbol()==PT_SEMI);
	scriptdecl* save = this->funcdecl;
	basics::string<> savename;
	if(this->funcdecl) savename = this->funcdecl->cName; // name of the parent

	bool has_predecl = false;
	scriptdecl predecl;

	// check conflicting function name
	this->funcdecl = prog->cHeader.search(name.string());
	if( !this->funcdecl )
	{	// create new
		prog->cHeader[name.string()].cName = name.string();
		this->funcdecl = prog->cHeader.search(name.string());
		this->funcdecl->cScript = &(*prog);
	}
	else if( name.string()=="main" || declare_only || this->funcdecl->cEntry )
	{	// redeclaration
		this->warning("redeclaration of function '%s' (line %u)\n", (const char*)name.string(), name.line());
		this->funcdecl = save;
		return false;
	}
	else
	{
		has_predecl = true;
		predecl = *this->funcdecl;
		this->funcdecl->cParam.clear();
	}
	this->funcdecl->cReturn = basics::variant::name2type(returnval.string());
	// parameter
	uservalue = 0;

	scriptdefines  tempdefines   = cDefines_;
	CVariableScope tempvariables = cVariable;
	cVariable.clear();

	// compile function parameters
	bool accept = compile_main(parameter, scope, 0, uservalue);

	if( has_predecl && !compare_declarations(name, predecl, *this->funcdecl, false) )
		return false;
	
	// compile body if exists
	if( accept && !declare_only )
	{
		this->put_command(OP_GOTO3);
		size_t inspos1 = prog->appendAddr(0);	// placeholder
		this->funcdecl->cEntry = prog->getCurrentPosition();

		accept = compile_main(body[1], scope, 0, uservalue);

		if( prog->cProgramm.last() != OP_RETURN )
			this->put_command(OP_RETURN);
		prog->replaceAddr( prog->getCurrentPosition() ,inspos1);
	}
	if(save)
		this->funcdecl = prog->cHeader.search(savename);

	cVariable = tempvariables;
	cDefines_ = tempdefines;
	return accept;
}
bool eacompiler::compile_parameter(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <constopt> <VarTypeopt> identifier
	// <constopt> <VarTypeopt> identifier '=' <Const Unary>
	// ['const']  ['string']
    // ['const']  ['double']
    // ['const']  ['int']
    // ['const']  ['auto']
    // ['const']  ['var']
	if( !this->funcdecl )
	{
		this->warning("no valid declaration (line %u)\n", node.line());
	}
	else if( exists_variable(node[2].string(), CFLAG_PARAM) )
	{
		this->warning("parameter '%s' already exists (line %u)\n", node[2].string().c_str(), node.line());
	}
	else
	{
		basics::variant paramvalue;
		const bool isconst = (node[0].symbol()==PT_CONST);
		const basics::var_t type = basics::variant::name2type(node[1].string());
		if(isconst) paramvalue.make_value(); else paramvalue.make_reference();
		if( node.childs() == 5 )
		{	// do the assignment
			if( !compile_main(node[4], scope, CFLAG_CONST, uservalue) )
				return false;
			if( this->cConstvalues.size() )
			{
				paramvalue = this->cConstvalues.last();
				this->cConstvalues.clear();

			}
			paramvalue.cast(type);
		}
		this->funcdecl->cParam.push(scriptdecl::parameter(type, isconst, paramvalue));
		return create_variable(node[2], node[2].string(), CFLAG_PARAM, scope, type, isconst, true);
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////
// function call
bool eacompiler::build_function_parameter(const scriptdecl& decl, const parse_node &node, uint scope, int& paramcount)
{	// run flat through the parameter list
	bool accept = true;
	parse_node parameter = node;
	parse_node worknode;
	size_t count=0;
	bool run=true;
	for(;accept&&run ; ++count)
	{	
		if( parameter.symbol() == PT_EXPRLIST )
		{	// <ExprList> = <Expr> ',' <ExprList>
			worknode  = parameter[0];
			parameter = parameter[2];
		}
		else
		{
			worknode = parameter;
			run = false;
		}
		
		const bool has_param = (count < decl.cParam.size());
		basics::var_t t = has_param ? decl.cParam[count].cType : basics::VAR_AUTO;
		if( worknode.symbol() == PT_EXPR )
		{	// empty, put default value
			this->put_value( has_param ? decl.cParam[count].cValue : basics::variant() );
		}
		else
		{	// something else, compile it
			const unsigned long myflags = (!has_param || decl.cParam[count].cConst)?CFLAG_LVALUE|CFLAG_RVALUE:CFLAG_RVALUE;
			int uservalue=0;
			accept = compile_main(worknode, scope, myflags, uservalue);
		}
		// do a cast when asked
		if(t==basics::VAR_FLOAT)
		{
			this->put_command(OP_CAST_FLOAT);
		}
		else if(t==basics::VAR_INTEGER)
		{
			this->put_command(OP_CAST_INTEGER);
		}
		else if(t==basics::VAR_STRING)
		{
			this->put_command(OP_CAST_STRING);
		}
	};
	// fill remaining parameter
	for(; accept && count<decl.cParam.size(); ++count)
	{	
		basics::var_t t = decl.cParam[count].cType;
		if( t!=basics::VAR_NONE )
		{	// put default value
			this->put_value( decl.cParam[count].cValue );
			// do the cast
			if(t==basics::VAR_FLOAT)
			{
				this->put_command(OP_CAST_FLOAT);
			}
			else if(t==basics::VAR_INTEGER)
			{
				this->put_command(OP_CAST_INTEGER);
			}
			else if(t==basics::VAR_STRING)
			{
				this->put_command(OP_CAST_STRING);
			}
		}
	}
	paramcount = count;
	return accept;
}

bool eacompiler::put_function_call(const parse_node &node, uint scope, const basics::string<>& name, bool membercall, bool global)
{
	scriptdecl decl;
	if( !global )
	{	//check for local subfunction
		decl = prog->get_declaration(name);
		if( !decl.cScript )
			global = true;
	}

	if( global )
	{	//check for global script function
		scriptprog::script temp  = scriptprog::get_script( name );
		if( temp.exists() )
		{
			decl = temp->get_declaration("main");
		}
		// check if function name exists on buildins
		else if( !buildin::exists(name) )
		{	// function name not found
			this->warning("function '%s' undefined, call will abort at runtime (line %u)\n", name.c_str(), node.line());
			// but accept it
		}
	}

	int paramcount;
	if( build_function_parameter(decl, node[2], scope, paramcount) )
	{
		if(membercall)
			++paramcount;

		this->put_nonconst();
		this->put_intcommand(OP_PUSH_INT, paramcount);

		if( global )
		{
			this->put_strcommand(OP_FUNCTION, name);
		}
		else
		{
			this->put_strcommand(OP_PUSH_STRING, prog->cName);
			this->put_strcommand(OP_SUBFUNCTION, name);
		}
		return true;
	}
	return false;
/*

	this->put_nonconst();
	if( !global )
	{	//check for local subfunction
		scriptdecl decl = prog->get_declaration(name);
		if( decl.cScript )
		{
			this->put_intcommand(OP_PUSH_INT, paramcnt);
			this->put_strcommand(OP_PUSH_STRING, prog->cName);
			this->put_strcommand(OP_SUBFUNCTION, name);
			return true;
		}
	}
	// check if function name exists
	if( !buildin::exists(name) )
	{	// function name not found
		this->warning("function '%s' undefined, call will abort at runtime (line %u)\n", name.c_str(), node.line());
		// but accept it
	}
	// some workaround to reuse a function "regex" to handle the regexliteral
	if(name=="regex" && paramcnt==2)
	{	// reserve a temp variable
		const basics::string<> varname("$re");
		if( !this->cVariable[varname].exists(CFLAG_TEMP) )
		{
			this->create_variable(node, varname, CFLAG_TEMP, scope, basics::VAR_AUTO, false, true);
			this->cVariable[varname][CFLAG_TEMP]->luse = 1;
			this->cVariable[varname][CFLAG_TEMP]->ruse = 2;
		}
		if( this->put_knownvariable(node, varname, CFLAG_LVALUE, 0, CFLAG_TEMP) )
		{
			this->put_command(OP_EMPTY);
			++paramcnt;
		}
	}
	this->put_intcommand(OP_PUSH_INT, paramcnt);
	this->put_strcommand(OP_FUNCTION, name);
	return true;
*/
}

bool eacompiler::put_subfunction_call(const parse_node &node, uint scope, const basics::string<>& host, const basics::string<>& name, bool membercall)
{
	scriptdecl decl;
	int paramcount;

	if( host == "buildin" )
	{	// forced usage of buildin's
		if( !buildin::exists(name) )
		{	// function name not found
			this->warning("buildin function '%s' not defined (line %u)\n", name.c_str(), node.line());
		}
		else if( build_function_parameter(decl, node, scope, paramcount) )
		{
			if(membercall)
				++paramcount;

			this->put_nonconst();
			this->put_intcommand(OP_PUSH_INT, paramcount);
			this->put_strcommand(OP_BLDFUNCTION, name);
			return true;
		}
	}
	else if( host == "player" )
	{	// some player member function, which actually is a readonly variable access
		return put_knownvariable(node, name, CFLAG_RVALUE|CFLAG_VARCREATE|CFLAG_NOASSIGN, scope, CFLAG_PERM|CFLAG_PLY);
	}
	else
	{	// call subfunction in another script
		scriptprog::script scr = scriptprog::get_script(host);
		if( !scr.exists() )
		{
			this->warning("script '%s' is not defined (line %u)\n", (const char*)host, node.line());
		}
		else
		{
			decl = scr->get_declaration(name);
			if( !decl.cScript )
			{	// subfunction name not found
				this->warning("subfunction '%s' in script '%s' is not defined (line %u)\n", name.c_str(), host.c_str(), node.line());
			}
			else if( build_function_parameter(decl, node, scope, paramcount) )
			{
				if(membercall)
					++paramcount;				
				this->put_intcommand(OP_PUSH_INT, paramcount);
				this->put_strcommand(OP_PUSH_STRING, host);
				this->put_strcommand(OP_SUBFUNCTION, name);
				return true;
			}
		}
	}
	return false;

/*
	this->put_nonconst();
	if( host == "buildin" )
	{	// forced usage of buildin's
		if( !buildin::exists(name) )
		{	// function name not found
			this->warning("buildin function '%s' not defined (line %u)\n", name.c_str(), node.line());
		}
		else
		{
			this->put_intcommand(OP_PUSH_INT, paramcnt);
			this->put_strcommand(OP_BLDFUNCTION, name);
			return true;
		}
	}
	else if( host == "player" )
	{	// some player member function, which actually is a readonly variable access
		return put_knownvariable(node, name, CFLAG_RVALUE|CFLAG_VARCREATE|CFLAG_NOASSIGN, scope, CFLAG_PERM|CFLAG_PLY);
	}
	else
	{	// call subfunction in another script
		scriptprog::script scr = scriptprog::get_script(host);
		if( !scr.exists() )
		{
			this->warning("script '%s' is not defined (line %u)\n", (const char*)host, node.line());
		}
		else
		{
			scriptdecl decl = scr->get_declaration(name);
			if( !decl.cScript )
			{	// subfunction name not found
				this->warning("subfunction '%s' in script '%s' is not defined (line %u)\n", name.c_str(), host.c_str(), node.line());
			}
			else
			{	
				this->put_intcommand(OP_PUSH_INT, paramcnt);
				this->put_strcommand(OP_PUSH_STRING, host);
				this->put_strcommand(OP_SUBFUNCTION, name);
				return true;
			}
		}
	}
	return false;
	*/
}

bool eacompiler::compile_function_call(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// identifer '(' <ExprList> ')'
	if(flags& CFLAG_LVALUE )
	{	//##TODO: check if this function does return a lvalue
	}

	const basics::string<>& name = node[0].string();
	// one additional parameter when called as memberfunction
	return put_function_call(node, scope, name, ((flags&CFLAG_MEMBER)!=0), false);
}
bool eacompiler::compile_globalfunction_call(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '::'  <Func Call>
	// <Func Call> ::= identifier '(' <ExprList> ')'
	const parse_node &childnode = node[1];
	const basics::string<>& name = childnode[0].string();
	// one additional parameter when called as memberfunction
	return put_function_call(node, scope, name, ((flags&CFLAG_MEMBER)!=0), true);
}
bool eacompiler::compile_subfunction_call(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op Pointer> '::'  <Func Call>
	if( node[0].symbol() != PT_IDENTIFIER )
	{	// otherwise the scope variable is invalid
		this->warning("invalid scope function (line %u)\n", node.line());
	}
	else
	{	// <Func Call> ::= identifier '(' <ExprList> ')'
		const parse_node &childnode = node[2];
		const basics::string<>& base = node[0].string();
		const basics::string<>& name = childnode[0].string();
		return put_subfunction_call(node, scope, base, name, ((flags&CFLAG_MEMBER)!=0));
	}
	return false;
}

bool eacompiler::compile_objlist(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{
	if( node.symbol() == PT_OBJLIST )
	{	// <Obj Inst> <Obj List>
		return compile_objlist(node[0], scope, flags, uservalue) &&
			compile_objlist(node[1], scope, flags, uservalue);
	}
	else
	{	// new specitem or orderd item
		inst.push_back( scriptinstance::instance() );
		if( compile_main(node, scope, flags, uservalue) )
		{
			//##TODO:
			// test for the necessary elements
			// instanciate new object
			return true;
		}
		inst.strip(1);
		return false;
	}

}

///////////////////////////////////////////////////////////////////////////
// objects
bool eacompiler::compile_object(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	//<Obj Type> <Obj Id> <Obj List> <Obj Script>
	//identifier, identifier or empty, instance list, scriptobject

	prog.clear();
	inst.clear();
	cVariable.clear();
	cStrTable.clear();
	cLabels.clear();
	cConstvalues.clear();
	scriptdefines  tempdefines   = cDefines_;

	// compile the specification
	if( !compile_objlist(node[2], scope, flags, uservalue) || inst.size()==0 )
		return false;

	// test type node[0]

	// test scriptid node[1]
	if( node[1].symbol()==PT_OBJID )
	{	// has no name
		// check if a specification has a node "name"
		// otherwise create some random name
	}
	else
	{
		prog->cName = node[1].string();
	}


	// check conflicting object id
	// create new script entry
	// create new script instances

	// compile body if exists
	// ';'				// nothing 
	// <Block>			// script block
	// identifier ';'	// reference to another script (main label)
	// <SubProg>  ';'	// reference to another script label
	// <ItemList> ';'	// item list
	const basics::string<> mainlabel = "main";
	basics::string<> startlabel = mainlabel;	// default
	
	bool accept = false;
	const parse_node &script = node[3];
	if( script.symbol()==PT_SEMI )
	{	// instance declaration only
		prog.clear();
		accept = true;
	}
	else
	{	// the instance also has a body
		const parse_node &item = script[0];

		///////////////////////////////////////////////////////////////////////
		if( script.symbol()==PT_BLOCK )
		{	// block body
			///////////////////////////////////////////////////////////////////
			// create a declaration header
			this->funcdecl = prog->cHeader.search(mainlabel);
			if( !this->funcdecl )
			{	// create new
				prog->cHeader[mainlabel].cName = mainlabel;
				this->funcdecl = prog->cHeader.search(mainlabel);
				this->funcdecl->cScript = &(*prog);
			}
			// else warn that declaration already exists
			///////////////////////////////////////////////////////////////////
			// '{' <Stm List> '}'
			prog->appendCommand(OP_START);
			size_t sizepos=prog->appendInt(0);
			// compile
			accept = compile_main(script[1], scope, 0, uservalue) &&
					 check_labels() && check_variable(scope);
			// cleanup
			if( prog->cProgramm.last() != OP_END )
				this->put_command(OP_END);
			prog->replaceInt(prog->getCurrentPosition(), sizepos);
			///////////////////////////////////////////////////////////////////
			this->funcdecl = NULL;

			// register script
			if(accept)
			{
				accept = scriptprog::regist(prog, node.line());
				if( accept )
				{
					if( cCompileOptions&OPT_COMPILEOUTPUT )
						prog->dump();
					this->cFile->scripts.push_back(prog->cName);
				}
			}
		}
		///////////////////////////////////////////////////////////////////////
		else  if( item.symbol()==PT_SUBPROG )
		{	// script with extra position
			///////////////////////////////////////////////////////////////////
			// identifier '::' identifier
			const basics::string<>& name  = item[0].string();
			startlabel = item[2].string();
			// find the programm
			prog = scriptprog::get_script(name);
			accept = true;
			if( !(accept=prog.exists()) )
			{
				this->warning("script '%s' does not exists (line %u)\n",
					name.c_str(), item.line());
			}
			else if( !(accept=prog->cLabels.exists(startlabel)) )
			{
				this->warning("label '%s' does not exist in script '%s' (line %u)\n",
					startlabel.c_str(), name.c_str(), item.line());
			}
			///////////////////////////////////////////////////////////////////
		}
		///////////////////////////////////////////////////////////////////////
		else if( item.symbol()==PT_IDENTIFIER )
		{	// single script duplication
			///////////////////////////////////////////////////////////////////
			// identifier
			const basics::string<>& name  = item.string();
			// find the programm
			prog = scriptprog::get_script(name);
			accept = true;
			if( !(accept=prog.exists()) )
			{
				this->warning("script '%s' does not exists (line %u)\n",
					name.c_str(), node.line());
			}
			///////////////////////////////////////////////////////////////////
		}
		///////////////////////////////////////////////////////////////////////
		else
		{	// items
			///////////////////////////////////////////////////////////////////
			// create a declaration header
			this->funcdecl = prog->cHeader.search(mainlabel);
			if( !this->funcdecl )
			{	// create new
				prog->cHeader[mainlabel].cName = mainlabel;
				this->funcdecl = prog->cHeader.search(mainlabel);
				this->funcdecl->cScript = &(*prog);
			}
			///////////////////////////////////////////////////////////////////
			prog->appendCommand(OP_START);
			size_t sizepos=prog->appendInt(0);

			create_label("OnInit",node.line());
			accept = compile_itemlist(item, scope, (flags & ~CFLAG_RVALUE) | CFLAG_RVALUE | CFLAG_CONST, uservalue);
			this->put_command(OP_END);
			create_label("OnClick",node.line());
			put_function_call(node, scope, "openshop", 0, true);
			// cleanup
			prog->appendCommand(OP_END);
			prog->replaceInt(prog->getCurrentPosition(), sizepos);
			///////////////////////////////////////////////////////////////////
			this->funcdecl = NULL;

			// register script
			if(accept)
			{
				accept = scriptprog::regist(prog, node.line());
				if( accept )
				{
					if( cCompileOptions&OPT_COMPILEOUTPUT )
						prog->dump();
					this->cFile->scripts.push_back(prog->cName);
				}
			}
		}
		///////////////////////////////////////////////////////////////////////
	}
	if( accept )
	{
		// set prog and startlabel to the instances
		basics::vector<scriptinstance::instance>::iterator iter(inst);
		for(; iter; ++iter)
		{
			(*iter)->cType   = node[0].string();
			if(prog.exists())
			{
				(*iter)->cScript = prog->cName;
				(*iter)->cStart  = startlabel;
			}
			if( accept && cCompileOptions&OPT_COMPILEOUTPUT )
			{
				printf("----------\ninstance '%s' with properties:\n", (*iter)->cType.c_str());
				scriptinstance::property::iterator propiter((*iter)->cProperty);
				for(; propiter; ++propiter)
				{
					printf("%s - %s\n", (const char*)propiter->key, (const char*)propiter->data.get_string());
				}
				if( !prog.exists() )
				{
					printf("using no script\n");
					
				}
				else if( 0==prog->cName.size() )
				{
					printf("using an unnamed script starting at '%s'\n", startlabel.c_str());
				}
				else
				{
					printf("using script '%s' starting at '%s'\n", prog->cName.c_str(), startlabel.c_str());
				}
			}
			// append to current file
			this->cFile->instances.push_back(*iter);
		}
	}
	prog.clear();
	inst.clear();
	cVariable.clear();
	cStrTable.clear();
	cLabels.clear();
	cConstvalues.clear();
	cDefines_ = tempdefines;

	return accept;
}

bool eacompiler::compile_block(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Block>
	bool accept = true;
	if( !prog.exists() )
	{	// global block:
		prog.clear();
		cVariable.clear();
		cStrTable.clear();
		cLabels.clear();
		cConstvalues.clear();
		scriptdefines  tempdefines   = cDefines_;

		// '{' <Stm List> '}'
		prog->clear();
		prog->appendCommand(OP_START);
		size_t sizepos=prog->appendInt(0);

		// compile
		accept = compile_main(node[1], scope, 0, uservalue) &&
				 check_labels() && check_variable(scope);
		// cleanup
		if( prog->cProgramm.last() != OP_END )
			this->put_command(OP_END);
		prog->replaceInt(prog->getCurrentPosition(), sizepos);

		if( accept )
		{
			if( cCompileOptions&OPT_COMPILEOUTPUT )
				prog->dump();
		}

		prog.clear();
		cVariable.clear();
		cStrTable.clear();
		cLabels.clear();
		cConstvalues.clear();
		cDefines_ = tempdefines;
	}
	else
	{	// just a block, increments scope
		// save the curent variable set
		scriptdefines  tempdefines   = cDefines_;
		CVariableScope tempvariable = cVariable;
		accept = compile_main(node[1], scope+1, flags, uservalue) &&
			check_variable(scope+1, false);
		// export the variables from the scope
		this->export_variable(node[2], scope+1, tempvariable);
		// restore the variable set
		cVariable = tempvariable;
		cDefines_ = tempdefines;
	}
	return accept;
}

bool eacompiler::compile_instance(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' <Spec List> ')'
	return compile_main(node[1], scope, (flags & ~CFLAG_RVALUE) | CFLAG_RVALUE | CFLAG_CONST, uservalue);
}

bool eacompiler::compile_ordernpc(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ')'
	// ordered parameter access
	// map, posx, posy, dir, name, sprite
	scriptinstance::property& prop = inst.last()->cProperty;
	prop.clear();
	prop["map"]		= basics::variant(basics::string<>(node[1].string()+1, node[1].string().size()-2));
	prop["posx"]	= basics::variant(basics::stringtoi(node[3].string().c_str()));
	prop["posy"]	= basics::variant(basics::stringtoi(node[5].string().c_str()));
	prop["dir"]		= basics::variant(basics::stringtoi(node[7].string().c_str()));
	prop["name"]	= basics::variant(basics::string<>(node[11].string()+1, node[11].string().size()-2));
	prop["sprite"]	= basics::variant(basics::stringtoi(node[13].string().c_str()));
	return true;
}

bool eacompiler::compile_ordertouch(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ')'
	// ordered parameter access
	// map, posx, posy, dir, name, sprite, touchupx, touchupy
	scriptinstance::property& prop = inst.last()->cProperty;
	prop.clear();
	prop["map"]		= basics::variant(basics::string<>(node[1].string()+1, node[1].string().size()-2));
	prop["posx"]	= basics::variant(basics::stringtoi(node[3].string().c_str()));
	prop["posy"]	= basics::variant(basics::stringtoi(node[5].string().c_str()));
	prop["dir"]		= basics::variant(basics::stringtoi(node[7].string().c_str()));
	prop["name"]	= basics::variant(basics::string<>(node[11].string()+1, node[11].string().size()-2));
	prop["sprite"]	= basics::variant(basics::stringtoi(node[13].string().c_str()));
	prop["touchx"]	= basics::variant(basics::stringtoi(node[17].string().c_str()));
	prop["touchy"]	= basics::variant(basics::stringtoi(node[19].string().c_str()));
	return true;
}
bool eacompiler::compile_ordermob(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ')'
	// ordered parameter access
	// map,area1x,area1y,area2x,area2y,name,sprite,count,time1,time2
	scriptinstance::property& prop = inst.last()->cProperty;
	prop.clear();
	prop["map"]		= basics::variant(basics::string<>(node[1].string()+1, node[1].string().size()-2));
	prop["area1x"]	= basics::variant(basics::stringtoi(node[3].string().c_str()));
	prop["area1y"]	= basics::variant(basics::stringtoi(node[5].string().c_str()));
	prop["area2x"]	= basics::variant(basics::stringtoi(node[7].string().c_str()));
	prop["area2y"]	= basics::variant(basics::stringtoi(node[9].string().c_str()));
	prop["name"]	= basics::variant(basics::string<>(node[11].string()+1, node[11].string().size()-2));
	prop["sprite"]	= basics::variant(basics::stringtoi(node[13].string().c_str()));
	prop["count"]	= basics::variant(basics::stringtoi(node[15].string().c_str()));
	prop["time1"]	= basics::variant(basics::stringtoi(node[17].string().c_str()));
	prop["time2"]	= basics::variant(basics::stringtoi(node[19].string().c_str()));
	return true;
}
bool eacompiler::compile_orderwarp(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ',' DecLiteral ')'
	// ordered parameter access
	//  1   3    5     7      9       11       13      15  
	// map,posx,posy,touchx,touchy,targetmap,targetx,targety
	scriptinstance::property& prop = inst.last()->cProperty;
	prop.clear();
	prop["map"]		= basics::variant(basics::string<>(node[1].string()+1, node[1].string().size()-2));
	prop["posx"]	= basics::variant(basics::stringtoi(node[3].string().c_str()));
	prop["posy"]	= basics::variant(basics::stringtoi(node[5].string().c_str()));
	prop["touchx"]	= basics::variant(basics::stringtoi(node[7].string().c_str()));
	prop["touchy"]	= basics::variant(basics::stringtoi(node[9].string().c_str()));
	prop["targetmap"]= basics::variant(basics::string<>(node[11].string()+1, node[11].string().size()-2));
	prop["targetx"]	= basics::variant(basics::stringtoi(node[13].string().c_str()));
	prop["targetx"]	= basics::variant(basics::stringtoi(node[15].string().c_str()));
	return true;
}
bool eacompiler::compile_orderscr(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' StringLiteral ')'
	// ordered parameter access
	scriptinstance::property& prop = inst.last()->cProperty;
	prop.clear();
	prop["name"] = basics::variant(basics::string<>(node[1].string()+1, node[1].string().size()-2));
	return true;
}
bool eacompiler::compile_specitem(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// identifier '=' <Op If>
	// specifier items are options of the current object instance
	const basics::string<> name = node[0].string();
	static const struct
	{
		const char* name;
		uint		count;
	}
	knownspec[] =
	{
		{ "area1x", 1 },
		{ "area1y", 1 },
		{ "area2x", 1 },
		{ "area2y", 1 },
		{ "count", 1 },
		{ "dir", 1 },
		{ "map", 1 },
		{ "name", 1 },
		{ "posx", 1 },
		{ "posy", 1 },
		{ "sprite", 1 },
		{ "targetmap", 1 },
		{ "targetx", 1 },
		{ "targety", 1 },
		{ "time1", 1 },
		{ "time2", 1 },
		{ "touchx", 1 },
		{ "touchy", 1 },
		{ "position", 3 }, // {map,posx,posy}
		{ "pos", 3 }, // {map,posx,posy}
		{ "target", 3 }, // {map,posx,posy}
	};

	scriptinstance::property& prop = inst.last()->cProperty;
	if( prop.exists( name ) )
	{
		this->warning("redefinition of specifier '%s' (line %u)", name.c_str(), node.line());
	}
	else if( compile_main(node[2], scope, (flags & ~CFLAG_RVALUE) | CFLAG_RVALUE | CFLAG_CONST, uservalue) && cConstvalues.size() )
	{	// test the value
		size_t i;
		for(i=0; i<sizeof(knownspec)/sizeof(*knownspec); ++i)
		{
			if( name == knownspec[i].name )
			{
				if(knownspec[i].count!=cConstvalues.last().size())
				{
					this->warning("specifier '%s' element count is wrong (expecting %i, have %i) (line %u)\ndefaulting values\n",
						name.c_str(), knownspec[i].count, cConstvalues.last().size(), node.line());
					return false;
				}
				else
				{
					break;
				}
			}
		}
		prop[name] = cConstvalues.last();

		if( name == "position" || name == "pos" )
		{	// split known multi spec's

			if( prop.exists("map") ||
				prop.exists("posx") ||
				prop.exists("posy") )
			{
				this->warning("partial redefinition of elements from specifier '%s' (line %u)", name.c_str(), node.line());
				return false;
			}
			prop["map"] = cConstvalues.last()[0];
			prop["posx"] = cConstvalues.last()[1];
			prop["posy"] = cConstvalues.last()[2];
		}
		if( name == "target" )
		{	// split known multi spec's
			if( prop.exists("targetmap") ||
				prop.exists("targetx") ||
				prop.exists("targety") )
			{
				this->warning("partial redefinition of elements from specifier '%s' (line %u)", name.c_str(), node.line());
				return false;
			}
			prop["targetmap"] = cConstvalues.last()[0];
			prop["targetx"] = cConstvalues.last()[1];
			prop["targety"] = cConstvalues.last()[2];
		}
		cConstvalues.clear();
		return true;
	}
	return false;
}


bool eacompiler::compile_itemlist(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <ItemOverwrite> ',' <ItemList>
	bool accept=false;
	if( node.symbol() == PT_ITEMLIST )
	{	// <ItemOverwrite> ',' <ItemList>
		int usertemp = uservalue;
		
		accept = compile_itemlist(node[0], scope, flags, usertemp) &&
				 compile_itemlist(node[2], scope, flags, uservalue);
		uservalue += usertemp;
	}
	else if( node.symbol() == PT_ITEMOVERWRITE )
	{
		accept = compile_itemoverwrite(node, scope, flags, uservalue);
		uservalue = 1;
	}
	else
	{	// decliteral, string or identifier
		accept = compile_item(node, scope, (flags & ~CFLAG_RVALUE) | CFLAG_RVALUE | CFLAG_CONST, uservalue);
		uservalue = 1;
	}
	return accept;
}
bool eacompiler::compile_item(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{
	if( compile_main(node, scope, (flags & ~CFLAG_RVALUE) | CFLAG_RVALUE | CFLAG_CONST, uservalue) )
	{
		//##TODO add check for valid items
		put_function_call(node, scope, "sellitem", 1, true);
		return true;
	}
	return false;
}
bool eacompiler::compile_itemoverwrite(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// DecLiteral    ':' DecLiteral
	// StringLiteral ':' DecLiteral
	// identifier    ':' DecLiteral
	
	if( compile_main(node[0], scope, (flags & ~CFLAG_RVALUE) | CFLAG_RVALUE | CFLAG_CONST, uservalue) &&
		compile_main(node[0], scope, (flags & ~CFLAG_RVALUE) | CFLAG_RVALUE | CFLAG_CONST, uservalue) )
	{
		//##TODO add check for valid items
		//put_function_call(node, scope, "sellitem", 2, true);
		this->put_nonconst();
		this->put_intcommand(OP_PUSH_INT, 2);
		this->put_strcommand(OP_FUNCTION, "sellitem");
		return true;
	}
	return false;
}



///////////////////////////////////////////////////////////////////////////////
/*	const stack operation:
	foo(A op B, flags, constret&, val&)
	{
		case A op B:
			foo (A, flags            , constretA)
			foo (B, flags | constretA, constretB)
			if( constretA && constretB )
			{
				valB = stack.pop
				valA = stack.pop
				stack.push valA op valB
				constret = true
			}
			else
				command.push op
		case A:
			// const
			stack.push eval()
			constret = true
		case B:
			// non-const
			if( flags&constretA)
			{
				command.push stack.all
			}
			command.push eval()
	}
*/
///////////////////////////////////////////////////////////////////////////////



bool eacompiler::is_const() const
{
	return this->cConstvalues.size()>0;
}

void eacompiler::put_value_unchecked(double d)
{
//	printf("push float '%lf'\n", d);
	prog->appendInt( prog->float2int(d) );
}
void eacompiler::put_value_unchecked(int64 i)
{
//	printf("push int '%i'\n", (int)i);
	if( i==0 || i==1 )
		prog->appendCommand( i?OP_PUSH_ONE:OP_PUSH_ZERO );
	else
		prog->appendVarCommand(OP_PUSH_INT, i);
}
void eacompiler::put_value_unchecked(const basics::string<>& s)
{
//	printf("push string '%s'\n", (const char*)s);
	uint32& addr = cStrTable[s];
	if( !addr )
		addr = prog->append_string( s );
	prog->appendVarCommand(OP_PUSH_STRING, addr);
}
void eacompiler::put_value_unchecked(const basics::variant& v)
{
	if( v.is_int() )
	{
		put_value_unchecked(v.get_int());
	}
	else if( v.is_float() )
	{
		put_value_unchecked(v.get_float());
	}
	else if( v.is_string() )
	{
		put_value_unchecked(v.get_string());
	}
	else if( v.is_array() )
	{	
		size_t i;
		for(i=0; i<v.size(); ++i)
			put_value_unchecked(v[i]);
//		printf("push array '%u' elements\n", (uint)v.size());
		prog->appendVarCommand(OP_CONCAT, v.size());
	}
	else 
	{
//		printf("push empty\n");
		prog->appendCommand(OP_PUSH_NONE);
	}
}

void eacompiler::put_nonconst()
{
	if( this->is_const() )
	{
		basics::vector<basics::variant>::iterator iter(cConstvalues);
		for(; iter; ++iter)
		{
			this->put_value_unchecked(*iter);
		}
		this->cConstvalues.resize(0);
	}
}
void eacompiler::put_value(double d, bool constval)
{
	if( constval )
	{
		this->cConstvalues.push(d);
	}
	else
	{
		this->put_nonconst();
		this->put_value_unchecked(d);
	}
}
void eacompiler::put_value(int64 i, bool constval)
{
	if( constval )
	{
		this->cConstvalues.push(i);
	}
	else
	{
		this->put_nonconst();
		this->put_value_unchecked(i);
	}
}
void eacompiler::put_value(const basics::string<>& s, bool constval)
{
	if( constval )
	{
		this->cConstvalues.push(s);
	}
	else
	{
		this->put_nonconst();
		this->put_value_unchecked(s);
	}
}
void eacompiler::put_value(const basics::variant& v, bool constval)
{
	basics::variant x(v);
	x.make_value();
	if( constval )
	{
		this->cConstvalues.push( x );
	}
	else
	{
		this->put_nonconst();
		this->put_value_unchecked(x);
	}
}
void eacompiler::put_command(command_t command)
{
	if( this->is_const() )
	{	// operate on the const values
		basics::variant *vb = cConstvalues.begin();
		basics::variant *ve = vb+cConstvalues.size();
		// two element commands
		if( vb+1<ve )
		{
			if( command == OP_ADD )
			{
				ve[-2] += ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_SUB )
			{
				ve[-2] -= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_MUL )
			{
				ve[-2] *= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_DIV )
			{
				ve[-2] /= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_MOD )
			{
				ve[-2] %= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_BIN_XOR )
			{
				ve[-2] ^= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_BIN_AND )
			{
				ve[-2] &= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_BIN_OR )
			{
				ve[-2] |= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_LSHIFT )
			{
				ve[-2] <<= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_RSHIFT )
			{
				ve[-2] >>= ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_EQUATE )
			{
				ve[-2] = ve[-2]==ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_UNEQUATE )
			{
				ve[-2] = ve[-2]!=ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_ISGT )
			{
				ve[-2] = ve[-2]> ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_ISGTEQ )
			{
				ve[-2] = ve[-2]>=ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_ISLT )
			{
				ve[-2] = ve[-2]< ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
			else if( command == OP_ISLTEQ )
			{
				ve[-2] = ve[-2]<=ve[-1];
				this->cConstvalues.strip(1);
				return;
			}
		}
		// one element commands
		if( vb<ve )
		{
			if( command == OP_NOT )
			{
				ve[-1].lognot();
				return;
			}
			else if( command == OP_INVERT )
			{
				ve[-1].invert(); 
				return;
			}
			else if( command == OP_NEGATE )
			{
				ve[-1].negate();
				return;
			}
			else if( command == OP_BOOLEAN )
			{
				ve[-1] = ve[-1].get_bool();
				return;
			}
			else if( command==OP_CAST_STRING )
			{
				ve[-1].cast(basics::VAR_STRING);
				return;
			}
			else if( command==OP_CAST_FLOAT )
			{
				ve[-1].cast(basics::VAR_FLOAT);
				return;
			}
			else if( command==OP_CAST_INTEGER )
			{
				ve[-1].cast(basics::VAR_INTEGER);
				return;
			}
			else if( command==OP_EVAL )
			{	// nothing
				return;
			}
			else if( command==OP_RETURN )
			{
				if(this->funcdecl)
				{	// do a cast when asked
					const basics::var_t t = this->funcdecl->cReturn;
					switch(t)
					{ 
					case basics::VAR_FLOAT: this->put_command(OP_CAST_FLOAT); break;
					case basics::VAR_INTEGER: this->put_command(OP_CAST_INTEGER); break;
					case basics::VAR_STRING: this->put_command(OP_CAST_STRING); break;
					default: break;
					}
				}
			}
		}
	}
	// otherwise do a nonconst command
	this->put_nonconst();
	prog->appendCommand(command);
}

void eacompiler::put_intcommand(command_t command, int p)
{
	if( this->is_const() )
	{
		if( command==OP_CONCAT && this->cConstvalues.size() >= (size_t)p )
		{
			const size_t pos = this->cConstvalues.size()-p;
			size_t i;
			this->cConstvalues[pos].create_array( p );
			for(i=1; (int)i<p; ++i)
			{	// put the elements into the array
				this->cConstvalues[pos][i] = this->cConstvalues[pos+i];
			}
			if(p>1)
				this->cConstvalues.strip(p-1);
			return;
		}
		else if( command==OP_REDUCE && this->cConstvalues.size() >= (size_t)p )
		{
			if(p>1)
			{
				this->cConstvalues[this->cConstvalues.size()-p] = this->cConstvalues[this->cConstvalues.size()-1];
				this->cConstvalues.strip(p-1);
			}
			return;
		}
	}
	// otherwise do a nonconst command
	this->put_nonconst();
	if( command==OP_PUSH_INT && (p==0 || p==1) )
	{
		prog->appendCommand( p?OP_PUSH_ONE:OP_PUSH_ZERO );
	}
	else
	{
		prog->appendVarCommand(command, p);
	}
}

void eacompiler::put_strcommand(command_t command, const basics::string<>& s)
{
	this->put_nonconst();
	uint32& addr = cStrTable[s];
	if( !addr )
		addr = prog->append_string( s );

	prog->appendVarCommand(command, addr);
}


void eacompiler::put_varcommand(command_t command, int64 i)
{
	this->put_nonconst();
	prog->appendVarCommand( command, i );
}


bool eacompiler::is_nonconst_operation(const parse_node &node) const
{
	return 
		( 
		node.symbol()==PT_DEFINEDECL || 
		node.symbol()==PT_INCLUDEDECL || 
		node.symbol()==PT_FUNCDECL || 
		node.symbol()==PT_OBJDECL || 
		node.symbol()==PT_PARAMSE || 
		node.symbol()==PT_PARAMS || 
		node.symbol()==PT_PARAM || 
		node.symbol()==PT_VARDECL || 
		node.symbol()==PT_VARIABLE || 
		node.symbol()==PT_SCOPEVAR || 
		node.symbol()==PT_MEMBERVAR || 
		node.symbol()==PT_MEMBERFUNC || 
		node.symbol()==PT_VARASSIGN || 
		node.symbol()==PT_BLOCK || 
		node.symbol()==PT_LABELSTM || 
		node.symbol()==PT_SUBDECL || 
		node.symbol()==PT_IFSTM || 
		node.symbol()==PT_WHILESTM || 
		node.symbol()==PT_FORSTM || 
		node.symbol()==PT_DOSTM || 
		node.symbol()==PT_SWITCHSTM || 
		node.symbol()==PT_CASESTM || 
		node.symbol()==PT_GOTOSTM || 
		node.symbol()==PT_LCTRSTM || 
		node.symbol()==PT_RETURNSTM || 
		node.symbol()==PT_EXPRSTM || 
		node.symbol()==PT_FUNCCALL || 
		node.symbol()==PT_REGEXPR || 
		node.symbol()==PT_OPASSIGN || 
		node.symbol()==PT_OPPOST || 
		node.symbol()==PT_OPPRE || 
		node.symbol()==PT_ARRAY || 
		node.symbol()==PT_RANGE || 
		node.symbol()==PT_SPLICE || 
		node.symbol()==PT_DUPLICATE ||
		(node.symbol()==PT_OPSIZEOF && node[2].symbol()==PT_IDENTIFIER)
		);
}




/// correct the variable access identifier.
uint eacompiler::variable_correct_access(uint access)
{	// nothing but scope and store
	access &= (CFLAG_SCPMASK|CFLAG_STORMASK);

	// default the scope
	if( access&CFLAG_PARAM )
	{	// parameters have no storage
		access &= CFLAG_PARAM;
	}
	else if( 0==(access&CFLAG_SCPMASK) )
	{
		if( 0==(access&CFLAG_STORMASK)  )
			access |= CFLAG_TEMP;	// pure temp as default
		else
			access |= CFLAG_PERM;	// permanent as default for storage types
	}
	else if( access&CFLAG_TEMP )
		access &= (CFLAG_TEMP | CFLAG_STORMASK);
	else if( access&CFLAG_PARAM )
		access &= (CFLAG_PARAM | CFLAG_STORMASK);

	if( 0==(access&CFLAG_STORMASK) && 0!=(access&CFLAG_PERM) )
	{	// perm:: -> perm::global::
		access |= CFLAG_GLB;
	}
	else if( (access&CFLAG_STORMASK) && ((access&CFLAG_STORMASK)<CFLAG_PLY || (access&CFLAG_STORMASK)>CFLAG_GLD) )
	{
		access = (access&~CFLAG_STORMASK) | CFLAG_GLB;
	}
	return access;
}
/// convert name to id.
bool eacompiler::variable_scopename2id(const basics::string<>& name, int& uservalue) const
{
	static const struct
	{
		const char* name;
		int		flag;
		int		prevent;
		const char* errmsg;
	} global_names[] =
	{
		{"parameter", CFLAG_PARAM, CFLAG_STORMASK|CFLAG_SCPMASK, "'parameter' scope cannot be combined with others\n"},
		{"temp",      CFLAG_TEMP, CFLAG_PERM, "scope is already set as permanent\n"},
		{"perm",      CFLAG_PERM, CFLAG_TEMP, "scope is already set as temporary\n"},
		{"player",    CFLAG_PLY, CFLAG_STORMASK, "variable storage is already set\n"},
		{"account",   CFLAG_ACC, CFLAG_STORMASK, "variable storage is already set\n"},
		{"login",     CFLAG_LOG, CFLAG_STORMASK, "variable storage is already set\n"},
		{"npc",       CFLAG_NPC, CFLAG_STORMASK, "variable storage is already set\n"},
		{"global",    CFLAG_GLB, CFLAG_STORMASK, "variable storage is already set\n"},
		{"party",     CFLAG_PRT, CFLAG_STORMASK, "variable storage is already set\n"},
		{"guild",     CFLAG_GLD, CFLAG_STORMASK, "variable storage is already set\n"},
	};
	size_t i;
	// build a scoped variable name, 
	// check for a global name
	for(i=0; i<sizeof(global_names)/sizeof(*global_names); ++i)
	{
		if( name == global_names[i].name )
		{
			if( 0!=(uservalue &	global_names[i].prevent) )
			{
				this->warning( global_names[i].errmsg );
				// error
				return false;
			}
			else
			{
				uservalue |= global_names[i].flag;
				// found and ok
				return true;
			}
		}
	}
	// not found
	return false;
}

// convert id to name.
const char* eacompiler::variable_getscope(uint access)
{
	if(access&CFLAG_PARAM)
		return "parameter";
	else if(access&CFLAG_TEMP)
		return "temp";
	else if(access&CFLAG_PERM)
		return "perm";
	else 
		return "";
}
// convert id to name.
const char* eacompiler::variable_getstore(uint access)
{
	if( 0==(access&CFLAG_PARAM) )
	{	// for non-parameters
		if( (access&CFLAG_STORMASK) == CFLAG_PLY )
			return "player";
		else if( (access&CFLAG_STORMASK) == CFLAG_ACC )
			return "account";
		else if( (access&CFLAG_STORMASK) == CFLAG_LOG )
			return "login";
		else if( (access&CFLAG_STORMASK) == CFLAG_NPC )
			return "npc";
		else if( (access&CFLAG_STORMASK) == CFLAG_GLB )
			return "global";
		else if( (access&CFLAG_STORMASK) == CFLAG_PRT )
			return "party";
		else if( (access&CFLAG_STORMASK) == CFLAG_GLD )
			return "guild";
	}
	return "";
}
// convert id to access name.
const char* eacompiler::variable_getaccess(uint access)
{
	if(access&CFLAG_PARAM)
		return "parameter";
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==CFLAG_TEMP)
		return "temp";
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==(CFLAG_TEMP|CFLAG_PLY))
		return "player::temp";
	else if((access&(CFLAG_PERM|CFLAG_STORMASK))==(CFLAG_PERM|CFLAG_PLY))
		return "player::perm";
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==(CFLAG_TEMP|CFLAG_ACC))
		return "account::temp";
	else if((access&(CFLAG_PERM|CFLAG_STORMASK))==(CFLAG_PERM|CFLAG_ACC))
		return "account::perm";	
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==(CFLAG_TEMP|CFLAG_LOG))
		return "login::temp";
	else if((access&(CFLAG_PERM|CFLAG_STORMASK))==(CFLAG_PERM|CFLAG_LOG))
		return "login::perm";	
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==(CFLAG_TEMP|CFLAG_NPC))
		return "npc::temp";
	else if((access&(CFLAG_PERM|CFLAG_STORMASK))==(CFLAG_PERM|CFLAG_NPC))
		return "npc::perm";	
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==(CFLAG_TEMP|CFLAG_GLB))
		return "global::temp";
	else if((access&(CFLAG_PERM|CFLAG_STORMASK))==(CFLAG_PERM|CFLAG_GLB))
		return "global::perm";
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==(CFLAG_TEMP|CFLAG_PRT))
		return "party::temp";
	else if((access&(CFLAG_PERM|CFLAG_STORMASK))==(CFLAG_PERM|CFLAG_PRT))
		return "party::perm";
	else if((access&(CFLAG_TEMP|CFLAG_STORMASK))==(CFLAG_TEMP|CFLAG_GLD))
		return "guild::temp";
	else if((access&(CFLAG_PERM|CFLAG_STORMASK))==(CFLAG_PERM|CFLAG_GLD))
		return "guild::perm";
	return "<invalid>";
}

/// create a variable.
/// fails when variable already exists
bool eacompiler::create_variable(const parse_node &node, const basics::string<>& name, uint access, uint scope, basics::var_t type, bool isconst, bool declared)
{
	access = this->variable_correct_access(access);
	CVar::variable *pvar = this->cVariable[name].search(access);
	const bool pure_temp = ((access&(CFLAG_TEMP|CFLAG_STORMASK))==CFLAG_TEMP);

	if( pvar )
	{	// double creation
		if( (*pvar)->scope<scope )
		{	// recreation in a different scope
			if( pure_temp )
			{	// redeclaring a temp in a new scope
				this->warning("declaration of variable 'temp::%s' (line %u)\n   shaddows previous declaration (line %u)\n", name.c_str(), node.line(), (*pvar)->cline);
				// but we declare it
			}
			else
			{	// non-temps can be declared everywhere but in the same scope
				return true;
			}	
		}
		else
		{	
			// recreation in same scope
			this->warning("variable '%s::%s' is already declared (line %u)\n   previous declaration on line %u", 
				variable_getaccess(access), name.c_str(), node.line(), (*pvar)->cline);
			// forbid when strict
			basics::variant strict;
			return (!is_defined("strict", strict) || false==strict.get_bool());
		}
	}
	// insert as new variable
	size_t id=0;
	const bool param=0!=(access&CFLAG_PARAM);
	if( param )
	{	// function parameter, should not happen
		id = this->cVariable.cnt_paramvar++;
	}
	else if( 0!=(access&CFLAG_TEMP) && 0==(access&CFLAG_STORMASK) )
	{	// pure temporary
		id = this->cVariable.cnt_tempvar++;
		// set the number of temp variables 
		if( this->funcdecl && this->funcdecl->cVarCnt<this->cVariable.cnt_tempvar )
			this->funcdecl->cVarCnt=this->cVariable.cnt_tempvar;
	}
	this->cVariable[name].insert(access, CVar(id, scope, type, isconst, param, declared, node.line()));
	return true;
}

	
/// put a variable with known access to the program.
/// create and initialite the var entry if not exists,
bool eacompiler::put_knownvariable(const parse_node &node, const basics::string<>& name, ulong flags, uint scope, uint access)
{
	access = this->variable_correct_access(access);
	CVar::variable *pv = this->cVariable[name].search(access);

	if( !pv )
	{	// does not exist
		if( 0==(flags&CFLAG_VARCREATE) )
		{	// no creation specified
			this->warning("variable '%s::%s' is used without prior declaration (line %u)\n",
				variable_getaccess(access), name.c_str(), node.line());
			// forbid when strict
			basics::variant strict;
			if( is_defined("strict", strict) && true==strict.get_bool() )
				return false;
		}

	}
	const basics::var_t type = (flags&CFLAG_VARSTRING)?basics::VAR_STRING:(flags&CFLAG_VARFLOAT)?basics::VAR_FLOAT:(flags&CFLAG_VARINT)?basics::VAR_INTEGER:basics::VAR_AUTO;
	if( ( !pv || (0!=(flags&CFLAG_VARCREATE)) ) && !create_variable(node, name, access, scope, type, false, (0!=(flags&CFLAG_VARCREATE))) )
	{	// failed to create
		return false;
	}
	else if( 0==(flags&CFLAG_VARSILENT) )
	{	// and push
		CVar &v = *(this->cVariable[name][access]);

		if( v.isconst )
		{
			if( 0!=(flags&CFLAG_LVALUE) )
			{
				this->warning("variable '%s::%s' is const in this context (line %u)\n", variable_getaccess(access), name.c_str(), node.line());
				return false;
			}
			// force read access
			flags |= CFLAG_RVALUE;
		}

		if( 0!=(flags&CFLAG_LVALUE) && 0!=(flags&CFLAG_ASSIGN) && v.type!=basics::VAR_AUTO)
		{
			this->put_command((v.type==basics::VAR_STRING)?OP_CAST_STRING:(v.type==basics::VAR_FLOAT)?OP_CAST_FLOAT:OP_CAST_INTEGER);
		}

		if( 0!=(access&CFLAG_TEMP) && 0==(access&CFLAG_STORMASK) )
		{	// pure temporary
			if( 0!=(flags&CFLAG_RVALUE) && 0==v.luse && 0==v.xuse )
				this->warning("variable 'temp::%s' is used uninitialized (line %u)\n", name.c_str(), node.line());
			else if( 0==(flags&CFLAG_RVALUE) && 0==(flags&CFLAG_LVALUE) && 0==v.luse )
				this->warning("variable 'temp::%s' is propably used uninitialized (line %u)\n", name.c_str(), node.line());

			this->put_varcommand((0!=(flags&CFLAG_LVALUE))?OP_PUSH_TEMPVAR:OP_PUSH_TEMPVAL, v.id);
		}
		else if( 0!=(access&CFLAG_PARAM) )
		{	// function parameter
			this->put_intcommand((0!=(flags&CFLAG_LVALUE))?OP_PUSH_PARAVAR:OP_PUSH_PARAVAL, v.id);
		}
		else
		{	// put a named variable
			this->put_strcommand(OP_PUSH_STRING, variable_getscope(access));
			this->put_strcommand(OP_PUSH_STRING, variable_getstore(access));
			this->put_strcommand((0!=(flags&CFLAG_LVALUE))?OP_PUSH_VAR:OP_PUSH_VAL, name);
		}

		// update
		v.uline = node.line();
		if( 0!=(flags&CFLAG_RVALUE) )
			++v.ruse;
		if( 0!=(flags&CFLAG_LVALUE) && 0==(flags&CFLAG_NOASSIGN) )
			++v.luse;
		if( 0==(flags&CFLAG_LVALUE) && 0==(flags&CFLAG_RVALUE) )
			++v.xuse;
	}
	return true;
}

/// put a variable with unknown access to the program.
/// fail when multiple variables of same name exist
bool eacompiler::put_variable(const parse_node &node, const basics::string<>& name, ulong flags, uint scope)
{
	basics::smap<uint, CVar::variable> &m = this->cVariable[name];
	basics::smap<uint, CVar::variable>::iterator iter(m);
	uint access = CFLAG_TEMP;
	if( m.size() == 0 )
	{	// create as temp
	}
	else if( m.size() == 1 )
	{	// use the access of the only existing variable
		access = iter->key;
	}
	else
	{	// go through the multiple choises
		basics::smap<uint, CVar::variable>::iterator iterX(m);
		uint cnt=1;

		this->warning("conflicting variable name '%s' (line %u)\ncan be:", name.c_str(), node.line());
		for(; iter; ++iter)
		{
			this->warning("\n'%s::%s'", variable_getaccess(iter->key), name.c_str());

			if( iterX->data->scope < iter->data->scope )
			{	// select the largest existing scope
				iterX = iter;
				cnt=1;
			}
			else if( iterX->data->scope == iter->data->scope )
			{	// and count the vars on the that scope
				++cnt;
			}
		}
		this->warning("\n");
		if( cnt==1 )
		{	// we use this variable
			this->warning("now using variable '%s::%s'\n", variable_getaccess(iterX->key), name.c_str());
			access = iterX->key;
		}
		else if( m.exists( (access = CFLAG_TEMP) ) )
		{	// prefer a temp var, when exist
			this->warning("prefering variable 'temp::%s'\n", name.c_str());
		}
		else
		{
			this->warning("failed to determin a suitable variable\n");
			return false;
		}
	}
	return this->put_knownvariable(node, name, flags, scope, access);
}

bool eacompiler::exists_variable(const basics::string<>& name, uint access)
{
	access = this->variable_correct_access(access);
	return this->cVariable.exists(name) && this->cVariable[name].exists(access);
}

bool eacompiler::check_variable(uint scope, bool final) const
{	// when leaving a scope, check for unused variables
	CVariableScope::iterator nameiter(this->cVariable);
	for( ; nameiter; ++nameiter)
	{
		basics::smap<uint,CVar::variable>::iterator iter(nameiter->data);
		for( ; iter; ++iter)
		{
			const CVar& v = *(iter->data);
			if( v.scope == scope )
			{	// only process variables from this scope
				if( (iter->key&(CFLAG_TEMP|CFLAG_STORMASK))==CFLAG_TEMP )
				{	// temp vars
					// need initialisation and should be used
					if( (final || v.declared) && !v.luse && !v.xuse  )
						this->warning("variable 'temp::%s' is never assigned (line %u)\n", nameiter->key.c_str(), v.cline);
					if( (final || v.declared) && !v.ruse && !v.xuse && nameiter->key != "$re" )
						this->warning("variable 'temp::%s' is unused (line %u)\n", nameiter->key.c_str(), v.cline);
				}
				else
				{	// global or parameters
					if( 0== v.ruse + v.luse - ((iter->key&CFLAG_PARAM)==CFLAG_PARAM) )
						this->warning("variable '%s::%s' is unused (line %u)\n", variable_getaccess(iter->key), nameiter->key.c_str(), v.cline);
				}
			}
		}
	}
	return true;
}

bool eacompiler::export_variable(const parse_node &node, uint scope, CVariableScope& target) const
{
	CVariableScope::iterator nameiter(this->cVariable);
	for( ; nameiter; ++nameiter)
	{
		basics::smap<uint,CVar::variable>::iterator iter(nameiter->data);
		for( ; iter; ++iter)
		{
			const CVar& v = *(iter->data);
			if( v.scope == scope && 
				!v.declared &&
				(iter->key&(CFLAG_TEMP|CFLAG_STORMASK))==CFLAG_TEMP &&
				NULL==target[nameiter->key].search(iter->key) )
			{	// only process temp variables from this scope, that have been auto-declared
				target[nameiter->key].insert(iter->key, iter->data);

				this->warning("variable 'temp::%s' (auto-created at line %u) exported to parent scope (line %u)\n",
					nameiter->key.c_str(), v.cline, node.line());
			}
		}
	}
	return true;
}

bool eacompiler::compile_identifier(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{
	basics::variant def;
	bool accept = true;
	///////////////////////////////////////////////////////////////////
	// a single variable
	// the variables can be 
	// define, unscoped temp/user variable
	// check define, fail when lvalue
	const basics::string<>& name = node.string();
	if( is_defined(name, def) )
	{
		if(flags&CFLAG_LVALUE)
		{
			this->warning("define '%s' cannot be lvalues (line %u)\n", name.c_str(), node.line());
			accept = false;
		}
		else if(flags&(CFLAG_VARCREATE|CFLAG_VARSILENT))
		{
			this->warning("variable name '%s' already used by a define (line %u)\n", name.c_str(), node.line());
			accept = false;
		}
		else
		{
			basics::smap<uint, CVar::variable> *m = this->cVariable.search(name);
			if( m && m->size() )
			{	// variable name exists
				basics::smap<uint, CVar::variable>::iterator iter(*m);

				this->warning("variable '%s' (line %u) is hidden by a previous define statement.\n", name.c_str(), node.line());
				this->warning("to specify the variable use '%s::%s'", variable_getaccess(iter->key), name.c_str());
				for(++iter; iter; ++iter)
					this->warning("or '%s::%s'", variable_getaccess(iter->key), name.c_str());
				this->warning("\n");
			}
			this->put_value(def);
		}
	}
	///////////////////////////////////////////////////////////////////
	// when in a const scope, no other identifiers but defines are allowed
	else if( (flags&CFLAG_CONST)!=0 )
	{
		this->warning("'%s' is not defined (line %u)\n", name.c_str(), node.line());
		accept = false;
	}
	///////////////////////////////////////////////////////////////////
	else
	{	// get the variable
		accept = (flags&CFLAG_VARCREATE)?put_knownvariable(node, name, flags, scope, CFLAG_TEMP):put_variable(node, name, flags, scope);
	}
	return accept;
}

bool eacompiler::compile_numberliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// {digit}*
	// 0b{Bin Digit}+
	// 0o{Oct Digit}+
	// 0x{Hex Digit}+
	if( flags&CFLAG_RVALUE || 0==(flags&CFLAG_LVALUE))
	{
		int64 num = 0;
		if( node.symbol()== PT_DECLITERAL )
			num = basics::stringtoi(node.c_str());
		else if( node.symbol()== PT_BINLITERAL )
			num = basics::stringtoib(node.c_str());
		else if( node.symbol()== PT_OCTLITERAL )
			num = basics::stringtoio(node.c_str());
		else if( node.symbol()== PT_HEXLITERAL )
			num = basics::stringtoix(node.c_str());
		else if( node.symbol()== PT_BOOLFALSE )
			num = 0;
		else if( node.symbol()== PT_BOOLTRUE )
			num = 1;
		this->put_value( num );
		return true;
	}
	else
	{
		this->warning("left hand assignment, not accepted (line %u)\n", node.line());
		return false;
	}
}

bool eacompiler::compile_floatliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// ({Digit}* '.' {Digit}+ (([eE][+-]?{Digit}+)|[TGMKkmunpfa])?)
	// ({Digit}+ '.' {Digit}* (([eE][+-]?{Digit}+)|[TGMKkmunpfa])?)
	// ({Digit}+              (([eE][+-]?{Digit}+)|[TGMKkmunpfa]) )
	if( flags&CFLAG_RVALUE || 0==(flags&CFLAG_LVALUE) )
	{
		this->put_value( basics::stringtod(node.c_str()) );
		return true;
	}	
	else
	{
		this->warning("left hand assignment, not accepted (line %u)\n", node.line());
		return false;
	}
}
bool eacompiler::compile_charliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '' ( {Char Ch}   | ('\'{&20 .. &FF}) )   ''
	if( flags&CFLAG_RVALUE || 0==(flags&CFLAG_LVALUE) )
	{
		this->put_value( (int64)((node.c_str())? node.c_str()[1]:0) );
		return true;
	}	
	else
	{
		this->warning("left hand assignment, not accepted (line %u)\n", node.line());
		return false;
	}
}

bool eacompiler::compile_stringliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '"'( {String Ch} | ('\'{&20 .. &FF}) )* '"'
	// '/' ( {RegEx Start Ch} | '\/' )( {RegEx Ch} | '\/' )* '/'
	if( flags&CFLAG_RVALUE || 0==(flags&CFLAG_LVALUE) )
	{
		this->put_value( basics::string<>(node.string().c_str()+1, node.string().size()-2) );
		return true;
	}	
	else
	{
		this->warning("left hand assignment, not accepted (line %u)\n", node.line());
		return false;
	}
}

bool eacompiler::compile_emptyliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '-'
	// 'default'
	if( flags&CFLAG_RVALUE || 0==(flags&CFLAG_LVALUE) )
	{
		this->put_value( basics::variant() );
		return true;
	}	
	else
	{
		this->warning("left hand assignment, not accepted (line %u)\n", node.line());
		return false;
	}
}

bool eacompiler::compile_semiliteral(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// ';'
	this->put_command(OP_POP);
	return true;
}


bool eacompiler::compile_switchstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{
	if( node.symbol() != PT_SWITCHSTM )
		return false;

	///////////////////////////////////////////////////////////////////////////
	// 'switch' '(' <Expr> ')' '{' <Case Stm> '}'
	size_t var_id=0;
	basics::string<> varname;
	parse_node worknode = node;
	CContBreak prev_controls = this->cControl;
	this->cControl = CContBreak();
	flags &= ~(CFLAG_USE_BREAK|CFLAG_USE_CONT);

	varname << "_#casetmp_" << (int)prog->getCurrentPosition();
	create_variable(node, varname, CFLAG_TEMP, scope, basics::VAR_AUTO, false, true);
	cVariable[varname][CFLAG_TEMP]->luse = 1;
	cVariable[varname][CFLAG_TEMP]->ruse = 2;
	var_id = cVariable[varname][CFLAG_TEMP]->id;
//	this->logging("create temporary variable %s (%u)\n", varname.c_str(), (uint)var_id);

	// compile <Expr>
	if( !compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue) )
		return false;
	// save the result from <Expr> into the temp variable
	this->put_varcommand( OP_PUSH_TEMPVAR, var_id );
//	this->logging("push temporary variable %s (%u)\n", varname.c_str(), (uint)var_id);
	this->put_command(OP_ASSIGN);
//	this->logging("OP_ASSIGN\n");
	this->put_command(OP_POP);

	worknode = node[5];

	///////////////////////////////////////////////////////////////////////////
	// use a if-else-if strucure to process the case; 
	// but first reorder the tree to a list
	basics::vector<parse_node> conditions;
	basics::vector<parse_node> expression;
	basics::vector<uint> gotomarker;
	size_t defaultgoto;
	int defaultpos=-1;

	///////////////////////////////////////////////////////////////////////////
	// run through the tree and build the lists
	// this actually could be combined with the second step
	while( worknode.symbol() == PT_CASESTM )
	{	// 'case' <Expr List> ':' <Stm List> <Case Stm>
		// 'default' ':' <Stm List> <Case Stm>
		// empty
		if( worknode.childs() == 5 )
		{	// normal case
			conditions.push(worknode[1]);
			expression.push(worknode[3]);
			worknode = worknode[4];
		}
		else if( worknode.childs() == 4 )
		{	// default case
			defaultpos = expression.size();
			conditions.push(worknode[0]);
			expression.push(worknode[2]);
			worknode = worknode[3];
		}
		else //if( worknode.childs() == 0 )
		{	// empty
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// go through the list and build the if-else-if
	size_t i;
	bool accept = true;
	for(i=0; i<conditions.size() && accept; ++i)
	{	// push the case value
		if( (int)i == defaultpos || conditions[i].symbol()==PT_EXPR )
		{	// spare default case and empty expression
			// ignore this
			gotomarker.push( 0 );
		}
		else if( conditions[i].symbol()==PT_EXPRLIST )
		{	// put the explession list as multiple case
			// <Expr List>  ::= <Expr> ',' <Expr List>
			//                | <Expr>
			worknode = conditions[i];
			
			size_t inspos1=0;
			while( accept && worknode.symbol()==PT_EXPRLIST )
			{	
				if( worknode[0].symbol()!=PT_EXPR )// ignore empty expressions
				{	
					this->put_varcommand( OP_PUSH_TEMPVAL, var_id );
					accept = compile_main(worknode[0], scope, flags, uservalue);
					this->put_command(OP_EQUATE);

//					this->logging("Conditional Jump True or Pop -> Label1\n");
					this->put_command(OP_IF3);
					inspos1 = prog->appendAddr(inspos1);	// placeholder
				}
				if( accept && worknode[2].symbol()==PT_EXPRLIST )
					worknode = worknode[2];
				else if( worknode[2].symbol()!=PT_EXPR )// ignore empty expressions
				{	
					this->put_varcommand( OP_PUSH_TEMPVAL, var_id );
					accept = compile_main(worknode[2], scope, flags, uservalue);
					this->put_command(OP_EQUATE);

//					this->logging("Conditional Jump True or Pop -> Label1\n");
					this->put_command(OP_IF3);
					inspos1 = prog->appendAddr(inspos1);	// placeholder
					break;
				}
			}
			gotomarker.push( inspos1 );
		}
		else
		{	// a single value
			// push the temporary variable
			this->put_varcommand( OP_PUSH_TEMPVAL, var_id );
//			this->logging("push temporary variable %s (%i)\n", varname.c_str(), (int)var_id);

			accept = compile_main(conditions[i], scope, flags, uservalue);

//			this->logging("PT_EQUAL\n");
			this->put_command(OP_EQUATE);

//			this->logging("Conditional Jump True -> CaseLabel%i\n", (int)i);
			this->put_command(OP_IF3);
			gotomarker.push( prog->appendAddr(0) );	// placeholder
		}
	}
//	this->logging("Goto -> LabelEnd/Default\n");
	this->put_command(OP_GOTO3);
	defaultgoto = prog->appendAddr(0);	// placeholder
	if( defaultpos>=0 )
		gotomarker[defaultpos] = defaultgoto;

	///////////////////////////////////////////////////////////////////////////
	// build the body of the case statements
	// update the jumps to the correct targets
	if(accept)
	{	
		for(i=0; i<expression.size()&& accept; ++i)
		{
			// replace with real target address
			this->replace_jumps(gotomarker[i], prog->getCurrentPosition());
			// compile the statement
			accept = compile_main(expression[i], scope, flags | CFLAG_USE_BREAK, uservalue);
		}
		size_t rend = prog->getCurrentPosition();
		if(defaultpos<0)
		{	// no default case, so redirect the last goto to the end
			this->replace_jumps(defaultgoto, rend);
		}
		// convert break -> goto REND
		this->replace_jumps(this->cControl.break_target, rend);
		// restore previous controls
		this->cControl = prev_controls;
	}
	return accept;
}

bool eacompiler::compile_ifstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'if' '(' <Expr> ')' <Normal Stm>
	bool accept = false;
	if( node.childs()==5 )
	{
		accept  = compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
		if( !accept )
		{	// nothing to do
		}
		else if( this->cConstvalues.size() )
		{	// the condidtion is const
			const bool is_true = this->cConstvalues.last().get_bool();
			this->cConstvalues.strip(1);
			if( is_true )
			{	// put in <Normal Stm>
				accept &= compile_main(node[4], scope, flags, uservalue);
			}
			// otherwise we don't need it
		}
		else
		{	// need to evaluate
//			this->logging("Conditional Jump False -> Label1\n");
			this->put_command(OP_NIF3);
			size_t inspos1 = prog->appendAddr(0);	// placeholder
			// put in <Normal Stm>
			accept &= compile_main(node[4], scope, flags, uservalue);
//			this->logging("Label1\n");
			// calculate and insert the correct jump 
			prog->replaceAddr( prog->getCurrentPosition() ,inspos1);
		}
	}
	else //if( node.childs()==7 )
	{	// 'if' '(' <Expr> ')' <Normal Stm> 'else' <Normal Stm>
		accept  = compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
		if( !accept )
		{	// nothing to do
		}
		else if( this->cConstvalues.size() )
		{
			const bool is_true = this->cConstvalues.last().get_bool();
			this->cConstvalues.strip(1);
			// put in <Normal Stm1> or <Normal Stm2>
			accept &= compile_main(node[is_true?4:6], scope, flags, uservalue);
		}
		else
		{
//			this->logging("Conditional Jump False -> Label1\n");
			this->put_command(OP_NIF3);
			size_t inspos1 = prog->appendAddr(0);	// placeholder
			// put in <Normal Stm>
			accept &= compile_main(node[4], scope, flags, uservalue);
//			this->logging("Goto -> Label2\n");
			this->put_command(OP_GOTO3);
			size_t inspos2 = prog->appendAddr(0);	// placeholder
//			this->logging("Label1\n");
			// calculate and insert the correct jump 
			prog->replaceAddr( prog->getCurrentPosition() ,inspos1);
			// put in <Normal Stm2>
			accept &= compile_main(node[6], scope, flags, uservalue);
//			this->logging("Label2\n");
			prog->replaceAddr( prog->getCurrentPosition(), inspos2);
		}
	}
	return accept;
}

bool eacompiler::compile_whilestm(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'while' '(' <Expr> ')' <Normal Stm>
	bool accept;
//	this->logging("Label1\n");
	CContBreak prev_controls = this->cControl;
	this->cControl = CContBreak( prog->getCurrentPosition() ); // position marker
	flags &= ~(CFLAG_USE_BREAK|CFLAG_USE_CONT);

	// execute <Expr>
	accept  = compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);

//	this->logging("Conditional Jump False -> Label2\n");
	this->put_command(OP_NIF3);
	size_t inspos2 = prog->appendAddr(0);	// placeholder offset

	// execute <Normal Stm>
	accept &= compile_main(node[4], scope, flags | CFLAG_USE_BREAK | CFLAG_USE_CONT, uservalue);

//	this->logging("Goto -> Label1\n");
	this->put_varcommand(OP_GOTO, this->cControl.continue_target);

//	this->logging("Label2\n");
	size_t tarpos2 = prog->getCurrentPosition();
	prog->replaceAddr(tarpos2 ,inspos2);
	// convert break -> goto Label2
	this->replace_jumps(this->cControl.break_target,tarpos2);
	this->cControl = prev_controls;
	// convert continue -> goto Label1, already done	
	return accept;
}

bool eacompiler::compile_forstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'for' '(' <Expr List> ';' <Expr> ';' <Expr List> ')' <Normal Stm>
	// execute <Arg1>
	CContBreak prev_controls = this->cControl;
	this->cControl = CContBreak();
	flags &= ~(CFLAG_USE_BREAK|CFLAG_USE_CONT);

	const bool has_initalize = (node[2].symbol() != PT_EXPR);
	const bool has_condition = (node[4].symbol() != PT_EXPR);
	const bool has_increment = (node[6].symbol() != PT_EXPR);
	
	bool accept = true;
	if( has_initalize )
	{
		accept = compile_main(node[2], scope, flags, uservalue);
		this->put_command(OP_POP);
	}
	
//	this->logging("Label1\n");
	size_t tarpos1 = prog->getCurrentPosition();// position marker
	// execute <Arg2>, need a value

	
	size_t inspos2=0;
	if(has_condition)
	{
		accept &= compile_main(node[4], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
//		this->logging("Conditional Jump False -> Label2\n");
		this->put_command(OP_NIF3);
		inspos2 = prog->appendAddr(0);	// placeholder
	}
	
	// execute the loop body
	accept &= compile_main(node[8], scope, flags | CFLAG_USE_BREAK | CFLAG_USE_CONT, uservalue);

	// convert continue -> goto Label3
	this->replace_jumps(this->cControl.continue_target, prog->getCurrentPosition());
	this->cControl.continue_target = 0;

	if(has_increment)
	{	// execute the incrementor <Arg3>
		accept &= compile_main(node[6], scope, flags, uservalue);
		this->put_command(OP_POP);
	}

//	this->logging("Goto -> Label1\n");
	this->put_varcommand(OP_GOTO, tarpos1);

//	this->logging("Label2\n");
	size_t tarpos2 = prog->getCurrentPosition();
	if(has_condition)
	{
		prog->replaceAddr(tarpos2, inspos2);
	}

	// convert break -> goto Label2
	this->replace_jumps(this->cControl.break_target,tarpos2);
	this->cControl = prev_controls;
	return accept;
}

bool eacompiler::compile_dostm(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'do' <Normal Stm> 'while' '(' <Expr> ')' ';'
//	this->logging("Label1\n");

	CContBreak prev_controls = this->cControl;
	this->cControl = CContBreak();
	flags &= ~(CFLAG_USE_BREAK|CFLAG_USE_CONT);

	size_t tarpos1 = prog->getCurrentPosition();

	// execute <Normal Stm>
	bool accept  = compile_main(node[1], scope, flags | CFLAG_USE_BREAK | CFLAG_USE_CONT, uservalue);
	
	// convert continue -> goto Label1
	this->replace_jumps(this->cControl.continue_target, prog->getCurrentPosition());

	// execute <Expr>
	accept &= compile_main(node[4], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
	this->put_command(OP_POP);

//	this->logging("Conditional Jump True -> Label1\n");
	this->put_varcommand(OP_IF, tarpos1);
	
	// convert break -> goto Label2
	this->replace_jumps(this->cControl.break_target, prog->getCurrentPosition());
	this->cControl = prev_controls;
	return accept;
}

bool eacompiler::compile_lctrlstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'break' ';'
	// 'continue' ';'
	if( (node.childs()>0 && node[0].symbol() == PT_CONTINUE) && (0!=(flags & CFLAG_USE_CONT)) )
	{
		
		if( this->cControl.has_continue )
		{
			this->put_varcommand(OP_GOTO, this->cControl.continue_target);
		}
		else
		{
			this->put_command(OP_GOTO3);
			this->cControl.continue_target = prog->appendAddr( this->cControl.continue_target );
		}
	}
	else if( (node.childs()>0 && node[0].symbol() == PT_BREAK) && (0!=(flags & CFLAG_USE_BREAK)) )
	{
		this->put_command(OP_GOTO3);
		this->cControl.break_target = prog->appendAddr( this->cControl.break_target );
	}
	else if( (node.childs()>0 && node[0].symbol() == PT_BREAK) )
	{	// accept break as synonym for "return <nothing>"
		this->put_value(basics::variant());
		this->put_command(OP_RETURN);
	}
	else
	{
		this->warning("keyword '%s' not allowed in this scope (line %u)\n", node[0].c_str(), node[0].line());
		return false;
	}
	return true;
}

bool eacompiler::compile_returnstm(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// 'return' <Expr List> ';'
	// 'end' ';'
	bool accept;
	if( node[0].is_terminal(PT_RETURN) )
	{
		accept = compile_main(node[1], scope, flags | CFLAG_RVALUE | CFLAG_LVALUE, uservalue);
		this->put_command(OP_RETURN);
	}
	else
	{
		accept = true;
		this->put_command(OP_END);
	}
	return accept;
}

bool eacompiler::compile_regexpr(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op If> '=~' RegExLiteral
	if( compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue) &&
		compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue) )
	{
		this->put_nonconst();
		
		// reserve a temp variable
		const basics::string<> varname("$re");
		if( !this->cVariable[varname].exists(CFLAG_TEMP) )
		{
			this->create_variable(node, varname, CFLAG_TEMP, scope, basics::VAR_AUTO, false, true);
			this->cVariable[varname][CFLAG_TEMP]->luse = 1;
			this->cVariable[varname][CFLAG_TEMP]->ruse = 2;
		}
		if( this->put_knownvariable(node, varname, CFLAG_LVALUE, 0, CFLAG_TEMP) )
		{
			this->put_command(OP_EMPTY);

			this->put_intcommand(OP_PUSH_INT, 2);
			this->put_strcommand(OP_FUNCTION, "regex");
			return true;
		}
	}
	return false;
}

bool eacompiler::compile_opassign(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op If> '='   <Op Assign>
	// <Op If> '+='  <Op Assign>
	// <Op If> '-='  <Op Assign>
	// <Op If> '*='  <Op Assign>
	// <Op If> '/='  <Op Assign>
	// <Op If> '^='  <Op Assign>
	// <Op If> '&='  <Op Assign>
	// <Op If> '|='  <Op Assign>
	// <Op If> '>>=' <Op Assign>
	// <Op If> '<<=' <Op Assign>
	// expecting 3 terminals in here, the first and third can be terminals or nonterminales
	// the second desides which assignment is to choose

	const unsigned short sym = node[1].symbol();
	// push the command
	command_t cmd=OP_ASSIGN;
	switch( sym )
	{
	case PT_EQ:			cmd = OP_ASSIGN; break;
	case PT_PLUSEQ:		cmd = OP_ADD_ASSIGN; break;
	case PT_MINUSEQ:	cmd = OP_SUB_ASSIGN; break;
	case PT_TIMESEQ:	cmd = OP_MUL_ASSIGN; break;
	case PT_DIVEQ:		cmd = OP_DIV_ASSIGN; break;
	case PT_PERCENTEQ:	cmd = OP_MOD_ASSIGN; break;
	case PT_CARETEQ:	cmd = OP_BIN_XOR_ASSIGN; break;
	case PT_AMPEQ:		cmd = OP_BIN_AND_ASSIGN; break;
	case PT_PIPEEQ:		cmd = OP_BIN_OR_ASSIGN; break;
	case PT_GTGTEQ:		cmd = OP_RSHIFT_ASSIGN; break;
	case PT_LTLTEQ:		cmd = OP_LSHIFT_ASSIGN; break;
	default: break;
	}

	// put the operand first then the variable
	bool accept=compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue) &&
		compile_main(node[0], scope, CFLAG_ASSIGN | CFLAG_LVALUE | ((sym==PT_EQ)?(flags&~CFLAG_RVALUE):(flags|CFLAG_RVALUE)), uservalue);

	if( cmd == OP_ASSIGN && node[0].symbol()==PT_CONCAT )
		cmd = OP_ARRAY_ASSIGN;
	this->put_command(cmd);
	return accept;
}


bool eacompiler::compile_opif(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op Or> '?' <Op If> ':' <Op If>
	// => if( or ) opif1 else opif2
	bool accept  = compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
	if( !accept )
	{	// nothing to do
	}
	else if( this->cConstvalues.size() )
	{	// decide on the const resule
		const bool is_true = this->cConstvalues.last().get_bool();
		this->cConstvalues.strip(1);
		accept = compile_main(node[is_true?2:4], scope, flags, uservalue);
	}
	else
	{
		int tempvalue=uservalue;
//		this->logging("Conditional Jump False -> Label1\n");
		this->put_command(OP_NIF3);
		size_t inspos1 = prog->appendAddr(0);	// placeholder
		// put in <Op If1>
		accept &= compile_main(node[2], scope, flags, tempvalue);
//		this->logging("Goto -> Label2\n");
		this->put_command(OP_GOTO3);
		size_t inspos2 = prog->appendAddr(0);	// placeholder
//		this->logging("Label1\n");
		// calculate and insert the correct jump offset
		prog->replaceAddr( prog->getCurrentPosition() ,inspos1);
		// put in <Op If2>
		accept &= compile_main(node[4], scope, flags, uservalue);
//		this->logging("Label2\n");
		prog->replaceAddr( prog->getCurrentPosition() ,inspos2);
	}
	return accept;
}

bool eacompiler::compile_opor(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op And> '||' <Op Or>
	//##TODO: add code reduction on const parameters
	bool accept  = compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
	if( !accept )
	{	// nothing to do
	}
	else if( this->cConstvalues.size() )
	{	// directly decide on the current result
		if( !this->cConstvalues.last().get_bool() )
		{	// first entry is constant false,
			// so we only need the second
			this->cConstvalues.strip(1);
			accept = compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
		}
		// otherwise
		// first entry is constant true,
		// so we can skip the second
		// but leave the boolean true on the const stack
	}
	else
	{
//		this->logging("Conditional Jump True or Pop -> Label1\n");
		this->put_command(OP_IF_POP3);
		size_t inspos1 = prog->appendAddr(0);	// placeholder
		accept &= compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
//		this->logging("Label1\n");
		// calculate and insert the correct jump 
		prog->replaceAddr( prog->getCurrentPosition() ,inspos1);
	}
	this->put_command(OP_BOOLEAN);
	return accept;
}

bool eacompiler::compile_opand(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op BinOR> '&&' <Op And>
	//##TODO: add code reduction on const parameters
	bool accept  = compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
	if( !accept )
	{	// nothing to do
	}
	else if( this->cConstvalues.size() )
	{	// directly decide on the current result
		if( this->cConstvalues.last().get_bool() )
		{	// first entry is constant true,
			// so we only need the second
			this->cConstvalues.strip(1);
			accept = compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
		}
		// otherwise
		// first entry is constant false,
		// so we can skip the second
		// but leave the boolean false on the const stack
	}
	else
	{
//		this->logging("Conditional Jump False or Pop -> Label1\n");
		this->put_command(OP_NIF_POP3);
		size_t inspos1 = prog->appendAddr(0);	// placeholder
		accept &= compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
//		this->logging("Label1\n");
		// calculate and insert the correct jump 
		prog->replaceAddr( prog->getCurrentPosition() ,inspos1);
	}
	this->put_command(OP_BOOLEAN);
	return accept;
}

bool eacompiler::compile_opbinary(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op BinAND> '&' <Op Equate>
	// <Op BinOr> '|' <Op BinXOR>
	// <Op BinXOR> '^' <Op BinAND>
	// <Op Equate> '==' <Op Compare>
	// <Op Equate> '!=' <Op Compare>
	// <Op Equate> '<>' <Op Compare>
	// <Op Compare> '>'  <Op Shift>
	// <Op Compare> '>=' <Op Shift>
	// <Op Compare> '<'  <Op Shift>
	// <Op Compare> '<=' <Op Shift>
	// <Op Shift> '<<' <Op AddSub>
	// <Op Shift> '>>' <Op AddSub>
	// <Op AddSub> '+' <Op MultDiv>
	// <Op AddSub> '-' <Op MultDiv>
	// <Op MultDiv> '*' <Op Unary>
	// <Op MultDiv> '/' <Op Unary>
	// <Op MultDiv> '%' <Op Unary>

	// put the operands on stack
	bool accept  =  compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue)
			&& compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
	switch( node[1].symbol() )
	{
	case PT_AMP:
//		this->logging("PT_OPBINAND\n");
		this->put_command(OP_BIN_AND);
		break;
	case PT_PIPE:
//		this->logging("PT_OPBINOR\n");
		this->put_command(OP_BIN_OR);
		break;
	case PT_CARET:
//		this->logging("PT_OPBINOR\n");
		this->put_command(OP_BIN_XOR);
		break;
	case PT_EQEQ:
//		this->logging("OP_EQUATE\n");
		this->put_command(OP_EQUATE);
		break;
	case PT_EXCLAMEQ:
	case PT_LTGT:
//		this->logging("OP_UNEQUATE\n");
		this->put_command(OP_UNEQUATE);
		break;
	case PT_GT:
//		this->logging("OP_ISGT\n");
		this->put_command(OP_ISGT);
		break;
	case PT_GTEQ:
//		this->logging("OP_ISGTEQ\n");
		this->put_command(OP_ISGTEQ);
		break;
	case PT_LT:
//		this->logging("OP_ISLT\n");
		this->put_command(OP_ISLT);
		break;
	case PT_LTEQ:
//		this->logging("OP_ISLTEQ\n");
		this->put_command(OP_ISLTEQ);
		break;
	case PT_LTLT:
//		this->logging("OP_ISLT\n");
		this->put_command(OP_LSHIFT);
		break;
	case PT_GTGT:
//		this->logging("OP_ISLTEQ\n");
		this->put_command(OP_RSHIFT);
		break;
	case PT_PLUS:
//		this->logging("PT_PLUS\n");
		this->put_command(OP_ADD);
		break;
	case PT_MINUS:
//		this->logging("PT_MINUS\n");
		this->put_command(OP_SUB);
		break;
	case PT_TIMES:
//		this->logging("PT_TIMES\n");
		this->put_command(OP_MUL);
		break;
	case PT_DIV:
//		this->logging("PT_DIV\n");
		this->put_command(OP_DIV);
		break;
	case PT_PERCENT:
//		this->logging("PT_PERCENT\n");
		this->put_command(OP_MOD);
		break;
	default:
		this->warning("unknown operation '%s' (line %u)\n", node[1].c_str(), node[1].line());
		accept = false;
		break;
	}// end switch
	return accept;
}

bool eacompiler::compile_opunary(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '-' <Op Unary>
	// '+' <Op Unary>
	// '~' <Op Unary>
	// '!' <Op Unary>
	bool accept  = compile_main(node[1], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
	// put the operands on stack
	switch( node[0].symbol() )
	{
	case PT_EXCLAM:
//		this->logging("PT_OPUNARY_NOT\n");
		this->put_command(OP_NOT);
		break;
	case PT_TILDE:
//		this->logging("PT_OPUNARY_INVERT\n");
		this->put_command(OP_INVERT);
		break;
	case PT_MINUS:
//		this->logging("PT_OPUNARY_NEGATE\n");
		this->put_command(OP_NEGATE);
		break;
	//case PT_PLUS:	// can just ignore
	//	break;		
	}// end switch
	return accept;
}

bool eacompiler::compile_oppost(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op Pointer> '++'
	// <Op Pointer> '--'
	// put the operands on stack
	const bool accept  = compile_main(node[0], scope, (flags & ~CFLAG_RVALUE) | CFLAG_LVALUE | CFLAG_NOASSIGN, uservalue);
	if( accept )
	{
		command_t cmd;
		if( (flags&CFLAG_RVALUE)!=CFLAG_RVALUE )
		{
			this->warning("prefere pre-operations when the result is not directly needed (line %u)\n", node.line());
			cmd = (node[1].symbol()==PT_MINUSMINUS)?OP_PRESUB:OP_PREADD;
		}
		else
		{
			cmd = (node[1].symbol()==PT_MINUSMINUS)?OP_POSTSUB:OP_POSTADD;
		}
		this->put_command(cmd);
	}
	return accept;
}

bool eacompiler::compile_oppre(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '++' <Op Pointer>
	// '--' <Op Pointer>
	// put the operands on stack
	const bool accept  = compile_main(node[1], scope, (flags & ~CFLAG_RVALUE) | CFLAG_LVALUE | CFLAG_NOASSIGN, uservalue);
	if( accept )
	{
		const command_t cmd = (node[0].symbol()==PT_MINUSMINUS)?OP_PRESUB:OP_PREADD;
		this->put_command(cmd);
	}
	return accept;
}

bool eacompiler::compile_opcast(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' <VarType> ')' <Op Unary>
	// put the operands on stack, first the value then the target type
	bool accept  = compile_main(node[3], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);

	if( PT_AUTO != node[1].symbol() )
	{	// don't need to cast to auto type
		if( PT_INT == node[1].symbol() )
		{
			this->put_command(OP_CAST_INTEGER);
		}
		else if( PT_STRING == node[1].symbol() )
		{
			this->put_command(OP_CAST_STRING);
		}
		else if( PT_DOUBLE == node[1].symbol() )
		{
			this->put_command(OP_CAST_FLOAT);
		}
		else
		{
			accept = false;
			this->warning("unrecognized cast target '%s' (line %u)\n", node[1].c_str(), node[1].line());
		}
	}
	return accept;
}

bool eacompiler::compile_opsizeof(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// sizeof '(' <VarType> ')'
	// sizeof '(' <Expr> ')'
	// put the operands on stack
	bool accept = true;
	switch( node[2].symbol() )
	{
	case PT_VARTYPE:
		// vartypes are '1' in size by default
		this->put_value((int64)1);
	case PT_EXPR:
		// empty expression node is zero size
		this->put_value((int64)0);
	default:
	{	// compile the node
		accept  = compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
		if( is_const() )
		{	// evaluate the expression here
			int64 val = this->cConstvalues.last().size();
			this->cConstvalues.strip(1);
			this->put_value( val );
		}
		else
		{	// need to evaluate at runtime
			this->put_command(OP_SIZEOF);
		}
		break;
	}
	}// end switch
//	this->logging("PT_OPSIZEOF ID, \n");
	return accept;
}

bool eacompiler::compile_memberfunc(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op Pointer> '.'  <Func Call>
	// put the operands on stack
	return  compile_main(node[0], scope, (flags & ~CFLAG_RVALUE) | CFLAG_LVALUE, uservalue) &&	// base variable
			compile_main(node[2], scope, flags | CFLAG_MEMBER, uservalue);	// member function
}

bool eacompiler::compile_membervariable(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op Pointer> '.'  identifier
	if( compile_main(node[0], scope, (flags & ~CFLAG_RVALUE) | CFLAG_LVALUE, uservalue) )
	{
		this->put_strcommand(OP_MEMBER, node[2].string());
		return true;
	}
	return false;
}

bool eacompiler::compile_scopevariable(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op Pointer> '::' identifier
	bool accept = false;
	int usertemp = 0;
	const parse_node &childnode = node[0];
	// first node
	if( childnode.symbol()==PT_SCOPEVAR )
	{	// can be a scope var
		accept = compile_main(childnode, scope, flags | CFLAG_LVALUE | CFLAG_GLOBAL, usertemp);
	}
	else if( childnode.symbol()==PT_IDENTIFIER )
	{	// or an identifer, which has to be global
		accept = this->variable_scopename2id(childnode.string(), usertemp);
		if(!accept)
			this->warning("variable scope name '%s' is invalid (line %u)\n", childnode.string().c_str(), node.line());
	}
	else
	{	// otherwise the scope variable is invalid
		this->warning("invalid scope variable (line %u)\n", node.line());
	}
	if( accept )
	{
		// second node
		if( flags&CFLAG_GLOBAL )
		{	// central nodes need to be global
			accept = this->variable_scopename2id(node[2].string(), usertemp);
			if(!accept)
				this->warning("variable scope name is invalid (line %u)\n", node.line());
			uservalue = usertemp;
		}
		else
		{	// access the variable
			// be aware that global names are accepted as variable names
			usertemp = this->variable_correct_access(usertemp);
			accept = put_knownvariable(node[2], node[2].string(), flags, scope, usertemp);
		}
	}
	return accept;
}

bool eacompiler::compile_array(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Op Pointer> '[' <Range> ']'
	const parse_node &childnode = node[2];
	bool accept = compile_main(node[0], scope, flags | CFLAG_LVALUE, uservalue) &&
				compile_main(childnode, scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
	if(accept)
	{
		if( flags&CFLAG_VARCREATE )
		{
			if( childnode.is_nonterminal(PT_RANGE) ||
				childnode.is_nonterminal(PT_SPLICE) ||
				childnode.is_nonterminal(PT_DUPLICATE) ||
				childnode.is_nonterminal(PT_EXPR) ||
				uservalue>1)
			{
				this->warning("invalid variable range (line %u)\n", node.line());
				accept = false;
			}
			else
			{
				this->put_intcommand(OP_CREATEARRAY,1);
			}
		}
		else if( childnode.is_nonterminal(PT_RANGE) )
		{	// <Expr> ':' <Expr>
			this->put_command(OP_RANGE);
		}
		else if( childnode.is_nonterminal(PT_SPLICE) )
		{	// <Expr>':'<Expr>':'<Expr>
			this->put_command(OP_SPLICE);
		}
		else if( childnode.is_nonterminal(PT_DUPLICATE) )
		{	// <Expr>':'<Expr>':*'<Expr>
			this->put_command(OP_DULICATE);
		}
		else
		{	// <Expr List>
			// uservalue contains the number of listelements
			// if there was a expression list
			if( uservalue>1 )
			{
				this->put_intcommand(OP_ARRAYSEL, uservalue);
			}
			else
			{	// single element or none
				this->put_command(OP_ARRAY);
			}
		}
	}
	return accept;
}

bool eacompiler::compile_range(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Expr> ':' <Expr>
	return
		compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue) &&
		compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
}

bool eacompiler::compile_rangemod(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Expr> ':' <Expr> ':' <Expr>
	// <Expr> ':' <Expr> ':*' <Expr>
	return
		compile_main(node[0], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue) &&
		compile_main(node[2], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue) &&
		compile_main(node[4], scope, (flags & ~CFLAG_LVALUE) | CFLAG_RVALUE, uservalue);
}

bool eacompiler::compile_eval(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '(' <Expr List> ')'
	// go down for righthand as well as for lefthand expressions
	const parse_node &exprlist = node[1];
	if( compile_main(exprlist, scope, flags, uservalue) )
	{
		if( exprlist.symbol()==PT_EXPRLIST && uservalue>1 )
			this->put_intcommand( OP_REDUCE, uservalue);
		if(flags&CFLAG_RVALUE)
			this->put_command(OP_EVAL);
		return true;
	}
	return false;
}

bool eacompiler::compile_concat(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// '{' <Expr> ',' <Expr List> '}'
	// '{' <Op Assign> '}'
	// '{' <RegExpr> '}'
	if(	compile_main(node[1], scope, flags, uservalue) )
	{
		if( node.childs() > 3 )
		{
			const parse_node &exprlist = node[3];
			if( !compile_main(exprlist, scope, flags, uservalue) )
				return false;
			
			// only put concat command when more than one element
			this->put_intcommand(OP_CONCAT, (exprlist.symbol()==PT_EXPRLIST)?1+uservalue:2);
		}
		return true;
	}
	return false;
}



bool eacompiler::compile_speclist(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{
	bool accept = true;
	int usertemp = 0;
	size_t i;
	for(i=0; accept && i<node.childs() && accept; ++i)
	{
		const parse_node &childnode = node[i];
		if( !childnode.is_terminal(PT_COMMA) && !childnode.is_terminal(PT_SEMI) )
		{
			const bool is_list = childnode.is_nonterminal(PT_SPECLIST);
			accept = compile_main(childnode, scope, flags, uservalue);
			usertemp += (is_list)?usertemp:1;
		}
	}
	uservalue = usertemp;
	return accept;
}

bool eacompiler::compile_commalist(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Param> ',' <Params> or <Param>
	// <Variable> ',' <VarList> or <Variable>
	bool accept = true;
	int usertemp = 0;
	accept = compile_main(node[0], scope, flags, uservalue);
	if(accept)
	{
		const parse_node &child = node[2];
		const bool is_list = child.is_nonterminal(PT_EXPRLIST) ||
							 child.is_nonterminal(PT_PARAMS) ||
							 child.is_nonterminal(PT_VARLIST);
		++usertemp;
		if( is_list )
		{
			accept = compile_commalist(child, scope, flags, uservalue);
			usertemp+=uservalue;
		}
		else
		{
			compile_main(child, scope, flags, uservalue);
			++usertemp;
		}
	}
	uservalue = usertemp;
	return accept;
}

bool eacompiler::compile_statements(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Stm List>  ::=  <Stm> <Stm List> or empty
	bool accept = true;
	if( node.childs() )
	{
		accept = compile_main(node[0], scope, flags, uservalue) &&
				 compile_statements(node[1], scope, flags, uservalue);
		++uservalue;
	}
	else
	{
		uservalue=0;
	}
	return accept;
}


bool eacompiler::compile_declarations(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{	// <Decls> ::= <Decl> <Decls> or empty
	bool accept = true;
	if( node.childs() )
	{
		accept = compile_main(node[0], scope, flags, uservalue) &&
				 compile_declarations(node[1], scope, flags, uservalue);
		++uservalue;
	}
	else
	{
		uservalue=0;
	}
	return accept;
}



//////////////////////////////////////////////////////////////////////////
// main compile loop, is called recursively with all parse tree nodes
bool eacompiler::compile_main(const parse_node &node, uint scope, unsigned long flags, int& uservalue)
{
	size_t i;
	bool accept;
	///////////////////////////////////////////////////////////////////////
	if( node.type()==1 )
	{	// terminals
		accept = true;
		switch( node.symbol() )
		{
		case PT_DECLITERAL:
		case PT_BINLITERAL:
		case PT_OCTLITERAL:
		case PT_HEXLITERAL:
		case PT_BOOLFALSE:
		case PT_BOOLTRUE:
		{	// {digit}*
			// 0b{Bin Digit}+
			// 0o{Oct Digit}+
			// 0x{Hex Digit}+
			accept = compile_numberliteral(node, scope, flags, uservalue);

//			this->logging("PT_<NUMBER>LITERAL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_FLOATLITERAL:
		{	// ({Digit}* '.' {Digit}+ (([eE][+-]?{Digit}+)|[TGMKkmunpfa])?)
			// ({Digit}+ '.' {Digit}* (([eE][+-]?{Digit}+)|[TGMKkmunpfa])?)
			// ({Digit}+              (([eE][+-]?{Digit}+)|[TGMKkmunpfa]) )
			accept = compile_floatliteral(node, scope, flags, uservalue);

//			this->logging("PT_FLOATLITERAL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_CHARLITERAL:
		{	// '' ( {Char Ch}   | ('\'{&20 .. &FF}) )   ''
			accept = compile_charliteral(node, scope, flags, uservalue);

//			this->logging("PT_CHARLITERAL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_STRINGLITERAL:
		case PT_REGEXLITERAL:
		{	// '"'( {String Ch} | ('\'{&20 .. &FF}) )* '"'
			// '/' ( {RegEx Start Ch} | '\/' )( {RegEx Ch} | '\/' )* '/'
			accept = compile_stringliteral(node, scope, flags, uservalue);

//			this->logging("PT_STRINGLITERAL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_MINUS:
		{	// '-'
			// 'default'
			accept = compile_emptyliteral(node, scope, flags, uservalue);

//			this->logging("PT_EMPTYVAL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_IDENTIFIER:
		{	// {Id Head}{Id Tail}*
			accept = compile_identifier(node, scope, flags, uservalue);

//			this->logging("PT_IDENTIFIER - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_SEMI:
		{	
			accept = compile_semiliteral(node, scope, flags, uservalue);

//			this->logging("clear stack\n");
			break;
		}
		default:
			accept = false;
			this->logging("unknown terminal: ");
			this->logging("<%s>: %s\n", node.name(), node.c_str());
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////
	// nonterminals
	else if( node.childs()==1 )
	{	// only one child, just go down, should not happen actually
		accept = compile_main(node[0], scope, flags, uservalue);
	}
	else if( 0 != (flags&CFLAG_CONST) && is_nonconst_operation(node) )
	{	// error when using nonconst operations in const scopes
		this->warning("cannot be used inside const statements (line %u)\n", node.line());
		accept = false;
	}
	else
	{	// process the childs depending on this node
		accept = false;
		switch(node.symbol())
		{
		case PT_DEFINEDECL:
		{	// 'define' identifier '=' <Op If> ';'
			accept = compile_define(node, scope, flags, uservalue);

//			this->logging("PT_DEFINEDECL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_INCLUDEDECL:
		{	// 'include' StringLiteral ';'
			accept = compile_include(node, scope, flags, uservalue);

//			this->logging("PT_INCLUDEDECL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_FUNCDECL:
		{	// <VarType> identifier '(' <Paramse> ')' ';'
			// <VarType> identifier '(' <Paramse> ')' <Block>
			accept = compile_funcdecl(node, scope, flags, uservalue);
			break;
		}
		case PT_SUBDECL:
		{	// 'function' <Func Decl>
			accept = compile_subfuncdecl(node, scope, flags, uservalue);
			break;
		}
		case PT_OBJDECL:
		{	// <Obj Type> <Obj Id> <Obj List> <Obj Script>
			accept = compile_object(node, scope, flags, uservalue);
			break;
		}
		case PT_OBJLIST:
		{	// <Obj Inst> <Obj List>
			accept = compile_objlist(node, scope, flags, uservalue);
			break;
		}
		case PT_OBJINST:
		{	// '(' <Spec List> ')'
			accept = compile_instance(node, scope, flags, uservalue);
			break;
		}
		case PT_ORDERNPC:
		{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ')'
			accept = compile_ordernpc(node, scope, flags, uservalue);
			break;
		}
		case PT_ORDERTOUCH:
		{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ')'
			accept = compile_ordertouch(node, scope, flags, uservalue);
			break;
		}
		case PT_ORDERMOB:
		{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ')'
			accept = compile_ordermob(node, scope, flags, uservalue);
			break;
		}
		case PT_ORDERWARP:
		{	// '(' StringLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' DecLiteral ',' StringLiteral ',' DecLiteral ',' DecLiteral ')'
			accept = compile_orderwarp(node, scope, flags, uservalue);
			break;
		}
		case PT_ORDERSCR:
		{	// '(' StringLiteral ')'
			accept = compile_orderscr(node, scope, flags, uservalue);
			break;
		}
		case PT_SPECLIST:
		{	// <Spec Item> ',' <Spec List>
			// <Spec Item> ';' <Spec List>
			// <Spec Item>     <Spec List>
			accept = compile_speclist(node, scope, flags, uservalue);
			break;
		}
		case PT_SPECITEM:
		{	// identifier '=' <Op If>
			accept = compile_specitem(node, scope, flags, uservalue);
			break;
		}
		case PT_OBJSCRIPT:
		{	// <SubProg>  ';'
			// <ItemList> ';'
			// handled within object declaration
			break;
		}
		case PT_SUBPROG:
		{	// identifier '::' identifier
			// handled within the parents
			break;
		}
		case PT_ITEMOVERWRITE:
		{	// DecLiteral    ':' DecLiteral
			// StringLiteral ':' DecLiteral
			// identifier    ':' DecLiteral
			accept = compile_itemoverwrite(node, scope, flags, uservalue);
			break;
		}
		case PT_PARAMSE:
		{	// empty
			// nothing to do here
			break;
		}
		case PT_PARAM:
		{	// <constopt> <VarTypeopt> identifier
			// <constopt> <VarTypeopt> identifier '=' <Const Unary>
			// ['const']  ['string']
            // ['const']  ['double']
            // ['const']  ['int']
            // ['const']  ['auto']
            // ['const']  ['var']
			accept = compile_parameter(node, scope, flags, uservalue);
			break;
		}
		case PT_VARDECL:
		{	// <VarType> <VarList>  ';'
			accept = compile_vardecl(node, scope, flags, uservalue);
			break;
		}
		case PT_VARIABLE:
		{	// <Op Pointer> <VarAssign>
			accept = compile_variable(node, scope, flags, uservalue);
			break;
		}
		case PT_VARASSIGN:
		{	// '=' <Expr>
			// empty
			// done inside compile_variable
			break;
		}
		case PT_BLOCK:
		{	// '{' <Stm List> '}'
			accept = compile_block(node, scope, flags, uservalue);
			break;
		}
		case PT_LABELSTM:
		{	// identifier ':'
			accept = compile_label(node, scope, flags, uservalue);

//			this->logging("PT_LABELSTM - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_IFSTM:
		{	// 'if' '(' <Expr> ')' <Normal Stm>
			accept = compile_ifstm(node, scope, flags, uservalue);
			break;
		}
		case PT_WHILESTM:
		{	// 'while' '(' <Expr> ')' <Normal Stm>
			accept = compile_whilestm(node, scope, flags, uservalue);
			break;
		}
		case PT_FORSTM:
		{	// 'for' '(' <Expr List> ';' <Expr> ';' <Expr List> ')' <Normal Stm>
			accept = compile_forstm(node, scope, flags, uservalue);
			break;
		}
		case PT_DOSTM:
		{	// 'do' <Normal Stm> 'while' '(' <Expr> ')' ';'
			accept = compile_dostm(node, scope, flags, uservalue);
			break;
		}
		case PT_SWITCHSTM:
		{	// 'switch' '(' <Expr> ')' '{' <Case Stm> '}'
			accept = compile_switchstm(node, scope, flags, uservalue);
			break;
		}
		case PT_CASESTM:
		{	// 'case' <Expr List> ':' <Stm List> <Case Stm>
			// 'default' ':' <Stm List> <Case Stm>
			// empty
			// nothing here is handled within the PT_SWITCHSTM
			break;
		}
		case PT_GOTOSTM:
		{	// 'goto' identifier ';'
			accept = compile_goto(node, scope, flags, uservalue);
			break;
		}
		case PT_GOSUBSTM:
		{	// 'gosub' identifier ';'
			// 'gosub' <SubProg> ';'
			accept = compile_gosub(node, scope, flags, uservalue);
			break;
		}
		case PT_LCTRSTM:
		{	// 'break' ';'
			// 'continue' ';'
			accept = compile_lctrlstm(node, scope, flags, uservalue);
			break;
		}
		case PT_RETURNSTM:
		{	// 'return' <Expr List> ';'
			// 'end' ';'
			accept = compile_returnstm(node, scope, flags, uservalue);

//			this->logging("PT_RETURNSTM - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_EXPRSTM:
		{	// <Expr List> ';'
			accept = compile_main(node[0], scope, flags, uservalue);
			this->put_command(OP_POP);
			break;
		}
		case PT_FUNCCALL:
		{	// identifier '(' <Expr List> ')'
			accept = compile_function_call(node, scope, flags, uservalue);

//			this->logging("PT_FUNCCALL - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_EXPR:
		{	// push an empty value
			this->put_value( basics::variant() );
			accept = true;
			break;
		}
		case PT_REGEXPR:
		{	// <Op If> '=~' RegExLiteral
			accept  =	compile_regexpr(node, scope, flags, uservalue);
			break;
		}
		case PT_OPASSIGN:
		{	// <Op If> '='   <Op Assign>
			// <Op If> '+='  <Op Assign>
			// <Op If> '-='  <Op Assign>
			// <Op If> '*='  <Op Assign>
			// <Op If> '/='  <Op Assign>
			// <Op If> '^='  <Op Assign>
			// <Op If> '&='  <Op Assign>
			// <Op If> '|='  <Op Assign>
			// <Op If> '>>=' <Op Assign>
			// <Op If> '<<=' <Op Assign>
			accept = compile_opassign(node, scope, flags, uservalue);

//			this->logging("PT_OPASSIGN - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_OPIF:
		{	// <Op Or> '?' <Op If> ':' <Op If>
			// => if( or ) opif1 else opif2
			accept = compile_opif(node, scope, flags, uservalue);

//			this->logging("PT_OPIF - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_OPOR:
		{	// <Op And> '||' <Op Or>
			accept = compile_opor(node, scope, flags, uservalue);
			break;
		}
		case PT_OPAND:
		{	// <Op BinOR> '&&' <Op And>
			//##TODO: add code reduction on const parameters
			accept = compile_opand(node, scope, flags, uservalue);
			break;
		}
		case PT_OPBINOR:
		case PT_OPBINXOR:
		case PT_OPBINAND:
		case PT_OPEQUATE:
		case PT_OPCOMPARE:
		case PT_OPSHIFT:
		case PT_OPADDSUB:
		case PT_OPMULTDIV:
		{	// <Op BinAND> '&' <Op Equate>
			// <Op BinOr> '|' <Op BinXOR>
			// <Op BinXOR> '^' <Op BinAND>
			
			// <Op Equate> '==' <Op Compare>
			// <Op Equate> '!=' <Op Compare>
			// <Op Equate> '<>' <Op Compare>
			// <Op Compare> '>'  <Op Shift>
			// <Op Compare> '>=' <Op Shift>
			// <Op Compare> '<'  <Op Shift>
			// <Op Compare> '<=' <Op Shift>
			// <Op Shift> '<<' <Op AddSub>
			// <Op Shift> '>>' <Op AddSub>
			// <Op AddSub> '+' <Op MultDiv>
			// <Op AddSub> '-' <Op MultDiv>
			// <Op MultDiv> '*' <Op Unary>
			// <Op MultDiv> '/' <Op Unary>
			// <Op MultDiv> '%' <Op Unary>

			// put the operands on stack
			accept  =  compile_opbinary(node, scope, flags, uservalue);
			break;
		}
		case PT_OPUNARY:
		{	// '-' <Op Unary>
			// '+' <Op Unary>
			// '~' <Op Unary>
			// '!' <Op Unary>
			accept  = compile_opunary(node, scope, flags, uservalue);
			break;
		}
		case PT_OPPOST:
		{	// <Op Pointer> '++'
			// <Op Pointer> '--'
			accept  = compile_oppost(node, scope, flags, uservalue);
			break;
		}
		case PT_OPPRE:
		{	// '++' <Op Pointer>
			// '--' <Op Pointer>
			accept  = compile_oppre(node, scope, flags, uservalue);
			break;
		}
		case PT_OPCAST:
		{	// '(' <VarType> ')' <Op Unary>
			accept  = compile_opcast(node, scope, flags, uservalue);

//			this->logging("PT_OPCAST - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_OPSIZEOF:
		{	// sizeof '(' <VarType> ')'
			// sizeof '(' <Expr> ')'
			accept = compile_opsizeof(node, scope, flags, uservalue); 
			break;
		}
		case PT_MEMBERFUNC:
		{	// <Op Pointer> '.'  <Func Call>
			accept = compile_memberfunc(node, scope, flags, uservalue); 

//			this->logging("PT_MEMBERFUNC - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_SCOPEFUNC:
		{	// <Op Pointer> '::'  <Func Call>
			accept = compile_subfunction_call(node, scope, flags, uservalue);

//			this->logging("PT_MEMBERFUNC - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_GLOBALFUNC:
		{	//  '::'  <Func Call>
			accept = compile_globalfunction_call(node, scope, flags, uservalue);

//			this->logging("PT_MEMBERFUNC - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_MEMBERVAR:
		{	// <Op Pointer> '.'  identifier
			accept = compile_membervariable(node, scope, flags, uservalue); 

//			this->logging("PT_MEMBERVAR - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_SCOPEVAR:
		{	// <Op Pointer> '::' identifier
			accept = compile_scopevariable(node, scope, flags, uservalue); 

//			this->logging("PT_SCOPEVAR - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_ARRAY:
		{	// <Op Pointer> '[' <Range> ']'
			accept = compile_array(node, scope, flags, uservalue); 

//			this->logging("PT_ARRAYACCESS - ");
//			this->logging(accept?"accept ":"error ");
//			node.print();
//			this->logging("\n");
			break;
		}
		case PT_RANGE:
		{	// <Expr> ':' <Expr>
			accept = compile_range(node, scope, flags, uservalue); 
			break;
		}
		case PT_SPLICE:
		case PT_DUPLICATE:
		{	// <Expr> ':' <Expr> ':' <Expr>
			// <Expr> ':' <Expr> ':*' <Expr>
			accept = compile_rangemod(node, scope, flags, uservalue); 
			break;
		}
		case PT_EVAL:
		{	// '(' <Expr List> ')'
			accept = compile_eval(node, scope, flags, uservalue); 
			break;
		}
		case PT_CONCAT:
		{	// '{' <Expr> ',' <Expr List> '}'
			accept = compile_concat(node, scope, flags, uservalue); 
			break;
		}
		///////////////////////////////////////////////////////////////////
		// comma seperated operands
		case PT_EXPRLIST:
		case PT_PARAMS:
		case PT_VARLIST:
		case PT_ITEMLIST:
		{	// <Expr> ',' <Expr List>
			// <Param> ',' <Params>
			// <Variable> ',' <VarList>
			// <ItemOverwrite> ',' <ItemList>
			accept = eacompiler::compile_commalist(node, scope, flags, uservalue);
			break;
		}
		///////////////////////////////////////////////////////////////////
		// just operand lists
		case PT_STMLIST:
		{	// check if childs are accepted
			accept = compile_statements(node, scope, flags, uservalue);
			break;
		}
		case PT_DECLS:
		{	// check if childs are accepted
			accept = compile_declarations(node, scope, flags, uservalue);
			break;
		}
		///////////////////////////////////////////////////////////////////
		// default prints unhandled statements
		default:
		{
//			this->logging("NTerm - ");
//			this->logging("%s - %s\n", node.name(), node.c_str());
			// go through the childs
			accept = true;
			for(i=0; i<node.childs(); ++i)
			{
				accept = compile_main(node[i], scope, flags, uservalue);
				if( !accept ) break;
			}
			break;
		}
		}// switch
	}
	///////////////////////////////////////////////////////////////////////
	return accept;
}


///////////////////////////////////////////////////////////////////////////////
/// compiler entry point.
bool eacompiler::compile(const parse_node &node)
{
	int uservalue=0;
	return this->compile_main(node, 0, CFLAG_DECL, uservalue);
}


///////////////////////////////////////////////////////////////////////////////
bool eacompiler::compile_file(const basics::string<>& filename, int option, bool forced)
{
	if( !basics::file_exists(filename) )
	{
		this->warning("file '%s' does not exist\n", filename.c_str(), this->cFile->c_str());
		return false;
	}
	else if( this->loadingfiles.find(filename) )
	{
		fprintf(stderr, "circle reference with file '%s'\n", filename.c_str());
		return false;
	}

	
	scriptfile::scriptfile_ptr safefile = this->cFile;
	// get the current file (creates when necessary)
	this->cFile = scriptfile::create(filename);
	if(!this->cFile->is_modified() && !forced)
		return true;

	this->cCompileOptions=option;
	// create parser
	basics::CParser_CommentStore parser(&this->cParseConfig);
	bool ok = true;
	bool run = true;
	// open input file
	if( !parser.open(filename) )
	{
		fprintf(stderr, "Could not open input file '%s'\n", filename.c_str());
		return false;
	}
	fprintf(stderr, "processing input file '%s'\n", filename.c_str());



	// remember the name
	this->loadingfiles.insert(filename);

	//save the current define set
	scriptdefines tmpdefine =  this->cDefines_;

	while(run)
	{
		short p = parser.parse(PT_DECL);
		if (p < 0)
		{	// an error
			parser.print_expects(filename);
			run = false;
			ok = false;
		}
		else if(0 == p)
		{	// finished
			run = false;
		}
		
		if( ok && parser.rt[0].symbol.idx==PT_DECL && parser.rt[0].cChildNum )
		{
			basics::CStackElement *child = &(parser.rt[parser.rt[0].cChildPos]);
			if( child &&
				( child->symbol.idx == PT_BLOCK ||
				  child->symbol.idx == PT_FUNCDECL ||
				  child->symbol.idx == PT_OBJDECL ||
				  child->symbol.idx == PT_DEFINEDECL ||
				  child->symbol.idx == PT_INCLUDEDECL
				  )
			  )
			{
				if( (option&OPT_BEAUTIFY)==OPT_BEAUTIFY )
				{
					eaprinter prn;
					prn.scope = 0; 
					prn.print_beautified(parser, 0);
					prn.print_comments(parser, -1);
					prn << '\n';
				}
				if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
				{	// print parse tree
					fprintf(stderr, "(%lu)----------------------------------------\n", (unsigned long)parser.rt.size());
					parser.print_rt_tree(0,0, false);
				}
				if( (option&OPT_TRANSFORM)==OPT_TRANSFORM )
				{	// print a transformation tree
					transformer pnode(parser);
					pnode.print_tree();
				}
				if( (option&OPT_COMPILE)==OPT_COMPILE ||
					(option&OPT_COMPILEOUTPUT)==OPT_COMPILEOUTPUT )
				{	// compiling
					ok=run = this->compile(parser);

					//##TODO: 
					// insert compile results in scriptfile

				}
				//////////////////////////////////////////////////////////
				// reinitialize parser
				parser.reinit();
			}
		}
	}
	parser.reset();

	// restore the define set
	this->cDefines_ = tmpdefine;

	// remove the name
	size_t pos;
	if( this->loadingfiles.find(filename, 0, pos) )
		this->loadingfiles.removeindex(pos);

	scriptfile::to_binary(this->cFile);
	
	// restore the file
	this->cFile = safefile;

	return ok;
}


