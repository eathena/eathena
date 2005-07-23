#ifndef __IO_H__
#define __IO_H__

#include "base.h"
#include "showmsg.h"	// ShowMessage
#include "utils.h"		// safefopen
#include "socket.h"		// buffer iterator
#include "timer.h"		// timed config reload
#include "dbaccess.h"	// SQL DB ACCESS includes
#include "strlib.h"
#include "mmo.h"






//////////////////////////////////////////////////////////////////////////
// basic interface for reading configs from file
//////////////////////////////////////////////////////////////////////////
class CConfig
{
public:
	CConfig()
	{}
	virtual ~CConfig()
	{}

	//////////////////////////////////////////////////////////////////////
	// Loading a file, stripping lines, splitting it to part1 : part2
	// calling the derived function for processing
	//////////////////////////////////////////////////////////////////////
	bool LoadConfig(const char* cfgName)
	{
		char line[1024], w1[1024], w2[1024], *ip;
		FILE *fp;

		if ((fp = safefopen(cfgName, "r")) == NULL) {
			ShowError("Configuration file (%s) not found.\n", cfgName);
			return false;
		}
		ShowInfo("Reading configuration file '%s'\n", cfgName);
		while(fgets(line, sizeof(line)-1, fp))
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
				{	// call recursive, prevent infinity loop
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
	}
	//////////////////////////////////////////////////////////////////////
	// virtual function for processing/storing tokens
	//////////////////////////////////////////////////////////////////////
	virtual bool ProcessConfig(const char*w1,const char*w2) = 0;


	//////////////////////////////////////////////////////////////////////
	// some global data processings
	//////////////////////////////////////////////////////////////////////
	static ulong String2IP(const char* str)
	{	// host byte order
		struct hostent *h = gethostbyname(str);
		if (h != NULL)
		{	// ip's are hostbyte order
			return	  (((ulong)h->h_addr[3]) << 0x18 )
					| (((ulong)h->h_addr[2]) << 0x10 )
					| (((ulong)h->h_addr[1]) << 0x08 )
					| (((ulong)h->h_addr[0])         );
		}
		else
			return ntohl(inet_addr(str));
	}
	static const char* IP2String(ulong ip, char*buffer=NULL)
	{	// host byte order
		// usage of the static buffer here is not threadsave
		static char temp[20], *pp= (buffer) ? buffer:temp;

		sprintf(pp, "%d.%d.%d.%d", (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
		return pp;
	}

	static int SwitchValue(const char *str, int defaultval=0)
	{
		if( str )
		{
			if (strcasecmp(str, "on") == 0 || strcasecmp(str, "yes") == 0 || strcasecmp(str, "oui") == 0 || strcasecmp(str, "ja") == 0 || strcasecmp(str, "si") == 0)
				return 1;
			else if (strcasecmp(str, "off") == 0 || strcasecmp(str, "no" ) == 0 || strcasecmp(str, "non") == 0 || strcasecmp(str, "nein") == 0)
				return 0;
			else
				return atoi(str);
		}
		else
			return defaultval;
	}
	static bool Switch(const char *str, bool defaultval=false)
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
	static bool CleanControlChars(char *str)
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

};









//////////////////////////////////////////////////////////////////////////
// Basic Database Interfaces
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// defaults for a sql database
//////////////////////////////////////////////////////////////////////////
class DBDefaults : public CConfig
{
protected:
	const char *sql_conf_name;
public:
	// server
	int	 server_port;
	char server_ip[32];
	char server_id[32];
	char server_pw[32];
	char server_db[32];

	// tables
	char table_login[256];
	char table_loginlog[256];

	// columns
	char column_account_id[256];
	char column_userid[256];
	char column_user_pass[256];
	char column_level[256];



	DBDefaults() : sql_conf_name("conf/inter_athena.conf")
	{	// setting defaults
		// db server defaults
		server_port = 3306;
		strcpy(server_ip, "127.0.0.1");
		strcpy(server_id, "ragnarok");
		strcpy(server_pw, "ragnarok");
		strcpy(server_db, "ragnarok");

		// db table defaults
		strcpy(table_login, "login");
		strcpy(table_loginlog, "loginlog");

		// db column defaults
		strcpy(column_account_id, "account_id");
		strcpy(column_userid, "userid");
		strcpy(column_user_pass, "user_pass");
		strcpy(column_level, "level");

		// loading from default file
		LoadConfig(sql_conf_name);
	}

	virtual bool ProcessConfig(const char*w1, const char*w2)
	{
		/////////////////////////////////////////////////////////////
		//add for DB connection
		/////////////////////////////////////////////////////////////
		if(strcasecmp(w1,"login_server_ip")==0){
			strcpy(server_ip, w2);
			ShowMessage ("set Database Server IP: %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_port")==0){
			server_port=atoi(w2);
			ShowMessage ("set Database Server Port: %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_id")==0){
			strcpy(server_id, w2);
			ShowMessage ("set Database Server ID: %s\n",w2);
		}
		else if(strcasecmp(w1,"login_server_pw")==0){
			strcpy(server_pw, w2);
			ShowMessage ("set Database Server PW: %s\n",w2);
		}

		/////////////////////////////////////////////////////////////
		// tables
		/////////////////////////////////////////////////////////////
		else if(strcasecmp(w1,"login_server_db")==0){
			strcpy(table_login, w2);
			ShowMessage ("set Database Server DB: %s\n",w2);
		}
		else if (strcasecmp(w1, "loginlog_db") == 0) {
			strcpy(table_loginlog, w2);
		}

		/////////////////////////////////////////////////////////////
		//added for custom column names for custom login table
		/////////////////////////////////////////////////////////////
		else if(strcasecmp(w1,"login_db_account_id")==0){
			strcpy(column_account_id, w2);
		}
		else if(strcasecmp(w1,"login_db_userid")==0){
			strcpy(column_userid, w2);
		}
		else if(strcasecmp(w1,"login_db_user_pass")==0){
			strcpy(column_user_pass, w2);
		}
		else if(strcasecmp(w1,"login_db_level")==0){
			strcpy(column_level, w2);
		}

		/////////////////////////////////////////////////////////////
		//support the import command, just like any other config
		/////////////////////////////////////////////////////////////
		else if(strcasecmp(w1,"import")==0){
			LoadConfig(w2);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// basic interface to a sql database
//////////////////////////////////////////////////////////////////////////

class Database : public global, public noncopyable
{
protected:
	DBDefaults& def;

	Database(DBDefaults &d) : def(d)
	{
		Open();
	}
	~Database()
	{
		Close();
	}
	bool Open()
	{
		bool ret = true;
		//DB connection start
		ShowInfo("Connect DB Server (%s)....\n", def.server_db);
		sql_connect(def.server_ip,def.server_id,def.server_pw,def.server_db,def.server_port)
		ShowStatus("connect success!\n");

		sql_query("INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver', '100','login server started')", def.table_loginlog);
		return ret;
	}

	bool Close()
	{
		bool ret = true;
		//set log.
		sql_query("INSERT DELAYED INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '', 'lserver','100', 'login server shutdown')", def.table_loginlog);

		//query
		//delete all server status
		sql_query("DELETE FROM `sstatus`",NULL);

		sql_close();
		ShowMessage("Close DB Connection (%s)....\n", def.server_db);

		return ret;
	}

};


class Login_DB : public Database
{
public:
	Login_DB(DBDefaults& d) : Database(d)
	{
	}
	~Login_DB()
	{
	}

	int isGM(unsigned long account_id) {
		int level;

		level = 0;
		sql_query("SELECT `%s` FROM `%s` WHERE `%s`='%d'",
			def.column_level, def.table_login, def.column_account_id, account_id);
		if (sql_res)
		{
			sql_fetch_row();
			level = atoi(sql_row[0]);
			if (level > 99)
				level = 99;
		}
		sql_free();
		return level;
	}


	bool NewAccount(const char*userid, const char* passwd, const char *sex)
	{
		bool ret = false;
		char t_uid[256], t_pass[256];

		jstrescapecpy(t_uid,  userid);
		jstrescapecpy(t_pass, passwd);

		sql_query("SELECT `%s` FROM `%s` WHERE `userid` = '%s'",def.column_userid, def.table_login, t_uid);

		if(sql_num_rows() == 0)
		{	// ok no existing acc,

			ShowMessage("Adding a new account user: %s with passwd: %s sex: %c\n",
				userid, passwd, sex);

			sql_query("INSERT INTO `%s` (`%s`, `%s`, `sex`, `email`) VALUES ('%s', '%s', '%c', '%s')",
				def.table_login, def.column_userid, def.column_user_pass,
				t_uid, t_pass, sex, "a@a.com");
			ret =false;

		}//rownum check (0!)
		sql_free();
		//all values for NEWaccount ok ?
		return ret;
	}


};









//////////////////////////////////////////////////////////////////////////
// Database Definitions
//////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// GM Database
///////////////////////////////////////////////////////////////////////////////

class GM_Database : public Database
{
	//////////////////////////////////////////////////////////////////////////
	// database entry structure
	class gm_account
	{
	public:
		unsigned long account_id;
		unsigned char level;

		gm_account(int a=0, int l=0):account_id(a), level(l)	{}
		bool operator==(const gm_account&g) const	{ return account_id==g.account_id; }
		bool operator> (const gm_account&g) const	{ return account_id> g.account_id; }
		bool operator< (const gm_account&g) const	{ return account_id< g.account_id; }
	};
	//////////////////////////////////////////////////////////////////////////
	// database itself is just a sorted list
	TslistDST<gm_account> gm_list;
	//////////////////////////////////////////////////////////////////////////


	GM_Database(DBDefaults& d) : Database(d)
	{
		read_gm_account();
	}
	~GM_Database()
	{}


	void read_gm_account(void)
	{
		gm_list.clear();

		sql_query("SELECT `%s`,`%s` FROM `%s`",
			def.column_account_id, def.column_level, def.table_login);

		size_t line_counter = 0;
		if (sql_res) {

			gm_list.realloc( (size_t)sql_num_rows() );

			while( sql_fetch_row() && line_counter < 4000)
			{
				line_counter++;
				gm_list.push( gm_account(atoi(sql_row[0]), atoi(sql_row[1])) );
			}
		}
		sql_free();
	}

	bool _tobuffer(unsigned char*buffer, size_t &btsz)
	{
		// sizeof(gm_account) might get wrong size
		// we stuff 4byte account_id and 1byte level = 5
		btsz = 5 * gm_list.size();
		buffer_iterator bi(buffer, btsz);
		gm_list.clear();
		for(size_t i=0; i<gm_list.size() && !bi.eof(); i++ )
		{
			bi = gm_list[i].account_id;
			bi = gm_list[i].level;
		}
		return true;
	}
	bool _frombuffer(const unsigned char*buffer, size_t btsz)
	{
		unsigned long ac;
		unsigned char lv;
		buffer_iterator bi(buffer, btsz);
		gm_list.clear();
		while( !bi.eof() )
		{
			ac = bi;
			lv = bi;
			gm_list.push( gm_account(ac, lv) );
		}
		return true;
	}

	unsigned char is_GM(unsigned long account_id)
	{
		size_t pos;
		if( gm_list.find(account_id, 0, pos) )
			return gm_list[pos].level;
		return 0;
	}
};







void database_log_interal(ulong ip, const char*user_id, int code, char*msg)
{	// log the message to database or txt

}



void database_log(ulong ip, const char*user_id, int code, char*msg,...)
{
	// initially using a static fixed buffer size
	static char		tempbuf[4096];
	// and make it multithread safe
	static Mutex	mtx;
	ScopeLock		sl(mtx);

	size_t sz  = 4096; // initial buffer size
	char *ibuf = tempbuf;
	va_list argptr;
	va_start(argptr, msg);

	do{
		// print
		if( vsnprintf(ibuf, sz, msg, argptr) >=0 ) // returns -1 in case of error
			break; // print ok, can break
		// otherwise
		// aFree the memory if it was dynamically alloced
		if(ibuf!=tempbuf) delete[] ibuf;
		// double the size of the buffer
		sz *= 2;
		ibuf = new char[sz];
		// and loop in again
	}while(1);

	database_log_interal(ip,user_id,code,ibuf);

	va_end(argptr);
	if(ibuf!=tempbuf) delete[] ibuf;
	//mutex will be released on scope exit
}

















//////////////////////////////////////////////////////////////////////////
// Interface for objects that might be transfered via bytestream
//////////////////////////////////////////////////////////////////////////

class CGlobalReg : public streamable
{
public:
	char str[32];
	long value;


	virtual bool toBuffer(buffer_iterator& bi) const
	{
		bi.str2buffer(str,32);
		bi = value;
		bi = (long)10;
	}
	virtual bool fromBuffer(buffer_iterator& bi)
	{
		bi.buffer2str(str,32);
		value = bi;
	}

};


class CAuthData
{
public:
	long account_id;
	char sex;
	char userid[24];
	char pass[33]; // 33 for 32 + NULL terminated
	char lastlogin[24];
	long logincount;
	long state; // packet 0x006a value + 1 (0: compte OK)
	char email[40]; // e-mail (by default: a@a.com)
	char error_message[20]; // Message of error code #6 = Your are Prohibited to log in until %s (packet 0x006a)
	time_t ban_until_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	char last_ip[16]; // save of last IP of connection
	char memo[255]; // a memo field
	long account_reg2_num;

	CGlobalReg globalreg[ACCOUNT_REG2_NUM];


	void test()
	{
		unsigned char buffer[10];
		buffer_iterator bi(buffer, 10);

		bi = globalreg[1];

	}

};


class logger : private Mutex
{
	bool	cToScreen;
	FILE*	cToFile;

	bool log_screen(const char* msg, va_list va)
	{
		if(cToScreen)
		{
			ScopeLock sl(*this);
			vprintf(msg,va);
			return true;
		}
		return false;
	}

	bool log_file(const char* msg, va_list va)
	{
		if(cToFile)
		{	ScopeLock sl(*this);
			vfprintf(cToFile,msg,va);
			return true;
		}
		return false;
	}
public:

	logger(bool s=true) : cToScreen(s), cToFile(NULL)
	{
	}
	logger(bool s, const char* n) : cToScreen(s), cToFile(NULL)
	{
		open(n);
	}
	logger(const char* n) : cToScreen(true), cToFile(NULL)
	{
		open(n);
	}
	~logger()
	{
		close();
	}

	bool open(const char* name)
	{
		close();
		cToFile = fopen(name, "a");
		return (NULL != cToFile);
	}

	bool close()
	{
		if(cToFile)
		{
			fclose(cToFile);
			cToFile = NULL;
		}
		return true;
	}

	bool SetLog(bool s)
	{
		bool x=cToScreen;
		cToScreen = s;
		return x;
	}
	bool isLog()
	{
		return cToScreen;
	}

	bool log(const char* msg, ...)
	{
		bool ret;
		va_list va;
		va_start( va, msg );
		ret  = log_screen(msg, va);
		ret &= log_file(msg, va);
		va_end( va );
		return ret;
	}
	bool log_screen(const char* msg,...)
	{
		bool ret;
		va_list va;
		va_start( va, msg );
		ret = log_screen(msg, va);
		va_end( va );
		return ret;
	}
	bool log_file(const char* msg,...)
	{
		bool ret;
		va_list va;
		va_start( va, msg );
		ret = log_file(msg, va);
		va_end( va );
		return ret;
	}
};









#endif//__IO_H__