#ifndef __IO_H__
#define __IO_H__

#include "base.h"
#include "showmsg.h"	// ShowMessage
#include "utils.h"		// safefopen
#include "socket.h"		// buffer iterator
#include "timer.h"		// timed config reload
#include "db.h"
#include "strlib.h"
#include "mmo.h"






//////////////////////////////////////////////////////////////////////////
// basic interface for reading configs from file
//////////////////////////////////////////////////////////////////////////
class CConfig
{
public:
	CConfig()
	{}
	virtual ~CConfig()
	{}
	//////////////////////////////////////////////////////////////////////
	// Loading a file, stripping lines, splitting it to part1 : part2
	// calling the derived function for processing
	//////////////////////////////////////////////////////////////////////
	bool LoadConfig(const char* cfgName)
	{
		char line[1024], w1[1024], w2[1024], *ip;
		FILE *fp;

		if ((fp = safefopen(cfgName, "r")) == NULL) {
			ShowError("Configuration file (%s) not found.\n", cfgName);
			return false;
		}
		ShowInfo("Reading configuration file '%s'\n", cfgName);
		while(fgets(line, sizeof(line), fp)) 
		{
			// terminate buffer
			line[sizeof(line)-1] = '\0';

			// skip leading spaces
			ip = line;
			while( isspace((int)((unsigned char)*ip) ) ) ip++; 

			// skipping comment lines
			if( ip[0] == '/' && ip[1] == '/')
				continue;
			
			memset(w2, 0, sizeof(w2));
			// format: "name:value"
			if (sscanf(ip, "%[^:]: %[^\r\n]", w1, w2) == 2)
			{
				CleanControlChars(w1);
				CleanControlChars(w2);

				if( strcasecmp(w1, "import") == 0 )
				{	// call recursive, prevent infinity loop (first order only)
					if( strcasecmp(cfgName,w2) !=0 )
						LoadConfig(w2);
				}
				else
				{	// calling derived function to process
					ProcessConfig(w1,w2);
				}
			}
		}
		fclose(fp);
		ShowInfo("Reading configuration file '%s' finished\n", cfgName);
		return true;
	}
	//////////////////////////////////////////////////////////////////////
	// virtual function for processing/storing tokens
	//////////////////////////////////////////////////////////////////////
	virtual bool ProcessConfig(const char*w1,const char*w2) = 0;

	//////////////////////////////////////////////////////////////////////
	// some global data processings
	//////////////////////////////////////////////////////////////////////
	static ulong String2IP(const char* str)
	{	// erturn value is host byte order
		// look up the name, can take long for timeout looking up non-existing addresses
		struct hostent *h = gethostbyname(str);
		if (h != NULL) 
		{	// returned ip's are hostbyte order
			return	  (((ulong)h->h_addr[3]) << 0x18 )
					| (((ulong)h->h_addr[2]) << 0x10 )
					| (((ulong)h->h_addr[1]) << 0x08 )
					| (((ulong)h->h_addr[0])         );
		}
		else
		{	// assume string is in ip format, just convert it
			return ntohl(inet_addr(str));
		}
	}
	static const char* IP2String(ulong ip, char*buffer=NULL)
	{	// given ip is in host byte order
		// usage of the static buffer here is not threadsave
		static char temp[32], *pp= (buffer) ? buffer:temp;
		sprintf(pp, "%d.%d.%d.%d", (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
		return pp;
	}

	static int SwitchValue(const char *str, int defaultmin=INT_MIN, int defaultmax=0x7FFFFFFF)
	{
		if( str )
		{
			if (strcasecmp(str, "on") == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja") == 0 || strcasecmp(str, "si") == 0)
				return 1;
			else if (strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0)
				return 0;
			else
			{
				int ret = atoi(str);
				return (ret<defaultmin) ? defaultmin : (ret>defaultmax) ? defaultmax : ret;
			}
		}
		else
			return 0;
	}
	static bool Switch(const char *str, bool defaultval=false)
	{
		if( str )
		{
			if (strcasecmp(str, "on") == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja") == 0 || strcasecmp(str, "si") == 0)
				return true;
			else if (strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0)
				return false;
		}
		return defaultval;
	}
	static bool CleanControlChars(char *str)
	{
		bool change = false;
		if(str)
		while( *str )
		{	// replace control chars 
			// but skip chars >0x7F which are negative in char representations
			if ( (*str<32) && (*str>0) )
			{
				*str = '_';
				change = true;
			}
			str++;
		}
		return change;
	}
};












///////////////////////////////////////////////////////////////////////////////
// common structures
///////////////////////////////////////////////////////////////////////////////
class CAuth
{
public:
	unsigned long account_id;
	unsigned long login_id1;
	unsigned long login_id2;
	unsigned long client_ip;

	CAuth(unsigned long aid=0) : account_id(aid)	{}
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
	size_t size() const
	{
		return 4*sizeof(unsigned long); 
	}
	void _tobuffer(unsigned char* &buf) const
	{
		if(!buf) return;
		_L_tobuffer( account_id,	buf);
		_L_tobuffer( login_id1, buf);
		_L_tobuffer( login_id2, buf);
		_L_tobuffer( client_ip, buf);
	}
	void _frombuffer(const unsigned char* &buf)
	{
		if(!buf) return;
		_L_frombuffer( account_id,	buf);
		_L_frombuffer( login_id1, buf);
		_L_frombuffer( login_id2, buf);
		_L_frombuffer( client_ip, buf);
	}
	void tobuffer(unsigned char* buf) const
	{
		_tobuffer(buf);
	}
	void frombuffer(const unsigned char* buf)
	{
		_frombuffer(buf);
	}
};

class CAccoutReg
{
public:
	unsigned short account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];

	CAccoutReg()	{ account_reg2_num=0; memset(account_reg2,0,sizeof(account_reg2)); }
	~CAccoutReg()	{}

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const
	{ 
		return sizeof(account_reg2_num)+ACCOUNT_REG2_NUM*sizeof(struct global_reg); 
	}
	void _tobuffer(unsigned char* &buf) const
	{
		size_t i;
		if(!buf) return;
		_W_tobuffer( (account_reg2_num),	buf);
		for(i=0; i<account_reg2_num && i<ACCOUNT_REG2_NUM; i++)
			_global_reg_tobuffer(account_reg2[i],buf);
	}
	void _frombuffer(const unsigned char* &buf)
	{
		size_t i;
		if(!buf) return;
		_W_frombuffer( (account_reg2_num),	buf);
		for(i=0; i<account_reg2_num && i<ACCOUNT_REG2_NUM; i++)
			_global_reg_frombuffer(account_reg2[i],buf);
	}
	void tobuffer(unsigned char* buf) const
	{
		_tobuffer(buf);
	}
	void frombuffer(const unsigned char* buf)
	{
		_frombuffer(buf);
	}
};
class CMapAccount : public CAuth, public CAccoutReg
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
	CMapAccount(unsigned long aid):CAuth(aid)	{}
	bool operator==(const CMapAccount& c) const { return this->account_id==c.account_id; }
	bool operator!=(const CMapAccount& c) const { return this->account_id!=c.account_id; }
	bool operator> (const CMapAccount& c) const { return this->account_id> c.account_id; }
	bool operator>=(const CMapAccount& c) const { return this->account_id>=c.account_id; }
	bool operator< (const CMapAccount& c) const { return this->account_id< c.account_id; }
	bool operator<=(const CMapAccount& c) const { return this->account_id<=c.account_id; }

	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const
	{
		return 
		sizeof(sex) +
		sizeof(gm_level) +
		sizeof(ban_until) +
		sizeof(valid_until) +
		CAuth::size();
		CAccoutReg::size();
	}
	void _tobuffer(unsigned char* &buf) const
	{
		unsigned long time;
		if(!buf) return;
		_B_tobuffer( sex,			buf);
		_B_tobuffer( gm_level,		buf);
		time = ban_until;	_L_tobuffer( time, buf);
		time = valid_until;	_L_tobuffer( time, buf);
		CAuth::_tobuffer(buf);
		CAccoutReg::_tobuffer(buf);
	}
	void _frombuffer(const unsigned char* &buf)
	{
		unsigned long time;
		if(!buf) return;
		_B_frombuffer( sex,			buf);
		_B_frombuffer( gm_level,	buf);
		_L_frombuffer( time, buf);	ban_until=time;
		_L_frombuffer( time, buf);	valid_until=time;
		CAuth::_frombuffer(buf);
		CAccoutReg::_frombuffer(buf);
	}
	void tobuffer(unsigned char* buf) const
	{
		_tobuffer(buf);
	}
	void frombuffer(const unsigned char* buf)
	{
		_frombuffer(buf);
	}
};
class CCharAccount : public CMapAccount
{
public:
	char email[40];

	CCharAccount()	{}
	CCharAccount(unsigned long aid):CMapAccount(aid)	{}
	~CCharAccount()	{}
	
	///////////////////////////////////////////////////////////////////////////
	// buffer transfer
	size_t size() const
	{
		return 
		sizeof(email) +
		CMapAccount::size();
	}
	void _tobuffer(unsigned char* &buf) const
	{
		if(!buf) return;
		_S_tobuffer( email,			buf, sizeof(email));
		CMapAccount::_tobuffer(buf);
	}
	void _frombuffer(const unsigned char* &buf)
	{
		if(!buf) return;
		_S_frombuffer( email,		buf, sizeof(email));
		CMapAccount::_frombuffer(buf);
	}
	void tobuffer(unsigned char* buf) const
	{
		_tobuffer(buf);
	}
	void frombuffer(const unsigned char* buf)
	{
		_frombuffer(buf);
	}
};

class CLoginAccount : public CCharAccount
{
public:
	char userid[24];
	char passwd[34];
	unsigned char state;
	unsigned char online;
	unsigned long login_count;
	char last_ip[16];
	char last_login[24];
	char error_message[24];
	char memo[256];

	CLoginAccount()	{}
	~CLoginAccount()	{}
	///////////////////////////////////////////////////////////////////////////
	// creation of a new account
	CLoginAccount(unsigned long accid, const char* uid, const char* pwd, unsigned char s, const char* em)
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
	CLoginAccount(unsigned long accid)	{ this->account_id=accid; }

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
// char structures
///////////////////////////////////////////////////////////////////////////////
class CCharCharAccount : public CCharAccount
{
public:
	unsigned long charlist[9];

	CCharCharAccount()		{}
	~CCharCharAccount()		{}
	CCharCharAccount(const CCharAccount& c) : CCharAccount(c)	{ memset(charlist,0,sizeof(charlist)); }
	

	///////////////////////////////////////////////////////////////////////////
	// creation and sorting by accountid
	CCharCharAccount(unsigned long aid):CCharAccount(aid) {}
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
	CCharCharacter(unsigned long cid)	{ memset(this, 0, sizeof(CCharCharacter)); server=-1; this->char_id=cid; }

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










///////////////////////////////////////////////////////////////////////////////
// Account Database Interface
// for storing accounts stuff in login
///////////////////////////////////////////////////////////////////////////////
class CAccountDBInterface : public global, public noncopyable
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDBInterface()			{}
	virtual ~CAccountDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()=0;
	virtual CLoginAccount& operator[](size_t i)=0;

	virtual bool existAccount(const char* userid) =0;
	virtual bool searchAccount(const char* userid, CLoginAccount&account) =0;
	virtual bool searchAccount(unsigned long accid, CLoginAccount&account) =0;
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account) =0;
	virtual bool removeAccount(unsigned long accid) =0;
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
	CAccountDBInterface *db;

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
	virtual size_t size()	{ return db->size(); }
	virtual CLoginAccount& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool existAccount(const char* userid)	{ return db->existAccount(userid); }
	virtual bool searchAccount(const char* userid, CLoginAccount&account)	{ return db->searchAccount(userid, account); }
	virtual bool searchAccount(unsigned long accid, CLoginAccount&account)	{ return db->searchAccount(accid, account); }
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account)	{ return db->insertAccount(userid, passwd, sex, email, account); }
	virtual bool removeAccount(unsigned long accid)	{ return db->removeAccount(accid); }
	virtual bool saveAccount(const CLoginAccount& account)	{ return db->saveAccount(account); }
};


///////////////////////////////////////////////////////////////////////////////
// Char Database Interface
// for storing stuff in char
///////////////////////////////////////////////////////////////////////////////
class CCharDBInterface : public global, public noncopyable
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CCharDBInterface()			{}
	virtual ~CCharDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()=0;
	virtual CCharCharacter& operator[](size_t i)=0;

	virtual bool existChar(const char* name) =0;
	virtual bool searchChar(const char* userid, CCharCharacter&character) =0;
	virtual bool searchChar(unsigned long charid, CCharCharacter&character) =0;
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data) =0;
	virtual bool removeChar(unsigned long charid) =0;
	virtual bool saveChar(const CCharCharacter& character) =0;

	virtual bool searchAccount(unsigned long accid, CCharCharAccount& account) =0;
	virtual bool saveAccount(CCharAccount& account) =0;
	virtual bool removeAccount(unsigned long accid)=0;

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
	virtual size_t size()	{ return db->size(); }
	virtual CCharCharacter& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool existChar(const char* name)	{ return db->existChar(name); }
	virtual bool searchChar(const char* name, CCharCharacter&data)	{ return db->searchChar(name, data); }
	virtual bool searchChar(unsigned long charid, CCharCharacter&data)	{ return db->searchChar(charid, data); }
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data)	{ return db->insertChar(account, name, str, agi, vit, int_, dex, luk, slot, hair_style, hair_color, data); }
	virtual bool removeChar(unsigned long charid)	{ return db->removeChar(charid); }
	virtual bool saveChar(const CCharCharacter& data)	{ return db->saveChar(data); }

	virtual bool searchAccount(unsigned long accid, CCharCharAccount& account)	{ return db->searchAccount(accid, account); }
	virtual bool saveAccount(CCharAccount& account)	{ return db->saveAccount(account); }
	virtual bool removeAccount(unsigned long accid)	{ return db->removeAccount(accid); }
};








#endif//__IO_H__
