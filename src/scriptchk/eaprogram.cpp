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
	{0,0,0,OP_NOP,"nop"},
	{0,0,2,OP_ASSIGN,"assign"},
	{0,0,2,OP_ARRAY_ASSIGN,"array-assign"},
	{0,0,2,OP_ADD_ASSIGN,"add-assign"},
	{0,0,2,OP_SUB_ASSIGN,"sub-assign"},
	{0,0,2,OP_MUL_ASSIGN,"mul-assign"},
	{0,0,2,OP_DIV_ASSIGN,"div-assign"},
	{0,0,2,OP_MOD_ASSIGN,"mod-assign"},
	{0,0,2,OP_BIN_XOR_ASSIGN,"xor-assign"},
	{0,0,2,OP_BIN_AND_ASSIGN,"and-assign"},
	{0,0,2,OP_BIN_OR_ASSIGN,"or-assign"},
	{0,0,2,OP_RSHIFT_ASSIGN,"rshift-assign"},
	{0,0,2,OP_LSHIFT_ASSIGN,"lshift-assign"},
	{0,0,2,OP_ADD,"add"},
	{0,0,2,OP_SUB,"sub"},
	{0,0,2,OP_MUL,"mul"},
	{0,0,2,OP_DIV,"div"},
	{0,0,2,OP_MOD,"mod"},
	{0,0,2,OP_BIN_AND,"binary and"},
	{0,0,2,OP_BIN_OR,"binary or"},
	{0,0,2,OP_BIN_XOR,"binary xor"},
	{0,0,2,OP_LSHIFT,"left shift"},
	{0,0,2,OP_RSHIFT,"right shift"},
	{0,0,2,OP_EQUATE,"equate"},
	{0,0,2,OP_UNEQUATE,"unequate"},
	{0,0,2,OP_ISGT,"greater than"},
	{0,0,2,OP_ISGTEQ,"greater equal"},
	{0,0,2,OP_ISLT,"less than"},
	{0,0,2,OP_ISLTEQ,"less equal"},
	{0,0,1,OP_NEGATE,"negate"},
	{0,0,1,OP_INVERT,"invert"},
	{0,0,1,OP_NOT,"not"},
	{0,0,1,OP_SIZEOF,"sizeof"},
	{0,0,1,OP_CAST_INTEGER,"cast to integer"},
	{0,0,1,OP_CAST_STRING,"cast to string"},
	{0,0,1,OP_CAST_FLOAT,"cast to float"},
	{0,0,1,OP_PREADD,"preadd"},
	{0,0,1,OP_PRESUB,"presub"},
	{0,0,1,OP_POSTADD,"postadd"},
	{0,0,1,OP_POSTSUB,"postsub"},
	{0,0,2,OP_MEMBER,"member access"},
	{0,0,2,OP_SCOPE,"scope access"},
	{1,1,1,OP_FUNCTION,"function call '%s'"},
	{2,1,1,OP_FUNCTION,"function call '%s'"},
	{3,1,1,OP_FUNCTION,"function call '%s'"},
	{4,1,1,OP_FUNCTION,"function call '%s'"},
	{1,1,1,OP_BLDFUNCTION,"buildin function call '%s'"},
	{2,1,1,OP_BLDFUNCTION,"buildin function call '%s'"},
	{3,1,1,OP_BLDFUNCTION,"buildin function call '%s'"},
	{4,1,1,OP_BLDFUNCTION,"buildin function call '%s'"},
	{1,1,2,OP_SUBFUNCTION,"subfunction call '%s'"},
	{2,1,2,OP_SUBFUNCTION,"subfunction call '%s'"},
	{3,1,2,OP_SUBFUNCTION,"subfunction call '%s'"},
	{4,1,2,OP_SUBFUNCTION,"subfunction call '%s'"},
	{0,0,0,OP_PUSH_NONE,"push none"},
	{0,0,0,OP_PUSH_ZERO,"push integer '0'"},
	{0,0,0,OP_PUSH_ONE,"push integer '1'"},
	{0,0,0,OP_PUSH_TWO,"push integer '2'"},
	{0,0,0,OP_PUSH_THREE,"push integer '3'"},
	{0,0,0,OP_PUSH_FOUR,"push integer '4'"},
	{0,0,0,OP_PUSH_FIVE,"push integer '5'"},
	{0,0,0,OP_PUSH_SIX,"push integer '6'"},
	{0,0,0,OP_PUSH_SEVEN,"push integer '7'"},
	{0,0,0,OP_PUSH_EIGHT,"push integer '8'"},
	{0,0,0,OP_PUSH_NINE,"push integer '9'"},
	{0,0,0,OP_PUSH_TEN,"push integer '10'"},
	{1,0,0,OP_PUSH_INT,"push integer '%i'"},
	{2,0,0,OP_PUSH_INT,"push integer '%i'"},
	{3,0,0,OP_PUSH_INT,"push integer '%i'"},
	{4,0,0,OP_PUSH_INT,"push integer '%i'"},
	{5,0,0,OP_PUSH_INT,"push integer '%i'"},
	{6,0,0,OP_PUSH_INT,"push integer '%i'"},
	{7,0,0,OP_PUSH_INT,"push integer '%i'"},
	{8,0,0,OP_PUSH_INT,"push integer '%i'"},
	{1,1,0,OP_PUSH_STRING,"push string '%s'"},
	{2,1,0,OP_PUSH_STRING,"push string '%s'"},
	{3,1,0,OP_PUSH_STRING,"push string '%s'"},
	{4,1,0,OP_PUSH_STRING,"push string '%s'"},
	{4,0,0,OP_PUSH_FLOAT,"push float '%lf'"},
	{1,1,2,OP_PUSH_VAR,"push variable reference '%s'"},
	{2,1,2,OP_PUSH_VAR,"push variable reference '%s'"},
	{3,1,2,OP_PUSH_VAR,"push variable reference '%s'"},
	{4,1,2,OP_PUSH_VAR,"push variable reference '%s'"},
	{1,1,2,OP_PUSH_VAL,"push variable value '%s'"},
	{2,1,2,OP_PUSH_VAL,"push variable value '%s'"},
	{3,1,2,OP_PUSH_VAL,"push variable value '%s'"},
	{4,1,2,OP_PUSH_VAL,"push variable value '%s'"},
	{1,0,0,OP_PUSH_PARAVAR,"push parameter reference '%i'"},
	{2,0,0,OP_PUSH_PARAVAR,"push parameter reference '%i'"},
	{3,0,0,OP_PUSH_PARAVAR,"push parameter reference '%i'"},
	{4,0,0,OP_PUSH_PARAVAR,"push parameter reference '%i'"},
	{1,0,0,OP_PUSH_PARAVAL,"push parameter value '%i'"},
	{2,0,0,OP_PUSH_PARAVAL,"push parameter value '%i'"},
	{3,0,0,OP_PUSH_PARAVAL,"push parameter value '%i'"},
	{4,0,0,OP_PUSH_PARAVAL,"push parameter value '%i'"},
	{1,0,0,OP_PUSH_TEMPVAR,"push temp reference '%i'"},
	{2,0,0,OP_PUSH_TEMPVAR,"push temp reference '%i'"},
	{3,0,0,OP_PUSH_TEMPVAR,"push temp reference '%i'"},
	{4,0,0,OP_PUSH_TEMPVAR,"push temp reference '%i'"},
	{1,0,0,OP_PUSH_TEMPVAL,"push temp value '%i'"},
	{2,0,0,OP_PUSH_TEMPVAL,"push temp value '%i'"},
	{3,0,0,OP_PUSH_TEMPVAL,"push temp value '%i'"},
	{4,0,0,OP_PUSH_TEMPVAL,"push temp value '%i'"},
	{0,0,2,OP_ARRAY,"array element access"},
	{1,0,0,OP_ARRAYSEL,"array selection, %i elements"},
	{2,0,0,OP_ARRAYSEL,"array selection, %i elements"},
	{3,0,0,OP_ARRAYSEL,"array selection, %i elements"},
	{4,0,0,OP_ARRAYSEL,"array selection, %i elements"},
	{0,0,3,OP_RANGE,"array range"},
	{0,0,4,OP_SPLICE,"array splice"},
	{0,0,4,OP_DULICATE,"array dulicate"},
	{1,0,0,OP_CONCAT,"concatination, %i elements"},
	{2,0,0,OP_CONCAT,"concatination, %i elements"},
	{3,0,0,OP_CONCAT,"concatination, %i elements"},
	{4,0,0,OP_CONCAT,"concatination, %i elements"},
	{1,0,0,OP_CREATEARRAY,"create array, %i dimensions"},
	{0,0,1,OP_EMPTY,"clear variable"},
	{0,0,0,OP_POP,"pop"},
	{0,0,1,OP_EVAL,"eval"},
	{0,0,1,OP_BOOLEAN,"boolean"},
	{1,0,1,OP_NIF,"conditional jump on false to '%i'"},
	{2,0,1,OP_NIF,"conditional jump on false to '%i'"},
	{3,0,1,OP_NIF,"conditional jump on false to '%i'"},
	{4,0,1,OP_NIF,"conditional jump on false to '%i'"},
	{1,0,1,OP_IF,"conditional jump on true to '%i'"},
	{2,0,1,OP_IF,"conditional jump on true to '%i'"},
	{3,0,1,OP_IF,"conditional jump on true to '%i'"},
	{4,0,1,OP_IF,"conditional jump on true to '%i'"},
	{1,0,1,OP_NIF_POP,"conditional jump on false or pop to '%i'"},
	{2,0,1,OP_NIF_POP,"conditional jump on false or pop to '%i'"},
	{3,0,1,OP_NIF_POP,"conditional jump on false or pop to '%i'"},
	{4,0,1,OP_NIF_POP,"conditional jump on false or pop to '%i'"},
	{1,0,1,OP_IF_POP,"conditional jump on true or pop to '%i'"},
	{2,0,1,OP_IF_POP,"conditional jump on true or pop to '%i'"},
	{3,0,1,OP_IF_POP,"conditional jump on true or pop to '%i'"},
	{4,0,1,OP_IF_POP,"conditional jump on true or pop to '%i'"},
	{1,0,0,OP_GOTO,"jump to '%i'"},
	{2,0,0,OP_GOTO,"jump to '%i'"},
	{3,0,0,OP_GOTO,"jump to '%i'"},
	{4,0,0,OP_GOTO,"jump to '%i'"},
	{1,0,0,OP_GOSUB,"gosub to '%i'"},
	{2,0,0,OP_GOSUB,"gosub to '%i'"},
	{3,0,0,OP_GOSUB,"gosub to '%i'"},
	{4,0,0,OP_GOSUB,"gosub to '%i'"},
	{0,0,0,OP_RETURN,"return"},
	{0,0,0,OP_END,"end"},
	{4,0,0,OP_START,"start, prog size '%i'"}
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
bool scriptprog::regist(script& scr, uint line)
{
	if( 0==scr->cName.size() )
	{	// when no name available, it's only used at the instances
		return true;
	}
	else if( scriptprog::cNamedProgs.exists(scr->cName) )
	{	// conflicting name
		fprintf(stderr, "programm with name '%s' (line %u) already exists\n"
			"ignoring name, script will be not available for external duplication\n", 
			scr->cName.c_str(), line); 
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
	cmd.cCount  = op.param;
	///////////
	cmd.cParam = getNumber(inx, op.size);
	///////////
	cmd.cString = "";
	if( op.type )
	{	// is string
		size_t i = cmd.cParam;
		if( i && i<cDataSeg.size() )
		{
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
	return pos + op.size;
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
		if( op.type )
		{	// 1 param first string
			this->logging(op.desc, ccmd.cString);
		}
		else if( op.size )
		{	// 1 param first integer
			if( op.code==OP_PUSH_FLOAT )
				this->logging(op.desc, int2float(ccmd.cParam));
			else
				this->logging(op.desc, (int)ccmd.cParam);
		}
		else
		{	// no param
			this->logging(op.desc);
		}
	}
}

















