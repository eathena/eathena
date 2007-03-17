// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _EAPROGRAM_
#define _EAPROGRAM_

///////////////////////////////////////////////////////////////////////////////
#include "basesafeptr.h"
#include "basestring.h"
#include "basevariant.h"


struct scriptprog;

///////////////////////////////////////////////////////////////////////////////
/// script function declaration
struct scriptdecl
{
	basics::string<>					cName;		///< function name
	basics::vector<basics::variant>		cParam;		///< parameter type, default value, const state
	basics::var_t						cReturn;	///< return value type
	size_t								cVarCnt;	///< number of temporary variables
	size_t								cEntry;		///< start position
	scriptprog*							cScript;	///< the script itself

	scriptdecl() : cReturn(basics::VAR_NONE), cVarCnt(0), cEntry(0), cScript(NULL)
	{}
};


///////////////////////////////////////////////////////////////////////////////
/// program storage.
struct scriptprog
{
	typedef basics::TObjPtrCount<scriptprog>		script;

	///////////////////////////////////////////////////////////////////////////////
	/// command declaration
	struct CCommand
	{
		unsigned char	cCommand;
		unsigned char	cCount;
		int64			cParam;
		const char*		cString;
		
	};
	struct COpcode
	{
		unsigned char size;		// number of bytes in the first parameter
		unsigned char type;		// type of first parameter
		unsigned char param;	// number of operands for the command
		unsigned char code;		// base command code
		const char* desc;		// some descrition
	};

	basics::string<>								cName;			///< name on the programm
	basics::map<basics::string<>, scriptdecl>		cHeader;		///< declarative part
	basics::vector<uchar>							cProgramm;		///< the programm itself
	basics::vector<uchar>							cDataSeg;		///< the data segment
	basics::smap<basics::string<>, uint>			cLabels;		///< labels
	static basics::smap<basics::string<>, script>	cNamedProgs;	///< lookup for progs with names
	static COpcode									cOpcodeTable[256];///< opcode processing

	scriptprog()
	{}
	~scriptprog()
	{
		this->unregist();
	}

	///////////////////////////////////////////////////////////////////////////
	// register a script via name
	static bool regist(script& scr, uint line);
	///////////////////////////////////////////////////////////////////////////
	// unregister script
	void unregist() const;
	///////////////////////////////////////////////////////////////////////////
	// get  script by name
	static script get_script(const basics::string<>& name);

	///////////////////////////////////////////////////////////////////////////
	// compares
	bool operator==(const scriptprog& a) const	{ return this==&a; }
	bool operator< (const scriptprog& a) const	{ return this< &a; }


	///////////////////////////////////////////////////////////////////////////
	// 
	size_t size() const					{ return this->cProgramm.size(); }
	size_t getCurrentPosition() const	{ return this->cProgramm.size(); }

	void clear()
	{
		this->cName.clear();
		this->cHeader.clear();
		this->cProgramm.clear();
		this->cDataSeg.clear();
		this->cLabels.clear();
	}

	///////////////////////////////////////////////////////////////////////////
	// fetch command and parameters; and go to next command
	bool getCommand(size_t &inx, CCommand& cmd) const;
	///////////////////////////////////////////////////////////////////////////
	size_t nextCommand(size_t pos) const;

	///////////////////////////////////////////////////////////////////////////
	// label functions
	bool create_label(const basics::string<>& name, uint pos);
	bool is_label(const basics::string<>& name) const;
	uint get_labeltarget(const basics::string<>& name) const;
	scriptdecl get_declaration(const basics::string<>& name) const;

	size_t append(const scriptprog& p);
	unsigned char getCommand(size_t &inx) const;
	size_t insertCommand(unsigned char val, size_t &inx);
	size_t replaceCommand(unsigned char val, size_t &inx);
	size_t appendCommand(unsigned char val);
	unsigned char getChar(size_t &inx) const;
	size_t insertChar(unsigned char val, size_t &inx);
	size_t replaceChar(unsigned char val, size_t &inx);
	size_t appendChar(unsigned char val);
	short getShort(size_t &inx) const;
	size_t insertShort(short val, size_t &inx);
	size_t replaceShort(short val, size_t &inx);
	size_t appendShort(int val);
	int getAddr(size_t &inx) const;
	size_t insertAddr(int val, size_t &inx);
	size_t replaceAddr(int val, size_t &inx);
	size_t appendAddr(int val);
	int getInt(size_t &inx) const;
	size_t insertInt(int val, size_t &inx);
	size_t replaceInt(int val, size_t &inx);
	size_t appendInt(int val);
	size_t appendInt64(int64 val);
	static double int2float(int num);
	static int float2int(double num);
	const char* get_string(size_t &inx) const;
	size_t append_string(const basics::string<>& val);
	size_t appendVarCommand(unsigned char cmd, int64 val);
	void dump() const;
	void printCommand(size_t &pos) const;

	int64 getNumber(size_t &inx, unsigned char sz) const;

	int logging(const char *fmt, ...) const
	{
		int ret = 0;
		va_list argptr;
		va_start(argptr, fmt);
		ret = vprintf(fmt, argptr);
		va_end(argptr);
		return ret;
	}

};





///////////////////////////////////////////////////////////////////////////////
#endif//_EAPROGRAM_
