#ifndef __PARAM_H__
#define __PARAM_H__


#include "base.h"
#include "showmsg.h"	// ShowMessage
#include "utils.h"		// safefopen
#include "socket.h"		// buffer iterator
#include "timer.h"		// timed config reload
#include "db.h"
#include "strlib.h"
#include "mmo.h"

//////////////////////////////////////////////////////////////////////////
//
// basic interface for reading configs from file
//
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
//
// basic class for using the old way timers
//
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
	bool init(unsigned long interval);
	// user function
	virtual bool timeruserfunc(unsigned long tick) = 0;
	// external calling from external timer implementation
	static int timercallback(int timer, unsigned long tick, int id, intptr data);
};



///////////////////////////////////////////////////////////////////////////////
// Parameter class
// for application independend parameter storage and distribution
// reads in config files and holds the variables
// needs activated RTTI
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// predefined conversion functions for common data types
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
// completely inlined since very simple
class CParamBase
{
	///////////////////////////////////////////////////////////////////////////
	// class data
	bool cReferenced;	// true when this parameter has been referenced
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CParamBase() : cReferenced(false)		{}
	virtual ~CParamBase()					{}
	///////////////////////////////////////////////////////////////////////////
	// check/set the reference
	bool isReferenced()						{ return cReferenced; }
	bool setReference()						{ return (cReferenced=true); }
	///////////////////////////////////////////////////////////////////////////
	// access functions for overloading
	virtual const std::type_info& getType()	{ return typeid(CParamBase); }
	virtual bool assign(const char*s)		{ return false; }
	virtual void print()					{ printf("value uninitialized"); }
};

///////////////////////////////////////////////////////////////////////////////
// parameter storage
// stores parameters and their names as smart pointers in a sorted list
// gives out references for changing parameters at their storage
// needs external locking of the provided mutex for multithread environment
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
		CParamLoader() : CTimerBase(60*1000)		// 1 minute interval for file checks
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
	CParamStorage()										{}
	CParamStorage(const char* name) : MiniString(name)	{}
	~CParamStorage()									{}
	///////////////////////////////////////////////////////////////////////////
	// class access
	void print()
	{
		printf("\nparameter name: '%s'", (const char*)(*this)); 
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
		ScopeLock sl(CParamStorage::getMutex());
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
		ScopeLock sl(CParamStorage::getMutex());
		size_t i;
		printf("\nList of Parameters (existing %i):", cParams.size());
		for(i=0; i<cParams.size(); i++)
			cParams[i].print();
		printf("\n---------------\n", cParams.size());

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



#endif//__PARAM_H__
