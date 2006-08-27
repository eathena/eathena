// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "basetypes.h"
#include "basefile.h"



//////////////////////////////
#ifdef WIN32
//////////////////////////////

#define RETCODE	"\r\n"
#define RET RETCODE

//////////////////////////////
#else/////////////////////////
//////////////////////////////

#define RETCODE "\n"
#define RET RETCODE

//////////////////////////////
#endif////////////////////////
//////////////////////////////


void dump(unsigned char *buffer, size_t num);

int config_switch(const char *str, int min=INT_MIN, int max=INT_MAX);
bool email_check(const char *email);
bool remove_control_chars(char *str);
const char *strcpytolower(char *tar, const char *str);
const char *strcpytolower(char *tar, size_t sz, const char *str);
const char *safestrcpy(char *tar, size_t sz, const char *src);
const char *replacecpy(char *tar, size_t sz, const char* src, char rplc='\t', char with=' ');
const char *mapname2buffer(unsigned char *buffer, size_t sz, const char *mapname);
const char *buffer2mapname(char *mapname, size_t sz, const char *buffer);
const char *is_valid_line(const char *line);
size_t prepare_line(char *line);




///////////////////////////////////////////////////////////////////////////////
/// commandline 2 list conversion.
/// splits a string on whitespaces/comma
/// but keeps quoted strings together it just strips off the quotes 
/// also does automatic number conversion on access 
/// together with on/off/yes/no detection
class CParameterList
{
public:
	///////////////////////////////////////////////////////////////////////////////
	/// internal read only string/number variant.
	/// resolves type on assignment
	class CParameter
	{
		// only the CParameterList is allowed to create
		friend class CParameterList;

		const char* cStr;

		/// private constructor
		CParameter(const char* str) : cStr(str)	{}
	public:
		/// public destructor.
		~CParameter()	{}

		// explicit access on string
		const char*string() const		{ return this->cStr; }
		// explicit access on integer
		int integer() const				{ return config_switch(this->cStr); }

		// type resolving on assign
		operator const char*() const	{ return this->string(); }
		operator bool() const			{ return this->integer(); }
		operator char() const			{ return this->integer(); }
		operator unsigned char() const	{ return this->integer(); }
		operator short() const			{ return this->integer(); }
		operator unsigned short() const	{ return this->integer(); }
		operator int() const			{ return this->integer(); }
		operator unsigned int() const	{ return this->integer(); }
		operator long() const			{ return this->integer(); }
		operator unsigned long() const	{ return this->integer(); }

		// compare with string
		// only do lowercase compare
		int compare(const char* str) const
		{	
			const char*ipp = this->cStr;
			if(ipp && str)
			{
				while( *ipp && *str && basics::locase(*ipp) == basics::locase(*str) )
					++ipp, ++str;
				return basics::locase(*ipp) - basics::locase(*str);
			}
			return (str)?-*str:ipp?*ipp:0;
		}

		friend bool operator==(const CParameter& p, const char* str)	{ return 0==p.compare(str); }
		friend bool operator!=(const CParameter& p, const char* str)	{ return 0!=p.compare(str); }
		friend bool operator< (const CParameter& p, const char* str)	{ return 0< p.compare(str); }

		friend bool operator==(const char* str, const CParameter& p)	{ return 0==p.compare(str); }
		friend bool operator!=(const char* str, const CParameter& p)	{ return 0!=p.compare(str); }
		friend bool operator< (const char* str, const CParameter& p)	{ return 0> p.compare(str); }
	};

private:

	char cTemp[1024];
	char cBuf[1024];
	char *cParam[128];
	size_t cCnt;
public:

	/// constructor
	CParameterList() : cCnt(0)
	{
		*this->cTemp=0;
	}
	// default destructor/copy/assing


	/// constructing constructor
	CParameterList(const char* line) : cCnt(0)
	{
		this->add_commandline(line);
	}
	
	/// add a new commandline
	bool add_commandline(const char* line)
	{
		char *wpp = this->cBuf, delim;
		const char *epp=this->cBuf+sizeof(this->cBuf)-1;
		const char* rpp = line;

		// skip all leading spaces
		while( *rpp && (basics::stringcheck::isspace(*rpp)||*rpp==',') ) ++rpp;

		// copy the whole line for testing purpose
		safestrcpy(this->cTemp, sizeof(this->cTemp), rpp);

		// clear previously command line
		this->cCnt=0;

		while( *rpp && wpp<epp && this->cCnt<sizeof(this->cParam)/sizeof(*this->cParam) )
		{
			// skip all leading spaces/comma
			while( *rpp && (basics::stringcheck::isspace(*rpp)||*rpp==',') ) ++rpp;

			// delimiter decision
			// skip the quote when string parameter
			delim = (*rpp=='"') ? *rpp++ : 0;

			// save the parameter start pointer
			this->cParam[this->cCnt] = wpp;

			// copy into the buffer up to the finishing delimiter
			while( *rpp && wpp<epp && ((delim && *rpp!=delim) || (!delim && !basics::stringcheck::isspace(*rpp) && *rpp!=',')) )
				*wpp++ = *rpp++;

			// terminate the copied string, when there is something
			if( wpp>this->cParam[this->cCnt] )
			{
				*wpp++=0;
				++this->cCnt;
			}
			// skip the delimiter
			if(*rpp) ++rpp;
		}
		return this->cCnt>0;
	}

	/// get the original line
	const char* line() const
	{
		return this->cTemp;
	}

	/// get parameter from index
	const CParameter operator[](int inx) const
	{
		return CParameter(this->string((size_t)inx));
	}
	/// get parameter from index
	const CParameter first() const
	{
		return CParameter(this->string(0));
	}
	/// get parameter from index
	const CParameter last() const
	{
		return CParameter(this->cCnt?this->string(this->cCnt-1):"");
	}

	/// get explicit string from index
	const char* string(size_t inx) const
	{
		return (inx<this->cCnt) ? this->cParam[inx] : "";
	}
	/// get explicit integer from index
	long integer(size_t inx) const
	{
		return (inx<this->cCnt) ? strtol(this->cParam[inx],NULL,0) : 0;
	}
	/// get number of parameters
	size_t size() const	{ return this->cCnt; }

	/// remove the entries from list.
	void erase(size_t st=0, size_t num=1)
	{
		if( st<this->cCnt )
		{
			if(st+num>this->cCnt)
				num = this->cCnt-st;
			this->cCnt -= num;
			size_t mvcnt = this->cCnt-st-num;
			if(mvcnt)
				memmove(this->cParam+st, this->cParam+st+num, mvcnt*sizeof(*cParam));
		}
	}
};





#endif//COMMON_UTILS_H
