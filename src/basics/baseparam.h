#ifndef __PARAM_H__
#define __PARAM_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basestring.h"
#include "basesync.h"
#include "baseministring.h"


NAMESPACE_BEGIN(basics)

//////////////////////////////////////////////////////////////////////////
// switch for using typeinfo or linked-list parameters
//#define PARAM_RTTI

//////////////////////////////////////////////////////////////////////////
/// test function.
void test_parameter(void);



///////////////////////////////////////////////////////////////////////////////
// Parameter class helpers
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// command line parser.
void parseCommandline(int argc, char **argv);





///////////////////////////////////////////////////////////////////////////////
/// predefined conversion functions for common data types.
bool paramconvert(sint64 &t, const char* str);
bool paramconvert(uint64 &t, const char* str);
bool paramconvert(double &t, const char* s);

inline bool paramconvert( long           &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( unsigned long  &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( int            &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( unsigned int   &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( short          &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( unsigned short &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( char           &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( unsigned char  &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
inline bool paramconvert( bool           &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=(0!=val); return ret; }
inline bool paramconvert( float          &t, const char* s) { double val; bool ret=paramconvert(val, s); t=val; return ret; }

///////////////////////////////////////////////////////////////////////////////
/// template conversion for the rest.
/// usable types need an assignment operator of string<> or const char* 
template <typename T> inline bool paramconvert( T &t, const char* s)
{
	t = (s) ? s : "";
	return true;
}



///////////////////////////////////////////////////////////////////////////////
/// clean a string.
/// remove leading/trailing whitespaces, concatinate multiple whitespaces and
/// replace control chars with '_'
const char* config_clean(char *str);


///////////////////////////////////////////////////////////////////////////////
// parameter converter in function form
/// parameter converter with upper/lower bound.
template <typename T> inline T config_switch(const char *str, const T& defaultmin, const T& defaultmax)
{
	T val;
	paramconvert(val, str);
	return (val<=defaultmin)? defaultmin : (val>=defaultmax)? defaultmax : val;
}
/// parameter converter with default value.
template <typename T> inline T config_switch(const char *str, const T& defaultval)
{
	T val;
	return ( paramconvert(val, str) ) ? val : defaultval;
}
/// plain parameter converter.
template <typename T> inline T config_switch(const char *str)
{
	T val;
	paramconvert(val, str);
	return val;
}

template<> inline bool config_switch<bool>(const char *str, const bool& defaultval)
{
	int val;
	paramconvert(val, str);
	return (val==0 || val==1) ? val : defaultval;
}



///////////////////////////////////////////////////////////////////////////////
// predeclaration
class CParameterList;


///////////////////////////////////////////////////////////////////////////////
/// read only string/number conversion variant.
/// used for resolving target types on assignment
class CParameter
{
	// only the CParameterList is allowed to create
	friend class CParameterList;

	const char* cStr;

	/// private constructor.
	/// only friends can create
	CParameter(const char* str) : cStr(str)	{}
public:
	/// public destructor.
	~CParameter()	{}

	// explicit access on string
	const char*string() const		{ return this->cStr; }
	// explicit access on integer
	int64 integer() const			{ return config_switch<int64>(this->cStr); }
	// explicit access on floating point
	double floating() const			{ return config_switch<double>(this->cStr); }

	// type resolving on assign
	operator const char*() const	{ return this->string(); }
	operator bool() const			{ return this->integer(); }
	operator char() const			{ return this->integer(); }
	operator unsigned char() const	{ return this->integer(); }
	operator short() const			{ return this->integer(); }
	operator unsigned short() const	{ return this->integer(); }
	operator int() const			{ return this->integer(); }
	operator unsigned int() const	{ return this->integer(); }
	operator long() const			{ return this->integer(); }
	operator unsigned long() const	{ return this->integer(); }
	operator int64() const			{ return this->integer(); }
	operator uint64() const			{ return this->integer(); }
	operator double() const			{ return this->floating(); }
	operator float() const			{ return this->floating(); }

	// compare with string
	// only do lowercase compare
	int compare(const char* str) const
	{	
		const char*ipp = this->cStr;
		if(ipp && str)
		{
			while( *ipp && *str && basics::locase(*ipp) == basics::locase(*str) )
				++ipp, ++str;
			return basics::locase(*ipp) - basics::locase(*str);
		}
		return (str)?-*str:ipp?*ipp:0;
	}

	friend bool operator==(const CParameter& p, const char* str)	{ return 0==p.compare(str); }
	friend bool operator!=(const CParameter& p, const char* str)	{ return 0!=p.compare(str); }
	friend bool operator< (const CParameter& p, const char* str)	{ return 0< p.compare(str); }

	friend bool operator==(const char* str, const CParameter& p)	{ return 0==p.compare(str); }
	friend bool operator!=(const char* str, const CParameter& p)	{ return 0!=p.compare(str); }
	friend bool operator< (const char* str, const CParameter& p)	{ return 0> p.compare(str); }
};

///////////////////////////////////////////////////////////////////////////////
/// commandline 2 list conversion.
/// splits a string on whitespaces or comma 
/// with accepting doubled comma as emtpy value
/// but keeping quoted strings together (it just strips off the quotes)
/// and also does automatic number conversion on access
/// together with on/off/yes/no detection.
/// limited to fixed string size
class CParameterList
{
private:

	char cTemp[1024];
	char cBuf[1024];
	char *cParam[128];
	size_t cCnt;
public:

	/// constructor
	CParameterList() : cCnt(0)
	{
		*this->cTemp=0;
	}
	// using default destructor/copy/assing

	/// constructing constructor
	CParameterList(const char* line) : cCnt(0)			{ this->add_commandline(line); }
	/// constructing assignment
	const CParameterList& operator=(const char* line)	{ this->add_commandline(line); return *this; }

	/// add a new commandline
	bool add_commandline(const char* line);

	/// get the original line
	const char* line() const
	{
		return this->cTemp;
	}
	/// get parameter from index
	const CParameter operator[](int inx) const
	{
		return CParameter(this->string((size_t)inx));
	}
	/// get parameter from index
	const CParameter first() const
	{
		return CParameter(this->string(0));
	}
	/// get parameter from index
	const CParameter last() const
	{
		return CParameter(this->cCnt?this->string(this->cCnt-1):"");
	}
	/// get explicit string from index
	const char* string(size_t inx) const
	{
		return (inx<this->cCnt) ? this->cParam[inx] : "";
	}
	/// get explicit integer from index
	long integer(size_t inx) const
	{
		return (inx<this->cCnt) ? strtol(this->cParam[inx],NULL,0) : 0;
	}
	/// get number of parameters
	size_t size() const
	{
		return this->cCnt;
	}

	/// remove entries from list.
	void erase(size_t st=0, size_t num=1);
};














//////////////////////////////////////////////////////////////////////////
/// basic interface for reading configs from file.
//////////////////////////////////////////////////////////////////////////
class CConfig
{
public:
	CConfig(){}
	virtual ~CConfig(){}

	/// load and parse config file.
	virtual bool LoadConfig(const char* cfgName);
	// proccess a config entry.
	virtual bool ProcessConfig(const char*w1,const char*w2) = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// basic class for using the old way timers.
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
		this->timerfinalize();
	}
	/// initialisation
	bool init(unsigned long interval);
	/// user function
	virtual bool timeruserfunc(unsigned long tick) = 0;
	/// external calling from external timer implementation
	static int timercallback(int timer, unsigned long tick, int id, numptr data);
	void timerfinalize();
};






///////////////////////////////////////////////////////////////////////////////
/// external parameter file processing function.
typedef bool (*paramproc)(const char* name);



#ifdef PARAM_RTTI
///////////////////////////////////////////////////////////////////////////////
/// RTTI Parameter class.
/// for application independend parameter storage and distribution
/// reads in config files and holds the variables
///
/// this class uses a global parameter object containing the data
/// data is initialized with string type and on first access to the data 
/// the type is changed according to the requested type
/// if a following access uses a different type, an exception is thrown
/// a global callback can be added which is called
/// when the stored value about to change
/// needs activated RTTI
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// parameter object class.
/// completely inlined since very simple
class CParamObj
{
	///////////////////////////////////////////////////////////////////////////
	// class data
	bool cReferenced;	///< true when this parameter has been referenced
	bool cFixed;		///< true when not changeable from file loading (ie. when given via command line)
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CParamObj(bool fixed=false) : cReferenced(false), cFixed(fixed)	{}
	virtual ~CParamObj()											{}
	///////////////////////////////////////////////////////////////////////////
	// check/set the reference
	bool isReferenced()						{ return cReferenced; }
	bool setReference(bool val=true)		{ return (cReferenced=val); }
	///////////////////////////////////////////////////////////////////////////
	// check/set the fixed
	bool isFixed()							{ return cFixed; }
	bool setFixed(bool val=true)			{ return (cFixed=val); }
	///////////////////////////////////////////////////////////////////////////
	// access functions for overloading
	virtual const std::type_info& getType()	{ return typeid(CParamObj); }
	virtual bool assign(const char*name, const char*s)		{ return false; }
	virtual void print()					{ printf("value uninitialized"); }
	virtual string<> value()				{ return string<>(); }
};

///////////////////////////////////////////////////////////////////////////////
/// parameter storage.
/// stores parameters and their names as smart pointers in a sorted list
/// gives out references for changing parameters at their storage
/// needs external locking of the provided mutex for multithread environment
class CParamBase : public string<>
{
	///////////////////////////////////////////////////////////////////////////
	/// internal class for loading/storing/reloading config files.
	/// and automatically setting up of changed parameters
	class CParamLoader : private CConfig, private CTimerBase
	{
		///////////////////////////////////////////////////////////////////////
		/// stores information about a file
		class CFileData : public string<>
		{
			///////////////////////////////////////////////////////////////////
			/// class data
			time_t modtime;	///< last modification time of the file
		public:
			paramproc proc;	///< external processing function
			///////////////////////////////////////////////////////////////////
			/// construction/destruction
			CFileData() : proc(NULL)	{}
			CFileData(const string<>& name, paramproc p) : string<>(name), proc(p)
			{
				struct stat s;
				if( 0==stat(name, &s) )
					modtime = s.st_mtime;
			}
			~CFileData()	{}

			///////////////////////////////////////////////////////////////////
			/// checking file state and updating the locals at the same time
			bool isModified()
			{
				struct stat s;
				return ( 0==stat((const char*)(*this), &s) && s.st_mtime!=this->modtime && (this->modtime=s.st_mtime)==this->modtime );
			}
		};
		///////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////
		/// class data
		TslistDCT<CFileData>	cFileList;			///< list of loaded files
	public:
		CParamLoader() : CTimerBase(60*1000)		///< 1 minute interval for file checks
		{}
		/////////////////////////////////////////////////////////////
		/// timer callback
		virtual bool timeruserfunc(unsigned long tick);
		/////////////////////////////////////////////////////////////
		/// config processing callback
		virtual bool ProcessConfig(const char*w1,const char*w2)
		{	/// create/update parameter
			CParamBase::create(w1, w2);
			return true;
		}
		/////////////////////////////////////////////////////////////
		/// overload CConfig::LoadConfig. 
		/// this additionally includes the file to the watchlist
		virtual bool LoadConfig(const char* cfgName)
		{
			CConfig::LoadConfig( cfgName );
			cFileList.insert( CFileData(cfgName,NULL) );
			return true;
		}
		/////////////////////////////////////////////////////////////
		/// external access
		void loadFile(const string<>& filename, paramproc p);
	};

	///////////////////////////////////////////////////////////////////////////
	/// static data.
	/// need a singleton here for allowing global parameters
	class CSingletonData : public Mutex
	{
	public:
		CParamLoader		cLoader;
		slist<CParamBase>	cParams;
	};
	static CSingletonData &getSingletonData();

public:
	///////////////////////////////////////////////////////////////////////////
	/// class Data
	TPtrCount<CParamObj>	cParam;	///< pointer to the parameter data
	ulong					cTime;	///< time of last access

	///////////////////////////////////////////////////////////////////////////
	/// construction/destruction
	CParamBase()										{}
	CParamBase(const string<>& name) : string<>(name)	{}
	~CParamBase()									{}
	///////////////////////////////////////////////////////////////////////////
	/// class access
	void print()
	{
		printf("\nname: %-16s ", (const char*)(*this)); 
		cParam->print();
	}

	string<> name()		{ return *this; }
	string<> value()	{ return cParam->value(); }

	///////////////////////////////////////////////////////////////////////////
	/// static access functions

	/// return a parameterbase object
	static CParamBase& getParam(const string<>& name);
	/// check if parameter exists
	static bool existParam(const string<>& name);
	/// aquirethe storage mutex for syncronisation
    static Mutex& getMutex()	{ return getSingletonData(); }
	/// clean unreferenced parameters
	static void clean();
	/// list all parameters
	static void listall();
	/// list all parameters
	static string<> getlist();
	/// create a new parameter/overwrite an existing
	static void create(const string<>& name, const string<>& value, bool fixed=false);
	/// load parameters from file
	static void loadFile(const string<>& name, paramproc p=NULL);
};





///////////////////////////////////////////////////////////////////////////////
/// variable parameter template.
template <class T> class CParam
{
private:
	///////////////////////////////////////////////////////////////////////////
	/// internal parameter data template
	template <class X> class CParamData : public CParamObj
	{
	public:
		///////////////////////////////////////////////////////////////////////
		/// the actual parameter data
		X	cData;
		bool (*cCallback)(const string<>& name, X& newval, const X& oldval);
		///////////////////////////////////////////////////////////////////////
		/// construction
		CParamData(const char* value, bool fixed=false) : CParamObj(fixed), cCallback(NULL)
		{
			paramconvert(cData, value);
		}
		CParamData(const X& value, bool fixed=false) : CParamObj(fixed), cData(value), cCallback(NULL)
		{}
		///////////////////////////////////////////////////////////////////////
		/// access on the parameter

		/// returns typeid of the currently stored parameter
		/// typeid needs #include <typeinfo> 
		/// which is a bit stange among the different std implementations
		virtual const std::type_info& getType()	{ return typeid(X); }

		/// assign a string to a parameter
		virtual bool assign(const string<>& name, const char* s)	
		{
			X oldvalue = this->cData;
			if( paramconvert(this->cData, s) &&
				(!cCallback || cData == oldvalue || (*cCallback)(name, this->cData, oldvalue)) )
			{	// keep it
				return true;
			}
			else
			{	// rollback
				this->cData = oldvalue;
				return false;
			}
		}

		/// assign a value to a parameter
		virtual bool assign(const string<>& name, const X&value)
		{
			if( !(this->cData == value) )
			{
				X oldvalue = this->cData;
				this->cData = value;
				if( !cCallback || (*cCallback)(name, this->cData, oldvalue) )
				{	// keep the value
					return true; 
				}
				else
				{	// rollback
					this->cData = oldvalue;
				}
			}
			return false; 
		}
		/// debugprint a parameter to stdout
		virtual void print()
		{
			string<> str;
			str << cData;
			printf("value='%s' (shared %s)", (const char*)str, typeid(X).name());
		}
		virtual string<> value()				{ string<> str; str << this->cData; return str; }
	};

	///////////////////////////////////////////////////////////////////////////
	// parameter storage
	CParamBase		cStor;	///< a copy of the storage to keep the smart pointers alive
	T&				cData;	///< a reference to the data for direct access
public:
	///////////////////////////////////////////////////////////////////////////
	/// construction/destruction (can use default copy/assign here)
	CParam(const string<>& name, bool (*callback)(const string<>& name, T& newval, const T& oldval)=NULL)
		: cStor(), cData(convert(name, "", cStor, callback))
	{}
	CParam(const string<>& name, const T& defaultvalue, bool (*callback)(const string<>& name, T& newval, const T& oldval)=NULL)
		: cStor(), cData(convert(name, defaultvalue, cStor, callback))
	{}

	virtual ~CParam()
	{}
	///////////////////////////////////////////////////////////////////////////
	/// direct access on the parameter value
	const T& operator=(const T& a)
	{
		if( !(a == cData) )
		{
			CParamBase &stor = CParamBase::getParam(cStor);
			CParamData<T>* tmp = dynamic_cast< CParamData<T>* >( (class CParamObj*)stor.cParam.get() );
			if(tmp)
			{
				tmp->assign(cStor, a);
				stor.cTime = GetTickCount();
			}
		}
		return a; 
	}
	operator const T&() const		{ return cData; }
	const T& operator*() const		{ return cData; }
	const T* operator->() const		{ return &cData; }
	const T& operator()() const		{ return cData; }
	const T& value() const			{ return cData; }
	const char*name() const			{ return cStor; }


	///////////////////////////////////////////////////////////////////////////
	// compares parameters directly.
	// have all six compares
	template <typename X> bool operator==(const CParam<X>& p)
	{
		return this->cData == p();
	}
	template <typename X> bool operator!=(const CParam<X>& p)
	{
		return this->cData != p();
	}
	template <typename X> bool operator< (const CParam<X>& p)
	{
		return this->cData <  p();
	}
	template <typename X> bool operator<=(const CParam<X>& p)
	{
		return this->cData <= p();
	}
	template <typename X> bool operator> (const CParam<X>& p)
	{
		return this->cData >  p();
	}
	template <typename X> bool operator>=(const CParam<X>& p)
	{
		return this->cData >= p();
	}

	///////////////////////////////////////////////////////////////////////////
	/// check and set parameter modification
	bool isModified()
	{
		CParamBase &stor = CParamBase::getParam(cStor);
		return (cStor.cTime != stor.cTime);
	}
	///////////////////////////////////////////////////////////////////////////
	/// set parameter status
	bool adjusted()
	{
		CParamBase &stor = CParamBase::getParam(cStor);
		bool ret = (cStor.cTime != stor.cTime);
		cStor.cTime = stor.cTime;
		return ret;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// determination and conversion of the stored parameter
	/// calling with a converted value
	static T& convert(const string<>& name, const T& value, CParamBase &basestor, bool (*callback)(const string<>& name, T& newval, const T& oldval))
	{
		ScopeLock sl(CParamBase::getMutex());
		// get a reference to the parameter
		CParamBase &stor = CParamBase::getParam(name);
		CParamData<T>* tmp=NULL;

		if( !stor.cParam.exists() )
		{	// there is no data pointer
			// create one
			stor.cParam = tmp = new CParamData<T>(value);
		}
		else if( stor.cParam->getType() == typeid(T) )
		{	// data has same type as requested, so can use it directly
			tmp = dynamic_cast< CParamData<T>* >( (class CParamObj*)stor.cParam.get() );
			// dynamic_cast is here because of my paranoia
		}
		else if( stor.cParam->getType() == typeid(string<>) )
		{	// otherwise we only accept string<> to convert the data
			//CParamData< string<> > *old = dynamic_cast< CParamData< string<> >* >( stor.cParam.operator->() );
			// dynamic cast does not work here even if actually should, just hard cast it then
			CParamData< string<> > *old = (CParamData< string<> >*)stor.cParam.get();
			if( !old )
				throw exception("Params: data conversion wrong type");
			stor.cParam = tmp = new CParamData<T>(old->cData);
			// this creation will change the pointer in the database
			// and disconnect all existing references to this node
		}
		else if( stor.cParam->getType() == typeid(MiniString) )
		{	// otherwise we only accept MiniString to convert the data
			//CParamData< string<> > *old = dynamic_cast< CParamData< string<> >* >( stor.cParam.operator->() );
			CParamData< MiniString > *old = (CParamData< MiniString >*)stor.cParam.get();
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
	///////////////////////////////////////////////////////////////////////////
	/// determination and conversion of the stored parameter
	/// calling with a string
	static T& convert(const string<>& name, const char* value, CParamBase &basestor, bool (*callback)(const string<>& name, T& newval, const T& oldval))
	{
		ScopeLock sl(CParamBase::getMutex());
		// get a reference to the parameter
		CParamBase &stor = CParamBase::getParam(name);
		CParamData<T>* tmp=NULL;
		if( !stor.cParam.exists() )
		{	// there is no data pointer
			// create one
			stor.cParam = tmp = new CParamData<T>(value);
		}
		else if( stor.cParam->getType() == typeid(T) )
		{	// data has same type as requested, so can use it directly
			tmp = dynamic_cast< CParamData<T>* >( (class CParamObj*)stor.cParam.get() );
			// dynamic_cast is here because of my paranoia
		}
		else if( stor.cParam->getType() == typeid(string<>) )
		{	// otherwise we only accept string<> to convert the data
			//CParamData< string<> > *old = dynamic_cast< CParamData< string<> >* >( stor.cParam.operator->() );
			CParamData< string<> > *old = (CParamData< string<> >*)stor.cParam.get();
			if( !old )
				throw exception("Params: data conversion wrong type");
			stor.cParam = tmp = new CParamData<T>(old->cData);
			// this creation will change the pointer in the database
			// and disconnect all existing references to this node
		}
		else if( stor.cParam->getType() == typeid(MiniString) )
		{	// otherwise we only accept MiniString to convert the data
			//CParamData< string<> > *old = dynamic_cast< CParamData< string<> >* >( stor.cParam.operator->() );
			CParamData< MiniString > *old = (CParamData< MiniString >*)stor.cParam.get();
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
	///////////////////////////////////////////////////////////////////////////
	/// create a new variable / overwrite the content of an existing
	static void create(const string<>& name, const T& value, bool fixed=false)
	{
		ScopeLock sl(CParamBase::getMutex());
		// get a reference to the parameter
		CParamBase &stor = CParamBase::getParam(name);
		if( !stor.cParam.exists() )
		{	// there is no data pointer
			// create one
			stor.cParam = (CParamObj*)new CParamData<T>(value,fixed);
		}
		else if( stor.cParam->isFixed() )
		{	// don't overwrite fixed parameters with creation command
			// maybe print a warning
			printf("parameter '%s' is fixed, ignoring new assignment\n", (const char*)name);
		}
		else if( stor.cParam->getType() == typeid(T) )
		{	// data is of same type and can be used directly
			CParamData<T>* tmp = dynamic_cast< CParamData<T>* >( (class CParamObj*)stor.cParam.get() );
			if( !(tmp->cData == value) )
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
	///////////////////////////////////////////////////////////////////////////
	/// check if a variable exists
	static bool exist(const string<>& name)
	{
		ScopeLock sl(CParamBase::getMutex());
		// get a reference to the parameter
		return CParamBase::existParam(name);
	}

	friend void CParamBase::create(const string<>& name, const string<>& value, bool fixed);
	friend bool existParam(const string<>& name);
	friend void createParam(const string<>& name, const string<>& value, bool fixed);
};


inline void CParamBase::create(const string<>& name, const string<>& value, bool fixed)
{
	CParam< string<> >::create(name, value, fixed);
}
///////////////////////////////////////////////////////////////////////////
/// load parameters from file
inline void loadParam(const string<>& filename, paramproc p=NULL)
{
	CParamBase::loadFile(filename,p);
}
///////////////////////////////////////////////////////////////////////////////
/// clean unused parameters
inline void cleanParam()
{
	CParamBase::clean();
}
///////////////////////////////////////////////////////////////////////////////
/// create/overwrite parameter
inline void createParam(const string<>& name, const string<>& value, bool fixed=false)
{
	CParam< string<> >::create(name, value, fixed);
}
///////////////////////////////////////////////////////////////////////////////
/// check if parameter exist
inline bool existParam(const string<>& name)
{
	return CParam< string<> >::exist(name);
}
///////////////////////////////////////////////////////////////////////////////
/// check for parameter and create if not exist
inline bool existParam(const string<>& name, const string<>& value)
{
	if( !existParam(name) )
		createParam(name, value);
	return true;
}




#else
///////////////////////////////////////////////////////////////////////////////
/// parameter class without using typeid
/// for application independend parameter storage and distribution
/// reads in config files and holds the variables
/// the parameter storage object stored the data as string 
/// and keeps this all the time
/// whenever a CParam Object links to the storage, the local typed data of
/// the CParam is initialized with the string, 
/// thus allowing different parameter types connecting to the same storage
/// global callbacks are not type-aware, but each CParam can have its
/// own callback which however is connected to the CParam object livetime
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// predeclaration
class CParamBase;
class CParamStorage;

///////////////////////////////////////////////////////////////////////////////
/// parameter object. is the actual storage object
/// name/value pairs are stored as strings, 
/// als time of last modification and a reference boolean
/// all currently linked CParam objects are linked on a single-linked-list at cParamRoot 
class CParamObj : public string<>
{
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class CParamBase;
	friend class CParamStorage;

	///////////////////////////////////////////////////////////////////////////
	/// class data
	/// class itself behaves like a string and represents the name
	string<>		cValue;			///< assigned value as string
	ulong			cTime;			///< last modification time as tickcount
	CParamBase*		cParamRoot;		///< start of the currently linked CParam's
	bool			cReferenced;	///< true when this object has been referenced
	bool			cFixed;			///< true when not changeable from file loading (ie. when given via command line)
	bool (*cCallback)(const string<>& name, string<>& newval, const string<>& oldval);
public:
	///////////////////////////////////////////////////////////////////////////
	/// construction/destruction
	CParamObj() : cParamRoot(NULL), cReferenced(false),cFixed(false),cCallback(NULL)	{}
	CParamObj(const string<>& name) : string<>(name), cParamRoot(NULL), cReferenced(false),cFixed(false),cCallback(NULL)	{}
	~CParamObj()	{}


	///////////////////////////////////////////////////////////////////////////
	// check/set the reference
	bool isReferenced()						{ return cReferenced; }
	bool setReference(bool val=true)		{ return (cReferenced=val); }

	///////////////////////////////////////////////////////////////////////////
	// check/set the fixed
	bool isFixed()							{ return cFixed; }
	bool setFixed(bool val=true)			{ return (cFixed=val); }

	///////////////////////////////////////////////////////////////////////////
	/// class access
	string<> name()		{ return *this; }
	string<> value()	{ return cValue; }

	///////////////////////////////////////////////////////////////////////////
	/// assign a new value
	void assignvalue(const string<>& value);
	///////////////////////////////////////////////////////////////////////////
	/// debug print
	void print();
};

///////////////////////////////////////////////////////////////////////////////
/// parameter base. contains a static wrapper to the parameter storage
/// and the basic functionality for parameter linking
class CParamBase
{
	///////////////////////////////////////////////////////////////////////////
	/// internal class for loading/storing/reloading config files.
	/// and automatically setting up of changed parameters
	class CParamLoader : private CConfig, private CTimerBase
	{
		///////////////////////////////////////////////////////////////////////
		/// stores information about a file
		class CFileData : public string<>
		{
			///////////////////////////////////////////////////////////////////
			// class data
			time_t		modtime;	///< last modification time of the file
		public:
			paramproc	proc;		///< alternative processing function

			///////////////////////////////////////////////////////////////////
			/// construction/destruction
			CFileData() : proc(NULL)	{}
			CFileData(const string<>& filename, paramproc p=NULL);
			~CFileData()	{}

			///////////////////////////////////////////////////////////////////
			/// checking file state and updating the locals at the same time
			bool isModified();
		};
		///////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////
		// class data
		slist<CFileData>	cFileList;				///< list of loaded files
	public:
		CParamLoader() : CTimerBase(60*1000)		///< 1 minute interval for file checks
		{}
		/////////////////////////////////////////////////////////////
		/// timer callback
		virtual bool timeruserfunc(unsigned long tick);
		/////////////////////////////////////////////////////////////
		/// config processing callback
		virtual bool ProcessConfig(const char*w1,const char*w2);
		/////////////////////////////////////////////////////////////
		/// overload CConfig::LoadConfig. 
		/// this additionally includes the file to the watchlist
		virtual bool LoadConfig(const char* cfgName)
		{
			this->loadFile(cfgName);
			return true;
		}
		/////////////////////////////////////////////////////////////
		/// load a file into the parameter manager
		void loadFile(const string<>& filename, paramproc p=NULL);
	};

	///////////////////////////////////////////////////////////////////////////
	/// static data
	/// need a singleton here for allowing global parameters
	class CSingletonData : public Mutex
	{
	public:
		CParamLoader						cLoader;
		slist< TObjPtrCount<CParamObj> >	cParams;
	};
	static CSingletonData &getSingletonData();

public:
	///////////////////////////////////////////////////////////////////////////
	// static access functions
	static TObjPtrCount<CParamObj> getParam(const string<>& name, const string<>& value);
	static TObjPtr<CParamObj> createParam(const string<>& name, const string<>& value, bool fixed=false);
	static bool existParam(const string<>& name);
	static void clean();
	static void listall();
	static string<> getlist();
	static void loadFile(const string<>& filename, paramproc p=NULL);


	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class CParamObj;
protected:

	///////////////////////////////////////////////////////////////////////////
	/// Class Data
	ulong					cTime;		///< time of local access
	TObjPtrCount<CParamObj>	cParamObj;	///< a pointer to the parameter object
	CParamBase*				cNext;		///< next element in link-list
	///////////////////////////////////////////////////////////////////////////
	/// link list functions
	void link();
	void unlink();
public:
	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	CParamBase() : cNext(NULL)
	{}
	virtual ~CParamBase()
	{
		unlink();
	}

	///////////////////////////////////////////////////////////////////////////
	/// access to the parameter object content
	string<> name() const
	{
		return  ( this->cParamObj.exists() ) ? this->cParamObj : string<>();
	}
	///////////////////////////////////////////////////////////////////////////
	/// access to the parameter object content
	string<> value() const
	{
		return  ( this->cParamObj.exists() ) ? this->cParamObj->cValue : string<>();
	}
	///////////////////////////////////////////////////////////////////////////
	/// access to the parameter object content
	void set_global_callback( bool (*callback)(const string<>& name, string<>& newval, const string<>& oldval))
	{
		cParamObj->cCallback = callback;
	}
protected:
	///////////////////////////////////////////////////////////////////////////
	/// function which is called on external parameter modification
	/// either by assigning values or changing an external file
	virtual void call(const string<>& newval)=0;
};





///////////////////////////////////////////////////////////////////////////////
/// the actual parameter template.
template <typename T> class CParam : public CParamBase
{
	///////////////////////////////////////////////////////////////////////////
	// class data
	T cValue;
	bool (*cLocalCallback)(const string<>& name, T& newval, const T& oldval);
public:

	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	CParam(const string<>& name, bool (*callback)(const string<>& name, T& newval, const T& oldval)=NULL)
		: cLocalCallback(callback)
	{
		string<> tmp;
		this->cParamObj = CParamBase::getParam(name, tmp);
		this->link();
	}
	CParam(const string<>& name, const T& value, bool (*callback)(const string<>& name, T& newval, const T& oldval)=NULL)
		: cLocalCallback(callback)
	{
		string<> tmp;
		tmp << value;
		this->cParamObj = CParamBase::getParam(name, tmp);
		this->link();
		paramconvert(cValue, this->cParamObj->value() );
	}
	virtual ~CParam()
	{ }

	///////////////////////////////////////////////////////////////////////////
	/// copy construction/assignment
	CParam(const CParam<T>& p)
		: CParamBase(p), cValue(p.cValue), cLocalCallback(p.cLocalCallback)
	{
		this->link();
	}
	const CParam& operator=(const CParam<T>& p)
	{
		this->unlink();
		this->cParamObj = p.cParamObj;
		this->cValue = p.cValue;
		this->cLocalCallback = p.callback;
		this->link();
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// class access
	operator const T&() const	{ return this->cValue; }
	const T& operator*() const	{ return this->cValue; }
	const T* operator->() const	{ return &this->cValue; }
	const T& operator()() const	{ return this->cValue; }
	const T& value() const		{ return this->cValue; }
	const char*name() const		{ return this->cParamObj->name(); }

	const T& operator=(const T& value)
	{	
		if( !(value==this->cValue) )
		{	// take over the new value but store the old one
			const T oldvalue = this->cValue;
			this->cValue = value;
			// check the callback
			if( !cLocalCallback || cLocalCallback(*cParamObj, this->cValue, oldvalue) )
			{	// set the value into the global object
				string<> tmpstr;
				tmpstr << this->cValue;
				this->cParamObj->assignvalue(tmpstr);
			}
			else
			{	// do rollback
				this->cValue = oldvalue;
			}
		}
		return value; 
	}

	///////////////////////////////////////////////////////////////////////////
	// compares parameters directly.
	// have all six compares
	template <typename X> bool operator==(const CParam<X>& p)
	{
		return this->cValue == p();
	}
	template <typename X> bool operator!=(const CParam<X>& p)
	{
		return this->cValue != p();
	}
	template <typename X> bool operator< (const CParam<X>& p)
	{
		return this->cValue <  p();
	}
	template <typename X> bool operator<=(const CParam<X>& p)
	{
		return this->cValue <= p();
	}
	template <typename X> bool operator> (const CParam<X>& p)
	{
		return this->cValue >  p();
	}
	template <typename X> bool operator>=(const CParam<X>& p)
	{
		return this->cValue >= p();
	}


protected:
	///////////////////////////////////////////////////////////////////////////
	/// function which is called on external parameter modification
	virtual void call(const string<>& newval)
	{
		T value;
		paramconvert(value, newval);
		if( !(value==this->cValue) )
		{	// take over the new value but store the old one
			const T oldvalue = this->cValue;
			this->cValue = value;
			// check the callback
			if( !cLocalCallback || cLocalCallback(*cParamObj, this->cValue, oldvalue) )
			{	// keep the new value
			}
			else
			{	// do rollback
				this->cValue = oldvalue;
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////
/// load parameters from file
inline void loadParam(const string<>& filename, paramproc p=NULL)
{
	CParamBase::loadFile(filename, p);
}
///////////////////////////////////////////////////////////////////////////
/// clean unused parameters
inline void cleanParam()
{
	CParamBase::clean();
}
///////////////////////////////////////////////////////////////////////////
/// create/update parameter
inline void createParam(const string<>& name, const string<>& value, bool fixed=false)
{
	CParamBase::createParam(name, value, fixed);
}
///////////////////////////////////////////////////////////////////////////
/// check if parameter exist
inline bool existParam(const string<>& name)
{
	return CParamBase::existParam(name);
}
///////////////////////////////////////////////////////////////////////////
/// check if parameter exist and create if not
inline bool existParam(const string<>& name, const string<>& value)
{
	CParamBase::getParam(name,value);
	return true;
}



#endif//PARAM_RTTI


///////////////////////////////////////////////////////////////////////////
// compares parameters with content type, both sides
template <typename T1, typename T2> bool operator==(const T1& t1, const CParam<T2>& p2)
{
	return t1 == p2();
}
template <typename T1, typename T2> bool operator!=(const T1& t1, const CParam<T2>& p2)
{
	return t1 != p2();
}
template <typename T1, typename T2> bool operator< (const T1& t1, const CParam<T2>& p2)
{
	return t1 <  p2();
}
template <typename T1, typename T2> bool operator<=(const T1& t1, const CParam<T2>& p2)
{
	return t1 <= p2();
}
template <typename T1, typename T2> bool operator> (const T1& t1, const CParam<T2>& p2)
{
	return t1 >  p2();
}
template <typename T1, typename T2> bool operator>=(const T1& t1, const CParam<T2>& p2)
{
	return t1 >= p2();
}


template <typename T1, typename T2> bool operator==(const CParam<T1>& p1, const T2& t2)
{
	return p1() == t2;
}
template <typename T1, typename T2> bool operator!=(const CParam<T1>& p1, const T2& t2)
{
	return p1() != t2;
}
template <typename T1, typename T2> bool operator< (const CParam<T1>& p1, const T2& t2)
{
	return p1() <  t2;
}
template <typename T1, typename T2> bool operator<=(const CParam<T1>& p1, const T2& t2)
{
	return p1() <= t2;
}
template <typename T1, typename T2> bool operator> (const CParam<T1>& p1, const T2& t2)
{
	return p1() >  t2;
}
template <typename T1, typename T2> bool operator>=(const CParam<T1>& p1, const T2& t2)
{
	return p1() >= t2;
}


///////////////////////////////////////////////////////////////////////////
// string concatinate
template <typename T> stringoperator<>& operator << (stringoperator<>& str, const CParam<T>& param)
{
	str << (const T&)param;
	return str;
}
template <typename T> string<>& operator << (string<>& str, const CParam<T>& param)
{
	(*str) << (const T&)param;
	return str;
}


NAMESPACE_END(basics)



#endif//__PARAM_H__
