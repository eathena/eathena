#include "base.h"
#include "baseio.h"
#include "lock.h"
#include "timer.h"
#include "utils.h"

///////////////////////////////////////////////////////////////////////////////
// basic mysql access
///////////////////////////////////////////////////////////////////////////////
#include "basesq.h"


///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// basic class for using the old way timers
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
			delete_timer(cTimer, timercallback);
			cTimer = -1;
		}
	}
	bool init(unsigned long interval)
	{
		if(interval<1000) 
			interval = 1000;
		cTimer = add_timer_interval(gettick()+interval, interval, timercallback, 0, (int)this);
		return (cTimer>=0);
	}

	// user function
	virtual bool timeruserfunc(unsigned long tick) = 0;

	// external calling from external timer implementation
	static int timercallback(int timer, unsigned long tick, int id, int data)
	{
		if(data)
		{
			CTimerBase* base = (CTimerBase*)data;
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
};


///////////////////////////////////////////////////////////////////////////////
#else// SQL
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
#endif// SQL
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Account Database
// for storing accounts stuff in login
///////////////////////////////////////////////////////////////////////////////

/*
	currently there is a bunch of more or less usefull data stored
	which also differs between current versions
	_useful_ data is
	///////////////////////////////////////////////////////////////////////////
*	Authentification Data used to authentify that clients are going from
	login through char to map.

	unsigned long login_id1;	// just a random id given by login
	unsigned long login_id2;	// just a random id given by login
	unsigned long client_ip;	// the current ip of the client

	a client has to show these three values to get autentified
	(gets it in login process)

	///////////////////////////////////////////////////////////////////////////
*	Account Data which holds the necessary data for an account
	
	unsigned long account_id;	// id to identify an account
	char userid[24];			// user name
	char passwd[34];			// user password
	unsigned char sex;			// gender
	unsigned char gm_level;		// gm_level
	unsigned char online;		// true when online (actually only usefull when adding datamining onto the storing data and not onto the server)
	char email[40];				// email address for confiming char deletion
	unsigned long login_count;	// number of logins
	char last_ip[16];			// ip from last login (as string)
	char last_login[24];		// timestamp of last login
	time_t ban_until;			// set to time(NULL)+delta for temporary ban
	time_t valid_until;			// set to time(NULL) for disabling

	the values state, error_message, memo are quite useless, 
	state might be usefull for debugging login of accounts
	but it is easier to read the output then to dig in the db for that

	///////////////////////////////////////////////////////////////////////////
*	Account Reg for account wide variables:

	unsigned short account_reg2_num;
	struct global_reg account_reg2[ACCOUNT_REG2_NUM];

*/

///////////////////////////////////////////////////////////////////////////////
#ifdef TXT_ONLY
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Account Database
// for storing accounts stuff in login
///////////////////////////////////////////////////////////////////////////////
class CAccountDB_txt : public CTimerBase, private CConfig, public CAccountDBInterface
{
	///////////////////////////////////////////////////////////////////////////
	// config stuff
	unsigned long next_account_id;
	char account_filename[1024];
	char GM_account_filename[1024];
	time_t creation_time_GM_account_file;

	size_t savecount;


	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CLoginAccount, 2> cList;
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
private:
	///////////////////////////////////////////////////////////////////////////
	// helper class for gm_level reading
	class CMapGM
	{
	public:
		unsigned long account_id;
		unsigned char gm_level;

		CMapGM(unsigned long accid=0) : account_id(accid), gm_level(0)	{}
		CMapGM(unsigned long accid, unsigned char lv) : account_id(accid), gm_level(lv)	{}

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
				if( !skip_empty_line(line) )
					continue;
				is_range = (sscanf(line, "%ld%*[-~]%ld %d",&start_range,&end_range,&level)==3); // ID Range [MC Cameri]
				if (!is_range && sscanf(line, "%ld %d", &account_id, &level) != 2 && sscanf(line, "%ld: %d", &account_id, &level) != 2)
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
				if( !skip_empty_line(line) )
					continue;

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
					temp.client_ip = ipaddress(last_ip);


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
				sprintf(line, "No account found in %s.", account_filename);
			}
			else
			{
				if( cList.size() == 1)
				{
					ShowStatus("1 account read in %s,\n", account_filename);
					sprintf(line, "1 account read in %s,", account_filename);
				}
				else
				{
					ShowStatus("%d accounts read in %s,\n", cList.size(), account_filename);
					sprintf(line, "%d accounts read in %s,", cList.size(), account_filename);
				}
				if (GM_count == 0)
				{
					ShowMessage("           of which is no GM account, and ");
					sprintf(str, "%s of which is no GM account and", line);
				}
				else if (GM_count == 1)
				{
					ShowMessage("           of which is 1 GM account, and ");
					sprintf(str, "%s of which is 1 GM account and", line);
				}
				else
				{
					ShowMessage("           of which is %d GM accounts, and ", GM_count);
					sprintf(str, "%s of which is %d GM accounts and", line, GM_count);
				}
				if (server_count == 0)
				{
					ShowMessage("no server account ('S').\n");
					sprintf(line, "%s no server account ('S').", str);
				}
				else if (server_count == 1)
				{
					ShowMessage("1 server account ('S').\n");
					sprintf(line, "%s 1 server account ('S').", str);
				}
				else
				{
					ShowMessage("%d server accounts ('S').\n", server_count);
					sprintf(line, "%s %d server accounts ('S').", str, server_count);
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
		if ((fp = lock_fopen(account_filename, &lock)) == NULL) {
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
						cList[i].account_id, 
						cList[i].userid, 
						cList[i].passwd, 
						(*cList[i].last_login)?cList[i].last_login:"-",
						(cList[i].sex == 2) ? 'S' : (cList[i].sex ? 'M' : 'F'),
						cList[i].login_count, 
						cList[i].state,
						(*cList[i].email)?cList[i].email:"a@a.com", 
						(*cList[i].error_message)?cList[i].error_message:"-", 
						(unsigned long)cList[i].valid_until,
						(*cList[i].last_ip)?cList[i].last_ip:"-", 
						(*cList[i].memo)?cList[i].memo:"-", 
						(unsigned long)cList[i].ban_until);
			for(k = 0; k< cList[i].account_reg2_num; k++)
				if(cList[i].account_reg2[k].str[0])
					fprintf(fp, "%s,%ld ", cList[i].account_reg2[k].str, cList[i].account_reg2[k].value);
			fprintf(fp, RETCODE);
		}
		fprintf(fp, "%ld\t%%newid%%"RETCODE, next_account_id);
		lock_fclose(fp, account_filename, &lock);
		return true;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	// functions for db interface
	virtual size_t size()						{ return cList.size(); }
	virtual CLoginAccount& operator[](size_t i)	{ return cList[i]; };

	virtual bool existAccount(const char* userid);
	virtual bool searchAccount(const char* userid, CLoginAccount&account);
	virtual bool searchAccount(unsigned long accid, CLoginAccount&account);
	virtual bool insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account);
	virtual bool removeAccount(unsigned long accid);
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


		}
		return true;
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
bool CAccountDB_txt::searchAccount(unsigned long accid, CLoginAccount&account)
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
	unsigned long accid = next_account_id++;
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
bool CAccountDB_txt::removeAccount(unsigned long accid)
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
	///////////////////////////////////////////////////////////////////////////
	// config stuff
	unsigned long next_char_id;

	char char_txt[1024];
	char backup_txt[1024];
	char friends_txt[1024];
	bool backup_txt_flag;

	bool name_ignoring_case;
	int char_name_option;
	char char_name_letters[256];
	unsigned long start_zeny;
	unsigned short start_weapon;
	unsigned short start_armor;
	struct point start_point;

	size_t savecount;

	///////////////////////////////////////////////////////////////////////////
	// data
	TMultiListP<CCharCharacter, 2>	cCharList;
	TslistDCT<CCharCharAccount>		cAccountList;

public:
	CCharDB_txt(const char *dbcfgfile) :
		CTimerBase(300*1000)		// 300sec save interval
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
		safestrcpy(start_point.map, "new_1-1.gat", sizeof(start_point.map));
		start_point.x=53;
		start_point.x=111;

		init(dbcfgfile);
	}
	~CCharDB_txt()	{}


private:
	///////////////////////////////////////////////////////////////////////////
	// Function to create the character line (for save)
	int char_to_str(char *str, size_t sz, const CCharCharacter &p)
	{
		size_t i;
		char *str_p = str;

		point last_point = p.last_point;

		if (last_point.map[0] == '\0') {
			memcpy(last_point.map, "prontera.gat", 16);
			last_point.x = 273;
			last_point.y = 354;
		}

		str_p += sprintf(str_p, 
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
			p.char_id, 
			p.account_id, p.slot, 
			p.name,
			p.class_, p.base_level, p.job_level,
			p.base_exp, p.job_exp, p.zeny,
			p.hp, p.max_hp, p.sp, p.max_sp,
			p.str, p.agi, p.vit, p.int_, p.dex, p.luk,
			p.status_point, p.skill_point,
			p.option, MakeWord(p.karma, p.chaos), p.manner,
			p.party_id, p.guild_id, p.pet_id,
			p.hair, p.hair_color, p.clothes_color,
			p.weapon, p.shield, p.head_top, p.head_mid, p.head_bottom,
			// store the checked lastpoint
			last_point.map, last_point.x, last_point.y,
			p.save_point.map, p.save_point.x, p.save_point.y,
			p.partner_id,p.father_id,p.mother_id,p.child_id,
			p.fame_points);
		for(i = 0; i < MAX_MEMO; i++)
			if (p.memo_point[i].map[0]) {
				str_p += sprintf(str_p, "%s,%d,%d", p.memo_point[i].map, p.memo_point[i].x, p.memo_point[i].y);
			}
		*(str_p++) = '\t';

		for(i = 0; i < MAX_INVENTORY; i++)
			if (p.inventory[i].nameid) {
				str_p += sprintf(str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
						 p.inventory[i].id, p.inventory[i].nameid, p.inventory[i].amount, p.inventory[i].equip,
						 p.inventory[i].identify, p.inventory[i].refine, p.inventory[i].attribute,
						 p.inventory[i].card[0], p.inventory[i].card[1], p.inventory[i].card[2], p.inventory[i].card[3]);
			}
		*(str_p++) = '\t';

		for(i = 0; i < MAX_CART; i++)
			if (p.cart[i].nameid) {
				str_p += sprintf(str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
						 p.cart[i].id, p.cart[i].nameid, p.cart[i].amount, p.cart[i].equip,
						 p.cart[i].identify, p.cart[i].refine, p.cart[i].attribute,
						 p.cart[i].card[0], p.cart[i].card[1], p.cart[i].card[2], p.cart[i].card[3]);
			}
		*(str_p++) = '\t';

		for(i = 0; i < MAX_SKILL; i++)
			if (p.skill[i].id && p.skill[i].flag != 1) {
				str_p += sprintf(str_p, "%d,%d ", p.skill[i].id, (p.skill[i].flag == 0) ? p.skill[i].lv : p.skill[i].flag-2);
			}
		*(str_p++) = '\t';

		for(i = 0; i < p.global_reg_num; i++)
			if (p.global_reg[i].str[0])
				str_p += sprintf(str_p, "%s,%ld ", p.global_reg[i].str, p.global_reg[i].value);
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
			p.last_point.map, &tmp_int[35], &tmp_int[36], //
			p.save_point.map, &tmp_int[37], &tmp_int[38], &tmp_int[39], 
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
			p.last_point.map, &tmp_int[35], &tmp_int[36], //
			p.save_point.map, &tmp_int[37], &tmp_int[38], &tmp_int[39], 
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
			p.last_point.map, &tmp_int[35], &tmp_int[36], //
			p.save_point.map, &tmp_int[37], &tmp_int[38], &tmp_int[39],
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
			p.last_point.map, &tmp_int[35], &tmp_int[36],
			p.save_point.map, &tmp_int[37], &tmp_int[38], &tmp_int[39], &next) == 43 )
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
			p.last_point.map, &tmp_int[35], &tmp_int[36], //
			p.save_point.map, &tmp_int[37], &tmp_int[38], &next) == 42 )
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
			p.last_point.map, &tmp_int[35], &tmp_int[36], //
			p.save_point.map, &tmp_int[37], &tmp_int[38], &next) == 41 )
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
			ShowMessage("           Character id #%ld -> new character not read.\n", p.char_id);
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
				if (sscanf(str+next, "%[^,],%d,%d%n", p.memo_point[i].map, &tmp_int[0], &tmp_int[1], &len) != 3)
				{
					ShowError(CL_BT_RED"Character Memo points invalid (id #%ld, name '%s').\n"CL_NORM, p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}
				p.memo_point[i].x = tmp_int[0];
				p.memo_point[i].y = tmp_int[1];
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
				}
				else // invalid structure
				{
					ShowError(CL_BT_RED"Character Inventory invalid (id #%ld, name '%s').\n"CL_NORM, p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}

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
					ShowError(CL_BT_RED"Character Cart Items invalid (id #%ld, name '%s').\n"CL_NORM, p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}

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
					ShowError(CL_BT_RED"Character skills invalid (id #%ld, name '%s').\n"CL_NORM, p.char_id, p.name);
					ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
					ret = false;
					break;
				}
				p.skill[tmp_int[0]].id = tmp_int[0];
				p.skill[tmp_int[0]].lv = tmp_int[1];
				next += len;
				if (str[next] == ' ')
					next++;
			}
		}

		if(ret)
		{	// start with the next char after the delimiter
			next++;
			for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; i++)
			{	// global_reg実装以前のathena.txt互換のため一応'\n'チェック
				if(sscanf(str + next, "%[^,],%ld%n", p.global_reg[i].str, &p.global_reg[i].value, &len) != 2)
				{	// because some scripts are not correct, the str can be "". So, we must check that.
					// If it's, we must not refuse the character, but just this REG value.
					// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
					if(str[next] == ',' && sscanf(str + next, ",%ld%n", &p.global_reg[i].value, &len) == 1)
						i--;
					else
					{
						ShowError(CL_BT_RED"Character Char Variable invalid (id #%ld, name '%s').\n"CL_NORM, p.char_id, p.name);
						ShowMessage("           Rest skipped, line saved to log file.\n", p.name);
						ret = false;
						break;
					}

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

			if( !skip_empty_line(line) )
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
			fb= lock_fopen(backup_txt, &lock);

		// Data save
		fp = lock_fopen(char_txt, &lock);
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
				fprintf(fp, "%s"RETCODE, line);
				if(fb) fprintf(fb, "%s"RETCODE, line);
			}
			fprintf(fp, "%d\t%%newid%%" RETCODE, next_char_id);
			lock_fclose(fp, char_txt, &lock);
			if(fb) lock_fclose(fp, backup_txt, &lock);
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
		memcpy(&tempchar.last_point, &start_point, sizeof(start_point));
		memcpy(&tempchar.save_point, &start_point, sizeof(start_point));


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
					fprintf(fp, "%ld", cCharList[i].char_id);
					for (k=0;k<MAX_FRIENDLIST;k++)
					{
						if (cCharList[i].friendlist[k].friend_id > 0 && cCharList[i].friendlist[k].friend_name[0])
							fprintf(fp, ",%ld,%s", cCharList[i].friendlist[k].friend_id,cCharList[i].friendlist[k].friend_name);
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
			struct friends friendlist[20];
			size_t pos;

			while(fgets(line, sizeof(line), fp))
			{
				if( !skip_empty_line(line) )
					continue;

				memset(friendlist,0,sizeof(friendlist));
				sscanf(line, "%ld,%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%24[^,],%ld,%s",
					&cid,
				&friendlist[ 0].friend_id,friendlist[ 0].friend_name,
				&friendlist[ 1].friend_id,friendlist[ 1].friend_name,
				&friendlist[ 2].friend_id,friendlist[ 2].friend_name,
				&friendlist[ 3].friend_id,friendlist[ 3].friend_name,
				&friendlist[ 4].friend_id,friendlist[ 4].friend_name,
				&friendlist[ 5].friend_id,friendlist[ 5].friend_name,
				&friendlist[ 6].friend_id,friendlist[ 6].friend_name,
				&friendlist[ 7].friend_id,friendlist[ 7].friend_name,
				&friendlist[ 8].friend_id,friendlist[ 8].friend_name,
				&friendlist[ 9].friend_id,friendlist[ 9].friend_name,
				&friendlist[10].friend_id,friendlist[10].friend_name,
				&friendlist[11].friend_id,friendlist[11].friend_name,
				&friendlist[12].friend_id,friendlist[12].friend_name,
				&friendlist[13].friend_id,friendlist[13].friend_name,
				&friendlist[14].friend_id,friendlist[14].friend_name,
				&friendlist[15].friend_id,friendlist[15].friend_name,
				&friendlist[16].friend_id,friendlist[16].friend_name,
				&friendlist[17].friend_id,friendlist[17].friend_name,
				&friendlist[18].friend_id,friendlist[18].friend_name,
				&friendlist[19].friend_id,friendlist[19].friend_name);

				if( cCharList.find( CCharCharacter(cid), pos, 0) )
				{	
					CCharCharacter &temp = cCharList(pos,0);
					size_t i;
					for (i=0; i<MAX_FRIENDLIST; i++)
					{
						temp.friendlist[i] = friendlist[i];
					}
				}
			}//end while
			fclose(fp);
			return true;
		}
		return false;
	}


public:
	///////////////////////////////////////////////////////////////////////////
	// access interface
	virtual size_t size()	{ return cCharList.size(); }
	virtual CCharCharacter& operator[](size_t i)	{ return cCharList[i]; }

	virtual bool existChar(const char* name);
	virtual bool searchChar(const char* name, CCharCharacter&data);
	virtual bool searchChar(unsigned long charid, CCharCharacter&data);
	virtual bool insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data);
	virtual bool removeChar(unsigned long charid);
	virtual bool saveChar(const CCharCharacter& data);

	virtual bool searchAccount(unsigned long accid, CCharCharAccount& account);
	virtual bool saveAccount(CCharAccount& account);
	virtual bool removeAccount(unsigned long accid);

private:
	///////////////////////////////////////////////////////////////////////////
	// Config processor
	virtual bool ProcessConfig(const char*w1, const char*w2);

	///////////////////////////////////////////////////////////////////////////
	// normal function
	bool init(const char* configfile)
	{	// init db
		CConfig::LoadConfig(configfile);
		return read_chars() && read_friends();
	}
	bool close()
	{
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
		char map[32];
		int x, y;
		if(sscanf(w2, "%[^,],%d,%d", map, &x, &y) == 3 &&  NULL!=strstr(map, ".gat") )
		{	// Verify at least if '.gat' is in the map name
			safestrcpy(start_point.map, map, sizeof(start_point.map));
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
bool CCharDB_txt::searchChar(unsigned long charid, CCharCharacter&data)
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
bool CCharDB_txt::removeChar(unsigned long charid)
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
bool CCharDB_txt::searchAccount(unsigned long accid, CCharCharAccount& account)
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
bool CCharDB_txt::removeAccount(unsigned long accid)
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
// a text based database with extra index
// not much tested though and quite basic
// but should be as fast as file access allows 
// faster than sql anyway + threadsafe (not shareable though)
///////////////////////////////////////////////////////////////////////////////
#define DB_OPCOUNTMAX 100	// # of db ops before forced cache flush

class txt_database : private Mutex
{
	///////////////////////////////////////////////////////////////////////////
	// index structure
	class _key
	{
	public:
		ulong cKey;	// key value
		ulong cPos;	// position in file
		ulong cLen;	// data length

		_key(ulong k=0, ulong p=0, ulong l=0):cKey(k), cPos(p), cLen(l)	{}
		bool operator==(const _key& k) const	{ return cKey==k.cKey; }
		bool operator> (const _key& k) const	{ return cKey> k.cKey; }
		bool operator< (const _key& k) const	{ return cKey< k.cKey; }
	};
	///////////////////////////////////////////////////////////////////////////
	// class data
	char *cName;			// path/name of db file
	FILE *cDB;				// file handle to db
	FILE *cIX;				// file handle to index

	size_t cOpCount;		// count of operations

	TslistDST<_key>	cIndex;	// the index

	///////////////////////////////////////////////////////////////////////////
public:
	///////////////////////////////////////////////////////////////////////////
	// construction/destruction
	txt_database() : cName(NULL),cDB(NULL), cIX(NULL), cOpCount(0)
	{}
	~txt_database()
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
		// <# of entries> \n <i>(0), <p>(0), <l>(0) \n ...
		fseek(cIX, 0, SEEK_SET);
		unsigned long sz, p, l;
		if( 1==fscanf(cIX,"%li\n", &sz) )
		{
			cIndex.realloc(sz);
			while( 3==fscanf(cIX,"%li,%li,%li\n",  &sz, &p, &l) )
			{
				cIndex.insert( _key(sz,p,l) );
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
			}
			if(cIX) 
			{
				// write index
				fseek(cIX, 0, SEEK_SET);
				fprintf(cIX,"%li\n", cIndex.size());
				for(size_t i=0; i<cIndex.size(); i++)
				{
					fprintf(cIX,"%li,%li,%li\n", 
						cIndex[i].cKey, cIndex[i].cPos, cIndex[i].cLen);
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
	bool insert(const ulong key, char* data)
	{
		if(!data)
			return false;

		ScopeLock sl(*this);
		ulong len = strlen(data);
		size_t i;
		if( cIndex.find( _key(key), 0, i) )
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
			cIndex.insert( _key(key, ftell(cDB), len) );
		}
		fwrite(data, 1, len, cDB);

		this->flush();
		return true;
	}
	///////////////////////////////////////////////////////////////////////////
	// delete
	bool remove(const ulong key)
	{
		ScopeLock sl(*this);
		size_t pos;
		if( cIndex.find( _key(key), 0, pos) )
		{
			cIndex.removeindex(pos);
			this->flush();
			return true;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// search
	bool find(const ulong key, char* data) const
	{
		if(!data)
			return false;

		ScopeLock sl(*this);
		size_t i;
		if( cIndex.find( _key(key), 0, i) )
		{
			fseek(cDB, cIndex[i].cPos, SEEK_SET);
			fread(data, cIndex[i].cLen, 1, cDB);
			data[cIndex[i].cLen]=0;
			return true;
		}
		return false;
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
		ulong k, p, l;

		char *ip = cName+strlen(cName);
		strcpy(ip, ".tmp");
		
		FILE* tmp=fopen(cName, "wb");
		if(!tmp)
			return false;

		for(size_t i=0; i<cIndex.size(); i++)
		{	
			k=cIndex[i].cKey;
			p=ftell(tmp);
			l=cIndex[i].cLen;

			fseek(cDB,cIndex[i].cPos, SEEK_SET);
			fread (buffer, l,1,cDB);

			fwrite(buffer, l,1,tmp);

			inx.insert( _key(k,p,l) );
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

		// cut the file name back to default
		*ip=0;
		return true;
	}
};
///////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// basic interface for reading configs from file
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Loading a file, stripping lines, splitting it to part1 : part2
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
		if (sscanf(ip, "%[^:]: %[^\r\n]", w1, w2) == 2)
		{
			CleanControlChars(w1);
			CleanControlChars(w2);
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

// Convert a string into an IP
ulong CConfig::String2IP(const char* str)
{	// return value is host byte order
	// look up the name, can take long for timeout looking up non-existing addresses

	struct hostent *h = gethostbyname(str);
	if (h != NULL) 
	{	// returned ip's are hostbyte order
		return	  (((ulong)h->h_addr[3]) << 0x18 )
				| (((ulong)h->h_addr[2]) << 0x10 )
				| (((ulong)h->h_addr[1]) << 0x08 )
				| (((ulong)h->h_addr[0])         );
	}
	else
	{	// assume string is in ip format, just convert it
		return ntohl(inet_addr(str));
	}
}

// Convert IP to string
const char* CConfig::IP2String(ulong ip, char*buffer)
{	// given ip is in host byte order
	// usage of the static buffer here is not threadsave
	static char temp[32], *pp= (buffer) ? buffer:temp;
	sprintf(pp, "%d.%d.%d.%d", (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
	return pp;
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

// Replace control chars with '_' and return location of change
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
	CAuth::size();
	CAccountReg::size();
}

void CMapAccount::_tobuffer(unsigned char* &buf) const
{
	unsigned long time;

	if(!buf) return;

	_B_tobuffer( sex,			buf);
	_B_tobuffer( gm_level,		buf);
	time = ban_until;	_L_tobuffer( time, buf);
	time = valid_until;	_L_tobuffer( time, buf);
	CAuth::_tobuffer(buf);
	CAccountReg::_tobuffer(buf);
}

void CMapAccount::_frombuffer(const unsigned char* &buf)
{
	unsigned long time;

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

