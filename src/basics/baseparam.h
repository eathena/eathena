#ifndef __PARAM_H__
#define __PARAM_H__

#include "basetypes.h"
#include "baseobjects.h"
#include "basestring.h"
#include "basesync.h"
#include "basetimerhandler.h"

NAMESPACE_BEGIN(basics)


///////////////////////////////////////////////////////////////////////////////
/// test function.
void test_parameter(void);



///////////////////////////////////////////////////////////////////////////////
// Parameter class helpers
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// command line parser.
void parseCommandline(int argc, char **argv);



///////////////////////////////////////////////////////////////////////////////
#if defined (TEMPLATE_NO_PARTIAL_SPECIALIZATION)
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// type that allows any type converted to it.
/// compiler is using this type when no other is available
struct param_any_conversion : noncopyable
{
	template <typename T> param_any_conversion(const volatile T&)
	{}
	template <typename T> param_any_conversion(T&)
	{}
};

///////////////////////////////////////////////////////////////////////////////
/// vector conversion fallback.
/// when T is a vector the call will go here
template <typename T>
inline paramconvert_vector(vector<T>& t, vector<T>& vec, const char* s)
{
	vector< string<> > strvec = string<>(s).split(',');
	vector< string<> >::iterator iter(strvec);
	T tmp;
	for(t.clear(); iter; ++iter)
	{
		if( paramconvert(tmp, *iter) )
			t.push_back(tmp);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// vector conversion fallback.
/// when T is not a vector the call will end up here
template <typename T>
inline paramconvert_vector(param_any_conversion, T& t, const char* s)
{
	t = (s) ? s : "";
}
///////////////////////////////////////////////////////////////////////////////
/// template conversion.
/// usable types need an assignment operator of string<> or const char* 
template <typename T>
inline bool paramconvert( T &t, const char* s)
{
	paramconvert_vector(t,t,s);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
#else//!defined (TEMPLATE_NO_PARTIAL_SPECIALIZATION)
///////////////////////////////////////////////////////////////////////////////
/// template conversion.
/// usable types need an assignment operator of string<> or const char* 
template <typename T>
inline bool paramconvert( T &t, const char* s)
{
	t = (s) ? s : "";
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// template conversion for vectors.
/// just partially specialize
template <typename T>
inline bool paramconvert( vector<T> &t, const char* s)
{
	vector< string<> > strvec = string<>(s).split(',');
	vector< string<> >::iterator iter(strvec);
	T tmp;
	for(t.clear(); iter; ++iter)
	{
		if( paramconvert(tmp, *iter) )
			t.push_back(tmp);
	}
	return true;
}
#endif
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// predefined conversion functions for common data types.
template<> bool paramconvert<sint64>(sint64 &t, const char* str);
template<> bool paramconvert<uint64>(uint64 &t, const char* str);
template<> bool paramconvert<double>(double &t, const char* s);

template<> inline bool paramconvert<long          >( long           &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<unsigned long >( unsigned long  &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<int           >( int            &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<unsigned int  >( unsigned int   &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<short         >( short          &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<unsigned short>( unsigned short &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<char          >( char           &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<unsigned char >( unsigned char  &t, const char* s) { uint64 val; bool ret=paramconvert(val, s); t=val; return ret; }
template<> inline bool paramconvert<bool          >( bool           &t, const char* s) { sint64 val; bool ret=paramconvert(val, s); t=(0!=val); return ret; }
template<> inline bool paramconvert<float         >( float          &t, const char* s) { double val; bool ret=paramconvert(val, s); t=(float)val; return ret; }



///////////////////////////////////////////////////////////////////////////////
/// clean a string.
/// remove leading/trailing whitespaces, concatinate multiple whitespaces and
/// replace control chars with '_'
const char* config_clean(char *str);


///////////////////////////////////////////////////////////////////////////////
// parameter converter in function form
/// parameter converter with upper/lower bound.
template <typename T>
inline T config_switch(const char *str, const T& defaultmin, const T& defaultmax)
{
	T val;
	paramconvert(val, str);
	return (val<=defaultmin)? defaultmin : (val>=defaultmax)? defaultmax : val;
}
/// parameter converter with default value.
template <typename T>
inline T config_switch(const char *str, const T& defaultval)
{
	T val;
	return ( paramconvert(val, str) ) ? val : defaultval;
}
/// plain parameter converter.
template <typename T>
inline T config_switch(const char *str)
{
	T val;
	paramconvert(val, str);
	return val;
}

template<>
inline bool config_switch<bool>(const char *str, const bool& defaultval)
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
	operator float() const			{ return (float)this->floating(); }

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
	/// proccess a config entry.
	virtual bool ProcessConfig(const char*w1,const char*w2) = 0;
};




///////////////////////////////////////////////////////////////////////////////
/// external parameter file processing function.
typedef bool (*paramfileproc)(const char* name);
///////////////////////////////////////////////////////////////////////////////
/// external parameter entry processing function.
typedef bool (*paramentrproc)(const char* name, const char* value);



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
	CParamObj() : string<>(), cParamRoot(NULL), cReferenced(false),cFixed(false),cCallback(NULL)	{}
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
	class CParamLoader;
	class CParamFile;

	///////////////////////////////////////////////////////////////////////////
	/// stores information about a parameter file
	class CParamFile : public string<>, private CConfig
	{
		///////////////////////////////////////////////////////////////////////
		// friends
		friend class CParamLoader;
		///////////////////////////////////////////////////////////////////////
		// class data
		time_t			modtime;	///< last modification time of the file
	public:
		paramfileproc	fileproc;	///< alternative processing function
		paramentrproc	entrproc;	///< alternative processing function

		///////////////////////////////////////////////////////////////////////
		/// construction/destruction
		CParamFile() : string<>(), CConfig(), fileproc(NULL), entrproc(NULL)	{}
		explicit CParamFile(const string<>& filename);
		CParamFile(const string<>& filename, paramfileproc p);
		CParamFile(const string<>& filename, paramentrproc p);
		CParamFile(const string<>& filename, const CParamFile& orig);
		~CParamFile()	{}

		///////////////////////////////////////////////////////////////////////
		/// default config processing callback.
		virtual bool ProcessConfig(const char*w1,const char*w2);
		///////////////////////////////////////////////////////////////////////
		/// overload CConfig::LoadConfig. 
		/// this additionally includes the file to the watchlist
		virtual bool LoadConfig(const char* cfgName)
		{
			CParamBase::loadParamFile( CParamFile(cfgName, *this) );
			return true;
		}
		///////////////////////////////////////////////////////////////////////
		/// 
		bool load()
		{
			return (this->fileproc) ? (*fileproc)(*this) : this->CConfig::LoadConfig(*this);
		}

		///////////////////////////////////////////////////////////////////////
		/// checking file state and updating the locals at the same time
		bool isModified();
	};
	
	///////////////////////////////////////////////////////////////////////////
	/// internal class for loading/storing/reloading config files.
	/// and automatically setting up of changed parameters
	class CParamLoader : private CTimerBase
	{
		///////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////
		// class data
		slist<CParamFile>	cFileList;			///< list of loaded files

	public:
		CParamLoader() : CTimerBase(60*1000)	///< 1 minute interval for file checks
		{}
		/////////////////////////////////////////////////////////////
		/// timer callback.
		virtual bool timeruserfunc(unsigned long tick);

		///////////////////////////////////////////////////////////////////////
		/// load a paramfile object.
		bool loadParamFile(const CParamFile& file);
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

	///////////////////////////////////////////////////////////////////////////
	/// load a file into the parameter manager.
	static bool loadParamFile(const CParamFile& fileobj);
	///////////////////////////////////////////////////////////////////////////
	static bool loadFile(const string<>& filename)
	{
		return CParamBase::loadParamFile( CParamFile(filename) );
	}
	static bool loadFile(const string<>& filename, paramfileproc p)
	{
		return CParamBase::loadParamFile( CParamFile(filename, p) );
	}
	static bool loadFile(const string<>& filename, paramentrproc p)
	{
		return CParamBase::loadParamFile( CParamFile(filename, p) );
	}
	///////////////////////////////////////////////////////////////////////////
	// friends
	friend class CParamObj;
protected:

	///////////////////////////////////////////////////////////////////////////
	/// Class Data
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
	virtual void update_value(const string<>& newval)=0;
};





///////////////////////////////////////////////////////////////////////////////
/// the actual parameter template.
template <typename T>
class CParam : public CParamBase
{
	///////////////////////////////////////////////////////////////////////////
	// class data
	T cValue;
	bool (*cLocalCallback)(const string<>& name, T& newval, const T& oldval);
public:

	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	CParam(const string<>& name, bool (*callback)(const string<>& name, T& newval, const T& oldval)=NULL);
	CParam(const string<>& name, const T& value, bool (*callback)(const string<>& name, T& newval, const T& oldval)=NULL);
	virtual ~CParam()
	{ }

	///////////////////////////////////////////////////////////////////////////
	/// copy construction/assignment
	CParam(const CParam<T>& p)
		: CParamBase(p), cValue(p.cValue), cLocalCallback(p.cLocalCallback)
	{
		this->link();
	}
	const CParam& operator=(const CParam<T>& p);

	///////////////////////////////////////////////////////////////////////////
	// class access
	operator const T&() const	{ return this->cValue; }
	const T& operator*() const	{ return this->cValue; }
	const T* operator->() const	{ return &this->cValue; }
	const T& operator()() const	{ return this->cValue; }
	const T& value() const		{ return this->cValue; }
	const char*name() const		{ return this->cParamObj->name(); }

	///////////////////////////////////////////////////////////////////////////
	/// value assignment.
	const T& operator=(const T& value);

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
	/// function which is called on external parameter modification.
	virtual void update_value(const string<>& newval);
};

///////////////////////////////////////////////////////////////////////////////
/// construction.
template <typename T>
CParam<T>::CParam(const string<>& name, bool (*callback)(const string<>& name, T& newval, const T& oldval))
	: CParamBase(), cLocalCallback(callback)
{
	string<> tmp;
	this->cParamObj = CParamBase::getParam(name, tmp);
	this->link();
	paramconvert(cValue, this->cParamObj->value() );
}

///////////////////////////////////////////////////////////////////////////////
/// construction.
template <typename T>
CParam<T>::CParam(const string<>& name, const T& value, bool (*callback)(const string<>& name, T& newval, const T& oldval))
	: CParamBase(), cLocalCallback(callback)
{
	string<> tmp;
	tmp << value;
	this->cParamObj = CParamBase::getParam(name, tmp);
	this->link();
	paramconvert(cValue, this->cParamObj->value() );
}

///////////////////////////////////////////////////////////////////////////////
/// assignment operator.
template <typename T>
const CParam<T>& CParam<T>::operator=(const CParam<T>& p)
{
	this->unlink();
	this->cParamObj = p.cParamObj;
	this->cValue = p.cValue;
	this->cLocalCallback = p.callback;
	this->link();
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
/// value assignment.
template <typename T>
const T& CParam<T>::operator=(const T& value)
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

///////////////////////////////////////////////////////////////////////////////
/// function which is called on external parameter modification
template <typename T>
void CParam<T>::update_value(const string<>& newval)
{
	if( !this->cLocalCallback )
	{	// no callback for value checking, 
		// just do the conversion
		paramconvert(this->cValue, newval);
	}
	else
	{	// have the local callback check the new value
		const T oldvalue = this->cValue;
		paramconvert(this->cValue, newval);
		if( !(oldvalue==this->cValue) )
		{	// check the callback
			if( !this->cLocalCallback(*cParamObj, this->cValue, oldvalue) )
			{	// rollback on error
				this->cValue = oldvalue;
			}
			// otherwise keep the new value
		}
	}
}



///////////////////////////////////////////////////////////////////////////////
/// load parameters from file.
inline bool loadParam(const string<>& filename)
{
	return CParamBase::loadFile(filename);
}
///////////////////////////////////////////////////////////////////////////////
/// load parameters from file.
/// with user defined file processing
inline bool loadParam(const string<>& filename, paramfileproc p)
{
	return CParamBase::loadFile(filename, p);
}
///////////////////////////////////////////////////////////////////////////////
/// load parameters from file.
/// with standard file processing but user defined entry processing
inline bool loadParam(const string<>& filename, paramentrproc p)
{
	return CParamBase::loadFile(filename, p);
}
///////////////////////////////////////////////////////////////////////////////
/// clean unused parameters
inline void cleanParam()
{
	CParamBase::clean();
}
///////////////////////////////////////////////////////////////////////////////
/// create/update parameter
inline void createParam(const string<>& name, const string<>& value, bool fixed=false)
{
	CParamBase::createParam(name, value, fixed);
}
///////////////////////////////////////////////////////////////////////////////
/// check if parameter exist
inline bool existParam(const string<>& name)
{
	return CParamBase::existParam(name);
}
///////////////////////////////////////////////////////////////////////////////
/// check if parameter exist and create if not
inline bool existParam(const string<>& name, const string<>& value)
{
	CParamBase::getParam(name,value);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// compares parameters with content type, both sides
template <typename T1, typename T2>
inline bool operator==(const T1& t1, const CParam<T2>& p2)
{
	return t1 == p2();
}
template <typename T1, typename T2>
inline bool operator!=(const T1& t1, const CParam<T2>& p2)
{
	return t1 != p2();
}
template <typename T1, typename T2>
inline bool operator< (const T1& t1, const CParam<T2>& p2)
{
	return t1 <  p2();
}
template <typename T1, typename T2>
inline bool operator<=(const T1& t1, const CParam<T2>& p2)
{
	return t1 <= p2();
}
template <typename T1, typename T2>
inline bool operator> (const T1& t1, const CParam<T2>& p2)
{
	return t1 >  p2();
}
template <typename T1, typename T2>
inline bool operator>=(const T1& t1, const CParam<T2>& p2)
{
	return t1 >= p2();
}

template <typename T1, typename T2>
inline bool operator==(const CParam<T1>& p1, const T2& t2)
{
	return p1() == t2;
}
template <typename T1, typename T2>
inline bool operator!=(const CParam<T1>& p1, const T2& t2)
{
	return p1() != t2;
}
template <typename T1, typename T2>
inline bool operator< (const CParam<T1>& p1, const T2& t2)
{
	return p1() <  t2;
}
template <typename T1, typename T2>
inline bool operator<=(const CParam<T1>& p1, const T2& t2)
{
	return p1() <= t2;
}
template <typename T1, typename T2>
inline bool operator> (const CParam<T1>& p1, const T2& t2)
{
	return p1() >  t2;
}
template <typename T1, typename T2>
inline bool operator>=(const CParam<T1>& p1, const T2& t2)
{
	return p1() >= t2;
}

///////////////////////////////////////////////////////////////////////////////
// string concatinate
template <typename T>
inline stringoperator<>& operator << (stringoperator<>& str, const CParam<T>& param)
{
	str << param();
	return str;
}
template <typename T>
inline  string<>& operator << (string<>& str, const CParam<T>& param)
{
	(*str) << param();
	return str;
}


NAMESPACE_END(basics)



#endif//__PARAM_H__
