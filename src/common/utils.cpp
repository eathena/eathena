#include "utils.h"
#include "showmsg.h"
#include "malloc.h"
#include "mmo.h"

void dump(unsigned char *buffer, size_t num)
{
	size_t icnt,jcnt;
	ShowMessage("         Hex                                                  ASCII\n");
	ShowMessage("         -----------------------------------------------      ----------------");
	
	for (icnt=0;icnt<num;icnt+=16)
	{
		ShowMessage("\n%p ",&buffer[icnt]);
		for (jcnt=icnt;jcnt<icnt+16;++jcnt)
		{
			if (jcnt < num)
				ShowMessage("%02hX ",buffer[jcnt]);
			else
				ShowMessage("   ");
		}
		ShowMessage("  |  ");
		for (jcnt=icnt;jcnt<icnt+16;++jcnt)
		{
			if (jcnt < num)
			{
				if (buffer[jcnt] > 31 && buffer[jcnt] < 127)
					ShowMessage("%c",buffer[jcnt]);
				else
					ShowMessage(".");
			}
			else
				ShowMessage(" ");
		}
	}
	ShowMessage("\n");
}

// Allocate a StringBuf  [MouseJstr]
struct StringBuf * StringBuf_Malloc() 
{
	struct StringBuf * ret = (struct StringBuf *)aMalloc(sizeof(struct StringBuf));
	StringBuf_Init(ret);
	return ret;
}

// Initialize a previously allocated StringBuf [MouseJstr]
void StringBuf_Init(struct StringBuf * sbuf)  {
	sbuf->max_ = 1024;
	sbuf->ptr_ = sbuf->buf_ = (char *)aMalloc((sbuf->max_ + 1)*sizeof(char));
}

// printf into a StringBuf, moving the pointer [MouseJstr]
int StringBuf_Printf(struct StringBuf *sbuf,const char *fmt,...) 
{
	va_list ap;
	int n, size, off;
	
	
	while (1) {
		/* Try to print in the allocated space. */
		size = sbuf->max_ - (sbuf->ptr_ - sbuf->buf_);
		va_start(ap, fmt);
		n = vsnprintf (sbuf->ptr_, size, fmt, ap);
		va_end(ap);

		/* If that worked, return the length. */
		if (n > -1 && n < size) {
			sbuf->ptr_ += n;
			break;
		}
		/* Else try again with more space. */
		sbuf->max_ *= 2; // twice the old size
		off = sbuf->ptr_ - sbuf->buf_;
		sbuf->buf_ = (char *) aRealloc(sbuf->buf_, (sbuf->max_ + 1)*sizeof(char));
		sbuf->ptr_ = sbuf->buf_ + off;
	}
	
	return sbuf->ptr_ - sbuf->buf_;
}

// Append buf2 onto the end of buf1 [MouseJstr]
int StringBuf_Append(struct StringBuf *buf1,const struct StringBuf *buf2) 
{
	int buf1_avail = buf1->max_ - (buf1->ptr_ - buf1->buf_);
	int size2 = buf2->ptr_ - buf2->buf_;

	if (size2 >= buf1_avail)  {
		int off = buf1->ptr_ - buf1->buf_;
		buf1->max_ += size2;
		buf1->buf_ = (char *) aRealloc(buf1->buf_, (buf1->max_ + 1)*sizeof(char));
		buf1->ptr_ = buf1->buf_ + off;
	}

	memcpy(buf1->ptr_, buf2->buf_, size2);
	buf1->ptr_ += size2;
	return buf1->ptr_ - buf1->buf_;
}

// Destroy a StringBuf [MouseJstr]
void StringBuf_Destroy(struct StringBuf *sbuf) 
{
	aFree(sbuf->buf_);
	sbuf->ptr_ = sbuf->buf_ = 0;
}

// Free a StringBuf returned by StringBuf_Malloc [MouseJstr]
void StringBuf_Free(struct StringBuf *sbuf) 
{
	StringBuf_Destroy(sbuf);
	aFree(sbuf);
}

// Return the built string from the StringBuf [MouseJstr]
char * StringBuf_Value(struct StringBuf *sbuf) 
{
	*sbuf->ptr_ = '\0';
	return sbuf->buf_;
}


int config_switch(const char *str) {
	if (strcasecmp(str, "on") == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja") == 0 || strcasecmp(str, "si") == 0)
		return 1;
	if (strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0)
		return 0;

	return atoi(str);
}
///////////////////////////////////////////////////////////////////////////
// converts a string to an ip (host byte order)
uint32 str2ip(const char *str)
{
	struct hostent*h;
	while( isspace( ((unsigned char)(*str)) ) ) str++;
	h = gethostbyname(str);
	if (h != NULL)
		return ipaddress( MakeDWord((unsigned char)h->h_addr[3], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[0]) );
	else
		return ipaddress( ntohl(inet_addr(str)) );
}



//-----------------------------------------------------
// Function to suppress control characters in a string.
//-----------------------------------------------------
bool remove_control_chars(char *str)
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

//------------------------------------------------------------
// E-mail check: return 0 (not correct) or 1 (valid). by [Yor]
//------------------------------------------------------------
bool email_check(const char *email) {
	char ch;
	const char* last_arobas;

	// athena limits
	if (!email || strlen(email) < 3 || strlen(email) > 39)
		return false;

	// part of RFC limits (official reference of e-mail description)
	if (strchr(email, '@') == NULL || email[strlen(email)-1] == '@')
		return false;

	if (email[strlen(email)-1] == '.')
		return false;

	last_arobas = strrchr(email, '@');

	if (strstr(last_arobas, "@.") != NULL ||
	    strstr(last_arobas, "..") != NULL)
		return false;

	for(ch = 1; ch < 32; ch++) {
		if (strchr(last_arobas, ch) != NULL) {
			return false;
		}
	}

	if (strchr(last_arobas, ' ') != NULL ||
	    strchr(last_arobas, ';') != NULL)
		return false;

	// all correct
	return true;
}






#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* checkpath(char *path, const char *srcpath)
{	// just make sure the char*path is not const
	char *p=path;
	if(NULL!=path && NULL!=srcpath)
	while(*srcpath) {
		if (*srcpath=='/') {
			*p++ = '\\';
			srcpath++;
		}
		else
			*p++ = *srcpath++;
	}
	*p = *srcpath; //EOS
	return path;
}

void findfile(const char *p, const char *pat, void (func)(const char*) )
{	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char tmppath[MAX_PATH+1];
	
	const char *path    = (p  ==NULL)? "." : p;
	const char *pattern = (pat==NULL)? "" : pat;
	
	checkpath(tmppath,path);
	if( PATHSEP != tmppath[strlen(tmppath)-1])
		strcat(tmppath, "\\*");
	else
		strcat(tmppath, "*");
	
	hFind = FindFirstFile(tmppath, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(FindFileData.cFileName, ".") == 0)
				continue;
			if (strcmp(FindFileData.cFileName, "..") == 0)
				continue;

			sprintf(tmppath,"%s%c%s",path,PATHSEP,FindFileData.cFileName);

			if (FindFileData.cFileName && strstr(FindFileData.cFileName, pattern)) {
				func( tmppath );
			}


			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				findfile(tmppath, pat, func);
			}
		}while (FindNextFile(hFind, &FindFileData) != 0);
		FindClose(hFind);
   }
   return;
}
#else

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


#define MAX_DIR_PATH 2048

char* checkpath(char *path, const char*srcpath)
{	// just make sure the char*path is not const
	char *p=path;
	if(NULL!=path && NULL!=srcpath)
	while(*srcpath) {
		if (*srcpath=='\\') {
			*p++ = '/';
			srcpath++;
		}
		else
			*p++ = *srcpath++;
	}
	*p = *srcpath; //EOS
	return path;
}


void findfile(const char *p, const char *pat, void (func)(const char*) )
{	
	DIR* dir;					// pointer to the scanned directory.
	struct dirent* entry;		// pointer to one directory entry.
	struct stat dir_stat;       // used by stat().
	char tmppath[MAX_DIR_PATH+1];
	char path[MAX_DIR_PATH+1]= ".";
	const char *pattern = (pat==NULL)? "" : pat;
	if(p!=NULL) strcpy(path,p);

	// open the directory for reading
	dir = opendir( checkpath(path, path) );
	if (!dir) {
		fprintf(stderr, "Cannot read directory '%s'\n", path);
		return;
	}

	// scan the directory, traversing each sub-directory
	// matching the pattern for each file name.
	while ((entry = readdir(dir))) {
		// skip the "." and ".." entries.
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;

		sprintf(tmppath,"%s%c%s",path, PATHSEP, entry->d_name);

		// check if the pattern matchs.
		if (entry->d_name && strstr(entry->d_name, pattern)) {
			func( tmppath );
		}
		// check if it is a directory.
		if (stat(tmppath, &dir_stat) == -1) {
			fprintf(stderr, "stat error %s\n': ", tmppath);
			continue;
		}
		// is this a directory?
		if (S_ISDIR(dir_stat.st_mode)) {
			// decent recursivly
			findfile(tmppath, pat, func);
		}
	}//end while
}
#endif








/*
#ifdef WIN32
	int	strcasecmp(const char *arg1, const char *arg2);
	int	strncasecmp(const char *arg1, const char *arg2, int n);
	void str_upper(char *name);
	void str_lower(char *name);
//    char *rindex(char *str, char c);
#endif


#ifdef WIN32

// replace with strrchr
char *rindex(char *str, char c)
{
        char *sptr;
        sptr = str;
        while(*sptr)
                ++sptr;
        if (c == '\0')
                return(sptr);
        while(str != sptr)
                if (*sptr-- == c)
                        return(++sptr);
        return(NULL);
}

int strcasecmp(const char *arg1, const char *arg2)
{
  int chk, i;
  if (arg1 == NULL || arg2 == NULL) {
		ShowError("SYSERR: strcasecmp() passed a NULL pointer, %p or %p.\n", arg1, arg2);
    return (0);
  }
  for (i = 0; arg1[i] || arg2[i]; i++)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	// not equal
  return (0);
}

int strncasecmp(const char *arg1, const char *arg2, int n)
{
  int chk, i;
  if (arg1 == NULL || arg2 == NULL) {
		ShowError("SYSERR: strn_cmp() passed a NULL pointer, %p or %p.\n", arg1, arg2);
    return (0);
  }
  for (i = 0; (arg1[i] || arg2[i]) && (n > 0); i++, n--)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	// not equal
  return (0);
}

void str_upper(char *name)
{
  int len = strlen(name);
  while (len--) {
	if (*name >= 'a' && *name <= 'z')
    	*name -= ('a' - 'A');
     name++;
  }
}

void str_lower(char *name)
{
  int len = strlen(name);
  while (len--) {
	if (*name >= 'A' && *name <= 'Z')
    	*name += ('a' - 'A');
    name++;
  }
}

#endif

*/



















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
	bool cReferenced;
public:
	CParamBase() : cReferenced(false)	{}
	virtual ~CParamBase()	{}
	virtual const std::type_info& getType()	{ return typeid(CParamBase); }
	virtual bool assign(const char*s)	{ return false; }
	virtual void print()	{ printf("\nparameter uninitialized"); }

	bool isReferenced()	{ return cReferenced; }
	bool setReference()	{ return (cReferenced=true); }
};

///////////////////////////////////////////////////////////////////////////////
// parameter storage
// stored parameters and their names as smart pointers in a sorted list
// gives out references for changing parameters at their sorage
// needs external lock of the provided mutex for multithread environment
class CParamStorage : public MiniString
{
	///////////////////////////////////////////////////////////////////////////
	// static data
	static TslistDCT<CParamStorage>	cParams;
	static Mutex cLock;
public:
	///////////////////////////////////////////////////////////////////////////
	// static data
	TPtrCount<CParamBase>		cParam;
	ulong						cTime;

	CParamStorage()
	{}
	CParamStorage(const char* name) : MiniString(name)
	{}
	~CParamStorage()
	{}
	void print()
	{
		printf("\nparameter name: %s", (const char*)(*this)); 
		cParam->print();
	}

	///////////////////////////////////////////////////////////////////////////
	// static access functions
	static CParamStorage& find(const char* name)
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
};

///////////////////////////////////////////////////////////////////////////////
// static data
TslistDCT<CParamStorage>	CParamStorage::cParams;
Mutex						CParamStorage::cLock;




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
		virtual bool assign(const X&s)		{  if(cData != s) { cData = s; return true; } return false; }
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
			CParamStorage &stor = CParamStorage::find(cStor);
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
		CParamStorage &stor = CParamStorage::find(cStor);
		return (cStor.cTime != stor.cTime);
	}
	bool adjusted()
	{
		CParamStorage &stor = CParamStorage::find(cStor);
		bool ret = (cStor.cTime != stor.cTime);
		cStor.cTime = stor.cTime;
		return ret;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// determination and conversion of the stored parameter
	static T& convert(const char* name, const T& value, CParamStorage &basestor)
	{
		ScopeLock sl(CParamStorage::getMutex());
		// get a reference to the parameter
		CParamStorage &stor = CParamStorage::find(name);
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
				throw CException("Params: data conversion wrong type");
			stor.cParam = tmp = new CParamData<T>(old->cData);
			// this creation will change the pointer in the database
			// and disconnect all existing references to this node
		}
		if(!tmp) throw CException("Params: data conversion failed");
		// return a reference to the data and copy the storage
		basestor = stor;
		basestor.cTime++;
		tmp->setReference();
		return tmp->cData;
	}
	static T& convert(const char* name, const char* value, CParamStorage &basestor)
	{
		ScopeLock sl(CParamStorage::getMutex());
		// get a reference to the parameter
		CParamStorage &stor = CParamStorage::find(name);
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
				throw CException("Params: data conversion wrong type");
			stor.cParam = tmp = new CParamData<T>(old->cData);
			// this creation will change the pointer in the database
			// and disconnect all existing references to this node
		}
		if(!tmp) throw CException("Params: data conversion failed");
		// return a reference to the data and copy the storage
		basestor = stor;
		basestor.cTime++;
		tmp->setReference();
		return tmp->cData;
	}
	///////////////////////////////////////////////////////////////////////////
	// create a new variable / overwrite the content of an existing
	static void create(const char* name, const T& value)
	{
		ScopeLock sl(CParamStorage::getMutex());
		// get a reference to the parameter
		CParamStorage &stor = CParamStorage::find(name);
		if( !stor.cParam.exists() )
		{	// there is no data pointer
			// create one
			stor.cParam = new CParamData<T>(value);
		}
		else if( stor.cParam->getType() == typeid(T) )
		{	// data is of same type and can be used directly
			CParamData<T>* tmp = dynamic_cast< CParamData<T>* >( (class CParamBase*)stor.cParam.get() );
			if( tmp->cData != value )
			{
				tmp->cData = value;
				stor.cTime = gettick();
			}
		}
		else
		{	// otherwise assign the new value
			if( stor.cParam->assign(value) )
				stor.cTime = gettick();
		}
	}

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


void parameertest()
{
	try {
		double a, xx(141.30);
		MiniString b="";
		// create new param entry
		createParam("double param", "0.2");
		createParam("double param2", "0.2111");
		createParam("double param3", "0.999");

		CParam<MiniString> parameter2("double param", "0.0");

		// create scope persistant object
		CParam<double> parameter("double param", 0.0);

		a=parameter;
		printf("%lf %s\n", a, (const char*)b);

		// modify param entry
		createParam("double param", "0.4");

		a=parameter;
		printf("%lf %s\n", a, (const char*)b);

		CParam<double> testvar("double param", 0.0);

		a=parameter;
		b=testvar;
		printf("%lf %s\n", a, (const char*)b);

		testvar = 5;
		parameter = xx;

		a=parameter;
		b=testvar;
		printf("%lf %s\n", a, (const char*)b);


		printf( typeid(testvar).name() );


		CParam<int> testint("int param", 8);
		CParam<MiniString> teststr("str param", "...test test...");


		CParamStorage::listall();

		CParamStorage::clean();
		
	}
	catch ( CException e )
	{
		printf( "exception %s", (const char*)e );
	}

	CParamStorage::listall();

	printf("\n");
}


///////////////////////////////////////////////////////////////////////////////




