#ifndef _DBLOOKUP_
#define _DBLOOKUP_

#include "basestring.h"



///////////////////////////////////////////////////////////////////////////////////////
// helpers
///////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////
struct itemdb_entry
{
	bool				conv_only;		// initialized by conversion
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

	itemdb_entry() : conv_only(false)
	{}

	bool operator==(const itemdb_entry& me) const	{ return this->ID == me.ID; }
	bool operator!=(const itemdb_entry& me) const	{ return this->ID != me.ID; }
	bool operator< (const itemdb_entry& me) const	{ return this->ID <  me.ID; }

	static bool insert(const itemdb_entry& me);
	static const itemdb_entry* lookup(const basics::string<>& str);
	static void load(const char* filename);
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
	bool				conv_only;		// initialized by conversion
	basics::string<>	ID;
	basics::string<>	Name1;
	basics::string<>	Name2;
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

	mobdb_entry() : conv_only(false)
	{}

	bool operator==(const mobdb_entry& me) const	{ return this->ID == me.ID; }
	bool operator!=(const mobdb_entry& me) const	{ return this->ID != me.ID; }
	bool operator< (const mobdb_entry& me) const	{ return this->ID <  me.ID; }


	static bool insert(const mobdb_entry& me);
	static const mobdb_entry* lookup(const basics::string<>& str);
	static void load(const char* filename);
};

/*
compound drops to list
optional:
mexp, mvp drop list
*/


/////////////////////////////////
struct npcdb_entry
{
	bool				conv_only;		// initialized by conversion
	basics::string<>	ID;				// mandatory	unique
	basics::string<>	Name1;			// mandatory	unique

	npcdb_entry() : conv_only(false)
	{}

	bool operator==(const npcdb_entry& me) const	{ return this->ID == me.ID; }
	bool operator!=(const npcdb_entry& me) const	{ return this->ID != me.ID; }
	bool operator< (const npcdb_entry& me) const	{ return this->ID <  me.ID; }

	static bool insert(const npcdb_entry& me);
	static const npcdb_entry* lookup(const basics::string<>& str);
	static void load(const char* filename);
};


/*
ea const table
*/

struct const_entry
{
	basics::string<>	name;
	int					value;
	bool				param;

	const_entry() {}
	~const_entry() {}
	const_entry(const const_entry& ce) : name(ce.name),value(ce.value),param(ce.param) {}
	const const_entry& operator=(const const_entry& ce)
	{
		name = ce.name;
		value = ce.value;
		param = ce.param;
		return *this;
	}

	static const const_entry* lookup(const basics::string<>& str);
	static void load(const char* filename);
};


#endif//_DBLOOKUP_
