#include "basetypes.h"
#include "baseobjects.h"
#include "baseministring.h"
#include "basesync.h"
#include "basefile.h"

#include "baseparam.h"

NAMESPACE_BEGIN(basics)




///////////////////////////////////////////////////////////////////////////////
/// clean a string.
/// remove leading/trailing whitespaces, concatinate multiple whitespaces
/// replace control chars with '_'
const char* config_clean(char *str)
{
	if(str)
	{
		char *src=str, *tar=str, mk=0;
		while(*src && stringcheck::isspace(*src) )
			src++;
		while(*src)
		{
			if( stringcheck::isspace(*src) )
				mk=' ', ++src;
			else
			{
				if( mk )
					*tar++=mk, mk=0;
				*tar = ( stringcheck::iscntrl(*src) ) ? '_' : *src;
				++tar, ++src;
			}
		}
		*tar=0;
		return str;
	}
	return "";
}

///////////////////////////////////////////////////////////////////////////////
template<> 
bool paramconvert<sint64>(sint64 &t, const char* str)
{
	t = (!str || strcasecmp(str, "false") == 0 || strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0) ? 0 :
		(        strcasecmp(str, "true" ) == 0 || strcasecmp(str, "on" ) == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja"  ) == 0 || strcasecmp(str, "si") == 0) ? 1 : 
		strtol(str, NULL, 0);
	return true;
}
template<> 
bool paramconvert<uint64>(uint64 &t, const char* str)
{
	t = (!str || strcasecmp(str, "false") == 0 || strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0) ? 0 :
		(        strcasecmp(str, "true" ) == 0 || strcasecmp(str, "on" ) == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja"  ) == 0 || strcasecmp(str, "si") == 0) ? 1 : 
		strtoul(str, NULL, 0);
	return true;
}
template<> 
bool paramconvert<double>(double &t, const char* s) 
{
	if( !s)
		t = 0;
	else if( s[0]=='0' && locase(s[1])=='x' ) // test for hex numbers, skip octals
		t = strtoul(s, NULL, 0);
	else
		t= strtod(s, NULL);
	return true;
}




///////////////////////////////////////////////////////////////////////////////
/// commandline 2 list conversion.
///////////////////////////////////////////////////////////////////////////////

/// add a new commandline
bool CParameterList::add_commandline(const char* line)
{
	char *wpp = this->cBuf, delim, sep;
	const char *epp=this->cBuf+sizeof(this->cBuf)-1;
	const char* rpp = line;

	// skip all leading spaces
	while( *rpp && (basics::stringcheck::isspace(*rpp)||*rpp==',') ) ++rpp;

	// copy the whole line for testing purpose
	strncpy(this->cTemp, rpp, sizeof(this->cTemp));
	this->cTemp[sizeof(this->cTemp)-1]=0;

	// clear previously command line
	this->cCnt=0;

	while( *rpp && wpp<epp && this->cCnt<sizeof(this->cParam)/sizeof(*this->cParam) )
	{
		// skip all leading spaces
		while( *rpp && basics::stringcheck::isspace(*rpp) ) ++rpp;
		// skip comma seperator when used
		if( (sep=*rpp)==',') 
		{
			++rpp;
			// skip spaces following the comma
			while( *rpp && basics::stringcheck::isspace(*rpp) ) ++rpp;
		}
		// delimiter decision
		// skip the quote when string parameter
		delim = (*rpp=='"') ? *rpp++ : 0;

		// save the parameter start pointer
		this->cParam[this->cCnt] = wpp;

		// copy into the buffer up to a finishing delimiter
		while( *rpp && wpp<epp && (delim?(*rpp!=delim):(!basics::stringcheck::isspace(*rpp) && *rpp!=',')) )
			*wpp++ = *rpp++;

		// terminate the copied string
		// either when there is something or when coma seperation
		if( wpp<epp && (wpp>this->cParam[this->cCnt] || (sep==',')) )
		{
			*wpp++=0;
			++this->cCnt;
		}
	}
	return this->cCnt>0;
}

///////////////////////////////////////////////////////////////////////////////
/// remove entries from list.
void CParameterList::erase(size_t st, size_t num)
{
	if( st<this->cCnt )
	{
		if(st+num>this->cCnt)
			num = this->cCnt-st;
		this->cCnt -= num;
		size_t mvcnt = this->cCnt-st-num;
		if(mvcnt)
			memmove(this->cParam+st, this->cParam+st+num, mvcnt*sizeof(*cParam));
	}
}

///////////////////////////////////////////////////////////////////////////////
// basic interface for reading configs from file
///////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
/// load and parse config file.
/// loads a file, strips lines,
/// splits it to "part1 : part2" or "part1 = part2"
/// cleans leading/trailing whitespaces
/// and calls the derived function for processing
bool CConfig::LoadConfig(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024], *ip, *kp;
	FILE *fp;
	// called with empty filename
	// just return silently 
	if( !cfgName || 0==*cfgName )
		return false;

	if( (fp = safefopen(cfgName, "r")) == NULL) {
		printf("Configuration file (%s) not found.\n", cfgName);
		return false;
	}

	printf("Reading configuration file '%s'\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		// terminate buffer
		line[sizeof(line)-1] = '\0';

		// skip leading spaces
		ip = line;
		while( stringcheck::isspace(*ip) ) ip++;

		// check for comments (only "//") 
		// does not check for escapes or string markers 
		// as a appropiate config grammer needs to be defined first
		for(kp=ip; *kp; ++kp)
		{	// cut of trailing comments/newlines
			if(kp[0]=='\r' || kp[0]=='\n' || (kp[0]=='/' && kp[1]=='/') )
			{
				kp[0] = 0;
				break;
			}
		}

		// skipping empty lines
		if( !ip[0] )
			continue;

		// format: "name:value" or "name=value"
		if( sscanf(ip, "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) == 2 )
		{
			config_clean(w1);
			if(!*w1) continue;
			config_clean(w2);

			if( strcasecmp(w1, "import") == 0 ||
				strcasecmp(w1, "include") == 0 )
			{	// call recursive, prevent infinity loop (first order only)
				if( strcasecmp(cfgName,w2) !=0 )
					this->LoadConfig(w2);
			}
			else
			{	// calling derived function to process
				this->ProcessConfig(w1,w2);
			}
		}
	}
	fclose(fp);
	printf("Reading configuration file '%s' finished\n", cfgName);
	return true;
}


#if defined(LOCAL_TIMER)
///////////////////////////////////////////////////////////////////////////////
// basic class for using the old way timers
///////////////////////////////////////////////////////////////////////////////
bool CTimerBase::init(unsigned long interval)
{
	if(interval<1000)
		interval = 1000;
	//cTimer = add_timer_interval(gettick()+interval, interval, timercallback, 0, basics::numptr(this), false);
	cTimer = -1;
	return (cTimer>=0);
}

// external calling from external timer implementation
int CTimerBase::timercallback(int timer, unsigned long tick, int id, numptr data)
{
	if(data.isptr)
	{
		CTimerBase* base = (CTimerBase*)data.ptr;
		if(timer==base->cTimer)
		{
			if( !base->timeruserfunc(tick) )
			{
//				delete_timer(base->cTimer, timercallback);
				base->cTimer = -1;
			}
		}
	}
	return 0;
}

void CTimerBase::timerfinalize()
{
	if(cTimer>0)
	{
//		delete_timer(cTimer, timercallback);
		cTimer = -1;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
// parse the commandline for parameters
// format
///////////////////////////////////////////////////////////////////////////////
void parseCommandline(int argc, char **argv)
{
	char buffer[1024];
	staticstring<char> str(buffer, sizeof(buffer));
	char w1[1024], w2[1024];
	int i;

	// reserved parameters
	{
		const char*path = argv[0];
		const char*pathend = strrchr(path, PATHSEP);
		const char*file;
		if(pathend)
		{	// had a pathname with a filename
			file = pathend+1;
		}
		else
		{	// no path given
			path = "";
			pathend = path+1;
			file = argv[0];
		}

		createParam("application", file, true);
		createParam("application_path", string<>(path,pathend-path), true);
	}

	for(i=1; i<argc; ++i)
	{
		if( is_file(argv[i]) )
		{
			// we have a valid filename, so we load it directly
			CParamBase::loadFile(argv[i]);

			// and clear the string, start new search
			str.empty();
		}
		else
		{	// not a file, check if it is a direct parameter
			str << argv[i] << ' ';
			memset(w1, 0, sizeof(w1));
			memset(w2, 0, sizeof(w2));
			// if not matching in the first place 
			// just try with the concatinated commandline
			// until something matches or the line runs out
			//## check effort with regex
			if( sscanf(argv[i], "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) == 2 ||
				sscanf(str,     "%1024[^:=]%*[:=]%1024[^\r\n]", w1, w2) == 2 )
			{
				config_clean(w1);
				config_clean(w2);

				if( strcasecmp(w1, "import") == 0 ||
					strcasecmp(w1, "include") == 0 ||
					strcasecmp(w1, "load") == 0 )
				{	// load the file
					CParamBase::loadFile(w2);
				}
				else
				{	// create the parameter
					createParam(w1, w2, true);
				}
				// clear the string, start new search
				str.empty();
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// Parameter Class
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// assign a value to the parameter object and call the childs
void CParamObj::assignvalue(const string<>& value)
{
	this->cTime = GetTickCount();
	if( this->cValue != value )
	{	// take over new value but store the old one
		string<> oldvalue = this->cValue;
		this->cValue = value;
		// check global callback
		if( !this->cCallback || this->cCallback(*this, this->cValue, oldvalue) )
		{	// and go through the childs on success
			CParamBase* ptr = this->cParamRoot;
			while( ptr )
			{
				ptr->call(value);
				ptr = ptr->cNext;
			}
		}
		else
		{	// otherwise do a rollback
			this->cValue = oldvalue;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// print the content to stdout
void CParamObj::print()
{
	CParamBase* ptr=this->cParamRoot;
	uint i=0;
	while(ptr)
		i++, ptr=ptr->cNext;
	printf("\nname: %-16s value='%s' (linked variable type)\nreferenced: %i, count: %i, timestamp %lu", 
		(const char*)(*this), (const char*)this->cValue, this->cReferenced, i, this->cTime);
}

///////////////////////////////////////////////////////////////////////////////
// construction
CParamBase::CParamFile::CParamFile(const string<>& filename)
	: string<>(filename), fileproc(NULL), entrproc(NULL)
{
	struct stat s;
	if( 0==stat((const char*)filename, &s) )
		modtime = s.st_mtime;
}

CParamBase::CParamFile::CParamFile(const string<>& filename, paramfileproc p)
	: string<>(filename), fileproc(p), entrproc(NULL)
{
	struct stat s;
	if( 0==stat((const char*)filename, &s) )
		modtime = s.st_mtime;
}

CParamBase::CParamFile::CParamFile(const string<>& filename, paramentrproc p)
	: string<>(filename), fileproc(NULL), entrproc(p)
{
	struct stat s;
	if( 0==stat((const char*)filename, &s) )
		modtime = s.st_mtime;
}

CParamBase::CParamFile::CParamFile(const string<>& filename, const CParamFile& orig)
	: string<>(filename), fileproc(orig.fileproc), entrproc(orig.entrproc)
{
	struct stat s;
	if( 0==stat((const char*)filename, &s) )
		modtime = s.st_mtime;
}

///////////////////////////////////////////////////////////////////////////////
// config processing callback
bool CParamBase::CParamFile::ProcessConfig(const char*w1,const char*w2)
{	
	if( this->entrproc )
	{	// call the external processing function
		return (*entrproc)(w1, w2);
	}
	else
	{	// create/update parameter
		CParamBase::createParam(w1, w2);
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// checking file state and updating the locals at the same time
bool CParamBase::CParamFile::isModified()
{
	struct stat s;
	return ( 0==stat((const char*)(*this), &s) && s.st_mtime!=this->modtime && (this->modtime=s.st_mtime)==this->modtime );
}

///////////////////////////////////////////////////////////////////////////////
// timer callback
bool CParamBase::CParamLoader::timeruserfunc(unsigned long tick)
{	// check all listed files for modification
	size_t i=this->cFileList.size();
	while(i)
	{
		--i;
		if( this->cFileList[i].isModified() && !this->cFileList[i].load() )
		{	// remove the file from watchlist when failed to load
			this->cFileList.removeindex(i);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// load a file object
bool CParamBase::CParamLoader::loadParamFile(const CParamFile& fileobj)
{
	size_t pos;
	if( this->cFileList.find(fileobj, 0, pos) )
	{
		if( this->cFileList[pos].fileproc != fileobj.fileproc ||
			this->cFileList[pos].entrproc != fileobj.entrproc )
		{	// update the processing function
			this->cFileList[pos].fileproc = fileobj.fileproc;
			this->cFileList[pos].entrproc = fileobj.entrproc;
		}
		else if( !this->cFileList[pos].isModified() )
		{	// nothing modified, just return true
			return true;
		}
	}
	else
	{	// insert the file in the list (temporarily)
		this->cFileList.insert( fileobj );
		if( !this->cFileList.find(fileobj, 0, pos) )
			return false;
	}
	if( !this->cFileList[pos].load() )
	{	// remove when loading has failed
		this->cFileList.removeindex(pos);
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// static data
// need a singleton here for allowing global parameters
CParamBase::CSingletonData &CParamBase::getSingletonData()
{
	static CParamBase::CSingletonData sd;
	return sd;
}

///////////////////////////////////////////////////////////////////////////////
// get a parameter, create it with default value when not existing
TObjPtrCount<CParamObj> CParamBase::getParam(const string<>& name, const string<>& value)
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t pos;

	CParamObj tmp(name);
	if( !sd.cParams.find(tmp, 0, pos) )
	{
		sd.cParams.insert(tmp);
		if( !sd.cParams.find(tmp, 0, pos) )
			throw exception("Params: insert failed");

		sd.cParams[pos]->cTime = GetTickCount();
		sd.cParams[pos]->cValue = value;
	}
	if( !sd.cParams[pos]->cReferenced ) sd.cParams[pos]->cReferenced=true;
	return sd.cParams[pos];
}

///////////////////////////////////////////////////////////////////////////////
// create a new parameter or update the values
TObjPtr<CParamObj> CParamBase::createParam(const string<>& name, const string<>& value, bool fixed)
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t pos;

	CParamObj tmp(name);
	if( !sd.cParams.find(tmp, 0, pos) )
	{
		sd.cParams.insert(tmp);
		if( !sd.cParams.find(tmp, 0, pos) )
			throw exception("Params: insert failed");
	}
	if( sd.cParams[pos]->isFixed() )
	{	// don't overwrite fixed parameters with creation command
		// maybe print a warning
		if( sd.cParams[pos]->value() != value )
			printf("parameter '%s'='%s' is fixed, ignoring assignment of '%s'\n",
				(const char*)name, (const char*)sd.cParams[pos]->value(), (const char*)value);
	}
	else
	{	
		sd.cParams[pos]->assignvalue(value);
		if(fixed) sd.cParams[pos]->setFixed();
	}
	return sd.cParams[pos];
}

///////////////////////////////////////////////////////////////////////////////
// check if a parameter exists
bool CParamBase::existParam(const string<>& name)
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t pos;
	CParamObj tmp(name);
	return sd.cParams.find(tmp, 0, pos);
}

///////////////////////////////////////////////////////////////////////////////
// remove all unreferenced parameters
void CParamBase::clean()
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t i=sd.cParams.size();
	while(i>0)
	{
		i--;
		if( !sd.cParams[i]->cReferenced )
			sd.cParams.removeindex(i);
	}
}

///////////////////////////////////////////////////////////////////////////////
// prints all parameters as list to stdout
void CParamBase::listall()
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t i;
	printf("\nList of Parameters (existing %li):", (unsigned long)sd.cParams.size());
	for(i=0; i<sd.cParams.size(); ++i)
		sd.cParams[i]->print();
	printf("\n---------------\n");
}

///////////////////////////////////////////////////////////////////////////////
// returns a string with all parameters used (as file format & <nl> delimited)
string<> CParamBase::getlist()
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t i;
	string<> str;
	for(i=0; i<sd.cParams.size(); ++i)
	{
		str << sd.cParams[i]->name() << " = " << sd.cParams[i]->value() << "\n";
	}
	return str;
}

///////////////////////////////////////////////////////////////////////////////
// load parameters from fileobject
bool CParamBase::loadParamFile(CParamBase::CParamFile fileobj)
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	return sd.cLoader.loadParamFile(fileobj);
}

///////////////////////////////////////////////////////////////////////////////
// link a parameter access object with the parameter storage object
void CParamBase::link()
{
	this->unlink();
	if( this->cParamObj.exists() )
	{
		// put this node in front
		this->cNext = this->cParamObj->cParamRoot;
		this->cParamObj->cParamRoot = this;
	}
}

///////////////////////////////////////////////////////////////////////////////
// unlink parameter access object
void CParamBase::unlink()
{
	if( this->cParamObj.exists() )
	{
		// remove node from list
		if( this->cParamObj->cParamRoot == this )
		{	// remove root node
			this->cParamObj->cParamRoot = this->cNext;
			this->cNext = NULL;
		}
		else
		{	// look in list
			CParamBase* ptr = this->cParamObj->cParamRoot;
			while( ptr && ptr->cNext!= this )
				ptr = ptr->cNext;
			if( ptr )
			{
				ptr->cNext=this->cNext;
				this->cNext=NULL;
			}
		}
	}
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
bool doublecallback(const string<>& name, double&newval, const double&oldval)
{
	printf( "%s is changing from %lf to %lf\n", (const char*)name, oldval, newval);
	return true;
}
#endif//DEBUG


void test_parameter()
{
#if defined(DEBUG)
	try {
		double a, xx(141.30);
		MiniString b="";
		// create new param entry
		createParam("double param", "0.2");
		createParam("double param2", "0.2111");
		createParam("double param3", "0.999");

		try
		{
			CParam< MiniString > parameter2("double param", "0.0");
		}
		catch(...)
		{
			printf("'double param' not creatable with type MiniString");
		}

		// create scope persistant object
		CParam<double> parameter("double param", 0.0, doublecallback);

		a=parameter;
		printf("%lf %s\n", a, (const char*)b);

		// modify param entry
		createParam("double param", "0.4");

		a=parameter;
		printf("%lf %s\n", a, (const char*)b);

		CParam<double> testvar("double param", 0.0, doublecallback);

		a=parameter;
		b=testvar;
		printf("%lf %s\n", a, (const char*)b);

		testvar = 5;
		parameter = xx;

		a=parameter;
		b=testvar;
		printf("%lf %s\n", a, (const char*)b);


		printf("%s\n", typeid(testvar).name() );


		CParam<int> testint("int param", 8);
		CParam<MiniString> teststr("str param", "...test test...");

		CParamBase::loadFile("config1.txt");

		CParam< vector<int> > testvec("parameter3");

		vector<int>::iterator iter(testvec());
		for(; iter; ++iter)
			printf("%i ", *iter);
		printf("\n");

		
		CParamBase::listall();

		CParamBase::clean();
		
	}
	catch ( exception e )
	{
		printf( "exception %s", e.what() );
	}

	CParamBase::listall();

	printf("\n");
#endif//DEBUG
}


///////////////////////////////////////////////////////////////////////////////

NAMESPACE_END(basics)
