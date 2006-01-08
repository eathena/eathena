#include "base.h"
#include "baseio.h"
#include "lock.h"
#include "timer.h"
#include "utils.h"
#include "strlib.h"

///////////////////////////////////////////////////////////////////////////////
// basic mysql access
///////////////////////////////////////////////////////////////////////////////
#include "basesq.h"






///////////////////////////////////////////////////////////////////////////////
// a text based database with extra index
// not much tested though and quite basic
// but should be as fast as file access allows
// faster than sql anyway + threadsafe (not shareable though)
///////////////////////////////////////////////////////////////////////////////
#define DB_OPCOUNTMAX 4	// # of db ops before forced cache flush
class simple_database : private Mutex
{
private:
	///////////////////////////////////////////////////////////////////////////
	// index structures
	class _key
	{
	public:
		uint32 cKey1;	// key value
		uint32 cKey2;	// key2 value
		uchar cFlag;	// flag value
		ulong cPos;		// position in file
		ulong cLen;		// data length

		_key()	{}
		_key(uint32 k1, uint32 k2)
			: cKey1(k1), cKey2(k2)
		{}
		_key(uint32 k1, uint32 k2, uchar f, ulong p, ulong l)
			: cKey1(k1), cKey2(k2), cFlag(f), cPos(p), cLen(l)
		{}
		bool operator==(const _key& k) const	{ return (cKey1==k.cKey1) && (cKey2==k.cKey2); }
		bool operator> (const _key& k) const	{ return (cKey1==k.cKey1) ?  (cKey2> k.cKey2) : (cKey1> k.cKey1); }
		bool operator< (const _key& k) const	{ return (cKey1==k.cKey1) ?  (cKey2< k.cKey2) : (cKey1< k.cKey1); }
	};

private:
	///////////////////////////////////////////////////////////////////////////
	// class data
	char *cName;			// path/name of db file
	FILE *cDB;				// file handle to db
	FILE *cIX;				// file handle to index

	size_t cOpCount;		// count of operations
	TslistDST<_key>	cIndex;	// the index
	
	

public:
	class iterator
	{
		simple_database &cDB;
	public:
		size_t cPos;
		iterator(simple_database& sdb, bool last=false) : cDB(sdb), cPos((last)?((sdb.cIndex.size())?(sdb.cIndex.size()-1):0):0)	{}
		iterator(simple_database& sdb, const uint32 key1, const uint32 key2) : cDB(sdb)
		{
			if( !cDB.cIndex.find( simple_database::_key(key1, key2), 0, cPos) )
				cPos = cDB.cIndex.size();
		}

		iterator  operator++(int)	{ iterator temp(*this); next(); return temp; }
		iterator& operator++()		{ next(); return *this; }
		iterator  operator--(int)	{ iterator temp(*this); prev(); return temp; }
		iterator& operator--()		{ prev(); return *this;}
		bool next()					{ cPos++; return (cPos<cDB.cIndex.size()); }
		bool prev()					{ cPos--; return (cPos<cDB.cIndex.size()); }

		operator const bool() const { return (cPos<cDB.cIndex.size()); }
		bool isValid() const		{ return (cPos<cDB.cIndex.size()); }

		uint32 Key1()				{ return isValid()?cDB.cIndex[cPos].cKey1:0; }
		uint32 Key2()				{ return isValid()?cDB.cIndex[cPos].cKey2:0; }
		uchar& Flag()				{ static uchar dummy; return isValid()?cDB.cIndex[cPos].cFlag:dummy; }

		bool find(const uint32 key1, const uint32 key2)
		{
			return cDB.cIndex.find( simple_database::_key(key1, key2), 0, cPos);
		}
		bool read(char* data, size_t maxlen)
		{
			return cDB.read(cPos, data, maxlen);
		}
	};
	friend class simple_database::iterator;
	///////////////////////////////////////////////////////////////////////////
public:
	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	simple_database() : cName(NULL),cDB(NULL), cIX(NULL), cOpCount(0)
	{}
	simple_database(const char *name) : cName(NULL),cDB(NULL), cIX(NULL), cOpCount(0)
	{
		open(name);
	}
	~simple_database()
	{
		close();
	}

	///////////////////////////////////////////////////////////////////////////
	// open the database, create when not exist
	// extension is always ".txt" for db and ".inx" for index
	bool open(const char *name)
	{
		ScopeLock sl(*this);
		char *ip;

		if(NULL==name)
			return false;

		close();

		// with some extra space for file extension
		cName = new char[5+strlen(name)];
		// copy and correct the path seperators
		checkpath(cName, name);

		ip = strrchr(cName, '.');		// find a point in the name
		if(!ip || strchr(ip, PATHSEP) )	// if name had no point
			ip = cName+strlen(cName);	// take all as name

		strcpy(ip, ".txt");
		cDB = fopen(cName, "rb+");
		if(!cDB)
		{	// either not exist or locked
			cDB = fopen(cName, "wb+");
			if(!cDB)	// locked
				return false;
		}

		strcpy(ip, ".inx");
		cIX = fopen(cName, "rb+");
		if(!cIX)
		{	// either not exist or locked
			cIX = fopen(cName, "wb+");
			if(!cIX)	// locked
			{
				fclose(cIX);
				return false;
			}
		}
		// cut the file name back to default
		*ip=0;

		// read index
		// structure is:
		// <# of entries> \n <i1>(0), <i2>(0), <f>(0), <p>(0), <l>(0) \n ...
		fseek(cIX, 0, SEEK_SET);
		unsigned long sz,k1,k2,f,p,l;
		if( 1==fscanf(cIX,"%li\n", &sz) )
		{
			cIndex.realloc(sz);
			while( 5==fscanf(cIX,"%li,%li,%li,%li,%li\n",  &k1,&k2,&f,&p,&l) )
			{
				cIndex.insert( _key(k1,k2,f,p,l) );
			}
		}

		// rebuild the database at startup might be not that expensive
		rebuild();

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// closes the database
	bool close()
	{
		ScopeLock sl(*this);
		flush();
		// rebuild at close either, but might need to test
		rebuild();

		if(cDB)
		{
			fclose(cDB);
			cDB = NULL;
		}
		if(cIX)
		{
			fclose(cIX);
			cIX = NULL;
		}
		if(cName)
		{
			delete[] cName;
			cName = NULL;
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// flushes cache to files
	bool flush(bool force=false)
	{
		ScopeLock sl(*this);
		bool ret = true;
		// check if necessary
		cOpCount++;
		if( force || cOpCount > DB_OPCOUNTMAX )
		{	// need to flush
			if(cDB)
			{
				// nothing to flush here right now
				fflush(cDB);
			}
			if(cIX)
			{
				// write index
				fseek(cIX, 0, SEEK_SET);
				fprintf(cIX,"%li\n", (unsigned long)cIndex.size());
				for(size_t i=0; i<cIndex.size(); i++)
				{
					fprintf(cIX,"%li,%li,%li,%li,%li\n",
						(unsigned long)cIndex[i].cKey1, (unsigned long)cIndex[i].cKey2,
						(unsigned long)cIndex[i].cFlag, (unsigned long)cIndex[i].cPos, (unsigned long)cIndex[i].cLen);
				}
				fflush(cIX);
			}
			// reset op counter
			cOpCount=0;
		}
		return ret;
	}
	///////////////////////////////////////////////////////////////////////////
	// insert/udate
	bool insert(const uint32 key1, const uint32 key2, char* data, size_t len)
	{
		
		if(data)
		{
			ScopeLock sl(*this);
			size_t i;
			if( cIndex.find( _key(key1, key2), 0, i) )
			{	// update
				if( cIndex[i].cLen >= len )
				{	// rewrite the old position
					fseek(cDB, cIndex[i].cPos, SEEK_SET);
				}
				else
				{	// insert new at the end
					fseek(cDB, 0, SEEK_END);
				}
				cIndex[i].cLen = len;
				cIndex[i].cPos = ftell(cDB);
			}
			else
			{	// insert new at the end
				fseek(cDB, 0, SEEK_END);
				cIndex.insert( _key(key1, key2, 0, ftell(cDB), len) );
			}
			fwrite(data, 1, len, cDB);
			fflush(cDB);
			this->flush();
		}
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// delete
	bool remove(const uint32 key1, const uint32 key2)
	{
		ScopeLock sl(*this);
		size_t pos;
		if( cIndex.find( _key(key1, key2), 0, pos) )
		{
			cIndex.removeindex(pos);
			this->flush();
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// search
	bool find(const uint32 key1, const uint32 key2, char* data, size_t maxlen) const
	{
		if(!data)
			return false;

		ScopeLock sl(*this);
		size_t i;
		if( cIndex.find( _key(key1, key2), 0, i) )
		{	
			return read(i, data, maxlen);
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// read index to buffer
	bool read(size_t inx, char* data, size_t maxlen) const
	{
		if( inx<cIndex.size() )
		{
			size_t sz = (cIndex[inx].cLen<maxlen)?cIndex[inx].cLen:maxlen;
			fseek(cDB, cIndex[inx].cPos, SEEK_SET);
			fread(data, sz, 1, cDB);
			data[sz]=0;
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	// gets a free index
	uint32 getfreekey()
	{	// only use keys 1..0xFFFFFFFE
		uint32 key=cIndex.size();
		if( key >= 0xFFFFFFFE )
			key=0;	// return 0 if no free key, 
		else if( !key )
			key=1;	// start with 1
		else if( key == cIndex[key-1].cKey1 )
			key++;	// increment last when range is fully used
		else
		{	// find some unused key within
			size_t a=0, b=key, c;
			while( b > a+1 )
			{
				c=(a+b)/2;
				if( (cIndex[c].cKey1-cIndex[a].cKey1) > (c-a) )
					b=c;
				else
					a=c;
			}
			key = cIndex[a].cKey1 + 1;
		}
		return key;
	}
	///////////////////////////////////////////////////////////////////////////
	// rebuild database and index
	bool rebuild()
	{
		if(!cName || !cDB || !cIX)
			return false;

		ScopeLock sl(*this);
		char buffer[1024];		// the fixed size here might be a problem
		TslistDST<_key>	inx;	// new index
		ulong k1,k2,f,p,l;

		char *ip = cName+strlen(cName);
		strcpy(ip, ".tmp");

		FILE* tmp=fopen(cName, "wb");
		if(!tmp)
			return false;

		for(size_t i=0; i<cIndex.size(); i++)
		{
			k1=cIndex[i].cKey1;
			k2=cIndex[i].cKey2;
			f=cIndex[i].cFlag;
			p=ftell(tmp);
			l=cIndex[i].cLen;

			fseek(cDB,cIndex[i].cPos, SEEK_SET);
			fread (buffer, l,1,cDB);
			fwrite(buffer, l,1,tmp);
			inx.insert( _key(k1,k2,f,p,l) );
		}
		fclose(tmp);
		fclose(cDB);

		// swap databases
		// need a new buffer for renaming
		char *name = new char[1+strlen(cName)];
		memcpy(name, cName,1+strlen(cName));
		strcpy(ip, ".txt");
		::remove( cName );
		::rename( name, cName );
		delete[] name;

		// reopen the database file
		cDB = fopen(cName, "rb+");
		if(!cDB)
		{
			cDB = fopen(cName, "wb+");
			if(!cDB)
			{
				close();
				return false;
			}
		}
		// replace the index
		cIndex = inx;
		flush(true);

		// cut the file name back to default
		*ip=0;
		return true;
	}
};
///////////////////////////////////////////////////////////////////////////////










///////////////////////////////////////////////////////////////////////////////
// common structures
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////
// CAuth Class
////////////////////////////////////////////////////
void CAuth::_tobuffer(unsigned char* &buf) const
{
	if(!buf) return;
	_L_tobuffer( account_id,	buf);
	_L_tobuffer( login_id1, buf);
	_L_tobuffer( login_id2, buf);
	_L_tobuffer( client_ip, buf);
}

void CAuth::_frombuffer(const unsigned char* &buf)
{
	if(!buf) return;
	_L_frombuffer( account_id,	buf);
	_L_frombuffer( login_id1, buf);
	_L_frombuffer( login_id2, buf);
	_L_frombuffer( client_ip, buf);
}

////////////////////////////////////////////////////
// CAccountReg Class
////////////////////////////////////////////////////
void CAccountReg::_tobuffer(unsigned char* &buf) const
{
	size_t i;

	if(!buf) return;

	_W_tobuffer( (account_reg2_num),	buf);

	for(i=0; i<account_reg2_num && i<ACCOUNT_REG2_NUM; i++)
		_global_reg_tobuffer(account_reg2[i],buf);
}

void CAccountReg::_frombuffer(const unsigned char* &buf)
{
	size_t i;

	if(!buf) return;

	_W_frombuffer( (account_reg2_num),	buf);

	for(i=0; i<account_reg2_num && i<ACCOUNT_REG2_NUM; i++)
		_global_reg_frombuffer(account_reg2[i],buf);
}

////////////////////////////////////////////////////
// CMapAccount Class
////////////////////////////////////////////////////
size_t CMapAccount::size() const
{
	return
		sizeof(sex) +
		sizeof(gm_level) +
		sizeof(ban_until) +
		sizeof(valid_until) +
		CAuth::size()+
		CAccountReg::size();
}

void CMapAccount::_tobuffer(unsigned char* &buf) const
{	// only take 32bits of the timer
	uint32 time;

	if(!buf) return;

	_B_tobuffer( sex,			buf);
	_B_tobuffer( gm_level,		buf);
	time = ban_until;	_L_tobuffer( time, buf);
	time = valid_until;	_L_tobuffer( time, buf);
	CAuth::_tobuffer(buf);
	CAccountReg::_tobuffer(buf);
}

void CMapAccount::_frombuffer(const unsigned char* &buf)
{	// only take 32bits of the timer
	uint32 time;

	if(!buf) return;

	_B_frombuffer( sex,			buf);
	_B_frombuffer( gm_level,	buf);
	_L_frombuffer( time, buf);	ban_until=time;
	_L_frombuffer( time, buf);	valid_until=time;
	CAuth::_frombuffer(buf);
	CAccountReg::_frombuffer(buf);
}

////////////////////////////////////////////////////
// CCharAccount Class
////////////////////////////////////////////////////
void CCharAccount::_tobuffer(unsigned char* &buf) const
{
	if(!buf) return;

	_S_tobuffer( email,			buf, sizeof(email));
	CMapAccount::_tobuffer(buf);
}

void CCharAccount::_frombuffer(const unsigned char* &buf)
{
	if(!buf) return;

	_S_frombuffer( email,		buf, sizeof(email));
	CMapAccount::_frombuffer(buf);
}




















///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Account Database
// for storing accounts stuff in login
///////////////////////////////////////////////////////////////////////////////
class CAccountDB_txt : public CTimerBase, private CConfig, public CAccountDBInterface
{
private:
	///////////////////////////////////////////////////////////////////////////
	// helper class for gm_level reading
	class CMapGM
	{
	public:
		uint32 account_id;
		unsigned char gm_level;

		CMapGM(uint32 accid=0) : account_id(accid), gm_level(0)	{}
		CMapGM(uint32 accid, unsigned char lv) : account_id(accid), gm_level(lv)	{}

		bool operator==(const CMapGM& c) const { return this->account_id==c.account_id; }
		bool operator!=(const CMapGM& c) const { return this->account_id!=c.account_id; }
		bool operator> (const CMapGM& c) const { return this->account_id> c.account_id; }
		bool operator>=(const CMapGM& c) const { return this->account_id>=c.account_id; }
		bool operator< (const CMapGM& c) const { return this->account_id< c.account_id; }
		bool operator<=(const CMapGM& c) const { return this->account_id<=c.account_id; }
	};
	///////////////////////////////////////////////////////////////////////////
	// read gm levels to a list
	bool readGMAccount(TslistDST<CMapGM> &gmlist)
	{
		struct stat file_stat;
		FILE *fp;

		// clear all gm_levels
		gmlist.resize(0);

		// get last modify time/date
		creation_time_GM_account_file = (0==stat(GM_account_filename, &file_stat))? 0 : file_stat.st_mtime;

		fp = safefopen(GM_account_filename, "r");
		if( fp )
		{
			char line[1024];
			int level;
			unsigned long account_id=0;
			size_t line_counter=0;
			unsigned long start_range = 0, end_range = 0, is_range = 0, current_id = 0;

			while( fgets(line, sizeof(line), fp) )
			{
				line_counter++;
				if( !get_prepared_line(line) )
					continue;
				is_range = (sscanf(line, "%lu%*[-~]%lu %d",&start_range,&end_range,&level)==3); // ID Range [MC Cameri]
				if (!is_range && sscanf(line, "%lu %d", &account_id, &level) != 2 && sscanf(line, "%ld: %d", &account_id, &level) != 2)
					ShowError("read_gm_account: file [%s], invalid 'acount_id|range level' format (line #%d).\n", GM_account_filename, line_counter);
				else if (level <= 0)
					ShowError("read_gm_account: file [%s] %dth account (line #%d) (invalid level [0 or negative]: %d).\n", GM_account_filename, gmlist.size()+1, line_counter, level);
				else
				{
					if (level > 99)
					{
						ShowWarning("read_gm_account: file [%s] %dth account (invalid level, but corrected: %d->99).\n", GM_account_filename, gmlist.size()+1, level);
						level = 99;
					}
					if (is_range)
					{
						if (start_range==end_range)
							ShowError("read_gm_account: file [%s] invalid range, beginning of range is equal to end of range (line #%d).\n", GM_account_filename, line_counter);
						else if (start_range>end_range)
							ShowError("read_gm_account: file [%s] invalid range, beginning of range must be lower than end of range (line #%d).\n", GM_account_filename, line_counter);
						else
							for (current_id = start_range;current_id<=end_range;current_id++)
							{
								gmlist.insert( CMapGM(current_id, level) );
							}
					}
					else
					{
						gmlist.insert( CMapGM(account_id, level) );
					}
				}
			}
			fclose(fp);
			ShowStatus("File '%s' read (%d GM accounts found).\n", GM_account_filename, gmlist.size());
			return true;
		}
		else
		{
			ShowError("read_gm_account: GM accounts file [%s] not found.\n", GM_account_filename);
			return false;
		}

	}
	///////////////////////////////////////////////////////////////////////////
	// read accounts from file
	bool readAccounts()
	{
		FILE *fp;
		if ((fp = safefopen(account_filename, "r")) == NULL)
		{
			// no account file -> no account -> no login, including char-server (ERROR)
			ShowError(CL_BT_RED"Accounts file [%s] not found.\n"CL_RESET, account_filename);
			return false;
		}
		else
		{
			size_t pos;
			CLoginAccount data;
			unsigned long account_id;
			int logincount, state, n, i, j, v;
			char line[2048], *p, userid[2048], pass[2048], lastlogin[2048], sex, email[2048], error_message[2048], last_ip[2048], memo[2048];
			unsigned long ban_until_time;
			unsigned long connect_until_time;
			char str[2048];
			int GM_count = 0;
			int server_count = 0;

			///////////////////////////////////
			TslistDST<CMapGM> gmlist;
			readGMAccount(gmlist);

			while( fgets(line, sizeof(line), fp) )
			{
				if( !get_prepared_line(line) )
					continue;
				*userid=0;
				*pass=0;
				*lastlogin=0;

				// database version reading (v2)
				if( 13 == (i=sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]\t%ld%n",
								&account_id, userid, pass, lastlogin, &sex, &logincount, &state, email, error_message, &connect_until_time, last_ip, memo, &ban_until_time, &n)) && (line[n] == '\t') )
				{	// with ban_time
					;
				}
				else if( 12 == (i=sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]%n",
								&account_id, userid, pass, lastlogin, &sex, &logincount, &state, email, error_message, &connect_until_time, last_ip, memo, &n)) && (line[n] == '\t') )
				{	// without ban_time
					ban_until_time=0;
				}
				// Old athena database version reading (v1)
				else if( 5 <= (i=sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%n",
								&account_id, userid, pass, lastlogin, &sex, &logincount, &state, &n)) )
				{
					*email=0;
					*last_ip=0;
					ban_until_time=0;
					connect_until_time=0;
					if (i < 6)
						logincount=0;
				}
				else if(sscanf(line, "%ld\t%%newid%%\n%n", &account_id, &i) == 1 && i > 0 && account_id > next_account_id)
				{
					next_account_id = account_id;
					continue;
				}

				/////////////////////////////////////
				// Some checks
				if (account_id > 10000000) //!!END_ACCOUNT_NUM
				{
					ShowError(CL_BT_RED"An account has an id higher than %d\n", 10000000);//!!
					ShowMessage("           Account id #%d -> account not read (saved in log file).\n"CL_RESET, account_id);

					//!! log account line to file as promised

					continue;
				}
				userid[23] = '\0';
				remove_control_chars(userid);

				if( cList.find( CLoginAccount(account_id), pos, 0) )
				{
					ShowWarning(CL_BT_RED"An account has an identical id to another.\n"CL_RESET);
					ShowMessage("           Account id #%d -> not read (saved in log file).\nCL_RESET", account_id);

					//!! log account line to file as promised
				}
				else if( cList.find( CLoginAccount(userid), pos, 1) )
				{
					ShowError(CL_BT_RED"Account name already exists.\n"CL_RESET);
					ShowMessage("           Account name '%s' -> new account not read.\n", userid); // 2 lines, account name can be long.
					ShowMessage("           Account saved in log file.\n");

					//!! log account line to file as promised
				}
				else
				{
					CLoginAccount temp;

					temp.account_id = account_id;
					safestrcpy(temp.userid, userid, sizeof(temp.userid));

					pass[23] = '\0';
					remove_control_chars(pass);
					safestrcpy(temp.passwd, pass, sizeof(temp.passwd));
					temp.sex = (sex == 'S' || sex == 's') ? 2 : (sex == 'M' || sex == 'm');
					if (temp.sex == 2)
						server_count++;

					remove_control_chars(email);
					if( !email_check(email) )
					{
						ShowWarning("Account %s (%d): invalid e-mail (replaced with a@a.com).\n", userid, account_id);
						safestrcpy(temp.email, "a@a.com", sizeof(temp.email));
					}
					else
						safestrcpy(temp.email, email, sizeof(temp.email));

					if( gmlist.find( CMapGM(account_id),0,pos) )
					{
						temp.gm_level = gmlist[pos].gm_level;
						GM_count++;
					}
					else
						temp.gm_level=0;

					temp.login_count = (logincount>0)?logincount:0;

					lastlogin[23] = '\0';
					if(lastlogin[0]=='-' && lastlogin[1]==0)
						lastlogin[0] = 0;// remove defaults
					else
						remove_control_chars(lastlogin);
					safestrcpy(temp.last_login, lastlogin, sizeof(temp.last_login));

					if(error_message[0]=='-' && error_message[1]==0)
						error_message[0] = 0;// remove defaults
					else
						remove_control_chars(error_message);
					safestrcpy(temp.error_message, error_message, sizeof(temp.error_message));

					if(memo[0]=='-' && memo[1]==0)
						memo[0] = 0;// remove defaults
					else
						remove_control_chars(memo);
					safestrcpy(temp.memo, memo, sizeof(temp.memo));

					temp.state = state;

					temp.ban_until = (i==13) ? ban_until_time : 0;
					temp.valid_until = connect_until_time;

					last_ip[15] = '\0';
					remove_control_chars(last_ip);
					if(*last_ip && *last_ip!='-')
					{
						ipaddress ip(last_ip);
						temp.client_ip = ip;
						ip.getstring(temp.last_ip, sizeof(temp.last_ip));
					}
					else
					{
						temp.client_ip  = INADDR_ANY;
						temp.last_ip[0] = 0;
					}
					


					p = line;
					for(j = 0; j < ACCOUNT_REG2_NUM; j++)
					{
						p += n;
						if(sscanf(p, "%[^\t,],%d %n", str, &v, &n) != 2)
						{	// We must check if a str is void. If it's, we can continue to read other REG2.
							// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
							if (p[0] == ',' && sscanf(p, ",%d %n", &v, &n) == 1)
							{
								j--;
								continue;
							} else
								break;
						}
						str[31] = '\0';
						remove_control_chars(str);
						safestrcpy(temp.account_reg2[j].str, str, sizeof(temp.account_reg2[0].str));
						temp.account_reg2[j].value = v;
					}
					temp.account_reg2_num = j;

					if (next_account_id <= account_id)
						next_account_id = account_id + 1;

					insert(temp);
				}
			}
			fclose(fp);

			if( cList.size() == 0 )
			{
				ShowError("No account found in %s.\n", account_filename);
				snprintf(line, sizeof(line), "No account found in %s.", account_filename);
			}
			else
			{
				if( cList.size() == 1)
				{
					ShowStatus("1 account read in %s,\n", account_filename);
					snprintf(line, sizeof(line), "1 account read in %s,", account_filename);
				}
				else
				{
					ShowStatus("%d accounts read in %s,\n", cList.size(), account_filename);
					snprintf(line, sizeof(line), "%ld accounts read in %s,", (unsigned long)cList.size(), account_filename);
				}
				if (GM_count == 0)
				{
					ShowMessage("           of which is no GM account, and ");
					snprintf(str, sizeof(str), "%s of which is no GM account and", line);
				}
				else if (GM_count == 1)
				{
					ShowMessage("           of which is 1 GM account, and ");
					snprintf(str, sizeof(str), "%s of which is 1 GM account and", line);
				}
				else
				{
					ShowMessage("           of which is %d GM accounts, and ", GM_count);
					snprintf(str, sizeof(str), "%s of which is %d GM accounts and", line, GM_count);
				}
				if (server_count == 0)
				{
					ShowMessage("no server account ('S').\n");
					snprintf(line, sizeof(line), "%s no server account ('S').", str);
				}
				else if (server_count == 1)
				{
					ShowMessage("1 server account ('S').\n");
					snprintf(line, sizeof(line), "%s 1 server account ('S').", str);
				}
				else
				{
					ShowMessage("%d server accounts ('S').\n", server_count);
					snprintf(line, sizeof(line), "%s %d server accounts ('S').", str, server_count);
				}
			}
	//		login_log("%s" RETCODE, line);
			return true;
		}
	}
	///////////////////////////////////////////////////////////////////////////
	// write accounts
	bool saveAccounts()
	{
		FILE *fp;
		size_t i, k;
		int lock;

		// Data save
		if ((fp = lock_fopen(account_filename, lock)) == NULL) {
			return false;
		}
		fprintf(fp, "// Accounts file: here are saved all information about the accounts.\n");
		fprintf(fp, "// Structure: ID, account name, password, last login time, sex, # of logins, state, email, error message for state 7, validity time, last (accepted) login ip, memo field, ban timestamp, repeated(register text, register value)\n");
		fprintf(fp, "// Some explanations:\n");
		fprintf(fp, "//   account name    : between 4 to 23 char for a normal account (standard client can't send less than 4 char).\n");
		fprintf(fp, "//   account password: between 4 to 23 char\n");
		fprintf(fp, "//   sex             : M or F for normal accounts, S for server accounts\n");
		fprintf(fp, "//   state           : 0: account is ok, 1 to 256: error code of packet 0x006a + 1\n");
		fprintf(fp, "//   email           : between 3 to 39 char (a@a.com is like no email)\n");
		fprintf(fp, "//   error message   : text for the state 7: 'Your are Prohibited to login until <text>'. Max 19 char\n");
		fprintf(fp, "//   valitidy time   : 0: unlimited account, <other value>: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");
		fprintf(fp, "//   memo field      : max 254 char\n");
		fprintf(fp, "//   ban time        : 0: no ban, <other value>: banned until the date: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");
		for(i = 0; i < cList.size(); i++)
		{
			fprintf(fp, "%ld\t"
						"%s\t"
						"%s\t"
						"%s\t"
						"%c\t"
						"%ld\t"
						"%ld\t"
						"%s\t"
						"%s\t"
						"%ld\t"
						"%s\t"
						"%s\t"
						"%ld\t",
						(unsigned long)cList[i].account_id,
						cList[i].userid,
						cList[i].passwd,
						(*cList[i].last_login)?cList[i].last_login:"-",
						(cList[i].sex == 2) ? 'S' : (cList[i].sex ? 'M' : 'F'),
						(unsigned long)cList[i].login_count,
						(unsigned long)cList[i].state,
						(*cList[i].email)?cList[i].email:"a@a.com",
						(*cList[i].error_message)?cList[i].error_message:"-",
						(unsigned long)cList[i].valid_until,
						(cList[i].last_ip[0])?cList[i].last_ip:"-",
						(*cList[i].memo)?cList[i].memo:"-",
						(unsigned long)cList[i].ban_until);
			for(k = 0; k< cList[i].account_reg2_num; k++)
				if(cList[i].account_reg2[k].str[0])
					fprintf(fp, "%s,%ld ", cList[i].account_reg2[k].str, (long)cList[i].account_reg2[k].value);
			fprintf(fp, RETCODE);
		}
		fprintf(fp, "%ld\t%%newid%%"RETCODE, (unsigned long)next_account_id);
		lock_fclose(fp, account_filename, lock);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// config stuff
	uint32 next_account_id;
	char account_filename[1024];
	char GM_account_filename[1024];
	time_t creation_time_GM_account_file;

	size_t savecount;

	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CLoginAccount, 2> cList;

	///////////////////////////////////////////////////////////////////////////
	// alternative interface data
	size_t	cPos;
	Mutex	cMx;

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CAccountDB_txt(const char* configfile):
		CTimerBase(60000),		// 60sec save interval
		next_account_id(200000),//!! START_ACCOUNT_NUM
		creation_time_GM_account_file(0),
		savecount(0)
	{
		safestrcpy(account_filename,"save/account.txt",sizeof(account_filename));
		safestrcpy(GM_account_filename,"conf/GM_account.txt",sizeof(GM_account_filename));

		init(configfile);
	}
	virtual ~CAccountDB_txt()
	{
		close();
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// functions for db interface
	virtual size_t size()						{ return cList.size(); }
	virtual CLoginAccount& operator[](size_t i)	{ return cList[i]; };

	virtual bool existAccount(const char* userid);
	virtual bool searchAccount(const char* userid, CLoginAccount&account);
	virtual bool searchAccount(uint32 accid, CLoginAccount&account);

	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account);
	virtual bool removeAccount(uint32 accid);
	virtual bool saveAccount(const CLoginAccount& account);

private:
	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2);
	bool insert(const CLoginAccount& la){ return cList.insert(la); }

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return readAccounts();
	}
	bool close()
	{
		return saveAccounts();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			saveAccounts();
		}

		//!! todo check changes in files and reload them if necessary
		struct stat file_stat;
		time_t new_time;

		// get last modify time/date
		if( 0!=stat(GM_account_filename, &file_stat) )
			new_time = 0; // error
		else
			new_time = file_stat.st_mtime;

		if(new_time != creation_time_GM_account_file)
		{
			//!! re-read GM-File
		}
		return true;
	}

	virtual bool aquire()
	{
		cMx.lock();
		return this->first();
	}
	virtual bool release()
	{
		cMx.unlock();
		return true; 
	}
	virtual bool first()
	{
		ScopeLock sl(cMx);
		cPos=0;
		return cList.size()>0; 
	}
	virtual operator bool()		{ ScopeLock sl(cMx); return cPos<cList.size(); }
	virtual bool operator++(int){ ScopeLock sl(cMx); cPos++; return (*this); }
	virtual bool save()			{ ScopeLock sl(cMx); return true; }

	virtual bool find(const char* userid)
	{
		ScopeLock sl(cMx);
		size_t pos;
		// search in index 1
		if( cList.find( CLoginAccount(userid), pos, 1) )
		{	// set position based to index 0 
			return cList.find( cList(pos,1), cPos, 0);
		}
		return false; 
	}
	virtual bool find(uint32 accid)
	{
		ScopeLock sl(cMx);
		return cList.find( CLoginAccount(accid), cPos, 0);
	}
	virtual CLoginAccount& operator()()
	{
		ScopeLock sl(cMx);
		if( cPos>=cList.size() )
			throw "access out of bound";
		return cList(cPos,0);
	}
};
///////////////////////////////////////////////////////////////////////////////
// class implementation
bool CAccountDB_txt::ProcessConfig(const char*w1, const char*w2)
{
	if(w1 && w2)
	{
		if( 0==strcasecmp(w1, "account_filename") )
			safestrcpy(account_filename, w2, sizeof(account_filename));
		else if( 0==strcasecmp(w1, "GM_account_filename") )
			safestrcpy(GM_account_filename, w2, sizeof(GM_account_filename));
//		else if (strcasecmp(w1, "gm_account_filename_check_timer") == 0)
//			gm_account_filename_check_timer = atoi(w2);

	}
	return true;
}
bool CAccountDB_txt::existAccount(const char* userid)
{	// check if account with userid already exist
	size_t pos;
	return cList.find( CLoginAccount(userid), pos, 1);
}
bool CAccountDB_txt::searchAccount(const char* userid, CLoginAccount&account)
{	// get account by userid
	size_t pos;
	if( cList.find( CLoginAccount(userid), pos, 1) )
	{
		account = cList(pos,1);
		return true;
	}
	return false;
}
bool CAccountDB_txt::searchAccount(uint32 accid, CLoginAccount&account)
{	// get account by account_id
	size_t pos;
	if( cList.find( CLoginAccount(accid), pos, 0) )
	{
		account = cList(pos,0);
		return true;
	}
	return false;
}
bool CAccountDB_txt::insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account)
{	// insert a new account
	CLoginAccount temp;
	size_t pos;
	uint32 accid = next_account_id++;
	if( cList.find( CLoginAccount(userid), pos, 1) )
	{	// remove an existing account
		cList.removeindex(pos, 1);
	}

	cList.insert( CLoginAccount(accid, userid, passwd, sex, email) );
	savecount++;

	if( cList.find( CLoginAccount(userid), pos, 1) )
	{
		account = cList(pos,1);
		return true;
	}
	return false;
}
bool CAccountDB_txt::removeAccount(uint32 accid)
{
	size_t pos;
	if( cList.find(CLoginAccount(accid),pos, 0) )
	{
		return cList.removeindex(pos, 0);
	}
	return false;
}
bool CAccountDB_txt::saveAccount(const CLoginAccount& account)
{
	size_t pos;

	if( cList.find(account, pos, 1) )
	{
		cList(pos,1) = account;

		savecount++;

		return true;
	}
	return false;
}


#endif



CAccountDBInterface* CAccountDB::getDB(const char *dbcfgfile)
{
#ifdef TXT_ONLY
	return new CAccountDB_txt(dbcfgfile);
#else
	return new CAccountDB_sql(dbcfgfile);
#endif// SQL
}

///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////

class CCharDB_txt : public CTimerBase, private CConfig, public CCharDBInterface
{
private:
	///////////////////////////////////////////////////////////////////////////
	// Function to create the character line (for save)
	int char_to_str(char *str, size_t sz, const CCharCharacter &p)
	{
		size_t i;
		char *str_p = str;

		point last_point = p.last_point;

		if (last_point.mapname[0] == '\0') {
			safestrcpy(last_point.mapname, "prontera", 16);
			last_point.x = 273;
			last_point.y = 354;
		}

		str_p += snprintf(str_p, str+sz-str_p,
			"%ld"
			"\t%ld,%d"
			"\t%s"
			"\t%d,%d,%d"
			"\t%ld,%ld,%ld"
			"\t%ld,%ld,%ld,%ld"
			"\t%d,%d,%d,%d,%d,%d"
			"\t%d,%d"
			"\t%d,%d,%d"
			"\t%ld,%ld,%ld"
			"\t%d,%d,%d"
			"\t%d,%d,%d,%d,%d"
			"\t%s,%d,%d"
			"\t%s,%d,%d"
			"\t%ld,%ld,%ld,%ld"
			"\t%ld"
			"\t",
			(unsigned long)p.char_id,
			(unsigned long)p.account_id, p.slot,
			p.name,
			p.class_, p.base_level, p.job_level,
			(unsigned long)p.base_exp, (unsigned long)p.job_exp, (unsigned long)p.zeny,
			(unsigned long)p.hp, (unsigned long)p.max_hp, (unsigned long)p.sp, (unsigned long)p.max_sp,
			p.str, p.agi, p.vit, p.int_, p.dex, p.luk,
			p.status_point, p.skill_point,
			p.option, MakeWord(p.karma, p.chaos), p.manner,
			(unsigned long)p.party_id, (unsigned long)p.guild_id, (unsigned long)p.pet_id,
			p.hair, p.hair_color, p.clothes_color,
			p.weapon, p.shield, p.head_top, p.head_mid, p.head_bottom,
			// store the checked lastpoint
			last_point.mapname, last_point.x, last_point.y,
			p.save_point.mapname, p.save_point.x, p.save_point.y,
			(unsigned long)p.partner_id,(unsigned long)p.father_id,(unsigned long)p.mother_id,(unsigned long)p.child_id,
			(unsigned long)p.fame_points);
		for(i = 0; i < MAX_MEMO; i++)
			if (p.memo_point[i].mapname[0]) {
				str_p += snprintf(str_p, str+sz-str_p, "%s,%d,%d", p.memo_point[i].mapname, p.memo_point[i].x, p.memo_point[i].y);
			}
		*(str_p++) = '\t';

		for(i = 0; i < MAX_INVENTORY; i++)
			if (p.inventory[i].nameid) {
				str_p += snprintf(str_p, str+sz-str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
						 p.inventory[i].id, p.inventory[i].nameid, p.inventory[i].amount, p.inventory[i].equip,
						 p.inventory[i].identify, p.inventory[i].refine, p.inventory[i].attribute,
						 p.inventory[i].card[0], p.inventory[i].card[1], p.inventory[i].card[2], p.inventory[i].card[3]);
			}
		*(str_p++) = '\t';

		for(i = 0; i < MAX_CART; i++)
			if (p.cart[i].nameid) {
				str_p += snprintf(str_p, str+sz-str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
						 p.cart[i].id, p.cart[i].nameid, p.cart[i].amount, p.cart[i].equip,
						 p.cart[i].identify, p.cart[i].refine, p.cart[i].attribute,
						 p.cart[i].card[0], p.cart[i].card[1], p.cart[i].card[2], p.cart[i].card[3]);
			}
		*(str_p++) = '\t';

		for(i = 0; i < MAX_SKILL; i++)
			if (p.skill[i].id && p.skill[i].flag != 1) {
				str_p += snprintf(str_p, str+sz-str_p, "%d,%d ", p.skill[i].id, (p.skill[i].flag == 0) ? p.skill[i].lv : p.skill[i].flag-2);
			}
		*(str_p++) = '\t';

		for(i = 0; i < p.global_reg_num; i++)
			if (p.global_reg[i].str[0])
				str_p += snprintf(str_p, str+sz-str_p, "%s,%ld ", p.global_reg[i].str, (long)p.global_reg[i].value);
		*(str_p++) = '\t';

		*str_p = '\0';
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	// Function to set the character from the line (at read of characters file)
	bool char_from_str(const char *str)
	{
		int tmp_int[256];
		int next, len;
		size_t i;
		CCharCharacter p;

		// initilialise character
		memset(&p, 0, sizeof(CCharCharacter));
		memset(tmp_int, 0, sizeof(tmp_int));

		if( sscanf(str,
			"%d\t%d,%d\t%[^\t]"
			"\t%d,%d,%d"
			"\t%d,%d,%d"
			"\t%d,%d,%d,%d"
			"\t%d,%d,%d,%d,%d,%d"
			"\t%d,%d"
			"\t%d,%d,%d"
			"\t%d,%d,%d"
			"\t%d,%d,%d"
			"\t%d,%d,%d,%d,%d"
			"\t%[^,],%d,%d"
			"\t%[^,],%d,%d"
			"\t%d,%d,%d,%d"
			"\t%d"
			"%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], p.name, //
			&tmp_int[3], &tmp_int[4], &tmp_int[5],
			&tmp_int[6], &tmp_int[7], &tmp_int[8],
			&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			&tmp_int[19], &tmp_int[20],
			&tmp_int[21], &tmp_int[22], &tmp_int[23], //
			&tmp_int[24], &tmp_int[25], &tmp_int[26],
			&tmp_int[27], &tmp_int[28], &tmp_int[29],
			&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			p.last_point.mapname, &tmp_int[35], &tmp_int[36], //
			p.save_point.mapname, &tmp_int[37], &tmp_int[38], &tmp_int[39],
			&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next) == 47 )
		{
			// my personal reordering
			//ShowMessage("char: new char data ver.5a\n");
		}
		else if( sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
			"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			"\t%[^,],%d,%d\t%[^,],%d,%d,%d,%d,%d,%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], p.name, //
			&tmp_int[3], &tmp_int[4], &tmp_int[5],
			&tmp_int[6], &tmp_int[7], &tmp_int[8],
			&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			&tmp_int[19], &tmp_int[20],
			&tmp_int[21], &tmp_int[22], &tmp_int[23], //
			&tmp_int[24], &tmp_int[25], &tmp_int[26],
			&tmp_int[27], &tmp_int[28], &tmp_int[29],
			&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			p.last_point.mapname, &tmp_int[35], &tmp_int[36], //
			p.save_point.mapname, &tmp_int[37], &tmp_int[38], &tmp_int[39],
			&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next) == 47 )
		{
			// Char structture of version 1488+
			//ShowMessage("char: new char data ver.5\n");
		}
		else if( sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
			"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			"\t%[^,],%d,%d\t%[^,],%d,%d,%d,%d,%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], p.name, //
			&tmp_int[3], &tmp_int[4], &tmp_int[5],
			&tmp_int[6], &tmp_int[7], &tmp_int[8],
			&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			&tmp_int[19], &tmp_int[20],
			&tmp_int[21], &tmp_int[22], &tmp_int[23], //
			&tmp_int[24], &tmp_int[25], &tmp_int[26],
			&tmp_int[27], &tmp_int[28], &tmp_int[29],
			&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			p.last_point.mapname, &tmp_int[35], &tmp_int[36], //
			p.save_point.mapname, &tmp_int[37], &tmp_int[38], &tmp_int[39],
			&tmp_int[40], &tmp_int[41], &tmp_int[42], &next) == 46 )
		{
			// Char structture of version 1363+
			tmp_int[43] = 0; // fame
			//ShowMessage("char: new char data ver.4\n");
		}
		else if( sscanf(str,"%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
			"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			"\t%[^,],%d,%d\t%[^,],%d,%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], p.name,
			&tmp_int[3], &tmp_int[4], &tmp_int[5],
			&tmp_int[6], &tmp_int[7], &tmp_int[8],
			&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			&tmp_int[19], &tmp_int[20],
			&tmp_int[21], &tmp_int[22], &tmp_int[23],
			&tmp_int[24], &tmp_int[25], &tmp_int[26],
			&tmp_int[27], &tmp_int[28], &tmp_int[29],
			&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			p.last_point.mapname, &tmp_int[35], &tmp_int[36],
			p.save_point.mapname, &tmp_int[37], &tmp_int[38], &tmp_int[39], &next) == 43 )
		{
			// Char structture of version 1008 and before 1363
			tmp_int[40] = 0; // father
			tmp_int[41] = 0; // mother
			tmp_int[42] = 0; // child
			tmp_int[43] = 0; // fame
			//ShowMessage("char: new char data ver.3\n");
		}
		else if( sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
			"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			"\t%[^,],%d,%d\t%[^,],%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], p.name,
			&tmp_int[3], &tmp_int[4], &tmp_int[5],
			&tmp_int[6], &tmp_int[7], &tmp_int[8],
			&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			&tmp_int[19], &tmp_int[20],
			&tmp_int[21], &tmp_int[22], &tmp_int[23],
			&tmp_int[24], &tmp_int[25], &tmp_int[26],
			&tmp_int[27], &tmp_int[28], &tmp_int[29],
			&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			p.last_point.mapname, &tmp_int[35], &tmp_int[36], //
			p.save_point.mapname, &tmp_int[37], &tmp_int[38], &next) == 42 )
		{
			// Char structture from version 384 to 1007
			tmp_int[39] = 0; // partner id
			tmp_int[40] = 0; // father
			tmp_int[41] = 0; // mother
			tmp_int[42] = 0; // child
			tmp_int[43] = 0; // fame
			//ShowMessage("char: old char data ver.2\n");
		}
		else if( sscanf(str, "%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
			"\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			"\t%[^,],%d,%d\t%[^,],%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], p.name,
			&tmp_int[3], &tmp_int[4], &tmp_int[5],
			&tmp_int[6], &tmp_int[7], &tmp_int[8],
			&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			&tmp_int[19], &tmp_int[20],
			&tmp_int[21], &tmp_int[22], &tmp_int[23],
			&tmp_int[24], &tmp_int[25],
			&tmp_int[27], &tmp_int[28], &tmp_int[29],
			&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			p.last_point.mapname, &tmp_int[35], &tmp_int[36], //
			p.save_point.mapname, &tmp_int[37], &tmp_int[38], &next) == 41 )
		{
			// Char structure of version 384 or older
			tmp_int[26] = 0; // pet id
			tmp_int[39] = 0; // partner id
			tmp_int[40] = 0; // father
			tmp_int[41] = 0; // mother
			tmp_int[42] = 0; // child
			tmp_int[43] = 0; // fame
			//ShowMessage("char: old char data ver.1\n");
		}
		else
		{
			ShowError(CL_BT_RED"Character line not recognized.\n"CL_NORM);
			ShowMessage("           Line saved in log file.""\n");
			return false;
		}
/* data for sql tables:

DROP TABLE IF EXISTS `char_characters`

CREATE TABLE IF NOT EXISTS `char_characters` (
  `char_id` 		INTEGER UNSIGNED NOT NULL auto_increment,
  `account_id`		INTEGER UNSIGNED NOT NULL default '0',
  `slot`			TINYINT UNSIGNED NOT NULL default '0',
  `name`			VARCHAR(24) NOT NULL default '',
  `class`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `base_level`		TINYINT UNSIGNED NOT NULL default '1',
  `job_level`		TINYINT UNSIGNED NOT NULL default '1',
  `base_exp`		BIGINT UNSIGNED NOT NULL default '0',
  `job_exp`			BIGINT UNSIGNED NOT NULL default '0',
  `zeny` 			BIGINT UNSIGNED NOT NULL default '500',
  `str` 			TINYINT UNSIGNED NOT NULL default '0',
  `agi` 			TINYINT UNSIGNED NOT NULL default '0',
  `vit` 			TINYINT UNSIGNED NOT NULL default '0',
  `int`				TINYINT UNSIGNED NOT NULL default '0',
  `dex` 			TINYINT UNSIGNED NOT NULL default '0',
  `luk`				TINYINT UNSIGNED NOT NULL default '0',
  `max_hp` 			MEDIUMINT UNSIGNED NOT NULL default '0',
  `hp`				MEDIUMINT UNSIGNED NOT NULL default '0',
  `max_sp`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `sp` 				MEDIUMINT UNSIGNED NOT NULL default '0',
  `status_point`	MEDIUMINT UNSIGNED NOT NULL default '0',
  `skill_point`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `option` 			INTEGER NOT NULL default '0',
  `karma`			INTEGER NOT NULL default '0',
  `manner`			INTEGER NOT NULL default '0',
  `party_id`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `guild_id`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `pet_id`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `hair`			TINYINT UNSIGNED NOT NULL default '0',
  `hair_color` 		TINYINT UNSIGNED NOT NULL default '0',
  `clothes_color`	TINYINT UNSIGNED NOT NULL default '0',
  `weapon`			MEDIUMINT UNSIGNED NOT NULL default '1',
  `shield`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `head_top`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `head_mid`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `head_bottom`		MEDIUMINT UNSIGNED NOT NULL default '0',
  `last_map`		VARCHAR(20) NOT NULL default 'new_5-1.gat',
  `last_x`			MEDIUMINT(3) UNSIGNED NOT NULL default '53',
  `last_y`			MEDIUMINT(3) UNSIGNED NOT NULL default '111',
  `save_map`		VARCHAR(20) NOT NULL default 'new_5-1.gat',
  `save_x`			MEDIUMINT(3) UNSIGNED NOT NULL default '53',
  `save_y`			MEDIUMINT(3) UNSIGNED NOT NULL default '111',
  `partner_id`		INTEGER UNSIGNED NOT NULL default '0',
  `father_id`		INTEGER UNSIGNED NOT NULL default '0',
  `mother_id`		INTEGER UNSIGNED NOT NULL default '0',
  `child_id`		INTEGER UNSIGNED NOT NULL default '0',
  `fame_points`		INTEGER UNSIGNED NOT NULL default '0',
  `online`			BOOL NOT NULL default 'false',
  PRIMARY KEY  (`char_id`),
  KEY `account_id` (`account_id`),
  KEY `party_id` (`party_id`),
  KEY `guild_id` (`guild_id`)
)

DROP TABLE IF EXISTS `char_reg_value`

CREATE TABLE IF NOT EXISTS `char_reg_value` (
  `char_id`			INTEGER UNSIGNED NOT NULL default '0',
  `str`				VARCHAR(255) NOT NULL default '',
  `value`			VARCHAR(255) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`str`),
  KEY `char_id` (`char_id`)
)

DROP TABLE IF EXISTS `char_friends`

CREATE TABLE IF NOT EXISTS `char_friends` (
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `friend_id`		INTEGER UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`friend_id`),
  KEY `char_id` (`char_id`)
)


DROP TABLE IF EXISTS `char_inventory`

CREATE TABLE IF NOT EXISTS `char_inventory` (
  `id`				BIGINT UNSIGNED NOT NULL auto_increment,
  `char_id`			INTEGER UNSIGNED NOT NULL default '0',
  `nameid`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `amount`			INTEGER UNSIGNED NOT NULL default '0',
  `equip`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `identify`		BOOL default 'TRUE',
  `refine`			TINYINT(2) UNSIGNED NOT NULL default '0',
  `attribute`		TINYINT UNSIGNED NOT NULL default '0',
  `card0` 			INTEGER NOT NULL default '0',
  `card1` 			INTEGER UNSIGNED NOT NULL default '0',
  `card2`			INTEGER UNSIGNED NOT NULL default '0',
  `card3`			INTEGER UNSIGNED NOT NULL default '0',
  `broken` 			BOOL default 'FALSE',
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
)

DROP TABLE IF EXISTS `char_cart`

CREATE TABLE IF NOT EXISTS `char_cart` (
  `id`				BIGINT UNSIGNED NOT NULL auto_increment,
  `char_id`			INTEGER UNSIGNED NOT NULL default '0',
  `nameid`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `amount`			INTEGER UNSIGNED NOT NULL default '0',
  `equip`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `identify`		BOOL default 'TRUE',
  `refine`			TINYINT(2) UNSIGNED NOT NULL default '0',
  `attribute`		TINYINT UNSIGNED NOT NULL default '0',
  `card0` 			INTEGER NOT NULL default '0',
  `card1` 			INTEGER UNSIGNED NOT NULL default '0',
  `card2`			INTEGER UNSIGNED NOT NULL default '0',
  `card3`			INTEGER UNSIGNED NOT NULL default '0',
  `broken` 			BOOL default 'FALSE',
  PRIMARY KEY  (`id`),
  KEY `char_id` (`char_id`)
)

DROP TABLE IF EXISTS `char_storage`

CREATE TABLE IF NOT EXISTS `char_storage` (
  `id`				BIGINT UNSIGNED NOT NULL auto_increment,
  `account_id`		INTEGER UNSIGNED NOT NULL default '0',
  `nameid`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `amount`			INTEGER UNSIGNED NOT NULL default '0',
  `equip`			MEDIUMINT UNSIGNED NOT NULL default '0',
  `identify`		BOOL default 'TRUE',
  `refine`			TINYINT(2) UNSIGNED NOT NULL default '0',
  `attribute`		TINYINT UNSIGNED NOT NULL default '0',
  `card0` 			INTEGER NOT NULL default '0',
  `card1` 			INTEGER UNSIGNED NOT NULL default '0',
  `card2`			INTEGER UNSIGNED NOT NULL default '0',
  `card3`			INTEGER UNSIGNED NOT NULL default '0',
  `broken` 			BOOL default 'FALSE',
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
)


DROP TABLE IF EXISTS `char_memo`

CREATE TABLE IF NOT EXISTS `char_memo` (
  `memo_id`			TINYINT UNSIGNED NOT NULL default '0',
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `map` 			VARCHAR(20) NOT NULL default '',
  `x` 				MEDIUMINT(3) UNSIGNED NOT NULL default '0',
  `y`				MEDIUMINT(3) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`memo_id`),
  KEY `char_id` (`char_id`)
)


DROP TABLE IF EXISTS `char_skill`

CREATE TABLE IF NOT EXISTS `char_skill` (
  `char_id` 		INTEGER UNSIGNED NOT NULL default '0',
  `id`				MEDIUMINT UNSIGNED NOT NULL default '0',
  `lv` 				TINYINT UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`id`),
  KEY `char_id` (`char_id`)
)

	will continue when i have more rest.

*/


		p.char_id = tmp_int[0];
		p.account_id = tmp_int[1];
		p.slot = tmp_int[2];
		p.class_ = tmp_int[3];
		p.base_level = tmp_int[4];
		p.job_level = tmp_int[5];
		p.base_exp = tmp_int[6];
		p.job_exp = tmp_int[7];
		p.zeny = tmp_int[8];
		p.hp = tmp_int[9];
		p.max_hp = tmp_int[10];
		p.sp = tmp_int[11];
		p.max_sp = tmp_int[12];
		p.str = tmp_int[13];
		p.agi = tmp_int[14];
		p.vit = tmp_int[15];
		p.int_ = tmp_int[16];
		p.dex = tmp_int[17];
		p.luk = tmp_int[18];
		p.status_point = tmp_int[19];
		p.skill_point = tmp_int[20];
		p.option = tmp_int[21];
		p.karma = GetByte(tmp_int[22],0);
		p.chaos = GetByte(tmp_int[22],1);
		p.manner = tmp_int[23];
		p.party_id = tmp_int[24];
		p.guild_id = tmp_int[25];
		p.pet_id = tmp_int[26];
		p.hair = tmp_int[27];
		p.hair_color = tmp_int[28];
		p.clothes_color = tmp_int[29];
		p.weapon = tmp_int[30];
		p.shield = tmp_int[31];
		p.head_top = tmp_int[32];
		p.head_mid = tmp_int[33];
		p.head_bottom = tmp_int[34];
		p.last_point.x = tmp_int[35];
		p.last_point.y = tmp_int[36];
		p.save_point.x = tmp_int[37];
		p.save_point.y = tmp_int[38];
		p.partner_id = tmp_int[39];
		p.father_id = tmp_int[40];
		p.mother_id = tmp_int[41];
		p.child_id = tmp_int[42];
		p.fame_points = tmp_int[43];

		size_t pos;
		if( cCharList.find(p, pos, 0) )
		{
			ShowError(CL_BT_RED"Character has an identical id to another.\n"CL_NORM);
			ShowMessage("           Character id #%ld -> new character not read.\n", (unsigned long)p.char_id);
			ShowMessage("           Character saved in log file.\n");
			return false;
		}
		else if( cCharList.find(p, pos, 1) )
		{
			ShowError(CL_BT_RED"Character name already exists.\n"CL_NORM);
			ShowMessage("           Character name '%s' -> new character not read.\n", p.name);
			ShowMessage("           Character saved in log file.\n");
			return false;
		}

		if( cAccountList.find( CCharCharAccount(p.account_id),0,pos) )
		{
			if( cAccountList[pos].charlist[p.slot] != 0 )
			{
				ShowError(CL_BT_RED"Character Slot already exists.\n"CL_NORM);
				ShowMessage("           Character name '%s' -> new character not read.\n", p.name);
				ShowMessage("           Character saved in log file.\n");
				return false;
			}
			else
			{
				cAccountList[pos].charlist[p.slot]=p.char_id;
			}
		}
		else
		{
			CCharCharAccount account(p.account_id);
			memset(account.charlist,0,sizeof(account.charlist));
			account.charlist[p.slot]=p.char_id;
			cAccountList.insert(account);
		}



		// 新規データ
		if (str[next] == '\n' || str[next] == '\r')
			return true;


		///////////////////////////////////////////////////////////////////////
		// more chaotic from here; code might look a bit weired
		bool ret = true;

		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'&&i<MAX_MEMO; i++)
			{
				if (sscanf(str+next, "%[^,],%d,%d%n", p.memo_point[i].mapname, &tmp_int[0], &tmp_int[1], &len) != 3)
				{
					ShowError(CL_BT_RED"Character Memo points invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}

				if( i<MAX_MEMO )
				{
					char*ip = strchr(p.memo_point[i].mapname, '.');
					if(ip) *ip=0;

					p.memo_point[i].x = tmp_int[0];
					p.memo_point[i].y = tmp_int[1];
				}
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}
		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'; i++)
			{
				if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
					&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
					&tmp_int[4], &tmp_int[5], &tmp_int[6],
					&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
				{
					// do nothing, it's ok
				}
				else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
						  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
						  &tmp_int[4], &tmp_int[5], &tmp_int[6],
						  &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11)
				{
					// do nothing, it's ok
				}
				else // invalid structure
				{
					ShowError(CL_BT_RED"Character Inventory invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}
				if( i<MAX_INVENTORY )
				{
					p.inventory[i].id = tmp_int[0];
					p.inventory[i].nameid = tmp_int[1];
					p.inventory[i].amount = tmp_int[2];
					p.inventory[i].equip = tmp_int[3];
					p.inventory[i].identify = tmp_int[4];
					p.inventory[i].refine = tmp_int[5];
					p.inventory[i].attribute = tmp_int[6];
					p.inventory[i].card[0] = tmp_int[7];
					p.inventory[i].card[1] = tmp_int[8];
					p.inventory[i].card[2] = tmp_int[9];
					p.inventory[i].card[3] = tmp_int[10];
				}
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}

		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'; i++)
			{
				if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
					&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
					&tmp_int[4], &tmp_int[5], &tmp_int[6],
					&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
				{
					// do nothing, it's ok
				}
				else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
						   &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
						   &tmp_int[4], &tmp_int[5], &tmp_int[6],
						   &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11)
				{
				}
				else // invalid structure
				{
					ShowError(CL_BT_RED"Character Cart Items invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}

				if( i<MAX_CART )
				{
					p.cart[i].id = tmp_int[0];
					p.cart[i].nameid = tmp_int[1];
					p.cart[i].amount = tmp_int[2];
					p.cart[i].equip = tmp_int[3];
					p.cart[i].identify = tmp_int[4];
					p.cart[i].refine = tmp_int[5];
					p.cart[i].attribute = tmp_int[6];
					p.cart[i].card[0] = tmp_int[7];
					p.cart[i].card[1] = tmp_int[8];
					p.cart[i].card[2] = tmp_int[9];
					p.cart[i].card[3] = tmp_int[10];
				}
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}

		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t'; i++) {
				if (sscanf(str + next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len) != 2)
				{
					ShowError(CL_BT_RED"Character skills invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}
				if(tmp_int[0] < MAX_SKILL )
				{
					p.skill[tmp_int[0]].id = tmp_int[0];
					p.skill[tmp_int[0]].lv = tmp_int[1];
				}
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}

		if(ret)
		{	// start with the next char after the delimiter
			unsigned long val;
			char str[32];
			next++;
			for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; i++)
			{	// global_reg実装以前のathena.txt互換のため一応'\n'チェック
				if(sscanf(str + next, "%32[^,],%ld%n", str, &val, &len) != 2)
				{	// because some scripts are not correct, the str can be "". So, we must check that.
					// If it's, we must not refuse the character, but just this REG value.
					// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
					if(str[next] == ',' && sscanf(str + next, ",%ld%n", &val, &len) == 1)
						i--;
					else
					{
						ShowError(CL_BT_RED"Character Char Variable invalid (id #%ld, name '%s').\n"CL_NORM, (unsigned long)p.char_id, p.name);
						ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
						ret = false;
						break;
					}

				}
				if( i<GLOBAL_REG_NUM )
				{
					safestrcpy(p.global_reg[i].str, str, sizeof(p.global_reg[i].str));
					p.global_reg[i].value = val;
				}
				next += len;
				if (str[next] == ' ')
					next++;
			}
			p.global_reg_num = i;
		}

		// insert the character to the list
		cCharList.insert(p);

		// check the next_char_id
		if(p.char_id >= next_char_id)
			next_char_id = p.char_id + 1;

		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// Function to read characters file
	bool read_chars(void)
	{
		char line[65536];
		int line_count;
		FILE *fp;


		fp = safefopen(char_txt, "r");
		if (fp == NULL)
		{
			ShowError("Characters file not found: %s.\n", char_txt);
	//		char_log("Characters file not found: %s." RETCODE, char_txt);
	//		char_log("Id for the next created character: %d." RETCODE, char_id_count);
			return false;
		}

		line_count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			unsigned long i;
			int j;
			line_count++;

			if( !get_prepared_line(line) )
				continue;
			line[sizeof(line)-1] = '\0';

			j = 0;
			if(sscanf(line, "%ld\t%%newid%%%n", &i, &j) == 1 && j > 0)
			{
				if(next_char_id < i)
					next_char_id = i;
				continue;
			}

			if( !char_from_str(line) )
			{
				// some error, message printed within
				//!! log line
				//char_log("%s", line);
			}

		}
		fclose(fp);

		if( cCharList.size() == 0 )
		{
			ShowError("No character found in %s.\n", char_txt);
			//char_log("No character found in %s." RETCODE, char_txt);
		}
		else if( cCharList.size() == 1 )
		{
			ShowStatus("1 character read in %s.\n", char_txt);
			//char_log("1 character read in %s." RETCODE, char_txt);
		}
		else
		{
			ShowStatus("mmo_char_init: %d characters read in %s.\n", cCharList.size(), char_txt);
			//char_log("mmo_char_init: %d characters read in %s." RETCODE, cList.size(), char_txt);
		}
		//char_log("Id for the next created character: %d." RETCODE, char_id_count);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// Function to save characters in files
	bool save_chars(void)
	{
		char line[65536];
		int lock;
		FILE *fp, *fb=NULL;

		if(backup_txt_flag)
			fb= lock_fopen(backup_txt, lock);

		// Data save
		fp = lock_fopen(char_txt, lock);
		if (fp == NULL)
		{
			ShowWarning("Server can't not save characters.\n");
			//char_log("Server can't not save characters." RETCODE);
			return false;
		}
		else
		{
			size_t i;
			for(i=0; i<cCharList.size(); i++)
			{
				char_to_str(line, sizeof(line), cCharList[i]);
				fprintf(fp, line); fprintf(fp, RETCODE);
				if(fb) { fprintf(fb, line); fprintf(fb, RETCODE); }
			}
			fprintf(fp, "%d\t%%newid%%" RETCODE, next_char_id);
			lock_fclose(fp, char_txt, lock);
			if(fb) lock_fclose(fp, backup_txt, lock);
			return true;
		}

	}


	///////////////////////////////////////////////////////////////////////////
	// Function to create a new character
	bool make_new_char(CCharCharAccount& account, const char *n, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color)
	{
		size_t pos;
		size_t i;
		CCharCharacter tempchar(n);

		if( remove_control_chars(tempchar.name) )
		{
			//char_log("Make new char error (control char received in the name): (connection #%d, account: %d)." RETCODE,
			//		 fd, sd->account_id);
			return false;
		}

		checktrim(tempchar.name);

		// check lenght of character name
		if( strlen(tempchar.name) < 4 )
		{
			//char_log("Make new char error (character name too small): (connection #%d, account: %d, name: '%s')." RETCODE,
			//		 fd, sd->account_id, dat);
			return false;
		}

		// Check Authorised letters/symbols in the name of the character
		if (char_name_option == 1)
		{	// only letters/symbols in char_name_letters are authorised
			for (i = 0; tempchar.name[i]; i++)
			{
				if( strchr(char_name_letters, tempchar.name[i]) == NULL )
				{
					//char_log("Make new char error (invalid letter in the name): (connection #%d, account: %d), name: %s, invalid letter: %c." RETCODE,
					//		 fd, sd->account_id, dat, dat[i]);
					return false;
				}
			}
		}
		else if (char_name_option == 2)
		{	// letters/symbols in char_name_letters are forbidden
			for (i = 0; tempchar.name[i]; i++)
				if (strchr(char_name_letters, tempchar.name[i]) != NULL)
				{
					//char_log("Make new char error (invalid letter in the name): (connection #%d, account: %d), name: %s, invalid letter: %c." RETCODE,
					//		 fd, sd->account_id, dat, dat[i]);
					return false;
				}
		} // else, all letters/symbols are authorised (except control char removed before)

		if( str<1 || str>9 ||					// stats single
			agi<1 || agi>9 ||
			vit<1 || vit>9 ||
			int_<1 || int_>9 ||
			dex<1 || dex>9 ||
			luk<1 || luk>9 ||
			str+int_ > 10 ||					// stats pair-wise
			agi+dex > 10 ||
			vit+luk > 10 ||
			str+agi+vit+int_+dex+luk != 5*6 ||	// stats summ
			slot >= 9 ||						// slots
			account.charlist[slot]!=0 ||
			hair_style >= 24 ||					// styles
			hair_style >= 9)
		{
			//char_log("Make new char error (invalid values): (connection #%d, account: %d) slot %d, name: %s, stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d" RETCODE,
			//		 fd, sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
			return false;
		}
//!! adding name_ignoring_case

		if( cCharList.find(tempchar, pos, 1) )
		{
			//char_log("Make new char error (name already exists): (connection #%d, account: %d) slot %d, name: %s (actual name of other char: %d), stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
			//		 fd, sd->account_id, dat[30], dat, char_dat[i].name, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
			return false;
		}

//!! testing wisp_server_name otherwise


		//char_log("Creation of New Character: (connection #%d, account: %d) slot %d, character Name: %s, stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
		//		 fd, sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);

		tempchar.char_id = next_char_id++;
		tempchar.account_id = account.account_id;
		tempchar.slot = slot;
		tempchar.class_ = 0;
		tempchar.base_level = 1;
		tempchar.job_level = 1;
		tempchar.base_exp = 0;
		tempchar.job_exp = 0;
		tempchar.zeny = start_zeny;
		tempchar.str = str;
		tempchar.agi = agi;
		tempchar.vit = vit;
		tempchar.int_ = int_;
		tempchar.dex = dex;
		tempchar.luk = luk;
		tempchar.max_hp = 40 * (100 + vit) / 100;
		tempchar.max_sp = 11 * (100 + int_) / 100;
		tempchar.hp = tempchar.max_hp;
		tempchar.sp = tempchar.max_sp;
		tempchar.status_point = 0;
		tempchar.skill_point = 0;
		tempchar.option = 0;
		tempchar.karma = 0;
		tempchar.manner = 0;
		tempchar.party_id = 0;
		tempchar.guild_id = 0;
		tempchar.hair = hair_style;
		tempchar.hair_color = hair_color;
		tempchar.clothes_color = 0;
		tempchar.inventory[0].nameid = start_weapon; // Knife
		tempchar.inventory[0].amount = 1;
		tempchar.inventory[0].equip = 0x02;
		tempchar.inventory[0].identify = 1;
		tempchar.inventory[1].nameid = start_armor; // Cotton Shirt
		tempchar.inventory[1].amount = 1;
		tempchar.inventory[1].equip = 0x10;
		tempchar.inventory[1].identify = 1;
		tempchar.weapon = 1;
		tempchar.shield = 0;
		tempchar.head_top = 0;
		tempchar.head_mid = 0;
		tempchar.head_bottom = 0;
		tempchar.last_point = start_point;
		tempchar.save_point = start_point;


		account.charlist[slot] = tempchar.char_id;
		cCharList.insert(tempchar);

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	bool save_friends()
	{
		FILE *fp = safefopen(friends_txt, "wb");
		if(fp)
		{
			size_t i, k;
			for(i=0; i<cCharList.size(); i++)
			{
				for (k=0;k<MAX_FRIENDLIST;k++)
				{
					if(cCharList[i].friendlist[k].friend_id > 0 && cCharList[i].friendlist[k].friend_name[0])
						break;
				}
				if(k<MAX_FRIENDLIST)
				{	// at least one friend exist
					fprintf(fp, "%ld", (unsigned long)cCharList[i].char_id);
					for (k=0;k<MAX_FRIENDLIST;k++)
					{
						if (cCharList[i].friendlist[k].friend_id > 0 && cCharList[i].friendlist[k].friend_name[0])
							fprintf(fp, ",%ld,%s", (unsigned long)cCharList[i].friendlist[k].friend_id,cCharList[i].friendlist[k].friend_name);
						else
							fprintf(fp,",,");
					}
					fprintf(fp, "\n");
				}
			}
			fclose(fp);
			return true;
		}
		return false;
	}
	///////////////////////////////////////////////////////////////////////////
	bool read_friends()
	{
		char line[1024];

		FILE *fp = safefopen(friends_txt, "rb");
		if(fp)
		{
			unsigned long cid=0;
			unsigned long fid[20];
			struct friends friendlist[20];
			size_t pos;

			while(fgets(line, sizeof(line), fp))
			{
				if( !get_prepared_line(line) )
					continue;

				memset(friendlist,0,sizeof(friendlist));
				sscanf(line, "%ld,%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%s",
					&cid,
				&fid[ 0],friendlist[ 0].friend_name,
				&fid[ 1],friendlist[ 1].friend_name,
				&fid[ 2],friendlist[ 2].friend_name,
				&fid[ 3],friendlist[ 3].friend_name,
				&fid[ 4],friendlist[ 4].friend_name,
				&fid[ 5],friendlist[ 5].friend_name,
				&fid[ 6],friendlist[ 6].friend_name,
				&fid[ 7],friendlist[ 7].friend_name,
				&fid[ 8],friendlist[ 8].friend_name,
				&fid[ 9],friendlist[ 9].friend_name,
				&fid[10],friendlist[10].friend_name,
				&fid[11],friendlist[11].friend_name,
				&fid[12],friendlist[12].friend_name,
				&fid[13],friendlist[13].friend_name,
				&fid[14],friendlist[14].friend_name,
				&fid[15],friendlist[15].friend_name,
				&fid[16],friendlist[16].friend_name,
				&fid[17],friendlist[17].friend_name,
				&fid[18],friendlist[18].friend_name,
				&fid[19],friendlist[19].friend_name);
				// cannot give the pointer to the struct member since the type can vary plattform dependend

				if( cCharList.find( CCharCharacter(cid), pos, 0) )
				{
					CCharCharacter &temp = cCharList(pos,0);
					size_t i;
					for (i=0; i<MAX_FRIENDLIST; i++)
					{
						temp.friendlist[i] = friendlist[i];
						temp.friendlist[i].friend_id = fid[i];

					}
				}
			}//end while
			fclose(fp);
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// config stuff
	uint32 next_char_id;

	char char_txt[1024];
	char backup_txt[1024];
	char friends_txt[1024];
	bool backup_txt_flag;

	bool name_ignoring_case;
	int char_name_option;
	char char_name_letters[256];
	uint32 start_zeny;
	unsigned short start_weapon;
	unsigned short start_armor;
	struct point start_point;

	size_t savecount;

	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CCharCharacter, 2>	cCharList;
	TslistDCT<CCharCharAccount>		cAccountList;
	simple_database					cMailDB;
	///////////////////////////////////////////////////////////////////////////
	// data for alternative interface
	Mutex	cMx;
	size_t	cPos;

public:
	CCharDB_txt(const char *dbcfgfile) :
		CTimerBase(300*1000),		// 300sec save interval
		cPos(0)
	{
		next_char_id = 150000;
		safestrcpy(char_txt, "save/athena.txt", sizeof(char_txt));
		safestrcpy(backup_txt, "save/backup.txt", sizeof(backup_txt));
		safestrcpy(friends_txt, "save/friends.txt", sizeof(friends_txt));

		backup_txt_flag=0;

		char_name_option=0;
		memset(char_name_letters,0,sizeof(char_name_letters));

		name_ignoring_case=0;

		start_zeny = 500;
		start_weapon = 1201;
		start_armor = 2301;
		safestrcpy(start_point.mapname, "new_1-1", sizeof(start_point.mapname));
		start_point.x=53;
		start_point.x=111;

		init(dbcfgfile);
	}
	~CCharDB_txt()	{ close(); }

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return cCharList.size(); }
	virtual CCharCharacter& operator[](size_t i)	{ return cCharList[i]; }

	virtual bool existChar(const char* name);
	virtual bool searchChar(const char* name, CCharCharacter&data);
	virtual bool searchChar(uint32 charid, CCharCharacter&data);
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data);
	virtual bool removeChar(uint32 charid);
	virtual bool saveChar(const CCharCharacter& data);

	virtual bool searchAccount(uint32 accid, CCharCharAccount& account);
	virtual bool saveAccount(CCharAccount& account);
	virtual bool removeAccount(uint32 accid);


	///////////////////////////////////////////////////////////////////////////
	// mail access interface
/*
	size_t tostring(char*&str, const uint32 val)
	{
		if(str)
		{
			*str++ = ((val>>0x1C)&0xF) + ( (((val>>0x1C)&0xF)<10) ? '0' : 'A'-10 );
			*str++ = ((val>>0x18)&0xF) + ( (((val>>0x18)&0xF)<10) ? '0' : 'A'-10 );
			*str++ = ((val>>0x14)&0xF) + ( (((val>>0x14)&0xF)<10) ? '0' : 'A'-10 );
			*str++ = ((val>>0x10)&0xF) + ( (((val>>0x10)&0xF)<10) ? '0' : 'A'-10 );
			*str++ = ((val>>0x0C)&0xF) + ( (((val>>0x0C)&0xF)<10) ? '0' : 'A'-10 );
			*str++ = ((val>>0x08)&0xF) + ( (((val>>0x08)&0xF)<10) ? '0' : 'A'-10 );
			*str++ = ((val>>0x04)&0xF) + ( (((val>>0x04)&0xF)<10) ? '0' : 'A'-10 );
			*str++ = ((val>>0x00)&0xF) + ( (((val>>0x00)&0xF)<10) ? '0' : 'A'-10 );
			return 8;
		}
		return 0;
	}
	size_t fromstring(const char*&str, uint32 &val)
	{
		if(str)
		{
			val =	(((uint32)(str[0]>='0' && str[0]<='9') ? (str[0]-'0') : ( (str[0]>='A' && str[0]<='F') ? (str[0]-'A'+10) : ( (str[0]>='a' && str[0]<='f') ? (str[0]-'a'+10) : 0 ))) << 0x1C) |
					(((uint32)(str[1]>='0' && str[1]<='9') ? (str[1]-'0') : ( (str[1]>='A' && str[1]<='F') ? (str[1]-'A'+10) : ( (str[1]>='a' && str[1]<='f') ? (str[1]-'a'+10) : 0 ))) << 0x18) |
					(((uint32)(str[2]>='0' && str[2]<='9') ? (str[2]-'0') : ( (str[2]>='A' && str[2]<='F') ? (str[2]-'A'+10) : ( (str[2]>='a' && str[2]<='f') ? (str[2]-'a'+10) : 0 ))) << 0x14) |
					(((uint32)(str[3]>='0' && str[3]<='9') ? (str[3]-'0') : ( (str[3]>='A' && str[3]<='F') ? (str[3]-'A'+10) : ( (str[3]>='a' && str[3]<='f') ? (str[3]-'a'+10) : 0 ))) << 0x10) |
					(((uint32)(str[4]>='0' && str[4]<='9') ? (str[4]-'0') : ( (str[4]>='A' && str[4]<='F') ? (str[4]-'A'+10) : ( (str[4]>='a' && str[4]<='f') ? (str[4]-'a'+10) : 0 ))) << 0x0C) |
					(((uint32)(str[5]>='0' && str[5]<='9') ? (str[5]-'0') : ( (str[5]>='A' && str[5]<='F') ? (str[5]-'A'+10) : ( (str[5]>='a' && str[5]<='f') ? (str[5]-'a'+10) : 0 ))) << 0x08) |
					(((uint32)(str[6]>='0' && str[6]<='9') ? (str[6]-'0') : ( (str[6]>='A' && str[6]<='F') ? (str[6]-'A'+10) : ( (str[6]>='a' && str[6]<='f') ? (str[6]-'a'+10) : 0 ))) << 0x04) |
					(((uint32)(str[7]>='0' && str[7]<='9') ? (str[7]-'0') : ( (str[7]>='A' && str[7]<='F') ? (str[7]-'A'+10) : ( (str[7]>='a' && str[7]<='f') ? (str[7]-'a'+10) : 0 ))) << 0x00) ;
			str+=8;
			return 8;
		}
		else
		{
			val = 0;
			return 0;
		}
	}
	size_t tostring(char*&str, const char* val, size_t sz, char stop='\t')
	{
		if(str)
		{	char *strtmp=str;
			if(val)
			{
				const char*ip=val;
				while( *ip && (val+sz-1 > ip) )
				{
					if(*ip == stop)
						*str++ =' ', ip++;
					else
						*str++ = *ip++;
				}
			}
			*str=0;
			return str-strtmp;
		}
		return 0;
	}
	size_t fromstring(const char*&str, char* val, size_t sz, char stop='\t')
	{
		if(val)
		{
			char*ip=val;
			if(str)
			{
				while( *str && (*str!=stop) && (val+sz-1 > ip) )
				{
					*ip++ = *str++;
				}
			}
			*ip=0;
			return ip-val;
		}
		return 0;
	}
	size_t mail_tostring(char*string, uint32 mid, unsigned char read, uint32 sid, const char*sname, uint32 tid, const char* tname, const char* head, const char* body)
	{
		if(string)
		{
			char* ip = string;
			tostring(ip, mid);
			*ip++='\t';
			*ip++='0'+((read<10)?read:9);
			*ip++='\t';
			tostring(ip, sid);
			*ip++='\t';
			tostring(ip, sname, 24);
			*ip++='\t';
			tostring(ip, tid);
			*ip++='\t';
			tostring(ip, tname, 24);
			*ip++='\t';
			tostring(ip, head, 32);
			*ip++='\t';
			tostring(ip, body, 80);
			*ip++='\t';
			*ip++='\n';
			*ip=0;
			return ip-string;
		}
		return 0;
	}
	size_t mail_fromstring(const char*string, uint32 &mid, unsigned char &read, uint32 &sid, char*sname, uint32 &tid, char* tname, char* head, char* body)
	{
		if(string)
		{
			const char* ip = string;
			fromstring(ip, mid); 
			while(*ip && *ip!='\t') ip++; ip++;
			read = (*ip>='0' && *ip<='9') ? (*ip-'0') : ( (*ip>='A' && *ip<='F') ? (*ip-'A'+10) : ( (*ip>='a' && *ip<='f') ? (*ip-'a'+10) : 0 ) );
			while(*ip && *ip!='\t') ip++; ip++;
			fromstring(ip, sid);
			while(*ip && *ip!='\t') ip++; ip++;
			fromstring(ip, sname, 24);
			while(*ip && *ip!='\t') ip++; ip++;
			fromstring(ip, tid);
			while(*ip && *ip!='\t') ip++; ip++;
			fromstring(ip, tname, 24);
			while(*ip && *ip!='\t') ip++; ip++;
			fromstring(ip, head, 32);
			while(*ip && *ip!='\t') ip++; ip++;
			fromstring(ip, body, 80);
			return ip-string;
		}
		return 0;
	}
*/
	virtual size_t getMailCount(uint32 cid, uint32 &all, uint32 &unread)
	{
		ScopeLock sl(cMx);
		simple_database::iterator iter(cMailDB);
		
		unread=all=0;
		while(iter)
		{
			if( iter.Key2()==cid )
			{
				all++;
				if( iter.Flag()==0 )
					unread++;
			}
			// next
			++iter;
		}
		return all;
	}

	virtual size_t listMail(uint32 cid, unsigned char box, unsigned char *buffer)
	{
		ScopeLock sl(cMx);
		simple_database::iterator iter(cMailDB);
		char buf[1024]="";
		unsigned long mmid, tid, sid;
		size_t count=0;
		char sname[32], tname[32], head[32], body[80];
		unsigned char read;
		while(iter)
		{
			if( iter.Key2()==cid &&
				(!iter.Flag() || box) &&
				iter.read(buf, sizeof(buf)) &&
				//!! replace the whole thing with regex
				8==sscanf(buf,
					"%lu\t%c\t%lu\t%24[^\t]\t%lu\t%24[^\t]\t%32[^\t]\t%80[^\t]\t\n",
					&mmid, &read, &sid, sname, &tid, tname, head, body) 
				)
			{
				CMailHead mh(mmid, iter.Flag(), sname, head);
				mh._tobuffer(buffer);// automatic buffer increment
				count++;
			}
			// next element
			++iter;
		}
		return count;
	}

	virtual bool readMail(uint32 cid, uint32 mid, CMail& mail)
	{
		ScopeLock sl(cMx);
		bool ret = false;
		char buffer[1024], sname[32], tname[32];
		unsigned long mmid=0, tid=0, sid=0;
		int read;
		simple_database::iterator iter(cMailDB, mid, cid);

		ret = ( iter.isValid() &&
				iter.read(buffer, sizeof(buffer)) &&
				//!! replace the whole thing with regex
				8==sscanf(buffer,
					"%lu\t%i\t%lu\t%24[^\t]\t%lu\t%24[^\t]\t%32[^\t]\t%80[^\t]\t\n",
					&mmid, &read, &sid, sname, &tid, tname, mail.head, mail.body) &&
				mid==mmid && ( cid==tid || cid==sid) );

		if(ret)
		{	
			safestrcpy(mail.name, (cid==tid)?sname:tname, sizeof(mail.name));
			mail.msid = mmid;
			mail.read = iter.Flag();
			if( !mail.read && cid==tid )
			{	// update readflag of own unread mails				
				size_t sz = snprintf(buffer, sizeof(buffer),
					"%lu\t%c\t%lu\t%.24s\t%lu\t%.24s\t%.32s\t%.80s\t\n",
					(unsigned long)mmid, '1', 
					(unsigned long)sid, sname,
					(unsigned long)tid, mail.name,
					mail.head, mail.body);
				// will update entry and index
				cMailDB.insert(mmid, tid, buffer, sz);
				// update readflag
				iter.Flag() = mail.read = 1;
			}
		}
		else
		{
			mail.msid=mail.read = mail.name[0] = mail.head[0] = mail.body[0] = 0;
		}
		return ret;
	}
	virtual bool deleteMail(uint32 cid, uint32 mid)
	{
		ScopeLock sl(cMx);
		return cMailDB.remove(mid, cid);
	}
	virtual bool sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32& msgid, uint32& tid)
	{
		ScopeLock sl(cMx);
		bool ret = false;
		size_t pos;
		// search in index 1
		if( cCharList.find( CCharCharacter(targetname), pos, 1) &&
			cCharList[pos].char_id!=senderid )
		{
			char buffer[1024];
			simple_database::iterator iter(cMailDB, true);
			uint32 targetid = cCharList(pos,1).char_id;
			uint32 mid = cMailDB.getfreekey();
			if(mid)
			{
				char sname[24], tname[24], h[32], b[80];
				replacecpy(sname, sendername, 24);
				replacecpy(tname, targetname, 24);
				replacecpy(h,     head,       32);
				replacecpy(b,     body,       80);
				
				// sscanf cannot handle empty/whitespaced strings
				// so just put in something harmless
				if(sname[0]==0) strcpy(sname, ".");
				if(tname[0]==0) strcpy(tname, ".");
				if(h[0]==0) strcpy(h, ".");
				if(b[0]==0) strcpy(b, ".");

				size_t sz = snprintf(buffer, sizeof(buffer),
					"%lu\t%c\t%lu\t%.24s\t%lu\t%.24s\t%.32s\t%.80s\t\n",
					(unsigned long)mid, '0', 
					(unsigned long)senderid, sname,
					(unsigned long)targetid, tname,
					h, b);

				msgid = mid;
				tid   = targetid;
				ret = cMailDB.insert(mid, tid, buffer, sz);
			}
		}
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquire()
	{
		cMx.lock();
		return this->first();
	}
	virtual bool release()
	{
		cPos=0;
		cMx.unlock();
		return true;
	}
	virtual bool first()
	{
		ScopeLock sl(cMx);
		cPos=0;
		return this->operator bool();
	}
	virtual operator bool()					
	{
		ScopeLock sl(cMx);
		return cCharList.size() > cPos;
	}
	virtual bool operator++(int)
	{
		ScopeLock sl(cMx);
		cPos++;
		return this->operator bool();
	}
	virtual bool save()
	{
		return true; 
	}

	virtual bool find(const char* name)
	{
		ScopeLock sl(cMx);
		size_t pos;
		// search in index 1
		if( cCharList.find( CCharCharacter(name), pos, 1) )
		{	// set position based to index 0 
			return cCharList.find( cCharList(pos,1), cPos, 0);
		}
		return false; 

	}
	virtual bool find(uint32 charid)
	{
		ScopeLock sl(cMx);
		return cCharList.find( CCharCharacter(charid), cPos, 0);
	}
	virtual CCharCharacter& operator()()
	{
		return cCharList[cPos]; 
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2);

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		cMailDB.open("save/mail");
		return read_chars() && read_friends();
	}
	bool close()
	{
		cMailDB.close();
		return save_chars() && save_friends();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 100 )
		{
			savecount=0;
			cMailDB.flush(true);
			save_chars();
			save_friends();
		}

		//!! todo check changes in files and reload them if necessary

		return true;
	}
};
bool CCharDB_txt::ProcessConfig(const char*w1, const char*w2)
{
	if(strcasecmp(w1, "char_txt") == 0)
	{
		safestrcpy(char_txt, w2, sizeof(char_txt));
	}
	else if(strcasecmp(w1, "backup_txt") == 0)
	{
		safestrcpy(backup_txt, w2, sizeof(backup_txt));
	}
	else if(strcasecmp(w1, "friends_txt") == 0)
	{
		safestrcpy(friends_txt, w2, sizeof(friends_txt));
	}
	else if(strcasecmp(w1, "backup_txt_flag") == 0)
	{
		backup_txt_flag = Switch(w2);
	}
	else if(strcasecmp(w1, "autosave_time") == 0)
	{
		// less then 10 seconds and more than an hour is not realistic
		CTimerBase::init( SwitchValue(w2, 10, 3600)*1000 );
	}
	else if(strcasecmp(w1, "start_point") == 0)
	{
		char mapname[32];
		int x, y;
		if(sscanf(w2, "%[^,],%d,%d", mapname, &x, &y) == 3 )
		{	
			char *ip=strchr(mapname, '.');
			if( ip != NULL ) *ip=0;
			safestrcpy(start_point.mapname, mapname, sizeof(start_point.mapname));
			start_point.x = x;
			start_point.y = y;
		}
	}
	else if(strcasecmp(w1, "start_zeny") == 0)
	{
		start_zeny = SwitchValue(w2,0);
	}
	else if(strcasecmp(w1, "start_weapon") == 0)
	{
		start_weapon = SwitchValue(w2,0);
	}
	else if(strcasecmp(w1, "start_armor") == 0)
	{
		start_armor = SwitchValue(w2,0);
	}
	else if(strcasecmp(w1, "name_ignoring_case") == 0)
	{
		name_ignoring_case = Switch(w2);
	}
	else if(strcasecmp(w1, "char_name_option") == 0)
	{
		char_name_option = atoi(w2);
	}
	else if(strcasecmp(w1, "char_name_letters") == 0)
	{
		safestrcpy(char_name_letters, w2, sizeof(char_name_letters));
	}
	return true;
}

bool CCharDB_txt::existChar(const char* name)
{
	size_t pos;
	return cCharList.find( CCharCharacter(name), pos, 1);
}
bool CCharDB_txt::searchChar(const char* name, CCharCharacter&data)
{
	size_t pos;
	if( cCharList.find( CCharCharacter(name), pos, 1) )
	{
		data = cCharList[pos];
		return true;
	}
	return false;
}
bool CCharDB_txt::searchChar(uint32 charid, CCharCharacter&data)
{
	size_t pos;
	if( cCharList.find( CCharCharacter(charid), pos, 0) )
	{
		data = cCharList[pos];
		return true;
	}
	return false;
}
bool CCharDB_txt::insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data)
{
	size_t pos;
	if( cAccountList.find(account,0,pos) &&
		make_new_char(cAccountList[pos], name, str, agi, vit, int_, dex, luk, slot, hair_style, hair_color) )
	{
		account = cAccountList[pos];
		return searchChar(name, data);
	}
	return false;
}
bool CCharDB_txt::removeChar(uint32 charid)
{
	size_t posc, posa;
	if( cCharList.find(CCharCharacter(charid),posc, 0) )
	{
		if( cAccountList.find(CCharAccount(cCharList[posc].account_id),0,posa) )
		{
			if(cCharList[posc].slot>=9 || cAccountList[posa].charlist[cCharList[posc].slot]!=cCharList[posc].char_id)
				ShowWarning("inconsistent account-character map\n");
			else
				cAccountList[posa].charlist[cCharList[posc].slot] = 0;
		}
		return cCharList.removeindex(posc, 0);
	}
	return false;
}
bool CCharDB_txt::saveChar(const CCharCharacter& data)
{
	size_t pos;
	if( cCharList.find( data, pos, 0) )
	{
		cCharList[pos] = data;
		return true;
	}
	return false;
}
bool CCharDB_txt::searchAccount(uint32 accid, CCharCharAccount& account)
{
	size_t pos;
	if( cAccountList.find(CCharCharAccount(accid),0,pos) )
	{
		account = cAccountList[pos];
		return true;
	}
	return false;
}
bool CCharDB_txt::saveAccount(CCharAccount& account)
{
	size_t pos;
	if( cAccountList.find(account,0,pos) )
	{	// exist -> update list entry
		cAccountList[pos].CCharAccount::operator=(account);
		return true;
	}
	else
	{	// create new
		return cAccountList.insert(account);
	}
}
bool CCharDB_txt::removeAccount(uint32 accid)
{
	size_t pos;
	if( cAccountList.find(CCharAccount(accid),0,pos) )
	{	// exist -> update list entry
		cAccountList.removeindex(pos);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
#else// SQL
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
#endif// SQL
///////////////////////////////////////////////////////////////////////////////

CCharDBInterface* CCharDB::getDB(const char *dbcfgfile)
{
#ifdef TXT_ONLY
	return new CCharDB_txt(dbcfgfile);
#else
	return NULL;
//	return new CCharDB_sql(dbcfgfile);
#endif// SQL
}










///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////

class CGuildDB_txt : public CTimerBase, private CConfig, public CGuildDBInterface
{
	bool string2guild(const char *str, CGuild &g)
	{
		int i, j, c;
		int tmp_int[16];
		char tmp_str[4][256];
		char tmp_str2[4096];
		char *pstr;

		// 基本データ
		if( 8 > sscanf(str,
			"%d\t%[^\t]\t%[^\t]\t%d,%d,%d,%d,%d\t%[^\t]\t%[^\t]\t",
			&tmp_int[0],
			tmp_str[0], tmp_str[1],
			&tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5],
			tmp_str[2], tmp_str[3]) )
		{
			return false;
		}

		g.guild_id = tmp_int[0];
		g.guild_lv = tmp_int[1];
		g.max_member = tmp_int[2];
		g.exp = tmp_int[3];
		g.skill_point = tmp_int[4];
		//g.castle_id = tmp_int[5]; just skip it
		safestrcpy(g.name, tmp_str[0], sizeof(g.name));
		safestrcpy(g.master, tmp_str[1], sizeof(g.master));
		safestrcpy(g.mes1, tmp_str[2], sizeof(g.mes1));
		safestrcpy(g.mes2, tmp_str[3], sizeof(g.mes2));

		for(j=0; j<6 && str!=NULL; j++)	// 位置スキップ
			str = strchr(str + 1, '\t');

		// メンバー
		if(g.max_member>MAX_GUILD)
			g.max_member=0;
		memset(&g.member,0, sizeof(g.member));
		for(i=0; i<g.max_member; i++)
		{
			if( sscanf(str+1, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\t%[^\t]\t",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4],
				&tmp_int[5], &tmp_int[6], &tmp_int[7], &tmp_int[8], &tmp_int[9],
				tmp_str[0]) < 11)
				return false;

			g.member[i].account_id	= tmp_int[0];
			g.member[i].char_id		= tmp_int[1];
			g.member[i].hair		= tmp_int[2];
			g.member[i].hair_color	= tmp_int[3];
			g.member[i].gender		= tmp_int[4];
			g.member[i].class_		= tmp_int[5];
			g.member[i].lv			= tmp_int[6];
			g.member[i].exp			= tmp_int[7];
			g.member[i].exp_payper	= tmp_int[8];
			g.member[i].position	= tmp_int[9];
			safestrcpy(g.member[i].name, tmp_str[0], sizeof(g.member[i].name));

			for(j=0; j<2 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str+1, '\t');
		}

		// 役職
		i = 0;
		memset(&g.position,0, sizeof(g.position));
		while( sscanf(str+1, "%d,%d%n", &tmp_int[0], &tmp_int[1], &j) == 2 &&
			str[1+j] == '\t')
		{
			if( sscanf(str+1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str[0]) < 3)
				return false;
			g.position[i].mode		= tmp_int[0];
			g.position[i].exp_mode	= tmp_int[1];
			safestrcpy(g.position[i].name, tmp_str[0], sizeof(g.position[i].name));

			for(j=0; j<2 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str+1, '\t');
			i++;
		}

		// エンブレム
		tmp_int[1] = 0;
		if( sscanf(str + 1, "%d,%d,%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str2)< 3 &&
			sscanf(str + 1, "%d,%[^\t]\t", &tmp_int[0], tmp_str2) < 2 )
			return false;
		g.emblem_len = tmp_int[0];
		g.emblem_id = tmp_int[1];
		for(i=0, pstr=tmp_str2; i<g.emblem_len; i++, pstr+=2)
		{
			int c1 = pstr[0], c2 = pstr[1], x1 = 0, x2 = 0;
			if (c1 >= '0' && c1 <= '9') x1 = c1 - '0';
			if (c1 >= 'a' && c1 <= 'f') x1 = c1 - 'a' + 10;
			if (c1 >= 'A' && c1 <= 'F') x1 = c1 - 'A' + 10;
			if (c2 >= '0' && c2 <= '9') x2 = c2 - '0';
			if (c2 >= 'a' && c2 <= 'f') x2 = c2 - 'a' + 10;
			if (c2 >= 'A' && c2 <= 'F') x2 = c2 - 'A' + 10;
			g.emblem_data[i] = (x1<<4) | x2;
		}

		str=strchr(str+1, '\t');	// 位置スキップ

		// 同盟リスト
		if (sscanf(str+1, "%d\t", &c) < 1)
			return false;

		str = strchr(str + 1, '\t');	// 位置スキップ
		memset(&g.alliance,0, sizeof(g.alliance));
		for(i = 0; i < c; i++)
		{
			if( sscanf(str + 1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str[0]) < 3)
				return false;
			g.alliance[i].guild_id		= tmp_int[0];
			g.alliance[i].opposition	= tmp_int[1];
			safestrcpy(g.alliance[i].name, tmp_str[0], sizeof(g.alliance[i].name));

			for(j=0; j<2 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str + 1, '\t');
		}

		// 追放リスト
		if (sscanf(str+1, "%d\t", &c) < 1)
			return false;

		str = strchr(str + 1, '\t');	// 位置スキップ
		memset(&g.explusion,0, sizeof(g.explusion));
		for(i=0; i<c; i++)
		{
			if( sscanf(str + 1, "%d,%d,%d,%d\t%[^\t]\t%[^\t]\t%[^\t]\t",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				tmp_str[0], tmp_str[1], tmp_str[2]) < 6)
				return false;
			g.explusion[i].account_id = tmp_int[0];
			g.explusion[i].rsv1 = tmp_int[1];
			g.explusion[i].rsv2 = tmp_int[2];
			g.explusion[i].rsv3 = tmp_int[3];
			safestrcpy(g.explusion[i].name, tmp_str[0], sizeof(g.explusion[i].name));
			safestrcpy(g.explusion[i].acc, tmp_str[1], sizeof(g.explusion[i].acc));
			safestrcpy(g.explusion[i].mes, tmp_str[2], sizeof(g.explusion[i].mes));

			for(j=0; j<4 && str!=NULL; j++)	// 位置スキップ
				str = strchr(str+1, '\t');
		}

		// ギルドスキル
		memset(&g.skill,0, sizeof(g.skill));
		for(i=0; i<MAX_GUILDSKILL; i++)
		{
			if (sscanf(str+1,"%d,%d ", &tmp_int[0], &tmp_int[1]) < 2)
				break;
			g.skill[i].id = tmp_int[0];
			g.skill[i].lv = tmp_int[1];
			str = strchr(str+1, ' ');
		}
		str = strchr(str + 1, '\t');

		return true;
	}

	ssize_t guild2string(char *str, size_t maxlen, const CGuild &g)
	{
		ssize_t i, c, len;

		// 基本データ
		len = snprintf(str, maxlen, "%ld\t%s\t%s\t%d,%d,%ld,%d,%d\t%s#\t%s#\t",
					  (unsigned long)g.guild_id, g.name, g.master,
					  g.guild_lv, g.max_member, (unsigned long)g.exp, g.skill_point, 0,//g.castle_id,
					  g.mes1, g.mes2);
		// メンバー
		for(i = 0; i < g.max_member; i++) {
			const struct guild_member &m = g.member[i];
			len += snprintf(str + len, maxlen-len, "%ld,%ld,%d,%d,%d,%d,%d,%ld,%ld,%d\t%s\t",
						   (unsigned long)m.account_id, (unsigned long)m.char_id,
						   m.hair, m.hair_color, m.gender,
						   m.class_, m.lv, (unsigned long)m.exp, (unsigned long)m.exp_payper, m.position,
						   ((m.account_id > 0) ? m.name : "-"));
		}
		// 役職
		for(i = 0; i < MAX_GUILDPOSITION; i++) {
			const struct guild_position &p = g.position[i];
			len += snprintf(str + len, maxlen-len, "%ld,%ld\t%s#\t", (unsigned long)p.mode, (unsigned long)p.exp_mode, p.name);
		}
		// エンブレム
		len += snprintf(str + len, maxlen-len, "%d,%ld,", g.emblem_len, (unsigned long)g.emblem_id);
		for(i = 0; i < g.emblem_len; i++) {
			len += snprintf(str + len, maxlen-len, "%02x", (unsigned char)(g.emblem_data[i]));
		}
		len += snprintf(str + len, maxlen-len, "$\t");
		// 同盟リスト
		
		for(i=0, c=0; i<MAX_GUILDALLIANCE; i++)
			if(g.alliance[i].guild_id > 0)
				c++;

		len += snprintf(str + len, maxlen-len, "%ld\t", (unsigned long)c);

		for(i = 0; i < MAX_GUILDALLIANCE; i++)
		{
			const struct guild_alliance &a = g.alliance[i];
			if (a.guild_id > 0)
				len += snprintf(str + len, maxlen-len, "%ld,%ld\t%s\t", (unsigned long)a.guild_id, (unsigned long)a.opposition, a.name);
		}
		// 追放リスト
		for(i=0,c=0; i<MAX_GUILDEXPLUSION; i++)
			if (g.explusion[i].account_id > 0)
				c++;

		len += snprintf(str + len, maxlen-len, "%ld\t", (unsigned long)c);
		for(i = 0; i < MAX_GUILDEXPLUSION; i++)
		{
			const struct guild_explusion &e = g.explusion[i];
			if (e.account_id > 0)
				len += snprintf(str + len, maxlen-len, "%ld,%ld,%ld,%ld\t%s\t%s\t%s#\t",
							   (unsigned long)e.account_id, (unsigned long)e.rsv1, (unsigned long)e.rsv2, (unsigned long)e.rsv3,
							   e.name, e.acc, e.mes );
		}
		// ギルドスキル
		for(i = 0; i < MAX_GUILDSKILL; i++)
		{
			len += snprintf(str + len, maxlen-len, "%d,%d ", g.skill[i].id, g.skill[i].lv);
		}
		len += snprintf(str + len, maxlen-len, "\t"RETCODE);
		return len;
	}

	ssize_t castle2string(char *str, size_t maxlen, const CCastle &gc)
	{
		ssize_t len;
		len = snprintf(str, maxlen, "%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld"RETCODE,	// added Guardian HP [Valaris]
					  gc.castle_id, (unsigned long)gc.guild_id, (unsigned long)gc.economy, (unsigned long)gc.defense, (unsigned long)gc.triggerE,
					  (unsigned long)gc.triggerD, (unsigned long)gc.nextTime, (unsigned long)gc.payTime, (unsigned long)gc.createTime, (unsigned long)gc.visibleC,
					  (unsigned long)gc.guardian[0].visible, (unsigned long)gc.guardian[1].visible, (unsigned long)gc.guardian[2].visible, (unsigned long)gc.guardian[3].visible,
					  (unsigned long)gc.guardian[4].visible, (unsigned long)gc.guardian[5].visible, (unsigned long)gc.guardian[6].visible, (unsigned long)gc.guardian[7].visible,
					  (unsigned long)gc.guardian[0].guardian_hp, (unsigned long)gc.guardian[1].guardian_hp, (unsigned long)gc.guardian[2].guardian_hp, (unsigned long)gc.guardian[3].guardian_hp,
					  (unsigned long)gc.guardian[4].guardian_hp, (unsigned long)gc.guardian[5].guardian_hp, (unsigned long)gc.guardian[6].guardian_hp, (unsigned long)gc.guardian[7].guardian_hp);
		return len;
	}

	// ギルド城データの文字列からの変換
	bool string2castle(char *str, CCastle &gc)
	{
		int tmp_int[26];
		memset(tmp_int, 0, sizeof(tmp_int));
		if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				   &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
				   &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12], &tmp_int[13],
				   &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18], &tmp_int[19], &tmp_int[20],
				   &tmp_int[21], &tmp_int[22], &tmp_int[23], &tmp_int[24], &tmp_int[25]) == 26) {
			gc.castle_id = tmp_int[0];
			gc.guild_id = tmp_int[1];
			gc.economy = tmp_int[2];
			gc.defense = tmp_int[3];
			gc.triggerE = tmp_int[4];
			gc.triggerD = tmp_int[5];
			gc.nextTime = tmp_int[6];
			gc.payTime = tmp_int[7];
			gc.createTime = tmp_int[8];
			gc.visibleC = tmp_int[9];
			gc.guardian[0].visible = tmp_int[10];
			gc.guardian[1].visible = tmp_int[11];
			gc.guardian[2].visible = tmp_int[12];
			gc.guardian[3].visible = tmp_int[13];
			gc.guardian[4].visible = tmp_int[14];
			gc.guardian[5].visible = tmp_int[15];
			gc.guardian[6].visible = tmp_int[16];
			gc.guardian[7].visible = tmp_int[17];
			gc.guardian[0].guardian_hp = tmp_int[18];
			gc.guardian[1].guardian_hp = tmp_int[19];
			gc.guardian[2].guardian_hp = tmp_int[20];
			gc.guardian[3].guardian_hp = tmp_int[21];
			gc.guardian[4].guardian_hp = tmp_int[22];
			gc.guardian[5].guardian_hp = tmp_int[23];
			gc.guardian[6].guardian_hp = tmp_int[24];
			gc.guardian[7].guardian_hp = tmp_int[25];	// end additions [Valaris]
		// old structure of guild castle
		} else if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
						  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
						  &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12], &tmp_int[13],
						  &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17]) == 18) {
			gc.castle_id = tmp_int[0];
			gc.guild_id = tmp_int[1];
			gc.economy = tmp_int[2];
			gc.defense = tmp_int[3];
			gc.triggerE = tmp_int[4];
			gc.triggerD = tmp_int[5];
			gc.nextTime = tmp_int[6];
			gc.payTime = tmp_int[7];
			gc.createTime = tmp_int[8];
			gc.visibleC = tmp_int[9];
			gc.guardian[0].visible = tmp_int[10];
			gc.guardian[1].visible = tmp_int[11];
			gc.guardian[2].visible = tmp_int[12];
			gc.guardian[3].visible = tmp_int[13];
			gc.guardian[4].visible = tmp_int[14];
			gc.guardian[5].visible = tmp_int[15];
			gc.guardian[6].visible = tmp_int[16];
			gc.guardian[7].visible = tmp_int[17];
			gc.guardian[0].guardian_hp = (gc.guardian[0].visible) ? 15670 + 2000 * gc.defense : 0;
			gc.guardian[1].guardian_hp = (gc.guardian[1].visible) ? 15670 + 2000 * gc.defense : 0;
			gc.guardian[2].guardian_hp = (gc.guardian[2].visible) ? 15670 + 2000 * gc.defense : 0;
			gc.guardian[3].guardian_hp = (gc.guardian[3].visible) ? 30214 + 2000 * gc.defense : 0;
			gc.guardian[4].guardian_hp = (gc.guardian[4].visible) ? 30214 + 2000 * gc.defense : 0;
			gc.guardian[5].guardian_hp = (gc.guardian[5].visible) ? 28634 + 2000 * gc.defense : 0;
			gc.guardian[6].guardian_hp = (gc.guardian[6].visible) ? 28634 + 2000 * gc.defense : 0;
			gc.guardian[7].guardian_hp = (gc.guardian[7].visible) ? 28634 + 2000 * gc.defense : 0;
		}
		else
		{
			return false;
		}
		return true;
	}
	bool readGuildsCastles()
	{
		char line[16384];
		FILE *fp;
		int i, j, c;

		///////////////////////////////////////////////////////////////////////
		fp = safefopen(guild_filename,"r");
		if(fp == NULL)
		{
			ShowError("can't read %s\n", guild_filename);
			return false;
		}

		c=0;
		while(fgets(line, sizeof(line), fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			CGuild g;
			j = 0;
			if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && next_guild_id <= (uint32)i) {
				next_guild_id = i;
				continue;
			}
			if( string2guild(line, g) && g.guild_id > 0 )
			{
				g.calcInfo();
				if(g.max_member>0 && g.max_member < MAX_GUILD)
				{
					for(i=0;i<g.max_member;i++)
					{
						if(g.member[i].account_id>0)
							break;
					}
					if(i<g.max_member)
					{


						if( g.guild_id >= next_guild_id )
							next_guild_id = g.guild_id + 1;

						cGuilds.insert(g);
					}
				}
			}
			else
			{
				ShowError("Guild: broken data [%s] line %d\n", guild_filename, c);
			}
		}
		fclose(fp);
		ShowStatus("Guild: %s read done (%d guilds)\n", guild_filename, cGuilds.size());
		///////////////////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////////////////////
		fp = safefopen(castle_filename, "r");
		if( fp==NULL )
		{
			ShowError("GuildCastle: cannot open %s\n", castle_filename);
			return false;
		}
		c = 0;
		while(fgets(line, sizeof(line), fp))
		{
			c++;

			if( !get_prepared_line(line) )
				continue;

			size_t pos;
			CCastle gc;
			if( string2castle(line, gc) )
			{	// clear guildcastles with removed guilds
				if( gc.guild_id && !cGuilds.find(CGuild(gc.guild_id), pos, 0) )
					gc.guild_id = 0;

				cCastles.insert(gc);
			}
			else
				ShowError("GuildCastle: broken data [%s] line %d\n", castle_filename, c);

		}
		fclose(fp);
		ShowStatus("GuildCastle: %s read done (%d castles)\n", castle_filename, cCastles.size());

		for(i = 0; i < MAX_GUILDCASTLE; i++)
		{	// check if castle exists
			size_t pos;
			if( !cCastles.find( CCastle(i), 0, pos) )
			{	// construct a new one if not
				cCastles.insert( CCastle(i) ); // constructor takes care of all settings
			}
		}
		///////////////////////////////////////////////////////////////////////
		return true;
	}
	bool saveGuildsCastles()
	{
		bool ret=true;
		char line[65536];
		FILE *fp;
		int lock;
		size_t i, sz;
		///////////////////////////////////////////////////////////////////////
		fp = lock_fopen(guild_filename, lock);
		if( fp == NULL) {
			ShowError("Guild: cannot open [%s]\n", guild_filename);
			ret = false;
		}
		else
		{
			for(i=0; i<cGuilds.size(); i++)
			{
				sz=guild2string(line, sizeof(line), cGuilds[i]);
				if(sz>0)
					fwrite(line, sz,1,fp);
			}
			lock_fclose(fp, guild_filename, lock);
		}


		///////////////////////////////////////////////////////////////////////
		fp = lock_fopen(castle_filename, lock);
		if( fp == NULL) {
			ShowError("Guild: cannot open [%s]\n", castle_filename);
			ret = false;
		}
		else
		{
			for(i=0; i<cCastles.size(); i++)
			{
				sz=castle2string(line, sizeof(line), cCastles[i]);
				//fprintf(fp, "%s" RETCODE, line);	// retcode integrated to line generation
				if(sz>0) fwrite(line, sz,1,fp);
			}
			lock_fclose(fp, castle_filename, lock);
		}
		///////////////////////////////////////////////////////////////////////
		return ret;
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildDB_txt(const char *configfile) :
		CTimerBase(60000),		// 60sec save interval
		next_guild_id(10000),
		savecount(0),
		cPosGuild(0), cPosCastle(0)
	{
		safestrcpy(guild_filename,"save/guild.txt",sizeof(guild_filename));
		safestrcpy(castle_filename,"save/castle.txt",sizeof(castle_filename));
		safestrcpy(guildexp_filename,"db/exp_guild.txt",sizeof(guildexp_filename));
		init(configfile);
	}
	virtual ~CGuildDB_txt()
	{
		close();
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CGuild, 2>	cGuilds;
	TslistDST<CCastle>		cCastles;
	uint32					next_guild_id;
	uint					savecount;
	char					guild_filename[1024];
	char					castle_filename[1024];
	char					guildexp_filename[1024];
	///////////////////////////////////////////////////////////////////////////
	// data for alternative interface
	Mutex cMxGuild;
	Mutex cMxCastle;
	size_t cPosGuild;
	size_t cPosCastle;


	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		CGuild::cGuildExp.init(guildexp_filename);
		return readGuildsCastles();
	}
	bool close()
	{
		return saveGuildsCastles();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			saveGuildsCastles();
		}
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()					{ return cGuilds.size(); }
	virtual CGuild& operator[](size_t i)	{ return cGuilds[i]; }

	virtual size_t castlesize()				{ return cCastles.size(); }
	virtual CCastle& castle(size_t i)		{ return cCastles[i]; }


	virtual bool searchGuild(const char* name, CGuild& guild)
	{
		size_t pos;
		if( cGuilds.find( CGuild(name), pos, 1) )
		{
			guild = cGuilds[pos];
			return true;
		}
		return false;
	}
	virtual bool searchGuild(uint32 guildid, CGuild& guild)
	{
		size_t pos;
		if( cGuilds.find( CGuild(guildid), pos, 0) )
		{
			guild = cGuilds[pos];
			return true;
		}
		return false;
	}
	virtual bool insertGuild(const struct guild_member &member, const char *name, CGuild &guild)
	{
		CGuild tmp(name);
		size_t pos, i;
		if( !cGuilds.find( tmp, pos, 1) )
		{
			//tmp.name[24];
			tmp.guild_id = next_guild_id++;

			tmp.member[0] = member;
			safestrcpy(tmp.master, member.name, sizeof(tmp.master));
			tmp.position[0].mode=0x11;
			safestrcpy(tmp.position[0].name,"GuildMaster", sizeof(tmp.position[0].name));
			safestrcpy(tmp.position[MAX_GUILDPOSITION-1].name,"Newbie", sizeof(tmp.position[0].name));
			for(i=1; i<MAX_GUILDPOSITION-1; i++)
				snprintf(tmp.position[i].name,sizeof(tmp.position[0].name),"Position %ld",(unsigned long)(i+1));

			tmp.max_member=16;
			tmp.average_lv=tmp.member[0].lv;
			for(i=0;i<MAX_GUILDSKILL;i++)
				tmp.skill[i].id = i+GD_SKILLBASE;

			guild = tmp;
			return cGuilds.insert(tmp);
		}
		return false;
	}
	virtual bool removeGuild(uint32 guildid)
	{
		size_t pos,i,k;
		if( cGuilds.find( CGuild(guildid), pos, 0) )
		{
			// clear alliances
			for(i=0; i<cGuilds.size(); i++)
			for(k=0; k<MAX_GUILDALLIANCE; k++)
			{
				if( cGuilds[i].alliance[k].guild_id == guildid )
					cGuilds[i].alliance[k].guild_id = 0;
			}
			// clear castles
			for(i=0; i<cCastles.size(); i++)
			{
				if( cCastles[i].guild_id == guildid )
					cCastles[i].guild_id = 0;
			}

			return cGuilds.removeindex(pos,0);
		}
		return false;
	}
	virtual bool saveGuild(const CGuild& guild)
	{
		size_t pos;
		if( cGuilds.find( guild, pos, 0) )
		{
			cGuilds[pos] = guild;
			savecount++;
			return true;
		}
		return false;
	}

	virtual bool searchCastle(ushort cid, CCastle& castle)
	{
		size_t pos;
		if( cCastles.find( CCastle(cid), 0, pos) )
		{
			castle = cCastles[pos];
			return true;
		}
		return false;
	}
	virtual bool saveCastle(CCastle& castle)
	{
		size_t pos;
		if( cCastles.find( castle, 0, pos) )
		{
			cCastles[pos] = castle;
			return true;
		}
		return false;
	}
	virtual bool removeCastle(ushort cid)
	{
		size_t pos;
		if( cCastles.find( CCastle(cid), 0, pos) )
		{
			cCastles.removeindex(pos);
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// alternative interface
	virtual bool aquireGuild()
	{
		cMxGuild.lock();
		return this->firstGuild(); 
	}
	virtual bool aquireCastle()
	{
		cMxCastle.lock();
		return this->firstCastle(); 
	}
	virtual bool releaseGuild()
	{
		cMxGuild.unlock();
		return true; 
	}
	virtual bool releaseCastle()
	{
		cMxCastle.unlock();
		return true; 
	}
	virtual bool firstGuild()
	{
		cPosGuild=0;
		return this->isGuildOk();
	}
	virtual bool firstCastle()
	{
		cPosCastle=0;
		return this->isCastleOk();
	}
	virtual bool isGuildOk()
	{
		return (cGuilds.size() < cPosGuild); 
	}
	virtual bool isCastleOk()
	{
		return (cCastles.size() < cPosCastle); 
	}
	virtual bool nextGuild()
	{
		cPosGuild++;
		return this->isGuildOk();
	}
	virtual bool nextCastle()
	{
		cPosGuild++;
		return this->isGuildOk();
	}
	virtual bool saveGuild()				{ return true; }
	virtual bool saveCastle()				{ return true; }

	virtual bool findGuild(const char* name)
	{
		size_t pos;
		if( cGuilds.find( CGuild(name), pos, 1) )
		{	// need index base 0
			return cGuilds.find( cGuilds(pos,1), cPosGuild, 0);
		}
		return false;
	}
	virtual bool findGuild(uint32 guildid)
	{
		return cGuilds.find( CGuild(guildid), cPosCastle, 0);
	}
	virtual bool findCastle(ushort cid)
	{
		return cCastles.find( CCastle(cid), 0, cPosCastle);
	}
	virtual CGuild& getGuild()
	{
		return cGuilds[cPosGuild];
	}
	virtual CCastle& getCastle()
	{
		return cCastles[cPosCastle];
	}

};



///////////////////////////////////////////////////////////////////////////////
#else// SQL
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
#endif// SQL
///////////////////////////////////////////////////////////////////////////////

CGuildDBInterface* CGuildDB::getDB(const char *dbcfgfile)
{
#ifdef TXT_ONLY
	return new CGuildDB_txt(dbcfgfile);
#else
	return NULL;
//	return new CGuildDB_sql(dbcfgfile);
#endif// SQL
}


///////////////////////////////////////////////////////////////////////////////
// Guild Experience
void CGuildExp::init(const char* filename)
{
	FILE* fp=safefopen(filename,"r");
	memset(exp,0,sizeof(exp));
	if(fp==NULL)
	{
		ShowError("can't read %s\n", filename);
	}
	else
	{
		char line[1024];
		int c=0;
		while(fgets(line,sizeof(line),fp) && c<100)
		{
			if( !get_prepared_line(line) )
				continue;
			exp[c]=atoi(line);
			c++;
		}
		fclose(fp);
	}
}
// static member of CGuild
CGuildExp CGuild::cGuildExp;



///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Party Database
///////////////////////////////////////////////////////////////////////////////
class CPartyDB_txt : public CTimerBase, private CConfig, public CPartyDBInterface
{

	ssize_t party_to_string(char *str, size_t maxlen, const CParty &p)
	{
		ssize_t i, len;
		len = snprintf(str, maxlen, "%ld\t%s\t%d,%d\t", (unsigned long)p.party_id, p.name, p.expshare, p.itemshare);
		for(i = 0; i < MAX_PARTY; i++)
		{
			const struct party_member &m = p.member[i];
			len += snprintf(str+len, maxlen-len, "%ld,%ld\t%s\t", (unsigned long)m.account_id, (unsigned long)m.leader, ((m.account_id > 0) ? m.name : "NoMember"));
		}
		snprintf(str+len, maxlen-len, RETCODE);
		return len;
	}
	bool party_from_string(const char *str, CParty &p)
	{
		int i, j;
		int tmp_int[16];
		char tmp_str[256];

		if (sscanf(str, "%d\t%255[^\t]\t%d,%d\t", &tmp_int[0], tmp_str, &tmp_int[1], &tmp_int[2]) != 4)
			return false;

		p.party_id = tmp_int[0];
		safestrcpy(p.name, tmp_str, sizeof(p.name));
		p.expshare = tmp_int[1];
		p.itemshare = tmp_int[2];
		for(j=0; j<3 && str != NULL; j++)
			str = strchr(str+1, '\t');

		for(i=0; i<MAX_PARTY; i++)
		{
			struct party_member &m = p.member[i];
			if(str == NULL)
				return false;
			if(sscanf(str + 1, "%d,%d\t%255[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str) != 3)
				return false;

			m.account_id = tmp_int[0];
			m.leader = tmp_int[1];
			safestrcpy(m.name, tmp_str, sizeof(m.name));
			for(j=0; j<2 && str != NULL; j++)
				str = strchr(str + 1, '\t');
		}
		return true;
	}
	bool readParties()
	{
		char line[8192];
		int c = 0;
		int i, j;
		FILE *fp = safefopen(party_filename, "r");

		if( fp == NULL )
		{
			ShowError("Party: cannot open %s\n", party_filename);
			return false;
		}
		while(fgets(line, sizeof(line), fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			j = 0;
			if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && next_party_id <= (uint32)i)
			{
				next_party_id = i;
			}
			else
			{
				CParty p;
				if( party_from_string(line, p) && p.party_id > 0)
				{
					if( !p.isEmpty() )
					{
						if(p.party_id >= next_party_id)
							next_party_id = p.party_id + 1;

						cParties.insert(p);
					}
				}
				else
				{
					ShowError("int_party: broken data [%s] line %d\n", party_filename, c);
				}
			}
		}
		fclose(fp);
		ShowStatus("Party: %s read done (%d parties)\n", party_filename, c);
		return true;
	}
	bool saveParties()
	{
		char line[65536];
		FILE *fp;
		int lock;
		size_t i;
		ssize_t sz;

		if ((fp = lock_fopen(party_filename, lock)) == NULL) {
			ShowError("Party: cannot open [%s]\n", party_filename);
			return false;
		}
		for(i=0; i<cParties.size(); i++)
		{
			sz = party_to_string(line, sizeof(line), cParties[i]);
			if(sz>0) fwrite(line, sz,1, fp);
		}
		fprintf(fp, "%d\t%%newid%%\n", next_party_id);
		lock_fclose(fp,party_filename, lock);
		return 0;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPartyDB_txt(const char *configfile) :
		CTimerBase(60000),		// 60sec save interval
		next_party_id(1000),
		savecount(0)
	{
		safestrcpy(party_filename,"save/party.txt",sizeof(party_filename));
		init(configfile);
	}
	virtual ~CPartyDB_txt()
	{
		close();
	}
private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CParty, 2>	cParties;
	uint32					next_party_id;
	uint					savecount;
	char					party_filename[1024];

	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return readParties();
	}
	bool close()
	{
		return saveParties();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			saveParties();
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()					{ return cParties.size(); }
	virtual CParty& operator[](size_t i)	{ return cParties[i]; }

	virtual bool searchParty(const char* name, CParty& party)
	{
		size_t pos;
		if( cParties.find( CParty(name), pos, 1) )
		{
			party = cParties[pos];
			return true;
		}
		return false;
	}
	virtual bool searchParty(uint32 pid, CParty& party)
	{
		size_t pos;
		if( cParties.find( CParty(pid), pos, 0) )
		{
			party = cParties[pos];
			return true;
		}
		return false;
	}
	virtual bool insertParty(uint32 accid, const char *nick, const char *map, ushort lv, const char *name, CParty &party)
	{
		size_t pos;
		CParty temp(name);
		if( !cParties.find( temp, pos, 0) )
		{
			//temp.name[24];
			temp.party_id = next_party_id++;
			temp.expshare = 0;
			temp.itemshare = 0;
			temp.member[0].account_id = accid;
			safestrcpy(temp.member[0].name, nick, sizeof(temp.member[0].name));
			safestrcpy(temp.member[0].mapname, map, sizeof(temp.member[0].mapname));
			char*ip = strchr(temp.member[0].mapname,'.');
			if(ip) *ip=0;

			temp.member[0].leader = 1;
			temp.member[0].online = 1;
			temp.member[0].lv = lv;

			cParties.insert(temp);
			party = temp;
			return true;
		}
		return false;
	}
	virtual bool removeParty(uint32 pid)
	{
		size_t pos;
		if( cParties.find( CParty(pid), pos, 0) )
		{
			cParties.removeindex(pos);
			return true;
		}
		return false;
	}
	virtual bool saveParty(const CParty& party)
	{
		size_t pos;
		if( cParties.find( party, pos, 0) )
		{
			cParties[pos] = party;
			return true;
		}
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////////
#else// SQL
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
#endif// SQL
///////////////////////////////////////////////////////////////////////////////

CPartyDBInterface* CPartyDB::getDB(const char *dbcfgfile)
{
#ifdef TXT_ONLY
	return new CPartyDB_txt(dbcfgfile);
#else
	return NULL;
//	return new CPartyDB_sql(dbcfgfile);
#endif// SQL
}






///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Storage Database Interface
///////////////////////////////////////////////////////////////////////////////
class CPCStorageDB_txt : public CTimerBase, private CConfig, public CPCStorageDBInterface
{
	ssize_t storage_to_string(char *str, size_t maxlen, const CPCStorage &stor)
	{
		int i,f=0;
		char *str_p = str;
		str_p += snprintf(str_p,maxlen,"%ld,%d\t",(unsigned long)stor.account_id, stor.storage_amount);
		for(i=0;i<MAX_STORAGE;i++)
		{
			if( (stor.storage[i].nameid) && (stor.storage[i].amount) )
			{
				str_p += snprintf(str_p,str+maxlen-str_p,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
					stor.storage[i].id,stor.storage[i].nameid,stor.storage[i].amount,stor.storage[i].equip,
					stor.storage[i].identify,stor.storage[i].refine,stor.storage[i].attribute,
					stor.storage[i].card[0],stor.storage[i].card[1],stor.storage[i].card[2],stor.storage[i].card[3]);
				f++;
			}
		}
		*(str_p++)='\t';
		*str_p='\0';
		if(!f)
		{
			str[0]=0;
			str_p=str;
		}
		return (str_p-str);
	}
	int storage_from_string(const char *str, CPCStorage &stor)
	{
		int tmp_int[256];
		int set,next,len,i;

		set=sscanf(str,"%d,%d%n",&tmp_int[0],&tmp_int[1],&next);
		stor.storage_amount = (tmp_int[1]<MAX_STORAGE)?tmp_int[1]:MAX_STORAGE;

		if(set!=2)
			return false;
		if(str[next]=='\n' || str[next]=='\r')
			return false;
		next++;
		for(i=0;str[next] && str[next]!='\t' && i<MAX_STORAGE;i++)
		{
			if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				&tmp_int[4], &tmp_int[5], &tmp_int[6],
				&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
			{
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				if (str[next] == ' ')
					next++;
			}
			else if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				&tmp_int[4], &tmp_int[5], &tmp_int[6],
				&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11)
			{
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				if (str[next] == ' ')
					next++;
			}
			else
				return false;
		}
		return true;
	}
	bool readPCStorage()
	{
		char line[65536];
		int c=0;
		unsigned long tmp;
		CPCStorage s;
		FILE *fp=safefopen(pcstorage_filename,"r");

		if(fp==NULL)
		{
			ShowError("Storage: cannot open %s\n", pcstorage_filename);
			return false;
		}
		while(fgets(line,sizeof(line),fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			sscanf(line,"%ld",&tmp);
			s.account_id=tmp;
			if(s.account_id > 0 && storage_from_string(line,s) )
			{
				cPCStorList.insert(s);
			}
			else
			{
				ShowError("Storage: broken data [%s] line %d\n",pcstorage_filename,c);
			}
		}
		fclose(fp);
		return true;
	}
	bool savePCStorage()
	{
		char line[65536];
		int lock;
		size_t i, sz;
		FILE *fp=lock_fopen(pcstorage_filename, lock);

		if( fp==NULL )
		{
			ShowError("Storage: cannot open [%s]\n",pcstorage_filename);
			return false;
		}
		for(i=0; i<cPCStorList.size(); i++)
		{
			sz = storage_to_string(line, sizeof(line), cPCStorList[i]);
			if(sz>0) fprintf(fp,"%s"RETCODE,line);
		}
		lock_fclose(fp, pcstorage_filename, lock);
		return true;
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPCStorageDB_txt(const char *dbcfgfile) :
		CTimerBase(60000),		// 60sec save interval
		savecount(0)
	{
		safestrcpy(pcstorage_filename, "save/storage.txt", sizeof(pcstorage_filename));
		init(dbcfgfile);
	}
	virtual ~CPCStorageDB_txt()
	{
		close();
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TslistDST<CPCStorage>	cPCStorList;
	uint savecount;
	char pcstorage_filename[1024];


	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return readPCStorage();
	}
	bool close()
	{
		return savePCStorage();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			savePCStorage();
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return cPCStorList.size(); }
	virtual CPCStorage& operator[](size_t i)	{ return cPCStorList[i]; }

	virtual bool searchStorage(uint32 accid, CPCStorage& stor)
	{
		size_t pos;
		if( cPCStorList.find( CPCStorage(accid), 0, pos) )
		{
			stor = cPCStorList[pos];
			return true;
		}
		return false;
	}
	virtual bool removeStorage(uint32 accid)
	{
		size_t pos;
		if( cPCStorList.find( CPCStorage(accid), 0, pos) )
		{
			cPCStorList.removeindex(pos);
			return true;
		}
		return false;
	}
	virtual bool saveStorage(const CPCStorage& stor)
	{
		size_t pos;
		if( cPCStorList.find( stor, 0, pos) )
		{
			cPCStorList[pos] = stor;
			return true;
		}
		else
			return cPCStorList.insert(stor);
	}
};


///////////////////////////////////////////////////////////////////////////////
#else// SQL
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
#endif// SQL
///////////////////////////////////////////////////////////////////////////////

CPCStorageDBInterface* CPCStorageDB::getDB(const char *dbcfgfile)
{
#ifdef TXT_ONLY
	return new CPCStorageDB_txt(dbcfgfile);
#else
	return NULL;
//	return new CPCStorageDB_sql(dbcfgfile);
#endif// SQL
}




///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////

class CGuildStorageDB_txt : public CTimerBase, private CConfig, public CGuildStorageDBInterface
{

	ssize_t guild_storage_to_string(char *str, size_t maxlen, const CGuildStorage &stor)
	{
		int i,f=0;
		char *str_p = str;
		str_p+=snprintf(str,maxlen,"%ld,%d\t",(unsigned long)stor.guild_id, stor.storage_amount);

		for(i=0;i<MAX_GUILD_STORAGE;i++)
		{
			if( (stor.storage[i].nameid) && (stor.storage[i].amount) )
			{
				str_p += snprintf(str_p,str+maxlen-str_p,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
					stor.storage[i].id,stor.storage[i].nameid,stor.storage[i].amount,stor.storage[i].equip,
					stor.storage[i].identify,stor.storage[i].refine,stor.storage[i].attribute,
					stor.storage[i].card[0],stor.storage[i].card[1],stor.storage[i].card[2],stor.storage[i].card[3]);
				f++;
			}
			*(str_p++)='\t';
			*str_p='\0';
		}
		if(!f)
		{
			str[0]=0;
			str_p=str;
		}
		return (str_p-str);
	}
	bool guild_storage_from_string(const char *str, CGuildStorage &stor)
	{
		int tmp_int[256];
		int set,next,len,i;

		set=sscanf(str,"%d,%d%n",&tmp_int[0],&tmp_int[1],&next);
		if(set!=2)
			return false;
		if(str[next]=='\n' || str[next]=='\r')
			return false;
		next++;

		stor.storage_amount = (tmp_int[1]<MAX_GUILD_STORAGE) ? tmp_int[1] : MAX_GUILD_STORAGE;
		for(i=0; str[next] && str[next]!='\t' && i<MAX_GUILD_STORAGE; i++)
		{
			if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				&tmp_int[4], &tmp_int[5], &tmp_int[6],
				&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
			{
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				while(str[next] == ' ') next++;
			}
			else if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
				  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
				  &tmp_int[4], &tmp_int[5], &tmp_int[6],
				  &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11) {
				stor.storage[i].id = tmp_int[0];
				stor.storage[i].nameid = tmp_int[1];
				stor.storage[i].amount = tmp_int[2];
				stor.storage[i].equip = tmp_int[3];
				stor.storage[i].identify = tmp_int[4];
				stor.storage[i].refine = tmp_int[5];
				stor.storage[i].attribute = tmp_int[6];
				stor.storage[i].card[0] = tmp_int[7];
				stor.storage[i].card[1] = tmp_int[8];
				stor.storage[i].card[2] = tmp_int[9];
				stor.storage[i].card[3] = tmp_int[10];
				next += len;
				while(str[next] == ' ')	next++;
			}
			else
				return false;
		}
		return true;
	}

	bool readGuildStorage()
	{
		char line[65536];
		int c=0;
		unsigned long tmp;
		CGuildStorage gs;
		FILE *fp=safefopen(guildstorage_filename,"r");
		if(fp==NULL){
			ShowMessage("cant't read : %s\n",guildstorage_filename);
			return 1;
		}
		while(fgets(line,sizeof(line),fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			sscanf(line,"%ld",&tmp);
			gs.guild_id=tmp;
			if(gs.guild_id > 0 && guild_storage_from_string(line,gs) )
			{
				cGuildStorList.insert(gs);
			}
			else
			{
				ShowError("Storage: broken data [%s] line %d\n", guildstorage_filename, c);
			}
		}
		fclose(fp);
		return true;
	}
	bool saveGuildStorage()
	{
		char line[65536];
		int lock;
		size_t i, sz;
		FILE *fp=lock_fopen(guildstorage_filename, lock);

		if( fp==NULL )
		{
			ShowError("Storage: cannot open [%s]\n",guildstorage_filename);
			return false;
		}
		for(i=0; i<cGuildStorList.size(); i++)
		{
			sz = guild_storage_to_string(line, sizeof(line), cGuildStorList[i]);
			if(sz>0) fprintf(fp,"%s"RETCODE,line);
		}
		lock_fclose(fp, guildstorage_filename, lock);
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CGuildStorageDB_txt(const char *dbcfgfile) :
		CTimerBase(60000),		// 60sec save interval
		savecount(0)
	{
		safestrcpy(guildstorage_filename, "save/g_storage.txt", sizeof(guildstorage_filename));
		init(dbcfgfile);
	}
	virtual ~CGuildStorageDB_txt()
	{
		close();
	}


private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TslistDST<CGuildStorage> cGuildStorList;
	uint savecount;
	char guildstorage_filename[1024];


	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{

		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return readGuildStorage();
	}
	bool close()
	{
		return saveGuildStorage();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			saveGuildStorage();
		}
		return true;
	}


	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return cGuildStorList.size(); }
	virtual CGuildStorage& operator[](size_t i)	{ return cGuildStorList[i]; }

	virtual bool searchStorage(uint32 gid, CGuildStorage& stor)
	{
		size_t pos;
		if( cGuildStorList.find( CGuildStorage(gid), 0, pos) )
		{
			stor = cGuildStorList[pos];
			return true;
		}
		return false;
	}
	virtual bool removeStorage(uint32 gid)
	{
		size_t pos;
		if( cGuildStorList.find( CGuildStorage(gid), 0, pos) )
		{
			cGuildStorList.removeindex(pos);
			return true;
		}
		return false;
	}
	virtual bool saveStorage(const CGuildStorage& stor)
	{
		size_t pos;
		if( cGuildStorList.find( stor, 0, pos) )
		{
			cGuildStorList[pos] = stor;
			return true;
		}
		else
			return cGuildStorList.insert(stor);
	}
};

///////////////////////////////////////////////////////////////////////////////
#else// SQL
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
#endif// SQL
///////////////////////////////////////////////////////////////////////////////

CGuildStorageDBInterface* CGuildStorageDB::getDB(const char *dbcfgfile)
{
#ifdef TXT_ONLY
	return new CGuildStorageDB_txt(dbcfgfile);
#else
	return NULL;
//	return new CGuildStorageDB_sql(dbcfgfile);
#endif// SQL
}



///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////

class CPetDB_txt : public CTimerBase, private CConfig, public CPetDBInterface
{
	int pet_to_string(char *str, size_t sz, CPet &pet)
	{
		int len;

		if(pet.hungry < 0)
			pet.hungry = 0;
		else if(pet.hungry > 100)
			pet.hungry = 100;
		if(pet.intimate < 0)
			pet.intimate = 0;
		else if(pet.intimate > 1000)
			pet.intimate = 1000;

		len=snprintf(str, sz, "%ld,%d,%s\t%ld,%ld,%d,%d,%d,%d,%d,%d,%d",
			(unsigned long)pet.pet_id,pet.class_,pet.name,
			(unsigned long)pet.account_id,(unsigned long)pet.char_id,
			pet.level,pet.egg_id, pet.equip_id,pet.intimate,pet.hungry,
			pet.rename_flag,pet.incuvate);

		return len;
	}

	bool pet_from_string(const char *str, CPet &pet)
	{
		int tmp_int[16];
		char tmp_str[256];
		int s=sscanf(str,"%d,%d,%[^\t]\t%d,%d,%d,%d,%d,%d,%d,%d,%d",&tmp_int[0],&tmp_int[1],tmp_str,&tmp_int[2],
				&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],&tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10]);

		if(s==12)
		{
			pet.pet_id = tmp_int[0];
			pet.class_ = tmp_int[1];
			safestrcpy(pet.name,tmp_str,24);
			pet.account_id = tmp_int[2];
			pet.char_id = tmp_int[3];
			pet.level = tmp_int[4];
			pet.egg_id = tmp_int[5];
			pet.equip_id = tmp_int[6];
			pet.intimate = tmp_int[7];
			pet.hungry = tmp_int[8];
			pet.rename_flag = tmp_int[9];
			pet.incuvate = tmp_int[10];

			if(pet.hungry < 0)
				pet.hungry = 0;
			else if(pet.hungry > 100)
				pet.hungry = 100;
			if(pet.intimate < 0)
				pet.intimate = 0;
			else if(pet.intimate > 1000)
				pet.intimate = 1000;

			return true;
		}
		return false;
	}

	bool readPets()
	{
		char line[65536];
		int c=0;
		CPet pet;
		FILE *fp=safefopen(pet_filename,"r");
		if(fp==NULL){
			ShowMessage("cant't read : %s\n",pet_filename);
			return 1;
		}
		while(fgets(line,sizeof(line),fp))
		{
			c++;
			if( !get_prepared_line(line) )
				continue;

			if( pet_from_string(line,pet) )
			{
				cPetList.insert(pet);
			}
			else
			{
				ShowError("Storage: broken data [%s] line %d\n", pet_filename, c);
			}
		}
		fclose(fp);
		return true;
	}
	bool savePets()
	{
		char line[65536];
		int lock;
		size_t i, sz;
		FILE *fp=lock_fopen(pet_filename, lock);

		if( fp==NULL )
		{
			ShowError("Storage: cannot open [%s]\n",pet_filename);
			return false;
		}
		for(i=0; i<cPetList.size(); i++)
		{
			sz = pet_to_string(line, sizeof(line), cPetList[i]);
			if(sz>0) fprintf(fp,"%s"RETCODE,line);
		}
		lock_fclose(fp, pet_filename, lock);
		return true;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	// construct/destruct
	CPetDB_txt(const char *dbcfgfile) :
		CTimerBase(60000),		// 60sec save interval
		savecount(0)
	{
		next_petid = 100;
		safestrcpy(pet_filename, "save/pet.txt", sizeof(pet_filename));
		init(dbcfgfile);
	}
	virtual ~CPetDB_txt()
	{
		close();
	}

private:
	///////////////////////////////////////////////////////////////////////////
	// data
	TslistDST<CPet> cPetList;
	uint32 next_petid;
	char pet_filename[1024];
	uint savecount;

	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2)
	{
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		if(configfile)
			CConfig::LoadConfig(configfile);
		return readPets();
	}
	bool close()
	{
		return savePets();
	}

	///////////////////////////////////////////////////////////////////////////
	// timer function
	virtual bool timeruserfunc(unsigned long tick)
	{
		// we only save if necessary:
		// we have do some authentifications without do saving
		if( savecount > 10 )
		{
			savecount=0;
			savePets();
		}
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()				{ return cPetList.size(); }
	virtual CPet& operator[](size_t i) { return cPetList[i]; }

	virtual bool searchPet(const char* name, CPet& pet)
	{
		size_t pos;
		if( cPetList.find( CPet(name), 0, pos) )
		{
			pet = cPetList[pos];
			return true;
		}
		return false;
	}
	virtual bool searchPet(uint32 pid, CPet& pet)
	{
		size_t pos;
		if( cPetList.find( CPet(pid), 0, pos) )
		{
			pet = cPetList[pos];
			return true;
		}
		return false;
	}
	virtual bool insertPet(uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name, CPet& pet)
	{
		if( cPetList.insert( CPet(next_petid, accid, cid, pet_class, pet_lv, pet_egg_id, pet_equip, intimate, hungry, renameflag, incuvat, pet_name) ) )
			return searchPet(next_petid++, pet);
		return false;
	}
	virtual bool removePet(uint32 pid)
	{
		size_t pos;
		if( cPetList.find( CPet(pid), 0, pos) )
		{
			cPetList.removeindex(pos);
			return true;
		}
		return false;
	}
	virtual bool savePet(const CPet& pet)
	{
		size_t pos;
		if( cPetList.find( pet, 0, pos) )
		{
			cPetList[pos] = pet;
			return true;
		}
		return false;
	}
};




///////////////////////////////////////////////////////////////////////////////
#else// SQL
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
#endif// SQL
///////////////////////////////////////////////////////////////////////////////

CPetDBInterface* CPetDB::getDB(const char *dbcfgfile)
{
#ifdef TXT_ONLY
	return new CPetDB_txt(dbcfgfile);
#else
	return NULL;
//	return new CPetDB_sql(dbcfgfile);
#endif// SQL
}






