#ifndef __EAPARSER_H__
#define __EAPARSER_H__

#include "scriptprn.h"


///////////////////////////////////////////////////////////////////////////////
// terminal definitions from parse tree
#include "eascript.h"


///////////////////////////////////////////////////////////////////////////////////////
// user defined parse processor
///////////////////////////////////////////////////////////////////////////////////////
#define OPT_PARSE			0x00
#define OPT_BEAUTIFY		0x01
#define OPT_PRINTTREE		0x02
#define OPT_TRANSFORM		0x04
#define OPT_COMPILE			0x08
#define OPT_COMPILEOUTPUT	0x10


///////////////////////////////////////////////////////////////////////////////////////
// user defined code beautifier.
///////////////////////////////////////////////////////////////////////////////////////
struct eaprinter : public default_printer
{
private:
	ICL_EMPTY_COPYCONSTRUCTOR(eaprinter)

	bool cHasDefault;
public:
	eaprinter() : 
	  cHasDefault(false)
	{}
	~eaprinter()
	{}

	virtual bool print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent=0);
};



///////////////////////////////////////////////////////////////////////////////
// Parse Tree Transformation
//  * Simplification 
//    - node reduction (ok)
//    - list unrolling (ok)
//  * Optimisation
//    - combining constants ( )
//    - removing unreachable nodes ( )
//    - reordering integer multiplication/division ( )
//
class transformer : public basics::global, public default_printer
{
private:
	ICL_EMPTY_COPYCONSTRUCTOR(transformer)

	///////////////////////////////////////////////////////////////////////////
	// types
	typedef transformer* ptransformer;

	///////////////////////////////////////////////////////////////////////////
	// class data
	ptransformer*			cList;
	size_t					cCount;
	unsigned short			cType;
	unsigned short			cSymbol;
	basics::string<>		cSymbolName;
	basics::string<>		cLexeme;
	unsigned short			cLine;
	unsigned short			cColumn;

	void insert(unsigned short t, unsigned short s, const basics::string<>& n, const basics::string<>& l, unsigned short line, unsigned short col)
	{
		// add element to List
		ptransformer* temp = new ptransformer[cCount+1];
		if(cList)
		{
			memcpy(temp,cList,cCount*sizeof(ptransformer));
			delete[] cList;
		}
		cList = temp;
		cList[cCount] = new transformer(t,s,n,l,line,col);
		++cCount;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	transformer(): cList(NULL),cCount(0),cType(0),cSymbol(0)	{}
	transformer(unsigned short t, unsigned short s, const basics::string<>& n, const basics::string<>& l, unsigned short line, unsigned short col)
		:  cList(NULL),cCount(0),cType(t),cSymbol(s),cSymbolName(n),cLexeme(l),cLine(line),cColumn(col)
	{}

	transformer(const basics::CParser& parser) :  cList(NULL),cCount(0),cType(0),cSymbol(0)
	{
		transformer temp;
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
	~transformer()
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
	const char* Lexeme()	const	{ return cLexeme; }
	const basics::string<>& LexemeObj() const { return cLexeme; }
	const char*SymbolName()	const	{ return cSymbolName; }
	unsigned short Symbol()	const	{ return cSymbol; }
	unsigned short Type()	const	{ return cType; }
	size_t count()	const			{ return cCount; }
	unsigned short Line() const		{ return cLine; }
	unsigned short Column() const	{ return cColumn; }

	const transformer& operator[](size_t inx) const	{ return (inx<cCount)?(*(cList[inx])):(*this); }

	///////////////////////////////////////////////////////////////////////////
	// parse tree functions
	void print_tree(size_t level=0)
	{
		size_t i;
		for(i=0; i<level; ++i) this->log("| ");
		this->log("%c-<%s>(%i)[%i] ::= '%s'\n", 
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
		if( se->cChildNum==1 )
		{
			this->reduce_tree(parser, se->cChildPos, 0);
		}
		else
		{
			ptransformer newlist = this;
			if( flat==0)
			{
				this->insert(se->symbol.Type, se->symbol.idx, se->symbol.Name, se->cToken.cLexeme, se->cToken.line, se->cToken.column);
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
						PT_COMMA==(parser.rt[se->cChildPos+1]).symbol.idx )
						flat = 1;
					newlist->reduce_tree(parser, j, flat);
				}
				else
				{
					newlist->insert(child->symbol.Type, child->symbol.idx, child->symbol.Name, child->cToken.cLexeme, se->cToken.line, se->cToken.column);
				}
			}
		}
	}
};




#endif//__EAPARSER_H__
