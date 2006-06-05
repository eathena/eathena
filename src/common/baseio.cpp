

#include "lock.h"
#include "timer.h"
#include "utils.h"

#include "baseio.h"
#include "basesq.h"
#include "basetx.h"


///////////////////////////////////////////////////////////////////////////////
// for simplicity have global database selection parameters. 
// move to the database wrapper class when debugging is finished
basics::Mutex									_parammtx;	///< lock
basics::TPtrCount<CAccountDBInterface>			_dbreader;	///< the reader
basics::smap<basics::string<>, basics::TPtrCount<CAccountDBInterface> >	_dbwriter;	///< map of writers


// functions for parameter update.
bool _readerselection(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval);
bool _writerselection(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval);


// parameters itself.
basics::CParam< basics::string<> > param_dbreader("read from", "", &_readerselection);
basics::CParam< basics::string<> > param_dbwriter("write to",  "", &_writerselection);


/// database reader seletion.
bool _readerselection(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval)
{
	basics::ScopeLock sl(_parammtx);

	// update the writer, 
	// the assignment will actually trigger _writerselection
	param_dbwriter = newval;

	// get the reader if writer has accepted
	if( _dbwriter.exists(newval) )
	{
		_dbreader = _dbwriter[newval];
		return true;
	}
	// otherwise keep the existing setting
	return false;
}

/// database writer seletion.
// remove the defines when database conversion is finished
bool _writerselection(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval)
{
	basics::ScopeLock sl(_parammtx);

	// check if the specified writer is already allocated
	if( !_dbwriter.exists(newval) )
	{
#if defined(WITH_TEXT)
		if( newval == "txt" )	// in-memory database on txt files
			_dbwriter[newval] = new CAccountDB_txt("");
		else 
#elif defined(SQL_EMBED)
		if( newval == "sqlmem")	// in-memory database on sql
			_dbwriter[newval] = new CAccountDB_txt("");
		else 
#elif !defined(WITH_TEXT) && !defined(SQL_EMBED)
		if( newval == "sql" )	// transition database on sql
			_dbwriter[newval] = new CAccountDB_sql("");
		else 
#endif
			return false;		// otherwise fail this assignment

		// append the new writer to the parameter string
		// which will be a comma seperated list of accepted writer names
		if(oldval.length()>0) newval << "," << oldval;
		return true;
	}
	return false;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// preliminary dynamic database interface

#if defined(WITH_TEXT) && defined(WITH_MYSQL)
bool ParamCallback_database_engine(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval);
basics::CParam< basics::string<> > database_engine("database_engine", "txt", ParamCallback_database_engine);


bool ParamCallback_database_engine(const basics::string<>& name, basics::string<>& newval, const basics::string<>& oldval)
{
	if( newval != "sql" && newval != "txt" )
	{
		ShowError("Parameter '%s' specified as '%s' but only 'txt' or 'sql' is supported.\n"CL_SPACE"Defaulting to %s.",
			(const char*)name, (const char*)newval, (const char*)oldval );

		return false;
	}
	return true;
}
#endif
///////////////////////////////////////////////////////////////////////////////


basics::CParam<bool> CAccountDBInterface::case_sensitive("case_sensitive", true);

CAccountDBInterface* CAccountDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine=="txt")
		return new CAccountDB_txt(dbcfgfile);
	if(database_engine=="sql")
		return new CAccountDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CAccountDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CAccountDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}


basics::CParam<bool>				CCharDBInterface::name_ignore_case("name_ignore_case", false);
basics::CParam<basics::charset>		CCharDBInterface::name_letters("name_letters", basics::charset("a-zA-Z0-9 ,."));
basics::CParam<uint32>				CCharDBInterface::start_zeny("start_zeny", 500);
basics::CParam<ushort>				CCharDBInterface::start_weapon("start_weapon", 1201);
basics::CParam<ushort>				CCharDBInterface::start_armor("start_armor", 2301);
basics::CParam<struct point>		CCharDBInterface::start_point("start_point", point("new_1-1,53,111"));
CFameList							CCharDBInterface::famelists[4]; // order: pk, smith, chem, teak

CCharDBInterface* CCharDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine=="txt")
		return new CCharDB_txt(dbcfgfile);
	if(database_engine=="sql")
		return new CCharDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CCharDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CCharDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}



CGuildDBInterface* CGuildDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine=="txt")
		return new CGuildDB_txt(dbcfgfile);
	if(database_engine=="sql")
		return new CGuildDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CGuildDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CGuildDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}







CPartyDBInterface* CPartyDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine=="txt")
		return new CPartyDB_txt(dbcfgfile);
	if(database_engine=="sql")
		return new CPartyDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CPartyDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CPartyDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}






CPCStorageDBInterface* CPCStorageDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine=="txt")
		return new CPCStorageDB_txt(dbcfgfile);
	if(database_engine=="sql")
		return new CPCStorageDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CPCStorageDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CPCStorageDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}



CGuildStorageDBInterface* CGuildStorageDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine=="txt")
		return new CGuildStorageDB_txt(dbcfgfile);
	if(database_engine=="sql")
		return new CGuildStorageDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CGuildStorageDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CGuildStorageDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}




CPetDBInterface* CPetDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine=="txt")
		return new CPetDB_txt(dbcfgfile);
	if(database_engine=="sql")
		return new CPetDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CPetDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CPetDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}
