// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "aegisscriptconv.h"
#include "aegisscript.h"


/// array for compile-emedded script engine
const unsigned char aegisengine[] = 
{
#include "aegisscript.engine.txt"
};


bool aegisparserstorage::getEngine(const unsigned char*& buf, unsigned long& sz)
{
	buf=aegisengine; sz =sizeof(aegisengine);
	return true;
}


bool aegisprinter::print_npchead(const basics::string<>& name,
								 const basics::string<>& map,
								 const basics::string<>& xpos,
								 const basics::string<>& ypos,
								 const char* dir,
								 const char* sprite,
								 const basics::string<>& touchx,
								 const basics::string<>& touchy)
{
	aegisprinter& prn = *this;
	char tmpname[32];
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

	// strip quotes from name
	str2strip_quotes(tmpname, sizeof(tmpname),name);

	// set npc globals
	this->cAegisName = name;
	this->cEAName = tmpname;
	this->cMapName = map;
	this->cHasTmpstr = false;
	this->cHasTmpval = false;


	prn << "npc ";
	prn.print_id(tmpname);
	prn << " (name=\"";
	prn.print_name(tmpname);
	prn << "\"";

	prn <<  ", map=" << map <<
			", xpos="  << atoi(xpos) << 
			", ypos="  << atoi(ypos) << 
			", dir=" << dirnames[(dir?atoi(dir)&0x07:0)] << 
			", sprite=" << sprite;

	if( 0!=atoi(touchx) ||
		0!=atoi(touchy) )
	{
		prn << ", touchup={"
			<< atoi(touchx) << ", "
			<< atoi(touchy) << '}';
	}
	prn << ')';
	prn << '\n';
	return true;
}

bool aegisprinter::print_callstm(basics::CParser_CommentStore& parser, int func, int param)
{
	aegisprinter& prn = *this;
	basics::CStackElement* funcname;

	// find the function name
	for(funcname = &parser.rt[ func ]; funcname->symbol.Type != 1; funcname = &parser.rt[ funcname->cChildPos ]) {}
	if( funcname->symbol.idx == AE_IDENTIFIER )
	{
		basics::vector<size_t> parameter;
		if(param>0) // valid rt position
		{
			basics::CStackElement* node = &parser.rt[ param ];
			if( node->symbol.idx==AE_CALLLIST )
			{	// a list
				for(;;)
				{
					if( node->cChildNum )
					{
						parameter.push(node->cChildPos+0); // first child is always valid
						if( node->cChildNum>1 )
						{	
							if( parser.rt[node->cChildPos+1].symbol.idx == AE_CALLLIST )
							{	// calllist continued
								node = &parser.rt[node->cChildPos+1];
								continue;
							}
							else
							{	// a final value node
								parameter.push(node->cChildPos+1); // first child is always valid
							}
						}
					}
					// always finish 
					break;
				}
			}
			else
			{	// a single value
				parameter.push(param);
			}	
		}
		
		//////////////////////////////////////////////////////////////////
		// function remapping
		if( funcname->cToken.cLexeme == "dialog" )
		{	// open message window and displaytext
			// new command can have arbitrary arguments, so skip any test
			prn << "dialog" << '(';
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "wait" )
		{	// displays the 'next' click button, 
			// no parameters allowed, just ignore them
			prn << "next()";
		}
		else if( funcname->cToken.cLexeme == "close" )
		{	// displays the 'close' click button, 
			// no parameters allowed, just ignore them
			prn << "close" << '(' << ')';
		}
		else if( funcname->cToken.cLexeme == "menu" )
		{	// displays a selection, aegis menu translates to ea select
			// arbitrary arguments, so skip any test
			prn << "select" << '(';
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "dlgwritestr" )
		{	// string input box
			// aegis default variable is "inputstr"
			// we'll need to make it explicit
			if( !this->cHasTmpstr ) this->cHasTmpstr=true, prn << "temp ";
			prn << "temp tmpstr = inputstring()";
		}
		else if( funcname->cToken.cLexeme == "dlgwrite" )
		{	// number input box with range check
			// aegis default variable is "input", 
			// additionally the variable "error" gets set to 1 if in case of error
			// we'll need to make it explicit
			if( !this->cHasTmpval ) this->cHasTmpval=true, prn << "temp ";
			prn << "temp tmpval = inputnumber()";
		}
		else if( funcname->cToken.cLexeme == "exitwhile" )
		{	// exit from a while loop
			// translates to break, ignores parameters
			prn << "break";
		}
		else if( funcname->cToken.cLexeme == "SetGlobalVar" ||
				 funcname->cToken.cLexeme == "SetGlobalStr" )
		{	// as names say
			// translates to assignment, 2 parameters
			if( parameter.size()>0 )
			{
				basics::CStackElement* node=&parser.rt[ parameter[0] ];
				for(; node->cChildNum==1; node = &parser.rt[ node->cChildPos ]) {}
				if( node->symbol.Type == 1 ) // should be a stringliteral
				{
					prn << "global::";
					this->print_idstring(node->cToken.cLexeme);
					prn << ' ' << '=' << ' ';
					if( parameter.size()>1 )
						print_beautified(parser, parameter[1], AE_EXPR);
					else
						prn << '0';
				}
			}
		}
		else if( funcname->cToken.cLexeme == "GetGlobalVar" ||
				 funcname->cToken.cLexeme == "GetGlobalStr" )
		{	// as names say
			// translates to assignment, 1 parameters
			if( parameter.size()>0 )
			{
				basics::CStackElement* node=&parser.rt[ parameter[0] ];
				for(; node->cChildNum==1; node = &parser.rt[ node->cChildPos ]) {}
				if( node->symbol.Type == 1 ) // should be a stringliteral
				{
					prn << "global::";
					this->print_idstring(node->cToken.cLexeme);
				}
				else
					prn << '0';
			}
		}
		else if( funcname->cToken.cLexeme == "dropitem" )
		{	// removes items
			// translates to delitem, 2 parameters
			prn << "delitem";
			prn << "select" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "getitem" )
		{	// adds items
			// translates to getitem, 2 parameters
			prn << "getitem" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "CheckMaxCount" && parameter.size()==2 )
		{	// possibly tests the itemcount with the second number
			// translates to countitem with compare, 2 parameters
			
			prn << '(';
			print_beautified(parser, parameter[1], AE_EXPR); // second has the number
			prn << ' ' << '<' << ' '
				<< "countitem(";
			print_beautified(parser, parameter[0], AE_EXPR);
			prn << ')' << ')';
		}
		else if( funcname->cToken.cLexeme == "GetPCCount" )
		{	// get number pc in map
			// translates to getmapusers, 1 parameter
			
			prn << "getmapusers(";
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "Emotion" )
		{	// emotion
			// translates to emotion, 1 parameter
			prn << "emotion(";
////////
// add parameter test
			
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "rand" )
		{	// random number
			// translates to rand, 2 parameters

			prn << "rand" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "cmdothernpc" )
		{	// cmdothernpc
			// translates to cmdothernpc, 2 parameters

			prn << "cmdothernpc" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "moveto" )
		{	// moveto
			// translates to warp, 3 parameters

			prn << "warp" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "makewaitingroom" )
		{	// makewaitingroom
			// translates to waitingroom, 2 parameters
			prn << "waitingroom" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "warpwaitingpctoarena" )
		{	// warpwaitingpctoarena
			// translates to warpwaitingpc, 3 parameters
			prn << "warpwaitingpc" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "callmonster" )
		{	// callmonster creates npc bound monsters
			// ->
/*
			// new function npc::createmonster
			prn << "npc::createmonster" << '(';
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
*/
			// use old ea functions as other option (with slight enhancement)
			// aegis:	callmonster <map> <id> <name> <x> <y>
			// ea:		monster		<map>,<x>,<y>,<name>,<id>,<cnt>,<event>
			// default aegis event name is the "OnMyMobDead"

			prn << "npc::mymobcount += monster(";
			print_beautified(parser, parameter[0], AE_EXPR);
			prn << ',' << ' ';
			print_beautified(parser, parameter[3], AE_EXPR);
			prn << ',' << ' ';
			print_beautified(parser, parameter[4], AE_EXPR);
			prn << ',' << ' ';
			print_beautified(parser, parameter[2], AE_EXPR);
			prn << ',' << ' ';
			print_beautified(parser, parameter[1], AE_EXPR);
			prn << ',' << ' ' << '1' << ',' << ' '
				<< '\"' << this->cEAName << "::" << "OnMyMobDead" << '\"' << ')';
		}
		else if( funcname->cToken.cLexeme == "resetmymob" )
		{	// resetmymob removes called npc monsters
			// ->
/*			// new function npc::removemonster
			// no parameters
			prn << "npc::removemonster" << '(' << ')';
*/
			// use old ea functions as other option
			// killmonster with mapname and eventname
			prn << "killmonster(" << this->cMapName << ',' << ' '
				<< '\"' << this->cEAName << "::" << "OnMyMobDead" << '\"' << ')' << ';' << '\n'
				<< "npc::mymobcount = 0";
		}
		else if( funcname->cToken.cLexeme == "enablenpc" )
		{	// enablenpc
			// ->
			// enablenpc, 1 parameter
			prn << "enablenpc" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "disablenpc" )
		{	// enablenpc
			// ->
			// enablenpc, 1 parameter
			prn << "disablenpc" << '(';
////////
// add parameter test
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		//////////////////////////////////////////////////////////////////
		else
		{	// default: just use the given function name

			prn.log(parser, func);

			print_beautified(parser, func, AE_CALLSTM);
			// function parameters if exist
			prn << '(';
			if( param>0 ) print_beautified(parser, param, AE_EXPR);
			prn << ')';
		}
		return true;
	}
	return false;
}

bool aegisprinter::print_varray(basics::CParser_CommentStore& parser, int rtpos, short parent)
{	// <Value> '[' <Expr> ']'
	aegisprinter& prn = *this;
//////
// replace with player variable access
//////

	basics::CStackElement* node=&parser.rt[ parser.rt[rtpos].cChildPos+2 ];
	for(; node->cChildNum==1; node = &parser.rt[ node->cChildPos ]) {}

	if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_SEX" )
	{	// access player variable
		prn << "player::gender()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_MAXWEIGHT" )
	{	// access player variable
		prn << "player::maxweight()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_WEIGHT" )
	{	// access player variable
		prn << "player::weight()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_MONEY" )
	{	// access player variable
		prn << "player::zeny()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_JOB" )
	{	// access player variable
		prn << "player::job()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_ISRIDING" )
	{	// access player variable
		prn << "player::is_riding()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_ISRIDING" )
	{	// access player variable
		prn << "player::is_riding()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_ISPECOON" )
	{	// access player variable
		prn << "player::is_riding()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_ISPARTYMASTER" )
	{	// access player variable
		prn << "player::is_partymaster()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_CLEVEL" )
	{	// access player variable
		prn << "player::baselevel()";
	}
	else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_CLEVEL" )
	{	// access player variable
		prn << "player::skillevel(\"NV_BASIC\")";
	}
// the others should be intentory access but double check with a loaded itemdb later
	else if( node->symbol.idx == AE_IDENTIFIER )
	{
		prn << "countitem(\"" << node->cToken.cLexeme << "\")";
	}
	else
	{	// default
		prn.log(parser, rtpos);
		prn << "variable";
		prn << '[';
		print_beautified(parser, parser.rt[rtpos].cChildPos+2, AE_ARRAY);
		prn << ']';
	}
	return true;
}
bool aegisprinter::print_npcvarray(basics::CParser_CommentStore& parser, int rtpos, short parent)
{	// 'npcv' <stringliteral> '[' <Expr> ']'
	aegisprinter& prn = *this;

	basics::CStackElement* node=&parser.rt[ parser.rt[rtpos].cChildPos+3 ];
	for(; node->cChildNum==1; node = &parser.rt[ node->cChildPos ]) {}

	if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_MYMOBCOUNT" )
	{	
		if( this->cAegisName != parser.rt[ parser.rt[rtpos].cChildPos+1 ].cToken.cLexeme )
		{	// warn on name conflicts
			fprintf(stderr, "possibly broken logic at line %i.\n"
							"calling mymobcount with npc %s while beeing in npc %s\n"
							"using the current npc as target\n", 
							(int)parser.rt[parser.rt[rtpos].cChildPos].cToken.line,
							(const char*)parser.rt[ parser.rt[rtpos].cChildPos+1 ].cToken.cLexeme,
							(const char*)this->cAegisName);
		}
/*
		prn << "npc::countmonster()";
*/
		// other option using old ea style
		// using a npc variable which is set at createmonster/killmonster
		prn << "npc::mymobcount";

	}
	// check what's there later
	else
	{
		prn.log(parser, rtpos);
		prn << "npc_variable(";
		print_beautified(parser, parser.rt[rtpos].cChildPos+1, AE_EXPR);
		prn << ')';
		prn << '[';
		print_beautified(parser, parser.rt[rtpos].cChildPos+3, AE_ARRAY);
		prn << ']';
	}
	return true;
}


bool aegisprinter::print_opand(basics::CParser_CommentStore& parser, int rtpos, short parent)
{	// <Op BinAND> '&' <Op Equate>
	// code has been using constructs like (var>0)&(var<2)
	// which can be combined to var == 1
	// so do an extra test on these it just costs nothing
	if( parser.rt[rtpos].cChildNum==3 )
	{
		basics::CStackElement* lnode=&parser.rt[ parser.rt[rtpos].cChildPos+0 ];
		basics::CStackElement* rnode=&parser.rt[ parser.rt[rtpos].cChildPos+2 ];

		for(; lnode->cChildNum==1; lnode = &parser.rt[ lnode->cChildPos ]) {}
		for(; rnode->cChildNum==1; rnode = &parser.rt[ rnode->cChildPos ]) {}
		if( lnode->symbol.idx==AE_VALUE && lnode->cChildNum==3 &&
			rnode->symbol.idx==AE_VALUE && rnode->cChildNum==3 )
		{
			lnode = &parser.rt[ lnode->cChildPos+1 ];
			rnode = &parser.rt[ rnode->cChildPos+1 ];

			for(; lnode->cChildNum==1; lnode = &parser.rt[ lnode->cChildPos ]) {}
			for(; rnode->cChildNum==1; rnode = &parser.rt[ rnode->cChildPos ]) {}
			
			if( lnode->symbol.idx == AE_OPCOMPARE && lnode->cChildNum==3 &&
				rnode->symbol.idx == AE_OPCOMPARE && rnode->cChildNum==3 )
			{	
				basics::CStackElement *rid, *rop, *rva, *lid, *lop, *lva;
				rid = &parser.rt[ rnode->cChildPos+0 ];
				rop = &parser.rt[ rnode->cChildPos+1 ];
				rva = &parser.rt[ rnode->cChildPos+2 ];
				lid = &parser.rt[ lnode->cChildPos+0 ];
				lop = &parser.rt[ lnode->cChildPos+1 ];
				lva = &parser.rt[ lnode->cChildPos+2 ];

				for(; rid->cChildNum==1; rid = &parser.rt[ rid->cChildPos ]) {}
				for(; rop->cChildNum==1; rop = &parser.rt[ rop->cChildPos ]) {}
				for(; rva->cChildNum==1; rva = &parser.rt[ rva->cChildPos ]) {}
				for(; lid->cChildNum==1; lid = &parser.rt[ lid->cChildPos ]) {}
				for(; lop->cChildNum==1; lop = &parser.rt[ lop->cChildPos ]) {}
				for(; lva->cChildNum==1; lva = &parser.rt[ lva->cChildPos ]) {}

				if( rid->symbol.idx==AE_IDENTIFIER &&
					lid->symbol.idx==AE_IDENTIFIER &&
					rid->cToken.cLexeme == lid->cToken.cLexeme &&
					lop->symbol.idx==AE_GT &&
					rop->symbol.idx==AE_LT &&
					lva->symbol.idx==AE_DECLITERAL &&
					rva->symbol.idx==AE_DECLITERAL )
				{
					int lval = atoi(lva->cToken.cLexeme);
					int rval = atoi(rva->cToken.cLexeme);
					if( rval-lval == 2 )
					{
						aegisprinter& prn = *this;
						prn << (lval+1) << ' ' << '=' << '=' << ' ';
						print_beautified(parser, rnode->cChildPos+0, parent); // id
						return true;
					}
				}
			}
		}
		// normal processing otherwise
		print_beautified(parser, parser.rt[rtpos].cChildPos+0, parent);
		print_beautified(parser, parser.rt[rtpos].cChildPos+1, parent);
		print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
	}
	return true;
}

bool aegisprinter::print_idstring(basics::string<>& origstr)
{
	aegisprinter& prn = *this;
	basics::string<> copystr;
	const char* str = origstr;
	if(*str == '\"')
	{	// strip quotes
		copystr = origstr(1,(origstr.size()>2)?origstr.size()-2:0);
		str = copystr;
	}

	// check for reserved words
	// aegis identifiers can start with numbers
	bool reserved = ( 
				(!basics::stringcheck::isalpha(*str) && *str!='_') ||
				0==strcasecmp(str,"if") ||
				0==strcasecmp(str,"else") ||
				0==strcasecmp(str,"for") ||
				0==strcasecmp(str,"while") ||
				0==strcasecmp(str,"return") ||
				0==strcasecmp(str,"goto") ||
				0==strcasecmp(str,"string") ||
				0==strcasecmp(str,"int") ||
				0==strcasecmp(str,"double") ||
				0==strcasecmp(str,"auto") ||
				0==strcasecmp(str,"var") );

		if(reserved) prn << '_';

		for(; *str; ++str)
			prn << (basics::stringcheck::isalnum(*str)?*str:'_');

		if(reserved) prn << '_';
		return true;
}

bool aegisprinter::print_identifier(basics::CParser_CommentStore& parser, int rtpos, short parent)
{
	aegisprinter& prn = *this;
	if( parser.rt[rtpos].cToken.cLexeme == "inputstr" )
	{	// variable for string input box
		prn << "temp::tmpstr";
	}
	else if( parser.rt[rtpos].cToken.cLexeme == "input" )
	{	// variable for number input box
		prn << "temp::tmpval";
	}
	else if( parser.rt[rtpos].cToken.cLexeme == "error" )
	{	// variable for number input box
		prn << "temp::tmpval<0";
	}
	else if( parser.rt[rtpos].cToken.cLexeme == "PcName" )
	{	// variable for number input box
		prn << "player::name";
	}
	else
	{
////////////////////
/*		if( parent != AE_CALLSTM &&
			parent != AE_LABEL &&
			parent != AE_VARDECL )
		{
			prn << '/' << '*'<< parent << '*' << '/';
		}

		if( parent == AE_EXPR )
		{	// most possibly a temp variable
			prn << "temp::";
		}
*/
////////////////////
		return this->print_idstring(parser.rt[rtpos].cToken.cLexeme);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
bool aegisprinter::print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent)
{
	aegisprinter& prn = *this;
	bool ret = true;

	prn.print_comments(parser, rtpos);

	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// terminals
		// in the grammer exists: Whitespace, NewLine, DecLiteral, StringLiteral, identifier
		
		switch( parser.rt[rtpos].symbol.idx )
		{
		case AE_IDENTIFIER:
		{	
			print_identifier(parser, rtpos, parent);
			break;
		}
		case AE_STRINGLITERAL:
		{
			const char*str = parser.rt[rtpos].cToken.cLexeme;
			// check for control chars
			for(; *str; ++str)
			{	// replace control chars inside a stringliteral (most likly tabs)
				prn << (basics::stringcheck::iscntrl(*str)?' ':*str);
			}
			break;
		}
		case AE_NL:
		case AE_NLOPT:
		case AE_NEWLINE:
			// ignore newline terminals
			break;
		default:
			// print the token as is
			prn << parser.rt[rtpos].cToken.cLexeme;
			break;
		}
	}
	else if( parser.rt[rtpos].cChildNum==1 )
	{	// nonterminal with only one child, just go down
		print_beautified(parser, parser.rt[rtpos].cChildPos, parent);
	}
	else if( parser.rt[rtpos].cChildNum>1 )
	{	// other nonterminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case AE_LABEL:
		{	// set labels to zero scope
			int tmpscope = prn.scope;
			prn.scope=0;

			//<Label>      ::= identifier ':' <OptValue> <nl>
			//<OptValue>   ::= <Op Unary> |

			// print the name
			print_beautified(parser, parser.rt[rtpos].cChildPos, AE_LABEL);
			// print the value just attached
			// so "OnTimer: 2000\n" -> "OnTimer2000:\n"

			
			size_t i = parser.rt[rtpos].cChildPos+2;
			while( parser.rt[i].symbol.Type != 1 && parser.rt[i].cChildNum )
			{	// find the first existing terminal
				i = parser.rt[i].cChildPos;
			}
			if( parser.rt[i].symbol.Type == 1 &&
				parser.rt[i].symbol.idx == AE_STRINGLITERAL)
			{	// strip quotes from stringliterals
				const char* str = parser.rt[i].cToken.cLexeme;
				if( *str=='\"' ) ++str;
				if( *str && *str!='\"' )
				{	// string not empty
					prn << basics::upcase(*str++);
					for(; *str && *str!='\"'; ++str)
						prn << *str;
				}
			}
			else
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
			}
			prn << ":\n";

			prn.scope = tmpscope;
			break;
		}
		case AE_OPAND:
		case AE_OPBINAND:
		{
			print_opand(parser, rtpos, parent);
			break;
		}
		case AE_VARDECL:
		{	// 'var' identifier <Var Init> <nl>
			prn << "temp ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1, AE_VARDECL);
			print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
			prn << ";\n";
			break;
		}
		case AE_VARINIT:
		{	// '=' <Rand Init>
            // '=' <Call Stm>
			// or empty
			if( parser.rt[rtpos].cChildNum > 0 )
			{
				prn << " = ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+1, parent);
			}
			break;
		}
		case AE_RANDINIT:
		{	// <Op If> DecLiteral
			// <Op If>				-> handled already
			if( parser.rt[rtpos].cChildNum > 1 )
			{	// rand with two limits
				prn << "rand(";
				print_beautified(parser, parser.rt[rtpos].cChildPos+0, AE_EXPR);
				prn << ',' << ' ';
				print_beautified(parser, parser.rt[rtpos].cChildPos+1, AE_EXPR);
				prn << ')';
			}
			else
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos+0, AE_EXPR);
			}
			break;
		}
		case AE_CALLSTM:
		{	// identifier <Call List>
			
			if( !print_callstm(parser, parser.rt[rtpos].cChildPos+0, parser.rt[rtpos].cChildPos+1) )
			{
				prn << "/* unknown: ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+0, 0);
				prn << " ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+1, 0);
				prn << " */";
			}
			break;
		}
		case AE_ARRAY:
		{	// <Value> '[' <Expr> ']'
			print_varray(parser, rtpos, parent);
			break;
		}
		case AE_NPCARRAY:
		{	// 'npcv' <stringliteral> '[' <Expr> ']'
			print_npcvarray(parser, rtpos, parent);
			break;
		}
		case AE_IFSTM:
		{	// 'if'  <Expr>  <nl> <Stm List> <Elseif Stm> 'endif' <nl>
			prn << "if( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1, AE_EXPR);
			prn << " )\n{\n";
			++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+3, parent);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			print_beautified(parser, parser.rt[rtpos].cChildPos+4, parent);
			break;
		}
		case AE_ELSEIFSTM:
		{	// 'elseif'  <Expr>  <nl> <Stm List> <ElseIfStm>
			if( parser.rt[rtpos].cChildNum>1 )
			{
				prn << "else if( ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+1, AE_EXPR);
				prn << " )\n{\n";
				++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+3, parent);
				--prn.scope;
				if(!prn.newline) prn << '\n';
				prn << "}\n";
				print_beautified(parser, parser.rt[rtpos].cChildPos+4, parent);
			}
			// else its a <elsstm> and already handled
			// or empty and not necessary to process
			break;
		}
		case AE_ELSESTM:
		{	// 'else' <nl> <Stm List>
			if( parser.rt[rtpos].cChildNum )
			{
				prn << "else\n{\n";
				++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
				--prn.scope;
				if(!prn.newline) prn << '\n';
				prn << "}\n";
			}
			break;
		}
		case AE_WHILESTM:
		{	
			prn << "while( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+1, AE_EXPR);
			prn << " )\n{\n";
			++prn.scope;
			print_beautified(parser, parser.rt[rtpos].cChildPos+3, parent);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case AE_FORSTM:
		{
			size_t i = parser.rt[rtpos].cChildPos;
			bool forward = atoi(parser.rt[i+3].cToken.cLexeme) <= atoi(parser.rt[i+5].cToken.cLexeme);
			prn << "for(";
			print_beautified(parser, i+1, AE_EXPR);
			prn << "=";
			print_beautified(parser, i+3, AE_EXPR);
			prn << "; ";
			print_beautified(parser, i+1, AE_EXPR);
			prn << (forward?"<=":">=");
			print_beautified(parser, i+5, AE_EXPR);
			prn << "; ";
			prn << (forward?"++":"--");
			print_beautified(parser, i+1, AE_EXPR);
			prn << ")\n{\n";
			++prn.scope;
			print_beautified(parser, i+7, parent);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}			
		case AE_DOSTM:
		{
			size_t i = parser.rt[rtpos].cChildPos;
			prn << "do\n{\n";
			++prn.scope;
			print_beautified(parser, i+2, parent); // <Stm List>
			if(!prn.newline)
				prn << '\n';
			--prn.scope;
			prn << "}\n";
			prn << "while( !(";	// negate because aegis does "until" but we do "while"
			print_beautified(parser, i+4, AE_EXPR);
			prn << ") );\n";
			break;
		}
		case AE_CHOOSESTM:
		{
			size_t i = parser.rt[rtpos].cChildPos;
			prn << "switch( ";
			print_beautified(parser, i+1, AE_EXPR); // <expr>
			prn << " )\n{\n";
			print_beautified(parser, i+3, parent); // <Case Stm>
			if(!prn.newline)
				prn << '\n';
			prn << "}\n";
			break;
		}
		case AE_CASESTM:
		{	// <Case Stm>  ::= case <Value> <nl> <Stm List> <Case Stm>
			//			   | default <nl> <Stm List> <Case Stm>
			//			   |
			if( parser.rt[rtpos].cChildNum )
			{
				size_t i = parser.rt[rtpos].cChildPos;

				if(!prn.newline) prn << '\n';
				if( AE_CASE == parser.rt[i].symbol.idx )
				{
					prn << "case ";
					++i;
					print_beautified(parser, i, AE_EXPR);
				}
				else
				{
					prn << "default";
				}
				prn << ":\n";
				++prn.scope;
				print_beautified(parser, i+2, parent);
				--prn.scope;
				print_beautified(parser, i+3, parent);
			}
			break;
		}
		case AE_CALLLIST:
		{	// <Expr> <Call List>
			// <Expr>
			print_beautified(parser, parser.rt[rtpos].cChildPos, AE_EXPR);
			if( parser.rt[rtpos].cChildNum > 1 )
			{
				prn << ", ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+1, parent);
			}
			break;
		}
		case AE_LCTRSTM:
		{	// 'break' <nl> or 'continue' <nl>
			print_beautified(parser, parser.rt[rtpos].cChildPos, AE_CALLSTM);
			prn << ';' << '\n';
			break;
		}
		case AE_EXPRSTM:
		{	// <Expr> <nl>
			// with:
			// <Expr> beeing either <Op Assign> or <Call Stm>
			// and special case when beeing a single identifier
			if( parser.rt[parser.rt[rtpos].cChildPos].symbol.idx == AE_IDENTIFIER )
			{	// function without parameters
				if( !print_callstm(parser, parser.rt[rtpos].cChildPos+0, -1) )
				{
					prn << "/* unknown: ";
					print_beautified(parser, parser.rt[rtpos].cChildPos+0, 0);
					prn << " ";
					print_beautified(parser, parser.rt[rtpos].cChildPos+1, 0);
					prn << " */";
				}
			}
			else
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos, AE_EXPR);
			}
			prn << ';';
			prn << '\n';
			break;
		}
		case AE_RETURNSTM:
		{
			// 'return' <arg> <nl>
			// 'return' <nl>
			if( parser.rt[rtpos].cChildNum > 2 )
			{	// should not exist
				prn << "return ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+1, AE_EXPR);
			}
			else
			{	// translates to end statement
				prn << "end";
			}
			prn << ";\n";
			break;
		}
		case AE_NPCOBJ:
		{	// <Obj> StringLiteral StringLiteral identifier DecLiteral DecLiteral DecLiteral DecLiteral DecLiteral <nl> <Block>
			// translates to normal npc
			size_t i = parser.rt[rtpos].cChildPos;
			
			basics::string<> sprite;
			sprite << '\"' << parser.rt[i+3].cToken.cLexeme << '\"';
			this->print_npchead( parser.rt[i+2].cToken.cLexeme,
								 parser.rt[i+1].cToken.cLexeme,
								 parser.rt[i+4].cToken.cLexeme,
								 parser.rt[i+5].cToken.cLexeme,
								 parser.rt[i+6].cToken.cLexeme,
								 sprite,
								 parser.rt[i+7].cToken.cLexeme,
								 parser.rt[i+8].cToken.cLexeme);
			// script body
			prn << "{\n";
			++prn.scope;
			prn << "end;\n";
			// aegis body, should start with a label (most likly OnClick)
			print_beautified(parser, parser.rt[rtpos].cChildPos+10, parent);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case AE_WARPOBJ:
		{	// 'warp' StringLiteral StringLiteral DecLiteral DecLiteral DecLiteral DecLiteral <nl> <Block>
			// translates to ontouch npc with warp sprite
			size_t i = parser.rt[rtpos].cChildPos;

			this->print_npchead( parser.rt[i+2].cToken.cLexeme,
								 parser.rt[i+1].cToken.cLexeme,
								 parser.rt[i+3].cToken.cLexeme,
								 parser.rt[i+4].cToken.cLexeme,
								 "0",
								 "45/*\"WARP\"*/",
								 parser.rt[i+5].cToken.cLexeme,
								 parser.rt[i+6].cToken.cLexeme);
			// script body
			prn << "{\n";
			++prn.scope;
			prn << "end;\n";
			// aegis body, should start with an ontouch label
			print_beautified(parser, parser.rt[rtpos].cChildPos+8, parent);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case AE_WARPNPCOBJ:
		{	// 'hiddenwarp' StringLiteral StringLiteral DecLiteral DecLiteral DecLiteral DecLiteral DecLiteral <nl> <Block>
			// translates to invisible npc 
			size_t i = parser.rt[rtpos].cChildPos;

			this->print_npchead( parser.rt[i+2].cToken.cLexeme,
								 parser.rt[i+1].cToken.cLexeme,
								 parser.rt[i+3].cToken.cLexeme,
								 parser.rt[i+4].cToken.cLexeme,
								 parser.rt[i+5].cToken.cLexeme,
								 "111/*\"INVISIBLE\"*/",
								 parser.rt[i+6].cToken.cLexeme,
								 parser.rt[i+7].cToken.cLexeme);
			// script body
			prn << "{\n";
			++prn.scope;
			prn << "end;\n";
			// aegis body, should start with a label
			print_beautified(parser, parser.rt[rtpos].cChildPos+9, parent);
			--prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		}
		case AE_NL:
		case AE_NLOPT:
		case AE_NEWLINE:
			// ignore newline terminals
			break;
		default:
		{
			size_t j,k;
			k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
			for(j=parser.rt[rtpos].cChildPos; j<k; ++j)
			{	// go down
				print_beautified(parser, j, parent);
			}
			break;
		}// end default case
		}// end switch
	}
	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////
bool aegisParser::process(const char*name) const
{
	bool ok = true;
	bool run = true;

	// Open input file
	if( !parser->input.open(name) )
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
		short p = parser->parse(AE_DECL);
		if (p < 0)
		{	// an error
			parser->print_expects(name);
			run = false;
			ok = false;
		}
		else if(0 == p)
		{	// finished
			run = false;
		}			
		if( ok && parser->rt[0].symbol.idx==AE_DECL && parser->rt[0].cChildNum )
		{
			basics::CStackElement *child = &(parser->rt[parser->rt[0].cChildPos]);
			if( child &&
				( child->symbol.idx == AE_NPCOBJ ||
				  child->symbol.idx == AE_WARPOBJ ||
				  child->symbol.idx == AE_WARPNPCOBJ
					
				  )
			  )
			{
				if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
				{
					fprintf(stderr, "(%li)----------------------------------------\n", (unsigned long)parser->rt.size());
					parser->print_rt_tree(0,0, false);
				}

				if( (option&OPT_TRANSFORM)==OPT_TRANSFORM )
				{
					//////////////////////////////////////////////////////////
					// tree transformation
					parsenode pnode(*parser);
					fprintf(stderr, "----------------------------------------\n");
					pnode.print_tree();
				}
				if( (option&OPT_BEAUTIFY)==OPT_BEAUTIFY && this->prn.output )
				{
					this->prn.scope=0;
					this->prn << '\n';

					//	this->prn << "not yet implemented\n";

					this->prn.print_beautified(*parser, 0, 0);
					this->prn.print_comments(*parser, -1);
				}					
				//////////////////////////////////////////////////////////
				// reinitialize parser
				parser->reinit();
//					fprintf(stderr, "............................................(%i)\n", global::getcount());
			}
		}
	}
	parser->reset();
	return ok;
}


