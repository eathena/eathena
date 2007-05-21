// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef __IO_H__
#define __IO_H__

#include "basetypes.h"
#include "basecharset.h"
#include "baseparam.h"
#include "basebits.h"

#include "showmsg.h"	// ShowMessage
#include "utils.h"		// safefopen
#include "timer.h"		// timed config reload
#include "db.h"
#include "mmo.h"



///////////////////////////////////////////////////////////////////////////////
// logging stub
///////////////////////////////////////////////////////////////////////////////
class log_interface
{
protected:
	basics::string<> name;
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	log_interface(const basics::string<>& n) : name(n)		{}
	virtual ~log_interface()	{}
public:
	///////////////////////////////////////////////////////////////////////////
	virtual void put(const char* str)=0;
};

//## derive from stream when integrated
inline log_interface& operator <<(log_interface& l, const char* str)
{
	l.put(str);
	return l;
}
inline log_interface& operator <<(log_interface& l, int i)
{
	basics::string<> str = i;
	l.put(str);
	return l;
}
inline log_interface& operator <<(log_interface& l, uint i)
{
	basics::string<> str = i;
	l.put(str);
	return l;
}



///////////////////////////////////////////////////////////////////////////////
// Database Conversion Interface
///////////////////////////////////////////////////////////////////////////////
template<typename T>
class CDB_if : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CDB_if)
public:
	CDB_if():warned(false)			{}
	virtual ~CDB_if()	{}

	virtual size_t size() const=0;
	virtual T& operator[](size_t i)=0;

	bool warned;
	virtual bool save_value(const T&val)
	{
		if(!warned)
		{
			ShowError("conversion not implemented yet");
			warned=true;
		}
		return false;
	}

	void convert(CDB_if& db)
	{
		size_t i;
		for(i=0; i<db.size(); ++i)
		{
			T& val = db[i];
			this->save_value(val);
		}
	}
};







///////////////////////////////////////////////////////////////////////////////
// Account Database Interface
// for storing accounts stuff in login
///////////////////////////////////////////////////////////////////////////////
class CAccountDBInterface : public CDB_if<CLoginAccount>
{
	ICL_EMPTY_COPYCONSTRUCTOR(CAccountDBInterface)
protected:
	static basics::CParam<bool> case_sensitive;

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDBInterface()			{}
	virtual ~CAccountDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CLoginAccount& operator[](size_t i)=0;

	virtual bool existAccount(const char* userid) =0;
	virtual bool searchAccount(const char* userid, CLoginAccount&account) =0;
	virtual bool searchAccount(uint32 accid, CLoginAccount&account) =0;
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account) =0;
	virtual bool removeAccount(uint32 accid) =0;
	virtual bool saveAccount(const CLoginAccount& account) =0;
};

///////////////////////////////////////////////////////////////////////////////
// Dynamic Account Database Implementation
// does create a realisation of a specific database implementation internally
//!! todo remove the txtonly/sqlonly options and combine it for config choosing; 
//!! todo integrate it with the txt->sql/sql->txt converters
///////////////////////////////////////////////////////////////////////////////
class CAccountDB : public CAccountDBInterface
{
	CAccountDBInterface* db;
	CAccountDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDB():db(NULL)	{}
	CAccountDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CAccountDB()									{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const { return db->size(); }
	virtual CLoginAccount& operator[](size_t i) { return (*db)[i]; }

	virtual bool existAccount(const char* userid) { return db->existAccount(userid); }
	virtual bool searchAccount(const char* userid, CLoginAccount&account) { return db->searchAccount(userid, account); }
	virtual bool searchAccount(uint32 accid, CLoginAccount&account) { return db->searchAccount(accid, account); }
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account)	{ return db->insertAccount(userid, passwd, sex, email, account); }
	virtual bool removeAccount(uint32 accid) { return db->removeAccount(accid); }
	virtual bool saveAccount(const CLoginAccount& account) { return db->saveAccount(account); }
};


///////////////////////////////////////////////////////////////////////////////
// Char Database Interface
// for storing stuff in char
///////////////////////////////////////////////////////////////////////////////
class CCharDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CCharDBInterface)
protected:
	///////////////////////////////////////////////////////////////////////////
	static basics::CParam<bool>				char_new;	
	static basics::CParam<bool>				name_ignore_case;
	static basics::CParam<basics::charset>	name_letters;
	static basics::CParam<uint32>			start_zeny;
	static basics::CParam<ushort>			start_weapon;
	static basics::CParam<ushort>			start_armor;
	static basics::CParam<struct point>		start_point;
	static CFameList						famelists[4]; // order: pk, smith, chem, teak

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CCharDBInterface()			{}
	virtual ~CCharDBInterface()	{}

	bool testChar(const CCharAccount& account, char *name, const unsigned char str, const unsigned char agi, const unsigned char vit, const unsigned char int_, const unsigned char dex, const unsigned char luk, const unsigned char slot, const unsigned char hair_style, const unsigned char hair_color);
public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CCharCharacter& operator[](size_t i)=0;

	virtual bool existChar(const char* name) =0;
	virtual bool searchChar(const char* name, CCharCharacter&character) =0;
	virtual bool searchChar(uint32 charid, CCharCharacter&character) =0;
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data) =0;
	virtual bool removeChar(uint32 charid) =0;
	virtual bool saveChar(const CCharCharacter& character) =0;

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account) =0;
	virtual bool saveAccount(CCharAccount& account) =0;
	virtual bool removeAccount(uint32 accid)=0;

	virtual size_t getMailCount(uint32 cid, uint32 &all, uint32 &unread) =0;
	virtual size_t listMail(uint32 cid, unsigned char box, unsigned char *buffer) =0;
	virtual bool readMail(uint32 cid, uint32 mid, CMail& mail) =0;
	virtual bool deleteMail(uint32 cid, uint32 mid) =0;
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32 zeny, const struct item& item, uint32& msgid, uint32& tid) =0;

	virtual void loadfamelist()=0;
	CFameList& getfamelist(size_t i)
	{
		return famelists[(i<4)?i:0];
	}
};
///////////////////////////////////////////////////////////////////////////////
// Dynamic Account Database Implementation
// does create a realisation of a specific database implementation internally
//!! todo remove the txtonly/sqlonly options and combine it for config choosing; 
//!! todo integrate it with the txt->sql/sql->txt converters
///////////////////////////////////////////////////////////////////////////////
class CCharDB : public CCharDBInterface
{
	CCharDBInterface *db;

	CCharDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CCharDB():db(NULL)	{}
	CCharDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CCharDB()									{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		if(db) this->loadfamelist();
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const	{ return db->size(); }
	virtual CCharCharacter& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool existChar(const char* name)	{ return db->existChar(name); }
	virtual bool searchChar(const char* name, CCharCharacter&data)	{ return db->searchChar(name, data); }
	virtual bool searchChar(uint32 charid, CCharCharacter&data)	{ return db->searchChar(charid, data); }
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data)	{ return db->insertChar(account, name, str, agi, vit, int_, dex, luk, slot, hair_style, hair_color, data); }
	virtual bool removeChar(uint32 charid)	{ return db->removeChar(charid); }
	virtual bool saveChar(const CCharCharacter& data)	{ return db->saveChar(data); }

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account)	{ return db->searchAccount(accid, account); }
	virtual bool saveAccount(CCharAccount& account)	{ return db->saveAccount(account); }
	virtual bool removeAccount(uint32 accid)	{ return db->removeAccount(accid); }


	virtual size_t getMailCount(uint32 cid, uint32 &all, uint32 &unread) { return db->getMailCount(cid,all,unread); }
	virtual size_t listMail(uint32 cid, unsigned char box, unsigned char *buffer) { return db->listMail(cid, box, buffer); }
	virtual bool readMail(uint32 cid, uint32 mid, CMail& mail) { return db->readMail(cid, mid, mail); }
	virtual bool deleteMail(uint32 cid, uint32 mid) { return db->deleteMail(cid, mid); }
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32 zeny, const struct item& item, uint32& msgid, uint32& tid) { return db->sendMail(senderid, sendername, targetname, head, body, zeny, item, msgid, tid); }

	virtual void loadfamelist()	{ db->loadfamelist(); }
};




///////////////////////////////////////////////////////////////////////////////
// Guild Database Interface
///////////////////////////////////////////////////////////////////////////////
class CGuildDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CGuildDBInterface)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDBInterface()				{}
	virtual ~CGuildDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CGuild& operator[](size_t i)=0;
	virtual size_t castlesize() const	=0;
	virtual CCastle& castle(size_t i) =0;

	virtual bool searchGuild(const char* name, CGuild& guild) =0;
	virtual bool searchGuild(uint32 guildid, CGuild& guild) =0;
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild) =0;
	virtual bool removeGuild(uint32 guildid) =0;
	virtual bool saveGuild(const CGuild& guild) =0;

	virtual bool searchCastle(ushort castleid, CCastle& castle) =0;
	virtual bool saveCastle(const CCastle& castle) =0;

	virtual bool getCastles(basics::vector<CCastle>& castlevector) =0;
	virtual uint32 has_conflict(uint32 guild_id, uint32 account_id, uint32 char_id)=0;
};

///////////////////////////////////////////////////////////////////////////////
// Dynamic Database Implementation
///////////////////////////////////////////////////////////////////////////////
class CGuildDB : public CGuildDBInterface
{
	CGuildDBInterface *db;

	CGuildDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDB():db(NULL)	{}
	CGuildDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CGuildDB()										{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const				{ return db->size(); }
	virtual CGuild& operator[](size_t i)	{ return (*db)[i]; }
	virtual size_t castlesize() const		{ return db->castlesize(); }
	virtual CCastle& castle(size_t i)		{ return db->castle(i); }

	virtual bool searchGuild(const char* name, CGuild& guild)	{ return db->searchGuild(name, guild); }
	virtual bool searchGuild(uint32 guildid, CGuild& guild)	{ return db->searchGuild(guildid, guild); }
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild)	{ return db->insertGuild(member, name, guild); }
	virtual bool removeGuild(uint32 guildid)	{ return db->removeGuild(guildid); }
	virtual bool saveGuild(const CGuild& guild)	{ return db->saveGuild(guild); }

	virtual bool searchCastle(ushort cid, CCastle& castle)	{ return db->searchCastle(cid, castle); }
	virtual bool saveCastle(const CCastle& castle)	{ return db->saveCastle(castle); }

	virtual bool getCastles(basics::vector<CCastle>& castlevector)	{ return db->getCastles(castlevector); }
	virtual uint32 has_conflict(uint32 guild_id, uint32 account_id, uint32 char_id) { return db->has_conflict(guild_id, account_id, char_id); }
};



///////////////////////////////////////////////////////////////////////////////
// Party Class Definition
///////////////////////////////////////////////////////////////////////////////
class CParty : public party
{
public:
	CParty() : party(0,NULL)				{}
	CParty(const char* n) : party(0,n)		{}
	CParty(uint32 pid)  : party(pid,NULL)	{}

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by guildid

	bool operator==(const CParty& c) const { return this->party_id==c.party_id; }
	bool operator!=(const CParty& c) const { return this->party_id!=c.party_id; }
	bool operator> (const CParty& c) const { return this->party_id> c.party_id; }
	bool operator>=(const CParty& c) const { return this->party_id>=c.party_id; }
	bool operator< (const CParty& c) const { return this->party_id< c.party_id; }
	bool operator<=(const CParty& c) const { return this->party_id<=c.party_id; }

	///////////////////////////////////////////////////////////////////////////
	// get the leader id
	uint32 leader() const
	{
		for(size_t i=0; i<MAX_PARTY; ++i)
		{
			if (this->member[i].account_id > 0 && this->member[i].leader )
				return this->member[i].account_id;
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CParty& c, size_t i=0) const	
	{
		if(i==0)
			return (this->party_id - c.party_id);
		else
			return strcmp(this->name, c.name); 
	}

	///////////////////////////////////////////////////////////////////////////
	// class internal functions
	bool isEmpty() const
	{
		int i;
		for(i = 0; i < MAX_PARTY; ++i)
		{
			if (this->member[i].account_id > 0)
				return false;
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Party Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPartyDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CPartyDBInterface)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPartyDBInterface()				{}
	virtual ~CPartyDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CParty& operator[](size_t i)=0;

	virtual bool searchParty(const char* name, CParty& party) =0;
	virtual bool searchParty(uint32 pid, CParty& party) =0;
	virtual bool insertParty(uint32 accid, const char *nick, const char *mapname, ushort lv, const char *name, CParty &party) =0;
	virtual bool removeParty(uint32 pid) =0;
	virtual bool saveParty(const CParty& party) =0;
};

///////////////////////////////////////////////////////////////////////////////
// Dynamic Database Implementation
///////////////////////////////////////////////////////////////////////////////
class CPartyDB : public CPartyDBInterface
{
	CPartyDBInterface *db;
	CPartyDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPartyDB():db(NULL)	{}
	CPartyDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CPartyDB()										{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const				{ return db->size(); }
	virtual CParty& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool searchParty(const char* name, CParty& party)	{ return db->searchParty(name, party); }
	virtual bool searchParty(uint32 pid, CParty& party)	{ return db->searchParty(pid, party); }
	virtual bool insertParty(uint32 accid, const char *nick, const char *mapname, ushort lv, const char *name, CParty &party)	{ return db->insertParty(accid, nick, mapname, lv, name, party); }
	virtual bool removeParty(uint32 pid)	{ return db->removeParty(pid); }
	virtual bool saveParty(const CParty& party)	{ return db->saveParty(party); }
};


///////////////////////////////////////////////////////////////////////////////
// Storage Class Definitions
///////////////////////////////////////////////////////////////////////////////
class CPCStorage : public pc_storage
{
public:
	CPCStorage()											{}
	explicit CPCStorage(uint32 accid) : pc_storage(accid)	{}

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by accountid

	bool operator==(const CPCStorage& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CPCStorage& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CPCStorage& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CPCStorage& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CPCStorage& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CPCStorage& c) const { return this->account_id<=c.account_id; }
};
class CGuildStorage : public guild_storage
{
public:
	CGuildStorage()											{}
	explicit CGuildStorage(uint32 gid) : guild_storage(gid)	{}

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by guildid

	bool operator==(const CGuildStorage& c) const { return this->guild_id==c.guild_id; }
	bool operator!=(const CGuildStorage& c) const { return this->guild_id!=c.guild_id; }
	bool operator> (const CGuildStorage& c) const { return this->guild_id> c.guild_id; }
	bool operator>=(const CGuildStorage& c) const { return this->guild_id>=c.guild_id; }
	bool operator< (const CGuildStorage& c) const { return this->guild_id< c.guild_id; }
	bool operator<=(const CGuildStorage& c) const { return this->guild_id<=c.guild_id; }
};

///////////////////////////////////////////////////////////////////////////////
// Storage Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPCStorageDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CPCStorageDBInterface)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPCStorageDBInterface()				{}
	virtual ~CPCStorageDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CPCStorage& operator[](size_t i)=0;

	virtual bool searchStorage(uint32 accid, CPCStorage& stor) =0;
	virtual bool removeStorage(uint32 accid) =0;
	virtual bool saveStorage(const CPCStorage& stor) =0;
};
class CGuildStorageDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CGuildStorageDBInterface)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildStorageDBInterface()				{}
	virtual ~CGuildStorageDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CGuildStorage& operator[](size_t i)=0;

	virtual bool searchStorage(uint32 gid, CGuildStorage& stor) =0;
	virtual bool removeStorage(uint32 gid) =0;
	virtual bool saveStorage(const CGuildStorage& stor) =0;
};
///////////////////////////////////////////////////////////////////////////////
// Dynamic Database Implementation
///////////////////////////////////////////////////////////////////////////////
class CPCStorageDB : public CPCStorageDBInterface
{
	CPCStorageDBInterface *db;

	CPCStorageDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPCStorageDB():db(NULL)	{}
	CPCStorageDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CPCStorageDB()										{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const					{ return db->size(); }
	virtual CPCStorage& operator[](size_t i) 	{ return (*db)[i]; }

	virtual bool searchStorage(uint32 accid, CPCStorage& stor)	{ return db->searchStorage(accid, stor); }
	virtual bool removeStorage(uint32 accid)					{ return db->removeStorage(accid); }
	virtual bool saveStorage(const CPCStorage& stor)			{ return db->saveStorage(stor); }
};
class CGuildStorageDB : public CGuildStorageDBInterface
{
	CGuildStorageDBInterface *db;

	CGuildStorageDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildStorageDB():db(NULL)	{}
	CGuildStorageDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CGuildStorageDB()										{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const					{ return db->size(); }
	virtual CGuildStorage& operator[](size_t i) { return (*db)[i]; }

	virtual bool searchStorage(uint32 gid, CGuildStorage& stor)	{ return db->searchStorage(gid, stor); }
	virtual bool removeStorage(uint32 gid)						{ return db->removeStorage(gid); }
	virtual bool saveStorage(const CGuildStorage& stor)			{ return db->saveStorage(stor); }
};




///////////////////////////////////////////////////////////////////////////////
// Pet Class
///////////////////////////////////////////////////////////////////////////////

class CPet : public petstatus
{public:
	CPet()					{}
	CPet(const char* n)		{ memset(this, 0, sizeof(CPet)); safestrcpy(this->name, sizeof(this->name), n); }
	CPet(uint32 pid)		{ memset(this, 0, sizeof(CPet)); this->pet_id=pid; }
	CPet(uint32 pid, uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name)
	{
		this->account_id	= accid;
		this->char_id		= cid;
		this->pet_id		= pid;
		this->class_		= pet_class;
		this->level			= pet_lv;
		this->egg_id		= pet_egg_id;
		this->equip_id		= pet_equip;
		this->intimate		= intimate;
		this->hungry		= hungry;
		this->rename_flag	= renameflag;
		this->incuvate		= incuvat;
		safestrcpy(this->name, sizeof(this->name), pet_name);
	}

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by id

	bool operator==(const CPet& c) const { return this->pet_id==c.pet_id; }
	bool operator!=(const CPet& c) const { return this->pet_id!=c.pet_id; }
	bool operator> (const CPet& c) const { return this->pet_id> c.pet_id; }
	bool operator>=(const CPet& c) const { return this->pet_id>=c.pet_id; }
	bool operator< (const CPet& c) const { return this->pet_id< c.pet_id; }
	bool operator<=(const CPet& c) const { return this->pet_id<=c.pet_id; }
};

///////////////////////////////////////////////////////////////////////////////
// Pet Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPetDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CPetDBInterface)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPetDBInterface()				{}
	virtual ~CPetDBInterface()		{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CPet& operator[](size_t i)=0;

	virtual bool searchPet(uint32 pid, CPet& pet) =0;
	virtual bool insertPet(uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name, CPet& pet) =0;
	virtual bool removePet(uint32 pid) =0;
	virtual bool savePet(const CPet& pet) =0;
};


class CPetDB : public CPetDBInterface
{
	CPetDBInterface *db;
	CPetDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPetDB():db(NULL)	{}
	CPetDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CPetDB()									{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const			{ return db->size(); }
	virtual CPet& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool searchPet(uint32 pid, CPet& pet) { return db->searchPet(pid, pet); }
	virtual bool insertPet(uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name, CPet& pet)
	{
		return db->insertPet(accid, cid, pet_class, pet_lv, pet_egg_id, pet_equip, intimate, hungry, renameflag, incuvat, pet_name, pet);
	}
	virtual bool removePet(uint32 pid)  { return db->removePet(pid); }
	virtual bool savePet(const CPet& pet) { return db->savePet(pet); }
};


///////////////////////////////////////////////////////////////////////////////
// Homunculus Class
///////////////////////////////////////////////////////////////////////////////

class CHomunculus : public homunstatus
{
public:
	CHomunculus()				{}
	CHomunculus(uint32 hid)			{ memset(this, 0, sizeof(CHomunculus)); this->homun_id=hid; }
	CHomunculus(const homunstatus &h) : homunstatus(h) { }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by id

	bool operator==(const CHomunculus& c) const { return this->homun_id==c.homun_id; }
	bool operator!=(const CHomunculus& c) const { return this->homun_id!=c.homun_id; }
	bool operator> (const CHomunculus& c) const { return this->homun_id> c.homun_id; }
	bool operator>=(const CHomunculus& c) const { return this->homun_id>=c.homun_id; }
	bool operator< (const CHomunculus& c) const { return this->homun_id< c.homun_id; }
	bool operator<=(const CHomunculus& c) const { return this->homun_id<=c.homun_id; }
};

///////////////////////////////////////////////////////////////////////////////
// Homunculus Database Interface
///////////////////////////////////////////////////////////////////////////////
class CHomunculusDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CHomunculusDBInterface)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CHomunculusDBInterface()				{}
	virtual ~CHomunculusDBInterface()		{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CHomunculus& operator[](size_t i)=0;

	virtual bool searchHomunculus(uint32 hid, CHomunculus& hom) =0;
	virtual bool insertHomunculus(CHomunculus& hom) =0;
	virtual bool removeHomunculus(uint32 hid) =0;
	virtual bool saveHomunculus(const CHomunculus& hom) =0;
};


class CHomunculusDB : public CHomunculusDBInterface
{
	CHomunculusDBInterface *db;
	CHomunculusDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CHomunculusDB():db(NULL)	{}
	CHomunculusDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CHomunculusDB()									{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const					{ return db->size(); }
	virtual CHomunculus& operator[](size_t i)	{ return (*db)[i]; }


	virtual bool searchHomunculus(uint32 hid, CHomunculus& hom)	{ return db->searchHomunculus(hid, hom); }
	virtual bool insertHomunculus(CHomunculus& hom)				{ return db->insertHomunculus(hom); }
	virtual bool removeHomunculus(uint32 hid)					{ return db->removeHomunculus(hid); }
	virtual bool saveHomunculus(const CHomunculus& hom)			{ return db->saveHomunculus(hom); }
};







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// playground
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
	VARSCOPE_SERVER,	// server variables
	VARSCOPE_CHAR,		// character variables
	VARSCOPE_ACCOUNT,	// account variables
	VARSCOPE_GUILD,		// guild variables
	VARSCOPE_PARTY		// party variables
};
enum
{
	VARTYPE_STRING,		// string
	VARTYPE_INTEGER,	// fixed point number
	VARTYPE_FLOAT		// floating point number
};

class CVar : public basics::string<>
{
	uchar				cScope;	// variable scope
	uchar				cType;	// variable type
	uint32				cID;	// storage id
	basics::string<>	cValue;	// stringified value
public:
	/// standard constructor
	CVar()	{}
	CVar(const basics::string<>& name, const basics::string<>& value)
		: basics::string<>(name), cValue(value)	{}
	CVar(const basics::string<>& name) : basics::string<>(name)	{}
	/// destructor
	virtual ~CVar()	{}

	// element access
	const basics::string<>& name()			{ return *this; }
	const basics::string<>& name() const	{ return *this; }
	basics::string<>& value()				{ return cValue; }
	const basics::string<>& value() const 	{ return cValue; }


	/// conversion from saved string
	bool from_string(const char* str);
	/// conversion to save string
	size_t to_string(char* str, size_t len) const;
	/// conversion from transfer buffer
	bool from_buffer(const unsigned char* buf);
	/// conversion to transfer buffer
	size_t to_buffer(unsigned char* buf, size_t len) const;

	void str2array()
	{
		////////////////////////
		// hallo	2		3
		// 3		xxx		4
		////////////////////////
		basics::string<> str = "(2,3):hallo,2,3,3,xxx,4";
		basics::CRegExp re( "\\(([^,\\(\\)\\:]*)(?:,([^,\\(\\)\\:]*))*\\):([^,]*)(?:,([^,]*))*");

		if( re.match(str) && re.sub_count()==4 )
		{
			// result should be now:
			// [1] - first dimension
			// [2][0][n] - second to last dimension
			// [3] - first value
			// [4][0][n] - second to last value
			size_t v,i,k;
			basics::vector<size_t>				dimension;
			basics::vector< basics::string<> >	values;
			bool error=false;

			i=v= atol( re(1,0) );					// [0]
			// dimension size of 0 makes no sense
			//if( (error=(v==0)) );
			error=(v==0);
			if(!error)
			{
				dimension.push_back(v);
				for(k=0; k<re.sub_count(2); ++k)
				{	
					v= atol( re(2,k) );			// [2][0][n]
					if( (error=(v==0)) )
						break;
					i*=v;						// number of elements inside the volume
					dimension.push_back(v);
				}
			}
			if(!error)
			{	// values
				values.push_back(re(3,0));		// [3]
			
				for(k=0; k<re.sub_count(4) && k<i; ++k)
				{	
					values.push_back(re(4,k));	// [4][0][n]
				}
			}

			printf("dimensions: ");
			for(i=0; i<dimension.size(); ++i)
				printf("%lu ", (ulong)dimension[i]);

			printf("\nvalues:");
			for(i=0; i<values.size(); ++i)
				printf("'%s' ", (const char*)values[i]);
		}	
	}
	void array2str()
	{
		basics::string<> str = "(2,3):hallo,2,3,3,xxx,4";

	}
};


///////////////////////////////////////////////////////////////////////////////
class CVarDBInterface : public basics::noncopyable, public basics::global
{
	ICL_EMPTY_COPYCONSTRUCTOR(CVarDBInterface)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CVarDBInterface()				{}
	virtual ~CVarDBInterface()		{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const=0;
	virtual CVar& operator[](size_t i)=0;

	virtual bool searchVar(const basics::string<>& name, CVar& var)
	{
		return this->searchVar((const char*)name, var);
	}
	virtual bool searchVar(const char* name, CVar& var) =0;
	virtual bool insertVar(const basics::string<>& name, const basics::string<>& value)
	{
		return this->insertVar((const char*)name, (const char*)value);
	}
	virtual bool insertVar(const char* name, const char* value) =0;
	virtual bool removeVar(const basics::string<>& name)
	{
		return this->removeVar((const char*)name);
	}
	virtual bool removeVar(const char* name) =0;
	virtual bool saveVar(const CVar& var) =0;
};

class CVarDB : public CVarDBInterface
{
	CVarDBInterface *db;
	CVarDBInterface* getDB(const char *dbcfgfile);
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CVarDB():db(NULL)	{}
	CVarDB(const char *dbcfgfile):db(getDB(dbcfgfile))	{}
	virtual ~CVarDB()									{ delete db; }

public:

	bool init(const char *dbcfgfile)
	{
		if(db) delete db;
		db = getDB(dbcfgfile);
		return (NULL!=db);
	}
	bool close()
	{
		if(db)
		{	delete db;
			db=NULL;
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const				 { return db->size(); }
	virtual CVar& operator[](size_t i)		 { return (*db)[i]; }

	virtual bool searchVar(const basics::string<>& name, CVar& var)
	{
		return db->searchVar(name, var);
	}
	virtual bool searchVar(const char* name, CVar& var)
	{
		return db->searchVar(name, var);
	}
	virtual bool insertVar(const basics::string<>& name, const basics::string<>& value)
	{
		return db->insertVar(name, value);
	}
	virtual bool insertVar(const char* name, const char* value)
	{
		return db->insertVar(name, value);
	}
	virtual bool removeVar(const basics::string<>& name)
	{
		return db->removeVar(name);
	}
	virtual bool removeVar(const char* name)
	{
		return db->removeVar(name);
	}
	virtual bool saveVar(const CVar& var)
	{
		const bool del = (var.value() == "" || var.value() == "0" );
		return (del) ? db->removeVar(var) : db->saveVar(var);
	}
};












template <typename T> class play_interface
{
protected:
	play_interface()	{}
public:
	virtual ~play_interface()	{}

	///////////////////////////////////////////////////////////////////////////
	// check/init structure
	virtual bool do_init()=0;
	///////////////////////////////////////////////////////////////////////////
	// read single entry using query-by-example
	virtual bool do_read(T& elem)=0;
	///////////////////////////////////////////////////////////////////////////
	// read all entries
	virtual bool do_read(basics::vectorinterface<T>& vec)=0;
	///////////////////////////////////////////////////////////////////////////
	// save entry
	virtual bool do_save(const T& elem)=0;
	///////////////////////////////////////////////////////////////////////////
	// save all entries
	virtual bool do_save(const basics::vectorinterface<T>& vec)=0;
	///////////////////////////////////////////////////////////////////////////
	// create new entry
	virtual bool do_create(const T& elem)=0;
	///////////////////////////////////////////////////////////////////////////
	// remove entry
	virtual bool do_remove(const T& elem)=0;
};






class variable : public basics::string<>
{
public:
								// name of the variable (the class itself)
	basics::string<> value;		// stringified value of the variable

	variable()	{}
	variable(const basics::string<>& n, const basics::string<>& v) :  basics::string<>(n), value(v) {}
	~variable()	{}


	///////////////////////////////////////////////////////////////////////////
	// current buffer transfer
	// change to binary stream io

	// Return size of class.
	// this here depends on actual string length
	size_t size() const	
	{
		return this->length()+1+this->value.length()+1;
	}
	// Put class into given buffer
	void _tobuffer(unsigned char* &buf) const
	{
		if(buf)
		{
			if( !this->is_empty() )
			{
				memcpy(buf, (const char*)*this, this->length());
				buf+=this->length();
			}
			*buf++=0;
			if( !this->value.is_empty() )
			{
				memcpy(buf, (const char*)this->value, this->value.length());
				buf+=this->value.length();
			}
			*buf++=0;
		}
	}
	// Get class from given buffer
	void _frombuffer(const unsigned char* &buf)
	{
		if(buf)
		{
			this->clear();
			this->value.clear();

			if( *buf )
			{
				this->assign( (const char*)buf );
				buf+=this->length();
			}
			buf++;
			if( *buf )
			{
				this->value = (const char*)buf;
				buf+=this->value.length();
			}
			buf++;
		}
	}
	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};


// types of variables
enum var_t
{
	VAR_NONE = 0,		// not specified
	VAR_SERVER,			// server side variables (char and maps)
	VAR_ACCOUNT1,		// account variables (account limited)
	VAR_ACCOUNT2,		// account variables (shared among different logins)
	VAR_CHAR,			// char variables (specific for a distinct char)
	VAR_PARTY,			// party variables (specific for a distinct party)
	VAR_GUILD			// guild variables (specific for a distinct guild)
};

// array of variables
// each array has a type and a specific id
class vararray
{
public:
	basics::slist<variable>	carray;
	var_t			ctype;
	uint32			cid;

	vararray() : ctype(VAR_NONE), cid(0)	{}
	~vararray()	{}


	///////////////////////////////////////////////////////////////////////////
	// current buffer transfer

	// Return size of class.
	// this here depends on actual string length
	// spend only one byte for the var_t
	size_t size() const	
	{
		basics::slist<variable>::iterator iter(carray);
		size_t len = 1+2*sizeof(uint32);
		while(iter)
		{
			len += iter->size();
			iter++;
		}
		return len;
	}
	// Put class into given buffer
	void _tobuffer(unsigned char* &buf) const
	{
		if(buf)
		{
			*buf++=ctype;
			_L_tobuffer(cid,buf);
			_L_tobuffer((uint32)carray.length(),buf);
			basics::slist<variable>::iterator iter(carray);
			while(iter)
			{
				iter->_tobuffer(buf);
				iter++;
			}
		}
	}
	// Get class from given buffer
	void _frombuffer(const unsigned char* &buf)
	{
		if(buf)
		{
			variable tmp;
			uint32 sz;
			ctype = (var_t)*buf++;
			_L_frombuffer(cid,buf);
			_L_frombuffer(sz,buf);
			while(sz--)
			{
				tmp._frombuffer(buf);
				carray.push(tmp);
			}
		}
	}
	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};



using basics::smap;
// array of variables organized as simple map
class varmap : public smap< basics::string<>, basics::string<> >
{
public:
	var_t			ctype;
	uint32			cid;

	varmap() : ctype(VAR_NONE), cid(0)	{}
	~varmap()	{}

	///////////////////////////////////////////////////////////////////////////
	// current buffer transfer

	// Return size of class.
	// this here depends on actual string length
	// spend only one byte for the var_t
	size_t size() const	
	{
		size_t len = 1+2*sizeof(uint32);

		smap< basics::string<>, basics::string<> >::iterator iter(*const_cast<varmap*>(this));
		while(iter)
		{
			len += iter->key.length();
			len += iter->data.length();
			len+=2;
		}
		return len;
	}
	// Put class into given buffer
	void _tobuffer(unsigned char* &buf) const
	{
		if(buf)
		{
			*buf++=this->ctype;
			_L_tobuffer(this->cid,buf);
			_L_tobuffer((uint32)this->smap< basics::string<>, basics::string<> >::length(),buf);
			smap< basics::string<>, basics::string<> >::iterator iter(*this);
			while(iter)
			{
				if( !iter->key.is_empty() )
				{
					memcpy(buf, (const char*)iter->key, iter->key.length());
					buf+=iter->key.length();
				}
				*buf++=0;
				if( !iter->data.is_empty() )
				{
					memcpy(buf, (const char*)iter->data, iter->data.length());
					buf+=iter->data.length();
				}
				*buf++=0;

				iter++;
			}
		}
	}
	// Get class from given buffer
	void _frombuffer(const unsigned char* &buf)
	{
		this->clear();
		if(buf)
		{
			basics::string<> k,d;
			variable tmp;
			uint32 sz;
			ctype = (var_t)*buf++;
			_L_frombuffer(cid,buf);
			_L_frombuffer(sz,buf);
			while(sz--)
			{
				k.clear();
				d.clear();

				if( *buf )
				{
					k = (const char*)buf;
					buf+=k.length();
				}
				buf++;
				if( *buf )
				{
					d = (const char*)buf;
					buf+=d.length();
				}
				buf++;

				(*this)[k] = d;
			}
		}
	}
	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};






#endif//__IO_H__




