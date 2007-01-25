// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "aegisscriptconv.h"
#include "aegisscript.h"
#include "dblookup.h"


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


bool aegisprinter::print_npchead(const char* head,
								 const basics::string<>& name,
								 const basics::string<>& map,
								 const basics::string<>& xpos,
								 const basics::string<>& ypos,
								 const char* dir,
								 const char* sprite,
								 int touchx,
								 int touchy)
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


	prn << (head?head:"npc")<< ' ';
	prn.print_id(tmpname);
	prn << " (name=\"";
	prn.print_name(tmpname);
	prn << "\"";

	prn <<  ", map=" << map <<
			", xpos="  << atoi(xpos) << 
			", ypos="  << atoi(ypos) << 
			", dir=" << dirnames[(dir?atoi(dir)&0x07:0)] << 
			", sprite=" << sprite;

	if( touchx>0 ||
		touchy>0 )
	{
		prn << ", touchup={"
			<< (touchx>0?touchx:0) << ", "
			<< (touchy>0?touchy:0) << '}';
	}
	prn << ')';
	prn << '\n';
	return true;
}

bool aegisprinter::print_equipid(basics::CParser_CommentStore& parser, int nodepos)
{
	aegisprinter& prn = *this;

	basics::CStackElement* node;
	for(node = &parser.rt[ nodepos ]; node->symbol.Type != 1; node= &parser.rt[ node->cChildPos ]) {}
	
	const uint pos = atoi(node->cToken.cLexeme)-1;
	if( node->symbol.idx == AE_DECLITERAL )
	{
		const char * equippos[] = 
		{
			"\"Head\"",
			"\"Body\"",
			"\"Left Hand\"",
			"\"Right Hand\"",
			"\"Robe\"",
			"\"Shoes\"",
			"\"Accessory 1\"",
			"\"Accessory 2\"",
			"\"Head 2\"",
			"\"Head 3\"",
			"\"Arrows\""
		};
		prn << ' ' << pos
			<< ' ' << '/' << '*' << ' '
			<< ((pos < sizeof(equippos)/sizeof(*equippos))?equippos[pos]:"invalid")
			<< ' ' << '*' << '/';
	}	
	else
	{
		prn << node->cToken.cLexeme;
	}
	return true;
}

bool aegisprinter::print_callstm(basics::CParser_CommentStore& parser, int namepos, int parapos)
{
	aegisprinter& prn = *this;
	basics::CStackElement* funcname;

	// find the function name
	for(funcname = &parser.rt[ namepos ]; funcname->symbol.Type != 1; funcname = &parser.rt[ funcname->cChildPos ]) {}
	if( funcname->symbol.idx == AE_IDENTIFIER )
	{
		// generate flat parameter list
		basics::vector<size_t> parameter;
		if(parapos>0) // valid parameter node
		{
			basics::CStackElement *paranode = &parser.rt[parapos];
			for(; paranode->cChildNum==1; paranode = &parser.rt[ paranode->cChildPos ]) {}
			if( paranode->symbol.idx==AE_CALLLIST )
			{	// a list

				size_t i = paranode->cChildPos;
				size_t k = i+paranode->cChildNum;
				for(;i<k;)
				{
					if( parser.rt[i].symbol.idx == AE_CALLLIST )
					{	// go down
						k = parser.rt[i].cChildPos+parser.rt[i].cChildNum;
						i = parser.rt[i].cChildPos;
					}
					else
					{	// remember
						size_t z=i;
						for(; parser.rt[z].cChildNum==1; z = parser.rt[z].cChildPos) {}
						parameter.push_back(z);
						++i;
					}
				}
			}
			else if(paranode->symbol.Type==1 || paranode->cChildNum)
			{	// a single value
				parameter.push(parapos);
			}
			// empty node otherwise
		}
		

/*
declare v							n			ENUM++
-> <array>
declare lv							s			ENUM++
-> <array>
declare npcv						sn			ENUM++
-> <npcarray>
declare SetLocalVar					sn			ENUM++
-> npcvar assignment
declare	strlocalvar					ns			ENUM++
-> npcvar declaration
declare PcName						.			ENUM++
-> player::name, or strcharinfo(0)
declare OnInit:						.			ENUM++	blockcheck
-> <Label>
declare OnClick:					.			ENUM++	blockcheck
-> <Label>
declare OnTouch:					.			ENUM++	blockcheck
-> <Label>
declare OnMyMobDead:				.			ENUM++	blockcheck
-> <Label>
declare OnTimer:					n			ENUM++	blockcheck
-> <Label>
declare OnCommand:					s			ENUM++	blockcheck
-> <Label>
declare OnStartArena:				.			ENUM++	blockcheck
-> <Label>
declare rand						nn			ENUM++
-> rand
declare lot							nn			ENUM++
-> rand ?
declare GetPCCount					s			ENUM++
-> getmapusers
declare return						.			ENUM++
-> end
declare guide						ssnnnn		ENUM++	blockcheck
-> <npc/mob/warpobj>
declare npc							ssnnnnnn	ENUM++	blockcheck
-> <npc/mob/warpobj>
declare mob							nsnnnn		ENUM++	blockcheck
-> <npc/mob/warpobj>
declare warp						ssnnnn		ENUM++	blockcheck
-> <npc/mob/warpobj>
declare trader						ssnnnn		ENUM++	blockcheck
-> <npc/mob/warpobj>
declare arenaguide					ssnnnnnn	ENUM++	blockcheck
-> <npc/mob/warpobj>
declare hiddenwarp					ssnnnn		ENUM++	blockcheck
-> <npc/mob/warpobj>
declare dialog						s			ENUM++
-> dialog
declare wait						.			ENUM++
-> next
declare close						.			ENUM++
-> close
declare putmob						snnnnnnnnn	ENUM++
-> monster
declare moveto						snn			ENUM++
-> warp
declare getitem						nn			ENUM++
-> getitem
declare dropitem					nn			ENUM++
-> delitem
declare menu						s?			ENUM++
-> select
declare sellitem					n			ENUM++
-> sellitem
declare error						.			ENUM++
-> tmpval<0
declare dlgwrite					nn			ENUM++
-> input
declare input						.			ENUM++
-> tmpval/tmpstr
declare	callmonster					snsnn		ENUM++
-> monster
declare	cmdothernpc					ss			ENUM++
-> cmdothernpc
declare	makewaitingroom				sn			ENUM++
-> waitingroom
declare	resetmymob					.			ENUM++
-> killmonster
declare	enablenpc					s			ENUM++
-> enablenpc
declare	disablenpc					s			ENUM++
-> disablenpc
declare IncLocalVar					sn			ENUM++
-> ++
declare DecLocalVar					sn			ENUM++
-> --
declare compass						nnnns		ENUM++
-> viewpoint
declare	GetEquipName				n			ENUM++
-> getequipname
declare GetEquipItemIdx				n			ENUM++
-> getequipid
declare GetEquipIsIdentify			n			ENUM++
-> getequipisidentify
declare ResetStat					.			ENUM++
-> resetstatus
declare ResetSkill					.			ENUM++
-> resetskill
declare	showimage					sn			ENUM++
-> cutin

  
declare trace						s			ENUM++
-> ?
declare GetEquipRefineryCnt			n			ENUM++
-> ?
declare GetEquipPercentRefinery		n			ENUM++
-> ?
declare GetEquipRefineryCost		n			ENUM++
-> ?
declare GetEquipIsSuccessRefinery	n			ENUM++
-> ?
declare	GetEquipWeaponLv			n			ENUM++
-> ?
declare GetEquipIsEnableRef			n			ENUM++
-> ?
declare LastNpcName					s			ENUM++
-> ?
declare effect						snnnnnn		ENUM++	blockcheck
-> ?
declare say							s			ENUM++
-> ?
declare getgold						n			ENUM++
-> ?
declare dropgold					n			ENUM++
-> ?
declare setitem						nn			ENUM++
-> ?
declare inc							nn			ENUM++
-> ?
declare dec							nn			ENUM++
-> ?
declare hpfullheal					.			ENUM++
-> ?
declare spfullheal					.			ENUM++
-> ?
declare hpheal						n			ENUM++
-> ?
declare spheal						n			ENUM++
-> ?
declare poisonheal					.			ENUM++
-> ?
declare stoneheal					.			ENUM++
-> ?
declare curseheal					.			ENUM++
-> ?
declare freezingheal				.			ENUM++
-> ?
declare silenceheal					.			ENUM++
-> ?
declare confusionheal				. 			ENUM++
-> ?
declare buyitem						n			ENUM++
-> ?
declare jobchange					n			ENUM++
-> ?
declare exchange					nnnn		ENUM++
-> ?
declare checkpoint					snn			ENUM++
-> ?
declare store						.			ENUM++
-> ?
declare cart						n			ENUM++
-> ?
declare	nude						.			ENUM++
-> ?
declare	changepallete				nn			ENUM++
-> ?
declare	addskill					nnnn		ENUM++
-> ?
declare InitTimer					.			ENUM++
-> ?
declare setarenaeventsize			n			ENUM++
-> ?
declare	enablearena					.			ENUM++
-> ?
declare	disablearena				.			ENUM++
-> ?
declare	warpwaitingpctoarena		snn			ENUM++
-> ?
declare	warpallpcinthemap			snn			ENUM++
-> ?
declare	broadcastinmap				s			ENUM++
-> ?
declare	stoptimer					.			ENUM++
-> ?
declare	addnpctimer					sn			ENUM++
-> ?
declare	subnpctimer					sn			ENUM++
-> ?
declare	callnpc						snsnnn		ENUM++
-> ?
declare SetFeeZeny					n			ENUM++
-> ?
declare SetFeeItem					nn			ENUM++
-> ?
declare SetReqLevel					nn			ENUM++
-> ?
declare SetTexJob					n			ENUM++
-> ?
declare DisableItemMove				.			ENUM++
-> ?
declare EnableItemMove				.			ENUM++
-> ?
declare SuccessRefItem				n			ENUM++
-> ?
declare FailedRefItem				n			ENUM++
-> ?
declare SetEffectStatus				n			ENUM++
-> ?
*/

		//////////////////////////////////////////////////////////////////
		// function remapping
		if( funcname->cToken.cLexeme == "dialog" )
		{	// open message window and displaytext
			// new command can have arbitrary arguments, so skip any test
			prn << "dialog" << '(';
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
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
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "dlgwritestr" )
		{	// string input box
			// aegis default variable is "inputstr"
			// we'll need to make it explicit
			if( !this->cHasTmpstr ) this->cHasTmpstr=true, prn << "string ";
			prn << "temp::tmpstr = inputstring()";
		}
		else if( funcname->cToken.cLexeme == "dlgwrite" )
		{	// number input box with range check
			// aegis default variable is "input", 
			// additionally the variable "error" gets set to 1 in case of error
			// we'll need to make it explicit
			if( !this->cHasTmpval ) prn << "int ";
			prn <<  "temp::tmpval = inputnumber();\n";
			if( parameter.size()>0 )
			{
				size_t cnt=0;

				if( !this->cHasTmpval ) prn << "int ";
				prn << "temp::tmperr = ";

				basics::CStackElement* node = &parser.rt[parameter[0]];

				if( node->symbol.idx==AE_DECLITERAL )
				{	// a number
					int num = atoi(node->cToken.cLexeme);
					if( num >1 )
					{
						prn << "( temp::tmpval>0 && temp::tmpval<" << num << ' ' << ')';
						++cnt;
					}
					// else don't need a lower bound check
				}
				else
				{	// type of first parameter unknown, might be an error
					prn << "temp::tmpval < ";
					print_beautified(parser, parameter[0], AE_EXPR);
					++cnt;
				}
				if( parameter.size()>1 )
				{
					if(cnt) prn << " || ";
					prn <<  "temp::tmpval > ";
					print_beautified(parser, parameter[1], AE_EXPR);
				}
			}
			this->cHasTmpval=true;
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
				if( node->symbol.Type == 1 )
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
				if( node->symbol.Type == 1 )
				{
					prn << "global::";
					this->print_idstring(node->cToken.cLexeme);
				}
				else
					prn << '0';
			}
		}
		else if( funcname->cToken.cLexeme == "strlocalvar" )
		{	// this is allocating a local variable
			if( parameter.size()<2 )
			{
				fprintf(stderr, "strlocalvar not enough parameter, line %i, ignoring\n",
					(int)parser.rt[parameter[0]].cToken.line);
			}
			else
			{
				prn << "var npc::";
				basics::CStackElement* node=&parser.rt[ parameter[1] ];
				for(; node->cChildNum==1; node = &parser.rt[ node->cChildPos ]) {}
				this->print_idstring(node->cToken.cLexeme);
			}
		}
		else if( funcname->cToken.cLexeme == "SetLocalVar" ||
				 funcname->cToken.cLexeme == "SetLocalStr" )
		{	// as names say
			// translates to assignment, 2 parameters
			if( parameter.size()>0 )
			{
				basics::CStackElement* node=&parser.rt[ parameter[0] ];
				for(; node->cChildNum==1; node = &parser.rt[ node->cChildPos ]) {}
				if( node->symbol.Type == 1 )
				{
					prn << "npc::";
					this->print_idstring(node->cToken.cLexeme);
					prn << ' ' << '=' << ' ';
					if( parameter.size()>1 )
						print_beautified(parser, parameter[1], AE_EXPR);
					else
						prn << '0';
				}
			}
		}
		else if( funcname->cToken.cLexeme == "IncLocalVar" ||
				 funcname->cToken.cLexeme == "DecLocalVar" )
		{	// as names say
			// translates to assignment, 2 parameters
			if( parameter.size()>0 )
			{
				basics::CStackElement* node=&parser.rt[ parameter[0] ];
				for(; node->cChildNum==1; node = &parser.rt[ node->cChildPos ]) {}
				if( node->symbol.Type == 1 )
				{
					prn << ((funcname->cToken.cLexeme == "IncLocalVar")?"++":"--");
					prn << "npc::";
					this->print_idstring(node->cToken.cLexeme);
				}
			}
		}

		else if( funcname->cToken.cLexeme == "dropitem" )
		{	// removes items
			// translates to delitem, 2 parameters
			prn << "delitem" << '(';

			if(parameter.size() ==2 )
			{
				const itemdb_entry* item = itemdb_entry::lookup( parser.rt[parameter[0]].cToken.cLexeme );
				if( item )
				{
					prn << '\"' << item->Name1 << '\"';
				}
				else
				{
					prn << '\"' << parser.rt[parameter[0]].cToken.cLexeme << '\"';
					fprintf(stderr, "item id '%s' not found, line %i\n",
						(const char*)parser.rt[parameter[0]].cToken.cLexeme,
						(int)parser.rt[parameter[0]].cToken.line);
				}
				prn << ',' << ' ';
				print_beautified(parser, parameter[1], AE_EXPR);
			}
			else if( parapos>0 )
				print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "getitem" )
		{	// adds items
			// translates to getitem, 2 parameters
			prn << "getitem" << '(';
			if(parameter.size() ==2 )
			{
				const itemdb_entry* item = itemdb_entry::lookup( parser.rt[parameter[0]].cToken.cLexeme );
				if( item )
				{
					prn << '\"' << item->Name1 << '\"';
				}
				else
				{
					prn << '\"' << parser.rt[parameter[0]].cToken.cLexeme << '\"';
					fprintf(stderr, "item id '%s' not found, line %i\n",
						(const char*)parser.rt[parameter[0]].cToken.cLexeme,
						(int)parser.rt[parameter[0]].cToken.line);
				}
				prn << ',' << ' ';
				print_beautified(parser, parameter[1], AE_EXPR);
			}
			else if( parapos>0 )
				print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "sellitem" )
		{	// add item to a shop, only valid within oninit of a shop npc
			// 1 parameters, must be identifier
			if( parapos>0 ) 
			{
				prn << "sellitem" << '(';
				const itemdb_entry* item = itemdb_entry::lookup( parser.rt[parameter[0]].cToken.cLexeme );
				if( item )
				{
					prn << '\"' << item->Name1 << '\"';
				}
				else
				{
					prn << '\"' << parser.rt[parameter[0]].cToken.cLexeme << '\"';
					fprintf(stderr, "item id '%s' not found, line %i\n",
						(const char*)parser.rt[parameter[0]].cToken.cLexeme,
						(int)parser.rt[parameter[0]].cToken.line);
				}
				prn << ')';
			}
		}
		else if( funcname->cToken.cLexeme == "GetEquipName" )
		{	// equiment name
			if(parameter.size()>0 )
			{
				prn << "getequipname(";
				print_equipid(parser, parameter[0]);
				prn << ')';
			}
		}
		else if( funcname->cToken.cLexeme == "GetEquipItemIdx" )
		{	// equiment name
			if(parameter.size()>0 )
			{
				prn << "getequipid(";
				print_equipid(parser, parameter[0]);
				prn << ')';
			}
		}
		else if( funcname->cToken.cLexeme == "GetEquipIsIdentify" )
		{	// equiment name
			if(parameter.size()>0 )
			{
				prn << "getequipisidentify(";
				print_equipid(parser, parameter[0]);
				prn << ')';
			}
		}
		else if( funcname->cToken.cLexeme == "CheckMaxCount" && parameter.size()==2 )
		{	// possibly tests the itemcount with the second number
			// translates to countitem with compare, 2 parameters
			
			prn << '(';
			print_beautified(parser, parameter[1], AE_EXPR); // second has the number
			prn << ' ' << '<' << ' '
				<< "countitem(";

			const itemdb_entry *item = itemdb_entry::lookup(parser.rt[parameter[0]].cToken.cLexeme);
			if( item )
			{
				prn << '\"' << item->Name1 << '\"';
			}
			else
			{
				print_beautified(parser, parameter[0], AE_EXPR);
			}
			prn << ')' << ')';
		}
		else if( funcname->cToken.cLexeme == "GetPCCount" )
		{	// get number pc in map
			// translates to getmapusers, 1 parameter
			
			prn << "getmapusers(";
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
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
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "cmdothernpc" )
		{	// cmdothernpc
			// translates to cmdothernpc, 2 parameters

			prn << "cmdothernpc" << '(';
////////
// add parameter test
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "moveto" )
		{	// moveto
			// translates to warp, 3 parameters

			prn << "warp" << '(';
////////
// add parameter test
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "makewaitingroom" )
		{	// makewaitingroom
			// translates to waitingroom, 2 parameters
			prn << "waitingroom" << '(';
////////
// add parameter test
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "warpwaitingpctoarena" )
		{	// warpwaitingpctoarena
			// translates to warpwaitingpc, 3 parameters
			prn << "warpwaitingpc" << '(';
////////
// add parameter test
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "compass" )
		{	// warpwaitingpctoarena
			// translates to warpwaitingpc, 3 parameters
			
			if( parameter.size()!=5 )
			{	// incorrect parameters
				fprintf(stderr, "incorrect parameter for compass, line %i",
					(int)funcname->cToken.line);
				prn << "// compass ";
				if(parapos>0)
					print_beautified(parser, parapos, AE_EXPR);
			}
			else
			{
				prn << "viewpoint" << '(';
				print_beautified(parser, parameter[0], AE_EXPR);
				prn << ',' << ' ';
				print_beautified(parser, parameter[1], AE_EXPR);
				prn << ',' << ' ';
				print_beautified(parser, parameter[2], AE_EXPR);
				prn << ',' << ' ';
				print_beautified(parser, parameter[3], AE_EXPR);
				prn << ',' << ' ';
				prn.print_without_quotes(parser.rt[parameter[4]].cToken.cLexeme);
				prn << ')';
			}
		}
		else if( funcname->cToken.cLexeme == "callmonster" )
		{	// callmonster creates npc bound monsters
			// ->
			// use old ea functions as other option (with slight enhancement)
			// aegis:	callmonster <map> <id> <name> <x> <y>
			// ea:		monster		<map>,<x>,<y>,<name>,<id>,<cnt>,<event>
			// default aegis event name is the "OnMyMobDead"

			if( parameter.size()!=5 )
			{	// incorrect parameters
				fprintf(stderr, "incorrect parameter for callmonster, line %i",
					(int)funcname->cToken.line);
				prn << "// callmonster ";
				if(parapos>0)
					print_beautified(parser, parapos, AE_EXPR);
			}
			else
			{

				prn << "npc::mymobcount += monster(";
				print_beautified(parser, parameter[0], AE_EXPR);
				prn << ',' << ' ';
				print_beautified(parser, parameter[3], AE_EXPR);
				prn << ',' << ' ';
				print_beautified(parser, parameter[4], AE_EXPR);
				prn << ',' << ' ';
				print_beautified(parser, parameter[2], AE_EXPR);
				prn << ',' << ' ';
				
				const mobdb_entry *mob = mobdb_entry::lookup( parser.rt[parameter[1]].cToken.cLexeme );
				if(mob)
					prn << '\"' << mob->Name1 << '\"';
				else
				{
					prn << '\"';
					print_beautified(parser, parameter[1], AE_LABEL);
					prn << '\"';
				}
				prn << ',' << ' ' << '1' << ',' << ' '
					<< '\"' << this->cEAName << "::" << "OnMyMobDead" << '\"' << ')';
			}
		}
		else if( funcname->cToken.cLexeme == "resetmymob" )
		{	// resetmymob removes called npc monsters
			// ->
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
			prn << "enablenpc(";
////////
// add parameter test
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "disablenpc" )
		{	// disablenpc
			// ->
			// disablenpc, 1 parameter
			prn << "disablenpc(";
////////
// add parameter test
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else if( funcname->cToken.cLexeme == "ResetStat" )
		{
			prn << "resetstatus()";
		}
		else if( funcname->cToken.cLexeme == "ResetSkill" )
		{
			prn << "resetskill()";
		}
		else if( funcname->cToken.cLexeme == "showimage" )
		{
			prn << "cutin(";
			if( parameter.size()!=2 )
			{	// incorrect parameters
				fprintf(stderr, "incorrect parameter for showimage, line %i",
					(int)funcname->cToken.line);
			}
			print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		//////////////////////////////////////////////////////////////////
		else if( funcname->cToken.cLexeme == "trace" ||
				 funcname->cToken.cLexeme == "GetEquipRefineryCnt" ||
				 funcname->cToken.cLexeme == "GetEquipPercentRefinery" ||
				 funcname->cToken.cLexeme == "GetEquipRefineryCost" ||
				 funcname->cToken.cLexeme == "GetEquipIsSuccessRefinery" ||
				 funcname->cToken.cLexeme == "GetEquipWeaponLv" ||
				 funcname->cToken.cLexeme == "GetEquipIsEnableRef" ||
				 funcname->cToken.cLexeme == "LastNpcName" ||
				 funcname->cToken.cLexeme == "effect" ||
				 funcname->cToken.cLexeme == "say" ||
				 funcname->cToken.cLexeme == "getgold" ||
				 funcname->cToken.cLexeme == "dropgold" ||
				 funcname->cToken.cLexeme == "setitem" ||
				 funcname->cToken.cLexeme == "inc" ||
				 funcname->cToken.cLexeme == "dec" ||
				 funcname->cToken.cLexeme == "hpfullheal" ||
				 funcname->cToken.cLexeme == "spfullheal" ||
				 funcname->cToken.cLexeme == "hpheal" ||
				 funcname->cToken.cLexeme == "spheal" ||
				 funcname->cToken.cLexeme == "poisonheal" ||
				 funcname->cToken.cLexeme == "stoneheal" ||
				 funcname->cToken.cLexeme == "curseheal" ||
				 funcname->cToken.cLexeme == "freezingheal" ||
				 funcname->cToken.cLexeme == "silenceheal" ||
				 funcname->cToken.cLexeme == "confusionheal" ||
				 funcname->cToken.cLexeme == "buyitem" ||
				 funcname->cToken.cLexeme == "jobchange" ||
				 funcname->cToken.cLexeme == "exchange" ||
				 funcname->cToken.cLexeme == "checkpoint" ||
				 funcname->cToken.cLexeme == "store" ||
				 funcname->cToken.cLexeme == "cart" ||
				 funcname->cToken.cLexeme == "nude" ||
				 funcname->cToken.cLexeme == "changepallete" ||
				 funcname->cToken.cLexeme == "addskill" ||
				 funcname->cToken.cLexeme == "InitTimer" ||
				 funcname->cToken.cLexeme == "setarenaeventsize" ||
				 funcname->cToken.cLexeme == "enablearena" ||
				 funcname->cToken.cLexeme == "disablearena" ||
				 funcname->cToken.cLexeme == "warpwaitingpctoarena" ||
				 funcname->cToken.cLexeme == "warpallpcinthemap" ||
				 funcname->cToken.cLexeme == "broadcastinmap" ||
				 funcname->cToken.cLexeme == "stoptimer" ||
				 funcname->cToken.cLexeme == "addnpctimer" ||
				 funcname->cToken.cLexeme == "subnpctimer" ||
				 funcname->cToken.cLexeme == "callnpc" ||
				 funcname->cToken.cLexeme == "SetFeeZeny" ||
				 funcname->cToken.cLexeme == "SetFeeItem" ||
				 funcname->cToken.cLexeme == "SetReqLevel" ||
				 funcname->cToken.cLexeme == "SetTexJob" ||
				 funcname->cToken.cLexeme == "DisableItemMove" ||
				 funcname->cToken.cLexeme == "EnableItemMove" ||
				 funcname->cToken.cLexeme == "SuccessRefItem" ||
				 funcname->cToken.cLexeme == "FailedRefItem" ||
				 funcname->cToken.cLexeme == "SetEffectStatus")
		{	// default: just use the given function name

			prn.log(parser, namepos);

			print_beautified(parser, namepos, AE_CALLSTM);
			// function parameters if exist
			prn << '(';
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
			prn << ')';
		}
		else
		{	// as aegis grammar is ambiguous 
			// we wronly detected a call statement here
			// while it's beeing an expression starting with an identifier
			// so:
			print_beautified(parser, namepos, AE_CALLSTM);
			if( parapos>0 ) print_beautified(parser, parapos, AE_EXPR);
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

	if( parser.rt[ parser.rt[rtpos].cChildPos+0 ].cToken.cLexeme == "lv" )
	{	// local variable
		prn << "npc::";
		this->print_idstring(node->cToken.cLexeme);
	}
	else if( parser.rt[ parser.rt[rtpos].cChildPos+0 ].cToken.cLexeme == "v" )
	{	// player variable
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
			prn << "player::is_pecoon()";
		}
		else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_ISCARTON" )
		{	// access player variable
			prn << "player::is_carton()";
		}
		else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_ISPARTYMASTER" )
		{	// access player variable
			prn << "player::is_partymaster()";
		}
		else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_CLEVEL" )
		{	// access player variable
			prn << "player::baselevel()";
		}
		else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_JOBLEVEL" )
		{	// access player variable
			prn << "player::joblevel()";
		}
		else if( node->symbol.idx == AE_IDENTIFIER && node->cToken.cLexeme == "VAR_NV_BASIC" )
		{	// access player variable
			prn << "player::skillevel(\"NV_BASIC\")";
		}
	// the others should be intentory access but double check with a loaded itemdb later
		else
		{
			const itemdb_entry *item = itemdb_entry::lookup(node->cToken.cLexeme);
			if( item )
			{
				prn << "countitem(\"" << item->Name1 << "\")";
			}
			else
			{	// some char variable (possibly)

				fprintf(stderr, "item '%s' not found, assuming player variable, line %i\n",
					(const char*)node->cToken.cLexeme,
					(int)node->cToken.line);

				prn << "player::";
				this->print_idstring(node->cToken.cLexeme);
			}
	/*		
			// default
			prn.log(parser, rtpos);
			prn << "variable";
			prn << '[';
			print_beautified(parser, parser.rt[rtpos].cChildPos+2, AE_ARRAY);
			prn << ']';
	*/
		}
	}
	else
	{	// some function with brackets
		print_callstm(parser, parser.rt[rtpos].cChildPos+0, parser.rt[rtpos].cChildPos+2);
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
		prn << "temp::tmperr";
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
			prn << "var ";
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
		case AE_LOTARRAY:
		{	// 'lot' '[' DecLiteral DecLiteral ']'
			// assuming a randomizer
			const size_t i = parser.rt[rtpos].cChildPos;
			prn << parser.rt[i+2].cToken.cLexeme << ' ' << '=' << '='
				<< "rand(" << parser.rt[i+2].cToken.cLexeme << ',' << ' '
				<< parser.rt[i+3].cToken.cLexeme << ')';
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
			this->cHasDefault=false;
			print_beautified(parser, i+3, parent); // <Case Stm>
			if(!prn.newline)
				prn << '\n';
			if( !this->cHasDefault )
			{	// always add a default case
				prn << "default:\nend;\n";
			}
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
					if( this->cHasDefault )
					{
						fprintf(stderr, "multiple default cases, line %i",
							(int)parser.rt[i].cToken.line);
					}
					this->cHasDefault=true;
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
			const npcdb_entry *npc = npcdb_entry::lookup( parser.rt[i+3].cToken.cLexeme );
			if(npc)
				sprite << '\"' << npc->Name1 << '\"';
			else
				sprite << '\"' << parser.rt[i+3].cToken.cLexeme << '\"'
						<< "/*unknown sprite id*/";

			const bool shop = parser.rt[i+0].cToken.cLexeme=="trader";

			this->print_npchead( shop?"shop":"npc",
								 parser.rt[i+2].cToken.cLexeme,
								 parser.rt[i+1].cToken.cLexeme,
								 parser.rt[i+4].cToken.cLexeme,
								 parser.rt[i+5].cToken.cLexeme,
								 parser.rt[i+6].cToken.cLexeme,
								 sprite,
								 (shop)?0:atoi(parser.rt[i+7].cToken.cLexeme),
								 (shop)?0:atoi(parser.rt[i+8].cToken.cLexeme) );
			// script body
			prn << "{\n";
			++prn.scope;
			if(shop) prn << "openshop();\n";
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

			basics::string<> sprite;
			const npcdb_entry *npc = npcdb_entry::lookup( "45" );
			if(npc)
				sprite << '\"' << npc->Name1 << '\"';
			else
				sprite << "45/*\"WARPPOINT\"*/";

			this->print_npchead( "npc",
								 parser.rt[i+2].cToken.cLexeme,
								 parser.rt[i+1].cToken.cLexeme,
								 parser.rt[i+3].cToken.cLexeme,
								 parser.rt[i+4].cToken.cLexeme,
								 "0",
								 sprite,
								 atoi(parser.rt[i+5].cToken.cLexeme),
								 atoi(parser.rt[i+6].cToken.cLexeme) );
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

			basics::string<> sprite;
			const npcdb_entry *npc = npcdb_entry::lookup( "111" );
			if(npc)
				sprite << '\"' << npc->Name1 << '\"';
			else
				sprite << "111/*\"HIDDENNPC\"*/";

			this->print_npchead( "npc",
								 parser.rt[i+2].cToken.cLexeme,
								 parser.rt[i+1].cToken.cLexeme,
								 parser.rt[i+3].cToken.cLexeme,
								 parser.rt[i+4].cToken.cLexeme,
								 parser.rt[i+5].cToken.cLexeme,
								 sprite,
								 atoi(parser.rt[i+6].cToken.cLexeme),
								 atoi(parser.rt[i+7].cToken.cLexeme) );
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
		case AE_MOBOBJ:
		{	// 'putmob' StringLiteral DecLiteral DecLiteral DecLiteral DecLiteral DecLiteral identifier DecLiteral DecLiteral DecLiteral <nl>

			// <> <map> <x> <y> <x> <y> <cnt> <id> <tick1> <tick2> <?>
			const size_t i = parser.rt[rtpos].cChildPos;
			prn << "monster (sprite=";
			const mobdb_entry *mob = mobdb_entry::lookup( parser.rt[i+7].cToken.cLexeme );
			if(mob)
				prn << '\"' << mob->Name1 << '\"';
			else
			{
				prn << '\"' << parser.rt[i+7].cToken.cLexeme << '\"';
				fprintf(stderr, "mob id '%s' not found, line %i\n",
					(const char*)parser.rt[i+7].cToken.cLexeme,
					(int)parser.rt[i+7].cToken.line);
			}
			prn << ", map=" << parser.rt[i+1].cToken.cLexeme;

			const uint x1 = atoi( parser.rt[i+2].cToken.cLexeme );
			const uint y1 = atoi( parser.rt[i+3].cToken.cLexeme );
			const uint x2 = atoi( parser.rt[i+4].cToken.cLexeme );
			const uint y2 = atoi( parser.rt[i+5].cToken.cLexeme );
			if( x1||x2||y1||y2 )
				prn << ", area={" << x1 << ", " << y1 << ", " << x2 << ", " << y2 << '}';

			prn << ", count=" << parser.rt[i+6].cToken.cLexeme;

			const int t1 = atoi( parser.rt[i+8].cToken.cLexeme );
			const int t2 = atoi( parser.rt[i+9].cToken.cLexeme );
			if( t1>0 || t2>0 )
			{
				prn << ", respawn=";
				if(t1>0 && t2>0)
					prn << "{"<< t1 << ", " << t2 << '}';
				else 
					prn << (t1>0?t1:t2);
			}
			prn << ')' << ';';
			break;
		}
		case AE_MOBDECL:
		{
			break;
		}
		case AE_EFFECTOBJ:
		{
			break;
		}
		case AE_DEFINITION:
		{	// 'define' identifier DecLiteral <nl>
			const size_t i = parser.rt[rtpos].cChildPos;
			const int val = atoi(parser.rt[i+2].cToken.cLexeme);
			if( this->defines.exists(parser.rt[i+1].cToken.cLexeme) )
			{
				if( this->defines[parser.rt[i+1].cToken.cLexeme] != val )
				{
					fprintf(stderr, "double definition %s, different values ('%i'!='%i')\n",
						(const char*)parser.rt[i+1].cToken.cLexeme,
						val, this->defines[parser.rt[i+1].cToken.cLexeme]);
				}
			}
			else
			{
				this->defines[parser.rt[i+1].cToken.cLexeme] = val;
			}
			break;
		}
		case AE_DECLARATION:
		{	// declare <nl>
			// just ignore
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

	if( option&OPT_OUTPUT )
	{
		basics::string<> str;
		const char*ip = strrchr(name,'.');
		if(ip)
			str.assign(name,ip-name);
		else
			str.assign(name);

		str << ".ea";

		this->prn.output = fopen(str, "wb");
		if(NULL==this->prn.output)
		{
			fprintf(stderr, "cannot open output file %s\noutputting to stdout\n", str.c_str());
			this->prn.output = stdout;
		}
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
				  child->symbol.idx == AE_WARPNPCOBJ ||
				  child->symbol.idx == AE_MOBOBJ ||
				  child->symbol.idx == AE_MOBDECL ||
				  child->symbol.idx == AE_EFFECTOBJ ||
				  child->symbol.idx == AE_DEFINITION ||
				  child->symbol.idx == AE_DECLARATION
				  )
			  )
			{
				if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
				{
					fprintf(stderr, "(%lu)----------------------------------------\n", (unsigned long)parser->rt.size());
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
			}
		}
	}
	parser->reset();
	if( this->prn.output != stdout )
	{
		fclose(this->prn.output);
		this->prn.output = stdout;
	}
	return ok;
}


