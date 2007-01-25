// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "eaopcode.h"
#include "eaprogram.h"
#include "eabuildin.h"



/// lookup for progs with names
basics::smap<basics::string<>, scriptprog::script>	scriptprog::cNamedProgs;

// protection for the static
static struct _delete_notify
{
	bool deleted;

	_delete_notify() : deleted(false)
	{}
	~_delete_notify()
	{
		deleted=true;
	}
} delete_notify;


///////////////////////////////////////////////////////////////////////////
// get  script by name
scriptprog::script scriptprog::get_script(const basics::string<>& name)
{
	const script* pscr = scriptprog::cNamedProgs.search(name);
	return (pscr)?*pscr:script();
}

///////////////////////////////////////////////////////////////////////////
// register script
bool scriptprog::regist(script& scr)
{
	if( 0==scr->cName.size() )
	{	// when no name available, it's only used at the instances
		return true;
	}
	else if( scriptprog::cNamedProgs.exists(scr->cName) )
	{	// conflicting name
		fprintf(stderr, "programm with name '%s' already exists\n"
			"ignoring name, script will be not available for external duplication\n", 
			scr->cName.c_str()); 
		scr->cName.clear();
		return true;
	}
	else
	{	// ok, so insert it
		scriptprog::cNamedProgs.insert(scr->cName, scr);
		basics::map<basics::string<>, scriptdecl>::iterator iter(scr->cHeader);
		for(; iter; ++iter)
		{
			buildin::create(scr->cName+"::"+iter->key, &iter->data);
			if(iter->key=="main")
			{	// register the main function also with the plain name
				buildin::create(scr->cName, &iter->data);
			}
		}
		return true;
	}
}
///////////////////////////////////////////////////////////////////////////
// unregister script
void scriptprog::unregist() const
{
	if( this->cName.size() && !delete_notify.deleted )
	{
		scriptprog::cNamedProgs.erase(this->cName);
	}
	// also unregister the declarations
	buildin::erase(this->cName);
	basics::map<basics::string<>, scriptdecl>::iterator iter(this->cHeader);
	for(; iter; ++iter)
	{
		buildin::erase(this->cName+"::"+iter->key);
	}
}



///////////////////////////////////////////////////////////////////////////////
// fetch command and parameters; and go to next command
bool scriptprog::getCommand(size_t &inx, CCommand& cmd) const
{
	cmd.cCommand = getCommand(inx);
	switch( cmd.cCommand )
	{
		// commands followed by an int (1 byte)
		case OP_PUSH_INT1:
		case OP_PUSH_TEMPVAR1:
		case OP_PUSH_TEMPVAL1:
		case OP_PUSH_PARAVAR:
		case OP_PUSH_PARAVAL:
		case OP_CONCAT1:
		case OP_CREATEARRAY:
		case OP_CAST:
		case OP_ARRAYSEL:
			cmd.cParam1 = getChar(inx);
			break;
		// commands followed by an int (2 byte)
		case OP_PUSH_INT2:
		case OP_PUSH_TEMPVAR2:
		case OP_PUSH_TEMPVAL2:
		case OP_CONCAT2:
			cmd.cParam1 = getShort(inx);
			break;
		// commands followed by an int (3 byte)
		case OP_PUSH_INT3:
		case OP_CONCAT3:
			cmd.cParam1 = getAddr(inx);
			break;
		// commands followed by an int or float (4 byte)
		case OP_PUSH_INT4:
		case OP_CONCAT4:
		case OP_PUSH_FLOAT:
		case OP_START:
			cmd.cParam1 = getInt(inx);
			break;
		// commands followed by an address
		case OP_NIF:
		case OP_IF:
		case OP_IF_POP:
		case OP_NIF_POP:
		case OP_GOTO:
		case OP_GOSUB:
		case OP_PUSH_ADDR:
			cmd.cParam1 = getAddr(inx);
			break;

		// commands followed by an int (1 byte) and a second unsigned char param
		case OP_FUNCTION:
			cmd.cString = get_string(inx);
			cmd.cParam1 = getChar(inx);
			break;

		// commands followed by a string
		case OP_PUSH_VAR:
		case OP_PUSH_VAL:
		case OP_PUSH_STRING:
			cmd.cString = get_string(inx);
			break;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
//
size_t scriptprog::nextCommand(size_t pos) const
{
	const int cmd = getCommand(pos);
	switch( cmd )
	{
	// commands with no parameters
	case OP_NOP:
	case OP_ASSIGN:
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
	case OP_RANGE:
	case OP_SPLICE:
	case OP_DULICATE:
	case OP_PUSH_NONE:
	case OP_CLEAR:
	case OP_POP:
	case OP_EVAL:
	case OP_BOOLEAN:
	case OP_END:
	case OP_RETURN:
		return pos;

	// commands with int parameters (1 bytes)
	case OP_PUSH_INT1:
	case OP_PUSH_TEMPVAR1:
	case OP_PUSH_TEMPVAL1:
	case OP_PUSH_PARAVAR:
	case OP_PUSH_PARAVAL:
	case OP_CONCAT1:
	case OP_CREATEARRAY:
	case OP_CAST:
	case OP_ARRAYSEL:
		return pos+1;

	// commands with int parameters (2 bytes)
	case OP_PUSH_INT2:
	case OP_PUSH_TEMPVAR2:
	case OP_PUSH_TEMPVAL2:
	case OP_CONCAT2:
		return pos+2;

	// commands with int parameters (3 bytes)
	case OP_PUSH_INT3:
	case OP_CONCAT3:
		return pos+3;

	// commands with int or float parameters (4 bytes)
	case OP_START:
	case OP_PUSH_FLOAT:
	case OP_PUSH_INT4:
	case OP_CONCAT4:
		return pos+4;

	// commands with int64
	case OP_PUSH_INT8:
		return pos+8;

	// commands with addr parameters
	case OP_NIF:
	case OP_IF:
	case OP_IF_POP:
	case OP_NIF_POP:
	case OP_GOTO:
	case OP_GOSUB:
	case OP_PUSH_ADDR:
		return pos+3;

	// commands with string parameter and 1 byte parameter count
	case OP_FUNCTION:
		return pos+4;

	// commands with string parameter
	case OP_PUSH_STRING:
	case OP_PUSH_VAR:
	case OP_PUSH_VAL:
		pos+=3;
		return pos;
	}
	printf("missing command entry %i\n", cmd);
	return pos;
}



///////////////////////////////////////////////////////////////////////////
// label functions
bool scriptprog::create_label(const basics::string<>& name, uint pos)
{
	return cLabels.insert(name, pos);
}
bool scriptprog::is_label(const basics::string<>& name) const
{
	return cLabels.exists(name);
}
uint scriptprog::get_labeltarget(const basics::string<>& name) const
{	
	const uint* ppos = cLabels.search(name);
	return ppos?*ppos:0;
}
scriptdecl scriptprog::get_declaration(const basics::string<>& name) const
{	
	const scriptdecl* decl = cHeader.search(name);
	return decl?*decl:scriptdecl();
}


///////////////////////////////////////////////////////////////////////////
// replacing temporary jump targets
bool scriptprog::replaceJumps(size_t start, size_t end, uchar cmd, int val)
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
size_t scriptprog::append(const scriptprog& p)
{
	size_t pos = this->getCurrentPosition();
	this->cProgramm.append(p.cProgramm);
	return pos;
}

///////////////////////////////////////////////////////////////////////////
// access functions
unsigned char scriptprog::getCommand(size_t &inx) const
{
	if( inx < cProgramm.size() )
		return cProgramm[inx++];
	return OP_END;
}
size_t scriptprog::insertCommand(unsigned char val, size_t &inx)
{
	size_t pos = inx;
	cProgramm.insert(val, 1, inx++);
	return pos;
}
size_t scriptprog::replaceCommand(unsigned char val, size_t &inx)
{
	size_t pos = inx;
	cProgramm[inx++] = val;
	return pos;
}
size_t scriptprog::appendCommand(unsigned char val)
{
	size_t pos = cProgramm.size();
	cProgramm.append(val);
	return pos;
}
///////////////////////////////////////////////////////////////////////////
unsigned char scriptprog::getChar(size_t &inx) const
{
	if( inx < cProgramm.size() )
		return cProgramm[inx++];
	return 0;
}
size_t scriptprog::insertChar(unsigned char val, size_t &inx)
{
	size_t pos = inx;
	cProgramm.insert(val, 1, inx++);
	return pos;
}
size_t scriptprog::replaceChar(unsigned char val, size_t &inx)
{
	size_t pos = inx;
	cProgramm[inx++] = val;
	return pos;
}
size_t scriptprog::appendChar(unsigned char val)
{
	size_t pos = cProgramm.size();
	cProgramm.append(val);
	return pos;
}
///////////////////////////////////////////////////////////////////////////
short scriptprog::getShort(size_t &inx) const
{	// getting a 16bit integer
	if( inx+1 < cProgramm.size() )
	{	
		return 	  ((unsigned short)cProgramm[inx++])
				| ((unsigned short)cProgramm[inx++]<<0x08);
	}
	return 0;
}
size_t scriptprog::insertShort(short val, size_t &inx)
{	// setting a 16bit integer
	size_t pos = inx;
	cProgramm.insert(basics::GetByte(val,0), 1, inx++);
	cProgramm.insert(basics::GetByte(val,1), 1, inx++);
	return pos;
}
size_t scriptprog::replaceShort(short val, size_t &inx)
{	// setting a 16bit integer
	size_t pos = inx;
	cProgramm[inx++] = basics::GetByte(val,0);
	cProgramm[inx++] = basics::GetByte(val,1);
	return pos;
}
size_t scriptprog::appendShort(int val)
{	// setting a 16bit integer
	size_t pos = cProgramm.size();
	cProgramm.append(basics::GetByte(val,0));
	cProgramm.append(basics::GetByte(val,1));
	return pos;
}
///////////////////////////////////////////////////////////////////////////
int scriptprog::getAddr(size_t &inx) const
{	// getting a 24bit integer
	if( inx+2 < cProgramm.size() )
	{	
		return 	  ((unsigned long)cProgramm[inx++])
				| ((unsigned long)cProgramm[inx++]<<0x08)
				| ((unsigned long)cProgramm[inx++]<<0x10);
	}
	return 0;
}
size_t scriptprog::insertAddr(int val, size_t &inx)
{	// setting a 24bit integer
	size_t pos = inx;
	cProgramm.insert(basics::GetByte(val,0), 1, inx++);
	cProgramm.insert(basics::GetByte(val,1), 1, inx++);
	cProgramm.insert(basics::GetByte(val,2), 1, inx++);
	return pos;
}
size_t scriptprog::replaceAddr(int val, size_t &inx)
{	// setting a 24bit integer
	size_t pos = inx;
	cProgramm[inx++] = basics::GetByte(val,0);
	cProgramm[inx++] = basics::GetByte(val,1);
	cProgramm[inx++] = basics::GetByte(val,2);
	return pos;
}
size_t scriptprog::appendAddr(int val)
{	// setting a 24bit integer
	size_t pos = cProgramm.size();
	cProgramm.append(basics::GetByte(val,0));
	cProgramm.append(basics::GetByte(val,1));
	cProgramm.append(basics::GetByte(val,2));
	return pos;
}
///////////////////////////////////////////////////////////////////////////
int scriptprog::getInt(size_t &inx) const
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
size_t scriptprog::insertInt(int val, size_t &inx)
{	// setting a 32bit integer
	size_t pos = inx;
	cProgramm.insert(basics::GetByte(val,0), 1, inx++);
	cProgramm.insert(basics::GetByte(val,1), 1, inx++);
	cProgramm.insert(basics::GetByte(val,2), 1, inx++);
	cProgramm.insert(basics::GetByte(val,3), 1, inx++);
	return pos;
}
size_t scriptprog::replaceInt(int val, size_t &inx)
{	// setting a 32bit integer
	size_t pos = inx;
	cProgramm[inx++] = basics::GetByte(val,0);
	cProgramm[inx++] = basics::GetByte(val,1);
	cProgramm[inx++] = basics::GetByte(val,2);
	cProgramm[inx++] = basics::GetByte(val,3);
	return pos;
}
size_t scriptprog::appendInt(int val)
{	// setting a 32bit integer
	size_t pos = cProgramm.size();
	cProgramm.append(basics::GetByte(val,0));
	cProgramm.append(basics::GetByte(val,1));
	cProgramm.append(basics::GetByte(val,2));
	cProgramm.append(basics::GetByte(val,3));
	return pos;
}
size_t scriptprog::appendInt64(int64 val)
{	// setting a 64bit integer
	size_t pos = cProgramm.size();
	cProgramm.append(basics::GetByte(val,0));
	cProgramm.append(basics::GetByte(val,1));
	cProgramm.append(basics::GetByte(val,2));
	cProgramm.append(basics::GetByte(val,3));
	int64 upperval = val>>32;
	cProgramm.append(basics::GetByte(upperval,0));
	cProgramm.append(basics::GetByte(upperval,1));
	cProgramm.append(basics::GetByte(upperval,2));
	cProgramm.append(basics::GetByte(upperval,3));
	return pos;
}
///////////////////////////////////////////////////////////////////////////
// converting float via union, !!not portable!!
double scriptprog::int2float(int num)
{	// converting via 32bit float
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
int scriptprog::float2int(double num)
{	// converting via 32bit float
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
const char* scriptprog::get_string(size_t &inx) const
{	
	size_t i = getAddr(inx);
	if(i && cDataSeg.size()>i)
	{
		const char *str = (const char *)&(cDataSeg[i]);
		// search for the EOS marker
		for(; cDataSeg[i] && i<cDataSeg.size(); ++i)	{}
		if( i<cDataSeg.size() ) 
			return str;
	}
	// not found
	return "";
}
size_t scriptprog::append_string(const basics::string<>& val)
{
	size_t addr = cDataSeg.size();
	if(addr==0) cDataSeg.append(0), ++addr;// initial
	cDataSeg.append((unsigned char*)val.c_str(), val.size());
	cDataSeg.append(0); //EOS
	return addr;
}

///////////////////////////////////////////////////////////////////////////
// append variable size command, 
// assuming command numbers for cmd1,2,3,4,8 are following directly
size_t scriptprog::appendVarCommand(unsigned char cmd, int64 val)
{
	size_t pos = cProgramm.size();
	// chars           0 ...        255
	// shorts     -32768 ...      32767
	// addr            0 ...   16777216
	// int   -2147483648 ... 2147483647
	// int64 rest
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
	else if(val>= INT32_MIN     && val<=INT32_MAX)
	{
		this->appendCommand(cmd+3);
		this->appendInt(val);
	}
	else
	{
		this->appendCommand(cmd+4);
		this->appendInt64(val);
	}
	return pos;
}

///////////////////////////////////////////////////////////////////////////
// debug
void scriptprog::dump() const
{
	size_t i;
	this->logging("---------------------------------------------------\n");
	this->logging("output for script '%s'\n", cName.size()?cName.c_str():"<unnamed>");

	this->logging("binary output:\n");
	for(i=0; i<cProgramm.size(); ++i)
	{
		this->logging(" %3i", cProgramm[i]);
		if(15==i%16) this->logging("\n");
	}
	this->logging("\ndata segment:\n");
	for(i=0; i<cDataSeg.size(); ++i)
	{
		if( cDataSeg[i] )
			this->logging("   %c", basics::stringcheck::isprint(cDataSeg[i])?cDataSeg[i]:'_');
		else
			this->logging("  \\0");
		if(15==i%16) this->logging("\n");
	}
	this->logging("\n\n");
	this->logging("command sequence:\n");
	for(i=0; i<cProgramm.size(); printCommand(i),this->logging("\n"))	{}

	{
		this->logging("\n");
		this->logging("labels:\n");

		basics::smap<basics::string<>, uint>::iterator iter(this->cLabels);
		for(; iter; ++iter)
		{
			this->logging("'%s' jump to %u\n", (const char*)iter->key, iter->data);
		}
	}

	{
		this->logging("\n");
		this->logging("headers:\n");

		basics::map<basics::string<>, scriptdecl>::iterator iter(this->cHeader);
		for(; iter; ++iter)
		{
			this->logging("'%s', %u parameter, start from %u\n", (const char*)iter->data.cName, (uint)iter->data.cParam.size(), (uint)iter->data.cEntry);
		}
	}

	this->logging("\n");
}

void scriptprog::printCommand(size_t &pos) const
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
		case OP_ARRAYSEL:
			this->logging("array select access, %i elements", (int)ccmd.cParam1); break;
		case OP_RANGE:
			this->logging("array range access"); break;
		case OP_SPLICE:
			this->logging("array splice access"); break;
		case OP_DULICATE:
			this->logging("array duplicate access"); break;
		case OP_CLEAR:
			this->logging("clear variable"); break;
		case OP_POP:
			this->logging("pop stack"); break;
		case OP_EVAL:
			this->logging("evaluate"); break;
		case OP_BOOLEAN:
			this->logging("create boolean"); break;
		case OP_END:
			this->logging("quit"); break;
		case OP_RETURN:
			this->logging("return"); break;

		// commands with int (float) parameters (4 bytes)
		case OP_START:
			this->logging("start (progsize=%i)", (int)ccmd.cParam1); break;
		case OP_PUSH_NONE:
			this->logging("push empty element"); break;
		case OP_PUSH_ADDR:
			this->logging("push addr '%i'", (int)ccmd.cParam1); break;
		case OP_PUSH_INT1:
			this->logging("push uchar '%i'", (int)ccmd.cParam1); break;
		case OP_PUSH_INT2:
			this->logging("push short '%i'", (int)ccmd.cParam1); break;
		case OP_PUSH_INT3:
			this->logging("push int3 '%i'", (int)ccmd.cParam1); break;
		case OP_PUSH_INT4:
			this->logging("push int '%i'", (int)ccmd.cParam1); break;
		case OP_PUSH_INT8:
			this->logging("push int64 '%i'", ccmd.cParam1); break;
		case OP_PUSH_FLOAT:
			this->logging("push float '%lf'", int2float(ccmd.cParam1) ); break;
		case OP_PUSH_TEMPVAR1:
		case OP_PUSH_TEMPVAR2:
			this->logging("push temp variable '%i'", (int)ccmd.cParam1); break;
		case OP_PUSH_TEMPVAL1:
		case OP_PUSH_TEMPVAL2:
			this->logging("push value from temp variable '%i'", (int)ccmd.cParam1); break;
		case OP_CREATEARRAY:
			this->logging("create array (%i dimension(s))", (int)ccmd.cParam1); break;
		case OP_CAST:
			this->logging("cast to %i", (int)ccmd.cParam1); break;
		case OP_CONCAT1:
		case OP_CONCAT2:
		case OP_CONCAT3:
		case OP_CONCAT4:
			this->logging("vectorize '%i' elements", (int)ccmd.cParam1); break;

		case OP_IF:
			this->logging("conditional jump on true to '%i'", (int)ccmd.cParam1); break;
		case OP_NIF:
			this->logging("conditional jump on false to '%i'", (int)ccmd.cParam1); break;
		case OP_IF_POP:
			this->logging("conditional jump on true or pop to '%i'", (int)ccmd.cParam1); break;
		case OP_NIF_POP:
			this->logging("conditional jump on false or pop to '%i'", (int)ccmd.cParam1); break;
		case OP_GOTO:
			this->logging("jump to '%i'", (int)ccmd.cParam1); break;
		case OP_GOSUB:
			this->logging("gosub to '%i'", (int)ccmd.cParam1); break;
		// commands with int and char parameters
		case OP_FUNCTION:
			this->logging("call script '%s' (%i args)", ccmd.cString, (int)ccmd.cParam1); break;
		// commands with string parameters (sizeof(pointer) bytes)
		case OP_PUSH_STRING:
			this->logging("push string '%s'", ccmd.cString); break;
		case OP_PUSH_VAL:
			this->logging("push value from global variable '%s'", ccmd.cString); break;
		case OP_PUSH_VAR:
			this->logging("push global variable '%s'", ccmd.cString); break;
		case OP_PUSH_PARAVAL:
			this->logging("push value from function parameter '%i'", (int)ccmd.cParam1); break;
		case OP_PUSH_PARAVAR:
			this->logging("push variable from function parameter '%i'", (int)ccmd.cParam1); break;
		default:
			this->logging("command %i not in list", (int)ccmd.cCommand); break;
		}
	}
}

















