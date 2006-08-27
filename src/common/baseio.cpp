// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder



#include "lock.h"
#include "timer.h"
#include "utils.h"

#include "baseio.h"
#include "basesq.h"
#include "basetx.h"


///////////////////////////////////////////////////////////////////////////////
// database implementation selection
// compile switches are:
// WITH_MYSQL	- for MySQL support available
// WITH_TEXT	- for text support available
// when both defined at compilation, it is swichable
// using command line option or config file entry named "database_engine"
// usable values are "sql" or "txt"
///////////////////////////////////////////////////////////////////////////////
#if !defined(WITH_MYSQL) && !defined(WITH_TEXT)
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif




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
	if(database_engine()=="txt")
		return new CAccountDB_txt(dbcfgfile);
	if(database_engine()=="sql")
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


basics::CParam<bool>				CCharDBInterface::char_new("char_new", false);
basics::CParam<bool>				CCharDBInterface::name_ignore_case("name_ignore_case", false);
basics::CParam<basics::charset>		CCharDBInterface::name_letters("name_letters", basics::charset("a-zA-Z0-9 ,.'"));
basics::CParam<uint32>				CCharDBInterface::start_zeny("start_zeny", 500);
basics::CParam<ushort>				CCharDBInterface::start_weapon("start_weapon", 1201);
basics::CParam<ushort>				CCharDBInterface::start_armor("start_armor", 2301);
basics::CParam<struct point>		CCharDBInterface::start_point("start_point", point("new_1-1,53,111"));
CFameList							CCharDBInterface::famelists[4]; // order: pk, smith, chem, teak


bool CCharDBInterface::testChar(const CCharAccount& account, char *name, const unsigned char str, const unsigned char agi, const unsigned char vit, const unsigned char int_, const unsigned char dex, const unsigned char luk, const unsigned char slot, const unsigned char hair_style, const unsigned char hair_color)
{
	if( !char_new() )
	{	// just ignore here, no error message
	}
	// ensure string termination
	else if( (name[23]=0), remove_control_chars(name) )
	{
		ShowError("Make new char error (control char received in the name): (account: %d).\n", account.account_id);
	}
	// check lenght of character name
	else if( basics::itrim(name), (strlen(name) < 4) )
	{
		ShowError("Make new char error (character name too small): (account: %d, name: '%s').\n",account.account_id, name);
	}
	// Check Authorised letters/symbols in the name of the character
	else if( this->name_letters() != name )
	{
		ShowError("Make new char error (invalid letter in the name): (account: %d), name: %s.\n", account.account_id, name);
	}
	//check stat error
	else if( (str + agi + vit + int_ + dex + luk !=6*5 ) || // stats
			 (slot >= 9) || // slots must not be over 9
			 (hair_style <= 0) || (hair_style >= 24) || // hair style
			 (hair_color >= 9) ||					   // Hair color?
			 // Check stats pairs and make sure they are balanced
			 ((str + int_) > 10) || // str + int pairs check
			 ((agi + luk ) > 10) || // agi + luk pairs check
			 ((vit + dex ) > 10) || // vit + dex pairs check
			 // Check individual stats
			 (str < 1 || str > 9) ||
			 (agi < 1 || agi > 9) ||
			 (vit < 1 || vit > 9) ||
			 (int_< 1 || int_> 9) ||
			 (dex < 1 || dex > 9) ||
			 (luk < 1 || luk > 9) )
	{
		ShowError("Make new char error (stats error, bot cheat) (aid: %d)\n", account.account_id);
	}
	else
		return true;

	return false;
}


CCharDBInterface* CCharDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine()=="txt")
		return new CCharDB_txt(dbcfgfile);
	if(database_engine()=="sql")
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
	if(database_engine()=="txt")
		return new CGuildDB_txt(dbcfgfile);
	if(database_engine()=="sql")
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
	if(database_engine()=="txt")
		return new CPartyDB_txt(dbcfgfile);
	if(database_engine()=="sql")
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
	if(database_engine()=="txt")
		return new CPCStorageDB_txt(dbcfgfile);
	if(database_engine()=="sql")
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
	if(database_engine()=="txt")
		return new CGuildStorageDB_txt(dbcfgfile);
	if(database_engine()=="sql")
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
	if(database_engine()=="txt")
		return new CPetDB_txt(dbcfgfile);
	if(database_engine()=="sql")
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


CHomunculusDBInterface* CHomunculusDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine()=="txt")
		return new CHomunculusDB_txt(dbcfgfile);
	if(database_engine()=="sql")
		return new CHomunculusDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CHomunculusDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CHomunculusDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}













/// conversion from saved string
bool CVar::from_string(const char* str)
{
	static basics::CRegExp re("([^,]+),([^,]*),([^,]*),([^,]*),([^,\n]*)");
	if( re.match(str) )
	{
		*this = re[1];			// name
		//re[2];				// will be storage type (account/char/party/... variable)
		//re[3];				// will be storage id (account_id/char_id/...)
		//re[4];				// will be variable type (string/int/float/...)
		this->cValue = re[5];	// value

		return (this->name()!="" && this->cValue!="0" && this->cValue!="");
	}
	return false;
}
/// conversion to save string
size_t CVar::to_string(char* str, size_t len) const
{
	if( this->name()!="" &&
		this->cValue!="0" &&
		this->cValue!="" )
	{
		int sz = snprintf(str,len, "%s,0,0,0,%s", (const char*)*this, (const char*)this->cValue);
		return (sz>0)?sz:0;
	}
	return 0;
}

/// conversion from transfer buffer
bool CVar::from_buffer(const unsigned char* buf)
{
	this->assign((const char*)buf,32);
	this->cValue.assign((const char*)buf+32);
	return true;
}
/// conversion to transfer buffer
size_t CVar::to_buffer(unsigned char* buf, size_t len) const
{
	memcpy(buf, (const char*)this->name(), 32);
	buf[31]=0;
	memcpy(buf+32, (const char*)this->cValue, 1+this->cValue.size());
	return 32+1+this->cValue.size();
}







CVarDBInterface* CVarDB::getDB(const char *dbcfgfile)
{
	if(dbcfgfile) basics::CParamBase::loadFile(dbcfgfile);
#if defined(WITH_TEXT) && defined(WITH_MYSQL)
	if(database_engine()=="txt")
		return new CVarDB_txt(dbcfgfile);
	if(database_engine()=="sql")
		return new CVarDB_sql(dbcfgfile);
#elif defined(WITH_TEXT)
	return new CVarDB_txt(dbcfgfile);
#elif defined(WITH_MYSQL)
	return new CVarDB_sql(dbcfgfile);
#else
#error "no database implementation specified, define 'WITH_MYSQL','WITH_TEXT' or both"
#endif
	return NULL;
}




















