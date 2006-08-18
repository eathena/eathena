#ifndef _SQL_H_
#define _SQL_H_

#include "baseio.h"
#include "basemysql.h"


#if defined(WITH_MYSQL)


///////////////////////////////////////////////////////////////////////////////
// sql base interface.
// wrapper for the sql handle, table control and parameter storage
class CSQLParameter
{
protected:

	///////////////////////////////////////////////////////////////////////////
	static basics::CMySQL sqlbase;					///< sql handle and connection pool
	///////////////////////////////////////////////////////////////////////////
	static basics::CParam< basics::string<> > mysqldb_id;	///< username
	static basics::CParam< basics::string<> > mysqldb_pw;	///< password
	static basics::CParam< basics::string<> > mysqldb_db;	///< database
	static basics::CParam< basics::string<> > mysqldb_ip;	///< server ip
	static basics::CParam< basics::string<> > mysqldb_cp;	///< server code page
	static basics::CParam< ushort   >         mysqldb_port;	///< server port

	///////////////////////////////////////////////////////////////////////////
	// parameters
	static basics::CParam< basics::string<> > tbl_login_log;
	static basics::CParam< basics::string<> > tbl_char_log;
	static basics::CParam< basics::string<> > tbl_map_log;

	static basics::CParam< basics::string<> > tbl_login_status;
	static basics::CParam< basics::string<> > tbl_char_status;
	static basics::CParam< basics::string<> > tbl_map_status;

	static basics::CParam< basics::string<> > tbl_account;
	
	static basics::CParam< basics::string<> > tbl_char;
	static basics::CParam< basics::string<> > tbl_memo;
	static basics::CParam< basics::string<> > tbl_inventory;
	static basics::CParam< basics::string<> > tbl_cart;
	static basics::CParam< basics::string<> > tbl_skill;
	static basics::CParam< basics::string<> > tbl_friends;

	static basics::CParam< basics::string<> > tbl_mail;

	static basics::CParam< basics::string<> > tbl_login_reg;
	static basics::CParam< basics::string<> > tbl_login_reg2;
	static basics::CParam< basics::string<> > tbl_char_reg;
	static basics::CParam< basics::string<> > tbl_guild_reg;

	static basics::CParam< basics::string<> > tbl_guild;
	static basics::CParam< basics::string<> > tbl_guild_skill;
	static basics::CParam< basics::string<> > tbl_guild_member;
	static basics::CParam< basics::string<> > tbl_guild_position;
	static basics::CParam< basics::string<> > tbl_guild_alliance;
	static basics::CParam< basics::string<> > tbl_guild_expulsion;
	
	static basics::CParam< basics::string<> > tbl_castle;
	static basics::CParam< basics::string<> > tbl_castle_guardian;
	
	static basics::CParam< basics::string<> > tbl_party;
	
	static basics::CParam< basics::string<> > tbl_storage;
	
	static basics::CParam< basics::string<> > tbl_guild_storage;
	
	static basics::CParam< basics::string<> > tbl_pet;
	static basics::CParam< basics::string<> > tbl_homunculus;
	static basics::CParam< basics::string<> > tbl_homunskill;

	static basics::CParam< basics::string<> > tbl_variable;

	static basics::CParam<bool> wipe_sql;
	static basics::CParam< basics::string<> > sql_engine;

	static basics::CParam<bool> log_login;
	static basics::CParam<bool> log_char;
	static basics::CParam<bool> log_map;


	static bool ParamCallback_Database_string(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval);
	static bool ParamCallback_Database_ushort(const basics::string<>& name, ushort& newval, const ushort& oldval);
	static bool ParamCallback_Tables(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval);


	///////////////////////////////////////////////////////////////////////////
	/// read number of rows from given table
	size_t get_table_size(basics::string<> tbl_name) const
	{
		basics::CMySQLConnection dbcon1(this->sqlbase);
		basics::string<> query;

		query << "SELECT COUNT(*) "
				 "FROM `" << dbcon1.escaped(tbl_name) << "` ";
		
		if( dbcon1.ResultQuery(query) )
		{
			return atol( dbcon1[0] );
		}
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////
	/// constructor.
	/// initialize the database on the first run
	CSQLParameter(const char* configfile)		
	{
		if(configfile) basics::CParamBase::loadFile(configfile);
		static bool first=true;
		if(first)
		{
			this->rebuild();
			first = false;
		}
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// destructor
	~CSQLParameter()	{}

	///////////////////////////////////////////////////////////////////////////
	// rebuild the tables
	static void rebuild();
};


///////////////////////////////////////////////////////////////////////////////
//
class CAccountDB_sql : public CAccountDBInterface, public CSQLParameter
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDB_sql(const char* configfile=NULL) : CSQLParameter(configfile)
	{
		 this->init(configfile);
	}
	virtual ~CAccountDB_sql()
	{
		close();
	}
protected:
	///////////////////////////////////////////////////////////////////////////
	bool init(const char* configfile);
	bool close();

	bool sql2struct(const basics::string<>& querycondition, CLoginAccount& account);
public:
	///////////////////////////////////////////////////////////////////////////
	// functions for db interface
	virtual size_t size() const;
	virtual CLoginAccount& operator[](size_t i);

	virtual bool existAccount(const char* userid);
	virtual bool searchAccount(const char* userid, CLoginAccount&account);
	virtual bool searchAccount(uint32 accid, CLoginAccount&account);
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account);
	virtual bool removeAccount(uint32 accid);
	virtual bool saveAccount(const CLoginAccount& account);
};

class CCharDB_sql : public CCharDBInterface, public CSQLParameter
{
public:
	CCharDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		init(dbcfgfile);
	}
	virtual ~CCharDB_sql()
	{}
protected:
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile);
	bool close(){ return true; }

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const;
	virtual CCharCharacter& operator[](size_t i);

	virtual bool existChar(const char* name);
	virtual bool existChar(uint32 char_id);
	virtual bool searchChar(const char* name, CCharCharacter&data);
	virtual bool searchChar(uint32 char_id, CCharCharacter&data);
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data);
	virtual bool removeChar(uint32 charid);
	virtual bool saveChar(const CCharCharacter& data);

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account);
	virtual bool saveAccount(CCharAccount& account);
	virtual bool removeAccount(uint32 accid);


	virtual size_t getMailCount(uint32 cid, uint32 &all, uint32 &unread);
	virtual size_t listMail(uint32 cid, unsigned char box, unsigned char *buffer);
	virtual bool readMail(uint32 cid, uint32 mid, CMail& mail);
	virtual bool deleteMail(uint32 cid, uint32 mid);
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32 zeny, const struct item& item, uint32& msgid, uint32& tid);

	virtual void loadfamelist();
};


///////////////////////////////////////////////////////////////////////////////
//
class CGuildDB_sql : public CGuildDBInterface, public CSQLParameter
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		this->init(dbcfgfile);
	}
	virtual ~CGuildDB_sql()
	{
		close();
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile);
	bool close()
	{
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const;
	virtual CGuild& operator[](size_t i);

	virtual size_t castlesize() const;
	virtual CCastle &castle(size_t i);


	virtual bool searchGuild(const char* name, CGuild& guild);
	virtual bool searchGuild(uint32 guildid, CGuild& guild); // TODO: Write
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &g);
	virtual bool removeGuild(uint32 guild_id);
	virtual bool saveGuild(const CGuild& g);

	virtual bool searchCastle(ushort castleid, CCastle& castle);
	virtual bool saveCastle(const CCastle& castle);
	virtual bool removeCastle(ushort castle_id);
	
	virtual bool getCastles(basics::vector<CCastle>& castlevector);
	virtual uint32 has_conflict(uint32 guild_id, uint32 account_id, uint32 char_id);
};

///////////////////////////////////////////////////////////////////////////////
//
class CPartyDB_sql : public CPartyDBInterface, public CSQLParameter
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPartyDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		init(dbcfgfile);
	}

	virtual ~CPartyDB_sql()
	{
		close();
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile);
	bool close()
	{
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const;
	virtual CParty& operator[](size_t i);

	virtual bool searchParty(const char* name, CParty& p);
	virtual bool searchParty(uint32 pid, CParty& p);
	virtual bool insertParty(uint32 accid, const char* nick, const char* mapname, ushort lv, const char* name, CParty& p);
	virtual bool removeParty(uint32 pid);
	virtual bool saveParty(const CParty& p);
};


///////////////////////////////////////////////////////////////////////////////
// Storage Database Interface
class CPCStorageDB_sql : public CPCStorageDBInterface, public CSQLParameter
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPCStorageDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		init(dbcfgfile);
	}

	~CPCStorageDB_sql()
	{
		close();
	}

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile);
	bool close()
	{
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	size_t size() const;
	CPCStorage& operator[](size_t i);

	virtual bool searchStorage(uint32 accid, CPCStorage& stor);
	virtual bool removeStorage(uint32 accid);
	virtual bool saveStorage(const CPCStorage& stor);

};

///////////////////////////////////////////////////////////////////////////////
//
class CGuildStorageDB_sql : public CGuildStorageDBInterface, public CSQLParameter
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildStorageDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		init(dbcfgfile);
	};
	virtual ~CGuildStorageDB_sql()
	{
		close();
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	size_t size() const;
	CGuildStorage& operator[](size_t i);

	virtual bool searchStorage(uint32 gid, CGuildStorage& stor);
	virtual bool removeStorage(uint32 gid);
	virtual bool saveStorage(const CGuildStorage& stor);

private:
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile);
	bool close()
	{
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Pet Database Interface
class CPetDB_sql : public CPetDBInterface, public CSQLParameter
{
public:
	CPetDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		init(dbcfgfile);
	}
	virtual ~CPetDB_sql()
	{
		close();
	}
protected:
	bool init(const char *dbcfgfile);
	bool close()
	{
		return true;
	}
public:
	virtual size_t size() const;
	virtual CPet& operator[](size_t i);

	virtual bool searchPet(uint32 pid, CPet& pet);
	virtual bool insertPet(uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name, CPet& pet);
	virtual bool removePet(uint32 pid);
	virtual bool savePet(const CPet& pet);
};


///////////////////////////////////////////////////////////////////////////////
// Homunculi Database Interface
class CHomunculusDB_sql : public CHomunculusDBInterface, public CSQLParameter
{
public:
	CHomunculusDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		init(dbcfgfile);
	}
	virtual ~CHomunculusDB_sql()
	{
		close();
	}
protected:
	bool init(const char *dbcfgfile);
	bool close()
	{
		return true;
	}
public:
	virtual size_t size() const;
	virtual CHomunculus& operator[](size_t i);

	virtual bool searchHomunculus(uint32 hid, CHomunculus& hom);
	virtual bool insertHomunculus(CHomunculus& hom);
	virtual bool removeHomunculus(uint32 hid);
	virtual bool saveHomunculus(const CHomunculus& hom);
};



///////////////////////////////////////////////////////////////////////////////
// Variable Database Interface
// testcase, possibly seperate into different implementations
class CVarDB_sql : public CVarDBInterface, public CSQLParameter
{
public:
	CVarDB_sql(const char *dbcfgfile) : CSQLParameter(dbcfgfile)
	{
		init(dbcfgfile);
	}
	virtual ~CVarDB_sql()
	{
		close();
	}
protected:
	bool init(const char *dbcfgfile);
	bool close()
	{
		return true;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size() const;
	virtual CVar& operator[](size_t i);


	virtual bool searchVar(const char* name, CVar& var);
	virtual bool insertVar(const char* name, const char* value);
	virtual bool removeVar(const char* name);
	virtual bool saveVar(const CVar& var);
};


#endif// defined(WITH_MYSQL)

#endif //_SQL_H_
