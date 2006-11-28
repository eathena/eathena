// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "basesafeptr.h"
#include "basetime.h"
#include "baseparser.h"
#include "basefile.h"

#include "eacompiler.h"
#include "scriptengine.h"
#include "scriptprn.h"

#include "baseparam.h"

///////////////////////////////////////////////////////////////////////////////
// basic class for using the old way timers
///////////////////////////////////////////////////////////////////////////////
bool basics::CTimerBase::init(unsigned long interval)
{
	return false;
}

// external calling from external timer implementation
int basics::CTimerBase::timercallback(int timer, unsigned long tick, int id, basics::numptr data)
{
	return 0;
}
void basics::CTimerBase::timerfinalize()
{

}



///////////////////////////////////////////////////////////////////////////////////////
// code beautifier
///////////////////////////////////////////////////////////////////////////////////////
struct eaprinter : public printer
{
public:
	eaprinter()
	{}
	~eaprinter()
	{}

	virtual bool print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent=0);
};

bool eaprinter::print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent)
{
	eaprinter &prn = *this;
	bool ret = true;

	print_comments(parser, rtpos);

	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// terminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case PT_RBRACE:
			scope--;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		case PT_LBRACE:
			if(!prn.newline) prn << '\n';
			prn << "{\n";
			++prn.scope;
			break;
		case PT_SEMI:
			prn << ";\n";
			break;
		case PT_COMMA:
			prn << ", ";
			break;
//		case PT_LPARAN:
//		case PT_RPARAN:
		default:
			// print the token
			prn << parser.rt[rtpos].cToken.cLexeme;
			break;
		}
	}
	else if( parser.rt[rtpos].cChildNum==1 )
	{	// only one child, just go down
		print_beautified(parser, parser.rt[rtpos].cChildPos);
	}
	else if( parser.rt[rtpos].cChildNum>1 )
	{	// nonterminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case PT_LABELSTM:
		{
			int tmpscope = prn.scope;
			prn.scope=0;
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			prn << ":\n";
			prn.scope = tmpscope;
			break;
		}
		case PT_IFSTM:
		case PT_WHILESTM:
		{
			// if '(' <Expr> ')' <Normal Stm>
			// if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
			// while '(' <Expr> ')' <Normal Stm>
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << "( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << " )\n";

			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
				++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+4);
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
				--prn.scope;

			if( parser.rt[rtpos].cChildNum==7 )
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos+5);
				if(!prn.newline) prn << '\n';

				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
					++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+6);
				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
					--prn.scope;
			}
			break;
		}
		case PT_DOSTM:
		{	// do <Normal Stm> while '(' <Expr> ')' ';'
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << '\n';
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
				++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
				--prn.scope;
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << "( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+4);
			prn << " )";
			print_beautified(parser, parser.rt[rtpos].cChildPos+6);
			break;
		}
		case PT_SWITCHSTM:
		{	// switch '(' <Expr> ')' '{' <Case Stms> '}'
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << "( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << " )\n";
			prn << "{\n";
			print_beautified(parser, parser.rt[rtpos].cChildPos+5);
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case PT_FORSTM:
		{	// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
			if(!prn.newline) prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << '(';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			prn << "; ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+4);
			prn << "; ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+6);
			prn << ")\n";
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
				++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+8);
			if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
				--prn.scope;
			break;
		}
		case PT_EXPRSTM:
		{
			size_t j,k;
			if(!prn.newline) prn << '\n';
			k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
			j = parser.rt[rtpos].cChildPos;
			for(; j<k; ++j)
			{
				print_beautified(parser, j);
			}
			break;
		}
		case PT_CASESTM:
		{	// <Case Stms>  ::= case <Value> ':' <Stm List> <Case Stms>
			//			   | default ':' <Stm List> <Case Stms>
			//			   |
			size_t j,k;
			int tmpscope = prn.scope;
			k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
			for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
			{	// go down
				if( PT_COLON==parser.rt[j].symbol.idx )
				{
					prn << ":\n";
					++prn.scope;
				}
				else
				{
					if( PT_CASESTM==parser.rt[j].symbol.idx )
						--prn.scope;
					print_beautified(parser, j);
				}
			}
			prn.scope = tmpscope;
			break;
		}
		case PT_SPECLIST:
		{	// <Spec Item> ',' <Spec List>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << ',' << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			break;
		}
		case PT_CONSTELEM:
		{	// '{' <Const List> '}'
			prn << '{';
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << '}';
			break;
		}
		case  PT_DEFINEDECL:
		{	// 'define' identifier '=' <Const Unary> ';'
			// 'define' identifier '=' identifier ';'
			prn << "define ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ' ' << '=' << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+3);
			prn << ';' << '\n';
			break;
		}
		case PT_INCLUDEDECL:
		{	// 'include' StringLiteral ';'
			prn << "include ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ';' << '\n';
			break;
		}
		case PT_FUNCDECL:
		{	// <VarType> identifier '(' <Paramse> ')' ';'
			// <VarType> identifier '(' <Paramse> ')' <Block>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << '(';
			print_beautified(parser, parser.rt[rtpos].cChildPos+3);
			prn << ')';
			print_beautified(parser, parser.rt[rtpos].cChildPos+5);
			if( !prn.newline) prn << '\n';
			break;
		}
		case PT_OBJDECL:
		{	// <Obj Type> <Obj Id> <Obj List> <Script>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			prn << ' ';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);
			print_beautified(parser, parser.rt[rtpos].cChildPos+3);
			if( !prn.newline) prn << '\n';
			break;
		}
		case PT_OBJLIST:
		{	// <Obj Inst> <Obj List>
			print_beautified(parser, parser.rt[rtpos].cChildPos+0);
			prn << '\n';
			print_beautified(parser, parser.rt[rtpos].cChildPos+1);
			if( !prn.newline) prn << '\n';
			break;
		}
		default:
		{
			size_t j,k;
			k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
			for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
			{	// go down
				print_beautified(parser, j);
			}
			break;
		}// end default case
		}// end switch
	}
	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////
// parse processor
///////////////////////////////////////////////////////////////////////////////////////

/// parseprocessor.
/// combimes creating the parser and processing of the files
template <typename T>
class parseprocessor : public basics::noncopyable
{
	/// internal file processor.
	class myfileprocessor : public basics::CFileProcessor
	{
		parseprocessor& pp;
	public:
		myfileprocessor(parseprocessor& p) : pp(p)	{}
		~myfileprocessor()	{}
	
		/// file processor entry point
		virtual bool process(const char*name) const
		{
			return pp.processfile(name);
		}
	};
	friend class myfileprocessor;

	// copy assignment are forbidden, therefore pointer variables are safe
	T* cParser;
	basics::CParseConfig* cConfig;

public:
	parseprocessor(const unsigned char* (*engineloader)(ulong&));
	parseprocessor(const char* filename);
	~parseprocessor();

	/// loads a file or directory.
	bool load(const char* name);
protected:
	/// file processor entry point.
	/// reuse the parser when possible, 
	/// create a new parser on recursions (eg. include statement processing)
	bool processfile(const char* name);

public:
	virtual bool process(T& parser, const char* name) const=0;
	virtual const char* get_ext() const =0;
};

/// constructor.
template <typename T>
parseprocessor<T>::parseprocessor<T>(const unsigned char* (*engineloader)(ulong&)) : cParser(NULL), cConfig(NULL)
{
	ulong sz;
	const unsigned char *e;
	if( !engineloader || !(e = engineloader(sz)) )
	{
		fprintf(stderr, "Error creating parser\n");
	}
	else if( !(this->cConfig = new basics::CParseConfig(e, sz)) )
	{
		fprintf(stderr, "Could not load engine\n");
	}
}

/// constructor.
template <typename T>
parseprocessor<T>::parseprocessor<T>(const char* filename) : cParser(NULL), cConfig(NULL)
{
	if( !filename || !(this->cConfig = new basics::CParseConfig(filename)) )
	{
		fprintf(stderr, "Could not load engine from '%s'\n", filename);
	}
}

/// destructor.
template <typename T>
parseprocessor<T>::~parseprocessor()
{
	if(this->cParser) delete this->cParser;
	if(this->cConfig) delete this->cConfig;
}

/// loads a file or directory.
template <typename T>
bool parseprocessor<T>::load(const char* name)
{
	bool ok = false;
	if( basics::is_folder( name ) )
	{
		myfileprocessor fp(*this);
		ok = basics::findFiles(name, this->get_ext(), fp);
	}
	else if( basics::match_wildcard(this->get_ext(), name) )
	{	// single file
		ok = this->processfile(name);
	}
	return ok;
}

/// file processor entry point.
/// reuse the parser when possible, 
/// create a new parser on recursions (eg. include statement processing)
//## check using a pool instead [Shinomori]
template <typename T>
bool parseprocessor<T>::processfile(const char* name)
{
	if( !this->cConfig )
	{
		return false;
	}

	T* localparser = this->cParser ? this->cParser : new T(this->cConfig);
	this->cParser = NULL;
	if( !localparser )
	{
		fprintf(stderr, "Error creating parser\n");
		return false;
	}
	/////////////////////
	bool ret = this->process(*localparser, name);
	/////////////////////
	if( this->cParser )
		delete localparser;
	else
		this->cParser = localparser;
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////
// parse processor
///////////////////////////////////////////////////////////////////////////////////////


#define OPT_PARSE			0x00
#define OPT_BEAUTIFY		0x01
#define OPT_PRINTTREE		0x02
#define OPT_TRANSFORM		0x04
#define OPT_COMPILEDEBUG	0x08
#define OPT_COMPILEOUTPUT	0x10

class PParser : public parseprocessor<basics::CParser_CommentStore>
{
	CScriptCompiler&				compiler;
	int								option;
	mutable eaprinter				prn;

public:
	PParser(CScriptCompiler c, int o) : 
		parseprocessor<basics::CParser_CommentStore>(getEngine),
		compiler(c),
		option(o)
	{}
	/// work on files with '.txt' extension
	virtual const char* get_ext() const	{ return "*.txt"; }

	virtual bool process(basics::CParser_CommentStore& parser, const char* name) const
	{
		bool ok = true;
		bool run = true;

		// Open input file
		if( !parser.input.open(name) )
		{
			fprintf(stderr, "Could not open input file %s\n", name);
			return false;
		}
		else
		{
			fprintf(stderr, "processing input file %s\n", name);
		}

		while(run)
		{
			short p = parser.parse(PT_DECL);
			if (p < 0)
			{	// an error
				parser.print_expects(name);
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
					  child->symbol.idx == PT_DEFINE ||
					  child->symbol.idx == PT_INCLUDE
					  )
				  )
				{
					if( (option&OPT_BEAUTIFY)==OPT_BEAUTIFY )
					{
						prn.scope = 0; 
						prn.print_beautified(parser, 0);
						prn.print_comments(parser, -1);
						prn << '\n';
					}
					if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
					{
						fprintf(stderr, "(%li)----------------------------------------\n", (unsigned long)parser.rt.size());
						parser.print_rt_tree(0,0, false);
					}

					if( (option&OPT_TRANSFORM)==OPT_TRANSFORM || 
						(option&OPT_COMPILEDEBUG)==OPT_COMPILEDEBUG ||
						(option&OPT_COMPILEOUTPUT)==OPT_COMPILEOUTPUT )
					{
						//////////////////////////////////////////////////////////
						// tree transformation
						parsenode pnode(parser);

						if( (option&OPT_TRANSFORM)==OPT_TRANSFORM )
							pnode.print_tree();
						
						//////////////////////////////////////////////////////////
						// compiling
						if( (option&OPT_COMPILEDEBUG)==OPT_COMPILEDEBUG ||
						(option&OPT_COMPILEOUTPUT)==OPT_COMPILEOUTPUT )
							run = compiler.CompileTree(pnode);
					}
					
					//////////////////////////////////////////////////////////
					// reinitialize parser
					parser.reinit();
//					fprintf(stderr, "............................................(%i)\n", global::getcount());
				}
			}
		}
		parser.reset();
		return ok;
	}
};


void usage(const char*p)
{
	fprintf(stderr, "usage: %s [engine file] [bptco] <input file/folder>\n", (p)?p:"<binary>");
	fprintf(stderr, "     option b: outputs beautified code\n");
	fprintf(stderr, "     option p: prints parse tree\n");
	fprintf(stderr, "     option t: prints transformation tree\n");
	fprintf(stderr, "     option c: prints compilation debug and output\n");
	fprintf(stderr, "     option o: prints compilation output only\n");
}

int get_option(const char* p)
{
	int option = OPT_PARSE;
	if(p)
	{
		while(*p)
		{
			if(*p=='b')
				option |= OPT_BEAUTIFY;
			else if(*p=='p')
				option |= OPT_PRINTTREE;
			else if(*p=='t')
				option |= OPT_TRANSFORM;
			else if(*p=='c')
				option |= OPT_COMPILEDEBUG;
			else if(*p=='o')
				option |= OPT_COMPILEOUTPUT;
			p++;
		}
	}
	return option;
}

int main(int argc, char *argv[])
{
//	buildEngine();

	ulong tick = GetTickCount();
	bool ok=false;

	// parse commandline
	int i, c, option=OPT_PARSE;

	for(c=0, i=1; i<argc; ++i)
	{
		if( basics::is_file(argv[i]) || basics::is_folder(argv[i]) )
		{
			++c;
		}
		else
		{	// test for option or overwrite
			{	// option
				option = get_option(argv[i]);
			}
		}
	}
	if(!c)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	CScriptEnvironment env;
	CScriptCompiler compiler(env, ((option&OPT_COMPILEDEBUG)==OPT_COMPILEDEBUG)?2:1);
	PParser pp(compiler, option);

	for(i=1; i<argc; ++i)
	{
		if( basics::is_file(argv[i]) || basics::is_folder(argv[i]) )
		{
			if( !(ok=pp.load(argv[i])) )
				break;
		}
	}

	fprintf(stderr, "\nready (%i)\n", ok);
	fprintf(stderr, "elapsed time: %li\n", (unsigned long)(GetTickCount()-tick));
	return EXIT_SUCCESS;
}
