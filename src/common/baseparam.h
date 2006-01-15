#ifndef __PARAM_H__
#define __PARAM_H__

#include "base.h"
#include "showmsg.h"	// ShowMessage
#include "utils.h"		// safefopen
#include "socket.h"		// buffer iterator
#include "timer.h"		// timed config reload
#include "strlib.h"		// checktrim


//////////////////////////////////////////////////////////////////////////
// test function
void test_parameter();



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
//			delete_timer(cTimer, timercallback);
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
// command line parser
void parseCommandline(int argc, char **argv);


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
	virtual bool assign(const char*name, const char*s)		{ return false; }
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
			// checking file state and updating the locals at the same time
			bool isModified()
			{
				struct stat s;
				return ( 0==stat((const char*)(*this), &s) && s.st_mtime!=this->modtime && (this->modtime=s.st_mtime)==this->modtime );
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
			{
				if( cFileList[i].isModified() )
				{
					LoadConfig( cFileList[i] );
				}
			}
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
	// need a singleton here for allowing global parameters
	class CSingletonData : public Mutex
	{
	public:
		CParamLoader				cLoader;
		TslistDCT<CParamStorage>	cParams;
	};
	static CSingletonData &getSingletonData();

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
	static CParamStorage& getParam(const char* name);
	static bool existParam(const char* name);
    static Mutex& getMutex()	{ return getSingletonData(); }
	// clean unreferenced parameters
	static void clean();
	static void listall();
	static void create(const char* name, const char* value);
	static void loadFile(const char* name);
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
		bool (*cCallback)(const char* name, const X& newval, X& oldval);
		///////////////////////////////////////////////////////////////////////
		// construction
		CParamData(const char* value) : cCallback(NULL)
		{
			paramconvert(cData, value);
		}
		CParamData(const X& value) : cData(value), cCallback(NULL)
		{}
		///////////////////////////////////////////////////////////////////////
		// access
		// typeid needs #include <typeinfo> 
		// which is a bit stange among the different std implementations
		virtual const std::type_info& getType()	{ return typeid(X); }
		virtual bool assign(const char*name, const char*s)	
		{
			if(cCallback)
			{
				X tmp;
				paramconvert(tmp, s);
				if( cData != tmp && (*cCallback)(name, tmp, cData) )
					cData = tmp;
				return true;
			}
			else
				return paramconvert(cData, s); 
		}
		virtual bool assign(const char*name, const X&s)
		{
			if(cData != s)
			{
				if( !cCallback || (*cCallback)(name, s, cData) )
					cData = s;
				return true; 
			}
			return false; 
		}
		virtual void print()
		{
			//printf("type: %s, value='%s'", typeid(X).name(), (const char*)MiniString(cData));
		}
	};

	///////////////////////////////////////////////////////////////////////////
	// parameter storage
	CParamStorage		cStor;	// a copy of the storage to keep the smart pointers alive
	T&					cData;	// a reference to the data for direct access
public:
	///////////////////////////////////////////////////////////////////////////
	// construction/destruction (can use default copy/assign here)
	CParam(const char* name, bool (*callback)(const char* name, const T& newval, T& oldval)=NULL)
		: cStor(), cData(convert(name, "", cStor, callback))
	{}
	CParam(const char* name, const T& defaultvalue, bool (*callback)(const char* name, const T& newval, T& oldval)=NULL)
		: cStor(), cData(convert(name, defaultvalue, cStor, callback))
	{}
	CParam(const char* name, const char* defaultvalue, bool (*callback)(const char* name, const T& newval, T& oldval)=NULL)
		: cStor(), cData(convert(name, defaultvalue, cStor, callback))
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
			CParamData<T>* tmp = dynamic_cast< CParamData<T>* >( (class CParamBase*)stor.cParam.get() );
			if(tmp)
			{
				tmp->assign(cStor, a);
				stor.cTime = GetTickCount();
			}
		}
		return a; 
	}
	operator const T&() const	{ return cData; }
	const T& value() const		{ return cData; }
	const char*name() const		{ return cStor; }
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
	static T& convert(const char* name, const T& value, CParamStorage &basestor, bool (*callback)(const char* name, const T& newval, T& oldval))
	{
		ScopeLock sl(CParamStorage::getMutex());
		// get a reference to the parameter
		CParamStorage &stor = CParamStorage::getParam(name);
		CParamData<T>* tmp=NULL;

		if( !stor.cParam.exists() )
		{	// there is no data pointer
			// create one
			stor.cParam = tmp = new CParamData<T>(value);
		}
		else if( stor.cParam->getType() == typeid(T) )
		{	// data has same type as requested, so can use it directly
			tmp = dynamic_cast< CParamData<T>* >( (class CParamBase*)stor.cParam.get() );
			// dynamic_cast is here because of my paranoia
		}
		else if( stor.cParam->getType() == typeid(MiniString) )
		{	// otherwise we only accept MiniString to convert the data
			//CParamData<MiniString> *old = dynamic_cast< CParamData<MiniString>* >( stor.cParam.operator->() );
			// dynamic cast does not work here even if actually should, just hard cast it then
			CParamData<MiniString> *old = (CParamData<MiniString>*)stor.cParam.get();
			if( !old )
				throw exception("Params: data conversion wrong type");
			stor.cParam = tmp = new CParamData<T>(old->cData);
			// this creation will change the pointer in the database
			// and disconnect all existing references to this node
		}
		if(!tmp) throw exception("Params: data conversion failed");
		// return a reference to the data and copy the storage
		tmp->cCallback = (callback) ? callback : NULL;
		basestor = stor;
		basestor.cTime++;
		tmp->setReference();
		return tmp->cData;
	}
	static T& convert(const char* name, const char* value, CParamStorage &basestor, bool (*callback)(const char* name, const T& newval, T& oldval))
	{
		ScopeLock sl(CParamStorage::getMutex());
		// get a reference to the parameter
		CParamStorage &stor = CParamStorage::getParam(name);
		CParamData<T>* tmp=NULL;
		if( !stor.cParam.exists() )
		{	// there is no data pointer
			// create one
			stor.cParam = tmp = new CParamData<T>(value);
		}
		else if( stor.cParam->getType() == typeid(T) )
		{	// data has same type as requested, so can use it directly
			tmp = dynamic_cast< CParamData<T>* >( (class CParamBase*)stor.cParam.get() );
			// dynamic_cast is here because of my paranoia
		}
		else if( stor.cParam->getType() == typeid(MiniString) )
		{	// otherwise we only accept MiniString to convert the data
			//CParamData<MiniString> *old = dynamic_cast< CParamData<MiniString>* >( stor.cParam.operator->() );
			CParamData<MiniString> *old = (CParamData<MiniString>*)stor.cParam.get();
			if( !old )
				throw exception("Params: data conversion wrong type");
			stor.cParam = tmp = new CParamData<T>(old->cData);
			// this creation will change the pointer in the database
			// and disconnect all existing references to this node
		}
		if(!tmp) throw exception("Params: data conversion failed");
		tmp->cCallback = (callback) ? callback : NULL;
		// return a reference to the data and copy the storage
		basestor = stor;
		basestor.cTime++;
		tmp->setReference();
		return tmp->cData;
	}
	//////////////////////////////////////////////////////////////////////////
	// create a new variable / overwrite the content of an existing
	static void create(const char* name, const T& value)
	{
		ScopeLock sl(CParamStorage::getMutex());
		// get a reference to the parameter
		CParamStorage &stor = CParamStorage::getParam(name);
		if( !stor.cParam.exists() )
		{	// there is no data pointer
			// create one
			stor.cParam = (CParamBase*)new CParamData<T>(value);
		}
		else if( stor.cParam->getType() == typeid(T) )
		{	// data is of same type and can be used directly
			CParamData<T>* tmp = dynamic_cast< CParamData<T>* >( (class CParamBase*)stor.cParam.get() );
			if( tmp->cData != value )
			{
				stor.cParam->assign(name, value);
				stor.cTime = GetTickCount();
			}
		}
		else
		{	// otherwise assign the new value
			if( stor.cParam->assign(name, value) )
				stor.cTime = GetTickCount();
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// check if a variable exists
	static bool exist(const char* name)
	{
		ScopeLock sl(CParamStorage::getMutex());
		// get a reference to the parameter
		return CParamStorage::existParam(name);
	}
	friend void CParamStorage::create(const char* name, const char* value);
	friend bool existParam(const char* name);
	friend void createParam(const char* name, const char* value);
};

inline bool existParam(const char* name)
{
	return CParam< MiniString >::exist(name);
}
inline void createParam(const char* name, const char* value)
{
	CParam< MiniString >::create(name, value);
}
inline void CParamStorage::create(const char* name, const char* value)
{
	CParam< MiniString >::create(name, value);
}



#endif//__PARAM_H__
