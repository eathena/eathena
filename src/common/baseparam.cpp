#include "baseparam.h"




//////////////////////////////////////////////////////////////////////////
// basic interface for reading configs from file
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Loading a file, stripping lines,
// splitting it to "part1 : part2" or "part1 = part2"
// calling the derived function for processing
/////////////////////////////////////////////////////////////////
bool CConfig::LoadConfig(const char* cfgName)
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
		if( sscanf(ip, "%[^:]: %[^\r\n]", w1, w2) == 2 ||
			sscanf(ip, "%[^=]= %[^\r\n]", w1, w2) == 2 )
		{
			CleanControlChars(w1);
			CleanControlChars(w2);
			checktrim(w1); checktrim(w2);
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

// Return 0/1 for no/yes
int CConfig::SwitchValue(const char *str, int defaultmin, int defaultmax)
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

// Return true/false for yes/no, if unknown return defaultval
bool CConfig::Switch(const char *str, bool defaultval)
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

// Replace control chars with '_' and return if changed
bool CConfig::CleanControlChars(char *str)
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






///////////////////////////////////////////////////////////////////////////////
// basic class for using the old way timers
///////////////////////////////////////////////////////////////////////////////

bool CTimerBase::init(unsigned long interval)
{
	if(interval<1000)
		interval = 1000;
	cTimer = add_timer_interval(gettick()+interval, interval, timercallback, 0, intptr(this), false);
	return (cTimer>=0);
}

// external calling from external timer implementation
int CTimerBase::timercallback(int timer, unsigned long tick, int id, intptr data)
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









///////////////////////////////////////////////////////////////////////////////
//
// Parameter Class
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// static data
CParamStorage::CParamLoader	CParamStorage::cLoader;
TslistDCT<CParamStorage>	CParamStorage::cParams;
Mutex						CParamStorage::cLock;




template <class T> T& CParam<T>::convert(const char* name, const T& value, CParamStorage &basestor)
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
template <class T> T& CParam<T>::convert(const char* name, const char* value, CParamStorage &basestor)
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
template <class T> void CParam<T>::create(const char* name, const T& value)
{
	ScopeLock sl(CParamStorage::getMutex());
	// get a reference to the parameter
	CParamStorage &stor = CParamStorage::getParam(name);
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


		printf("%s\n", typeid(testvar).name() );


		CParam<int> testint("int param", 8);
		CParam<MiniString> teststr("str param", "...test test...");

		CParamStorage::loadFile("conf/login_athena.conf");

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


