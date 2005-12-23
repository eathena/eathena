#ifndef __IO_H__
#define __IO_H__

#include "base.h"
#include "baseparam.h"
#include "showmsg.h"	// ShowMessage
#include "utils.h"		// safefopen
#include "socket.h"		// buffer iterator
#include "timer.h"		// timed config reload
#include "db.h"
#include "strlib.h"
#include "mmo.h"




///////////////////////////////////////////////////////////////////////////////
// Account Database
// for storing accounts stuff in login
///////////////////////////////////////////////////////////////////////////////

/*
	currently there is a bunch of more or less usefull data stored
	which also differs between current versions
	_useful_ data is
	///////////////////////////////////////////////////////////////////////////
*	Authentification Data used to authentify that clients are going from
	login through char to map.

	uint32 login_id1;	// just a random id given by login
	uint32 login_id2;	// just a random id given by login
	uint32 client_ip;	// the current ip of the client

	a client has to show these three values to get autentified
	(gets it in login process)

	///////////////////////////////////////////////////////////////////////////
*	Account Data which holds the necessary data for an account

	uint32 account_id;			// id to identify an account
	char userid[24];			// user name
	char passwd[34];			// user password
	unsigned char sex;			// gender
	unsigned char gm_level;		// gm_level
	unsigned char online;		// true when online (actually only usefull when adding datamining onto the storing data and not onto the server)
	char email[40];				// email address for confiming char deletion
	uint32 login_count;			// number of logins
	char last_login[24];		// timestamp of last login
	time_t ban_until;			// set to time(NULL)+delta for temporary ban
	time_t valid_until;			// set to time(NULL)+delta for temporary valid account or time(NULL) for complete disable

	the values last_ip, state, error_message, memo are quite useless,
	state might be usefull for debugging login of accounts
	but it is easier to read the output then to dig in the db for that

	///////////////////////////////////////////////////////////////////////////
*	Account Reg for account wide variables:

	unsigned short account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];
	///////////////////////////////////////////////////////////////////////////
*/


///////////////////////////////////////////////////////////////////////////////
// common structures
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Authentification
///////////////////////////////////////////////////////////////////////////////
class CAuth
{
public:
	uint32 account_id;
	uint32 login_id1;
	uint32 login_id2;
	uint32 client_ip;

	CAuth(uint32 aid=0) : account_id(aid)	{}
	~CAuth()	{}

	///////////////////////////////////////////////////////////////////////////
	// sorting by accountid
	bool operator==(const CAuth& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CAuth& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CAuth& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CAuth& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CAuth& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CAuth& c) const { return this->account_id<=c.account_id; }

	///////////////////////////////////////////////////////////////////////////
	// comparing
	bool isEqual(const CAuth& a) const	{ return account_id==a.account_id && client_ip==a.client_ip && login_id1==a.login_id1 && login_id2==a.login_id2; }

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	{ return 4*sizeof(uint32); }	// Return size of class

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account Reg
///////////////////////////////////////////////////////////////////////////////
class CAccountReg
{
public:
	uint16 account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];

	CAccountReg()	{ account_reg2_num=0; memset(account_reg2,0,sizeof(account_reg2)); }
	~CAccountReg()	{}

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	{ return sizeof(account_reg2_num)+ACCOUNT_REG2_NUM*sizeof(struct global_reg); }

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account related data for map server
///////////////////////////////////////////////////////////////////////////////
class CMapAccount : public CAuth, public CAccountReg
{
public:
	unsigned char sex;
	unsigned char gm_level;
	time_t ban_until;
	time_t valid_until;

	CMapAccount():CAuth(0)	{}
	~CMapAccount()	{}

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by accountid
	CMapAccount(uint32 aid):CAuth(aid)	{}
	bool operator==(const CMapAccount& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CMapAccount& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CMapAccount& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CMapAccount& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CMapAccount& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CMapAccount& c) const { return this->account_id<=c.account_id; }

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const;	// Return size of class

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account related data for char server
///////////////////////////////////////////////////////////////////////////////
class CCharAccount : public CMapAccount
{
public:
	char email[40];

	CCharAccount()	{}
	CCharAccount(uint32 aid):CMapAccount(aid)	{}
	~CCharAccount()	{}
	
	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	{ return sizeof(email)+CMapAccount::size();	}	// Return size of class

	void _tobuffer(unsigned char* &buf) const;		// Put class into given buffer
	void _frombuffer(const unsigned char* &buf);	// Get class from given buffer

	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

///////////////////////////////////////////////////////////////////////////////
// Account related data for login server
///////////////////////////////////////////////////////////////////////////////
class CLoginAccount : public CCharAccount
{
public:
	char userid[24];
	char passwd[34];
	unsigned char online;
	uint32 login_count;
	char last_login[24];
////////////////////////////
	// marked for deletion
	unsigned char state;
	char last_ip[16];
	char error_message[24];
	char memo[256];
//////////////////////////

	CLoginAccount()	{}
	~CLoginAccount()	{}
	///////////////////////////////////////////////////////////////////////////
	// creation of a new account
	CLoginAccount(uint32 accid, const char* uid, const char* pwd, unsigned char s, const char* em)
	{	// init account data
		this->account_id = accid;
		safestrcpy(this->userid, uid, 24);
		safestrcpy(this->passwd, pwd, 34);
		this->sex = sex;
		if( !email_check(em) )
			safestrcpy(this->email, "a@a.com", 40);
		else
			safestrcpy(this->email, em, 40);
		this->gm_level=0;
		this->login_count=0;
		*this->last_login= 0;
		this->ban_until = 0;
		this->valid_until = 0;
		this->account_reg2_num=0;
	}
	CLoginAccount(const char* uid)		{ safestrcpy(this->userid, uid, sizeof(this->userid));  }
	CLoginAccount(uint32 accid)	{ this->account_id=accid; }

	const CLoginAccount& operator=(const CCharAccount&a)
	{
		this->CCharAccount::operator=(a);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CLoginAccount& c, size_t i=0) const	
	{
		if(i==0)
			return (account_id - c.account_id);
		else
			return strcmp(this->userid, c.userid); 
	}

	// no buffer transfer necessary
};

///////////////////////////////////////////////////////////////////////////////
// mail structures
///////////////////////////////////////////////////////////////////////////////
class CMailHead
{
public:
	uint32 msid;
	unsigned char read;
	char name[24];
	char head[32];

	CMailHead()	{}
	CMailHead(uint32 id, unsigned char r, const char *n, const char *h) : msid(id), read(r)
	{
		safestrcpy(name,	n, sizeof(name));
		safestrcpy(head,	h, sizeof(head));
	}
	~CMailHead()	{}
	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	
	{
		return ( sizeof(msid)+sizeof(read)+sizeof(name)+sizeof(head) );	
	}	// Return size of class

	void _tobuffer(unsigned char* &buf) const		// Put class into given buffer
	{
		_L_tobuffer(msid,	buf);
		_B_tobuffer(read,	buf);
		_S_tobuffer(name,	buf, 24);
		_S_tobuffer(head,	buf, 32);
	}
	void _frombuffer(const unsigned char* &buf)		// Get class from given buffer
	{
		_L_frombuffer(msid,	buf);
		_B_frombuffer(read,	buf);
		_S_frombuffer(name,	buf, 24);
		_S_frombuffer(head,	buf, 32);
	}
	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};

class CMail : public CMailHead
{
public:
	char body[80];

	CMail()	{}
	CMail(uint32 id, unsigned char r, const char *n, const char *h, const char *b)
		: CMailHead(id, r, n, h)
	{
		safestrcpy(body,	b, sizeof(body));
	}
	~CMail()	{}
	
	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const	
	{
		return ( CMailHead::size()+sizeof(body) );	
	}	// Return size of class

	void _tobuffer(unsigned char* &buf) const		// Put class into given buffer
	{
		CMailHead::_tobuffer(buf);
		_S_tobuffer(body,	buf, 80);
	}
	void _frombuffer(const unsigned char* &buf)		// Get class from given buffer
	{
		CMailHead::_frombuffer(buf);
		_S_frombuffer(body,	buf, 80);
	}
	void tobuffer(unsigned char* buf) const	{ _tobuffer(buf); }		// Put class into given buffer
	void frombuffer(const unsigned char* buf) {	_frombuffer(buf); } // Get class from given buffer
};


///////////////////////////////////////////////////////////////////////////////
// char structures
///////////////////////////////////////////////////////////////////////////////
class CCharCharAccount : public CCharAccount
{
public:
	uint32 charlist[9];

	CCharCharAccount()		{}
	~CCharCharAccount()		{}
	CCharCharAccount(const CCharAccount& c) : CCharAccount(c)	{ memset(charlist,0,sizeof(charlist)); }
	

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by accountid
	CCharCharAccount(uint32 aid):CCharAccount(aid) {}
	bool operator==(const CCharAccount& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CCharAccount& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CCharAccount& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CCharAccount& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CCharAccount& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CCharAccount& c) const { return this->account_id<=c.account_id; }
};



class CCharCharacter : public mmo_charstatus
{
	int	server;
public:
	CCharCharacter():server(-1)		{}
	~CCharCharacter()		{}


	CCharCharacter(const char* n)		{ memset(this, 0, sizeof(CCharCharacter)); server=-1; safestrcpy(this->name, n, sizeof(this->name)); }
	CCharCharacter(uint32 cid)	{ memset(this, 0, sizeof(CCharCharacter)); server=-1; this->char_id=cid; }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by charid

	bool operator==(const CCharCharacter& c) const { return this->char_id==c.char_id; }
	bool operator!=(const CCharCharacter& c) const { return this->char_id!=c.char_id; }
	bool operator> (const CCharCharacter& c) const { return this->char_id> c.char_id; }
	bool operator>=(const CCharCharacter& c) const { return this->char_id>=c.char_id; }
	bool operator< (const CCharCharacter& c) const { return this->char_id< c.char_id; }
	bool operator<=(const CCharCharacter& c) const { return this->char_id<=c.char_id; }


	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CCharCharacter& c, size_t i=0) const	
	{
		if(i==0)
			return (this->char_id - c.char_id);
		else
			return strcmp(this->name, c.name); 
	}

};

///////////////////////////////////////////////////////////////////////////////
// map structure
///////////////////////////////////////////////////////////////////////////////
class CMapCharacter : public CCharCharacter, public CAuth
{
public:
	CMapCharacter()			{}
	~CMapCharacter()		{}

};



class CDBInterface : public noncopyable, public global
{
protected:
	CDBInterface()	{}
public:
	~CDBInterface()	{}
};


///////////////////////////////////////////////////////////////////////////////
// Account Database Interface
// for storing accounts stuff in login
///////////////////////////////////////////////////////////////////////////////
class CAccountDBInterface : public CDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDBInterface()			{}
	virtual ~CAccountDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
//	virtual size_t size()=0;
//	virtual CLoginAccount& operator[](size_t i)=0;

	virtual bool existAccount(const char* userid) =0;
	virtual bool searchAccount(const char* userid, CLoginAccount&account) =0;
	virtual bool searchAccount(uint32 accid, CLoginAccount&account) =0;
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account) =0;
	virtual bool removeAccount(uint32 accid) =0;
	virtual bool saveAccount(const CLoginAccount& account) =0;


	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquire() =0;
	virtual bool release() =0;
	virtual bool first() =0;
	virtual operator bool() =0;
	virtual bool operator++(int) =0;
	virtual bool save()=0;

	virtual bool find(const char* userid)=0;
	virtual bool find(uint32 accid)=0;
	virtual CLoginAccount& operator()()=0;
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
//	virtual size_t size()	{ return db->size(); }
//	virtual CLoginAccount& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool existAccount(const char* userid)	{ return db->existAccount(userid); }
	virtual bool searchAccount(const char* userid, CLoginAccount&account)	{ return db->searchAccount(userid, account); }
	virtual bool searchAccount(uint32 accid, CLoginAccount&account)	{ return db->searchAccount(accid, account); }
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account)	{ return db->insertAccount(userid, passwd, sex, email, account); }
	virtual bool removeAccount(uint32 accid)	{ return db->removeAccount(accid); }
	virtual bool saveAccount(const CLoginAccount& account)	{ return db->saveAccount(account); }


	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquire()					{ return db->aquire(); }
	virtual bool release()					{ return db->release(); }
	virtual bool first()					{ return db->first(); }
	virtual operator bool()					{ return *db; }
	virtual bool operator++(int)			{ return (*db)++; }
	virtual bool save()						{ return db->save(); }

	virtual bool find(const char* userid)	{ return db->find(userid); }
	virtual bool find(uint32 accid)			{ return db->find(accid); }
	virtual CLoginAccount& operator()()		{ return db->operator()(); }
};


///////////////////////////////////////////////////////////////////////////////
// Char Database Interface
// for storing stuff in char
///////////////////////////////////////////////////////////////////////////////
class CCharDBInterface : public CDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CCharDBInterface()			{}
	virtual ~CCharDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
//	virtual size_t size()=0;
//	virtual CCharCharacter& operator[](size_t i)=0;

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
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32& msgid, uint32& tid) =0;

	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquire()=0;
	virtual bool release()=0;
	virtual bool first()=0;
	virtual operator bool()=0;
	virtual bool operator++(int)=0;
	virtual bool save()=0;

	virtual bool find(const char* name)=0;
	virtual bool find(uint32 charid)=0;
	virtual CCharCharacter& operator()()=0;
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
//	virtual size_t size()	{ return db->size(); }
//	virtual CCharCharacter& operator[](size_t i)	{ return (*db)[i]; }

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
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32& msgid, uint32& tid) { return db->sendMail(senderid, sendername, targetname, head, body, msgid, tid); }


	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquire()					{ return db->aquire(); }
	virtual bool release()					{ return db->release(); }
	virtual bool first()					{ return db->first(); }
	virtual operator bool()					{ return *db; }
	virtual bool operator++(int)			{ return (*db)++; }
	virtual bool save()						{ return db->save(); }

	virtual bool find(const char* name)		{ return db->find(name); }
	virtual bool find(uint32 charid)		{ return db->find(charid); }
	virtual CCharCharacter& operator()()	{ return db->operator()(); }
};



///////////////////////////////////////////////////////////////////////////////
// Guild Class Definition
///////////////////////////////////////////////////////////////////////////////
class CGuildExp
{
	uint32 exp[100];
public:
	CGuildExp()	{ memset(exp,0,sizeof(exp)); }
	void init(const char* filename);

	uint32 operator[](size_t i)	{ i--; return (i<100) ? exp[i] : 0; }
};

class CGuild : public guild
{
public:
	CGuild()					{  }
	CGuild(const char* n)		{ memset(this, 0, sizeof(CGuild)); safestrcpy(this->name, n, sizeof(this->name)); }
	CGuild(uint32 gid)			{ memset(this, 0, sizeof(CGuild)); this->guild_id=gid; }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by guildid

	bool operator==(const CGuild& c) const { return this->guild_id==c.guild_id; }
	bool operator!=(const CGuild& c) const { return this->guild_id!=c.guild_id; }
	bool operator> (const CGuild& c) const { return this->guild_id> c.guild_id; }
	bool operator>=(const CGuild& c) const { return this->guild_id>=c.guild_id; }
	bool operator< (const CGuild& c) const { return this->guild_id< c.guild_id; }
	bool operator<=(const CGuild& c) const { return this->guild_id<=c.guild_id; }


	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CGuild& c, size_t i=0) const	
	{
		if(i==0)
			return (this->guild_id - c.guild_id);
		else
			return strcmp(this->name, c.name); 
	}

	///////////////////////////////////////////////////////////////////////////
	// class internal functions

	unsigned short checkSkill(unsigned short id)
	{
		unsigned short idx = id - GD_SKILLBASE;
		if(idx < MAX_GUILDSKILL)
			return skill[idx].lv;
		return 0;
	}

	static CGuildExp cGuildExp;

	bool calcInfo()
	{
		size_t i,c;
		uint32 nextexp;
		unsigned short before_max_member = this->max_member;
		unsigned short before_guild_lv = this->guild_lv;
		unsigned short before_skill_point = this->skill_point;

		// スキルIDの設定
		for(i=0;i<MAX_GUILDSKILL;i++)
			this->skill[i].id=i+GD_SKILLBASE;

		// ギルドレベル
		if(this->guild_lv<=0) this->guild_lv=1;
		nextexp = cGuildExp[this->guild_lv];
		while(this->exp >= nextexp && nextexp > 0)
		{
			this->exp-=nextexp;
			this->guild_lv++;
			this->skill_point++;
			nextexp = cGuildExp[this->guild_lv];
		}

		// ギルドの次の経験値
		this->next_exp = cGuildExp[this->guild_lv];

		// メンバ上限（ギルド拡張適用）
		this->max_member = 16 + this->checkSkill(GD_EXTENSION) * 6; //  Guild Extention skill - adds by 6 people per level to Max Member [Lupus]

		// 平均レベルとオンライン人数
		this->average_lv=0;
		this->connect_member=0;
		for(i=0,c=0; i<this->max_member; i++){
			if(this->member[i].account_id>0){
				this->average_lv+=this->member[i].lv;
				c++;

				if(this->member[i].online>0)
					this->connect_member++;
			}
		}
		if(c) this->average_lv/=c;

		// return true on changing a value
		return ( before_max_member != this->max_member ||
				 before_guild_lv != this->guild_lv ||
				 before_skill_point != this->skill_point );
	}
	int isEmpty()
	{
		size_t i;
		for(i=0; i<this->max_member; i++)
		{
			if (this->member[i].account_id > 0)
				return true;
		}
		return false;
	}
};
///////////////////////////////////////////////////////////////////////////////
// Guild Castle Class Definition
///////////////////////////////////////////////////////////////////////////////
class CCastle : public guild_castle
{
public:
	CCastle()	{}
	CCastle(ushort cid)				{ memset(this, 0, sizeof(CCastle)); this->castle_id=cid; }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by id
	bool operator==(const CCastle& c) const { return this->castle_id==c.castle_id; }
	bool operator!=(const CCastle& c) const { return this->castle_id!=c.castle_id; }
	bool operator> (const CCastle& c) const { return this->castle_id> c.castle_id; }
	bool operator>=(const CCastle& c) const { return this->castle_id>=c.castle_id; }
	bool operator< (const CCastle& c) const { return this->castle_id< c.castle_id; }
	bool operator<=(const CCastle& c) const { return this->castle_id<=c.castle_id; }
};

///////////////////////////////////////////////////////////////////////////////
// Guild Database Interface
///////////////////////////////////////////////////////////////////////////////
class CGuildDBInterface : public CDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDBInterface()				{}
	virtual ~CGuildDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
//	virtual size_t size()=0;
//	virtual CGuild& operator[](size_t i)=0;
//	virtual size_t castlesize()	=0;
//	virtual CCastle& castle(size_t i) =0;

	virtual bool searchGuild(const char* name, CGuild& guild) =0;
	virtual bool searchGuild(uint32 guildid, CGuild& guild) =0;
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild) =0;
	virtual bool removeGuild(uint32 guildid) =0;
	virtual bool saveGuild(const CGuild& guild) =0;

	virtual bool searchCastle(ushort castleid, CCastle& castle) =0;
	virtual bool saveCastle(CCastle& castle) =0;
	virtual bool removeCastle(ushort castleid)=0;

	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquireGuild()=0;
	virtual bool aquireCastle()=0;
	virtual bool releaseGuild()=0;
	virtual bool releaseCastle()=0;
	virtual bool firstGuild()=0;
	virtual bool firstCastle()=0;
	virtual bool isGuildOk()=0;
	virtual bool isCastleOk()=0;
	virtual bool nextGuild()=0;
	virtual bool nextCastle()=0;
	virtual bool saveGuild()=0;
	virtual bool saveCastle()=0;

	virtual bool findGuild(const char* name)=0;
	virtual bool findGuild(uint32 guildid)=0;
	virtual bool findCastle(ushort cid)=0;

	virtual CGuild& getGuild()=0;
	virtual CCastle& getCastle()=0;
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
//	virtual size_t size()					{ return db->size(); }
//	virtual CGuild& operator[](size_t i)	{ return (*db)[i]; }
//	virtual size_t castlesize()				{ return db->castlesize(); }
//	virtual CCastle& castle(size_t i)		{ return db->castle(i); }

	virtual bool searchGuild(const char* name, CGuild& guild)	{ return db->searchGuild(name, guild); }
	virtual bool searchGuild(uint32 guildid, CGuild& guild)	{ return db->searchGuild(guildid, guild); }
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild)	{ return db->insertGuild(member, name, guild); }
	virtual bool removeGuild(uint32 guildid)	{ return db->removeGuild(guildid); }
	virtual bool saveGuild(const CGuild& guild)	{ return db->saveGuild(guild); }

	virtual bool searchCastle(ushort cid, CCastle& castle)	{ return db->searchCastle(cid, castle); }
	virtual bool saveCastle(CCastle& castle)	{ return db->saveCastle(castle); }
	virtual bool removeCastle(ushort cid)	{ return db->removeCastle(cid); }


	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquireGuild()				{ return db->aquireGuild(); }
	virtual bool aquireCastle()				{ return db->aquireCastle(); }
	virtual bool releaseGuild()				{ return db->releaseGuild(); }
	virtual bool releaseCastle()			{ return db->releaseCastle(); }
	virtual bool firstGuild()				{ return db->firstGuild(); }
	virtual bool firstCastle()				{ return db->firstCastle(); }
	virtual bool isGuildOk()				{ return db->isGuildOk(); }
	virtual bool isCastleOk()				{ return db->isCastleOk(); }
	virtual bool nextGuild()				{ return db->nextGuild(); }
	virtual bool nextCastle()				{ return db->nextCastle(); }
	virtual bool saveGuild()				{ return db->saveGuild(); }
	virtual bool saveCastle()				{ return db->saveCastle(); }

	virtual bool findGuild(const char* name){ return db->findGuild(name); }
	virtual bool findGuild(uint32 guildid)	{ return db->findGuild(guildid); }
	virtual bool findCastle(ushort cid)		{ return db->findCastle(cid); }

	virtual CGuild& getGuild()				{ return db->getGuild(); }
	virtual CCastle& getCastle()			{ return db->getCastle(); }
};



///////////////////////////////////////////////////////////////////////////////
// Party Class Definition
///////////////////////////////////////////////////////////////////////////////
class CParty : public party
{
public:
	CParty()					{}
	CParty(const char* n)		{ memset(this, 0, sizeof(CParty)); safestrcpy(this->name, n, sizeof(this->name)); }
	CParty(uint32 pid)			{ memset(this, 0, sizeof(CParty)); this->party_id=pid; }

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by guildid

	bool operator==(const CParty& c) const { return this->party_id==c.party_id; }
	bool operator!=(const CParty& c) const { return this->party_id!=c.party_id; }
	bool operator> (const CParty& c) const { return this->party_id> c.party_id; }
	bool operator>=(const CParty& c) const { return this->party_id>=c.party_id; }
	bool operator< (const CParty& c) const { return this->party_id< c.party_id; }
	bool operator<=(const CParty& c) const { return this->party_id<=c.party_id; }


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
	bool isEmpty() 
	{
		int i;
		for(i = 0; i < MAX_PARTY; i++)
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
class CPartyDBInterface : public CDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPartyDBInterface()				{}
	virtual ~CPartyDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()=0;
	virtual CParty& operator[](size_t i)=0;

	virtual bool searchParty(const char* name, CParty& party) =0;
	virtual bool searchParty(uint32 pid, CParty& party) =0;
	virtual bool insertParty(uint32 accid, const char *nick, const char *map, ushort lv, const char *name, CParty &party) =0;
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
	virtual size_t size()					{ return db->size(); }
	virtual CParty& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool searchParty(const char* name, CParty& party)	{ return db->searchParty(name, party); }
	virtual bool searchParty(uint32 pid, CParty& party)	{ return db->searchParty(pid, party); }
	virtual bool insertParty(uint32 accid, const char *nick, const char *map, ushort lv, const char *name, CParty &party)	{ return db->insertParty(accid, nick, map, lv, name, party); }
	virtual bool removeParty(uint32 pid)	{ return db->removeParty(pid); }
	virtual bool saveParty(const CParty& party)	{ return db->saveParty(party); }
};


///////////////////////////////////////////////////////////////////////////////
// Storage Class Definitions
///////////////////////////////////////////////////////////////////////////////
class CPCStorage : public pc_storage
{
public:
	CPCStorage()						{}
	CPCStorage(uint32 accid)			{ memset(this, 0, sizeof(CPCStorage)); this->account_id=accid; }

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
	CGuildStorage()						{}
	CGuildStorage(uint32 gid)			{ memset(this, 0, sizeof(CGuildStorage)); this->guild_id=gid; }

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
class CPCStorageDBInterface : public CDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPCStorageDBInterface()				{}
	virtual ~CPCStorageDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()=0;
	virtual CPCStorage& operator[](size_t i)=0;

	virtual bool searchStorage(uint32 accid, CPCStorage& stor) =0;
	virtual bool removeStorage(uint32 accid) =0;
	virtual bool saveStorage(const CPCStorage& stor) =0;
};
class CGuildStorageDBInterface : public CDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildStorageDBInterface()				{}
	virtual ~CGuildStorageDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()=0;
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
	virtual size_t size()						{ return db->size(); }
	virtual CPCStorage& operator[](size_t i) 	{ return (*db)[i]; }

	virtual bool searchStorage(uint32 accid, CPCStorage& stor)	{ return db->searchStorage(accid, stor); }
	virtual bool removeStorage(uint32 accid)	{ return removeStorage(accid); }
	virtual bool saveStorage(const CPCStorage& stor)	{ return saveStorage(stor); }
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
	virtual size_t size()						{ return db->size(); }
	virtual CGuildStorage& operator[](size_t i) { return (*db)[i]; }

	virtual bool searchStorage(uint32 gid, CGuildStorage& stor)	{ return db->searchStorage(gid, stor); }
	virtual bool removeStorage(uint32 gid)	{ return removeStorage(gid); }
	virtual bool saveStorage(const CGuildStorage& stor)	{ return saveStorage(stor); }
};
















///////////////////////////////////////////////////////////////////////////////
// Pet Class
///////////////////////////////////////////////////////////////////////////////

class CPet : public s_pet
{public:
	CPet()					{}
	CPet(const char* n)		{ memset(this, 0, sizeof(CPet)); safestrcpy(this->name, n, sizeof(this->name)); }
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
		safestrcpy(this->name, pet_name, sizeof(this->name));
	}

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by guildid

	bool operator==(const CPet& c) const { return this->pet_id==c.pet_id; }
	bool operator!=(const CPet& c) const { return this->pet_id!=c.pet_id; }
	bool operator> (const CPet& c) const { return this->pet_id> c.pet_id; }
	bool operator>=(const CPet& c) const { return this->pet_id>=c.pet_id; }
	bool operator< (const CPet& c) const { return this->pet_id< c.pet_id; }
	bool operator<=(const CPet& c) const { return this->pet_id<=c.pet_id; }


	///////////////////////////////////////////////////////////////////////////
	// compare for Multilist
	int compare(const CPet& c, size_t i=0) const	
	{
		if(i==0)
			return (this->pet_id - c.pet_id);
		else
			return strcmp(this->name, c.name); 
	}
};

///////////////////////////////////////////////////////////////////////////////
// Pet Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPetDBInterface : public CDBInterface
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPetDBInterface()				{}
	virtual ~CPetDBInterface()		{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()=0;
	virtual CPet& operator[](size_t i)=0;

	virtual bool searchPet(const char* name, CPet& pet) =0;
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
	virtual size_t size()				{ return db->size(); }
	virtual CPet& operator[](size_t i) { return (*db)[i]; }

	virtual bool searchPet(const char* name, CPet& pet) { return db->searchPet(name, pet); }
	virtual bool searchPet(uint32 pid, CPet& pet) { return db->searchPet(pid, pet); }
	virtual bool insertPet(uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name, CPet& pet)
	{
		return db->insertPet(accid, cid, pet_class, pet_lv, pet_egg_id, pet_equip, intimate, hungry, renameflag, incuvat, pet_name, pet);
	}
	virtual bool removePet(uint32 pid)  { return db->removePet(pid); }
	virtual bool savePet(const CPet& pet) { return db->savePet(pet); }
};


#endif//__IO_H__
