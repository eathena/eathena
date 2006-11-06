// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "oldeascriptconv.h"
#include "oldeascript.h"


const unsigned char oldeaengine[] = 
{
#include "oldeascript.engine.txt"
};

bool oldeaparserstorage::getEngine(const unsigned char*& buf, unsigned long& sz)
{
	buf=oldeaengine; sz =sizeof(oldeaengine);
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
// helpers
///////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////
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
	basics::string<>	Job;			// optional		when adding 0==all allowed
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

	bool operator==(const itemdb_entry& me) const	{ return this->ID == me.ID; }
	bool operator!=(const itemdb_entry& me) const	{ return this->ID != me.ID; }
	bool operator< (const itemdb_entry& me) const	{ return this->ID <  me.ID; }
};

/*
different item types
each with different additional data

virtual common_item:
mandatory:
id, name, (alternative name), type (not allocated), Price, Weight
optional:
sell

virtual basic_item : inherits common_item
optional:
gender,job,upper,eLV


*Healing : inherits basic_item
mandatory:
script

*Usable : inherits basic_item
mandatory:
script

*delayed Usable : inherits basic_item
mandatory:
script

*Misc : inherits common_item

virtual equip_item : inherits basic_item
mandatory:
Loc
optional:
Slot,Refineable,view, script(s)

*Weapon : inherits equip_item
mandatory:
ATK
optional:
Range,wLV

*Armor : inherits equip_item
mandatory:
DEF

*Card : inherits basic_item
mandatory:
Loc

*Arrow : inherits basic_item
mandatory:
ATK

*Pet Egg : inherits common_item

*Pet Equipment : inherits common_item
*/

/////////////////////////////////
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

	bool operator==(const mobdb_entry& me) const	{ return this->ID == me.ID; }
	bool operator!=(const mobdb_entry& me) const	{ return this->ID != me.ID; }
	bool operator< (const mobdb_entry& me) const	{ return this->ID <  me.ID; }
};

/*
compound drops to list
optional:
mexp, mvp drop list
*/


basics::slist<mobdb_entry>	mobdb;
basics::slist<itemdb_entry>	itemdb;




///////////////////////////////////////////////////////////////////////////////////////
// code beautifier
///////////////////////////////////////////////////////////////////////////////////////

void oldeaprinter::print_newnpchead(const char*name, const char*map, int x, int y, int d, int s, int tx, int ty)
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

	oldeaprinter& prn = *this;

	prn << "(name=\"" << name << "\"";
	if(map && x>0 && y>0 && s>0)
	{
		prn <<  ", map=\"" << map << "\""
				", xpos=" << x << 
				", ypos=" << y << 
				", dir=" << dirnames[d&0x07] << 
				", sprite=" << s;
	}
	if(tx>0 || ty>0)
	{
		prn << ", touchup={"<< tx << "," << ty << "}";
	}
	prn << ')';
}


void oldeaprinter::print_oldscripthead(const char* str)
{	// split the old npc header
	// old format:
	// map,x,y,d<tab>script<tab>name<tab>sprite,[x,y,]
	oldeaprinter& prn = *this;
	char map[32], tmp[128];
	int x=0,y=0,d=0,s=0,tx=0,ty=0;
	int res = sscanf(str, "%32[^,],%d,%d,%d\tscript\t%128[^\t]\t%d,%d,%d", 
							map,&x,&y,&d,tmp,&s,&tx,&ty);
	if( res>=6 )
	{	// new format:
		// npc [id] (map=map, xpos=x, ypos=y, dir=d,name=name, sprite=sprite, touchup={x,y})
		char idname[32], name[32], *ip, *kp;
		if( (ip=strstr(tmp, "::")) )
		{	// cut off the script id
			*ip=0;
			str2id(idname, sizeof(idname), ip+2);
		}
		else
		{	// take full name as id
			str2id(idname, sizeof(idname), tmp);
		}
		str2name(name, sizeof(name), tmp);

		// cut map extension
		kp = strchr(map, '.');
		if(kp) *kp=0;

		prn << "npc " << idname << " ";
		if(res>6)
			print_newnpchead(name, map, x, y, d, s, ty, ty);
		else
			print_newnpchead(name, map, x, y, d, s);
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldminscripthead( const char* str )
{	// split the old npc header
	// old format:
	// -<tab>script<tab>name<tab>num
	oldeaprinter& prn = *this;
	char tmp[128];
	int s=0;
	int res = sscanf(str, "-\tscript\t%128[^\t]\t%d,", 
							tmp,&s);

	if( res>=1 )
	{	// new format:
		// npc [id] (name=name)
		char idname[32];
		str2id(idname, sizeof(idname), tmp);

		prn << "npc " << idname << " ";
		print_newnpchead(idname);
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldduphead( const char* str )
{
	// old format:
	// map,x,y,d<tab>duplicate(id)<tab>name<tab>sprite,[x,y,]
	oldeaprinter& prn = *this;
	char map[32], tmp1[128], tmp2[128];
	int x=0,y=0,d=0,s=0,tx=0,ty=0;
	int res = sscanf(str, "%32[^,],%d,%d,%d\tduplicate(%128[^)])\t%128[^\t]\t%d,%d,%d", 
							map,&x,&y,&d,tmp1,tmp2,&s,&tx,&ty);
	if( res>=7 )
	{	// new format:
		// npc [id] (map=map, xpos=x, ypos=y, dir=d,name=name, sprite=sprite, touchup={x,y})
		
		// cut map extension
		char *kp = strchr(map, '.');
		if(kp) *kp=0;

		char idname[32];
		str2id(idname, sizeof(idname), tmp1);
		char name[32];
		str2name(name, sizeof(name), tmp2);

		// use buildin invisible npc when com
		if(s<0) s=111;	
		prn << "npc ";
		if(res>7)
			print_newnpchead(name, map, x, y, d, s, ty, ty);
		else
			print_newnpchead(name, map, x, y, d, s);
		prn << ' ' << idname << ';';
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldshophead( const char* str )
{
	// {File Ch}+ ',' {Digit}+ ',' {Digit}+ ',' {Digit} {SP Delim}+ 'shop' {HT}+ {Head Ch}+ {HT}+      {Digit}+ ( {SP Delim}*',' {SP Delim}*{Digit}+ ':' {SP Delim}* [-]? {Digit}+ )*
	// '-'                                              {SP Delim}+ 'shop' {HT}+ {Head Ch}+ {HT}+      '-'      ( {SP Delim}*',' {SP Delim}*{Digit}+ ':' {SP Delim}* [-]? {Digit}+ )*
	oldeaprinter& prn = *this;
	const bool shopnpc = (*str!='-');
	char map[32]="", tmp[128]="";
	int x=0,y=0,d=0,s=0,n=0;
	bool ok;

	if( shopnpc )
	{	/// shop npc
		ok = 6==sscanf(str, "%32[^,],%d,%d,%d\tshop\t%128[^\t]\t%d,%n",
						map, &x,&y,&d,tmp,&s,&n);
	}
	else
	{	// virtual shop
		ok = 1==sscanf(str, "-\tshop\t%128[^\t]\t-,%n", tmp, &n);			
	}
	if(ok)
	{
		char idname[32], name[32], *ip;
		if( (ip=strstr(tmp, "::")) )
		{	// cut off the script id
			*ip=0;
			str2id(idname, sizeof(idname), ip+2);
		}
		else
		{	// take full name as id
			str2id(idname, sizeof(idname), tmp);
		}
		str2name(name, sizeof(name), tmp);
		
		
		prn << "npc " << idname << " ";
		if(shopnpc)
		{
			// cut map extension
			char *kp = strchr(map, '.');
			if(kp) *kp=0;
			print_newnpchead(name, map, x, y, d, s);
		}
		else
		{
			print_newnpchead(name);
		}
		prn << '\n';

		size_t cnt=0;
		int id, price;
		str+=n-1;	// should stand at the first comma before the item/price list

		if( *str==',' && 2==sscanf(str,",%d:%d%n", &id, &price,&n) )
		{
			//## TODO: replace item id's with item names from db
			prn << '\t' << id;
			if(price>0)
				prn << ':' << price;
			for(str+=n, ++cnt; *str==',' && 2==sscanf(str,",%d:%d%n", &id, &price,&n); str+=n, ++cnt)
			{	
				prn << ',' << id;
				if(price>0)
					prn << ':' << price;
			}
		}
		prn << ";";
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldfunctionhead( const char* str )
{
	// old format:
	// function<tab>script<tab>name<tab>
	oldeaprinter& prn = *this;
	char tmp[128];
	int res = sscanf(str, "function\tscript\t%128[^\t]\t", 
							tmp);

	if( res>=1 )
	{	// new format:
		// <type> name([parameter])
		char idname[32];
		str2id(idname, sizeof(idname), tmp);

		prn << "auto " << idname << "() // TODO: add real function parameters";
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldmonsterhead( const char* str )
{	
	// old format:
	// <map> rectangle <name> <id>,<count>,<time1>,<time2>,<event>
	oldeaprinter& prn = *this;
	char map[32], tmp[128],event[128];
	int x1,y1,x2,y2,id,cn,t1,t2;
	int res = sscanf(str, "%32[^,],%d,%d,%d,%d\tmonster\t%128[^\t]\t%d,%d,%d,%d,%128s", 
						map,&x1,&y1,&x2,&y2,tmp,&id,&cn,&t1,&t2,event);
	if(res>=10)
	{
		char name[32];
		str2name(name, sizeof(name), tmp);

		// cut map extension
		char *kp = strchr(map, '.');
		if(kp) *kp=0;

		//## TODO check if giving explicit names is necessary, remove them if identical with dbname

		prn << "monster (sprite=" << id << ", name=\"" << name << "\", map=\"" << map << "\"";
		if( x1||x2||y1||y2 )
			prn << ", area={" << x1 << ", " << y1 << ", " << x2 << ", " << y2 << '}';
		prn << ", count=" << cn;
		if( t1 || t2 )
		{
			prn << ", respawn=";
			if(t1>0 && t2>0)
				prn << "{"<< t1 << ", " << t2 << '}';
			else 
				prn << (t1>0?t1:t2);
		}
		prn << ')';

		if( basics::stringcheck::isalpha(*event) )
			prn << ' ' << event;
		prn << ';';
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldwarphead( const char* str )
{
	// old format:
	// map,x,y,d<tab>warp<tab>name<tab>tx,ty,map,x,y
	oldeaprinter& prn = *this;
	char smap[32], tmap[32], tmp[128];
	int sx,sy,tx,ty,ux,uy,d;
	int res = sscanf(str, "%32[^,],%d,%d,%d\twarp\t%128[^\t]\t%d,%d,%32[^,],%d,%d", 
							smap,&sx,&sy,&d,tmp,&ux,&uy,tmap,&tx,&ty);
	if( res==10 )
	{	
		char idname[32], *kp;
		str2id(idname, sizeof(idname), tmp);

		kp=strchr(smap, '.'); if(kp) *kp=0;
		kp=strchr(tmap, '.'); if(kp) *kp=0;

		prn << "warp " << idname << 
			" (pos={\"" << smap << "\", "<< sx <<", " << sy << '}';
		// warps have default touchup of 1x1
		if(ux>1 || uy>1)
		{	
			if(ux<1) ux =1;
			if(uy<1) uy =1;
			prn << ", touchup={" <<ux << ", " << uy << '}';
		}
		prn << ", target={\"" << tmap << "\", " << tx << ", " << ty << '}' <<
			");";
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldmapflaghead( const char* str )
{
	oldeaprinter& prn = *this;
	// <map> mapflag <id> [<id> | position]
	// {File Ch}+ {SP Delim}+ 'mapflag' {SP Delim}+ {Id Tail}+ {SP Delim}* ({Head Ch}|',')*
	char map[32], type[128], param[128];
	int res = sscanf(str, "%32[^\t]\tmapflag\t%128[^\t]\t%128s", 
							map,type,param);
	if( res>=2 )
	{
		char *kp;
		char smap[32];
		memcpy(smap, map, sizeof(smap));
		kp=strchr(map, '.'); if(kp) *kp=0;

		prn << "map " << map << " (file=\"" << smap << "\", flag=\"" << type;
		if(res>=2)
			prn  << ", " << param;
		prn << "\");";
	}
	else
	{	// error, no format change
		prn << str;
	}
	prn << '\n';
}

void oldeaprinter::print_oldmobdbhead( const char* str )
{
//	oldeaprinter& prn = *this;
	//ID,Name,JName,LV,HP,SP,EXP,JEXP,Range1,ATK1,ATK2,DEF,MDEF,STR,AGI,VIT,INT,DEX,LUK,Range2,Range3,Scale,Race,Element,Mode,Speed,ADelay,aMotion,dMotion,Drop1id,Drop1per,Drop2id,Drop2per,Drop3id,Drop3per,Drop4id,Drop4per,Drop5id,Drop5per,Drop6id,Drop6per,Drop7id,Drop7per,Drop8id,Drop8per,Drop9id,Drop9per,DropCardid,DropCardper,MEXP,ExpPer,MVP1id,MVP1per,MVP2id,MVP2per,MVP3id,MVP3per
	basics::vector< basics::string<> > strings = basics::split<char>(str, ',');

	// 57
	if( strings.size() < 57 )
		strings.resize(57);
	
	mobdb_entry me;
	me.ID			= strings[ 0];
	me.Name			= strings[ 1];
	me.JKName		= strings[ 2];
	me.IName		= strings[ 2];
	me.LV			= strings[ 3];
	me.HP			= strings[ 4];
	me.SP			= strings[ 5];
	me.BEXP			= strings[ 6];
	me.JEXP			= strings[ 7];
	me.Range1		= strings[ 8];
	me.ATK1			= strings[ 9];
	me.ATK2			= strings[10];
	me.DEF			= strings[11];
	me.MDEF			= strings[12];
	me.STR			= strings[13];
	me.AGI			= strings[14];
	me.VIT			= strings[15];
	me.INT			= strings[16];
	me.DEX			= strings[17];
	me.LUK			= strings[18];
	me.Range2		= strings[19];
	me.Range3		= strings[20];
	me.Scale		= strings[21];
	me.Race			= strings[22];
	me.Element		= strings[23];
	me.Mode			= strings[24];
	me.Speed		= strings[25];
	me.ADelay		= strings[26];
	me.aMotion		= strings[27];
	me.dMotion		= strings[28];
	me.Drop1id		= strings[29];
	me.Drop1per		= strings[30];
	me.Drop2id		= strings[31];
	me.Drop2per		= strings[32];
	me.Drop3id		= strings[33];
	me.Drop3per		= strings[34];
	me.Drop4id		= strings[35];
	me.Drop4per		= strings[36];
	me.Drop5id		= strings[37];
	me.Drop5per		= strings[38];
	me.Drop6id		= strings[39];
	me.Drop6per		= strings[40];
	me.Drop7id		= strings[41];
	me.Drop7per		= strings[42];
	me.Drop8id		= strings[43];
	me.Drop8per		= strings[44];
	me.Drop9id		= strings[45];
	me.Drop9per		= strings[46];
	me.DropCardid	= strings[47];
	me.DropCardper	= strings[48];
	me.MEXP			= strings[49];
	me.ExpPer		= strings[50];
	me.MVP1id		= strings[51];
	me.MVP1per		= strings[52];
	me.MVP2id		= strings[53];
	me.MVP2per		= strings[54];
	me.MVP3id		= strings[55];
	me.MVP3per		= strings[56];

	size_t pos;
	if( mobdb.find(me,0,pos) ) 
		fprintf(stderr, "mobdb entry [%s] already exists", (const char*)me.ID);

	mobdb.push(me);
}

void oldeaprinter::print_oldmobdbheadea( const char* str )
{
//	oldeaprinter& prn = *this;
	// ID,Sprite_Name,kROName,iROName,LV,HP,SP,EXP,JEXP,Range1,ATK1,ATK2,DEF,MDEF,STR,AGI,VIT,INT,DEX,LUK,Range2,Range3,Scale,Race,Element,Mode,Speed,ADelay,aMotion,dMotion,MEXP,ExpPer,MVP1id,MVP1per,MVP2id,MVP2per,MVP3id,MVP3per,Drop1id,Drop1per,Drop2id,Drop2per,Drop3id,Drop3per,Drop4id,Drop4per,Drop5id,Drop5per,Drop6id,Drop6per,Drop7id,Drop7per,Drop8id,Drop8per,Drop9id,Drop9per,DropCardid,DropCardper
	// strip the elements
	basics::vector< basics::string<> > strings = basics::split<char>(str, ',');

	// 58
	if( strings.size() < 58 )
		strings.resize(58);
	
	mobdb_entry me;
	me.ID			= strings[ 0];
	me.Name			= strings[ 1];
	me.JKName		= strings[ 2];
	me.IName		= strings[ 3];
	me.LV			= strings[ 4];
	me.HP			= strings[ 5];
	me.SP			= strings[ 6];
	me.BEXP			= strings[ 7];
	me.JEXP			= strings[ 8];
	me.Range1		= strings[ 9];
	me.ATK1			= strings[10];
	me.ATK2			= strings[11];
	me.DEF			= strings[12];
	me.MDEF			= strings[13];
	me.STR			= strings[14];
	me.AGI			= strings[15];
	me.VIT			= strings[16];
	me.INT			= strings[17];
	me.DEX			= strings[18];
	me.LUK			= strings[19];
	me.Range2		= strings[20];
	me.Range3		= strings[21];
	me.Scale		= strings[22];
	me.Race			= strings[23];
	me.Element		= strings[24];
	me.Mode			= strings[25];
	me.Speed		= strings[26];
	me.ADelay		= strings[27];
	me.aMotion		= strings[28];
	me.dMotion		= strings[29];
	me.MEXP			= strings[30];
	me.ExpPer		= strings[31];
	me.MVP1id		= strings[32];
	me.MVP1per		= strings[33];
	me.MVP2id		= strings[34];
	me.MVP2per		= strings[35];
	me.MVP3id		= strings[36];
	me.MVP3per		= strings[37];
	me.Drop1id		= strings[38];
	me.Drop1per		= strings[39];
	me.Drop2id		= strings[40];
	me.Drop2per		= strings[41];
	me.Drop3id		= strings[42];
	me.Drop3per		= strings[43];
	me.Drop4id		= strings[44];
	me.Drop4per		= strings[45];
	me.Drop5id		= strings[46];
	me.Drop5per		= strings[47];
	me.Drop6id		= strings[48];
	me.Drop6per		= strings[49];
	me.Drop7id		= strings[50];
	me.Drop7per		= strings[51];
	me.Drop8id		= strings[52];
	me.Drop8per		= strings[53];
	me.Drop9id		= strings[54];
	me.Drop9per		= strings[55];
	me.DropCardid	= strings[56];
	me.DropCardper	= strings[57];
	size_t pos;
	if( mobdb.find(me,0,pos) ) 
		fprintf(stderr, "mobdb %s entry already exists", (const char*)me.ID);
	mobdb.push(me);
}

int oldeaprinter::print_olditemdbhead( const char* str )
{	
	oldeaprinter& prn = *this;
	//ID,Name,Name,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Gender,Loc,wLV,eLV,Refineable,View,{UseScript},{EquipScript}
	// strip the elements

	basics::vector< basics::string<> > strings = basics::split<char>(str, ',');

	// 18
	if( strings.size() < 18 )
		strings.resize(18);
	
	itemdb_entry me;
	me.ID			= strings[ 0];
	me.Name1		= strings[ 1];
	me.Name2		= strings[ 2];
	me.Type			= strings[ 3];
	me.Price		= strings[ 4];
	me.Sell			= strings[ 5];
	me.Weight		= strings[ 6];
	me.ATK			= strings[ 7];
	me.DEF			= strings[ 8];
	me.Range		= strings[ 9];
	me.Slot			= strings[10];
	me.Job			= strings[11];
	me.Upper		= "";
	me.Gender		= strings[12];
	me.Loc			= strings[13];
	me.wLV			= strings[14];
	me.eLV			= strings[15];
	me.Refineable	= strings[16];
	me.View			= strings[17];

	if( atoi(me.Sell) <=0 ) me.Sell = "";
	if( atoi(me.ATK) <=0 ) me.ATK = "";
	if( atoi(me.DEF) <=0 ) me.DEF = "";
	if( atoi(me.Range) <=0 ) me.Range = "";
	if( atoi(me.Slot) <=0 ) me.Slot = "";
	//## TODO change to m/f maybe combine with job/upper into a restriction section
	if( atoi(me.Gender) ==0 ) me.Gender = "0";
	else if( atoi(me.Gender) ==1 ) me.Gender = "1";
	else me.Gender = "";
	if( atoi(me.Loc) <=0 ) me.Loc = "";
	if( atoi(me.wLV) <=0 ) me.wLV = "";
	if( atoi(me.eLV) <=0 ) me.eLV = "";
	if( atoi(me.Refineable) <=0 ) me.Refineable = "";
	if( atoi(me.View) <=0 ) me.View = "";

	size_t pos;
	if( itemdb.find(me,0,pos) ) 
		fprintf(stderr, "itemdb %s entry already exists", (const char*)me.ID);
	itemdb.push(me);

	prn << me.ID << ','
		<< me.Name1 << ','
		<< me.Name2 << ','
		<< me.Type << ','
		<< me.Price << ','
		<< me.Sell << ','
		<< me.Weight << ','
		<< me.ATK << ','
		<< me.DEF << ','
		<< me.Range << ','
		<< me.Slot << ','
		<< me.Job << ','
		<< me.Upper << ','
		<< me.Gender << ','
		<< me.Loc << ','
		<< me.wLV << ','
		<< me.eLV << ','
		<< me.Refineable << ','
		<< me.View << ',';

	return atoi(me.Type);
}

int oldeaprinter::print_olditemdbheadea( const char* str )
{
	oldeaprinter& prn = *this;
	//ID,Name,Name,Type,Price,Sell,Weight,ATK,DEF,Range,Slot,Job,Upper,Gender,Loc,wLV,eLV,Refineable,View,{UseScript},{EquipScript},{UnEquipScript}
	// strip the elements

	basics::vector< basics::string<> > strings = basics::split<char>(str, ',');

	// 19
	if( strings.size() < 19 )
		strings.resize(19);
	
	itemdb_entry me;
	me.ID			= strings[ 0];
	me.Name1		= strings[ 1];
	me.Name2		= strings[ 2];
	me.Type			= strings[ 3];
	me.Price		= strings[ 4];
	me.Sell			= strings[ 5];
	me.Weight		= strings[ 6];
	me.ATK			= strings[ 7];
	me.DEF			= strings[ 8];
	me.Range		= strings[ 9];
	me.Slot			= strings[10];
	me.Job			= strings[11];
	me.Upper		= strings[12];
	me.Gender		= strings[13];
	me.Loc			= strings[14];
	me.wLV			= strings[15];
	me.eLV			= strings[16];
	me.Refineable	= strings[17];
	me.View			= strings[18];


	if( atoi(me.Sell) <=0 ) me.Sell = "";
	if( atoi(me.ATK) <=0 ) me.ATK = "";
	if( atoi(me.DEF) <=0 ) me.DEF = "";
	if( atoi(me.Range) <=0 ) me.Range = "";
	if( atoi(me.Slot) <=0 ) me.Slot = "";
	if( atoi(me.Gender) <=0 ) me.Gender = "";
	if( atoi(me.Loc) <=0 ) me.Loc = "";
	if( atoi(me.wLV) <=0 ) me.wLV = "";
	if( atoi(me.eLV) <=0 ) me.eLV = "";
	if( atoi(me.Refineable) <=0 ) me.Refineable = "";
	if( atoi(me.View) <=0 ) me.View = "";


	size_t pos;
	if( itemdb.find(me,0,pos) ) 
		fprintf(stderr, "itemdb %s entry already exists", (const char*)me.ID);
	itemdb.push(me);

	prn << me.ID << ','
		<< me.Name1 << ','
		<< me.Name2 << ','
		<< me.Type << ','
		<< me.Price << ','
		<< me.Sell << ','
		<< me.Weight << ','
		<< me.ATK << ','
		<< me.DEF << ','
		<< me.Range << ','
		<< me.Slot << ','
		<< me.Job << ','
		<< me.Upper << ','
		<< me.Gender << ','
		<< me.Loc << ','
		<< me.wLV << ','
		<< me.eLV << ','
		<< me.Refineable << ','
		<< me.View << ',';

	return atoi(me.Type);
}



bool oldeaprinter::print_beautified(basics::CParser_CommentStore& parser, int rtpos)
{
	oldeaprinter& prn = *this;
	bool ret = true;

	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// terminals

		prn.print_comments(parser, parser.rt[rtpos].cToken.line);

		switch( parser.rt[rtpos].symbol.idx )
		{
		case PT_IDENTIFIER:
		{	// convert variable scopes
			//##	account2	->login::
			//#		account		->account::
			//$		char		->player::
			//@		global		->global::
			//.		script		->script::

			const char*str = parser.rt[rtpos].cToken.cLexeme;
			const char*epp = str+parser.rt[rtpos].cToken.cLexeme.size()-1;
			// strip 
			for( ; str<=epp && !basics::stringcheck::isalnum(*epp) && *epp!='_'; --epp) {}
			if(str[0]=='.' && str[1]=='@')
			{
				str+=2;
				if(str<=epp) prn << "temp::";
				//could also skip these as default vars are temp
			}
			else if(str[0]=='.')
			{
				++str;
				if(str<=epp) prn << "npc::";
			}
			else if(str[0]=='@')
			{
				++str;
				if(str<=epp) prn << "global::";
			}
			else if(str[0]=='$')
			{
				++str;
				if(str<=epp) prn << "player::";
			}
			else if(str[0]=='#' && str[1]=='#')
			{
				str+=2;
				if(str<=epp) prn << "login::";
			}
			else if(str[0]=='#')
			{
				++str;
				if(str<=epp) prn << "account::";
			}
			if(str<=epp)
			{	// check for reserved words
				bool reserved = ( 0==strcmp(str,"if") ||
								  0==strcmp(str,"else") ||
								  0==strcmp(str,"for") ||
								  0==strcmp(str,"while") ||
								  0==strcmp(str,"return") ||
								  0==strcmp(str,"goto") ||
								  0==strcmp(str,"string") ||
								  0==strcmp(str,"int") ||
								  0==strcmp(str,"double") ||
								  0==strcmp(str,"auto") ||
								  0==strcmp(str,"var") );
				if(reserved) prn << '_';
				for(; str<=epp; ++str)
					prn << *str;
				if(reserved) prn << '_';
			}
			break;
		}

		case PT_RBRACE:
			if(prn.scope) --prn.scope;
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
		case PT_GOTO:
			prn << "goto ";
			break;
		case PT_LPARAN:
		case PT_RPARAN:
			prn << parser.rt[rtpos].cToken.cLexeme;
			break;

		case PT_OLDSCRIPTHEAD:
			print_oldscripthead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDMINSCRIPTHEAD:
			print_oldminscripthead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDFUNCHEAD:
			print_oldfunctionhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDMONSTERHEAD:
			print_oldmonsterhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDWARPHEAD:
			print_oldwarphead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDMAPFLAGHEAD:
			print_oldmapflaghead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDDUPHEAD:
			print_oldduphead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDSHOPHEAD:
			print_oldshophead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDMOBDBHEAD:
			print_oldmobdbhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDMOBDBHEAD_EA:
			print_oldmobdbheadea( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDITEMDBHEAD:
			print_olditemdbhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case PT_OLDITEMDBHEAD_EA:
			print_olditemdbheadea( parser.rt[rtpos].cToken.cLexeme );
			break;

		default:
			// print the token as is
			prn << parser.rt[rtpos].cToken.cLexeme;
			break;
		}
	}
	else if( parser.rt[rtpos].cChildNum==1 )
	{	// nonterminal with only one child, just go down
		print_beautified(parser, parser.rt[rtpos].cChildPos);
	}
	else if( parser.rt[rtpos].cChildNum>1 )
	{	// other nonterminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case PT_LABELSTM:
		{	// set labels to zero scope
			int tmpscope = prn.scope;
			prn.scope=0;
			prn << parser.rt[parser.rt[rtpos].cChildPos].cToken.cLexeme << ":\n";
			prn.scope = tmpscope;
			break;
		}
		case PT_SUBFUNCTION:
		{	// 'function' <identifier> <block>
			char idname[64];
			const char* tmp = parser.rt[parser.rt[rtpos].cChildPos+1].cToken.cLexeme;
			str2id(idname, sizeof(idname), tmp);

			prn << "function auto " << idname << "() // TODO: add real function parameters\n";
			// print the function body
			print_beautified(parser, parser.rt[rtpos].cChildPos+2);

			break;
		}
		case PT_CALLSTM:
		{	// either a real call statement with <function name> <parameter list>
			// or a function with one arument

			if( parser.rt[parser.rt[rtpos].cChildPos].cToken.cLexeme=="callfunc" )
			{	// transform to real function calls
				// "callfunc <name> <parameterlist>" -> <name>(<parameterlist>)
				basics::CStackElement& listnode = parser.rt[parser.rt[rtpos].cChildPos+1];
				if(listnode.cChildNum)
				{	
					// listnode's first child is the name, so strip any quote
					const char* kp=parser.rt[listnode.cChildPos].cToken.cLexeme;
					for(++kp; *kp && *kp!='"'; ++kp)
						prn << *kp;
					
					// argument list
					prn << "(";
					size_t j,k;
					k = listnode.cChildPos+listnode.cChildNum;
					for(j=listnode.cChildPos+2; j<k; ++j)
					{	// go down
						print_beautified(parser, j);
					}
					prn << ");\n";
				}
				else
				{	// listnode itself is the name, so strip any quote
					const char* kp=listnode.cToken.cLexeme;
					for(++kp; *kp && *kp!='"'; ++kp)
						prn << *kp;

					prn << "();\n";
				}
			}
			else if( parser.rt[parser.rt[rtpos].cChildPos].cToken.cLexeme=="set" )
			{	// transform "set" functions
				// from "set <a>, <b>" to "<a> = <b>;
				basics::CStackElement& listnode = parser.rt[parser.rt[rtpos].cChildPos+1];
				
				if( listnode.symbol.idx == PT_CALLLIST )
				{
					print_beautified(parser, listnode.cChildPos);
					prn << " = ";
					if(listnode.cChildNum>2)
						print_beautified(parser, listnode.cChildPos+2);
					else
						prn << '0';
				}
				else
				{	// fix invalid set commands with only one argument
					print_beautified(parser, parser.rt[rtpos].cChildPos+1);
					prn << " = ";
					// fix invalid set commands
					if(parser.rt[rtpos].cChildNum>3)
						print_beautified(parser, parser.rt[rtpos].cChildNum+3);
					else
						prn << '0';
				}
				prn << ";\n";
			}
			else
			{	// transform the rest to function calls
				// since the grammar is ambiguous
				// we cannod decide between "func( 1 )" and "func (1)", 
				// where "1" is a function parameter or "(1)" is a evaluation
				// so neet to test it explicitely

				// function name
				print_beautified(parser, parser.rt[rtpos].cChildPos);

				if( parser.rt[rtpos].cChildNum==3 )
				{
					const bool eval = ( PT_EVALUATION==parser.rt[parser.rt[rtpos].cChildPos+1].symbol.idx );
					if(!eval) prn << '(';
					print_beautified(parser, parser.rt[rtpos].cChildPos+1);
					if(!eval) prn << ')';
				}
				else
				{	// no call parameter
					prn << '('<<')';
				}
				prn << ';' << '\n';
			}
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
			if(!prn.newline)
				prn << '\n';

			if( PT_IF == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx ||
				PT_WHILE == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos+0);
				prn << "( ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+2);
				prn << " )\n";

				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
					++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+4);
				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+4 ].symbol.idx )
					if(prn.scope) --prn.scope;

				if( parser.rt[rtpos].cChildNum==7 )
				{
					print_beautified(parser, parser.rt[rtpos].cChildPos+5);
					if(!prn.newline)
						prn << '\n';

					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
						++prn.scope;
					print_beautified(parser, parser.rt[rtpos].cChildPos+6);
					if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+6 ].symbol.idx )
						if(prn.scope) --prn.scope;
				}
			}
			else if( PT_DO == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
			{	// do <Normal Stm> while '(' <Expr> ')' ';'
				print_beautified(parser, parser.rt[rtpos].cChildPos+0);
				prn << '\n';
				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
					++prn.scope;
				print_beautified(parser, parser.rt[rtpos].cChildPos+1);
				if( PT_BLOCK != parser.rt[ parser.rt[rtpos].cChildPos+1 ].symbol.idx )
					if(prn.scope) --prn.scope;
				if(!prn.newline)
					prn << '\n';
				print_beautified(parser, parser.rt[rtpos].cChildPos+2);
				prn << "( ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+4);
				prn << " )";
				print_beautified(parser, parser.rt[rtpos].cChildPos+6);
			}
			else if( PT_SWITCH == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
			{	// switch '(' <Expr> ')' '{' <Case Stms> '}'
				print_beautified(parser, parser.rt[rtpos].cChildPos+0);
				prn << "( ";
				print_beautified(parser, parser.rt[rtpos].cChildPos+2);
				prn << " )\n";
				prn << "{\n";
				print_beautified(parser, parser.rt[rtpos].cChildPos+5);
				if(!prn.newline)
					prn << '\n';

				prn << "}\n";
			}
			else if( PT_FOR == parser.rt[ parser.rt[rtpos].cChildPos ].symbol.idx )
			{	// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
				print_beautified(parser, parser.rt[rtpos].cChildPos+0);
				prn << "(";
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
					if(prn.scope) --prn.scope;
			}
			else
			{
				size_t j,k;
				k = parser.rt[rtpos].cChildPos+parser.rt[rtpos].cChildNum;
				j = parser.rt[rtpos].cChildPos;
				for(; j<k; ++j)
				{
					print_beautified(parser, j);
				}
			}
			break;
		}
		case PT_CASESTMS:
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
					if( PT_CASESTMS==parser.rt[j].symbol.idx )
						if(prn.scope) --prn.scope;
					print_beautified(parser, j);
				}
			}
			prn.scope = tmpscope;
			break;
		}
		case PT_RETURNSTMS:
		{
			print_beautified(parser, parser.rt[rtpos].cChildPos);
			if( parser.rt[rtpos].cChildNum==3 )
			{	// 'return' <arg> ';'
				prn << ' ';
				print_beautified(parser, parser.rt[rtpos].cChildPos+1 );
			}
			prn << ";\n";
			break;
		}
		case PT_OLDITEMDB:
		{	// OldItemDBHead <Block> ',' <Block>
			// OldItemDBHead_eA <Block> ',' <Block> ',' <Block>

			if( parser.rt[rtpos].cChildNum>=4 ) // 4 or 6
			{
				basics::CStackElement& headnode = parser.rt[parser.rt[rtpos].cChildPos];


				// print the header
				// cannot go down recursively because we need the return value,
				// so just pretend a terminal printing
				prn.print_comments(parser, parser.rt[rtpos].cToken.line);
				
				// set printer to line mode
				prn.ignore_nl = true;

				int type = (parser.rt[rtpos].cChildNum==4) ?
					print_olditemdbhead( headnode.cToken.cLexeme ) :
					print_olditemdbheadea( headnode.cToken.cLexeme );

				// 0: Healing, 2: Usable, 3: Misc, 4: Weapon, 
				// 5: Armor, 6: Card, 7: Pet Egg,
				// 8: Pet Equipment, 10: Arrow, 
				// 11: Usable with delayed consumption (possibly unnecessary)

				// use script for types 0,2,11
				// equip script(s) for types 4,5,6,10

				if( (type==0) || (type==2) || (type==11) )
				{	// use script, just append
					basics::CStackElement& node1 = parser.rt[parser.rt[rtpos].cChildPos+1];
					if( node1.symbol.idx == PT_BLOCK &&
						parser.rt[node1.cChildPos+1].cChildNum)
					{
						prn << "{ OnUse: ";
						print_beautified(parser, node1.cChildPos+1);
						prn << '}';
					}

				}
				else if( (type==4) || (type==5) || (type==6) || (type==10) )
				{	// equip script, merge
					const basics::CStackElement& node1 = parser.rt[parser.rt[rtpos].cChildPos+3];
					const basics::CStackElement& node2 = parser.rt[parser.rt[rtpos].cChildPos + (parser.rt[rtpos].cChildNum==6)?5:0];

					const bool n1 = (node1.symbol.idx == PT_BLOCK && parser.rt[node1.cChildPos+1].cChildNum);
					const bool n2 = (parser.rt[rtpos].cChildNum==6 && node2.symbol.idx == PT_BLOCK && parser.rt[node2.cChildPos+1].cChildNum);

					if( n1 || n2 )
					{
						prn << "{ ";
					
						if( n1 )
						{
							prn << "OnEquip: ";
							print_beautified(parser, node1.cChildPos+1);
							prn << "end; ";
						}
						if( n2 )
						{
							prn << "OnUnequip: ";
							print_beautified(parser, node2.cChildPos+1);
							prn << "end; ";
						}
						prn << '}';
					}
				}
			//## TODO not yet, wait for finished item format
			//	else
			//	{
			//		prn << ';';
			//	}
				prn.ignore_nl = false;
				prn << '\n';
			}
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





bool oldeaParser::process(const char*name) const
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
		short p = parser->parse(PT_DECL);
		if (p < 0)
		{	// an error
			fprintf(stderr, "Parse Error in file '%s', line %i, col %i\n", name, parser->input.line, parser->input.column);

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

					this->prn.print_beautified(*parser, 0);
					this->prn.print_comments(*parser, 0xFFFFFFFF);
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


