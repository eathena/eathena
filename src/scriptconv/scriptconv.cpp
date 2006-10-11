// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "basesafeptr.h"
#include "basetime.h"
#include "baseparser.h"
#include "basefile.h"

#include "scriptengine.h"


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
		int ret = printf(str);
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
						PT_COMMA==(parser.rt[se->cChildPos+1]).symbol.idx )
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


///////////////////////////////////////////////////////////////////////////////////////
// helpers
///////////////////////////////////////////////////////////////////////////////////////


const char* num2dir(int dir)
{
	static const char* dirnames[8] = 
	{
		"north",
		"northeast",
		"east",
		"southeast",
		"south",
		"southwest",
		"west",
		"northwest",
	};
	return dirnames[dir&0x07];
}

struct _dbstorage : public basics::noncopyable, public basics::nonallocable
{
	_dbstorage()
	{}
	~_dbstorage()
	{}

	struct itemdb_entry
	{
		basics::string<>	ID;				// mandatory	unique
		basics::string<>	Name1;			// mandatory	unique
		basics::string<>	Name2;			// optional		unique
		basics::string<>	Type;			// mandatory	
															// 0: Healing, 2: Usable, 3: Misc, 4: Weapon, 
															// 5: Armor, 6: Card, 7: Pet Egg,
															// 8: Pet Equipment, 10: Arrow, 
															// 11: Usable with delayed consumption (possibly unnecessary)
		basics::string<>	Price;			// mandatory	as said
		basics::string<>	Sell;			// optional		defaults to price/2
		basics::string<>	Weight;			// mandatory	
		basics::string<>	ATK;			// optional
		basics::string<>	DEF;			// optional
		basics::string<>	Range;			// optional
		basics::string<>	Slot;			// optional
		basics::string<>	Job;			// mandatory
		basics::string<>	Upper;			// optional
		basics::string<>	Gender;			// optional
		basics::string<>	Loc;			// optional
		basics::string<>	wLV;			// optional
		basics::string<>	eLV;			// optional
		basics::string<>	Refineable;		// optional
		basics::string<>	View;			// optional
		basics::string<>	UseScript;		// optional
		basics::string<>	EquipScript1;	// optional
		basics::string<>	EquipScript2;	// optional
	};

	struct mobdb_entry
	{
		basics::string<>	ID;
		basics::string<>	Name;
		basics::string<>	JKName;
		basics::string<>	IName;
		basics::string<>	LV;
		basics::string<>	HP;
		basics::string<>	SP;
		basics::string<>	BEXP;
		basics::string<>	JEXP;
		basics::string<>	Range1;
		basics::string<>	ATK1;
		basics::string<>	ATK2;
		basics::string<>	DEF;
		basics::string<>	MDEF;
		basics::string<>	STR;
		basics::string<>	AGI;
		basics::string<>	VIT;
		basics::string<>	INT;
		basics::string<>	DEX;
		basics::string<>	LUK;
		basics::string<>	Range2;
		basics::string<>	Range3;
		basics::string<>	Scale;
		basics::string<>	Race;
		basics::string<>	Element;
		basics::string<>	Mode;
		basics::string<>	Speed;
		basics::string<>	ADelay;
		basics::string<>	aMotion;
		basics::string<>	dMotion;
		basics::string<>	Drop1id;
		basics::string<>	Drop1per;
		basics::string<>	Drop2id;
		basics::string<>	Drop2per;
		basics::string<>	Drop3id;
		basics::string<>	Drop3per;
		basics::string<>	Drop4id;
		basics::string<>	Drop4per;
		basics::string<>	Drop5id;
		basics::string<>	Drop5per;
		basics::string<>	Drop6id;
		basics::string<>	Drop6per;
		basics::string<>	Drop7id;
		basics::string<>	Drop7per;
		basics::string<>	Drop8id;
		basics::string<>	Drop8per;
		basics::string<>	Drop9id;
		basics::string<>	Drop9per;
		basics::string<>	DropCardid;
		basics::string<>	DropCardper;
		basics::string<>	MEXP;
		basics::string<>	ExpPer;
		basics::string<>	MVP1id;
		basics::string<>	MVP1per;
		basics::string<>	MVP2id;
		basics::string<>	MVP2per;
		basics::string<>	MVP3id;
		basics::string<>	MVP3per;
	};
} dbstorage;





///////////////////////////////////////////////////////////////////////////////////////
// code beautifier
///////////////////////////////////////////////////////////////////////////////////////
//FILE *output=NULL;
bool output=true;
void printoutput(const char* str, int scope, bool &newline, bool &limiter)
{
	if(str)
	{
		if(newline)
		{
			int i;
			for(i=0; i<scope; ++i)
				printf("\t");
			newline=false;
		}
		else if(limiter && isalnum(str[0]))
		{
			printf(" ");
		}
		printf(str);
		limiter = 0!=isalnum( str[strlen(str)-1] );
	}
}


void print_comments(basics::CParser_CommentStore& parser, int &scope, bool &newline, bool &limiter, size_t linelimit)
{
	// print comments
	while( parser.cCommentList.size() )
	{
		if( parser.cCommentList[0].line < linelimit )
		{
			if(!newline) 
			{
				printoutput("\n", scope, newline, limiter);
				newline=true;
			}

			printoutput( (parser.cCommentList[0].multi)?"/*":"// ", scope, newline, limiter);
			printoutput( parser.cCommentList[0].content, scope, newline, limiter);
			printoutput( (parser.cCommentList[0].multi)?"*/\n":"\n", scope, newline, limiter);
			newline=true;
			parser.cCommentList.removeindex(0);
		}
		else
			break;
	}
}

bool print_beautified(basics::CParser_CommentStore& parser, int rtpos, int &scope, bool &newline, bool &limiter)
{
	if( output )
	{
		bool ret = true;

		if( parser.rt[rtpos].symbol.Type == 1 )
		{	// terminals

			print_comments(parser, scope, newline, limiter, parser.rt[rtpos].cToken.line);

			switch( parser.rt[rtpos].symbol.idx )
			{
			case PT_RBRACE:
				scope--;
				if(!newline) printoutput("\n", scope, newline, limiter);
				newline=true;
				printoutput("}\n", scope, newline, limiter);
				newline=true;
				break;
			case PT_LBRACE:
				if(!newline) printoutput("\n", scope, newline, limiter);
				newline=true;
				printoutput("{\n", scope, newline, limiter);
				newline=true;
				scope++;
				break;
			case PT_SEMI:
				printoutput(";\n", scope, newline, limiter);
				newline=true;
				break;
			case PT_COMMA:
				printoutput(", ", scope, newline, limiter);
				break;
			case PT_LPARAN:
			case PT_RPARAN:
				printoutput((const char*)parser.rt[rtpos].cToken.cLexeme, scope, newline, limiter);
				break;

			case PT_OLDSCRIPTHEAD:
			{	// split the old npc header
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				// old format:
				// map,x,y,d<tab>script<tab>name<tab>sprite,[x,y,]
				char map[32], name[32];
				int x=0,y=0,d=0,s=0,tx=0,ty=0;
				int res = sscanf(str, "%32[^,],%d,%d,%d\tscript\t%32[^\t]\t%d,%d,%d", 
										map,&x,&y,&d,name,&s,&tx,&ty);
				if( res>=6 )
				{	// new format:
					// npc [id] (map=map, xpos=x, ypos=y, dir=d,name=name, sprite=sprite, touchup={x,y})
					char buffer[1024];
					char idname[32], *ip, *kp;
					if( (ip=strstr(name, "::")) )
					{	// cut off the script id
						*ip=0;
						memcpy(idname, ip+2, sizeof(idname));
					}
					else
					{	// take full name as id
						memcpy(idname, name, sizeof(idname));
					}
					// make valid id
					ip=idname;
					if( *ip && !basics::stringcheck::isalpha(*ip) )
						*ip++='_';
					for(; *ip && ip<idname+sizeof(idname)-1; ++ip)
					{
						if( !basics::stringcheck::isalnum(*ip) )
							*ip++='_';
					}
					*ip=0;

					// cut map extension
					kp = strchr(map, '.');
					if(kp) *kp=0;

					if(res>6)
					{
						snprintf(buffer, sizeof(buffer),
							"npc %s (name=\"%s\", map=\"%s\", xpos=%d, ypos=%d, dir=%s, sprite=%d, touchup={%d,%d})",
							idname, name, map, x, y, num2dir(d), s, ty, ty);
					}
					else
					{
						snprintf(buffer, sizeof(buffer),
							"npc %s (name=\"%s\", map=\"%s\", xpos=%d, ypos=%d, dir=%s, sprite=%d)",
							idname, name, map, x, y, num2dir(d), s);
					}
					printoutput(buffer, scope, newline, limiter);
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}
			case PT_OLDMINSCRIPTHEAD:
			{	// split the old npc header
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				// old format:
				// -<tab>script<tab>name<tab>num
				char name[32];
				int s=0;
				int res = sscanf(str, "-\tscript\t%32[^\t]\t%d,", 
										name,&s);

				if( res>=1 )
				{	// new format:
					// npc [id] (name=name)
					char buffer[1024];
					char idname[32], *ip, *kp;
					for(ip=idname, kp=name; *kp; ++kp)
					{
						if( basics::stringcheck::isalnum(*kp) )
							*ip++ = *kp;
					}
					*ip=0;
					snprintf(buffer, sizeof(buffer),
						"npc %s (name=\"%s\")",
						idname, name);
					printoutput(buffer, scope, newline, limiter);
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}
			case PT_OLDFUNCHEAD:
			{
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				// old format:
				// function<tab>script<tab>name<tab>
				char name[32];
				int res = sscanf(str, "function\tscript\t%32[^\t]\t", 
										name);

				if( res>=1 )
				{	// new format:
					// <type> name([parameter])
					char buffer[1024];
					char idname[32], *ip, *kp;
					for(ip=idname, kp=name; *kp; ++kp)
					{
						if( basics::stringcheck::isalnum(*kp) )
							*ip++ = *kp;
					}
					*ip=0;
					snprintf(buffer, sizeof(buffer),
						"auto %s() // TODO: add real function parameters",
						idname);
					printoutput(buffer, scope, newline, limiter);
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}
			case PT_OLDMONSTERHEAD:
			{	
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				// old format:
				// <map> rectangle <name> <id>,<count>,<time1>,<time2>,<event>
				char map[32], name[32],event[128];
				int x1,y1,x2,y2,id,cn,t1,t2;
				int res = sscanf(str, "%32[^,],%d,%d,%d,%d\tmonster\t%32[^\t]\t%d,%d,%d,%d,%128s", 
									map,&x1,&y1,&x2,&y2,name,&id,&cn,&t1,&t2,event);
				if(res>=10)
				{
					char buffer[1024];
					char *kp;

					// cut map extension
					kp = strchr(map, '.');
					if(kp) *kp=0;

					if( (x1||x2||y1||y2) && (t1||t2) )
					{
						snprintf(buffer, sizeof(buffer),
							"monster (name=\"%s\", map=\"%s\", area={%d, %d, %d, %d}, sprite=%d, count=%d, respawn={%d, %d})",
							name, map, x1,y1,x2,y2,id,cn,t1,t2);
					}
					else if( (x1||x2||y1||y2) )
					{
						snprintf(buffer, sizeof(buffer),
							"monster (name=\"%s\", map=\"%s\", area={%d, %d, %d, %d}, sprite=%d, count=%d)",
							name, map, x1,y1,x2,y2,id,cn);
					}
					else if( (t1||t2) )
					{
						snprintf(buffer, sizeof(buffer),
							"monster (name=\"%s\", map=\"%s\", sprite=%d, count=%d, respawn={%d, %d})",
							name, map, id,cn,t1,t2);
					}
					else
					{
						snprintf(buffer, sizeof(buffer),
							"monster (name=\"%s\", map=\"%s\", sprite=%d, count=%d)",
							name, map, id,cn);
					}

					printoutput(buffer, scope, newline, limiter);

					if( strstr(event, "::") )
					{
						snprintf(buffer, sizeof(buffer),
							" %s",
							event);
					}
					else
					{
						snprintf(buffer, sizeof(buffer),
							";");
					}
					printoutput(buffer, scope, newline, limiter);
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}
			case PT_OLDWARPHEAD:
			{
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				// old format:
				// map,x,y,d<tab>warp<tab>name<tab>tx,ty,map,x,y
				char smap[32], tmap[32], name[32];
				int sx,sy,tx,ty,ux,uy,d;
				int res = sscanf(str, "%32[^,],%d,%d,%d\twarp\t%32[^\t]\t%d,%d,%32[^,],%d,%d", 
										smap,&sx,&sy,&d,name,&ux,&uy,tmap,&tx,&ty);
				if( res==10 )
				{	
					char buffer[1024];
					char idname[32], *ip, *kp;
					for(ip=idname, kp=name; *kp; ++kp)
					{
						if( basics::stringcheck::isalnum(*kp) )
							*ip++ = *kp;
					}
					*ip=0;
					kp=strchr(smap, '.'); if(kp) *kp=0;
					kp=strchr(tmap, '.'); if(kp) *kp=0;

					snprintf(buffer, sizeof(buffer),
						"warp %s (map=\"%s\", xpos=%d, ypos=%d, touchup={%d,%d}, target={\"%s\", %d, %d});",
						idname, smap,sx,sy,ux,uy,tmap,tx,ty);
					printoutput(buffer, scope, newline, limiter);
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}
			case PT_OLDMAPFLAGHEAD:
			{
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				// <map> mapflag <id> [<id> | position]
				// {File Ch}+ {SP Delim}+ 'mapflag' {SP Delim}+ {Id Tail}+ {SP Delim}* ({Head Ch}|',')*
				char map[32], type[32], param[128];
				int res = sscanf(str, "%32[^\t]\tmapflag\t%32[^\t]\t%128s", 
										map,type,param);
				if( res>=2 )
				{
					char buffer[1024];
					char *kp;
					char smap[32];
					memcpy(smap, map, sizeof(smap));
					kp=strchr(map, '.'); if(kp) *kp=0;

					if(res==2)
					{
						snprintf(buffer, sizeof(buffer),
									"map %s (file=\"%s\", flag=\"%s\");",
									map, smap, type);
					}
					else
					{
						snprintf(buffer, sizeof(buffer),
									"map %s (file=\"%s\", flag=\"%s,%s\");",
									map, smap, type, param);
					}

					printoutput(buffer, scope, newline, limiter);
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}
			case PT_OLDDUPHEAD:
			{
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				// old format:
				// map,x,y,d<tab>duplicate(id)<tab>name<tab>sprite,[x,y,]
				char map[32], name[32], id[32];
				int x=0,y=0,d=0,s=0,tx=0,ty=0;
				int res = sscanf(str, "%32[^,],%d,%d,%d\tduplicate(%32[^)])\t%32[^\t]\t%d,%d,%d", 
										map,&x,&y,&d,id,name,&s,&tx,&ty);
				if( res>=7 )
				{	// new format:
					// npc [id] (map=map, xpos=x, ypos=y, dir=d,name=name, sprite=sprite, touchup={x,y})
					char buffer[1024];
					char *kp;
		
					// cut map extension
					kp = strchr(map, '.');
					if(kp) *kp=0;

					if(res>7)
					{
						snprintf(buffer, sizeof(buffer),
							"npc (name=\"%s\", map=\"%s\", xpos=%d, ypos=%d, dir=%s, sprite=%d, touchup={%d,%d}) %s",
							name, map, x, y, num2dir(d), s, ty, ty, id);
					}
					else
					{
						snprintf(buffer, sizeof(buffer),
							"npc (name=\"%s\", map=\"%s\", xpos=%d, ypos=%d, dir=%s, sprite=%d) %s",
							name, map, x, y, num2dir(d), s, id);
					}
					printoutput(buffer, scope, newline, limiter);
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}

			case PT_OLDSHOPHEAD:
			{
				// {File Ch}+ ',' {Digit}+ ',' {Digit}+ ',' {Digit} {SP Delim}+ 'shop' {HT}+ {Head Ch}+ {HT}+      {Digit}+ ( {SP Delim}*',' {SP Delim}*{Digit}+ ':' {SP Delim}* [-]? {Digit}+ )*
				// '-'                                              {SP Delim}+ 'shop' {HT}+ {Head Ch}+ {HT}+      '-'      ( {SP Delim}*',' {SP Delim}*{Digit}+ ':' {SP Delim}* [-]? {Digit}+ )*
				const char* str = parser.rt[rtpos].cToken.cLexeme;
				const bool shopnpc = (*str!='-');
				char map[32]="", name[32]="";
				int x=0,y=0,d=0,s=0,n=0;
				bool ok;

				if( shopnpc )
				{	/// shop npc
					ok = 6==sscanf(str, "%32[^,],%d,%d,%d\tshop\t%32[^\t]\t%d,%n",
									map, &x,&y,&d,name,&s,&n);
				}
				else
				{	// virtual shop
					ok = 1==sscanf(str, "-\tshop\t%32[^\t]\t-,%n", name, &n);			
				}
				if(ok)
				{
					char buffer[1024], idname[32];
					memcpy(idname, name, sizeof(idname));
					// convert to id
					char*ip=idname;
					if( *ip && !basics::stringcheck::isalpha(*ip) )
						*ip++='_';
					for(; *ip && ip<idname+sizeof(idname)-1; ++ip)
					{
						if( !basics::stringcheck::isalnum(*ip) )
							*ip++='_';
					}
					
					// cut map extension
					if(shopnpc)
					{
						char *kp = strchr(map, '.');
						if(kp) *kp=0;

						snprintf(buffer, sizeof(buffer), "npc %s (name=\"%s\", map=\"%s\", xpos=%d, ypos=%d, dir=%d, sprite=%d)\n",
							idname, name, map, x,y,d,s);
					}
					else
					{
						sprintf(buffer, "npc %s (name=\"%s\")\n",
							idname, name);
					}
					printoutput(buffer, scope, newline, limiter);

					size_t cnt=0;
					int id, price;
					bool comma=false;
					str+=n-1;	// should stand at the first comma before the item/price list
					for(; *str==',' && 2==sscanf(str,",%d:%d%n", &id, &price,&n); str+=n, ++cnt)
					{	//## TODO: replace item id's with item names from db
						if(price>0)
							sprintf(buffer, "%c%i:%i", (comma?',':(comma=true,'\t')), id, price);
						else
							sprintf(buffer, "%c%i", (comma?',':(comma=true,'\t')), id);
						printoutput(buffer, scope, newline, limiter);
					}
					if( 0==cnt )
					{	// no items, just close the declaration
						printoutput(";", scope, newline, limiter);
					}
				}
				else
				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;

				break;
			}

			case PT_OLDITEMDBHEAD:
			case PT_OLDITEMDBHEAD_EA:
			case PT_OLDMOBDBHEAD:
			case PT_OLDMOBDBHEAD_EA:
			{
				const char* str = parser.rt[rtpos].cToken.cLexeme;

				{	// error, no format change
					printoutput(str, scope, newline, limiter);
				}
				printoutput("\n", scope, newline, limiter);
				newline=true;				
			}
			default:
				// print the token
				printoutput((const char*)parser.rt[rtpos].cToken.cLexeme, scope, newline, limiter);
				break;
			}
		}
		else if( parser.rt[rtpos].cChildNum==1 )
		{	// only one child, just go down
			print_beautified(parser, parser.rt[rtpos].cChildPos, scope, newline, limiter);
		}
		else if( parser.rt[rtpos].cChildNum>1 )
		{	// nonterminals
			switch( parser.rt[rtpos].symbol.idx )
			{
			case PT_LABELSTM:
			{
				int tmpscope = scope;
				scope=0;
				print_beautified(parser, parser.rt[rtpos].cChildPos, scope, newline, limiter);
				printoutput(":\n", scope, newline, limiter);
				newline=true;
				scope = tmpscope;
				break;
			}
			case PT_CALLSTM:
			{
				if( parser.rt[parser.rt[rtpos].cChildPos].cToken.cLexeme=="callfunc" )
				{	// transform to real function calls
					basics::CStackElement& listnode = parser.rt[parser.rt[rtpos].cChildPos+1];
					if(listnode.cChildNum)
					{	// name
						char buffer[128], *ip;
						const char* kp=parser.rt[listnode.cChildPos].cToken.cLexeme;
						++kp;
						for(ip=buffer; *kp && *kp!='"'; ++kp)
							if(ip<buffer+sizeof(buffer)-1) *ip++=*kp;
						*ip=0;

						printoutput(buffer, scope, newline, limiter);
						
						// argument list
						printoutput("(", scope, newline, limiter);
						size_t j,k;
						k = listnode.cChildPos+listnode.cChildNum;
						for(j=listnode.cChildPos+2; j<k; ++j)
						{	// go down
							print_beautified(parser, j, scope, newline, limiter);
						}
						printoutput(");\n", scope, newline, limiter);
					}
					else
					{	// listnode itself is the name, so strip any quote
						char buffer[128], *ip;
						const char* kp=listnode.cToken.cLexeme;
						++kp;
						for(ip=buffer; *kp && *kp!='"'; ++kp)
							if(ip<buffer+sizeof(buffer)-1) *ip++=*kp;
						*ip=0;

						printoutput(buffer, scope, newline, limiter);
						printoutput("();\n", scope, newline, limiter);
					}
				}
				else if( parser.rt[parser.rt[rtpos].cChildPos].cToken.cLexeme=="set" )
				{	// transform "set" functions
					// from "set <a>, <b>" to "<a> = <b>;
					basics::CStackElement& listnode = parser.rt[parser.rt[rtpos].cChildPos+1];
					
					print_beautified(parser, listnode.cChildPos, scope, newline, limiter);
					printoutput(" = ", scope, newline, limiter);
					print_beautified(parser, listnode.cChildPos+2, scope, newline, limiter);
					printoutput(";\n", scope, newline, limiter);
				}
				else
				{	// transform to function calls
					print_beautified(parser, parser.rt[rtpos].cChildPos, scope, newline, limiter);
					printoutput("(", scope, newline, limiter);
					if( parser.rt[rtpos].cChildNum==3 )
						print_beautified(parser, parser.rt[rtpos].cChildPos+1, scope, newline, limiter);
					printoutput(");\n", scope, newline, limiter);
				}
				newline=true;
				break;
			}
			case PT_NORMALSTM:
			{	// can be:
				// if '(' <Expr> ')' <Normal Stm>
				// if '(' <Expr> ')' <Normal Stm> else <Normal Stm>
				// while '(' <Expr> ')' <Normal Stm>
				// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
				// do <Normal Stm> while '(' <Expr> ')' ';'
				// switch '(' <Expr> ')' '{' <Case Stms> '}'
				// <ExprList> ';'
				// ';'              !Null statement
				if(!newline)
				{
					printoutput("\n", scope, newline, limiter);
					newline=true;
				}

				if( PT_IF == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx ||
					PT_WHILE == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("( ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput(" )\n", scope, newline, limiter);
					newline = true;

					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
						scope++;
					print_beautified(parser, parser.rt[rtpos].cChildPos+4, scope, newline, limiter);
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
						scope--;

					if( parser.rt[rtpos].cChildNum==7 )
					{
						print_beautified(parser, parser.rt[rtpos].cChildPos+5, scope, newline, limiter);
						if(!newline)
						{
							printoutput("\n", scope, newline, limiter);
							newline=true;
						}
						if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
							scope++;
						print_beautified(parser, parser.rt[rtpos].cChildPos+6, scope, newline, limiter);
						if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
							scope--;
					}
				}
				else if( PT_DO == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{	// do <Normal Stm> while '(' <Expr> ')' ';'
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("\n", scope, newline, limiter);
					newline = true;
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
						scope++;
					print_beautified(parser, parser.rt[rtpos].cChildPos+1, scope, newline, limiter);
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
						scope--;
					if(!newline)
					{
						printoutput("\n", scope, newline, limiter);
						newline=true;
					}
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput("( ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+4, scope, newline, limiter);
					printoutput(" )", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+6, scope, newline, limiter);
				}
				else if( PT_SWITCH == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{	// switch '(' <Expr> ')' '{' <Case Stms> '}'
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("( ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput(" )\n", scope, newline, limiter);
					newline = true;
					printoutput("{\n", scope, newline, limiter);
					newline = true;
					print_beautified(parser, parser.rt[rtpos].cChildPos+5, scope, newline, limiter);
					if(!newline)
					{
						printoutput("\n", scope, newline, limiter);
						newline=true;
					}
					printoutput("}\n", scope, newline, limiter);
					newline = true;
				}
				else if( PT_FOR == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
				{	// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, scope, newline, limiter);
					printoutput("(", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+2, scope, newline, limiter);
					printoutput("; ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+4, scope, newline, limiter);
					printoutput("; ", scope, newline, limiter);
					print_beautified(parser, parser.rt[rtpos].cChildPos+6, scope, newline, limiter);
					printoutput(")\n", scope, newline, limiter);
					newline=true;
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
						scope++;
					print_beautified(parser, parser.rt[rtpos].cChildPos+8, scope, newline, limiter);
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+8 ].symbol.idx )
						scope--;
				}
				else
				{
					size_t j,k;
					k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
					j = parser.rt[rtpos].cChildPos;
					for(; j<k; ++j)
					{
						print_beautified(parser, j, scope, newline, limiter);
					}
				}
				break;
			}
			case PT_CASESTMS:
			{	// <Case Stms>  ::= case <Value> ':' <Stm List> <Case Stms>
				//			   | default ':' <Stm List> <Case Stms>
				//			   |
				size_t j,k;
				int tmpscope = scope;
				k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
				for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
				{	// go down
					if( PT_COLON==parser.rt[j].symbol.idx )
					{
						printoutput(":\n", scope, newline, limiter);
						newline=true;
						scope++;
					}
					else
					{
						if( PT_CASESTMS==parser.rt[j].symbol.idx )
							scope--;
						print_beautified(parser, j, scope, newline, limiter);
					}
				}
				scope = tmpscope;
				break;
			}
			default:
			{
				size_t j,k;
				k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
				for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
				{	// go down
					print_beautified(parser, j, scope, newline, limiter);
				}
				break;
			}// end default case
			}// end switch
		}
		return ret;
	}
	return false;
}





///////////////////////////////////////////////////////////////////////////////////////
// parse processor
///////////////////////////////////////////////////////////////////////////////////////


#define OPT_PARSE			0x00
#define OPT_BEAUTIFY		0x01
#define OPT_PRINTTREE		0x02
#define OPT_TRANSFORM		0x04


class PParser : public basics::CFileProcessor
{
	basics::CParser_CommentStore*	parser;
	int								option;

public:
	PParser(basics::CParser_CommentStore* p, int o) : parser(p), option(o)
	{}

	virtual bool process(const char*name) const
	{
		bool ok = true;
		bool run = true;

		// Open input file
		if( !parser->input.open(name) )
		{
			printf("Could not open input file %s\n", name);
			return false;
		}
		else
		{
			printf("processing input file %s\n", name);
		}

		while(run)
		{
			short p = parser->parse(PT_DECL);
			if (p < 0)
			{	// an error
				printf("Parse Error in file '%s', line %i, col %i\n", name, parser->input.line, parser->input.column);

				parser->print_expects();

				run = false;
				ok = false;
			}
			else if(0 == p)
			{	// finished
				run = false;
			}			
			if( ok && parser->rt[0].symbol.idx==PT_DECL && parser->rt[0].cChildNum )
			{
				basics::CStackElement *child = &(parser->rt[parser->rt[0].cChildPos]);
				if( child &&
					( child->symbol.idx == PT_BLOCK ||

					  child->symbol.idx == PT_OLDSCRIPT ||
					  child->symbol.idx == PT_OLDFUNC ||
					  child->symbol.idx == PT_OLDMAPFLAG ||
					  child->symbol.idx == PT_OLDNPC ||
					  child->symbol.idx == PT_OLDDUP ||
					  child->symbol.idx == PT_OLDMOB ||
					  child->symbol.idx == PT_OLDSHOP ||
					  child->symbol.idx == PT_OLDWARP ||
					  child->symbol.idx == PT_OLDITEMDB ||
					  child->symbol.idx == PT_OLDMOBDB ||

					  child->symbol.idx == PT_OLDDUPHEAD ||
					  child->symbol.idx == PT_OLDFUNCHEAD ||
					  child->symbol.idx == PT_OLDITEMDBHEAD ||
					  child->symbol.idx == PT_OLDITEMDBHEAD_EA ||
					  child->symbol.idx == PT_OLDMAPFLAGHEAD ||
					  child->symbol.idx == PT_OLDMINSCRIPTHEAD ||
					  child->symbol.idx == PT_OLDMOBDBHEAD ||
					  child->symbol.idx == PT_OLDMOBDBHEAD_EA ||
					  child->symbol.idx == PT_OLDMONSTERHEAD ||
					  child->symbol.idx == PT_OLDSCRIPTHEAD ||
					  child->symbol.idx == PT_OLDSHOPHEAD ||
					  child->symbol.idx == PT_OLDWARPHEAD
					  )
				  )
				{
					if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
					{
						printf("(%li)----------------------------------------\n", (unsigned long)parser->rt.size());
						parser->print_rt_tree(0,0, false);
					}

					if( (option&OPT_TRANSFORM)==OPT_TRANSFORM )
					{
						//////////////////////////////////////////////////////////
						// tree transformation
						parsenode pnode(*parser);
						printf("----------------------------------------\n");
						pnode.print_tree();
					}
					if( (option&OPT_BEAUTIFY)==OPT_BEAUTIFY )
					{
						bool newline=true;
						bool limiter=true;
						int scope = 0; 
						print_beautified(*parser, 0, scope, newline, limiter);
						print_comments(*parser, scope, newline, limiter, 0xFFFFFFFF);
					}					
					//////////////////////////////////////////////////////////
					// reinitialize parser
					parser->reinit();
//					printf("............................................(%i)\n", global::getcount());
				}
			}
		}
		parser->reset();
		return ok;
	}
};

void usage(const char*p)
{
	printf("usage: %s [engine file] [bptco] <input file/folder>\n", (p)?p:"<binary>");
	printf("     option b: outputs beautified code\n");
	printf("     option p: prints parse tree\n");
	printf("     option t: prints transformation tree\n");
}

int get_option(const char* p)
{
	int option = OPT_PARSE;
	if(p)
	{
		for(; *p; ++p)
		{
			if(*p=='b')
				option |= OPT_BEAUTIFY;
			else if(*p=='p')
				option |= OPT_PRINTTREE;
			else if(*p=='t')
				option |= OPT_TRANSFORM;
		}
	}
	return option;
}


// Accepts 3 arguments [engine file] [option(s)] <input file>
int main(int argc, char *argv[])
{
//	buildEngine();

	ulong tick = GetTickCount();
	basics::CParser_CommentStore* parser = 0;
	basics::CParseConfig* parser_config = 0;
	bool ok;


	// parse commandline
	const char* enginefile=NULL;
	const char* inputfile=NULL;
	int i, option=OPT_PARSE;

	for(i=1; i<argc; ++i)
	{
		if( basics::is_file(argv[i]) )
		{
			if(!enginefile)
				enginefile = argv[i];
			else 
				inputfile = argv[i];
		}
		else if( basics::is_folder(argv[i]) )
		{
			inputfile = argv[i];
		}
		else
		{
			option = get_option(argv[i]);
		}
	}
	if(enginefile && !inputfile)
		basics::swap(enginefile, inputfile);

	if(!inputfile)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if(enginefile)
	{
		try
		{
			parser_config = new basics::CParseConfig( enginefile );
		}
		catch(...)
		{
			parser_config=NULL;
		}
		if (!parser_config)
		{
			printf("Could not open engine file %s\n", enginefile);
			return EXIT_FAILURE;		
		}
	}
	else
	{
		ulong sz;
		const unsigned char *e = getEngine(sz);
		if(!e)
		{
			printf("Error creating parser\n");
			return EXIT_FAILURE;
		}
		parser_config = new basics::CParseConfig(e, sz);
		if (!parser_config)
			printf("Could not load engine\n");

	}

	parser = new basics::CParser_CommentStore(parser_config);
	if (!parser){
		printf("Error creating parser\n");
		return EXIT_FAILURE;
	}

	PParser pp(parser, option);

	if( basics::is_folder( inputfile ) )
	{
		ok=basics::findFiles(inputfile, "*.txt", pp);
	}
	else
	{	// single file
		ok=pp.process( inputfile );
	}
	printf("\nready (%i)\n", ok);
	if (parser)  delete parser;
	if (parser_config) delete parser_config;

	printf("elapsed time: %li\n", (unsigned long)(GetTickCount()-tick));

	return EXIT_SUCCESS;
}
