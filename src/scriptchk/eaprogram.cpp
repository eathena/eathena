// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////////
//## <unmanaged code>
///////////////////////////////////////////////////////////////////////////////

#include "eaopcode.h"
#include "eaprogram.h"
#include "eabuildin.h"


scriptprog::COpcode scriptprog::cOpcodeTable[256] = 
{
	{0,0,0,"nop",OP_NOP},
	{0,0,0,"assign",OP_ASSIGN}, 
	{0,0,0,"binary and",OP_BIN_AND},
	{0,0,0,"binary or",OP_BIN_OR},
	{0,0,0,"binary xor",OP_BIN_XOR},
	{0,0,0,"equate",OP_EQUATE},
	{0,0,0,"unequate",OP_UNEQUATE},
	{0,0,0,"greater than",OP_ISGT},
	{0,0,0,"greater equal",OP_ISGTEQ},
	{0,0,0,"less than",OP_ISLT},
	{0,0,0,"less equal",OP_ISLTEQ},
	{0,0,0,"left shift",OP_LSHIFT},
	{0,0,0,"right shift",OP_RSHIFT},
	{0,0,0,"add",OP_ADD},
	{0,0,0,"sub",OP_SUB},
	{0,0,0,"mul",OP_MUL},
	{0,0,0,"div",OP_DIV},
	{0,0,0,"mod",OP_MOD},
	{0,0,0,"negate",OP_NEGATE},
	{0,0,0,"invert",OP_INVERT},
	{0,0,0,"not",OP_NOT},
	{0,0,0,"sizeof",OP_SIZEOF},
	{0,0,0,"cast to integer",OP_CAST_INTEGER},
	{0,0,0,"cast to string",OP_CAST_STRING},
	{0,0,0,"cast to float",OP_CAST_FLOAT},
	{0,0,0,"preadd",OP_PREADD},
	{0,0,0,"presub",OP_PRESUB},
	{0,0,0,"postadd",OP_POSTADD},
	{0,0,0,"postsub",OP_POSTSUB},
	{0,0,0,"member access",OP_MEMBER},
	{0,0,0,"scope access",OP_SCOPE},
	{1,1,1,"function call '%s', %i parameter",OP_FUNCTION},
	{2,1,1,"function call '%s', %i parameter",OP_FUNCTION},
	{3,1,1,"function call '%s', %i parameter",OP_FUNCTION},
	{4,1,1,"function call '%s', %i parameter",OP_FUNCTION},
	{1,1,1,"subfunction call '%s', %i parameter",OP_SUBFUNCTION},
	{2,1,1,"subfunction call '%s', %i parameter",OP_SUBFUNCTION},
	{3,1,1,"subfunction call '%s', %i parameter",OP_SUBFUNCTION},
	{4,1,1,"subfunction call '%s', %i parameter",OP_SUBFUNCTION},
	{0,0,0,"push none",OP_PUSH_NONE},
	{1,0,0,"push address '%i'",OP_PUSH_ADDR},
	{2,0,0,"push address '%i'",OP_PUSH_ADDR},
	{3,0,0,"push address '%i'",OP_PUSH_ADDR},
	{4,0,0,"push address '%i'",OP_PUSH_ADDR},
	{1,0,0,"push integer '%i'",OP_PUSH_INT},
	{2,0,0,"push integer '%i'",OP_PUSH_INT},
	{3,0,0,"push integer '%i'",OP_PUSH_INT},
	{4,0,0,"push integer '%i'",OP_PUSH_INT},
	{5,0,0,"push integer '%i'",OP_PUSH_INT},
	{6,0,0,"push integer '%i'",OP_PUSH_INT},
	{7,0,0,"push integer '%i'",OP_PUSH_INT},
	{8,0,0,"push integer '%i'",OP_PUSH_INT},
	{1,1,0,"push string '%s'",OP_PUSH_STRING},
	{2,1,0,"push string '%s'",OP_PUSH_STRING},
	{3,1,0,"push string '%s'",OP_PUSH_STRING},
	{4,1,0,"push string '%s'",OP_PUSH_STRING},
	{4,0,0,"push float '%lf'",OP_PUSH_FLOAT},
	{1,1,0,"push variable reference '%s'",OP_PUSH_VAR},
	{2,1,0,"push variable reference '%s'",OP_PUSH_VAR},
	{3,1,0,"push variable reference '%s'",OP_PUSH_VAR},
	{4,1,0,"push variable reference '%s'",OP_PUSH_VAR},
	{1,1,0,"push variable value '%s'",OP_PUSH_VAL},
	{2,1,0,"push variable value '%s'",OP_PUSH_VAL},
	{3,1,0,"push variable value '%s'",OP_PUSH_VAL},
	{4,1,0,"push variable value '%s'",OP_PUSH_VAL},
	{1,0,0,"push parameter reference '%i'",OP_PUSH_PARAVAR},
	{2,0,0,"push parameter reference '%i'",OP_PUSH_PARAVAR},
	{3,0,0,"push parameter reference '%i'",OP_PUSH_PARAVAR},
	{4,0,0,"push parameter reference '%i'",OP_PUSH_PARAVAR},
	{1,0,0,"push parameter value '%i'",OP_PUSH_PARAVAL},
	{2,0,0,"push parameter value '%i'",OP_PUSH_PARAVAL},
	{3,0,0,"push parameter value '%i'",OP_PUSH_PARAVAL},
	{4,0,0,"push parameter value '%i'",OP_PUSH_PARAVAL},
	{1,0,0,"push temp reference '%i'",OP_PUSH_TEMPVAR},
	{2,0,0,"push temp reference '%i'",OP_PUSH_TEMPVAR},
	{3,0,0,"push temp reference '%i'",OP_PUSH_TEMPVAR},
	{4,0,0,"push temp reference '%i'",OP_PUSH_TEMPVAR},
	{1,0,0,"push temp value '%i'",OP_PUSH_TEMPVAL},
	{2,0,0,"push temp value '%i'",OP_PUSH_TEMPVAL},
	{3,0,0,"push temp value '%i'",OP_PUSH_TEMPVAL},
	{4,0,0,"push temp value '%i'",OP_PUSH_TEMPVAL},
	{0,0,0,"array element access",OP_ARRAY},
	{1,0,0,"array selection, %i elements",OP_ARRAYSEL},
	{2,0,0,"array selection, %i elements",OP_ARRAYSEL},
	{3,0,0,"array selection, %i elements",OP_ARRAYSEL},
	{4,0,0,"array selection, %i elements",OP_ARRAYSEL},
	{0,0,0,"array range",OP_RANGE},
	{0,0,0,"array splice",OP_SPLICE},
	{0,0,0,"array dulicate",OP_DULICATE},
	{1,0,0,"concatination, %i elements",OP_CONCAT},
	{2,0,0,"concatination, %i elements",OP_CONCAT},
	{3,0,0,"concatination, %i elements",OP_CONCAT},
	{4,0,0,"concatination, %i elements",OP_CONCAT},
	{1,0,0,"create array, %i dimensions",OP_CREATEARRAY},
	{0,0,0,"clear",OP_CLEAR},
	{0,0,0,"pop",OP_POP},
	{0,0,0,"eval",OP_EVAL},
	{0,0,0,"boolean",OP_BOOLEAN},
	{4,0,0,"start, prog size '%i'",OP_START},
	{0,0,0,"end",OP_END},
	{0,0,0,"return",OP_RETURN},
	{1,0,0,"conditional jump on false to '%i'",OP_NIF},
	{2,0,0,"conditional jump on false to '%i'",OP_NIF},
	{3,0,0,"conditional jump on false to '%i'",OP_NIF},
	{4,0,0,"conditional jump on false to '%i'",OP_NIF},
	{1,0,0,"conditional jump on true to '%i'",OP_IF},
	{2,0,0,"conditional jump on true to '%i'",OP_IF},
	{3,0,0,"conditional jump on true to '%i'",OP_IF},
	{4,0,0,"conditional jump on true to '%i'",OP_IF},
	{1,0,0,"conditional jump on false or pop to '%i'",OP_NIF_POP},
	{2,0,0,"conditional jump on false or pop to '%i'",OP_NIF_POP},
	{3,0,0,"conditional jump on false or pop to '%i'",OP_NIF_POP},
	{4,0,0,"conditional jump on false or pop to '%i'",OP_NIF_POP},
	{1,0,0,"conditional jump on true or pop to '%i'",OP_IF_POP},
	{2,0,0,"conditional jump on true or pop to '%i'",OP_IF_POP},
	{3,0,0,"conditional jump on true or pop to '%i'",OP_IF_POP},
	{4,0,0,"conditional jump on true or pop to '%i'",OP_IF_POP},
	{1,0,0,"jump to '%i'",OP_GOTO},
	{2,0,0,"jump to '%i'",OP_GOTO},
	{3,0,0,"jump to '%i'",OP_GOTO},
	{4,0,0,"jump to '%i'",OP_GOTO},
	{1,0,0,"gosub to '%i'",OP_GOSUB},
	{2,0,0,"gosub to '%i'",OP_GOSUB},
	{3,0,0,"gosub to '%i'",OP_GOSUB},
	{4,0,0,"gosub to '%i'",OP_GOSUB},
};


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
	const COpcode& op = scriptprog::cOpcodeTable[cmd.cCommand];
	///////////
	cmd.cCommand = op.code;
	///////////
	cmd.cParam1 = getNumber(inx, op.size1);
	///////////
	cmd.cParam2 = getNumber(inx, op.size2);
	///////////
	if( op.type1 )
	{	// is string
		cmd.cString = "";
		if( cmd.cParam1 && cDataSeg.size()>cmd.cParam1 )
		{
			size_t i = cmd.cParam1;
			const char *str = (const char *)&(cDataSeg[i]);
			// search for the EOS marker
			for(; cDataSeg[i] && i<cDataSeg.size(); ++i)	{}
			if( i<cDataSeg.size() ) 
				cmd.cString = str;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
//
size_t scriptprog::nextCommand(size_t pos) const
{
	const unsigned char cmd = getCommand(pos);
	const COpcode& op = scriptprog::cOpcodeTable[cmd];
	return pos + op.size1+op.size2;
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


///////////////////////////////////////////////////////////////////////////////
// get a number.
// fetch sz bytes from the stream and build a integer
int64 scriptprog::getNumber(size_t &inx, unsigned char sz) const
{
	uint64 ret=0;
	size_t shift=0;
	uint64 val=0;
	for( ; sz && inx<cProgramm.size(); --sz, ++inx, shift+=8)
	{
		val = cProgramm[inx];
		ret |= val<<shift;
	}
	if(val&0x80)
	{	// append sign bits
		ret |= (~LLCONST(0))<<shift;
	}
	return ret;
}


///////////////////////////////////////////////////////////////////////////
// append variable size command, 
// assuming command numbers for cmd1,2,3,4,8 are following directly
size_t scriptprog::appendVarCommand(unsigned char cmd, int64 val)
{
	size_t pos = cProgramm.size();
	this->appendCommand(cmd);
	const int64 cmp = (val<0)?-1:0;
	const unsigned char sign = (val<0)?0x80:0x00;
	unsigned char v;
	size_t num=0;
	do
	{
		v = (unsigned char)(0xFF&val);
		cProgramm.append( v );
		val>>=8;
		++num;
	} while( val!=cmp || (v&0x80)!=sign );

	if(num>1)
	{
		size_t mypos=pos;
		this->replaceCommand(cmd+num-1, mypos);
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
		const COpcode& op = scriptprog::cOpcodeTable[ccmd.cCommand];
		if( op.type1 && op.size2 )
		{	// 2 params first string
			this->logging(op.desc, ccmd.cString, (int)ccmd.cParam2);
		}
		else if( op.type1 )
		{	// 1 param first string
			this->logging(op.desc, ccmd.cString);
		}
		else if( op.size1 )
		{	// 1 param first integer
			if( op.code==OP_PUSH_FLOAT )
				this->logging(op.desc, int2float(ccmd.cParam1));
			else
				this->logging(op.desc, (int)ccmd.cParam1);
		}
		else
		{	// no param
			this->logging(op.desc);
		}
	}
}

















