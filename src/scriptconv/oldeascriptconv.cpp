// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "oldeascriptconv.h"
#include "oldeascript.h"
#include "dblookup.h"


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
				", sprite=";
		
		const npcdb_entry *npc = npcdb_entry::lookup( basics::string<>(s) );
		if(npc)
			prn << '\"' << npc->Name1 << '\"';
		else
		{	// test for a mob sprite
			const mobdb_entry *mob = mobdb_entry::lookup( basics::string<>(s) );
			if(mob)
				prn << '\"' << mob->Name1 << '\"';
			else
				prn << s;
		}

			
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
			prn << '\t';
			const itemdb_entry *item = itemdb_entry::lookup( basics::string<>(id) );
			if(item)
				prn << '\"' << item->Name1 << '\"';
			else
				prn << id;
			if(price>0)
				prn << ':' << price;
			for(str+=n, ++cnt; *str==',' && 2==sscanf(str,",%d:%d%n", &id, &price,&n); str+=n, ++cnt)
			{	
				prn << ',';
				if(cnt%5==0) prn << '\n' << '\t';
				item = itemdb_entry::lookup( basics::string<>(id) );
				if(item)
					prn << '\"' << item->Name1 << '\"';
				else
					prn << id;
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

		prn << "monster (sprite=";
		const mobdb_entry *mob = mobdb_entry::lookup( basics::string<>(id) );
		if(mob)
			prn << '\"' << mob->Name1 << '\"';
		else
			prn << id;
		prn << ", name=\"" << name << "\", map=\"" << map << "\"";
		if( x1||x2||y1||y2 )
			prn << ", area={" << x1 << ", " << y1 << ", " << x2 << ", " << y2 << '}';
		prn << ", count=" << cn;
		if( t1>0 || t2>0 )
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
	me.Name1		= strings[ 1];
	me.Name2		= strings[ 2];
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

	mobdb_entry::insert(me);
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
	me.Name1		= strings[ 1];
	me.Name2		= strings[ 2];
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

	mobdb_entry::insert(me);
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

	itemdb_entry::insert(me);

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

	itemdb_entry::insert(me);

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

bool oldeaprinter::transform_function(basics::CParser_CommentStore& parser, int namepos, int parapos)
{
	oldeaprinter& prn = *this;

	basics::CStackElement *namenode = &parser.rt[namepos];
	basics::CStackElement *paranode = (parapos>0)?&parser.rt[parapos]:NULL;

	for(; namenode->cChildNum==1; namenode = &parser.rt[ namenode->cChildPos ]) {}
	if(paranode) for(; paranode->cChildNum==1; paranode = &parser.rt[ paranode->cChildPos ]) {}

	basics::string<> function_name = namenode->cToken.cLexeme;
	if(namenode->symbol.idx == EA_STRINGLITERAL)
	{	// first parameter of a callfunc has quotes, need stripping
		function_name.truncate(1,function_name.size()>2?function_name.size()-2:0);
	}

	if( function_name=="callfunc" )
	{	// callfunc <name>, <parameters>
		// -> <name> '(' <parameers> ')'
		if(paranode && (paranode->cChildNum==3 || paranode->cChildNum==1) ) 
		{	// calling function with parameters or with empty list
			transform_function(parser, paranode->cChildPos, paranode->cChildNum==3?(int)(paranode->cChildPos+2):-1);
		}
		else if(paranode && paranode->cChildNum==0) 
		{	// calling function with given list itself
			transform_function(parser, parapos, -1);
		}
		else
		{
			prn << '/' << '*' << " ignoring: ";
			transform_print_unprocessed(parser, namepos,0);
			transform_print_unprocessed(parser, parapos,0);
			prn << '*' << '/';
		}
	}
	else
	{
		// generate flat parameter list
		// also unfold a possible evaluation nonterminal
		basics::vector<size_t> parameter;
		if(paranode) // valid parameter node
		{
			basics::CStackElement* node = paranode;
			if( node->symbol.idx==EA_CALLLIST )
			{	// a list

				size_t i = node->cChildPos;
				size_t k = i+node->cChildNum;
				for(;i<k;)
				{
					if( parser.rt[i].symbol.idx == EA_COMMA )
					{	// skip
						++i;
					}
					else if( parser.rt[i].symbol.idx == EA_CALLLIST )
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
			else if( node->symbol.idx==EA_EVALUATION )
			{	// is '(' <value> ')', so strip the parenthesis
				parameter.push( node->cChildPos+1 );
			}
			else if(paranode->symbol.Type==1 || paranode->cChildNum)
			{	// a single value
				parameter.push(parapos);
			}
			// empty node otherwise
		}

		///////////////////////////////////////////////////////////////////////
		// function transformation

		size_t paramcounter= parameter.size();

		if( function_name == "set" )
		{	// setfunction, 2 parameters
			// transform to assignment
			if( parameter.size() )
			{	
				print_beautified(parser, parameter[0], EA_EXPR);
				prn << ' ' << '=' << ' ';
				if(parameter.size()>1)
				{
					if( parameter.size()>2 )
					{
						fprintf(stderr, "incorrect set command with more than 2 parameters, line %i, concatinating\n",
							(int)namenode->cToken.line);

						// concat the values
						prn << '{';
						size_t i;
						for(i=1; i<parameter.size()-1; ++i)
						{
							print_beautified(parser, parameter[i], EA_EXPR);
							prn << ',' << ' ';
						}
						print_beautified(parser, parameter[i], EA_EXPR);
						prn << '}';
					}
					else
					{	// default
						print_beautified(parser, parameter[1], EA_EXPR);
					}
				}
				else
				{
					fprintf(stderr, "incorrect set command with only one parameter, line %i, defaulting\n",
						(int)namenode->cToken.line);
					prn << '0';
				}
			}
			// just skip the whole thing when no parameters
			// quit here
			return true;
		}
		else if( function_name == "callsub" )
		{	
			if( parameter.size() )
			{
				prn << "callsub(";
				print_beautified(parser, parameter[0], EA_LABELSTM);
				prn << ')';
			}
			// ignore otherwisse
			// quit here
			return true;
		}
		else if( namenode->cToken.cLexeme=="close2" )
		{	// old close2 behaviour is only close the window, but not the script
			// this is now done with the new close function;
			
			prn <<	"// the new close function only close the window, but not the script\n"
					"// this was formally done with close2\n"
					"close()";
			// quit here
			return true;
		}
		else if( function_name=="close" )
		{	// old close behaviour is to close the window and the script
			// but very harsh and not waiting for the users close click
			// this is now done with new end statement;
			prn <<  "// the new close function only close the window, but not the script\n"
					"close();\nend";
			// quit here
			return true;
		}
		else if( function_name=="menu" )
		{	// transform "menu" functions
			// from "menu <a>, labela, <b>, labelb" to 
			// "switch(select(<a>, <b>); { case 1: goto labela; case 2: goto labelb; default: end; }

			if( (parameter.size() > 0) && ((parameter.size()&0x01) == 0) )
			{	// a list with even number of parameters
				size_t i;
				prn << "switch( select(" ;
				for(i=0;;)
				{	// print the arguments
					print_beautified(parser, parameter[i], 0);
					i+=2;
					if(i<parameter.size()) 
						prn<<','<<' ';
					else
						break;
				}
				prn << ") )\n{\n";
				for(i=1; i<parameter.size(); i+=2)
				{	// print the arguments
					prn << "case " << (1+(i>>1)) << ':' << ' ';
					if( parser.rt[parameter[i]].symbol.idx == EA_IDENTIFIER ) // label marker
						prn << "goto " << parser.rt[parameter[i]].cToken.cLexeme << ';' << '\n';
					else // '-'
						prn << "break;\n";
				}
				prn << "default: end;\n}\n";
			}
			else
			{	// line is wrong, comment it
				prn << "// incorrect menu\n/*";
				transform_print_unprocessed(parser, namepos, 0);
				transform_print_unprocessed(parser, parapos, 0);
				prn << "*/\n";
				if( !basics::is_console(stdout) )
					fprintf(stderr, "incorrect menu at line %i\n", (int)namenode->cToken.line);
			}
			// quit here
			return true;
		}
		else if( function_name=="delitem" || function_name=="getitem" || function_name=="getitem2" )
		{	// delitem/getitem ( <item>, <count>, parameters )

			prn << function_name << '(';
			if( parameter.size() >=1 )
			{
				const itemdb_entry *item = itemdb_entry::lookup(parser.rt[parameter[0]].cToken.cLexeme);
				if( item )
				{
					prn << '\"' << item->Name1 << '\"';
				}
				else
				{
					print_beautified(parser, parameter[0], 0);
				}
				prn << ',' << ' ';
				if( parameter.size() >=2 )
				{
					print_beautified(parser, parameter[1], 0);
				}
				else
				{
					prn << '1';
				}
				if(parameter.size()>2) prn << ',' << ' ';
				// print the rest starting from position 2
				paramcounter = 2;
			}
		}
		else if( function_name=="countitem" )
		{	// countitem ( <item> )
			prn << "countitem" << '(';
			if( parameter.size() >=1 )
			{
				const itemdb_entry *item = itemdb_entry::lookup(parser.rt[parameter[0]].cToken.cLexeme);
				if( item )
				{
					prn << '\"' << item->Name1 << '\"';
				}
				else
				{
					print_beautified(parser, parameter[0], 0);
				}
			}
			// no further parameter
		}
		else if( function_name == "areamonster" )
		{	// ea:		monster		<map>,<x>,<y>,<x>,<y>,<name>,<id>,<cnt>,<event>
			prn << "monster(";
			if( parameter.size()>=7 )
			{
				print_beautified(parser, parameter[0], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[1], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[2], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[3], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[4], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[5], 0);
				prn << ',' << ' ';
				
				const mobdb_entry *mob = mobdb_entry::lookup( parser.rt[parameter[6]].cToken.cLexeme );
				if(mob)
					prn << '\"' << mob->Name1 << '\"';
				else
				{
					prn << '\"';
					print_beautified(parser, parameter[6], 0);
					prn << '\"';
				}
				if(parameter.size()>7) prn << ',' << ' ';
				paramcounter = 7;
			}
			else
				paramcounter = 0;
		}
		else if( function_name == "monster" )
		{	// ea:		monster		<map>,<x>,<y>,<name>,<id>,<cnt>,<event>

			prn << "monster(";
			if( parameter.size()>=5 )
			{
				print_beautified(parser, parameter[0], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[1], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[2], 0);
				prn << ',' << ' ';
				print_beautified(parser, parameter[3], 0);
				prn << ',' << ' ';
				
				const mobdb_entry *mob = mobdb_entry::lookup( parser.rt[parameter[4]].cToken.cLexeme );
				if(mob)
					prn << '\"' << mob->Name1 << '\"';
				else
				{
					prn << '\"';
					print_beautified(parser, parameter[4], 0);
					prn << '\"';
				}
				if(parameter.size()>5) prn << ',' << ' ';
				paramcounter = 5;
			}
			else
				paramcounter = 0;
		}
		////////////////////
		else // default is using it is
		{
			prn << function_name << '(';
			paramcounter = 0;
		}

		if(paramcounter<parameter.size())
		{
			size_t i=paramcounter;
			print_beautified(parser, parameter[i], EA_EXPR);
			for(++i; i<parameter.size(); ++i)
			{
				prn << ',' << ' ';
				print_beautified(parser, parameter[i], EA_EXPR);
			}
		}
		prn << ')';
	}
	return true;
}


bool oldeaprinter::transform_identifier(basics::CParser_CommentStore& parser, int rtpos, short parent)
{	// convert variable scopes
	oldeaprinter& prn = *this;
	const char*str = parser.rt[rtpos].cToken.cLexeme;
	const char*epp = str+parser.rt[rtpos].cToken.cLexeme.size()-1;
	const char*add = "";
	bool maybe_player = false;

	prn.log(parser, rtpos);

	// strip 
	for( ; str<=epp && !basics::stringcheck::isalnum(*epp) && *epp!='_'; --epp) {}
	if(str[0]=='.' && str[1]=='@')
	{
		str+=2;
		add = "temp::";
		//could also skip these as default vars are temp
	}
	else if(str[0]=='.')
	{
		++str;
		add = "npc::";
	}
	else if(str[0]=='@')
	{
		++str;
		add = "temp::";
		//could also skip these as default vars are temp
	}
	else if(str[0]=='$' && str[1]=='@')
	{
		str+=2;
		add = "temp::";
		//could also skip these as default vars are temp
	}
	else if(str[0]=='$')
	{
		++str;
		add = "global::";
	}
	else if(str[0]=='#' && str[1]=='#')
	{
		str+=2;
		add = "login::";
	}
	else if(str[0]=='#')
	{
		++str;
		add = "account::";
	}
	else if(parent==0)
	{
		maybe_player = true;
	}
	
	// strip any other stuff from the beginning
	for( ; str<=epp && !basics::stringcheck::isalnum(*str) && *str!='_'; ++str) {}
	if(str<epp)
	{
		basics::string<> tmp(str, epp-str+1);
		// test for stuff from constdb
		const const_entry* ce;
		if( maybe_player && (ce = const_entry::lookup( tmp )) &&
			!ce->param )
		{	// check constant defines from constdb
			// but have player parameters with player:: prefix
			maybe_player = false;
		}

		if(maybe_player) add = "player::";

		// check for reserved words
		bool reserved = ( tmp == "if" ||
						  tmp == "else" ||
						  tmp == "for" ||
						  tmp == "while" ||
						  tmp == "return" ||
						  tmp == "goto" ||
						  tmp == "string" ||
						  tmp == "int" ||
						  tmp == "double" ||
						  tmp == "auto" ||
						  tmp == "var" );
		prn << add;
		if(reserved) prn << '_';
		for(; str<=epp; ++str)
			prn << *str;
		if(reserved) prn << '_';
	}
	return true;
}
bool oldeaprinter::transform_print_unprocessed(basics::CParser_CommentStore& parser, int rtpos, short parent)
{
	oldeaprinter& prn = *this;
	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// terminal
		prn << parser.rt[rtpos].cToken.cLexeme << ' ';
	}
	else
	{	// non terminal
		size_t j = parser.rt[rtpos].cChildPos;
		size_t k = j+parser.rt[rtpos].cChildNum;
		for(; j<k; ++j)
		{	// go down
			transform_print_unprocessed(parser, j, parent);
		}
	}
	return true;
}


bool oldeaprinter::print_beautified(basics::CParser_CommentStore& parser, int rtpos, short parent)
{
	oldeaprinter& prn = *this;
	bool ret = true;

	prn.print_comments(parser, rtpos);

	if( parser.rt[rtpos].symbol.Type == 1 )
	{	// terminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case EA_IDENTIFIER:
			ret = transform_identifier(parser, rtpos, parent);
			break;
		case EA_RBRACE:
			if(prn.scope) --prn.scope;
			if(!prn.newline) prn << '\n';
			prn << "}\n";
			break;
		case EA_LBRACE:
			if(!prn.newline) prn << '\n';
			prn << "{\n";
			++prn.scope;
			break;
		case EA_SEMI:
			prn << ";\n";
			break;
		case EA_COMMA:
			prn << ", ";
			break;
		case EA_GOTO:
			prn << "goto ";
			break;
		case EA_LPARAN:
		case EA_RPARAN:
			prn << parser.rt[rtpos].cToken.cLexeme;
			break;

		case EA_OLDSCRIPTHEAD:
			print_oldscripthead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDMINSCRIPTHEAD:
			print_oldminscripthead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDFUNCHEAD:
			print_oldfunctionhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDMONSTERHEAD:
			print_oldmonsterhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDWARPHEAD:
			print_oldwarphead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDMAPFLAGHEAD:
			print_oldmapflaghead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDDUPHEAD:
			print_oldduphead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDSHOPHEAD:
			print_oldshophead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDMOBDBHEAD:
			print_oldmobdbhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDMOBDBHEAD_EA:
			print_oldmobdbheadea( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDITEMDBHEAD:
			print_olditemdbhead( parser.rt[rtpos].cToken.cLexeme );
			break;

		case EA_OLDITEMDBHEAD_EA:
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
		print_beautified(parser, parser.rt[rtpos].cChildPos, parent);
	}
	else if( parser.rt[rtpos].cChildNum>1 )
	{	// other nonterminals
		switch( parser.rt[rtpos].symbol.idx )
		{
		case EA_LABELSTM:
		{	// set labels to zero scope
			int tmpscope = prn.scope;
			prn.scope=0;
			prn << parser.rt[parser.rt[rtpos].cChildPos].cToken.cLexeme << ":\n";
			prn.scope = tmpscope;
			break;
		}
		case EA_SUBFUNCTION:
		{	// 'function' <identifier> <block>
			// 'function' <identifier> ';'
			prn << "function auto ";
			prn.print_id(parser.rt[parser.rt[rtpos].cChildPos+1].cToken.cLexeme);
			prn << "() // TODO: add real function parameters\n";
			// print the function body or the semicolon
			print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
			break;
		}
		case EA_FUNCCALL:
		{	// identifier '(' <Call Liste> ')'
			ret = transform_function(parser, parser.rt[rtpos].cChildPos, parser.rt[rtpos].cChildPos+2);
			break;
		}
		case EA_CALLSTM:
		{	// identifier <Call List>  ';'
			// identifier  ';'
			prn.log(parser, rtpos);
			ret = transform_function(parser, parser.rt[rtpos].cChildPos, parser.rt[rtpos].cChildNum==3?(int)parser.rt[rtpos].cChildPos+1:-1);
			if(!prn.newline)
				prn << ';' << '\n';
			break;
		}
		case EA_ARG:
		{	// argument in a for statement, 
			// can be <expr> (which should have been handled already) or a call statement

			prn.log(parser, rtpos);
			
			if( parser.rt[rtpos].cChildNum > 1 )
			{
				//we only accept a single "set" command here
				if( parser.rt[parser.rt[rtpos].cChildPos].cToken.cLexeme=="set" )
				{	
					transform_function(parser, parser.rt[rtpos].cChildPos, parser.rt[rtpos].cChildPos+1);
				}
				else
				{	// argument is wrong, ignore it
					if( !basics::is_console(stdout) )
						fprintf(stderr, "ignoring incorrect 'for' arguments at line %i\n", (int)parser.rt[parser.rt[rtpos].cChildPos].cToken.line);
					prn << '/' << '*' << ' ';
					transform_print_unprocessed(parser, rtpos, parent);
					prn << '*' << '/';
				}
			}
			else if( parser.rt[rtpos].cChildNum )
			{
				print_beautified(parser, parser.rt[rtpos].cChildPos, parent);
			}
			break;
		}
		case EA_IFSTM:
		case EA_WHILESTM:
		{
			// if    '(' <Expr> ')' <Normal Stm>
			// if    '(' <Expr> ')' <Normal Stm> else <Normal Stm>
			// while '(' <Expr> ')' <Normal Stm>

			print_beautified(parser, parser.rt[rtpos].cChildPos+0, parent);
			prn << "( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
			prn << " )\n";

			basics::CStackElement* child = &parser.rt[ parser.rt[rtpos].cChildPos+4 ];
			for( ; child->cChildNum ==1; child = &parser.rt[ child->cChildPos ]) {}

			if( EA_BLOCK != child->symbol.idx )
				prn << '{' << '\n', ++prn.scope;

			print_beautified(parser, parser.rt[rtpos].cChildPos+4, parent);
			if(!prn.newline) prn << '\n';

			if( EA_BLOCK != child->symbol.idx )
				--prn.scope, prn << '}' << '\n';

			if( parser.rt[rtpos].cChildNum==7 )
			{	// else part
				if(!prn.newline)
					prn << '\n';
				prn << "else";
				
				bool is_elseif = false;
				bool is_block  = false;
				// find the first block with more than one children
				basics::CStackElement* child = &parser.rt[ parser.rt[rtpos].cChildPos+6 ];
				for( ; child->cChildNum ==1; child = &parser.rt[ child->cChildPos ]) {}
				if( child->cChildNum>1 )
				{
					is_block = EA_BLOCK == child->symbol.idx;
					is_elseif = (EA_NORMALSTM == child->symbol.idx && EA_IF == parser.rt[ child->cChildPos ].symbol.idx) ||
								(EA_IF        == child->symbol.idx);
				}
				if( is_block )
					prn << '\n';
				else if( is_elseif )
					prn << ' ';
				else
					prn << '\n' << '{' << '\n', ++prn.scope;

				print_beautified(parser, parser.rt[rtpos].cChildPos+6, parent);
				if(!prn.newline) prn << '\n';

				if( !is_block && !is_elseif )
					--prn.scope, prn << '}' << '\n';
			}
			break;
		}
		case EA_DOSTM:
		{	// do <Normal Stm> while '(' <Expr> ')' ';'

			basics::CStackElement* child = &parser.rt[ parser.rt[rtpos].cChildPos+1 ];
			for( ; child->cChildNum ==1; child = &parser.rt[ child->cChildPos ]) {}

			prn << "do\n";
			if( EA_BLOCK != child->symbol.idx )
				prn << '{' << '\n', ++prn.scope;

			print_beautified(parser, parser.rt[rtpos].cChildPos+1, parent);
			if(!prn.newline) prn << '\n';

			if( EA_BLOCK != child->symbol.idx )
				--prn.scope, prn << '}' << '\n';

			prn << "while( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+4, parent);
			prn << " );\n";
			break;
		}
		case EA_SWITCHSTM:
		{	// switch '(' <Expr> ')' '{' <Case Stms> '}'
			prn << "switch ( ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
			prn << " )\n";
			prn << "{\n";

			this->cHasDefault = false;
			print_beautified(parser, parser.rt[rtpos].cChildPos+5, parent);
			if(!prn.newline) prn << '\n';
			if( !this->cHasDefault )
			{	// always add a default case
				prn << "default:\nend;\n";
			}
			prn << "}\n";
			break;
		}
		case EA_FORSTM:
		{	// for '(' <Arg> ';' <Arg> ';' <Arg> ')' <Normal Stm>
			prn << "for(";
			print_beautified(parser, parser.rt[rtpos].cChildPos+2, parent);
			prn << "; ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+4, parent);
			prn << "; ";
			print_beautified(parser, parser.rt[rtpos].cChildPos+6, parent);
			prn << ")\n";

			basics::CStackElement* child = &parser.rt[ parser.rt[rtpos].cChildPos+1 ];
			for( ; child->cChildNum ==1; child = &parser.rt[ child->cChildPos ]) {}

			if( EA_BLOCK != child->symbol.idx )
				prn << '{' << '\n', ++prn.scope;

			print_beautified(parser, parser.rt[rtpos].cChildPos+8, parent);
			if(!prn.newline) prn << '\n';
			
			if( EA_BLOCK != child->symbol.idx )
				--prn.scope, prn << '}' << '\n';
			break;
		}
		case EA_EXPRSTM:
		{	// <ExprList> ';'
			size_t j = parser.rt[rtpos].cChildPos;
			size_t k = j+parser.rt[rtpos].cChildNum;
			for(; j<k; ++j)
			{	// go down
				print_beautified(parser, j, parent);
			}
			if(!prn.newline) prn << '\n';
			break;
		}
		case EA_CASESTM:
		{	// <Case Stms>  ::= case <Value> ':' <Stm List> <Case Stms>
			//			   | default ':' <Stm List> <Case Stms>
			//			   |
			size_t i = parser.rt[rtpos].cChildPos;

			if(!prn.newline) prn << '\n';
			if( EA_CASE == parser.rt[i].symbol.idx )
			{
				prn << "case ";
				++i;
				print_beautified(parser, i, parent);
			}
			else
			{
				if( this->cHasDefault )
				{
					fprintf(stderr, "multiple default cases, line %i",
						(int)parser.rt[i].cToken.line);
				}
				this->cHasDefault = true;
				prn << "default";
			}
			prn << ":\n";
			++prn.scope;
			print_beautified(parser, i+2, parent);
			--prn.scope;
			print_beautified(parser, i+3, parent);
			break;
		}
		case EA_RETURNSTM:
		{
			print_beautified(parser, parser.rt[rtpos].cChildPos, parent);
			if( parser.rt[rtpos].cChildNum==3 )
			{	// 'return' <arg> ';'
				prn << ' ';
				print_beautified(parser, parser.rt[rtpos].cChildPos+1, parent);
			}
			prn << ";\n";
			break;
		}
		case EA_GOTOSTM:
		{
			prn << "goto ";
			prn << parser.rt[parser.rt[rtpos].cChildPos+1].cToken.cLexeme;
			prn << ';' << '\n';
			break;
		}
		case EA_OLDITEMDB:
		{	// OldItemDBHead <Block> ',' <Block>
			// OldItemDBHead_eA <Block> ',' <Block> ',' <Block>

			if( parser.rt[rtpos].cChildNum>=4 ) // 4 or 6
			{
				basics::CStackElement& headnode = parser.rt[parser.rt[rtpos].cChildPos];


				// print the header
				// cannot go down recursively because we need the return value,
				// so just pretend a terminal printing
				prn.print_comments(parser, rtpos);
				
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
					if( node1.symbol.idx == EA_BLOCK &&
						parser.rt[node1.cChildPos+1].cChildNum)
					{
						prn << "{ OnUse: ";
						print_beautified(parser, node1.cChildPos+1, parent);
						prn << '}';
					}

				}
				else if( (type==4) || (type==5) || (type==6) || (type==10) )
				{	// equip script, merge
					const basics::CStackElement& node1 = parser.rt[parser.rt[rtpos].cChildPos+3];
					const basics::CStackElement& node2 = parser.rt[parser.rt[rtpos].cChildPos + (parser.rt[rtpos].cChildNum==6)?5:0];

					const bool n1 = (node1.symbol.idx == EA_BLOCK && parser.rt[node1.cChildPos+1].cChildNum);
					const bool n2 = (parser.rt[rtpos].cChildNum==6 && node2.symbol.idx == EA_BLOCK && parser.rt[node2.cChildPos+1].cChildNum);

					if( n1 || n2 )
					{
						prn << "{ ";
					
						if( n1 )
						{
							prn << "OnEquip: ";
							print_beautified(parser, node1.cChildPos+1, parent);
							prn << "end; ";
						}
						if( n2 )
						{
							prn << "OnUnequip: ";
							print_beautified(parser, node2.cChildPos+1, parent);
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
			size_t j = parser.rt[rtpos].cChildPos;
			size_t k = j+parser.rt[rtpos].cChildNum;
			for(; j<k; ++j)
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
		short p = parser->parse(EA_DECL);
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
		if( ok && parser->rt[0].symbol.idx==EA_DECL && parser->rt[0].cChildNum )
		{
			basics::CStackElement *child = &(parser->rt[parser->rt[0].cChildPos]);
			if( child &&
				( child->symbol.idx == EA_BLOCK ||

				  child->symbol.idx == EA_OLDSCRIPT ||
				  child->symbol.idx == EA_OLDFUNC ||
				  child->symbol.idx == EA_OLDMAPFLAG ||
				  child->symbol.idx == EA_OLDNPC ||
				  child->symbol.idx == EA_OLDDUP ||
				  child->symbol.idx == EA_OLDMOB ||
				  child->symbol.idx == EA_OLDSHOP ||
				  child->symbol.idx == EA_OLDWARP ||
				  child->symbol.idx == EA_OLDITEMDB ||
				  child->symbol.idx == EA_OLDMOBDB ||

				  child->symbol.idx == EA_OLDDUPHEAD ||
				  child->symbol.idx == EA_OLDFUNCHEAD ||
				  child->symbol.idx == EA_OLDITEMDBHEAD ||
				  child->symbol.idx == EA_OLDITEMDBHEAD_EA ||
				  child->symbol.idx == EA_OLDMAPFLAGHEAD ||
				  child->symbol.idx == EA_OLDMINSCRIPTHEAD ||
				  child->symbol.idx == EA_OLDMOBDBHEAD ||
				  child->symbol.idx == EA_OLDMOBDBHEAD_EA ||
				  child->symbol.idx == EA_OLDMONSTERHEAD ||
				  child->symbol.idx == EA_OLDSCRIPTHEAD ||
				  child->symbol.idx == EA_OLDSHOPHEAD ||
				  child->symbol.idx == EA_OLDWARPHEAD
				  )
			  )
			{
				if( (option&OPT_PRINTTREE)==OPT_PRINTTREE )
				{
					parser->print_rt_tree(0,0, false);
				}

				if( (option&OPT_TRANSFORM)==OPT_TRANSFORM )
				{
					//////////////////////////////////////////////////////////
					// tree transformation
					parsenode pnode(*parser);
					pnode.print_tree();
				}
				if( (option&OPT_BEAUTIFY)==OPT_BEAUTIFY && this->prn.output )
				{
					this->prn.scope=0;
					this->prn << '\n';

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
	return ok;
}


