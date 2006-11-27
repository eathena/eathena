// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _SCRIPTENGINE_
#define _SCRIPTENGINE_

#include "basesync.h"
#include "basetime.h"
#include "baseparser.h"
#include "basefile.h"


const unsigned char* extractEngine(const unsigned char*buffer, unsigned long sourcesz, unsigned long &sz);
void buildEngine(const char*filename);



#define OPT_PARSE			0x00
#define OPT_BEAUTIFY		0x01
#define OPT_PRINTTREE		0x02
#define OPT_TRANSFORM		0x04
#define OPT_LOGGING			0x08


struct parserstorage
{
	basics::CParser_CommentStore* parser;
	basics::CParseConfig* parser_config;

	parserstorage() : parser(NULL), parser_config(NULL)
	{}
	virtual ~parserstorage()
	{
		if(parser_config) delete parser_config;
		if(parser) delete parser;
	}

	bool init(const char* enginefile);
	virtual bool getEngine(const unsigned char*& buf, unsigned long& sz)=0;

	bool is_valid() const { return NULL!=parser; }
	operator basics::CParser_CommentStore*() { return parser; }
};



///////////////////////////////////////////////////////////////////////////////
// logging output collector/rerouter
///////////////////////////////////////////////////////////////////////////////
class CLogger
{
	int enable;
	int do_print(const char *fmt, va_list& argptr)
	{
		int ret=0;
		static char		tempbuf[4096]; // initially using a static fixed buffer size 
		static basics::Mutex	mtx;
		basics::ScopeLock		sl(mtx);
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
		int ret = fprintf(stdout, str);
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

class parsenode : public basics::global, public CLogger
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
	basics::string<>		cSymbolName;
	basics::string<>		cLexeme;
	unsigned short			cLine;
	unsigned short			cColumn;


	void insertnode(unsigned short t, unsigned short s, const basics::string<>& n, const basics::string<>& l, unsigned short line, unsigned short col)
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
	parsenode(unsigned short t, unsigned short s, const basics::string<>& n, const basics::string<>& l, unsigned short line, unsigned short col)
		: cList(NULL),cCount(0),cType(t),cSymbol(s),cSymbolName(n),cLexeme(l),cLine(line),cColumn(col)
	{}

	parsenode(const basics::CParser& parser) :  cList(NULL),cCount(0),cType(0),cSymbol(0)
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
	const char* Lexeme() const			{ return cLexeme; }
	const basics::string<>& LexemeObj() const	{ return cLexeme; }
	const char*SymbolName() const		{ return cSymbolName; }
	unsigned short Symbol() const		{ return cSymbol; }
	unsigned short Type() const			{ return cType; }
	size_t count() const				{ return cCount; }
	unsigned short Line() const			{ return cLine; }
	unsigned short Column() const		{ return cColumn; }

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

	void reduce_tree(const basics::CParser& parser, int rtpos, int flat)
	{
		size_t j, k;
		const basics::CStackElement* se = &parser.rt[rtpos];
		const basics::CStackElement* child;
		//if( se->cChildNum==1 && parser.rt[se->cChildPos].symbol.Type!=1 )// leave the last non-terminal intact
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
						parser.rt[se->cChildPos+1].cToken.cLexeme == ',' )
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
#endif//_SCRIPTENGINE_
