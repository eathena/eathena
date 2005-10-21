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
	CConfig(){}
	virtual ~CConfig(){}

	bool LoadConfig(const char* cfgName);							// Load and parse config
	virtual bool ProcessConfig(const char*w1,const char*w2) = 0;	// Proccess config

	static int SwitchValue(const char *str, int defaultmin=INT_MIN, int defaultmax=INT_MAX);  // Return 0/1 for no/yes
	static bool Switch(const char *str, bool defaultval=false);		// Return true/false for yes/no, if unknown return defaultval
	
	static bool CleanControlChars(char *str);						// Replace control chars with '_' and return location of change
};


///////////////////////////////////////////////////////////////////////////////
// basic class for using the old way timers
///////////////////////////////////////////////////////////////////////////////
class CTimerBase : public global, public noncopyable
{
	int cTimer;
protected:
	CTimerBase(unsigned long interval)
	{
		init(interval);
	}
	virtual ~CTimerBase()
	{
		if(cTimer>0)
		{
			delete_timer(cTimer, timercallback);
			cTimer = -1;
		}
	}
	bool init(unsigned long interval)
	{
		if(interval<1000)
			interval = 1000;
		cTimer = add_timer_interval(gettick()+interval, interval, timercallback, 0, intptr(this), false);
		return (cTimer>=0);
	}

	// user function
	virtual bool timeruserfunc(unsigned long tick) = 0;

	// external calling from external timer implementation
	static int timercallback(int timer, unsigned long tick, int id, intptr data)
	{
		if(data.ptr)
		{
			CTimerBase* base = (CTimerBase*)data.ptr;
			if(timer==base->cTimer)
			{
				if( !base->timeruserfunc(tick) )
				{
					delete_timer(base->cTimer, timercallback);
					base->cTimer = -1;
				}
			}
		}
		return 0;
	}
};



///////////////////////////////////////////////////////////////////////////////
// Parameter class
// for parameter storage and distribution
// reads in config files and holds the variables
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// predefined conversion functions for common data types
//!! move them to virtual convert implementations directly at CParamData
inline bool paramconvert(long &t, const char* str)
{
	char *ss=NULL;
	t = (!str || strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0) ? 0 :
		(        strcasecmp(str, "on" ) == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja"  ) == 0 || strcasecmp(str, "si") == 0) ? 1 : 
		strtol(str, &ss, 0);
	return true;
}
inline bool paramconvert(ulong &t, const char* str)
{
	char *ss=NULL;
	t = (!str || strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0) ? 0 :
		(        strcasecmp(str, "on" ) == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja"  ) == 0 || strcasecmp(str, "si") == 0) ? 1 : 
		strtoul(str, &ss, 0);
	return true;
}
inline bool paramconvert(double &t, const char* s) 
{
	char *ss=0;
	t= (s) ? strtod(s, &ss) : 0;
	//if(ss && *ss) return PARAM_CONVERSION;
	return true;
}

inline bool paramconvert( int            &t, const char* s) {  long val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( unsigned       &t, const char* s) { ulong val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( short          &t, const char* s) {  long val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( unsigned short &t, const char* s) { ulong val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( char           &t, const char* s) {  long val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( unsigned char  &t, const char* s) { ulong val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( bool           &t, const char* s) {  long val; bool ret=paramconvert(val, s); t=(0!=val); return ret; }
inline bool paramconvert( float          &t, const char* s) { double val; bool ret=paramconvert( val, s); t=val; return ret; }

///////////////////////////////////////////////////////////////////////////////
// template for the rest
// usable types need an assignment operator of MiniString or const char* 
template <class T> inline bool paramconvert( T &t, const char* s)
{
	t=(s) ? s : "";
	return true;
}



///////////////////////////////////////////////////////////////////////////////
// parameter base class
class CParamBase
{
	///////////////////////////////////////////////////////////////////////////
	// class data
	bool cReferenced;	// true when this parameter has been referenced
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CParamBase() : cReferenced(false)	{}
	virtual ~CParamBase()	{}
	///////////////////////////////////////////////////////////////////////////
	// check/set the reference
	bool isReferenced()	{ return cReferenced; }
	bool setReference()	{ return (cReferenced=true); }
	///////////////////////////////////////////////////////////////////////////
	// access functions for overloading
	virtual const std::type_info& getType()	{ return typeid(CParamBase); }
	virtual bool assign(const char*s)	{ return false; }
	virtual void print()	{ printf("\nparameter uninitialized"); }
};

///////////////////////////////////////////////////////////////////////////////
// parameter storage
// stored parameters and their names as smart pointers in a sorted list
// gives out references for changing parameters at their storage
// needs external lock of the provided mutex for multithread environment
class CParamStorage : public MiniString
{
	///////////////////////////////////////////////////////////////////////////
	// internal class for loading/storing/reloading config files
	// and automatically setting up of changed parameters
	class CParamLoader : private CConfig, private CTimerBase
	{
		///////////////////////////////////////////////////////////////////////
		// stores information about a file
		class CFileData : public MiniString
		{
			///////////////////////////////////////////////////////////////////
			// class data
			time_t modtime;	// last modification time of the file
		public:
			///////////////////////////////////////////////////////////////////
			// construction/destruction
			CFileData()	{}
			CFileData(const char* name) : MiniString(name)
			{
				struct stat s;
				if( 0==stat(name, &s) )
					modtime = s.st_mtime;
			}
			~CFileData()	{}

			///////////////////////////////////////////////////////////////////
			// checking file state
			bool isModified()
			{
				struct stat s;
				return ( 0==stat((const char*)(*this), &s) && s.st_mtime!=this->modtime );
			}
		};
		///////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////
		// class data
		TslistDCT<CFileData>	cFileList;			// list of loaded files
	public:
		CParamLoader() : CTimerBase(5*60*1000)		// 5 minutes interval for file checks
		{}
		/////////////////////////////////////////////////////////////
		// timer callback
		virtual bool timeruserfunc(unsigned long tick)
		{	// check all listed files for modification
			size_t i;
			for(i=0; i<cFileList.size(); i++)
				if( cFileList[i].isModified() )
					LoadConfig( cFileList[i] );
			return true;
		}
		/////////////////////////////////////////////////////////////
		// config processing callback
		virtual bool ProcessConfig(const char*w1,const char*w2)
		{	// create/update parameter
			CParamStorage::create(w1, w2);
			return true;
		}
		/////////////////////////////////////////////////////////////
		// external access
		void loadFile(const char* name)
		{
			LoadConfig( name );
			cFileList.insert( CFileData(name) );
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// static data
	static CParamLoader				cLoader;
	static TslistDCT<CParamStorage>	cParams;	
	static Mutex					cLock;
public:
	///////////////////////////////////////////////////////////////////////////
	// class Data
	TPtrCount<CParamBase>		cParam;	// pointer to the parameter data
	ulong						cTime;	// time of last access

	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	CParamStorage()
	{}
	CParamStorage(const char* name) : MiniString(name)
	{}
	~CParamStorage()
	{}
	///////////////////////////////////////////////////////////////////////////
	// class access
	void print()
	{
		printf("\nparameter name: %s", (const char*)(*this)); 
		cParam->print();
	}

	///////////////////////////////////////////////////////////////////////////
	// static access functions
	static CParamStorage& getParam(const char* name)
	{
		ScopeLock sl(CParamStorage::getMutex());
		size_t pos;
		CParamStorage tmp(name);
		if( !cParams.find(tmp, 0, pos) )
		{
			tmp.cTime = gettick();
			cParams.insert(tmp);
			if( !cParams.find(tmp, 0, pos) )
				throw CException("Params: insert failed");		
		}
		return cParams[pos];
	}
    static Mutex& getMutex()	{ return cLock; }
	// clean unreferenced parameters
	static void clean()
	{
		size_t i=cParams.size();
		while(i>0)
		{
			i--;
			if( !cParams[i].cParam->isReferenced() )
				cParams.removeindex(i);
		}
	}
	static void listall()
	{
		size_t i;
		printf("\nList of Parameters (existing %i):", cParams.size());
		for(i=0; i<cParams.size(); i++)
			cParams[i].print();
	}
	static void create(const char* name, const char* value);
	static void loadFile(const char* name)
	{
		cLoader.loadFile(name);
	}
};





///////////////////////////////////////////////////////////////////////////////
// variable parameter template
template <class T> class CParam
{
private:
	///////////////////////////////////////////////////////////////////////////
	// internal parameter data template
	template <class X> class CParamData : public CParamBase
	{
	public:
		///////////////////////////////////////////////////////////////////////
		// the actual parameter data
		X	cData;
		///////////////////////////////////////////////////////////////////////
		// construction
		CParamData(const char* value)
		{
			paramconvert(cData, value);
		}
		CParamData(const X& value) : cData(value)
		{}
		///////////////////////////////////////////////////////////////////////
		// access
		// typeid needs #include <typeinfo> 
		// which is a bit stange among the different std implementations
		virtual const std::type_info& getType()	{ return typeid(X); }
		virtual bool assign(const char*s)	{ return paramconvert(cData, s); }
		virtual bool assign(const X&s)		{ if(cData != s) { cData = s; return true; } return false; }
		virtual void print()
		{
			printf("type: %s, value='%s'", typeid(X).name(), (const char*)MiniString(cData));
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// parameter storage
	CParamStorage		cStor;	// a copy of the storage to keep the smart pointers alive
	T&					cData;	// a reference to the data
public:
	///////////////////////////////////////////////////////////////////////////
	// construction/destruction (can use default copy/assign here)
	CParam(const char* name)
		: cStor(), cData(convert(name, "", cStor))
	{}
	CParam(const char* name, const T& defaultvalue)
		: cStor(), cData(convert(name, defaultvalue, cStor))
	{}
	CParam(const char* name, const char* defaultvalue)
		: cStor(), cData(convert(name, defaultvalue, cStor))
	{}
	virtual ~CParam()
	{}
	///////////////////////////////////////////////////////////////////////////
	// direct access on the parameter value
	const T& operator=(const T& a)
	{
		if( a != cData )
		{
			CParamStorage &stor = CParamStorage::getParam(cStor);
			stor.cTime = gettick();
			cData = a; 
		}
		return a; 
	}
	operator const T&()	{ return cData; }

	const char*name()	{ return cStor; }
	///////////////////////////////////////////////////////////////////////////
	// check and set parameter modification
	bool isModified()
	{
		CParamStorage &stor = CParamStorage::getParam(cStor);
		return (cStor.cTime != stor.cTime);
	}
	bool adjusted()
	{
		CParamStorage &stor = CParamStorage::getParam(cStor);
		bool ret = (cStor.cTime != stor.cTime);
		cStor.cTime = stor.cTime;
		return ret;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// determination and conversion of the stored parameter
	static T& convert(const char* name, const T& value, CParamStorage &basestor);
	static T& convert(const char* name, const char* value, CParamStorage &basestor);
	//////////////////////////////////////////////////////////////////////////
	// create a new variable / overwrite the content of an existing
	static void create(const char* name, const T& value);

	friend void createParam(const char* name, const char* value);
	friend void CParamStorage::create(const char* name, const char* value);

};

inline void createParam(const char* name, const char* value)
{
	CParam<MiniString>::create(name, value);
}
inline void CParamStorage::create(const char* name, const char* value)
{
	CParam<MiniString>::create(name, value);
}





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
	virtual bool searchAccount(uint32 accid, CLoginAccount&account)	{ return db->searchAccount(accid, account); }
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account)	{ return db->insertAccount(userid, passwd, sex, email, account); }
	virtual bool removeAccount(uint32 accid)	{ return db->removeAccount(accid); }
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
	virtual bool searchChar(uint32 charid, CCharCharacter&character) =0;
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data) =0;
	virtual bool removeChar(uint32 charid) =0;
	virtual bool saveChar(const CCharCharacter& character) =0;

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account) =0;
	virtual bool saveAccount(CCharAccount& account) =0;
	virtual bool removeAccount(uint32 accid)=0;
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
	virtual bool searchChar(uint32 charid, CCharCharacter&data)	{ return db->searchChar(charid, data); }
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data)	{ return db->insertChar(account, name, str, agi, vit, int_, dex, luk, slot, hair_style, hair_color, data); }
	virtual bool removeChar(uint32 charid)	{ return db->removeChar(charid); }
	virtual bool saveChar(const CCharCharacter& data)	{ return db->saveChar(data); }

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account)	{ return db->searchAccount(accid, account); }
	virtual bool saveAccount(CCharAccount& account)	{ return db->saveAccount(account); }
	virtual bool removeAccount(uint32 accid)	{ return db->removeAccount(accid); }
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
class CGuildDBInterface : public global, public noncopyable
{
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDBInterface()				{}
	virtual ~CGuildDBInterface()	{}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()=0;
	virtual CGuild& operator[](size_t i)=0;

	virtual bool searchGuild(const char* name, CGuild& guild) =0;
	virtual bool searchGuild(uint32 guildid, CGuild& guild) =0;
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild) =0;
	virtual bool removeGuild(uint32 guildid) =0;
	virtual bool saveGuild(const CGuild& guild) =0;

	virtual bool searchCastle(ushort castleid, CCastle& castle) =0;
	virtual bool saveCastle(CCastle& castle) =0;
	virtual bool removeCastle(ushort castleid)=0;
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
	virtual size_t size()					{ return db->size(); }
	virtual CGuild& operator[](size_t i)	{ return (*db)[i]; }

	virtual bool searchGuild(const char* name, CGuild& guild)	{ return db->searchGuild(name, guild); }
	virtual bool searchGuild(uint32 guildid, CGuild& guild)	{ return db->searchGuild(guildid, guild); }
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild)	{ return db->insertGuild(member, name, guild); }
	virtual bool removeGuild(uint32 guildid)	{ return db->removeGuild(guildid); }
	virtual bool saveGuild(const CGuild& guild)	{ return db->saveGuild(guild); }

	virtual bool searchCastle(ushort cid, CCastle& castle)	{ return db->searchCastle(cid, castle); }
	virtual bool saveCastle(CCastle& castle)	{ return db->saveCastle(castle); }
	virtual bool removeCastle(ushort cid)	{ return db->removeCastle(cid); }
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
	bool isempty() 
	{
		int i;
		for(i = 0; i < MAX_PARTY; i++) {
			if (this->member[i].account_id > 0) {
				return false;
			}
		}
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Party Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPartyDBInterface : public global, public noncopyable
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
class CPCStorageDBInterface : public global, public noncopyable
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
class CGuildStorageDBInterface : public global, public noncopyable
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
	virtual bool saveStorage(const CPCStorage& stor)	{ saveStorage(stor); }
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
	virtual bool saveStorage(const CGuildStorage& stor)	{ saveStorage(stor); }
};



#endif//__IO_H__
