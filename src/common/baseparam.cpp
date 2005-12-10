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
		if( !ip[0] || (ip[0] == '/' && ip[1] == '/'))
			continue;

		memset(w2, 0, sizeof(w2));
		// format: "name:value"
		if( sscanf(ip, "%[^=]= %[^\r\n]", w1, w2) == 2 ||
			sscanf(ip, "%[^:]: %[^\r\n]", w1, w2) == 2 )
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
CParamStorage::CSingletonData &CParamStorage::getSingletonData()
{
	static CParamStorage::CSingletonData sd;
	return sd;
}

///////////////////////////////////////////////////////////////////////////
// static access functions
CParamStorage& CParamStorage::getParam(const char* name)
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t pos;
	CParamStorage tmp(name);
	if( !sd.cParams.find(tmp, 0, pos) )
	{
		tmp.cTime = gettick();
		sd.cParams.insert(tmp);
		if( !sd.cParams.find(tmp, 0, pos) )
			throw CException("Params: insert failed");		
	}
	return sd.cParams[pos];
}
// clean unreferenced parameters
void CParamStorage::clean()
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t i=sd.cParams.size();
	while(i>0)
	{
		i--;
		if( !sd.cParams[i].cParam->isReferenced() )
			sd.cParams.removeindex(i);
	}
}
void CParamStorage::listall()
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	size_t i;
	printf("\nList of Parameters (existing %i):", sd.cParams.size());
	for(i=0; i<sd.cParams.size(); i++)
		sd.cParams[i].print();
	printf("\n---------------\n", sd.cParams.size());

}
void CParamStorage::loadFile(const char* name)
{
	CSingletonData &sd = getSingletonData();
	ScopeLock sl(sd);
	sd.cLoader.loadFile(name);
}






bool doublecallback(const char*name, const double&newval, double&oldval)
{
	printf( "%s is changing from %lf to %lf\n", name, oldval, newval);
	return true;
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

		try
		{
			CParam<MiniString> parameter2("double param", "0.0");
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


