// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "baseio.h"
#include "lock.h"
#include "timer.h"
#include "utils.h"

#include "basetx.h"



///////////////////////////////////////////////////////////////////////////////
/// read gm accounts from external gmaccountfile
bool CAccountDB_mem::readGMAccount(basics::slist<CAccountDB_mem::CMapGM> &gmlist)
{
	basics::CParam< basics::string<> > GM_account_filename("GM_account_filename", "conf/GM_account.txt");

	struct stat file_stat;
	FILE *fp;

	// clear all gm_levels
	gmlist.clear();
	

	// get last modify time/date
	creation_time_GM_account_file = (0==stat(GM_account_filename(), &file_stat))? file_stat.st_mtime : 0;

	fp = basics::safefopen(GM_account_filename(), "rb");
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
			if( !is_valid_line(line) )
				continue;
			is_range = (sscanf(line, "%lu%*[-~]%lu %d",&start_range,&end_range,&level)==3); // ID Range [MC Cameri]
			if ( !is_range && sscanf(line, "%lu%*[:=]%d", &account_id, &level)  != 2 && sscanf(line, "%lu %d", &account_id, &level)  != 2)
				ShowError("read_gm_account: file [%s], invalid 'acount_id|range level' format (line #%d).\n", (const char*)GM_account_filename(), line_counter);
			else if (level <= 0)
				ShowError("read_gm_account: file [%s] %dth account (line #%d) (invalid level [0 or negative]: %d).\n", (const char*)GM_account_filename(), gmlist.size()+1, line_counter, level);
			else
			{
				if (level > 99)
				{
					ShowWarning("read_gm_account: file [%s] %dth account (invalid level, but corrected: %d->99).\n", (const char*)GM_account_filename(), gmlist.size()+1, level);
					level = 99;
				}
				if (is_range)
				{
					if (start_range==end_range)
						ShowError("read_gm_account: file [%s] invalid range, beginning of range is equal to end of range (line #%d).\n", (const char*)GM_account_filename(), line_counter);
					else if (start_range>end_range)
						ShowError("read_gm_account: file [%s] invalid range, beginning of range must be lower than end of range (line #%d).\n", (const char*)GM_account_filename(), line_counter);
					else
						for (current_id = start_range; current_id<=end_range; ++current_id)
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
		ShowStatus("File '%s' read (%d GM accounts found).\n", (const char*)GM_account_filename(), gmlist.size());
		return true;
	}
	else
	{
		ShowWarning("External GM accounts file [%s] not found.\n", (const char*)GM_account_filename());
		return false;
	}

}
///////////////////////////////////////////////////////////////////////////////
CAccountDB_mem::CAccountDB_mem(const char* configfile) :
	creation_time_GM_account_file(0),
	cPos(0)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CAccountDB_mem::~CAccountDB_mem()
{
}
///////////////////////////////////////////////////////////////////////////////
size_t CAccountDB_mem::size() const
{
	return cList.size();
}
///////////////////////////////////////////////////////////////////////////////
CLoginAccount& CAccountDB_mem::operator[](size_t i)
{
	return cList[i];
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::existAccount(const char* userid)
{	// check if account with userid already exist
	size_t pos;
	return cList.find( CLoginAccount(userid), pos, 1);
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::searchAccount(const char* userid, CLoginAccount&account)
{	// get account by userid
	size_t pos;
	if( cList.find( CLoginAccount(userid), pos, 1) )
	{
		account = cList(pos,1);
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::searchAccount(uint32 accid, CLoginAccount&account)
{	// get account by account_id
	size_t pos;
	if( cList.find( CLoginAccount(accid), pos, 0) )
	{
		account = cList(pos,0);
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::insertAccount(const char* userid, const char* passwd, unsigned char sex, const char* email, CLoginAccount&account)
{	// insert a new account
	CLoginAccount temp;
	size_t pos;

	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_account_num("start_account_num", 10000000);
	uint32 accid = start_account_num;
	const size_t p=this->cList.size();
	if( 0==p || cList[0].account_id>accid )
	{	// take the start value
	}
	else if( cList[0].account_id+p-1 == cList[p-1].account_id )
	{	// increment last id when range is fully used
		accid = this->cList[p-1].account_id++;
	}
	else
	{	// find some unused key within
		size_t a=0, b=p, c;
		while( b > a+1 )
		{
			c=(a+b)/2;
			if( this->cList[c].account_id > accid && 
				(this->cList[c].account_id-this->cList[a].account_id) > (c-a) )
				b=c;
			else
				a=c;
		}
		accid = this->cList[a].account_id + 1;
		// don't use id's less than start_account_num		
		if( accid < start_account_num() )
			accid = start_account_num;
	}
	///////////////////////////////////////////////////////////////////////////
	
	if( this->cList.find( CLoginAccount(userid), pos, 1) )
	{	// remove an existing account
		this->do_removeAccount( this->cList(pos,1) );
		this->removeAccount(this->cList(pos,1).account_id);
	}

	this->cList.insert( CLoginAccount(accid, userid, passwd, sex, email) );

	if( this->cList.find( CLoginAccount(userid), pos, 1) )
	{
		account = this->cList(pos,1);
		this->do_createAccount( account );
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::removeAccount(uint32 accid)
{
	size_t pos;
	if( this->cList.find(CLoginAccount(accid),pos, 0) )
	{
		this->do_removeAccount( this->cList(pos,0) );
		return this->cList.removeindex(pos, 0);
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::saveAccount(const CLoginAccount& account)
{
	size_t pos;
	if( this->cList.find(account, pos, 1) )
	{
		this->cList(pos,1) = account;
		this->do_saveAccount(account);
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::aquire()
{
	this->cMx.lock();
	return this->first();
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::release()
{
	this->cMx.unlock();
	return true; 
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::first()
{
	basics::ScopeLock sl(this->cMx);
	this->cPos=0;
	return this->cList.size()>0; 
}
///////////////////////////////////////////////////////////////////////////////
CAccountDB_mem::operator bool()
{
	basics::ScopeLock sl(this->cMx);
	return this->cPos<this->cList.size();
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::operator++(int)
{
	basics::ScopeLock sl(cMx); 
	++this->cPos;
	return (*this);
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::save()
{
	if(this->cPos<this->cList.size())
	{
		this->do_saveAccount(this->cList(this->cPos,0));
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::find(const char* userid)
{
	basics::ScopeLock sl(this->cMx);
	size_t pos;
	// search in index 1
	if( cList.find( CLoginAccount(userid), pos, 1) )
	{	// set position based to index 0 
		return this->cList.find( this->cList(pos,1), this->cPos, 0);
	}
	return false; 
}
///////////////////////////////////////////////////////////////////////////////
bool CAccountDB_mem::find(uint32 accid)
{
	basics::ScopeLock sl(this->cMx);
	return this->cList.find( CLoginAccount(accid), this->cPos, 0);
}
///////////////////////////////////////////////////////////////////////////////
CLoginAccount& CAccountDB_mem::operator()()
{
	basics::ScopeLock sl(this->cMx);
	if( this->cPos>=cList.size() )
		throw "access out of bound";
	return this->cList(this->cPos,0);
}








///////////////////////////////////////////////////////////////////////////
// Function to create a new character
bool CCharDB_mem::make_new_char(CCharCharAccount& account, const char *n, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter& target)
{
	size_t pos;
	target = CCharCharacter(n);

	// check slot usage
	if( account.charlist[slot]!=0 )
	{
		return false;
	}

//!! adding name_ignoring_case

	if( cCharList.find(target, pos, 1) )
	{
		//char_log("Make new char error (name already exists): (connection #%d, account: %d) slot %d, name: %s (actual name of other char: %d), stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
		//		 fd, sd->account_id, dat[30], dat, char_dat[i].name, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);
		return false;
	}

//!! testing wisp_server_name otherwise


	//char_log("Creation of New Character: (connection #%d, account: %d) slot %d, character Name: %s, stats: %d+%d+%d+%d+%d+%d=%d, hair: %d, hair color: %d." RETCODE,
	//		 fd, sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[24] + dat[25] + dat[26] + dat[27] + dat[28] + dat[29], dat[33], dat[31]);

	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_char_num("start_char_num", 20000000);
	uint32 char_id=start_char_num;
	const size_t p=this->cCharList.size();
	if( 0==p || cCharList[0].char_id > char_id)
	{	// take the start value
	}
	else if( cCharList[0].char_id+p-1 == cCharList[p-1].char_id )
	{	// increment last id when range is fully used
		char_id = cCharList[p-1].char_id++;
	}
	else
	{	// find some unused key within
		size_t a=0, b=p, c;
		while( b > a+1 )
		{
			c=(a+b)/2;
			if( cCharList[c].char_id > char_id && 
				(cCharList[c].char_id-cCharList[a].char_id) > (c-a) )
				b=c;
			else
				a=c;
		}
		char_id = cCharList[a].char_id + 1;
		if( char_id < start_char_num() )
			char_id = start_char_num;
	}
	///////////////////////////////////////////////////////////////////////////

	target.char_id = char_id;
	target.account_id = account.account_id;
	target.slot = slot;
	target.class_ = 0;
	target.base_level = 1;
	target.job_level = 1;
	target.base_exp = 0;
	target.job_exp = 0;
	target.zeny = start_zeny;
	target.str = str;
	target.agi = agi;
	target.vit = vit;
	target.int_ = int_;
	target.dex = dex;
	target.luk = luk;
	target.max_hp = 40 * (100 + vit) / 100;
	target.max_sp = 11 * (100 + int_) / 100;
	target.hp = target.max_hp;
	target.sp = target.max_sp;
	target.status_point = 0;
	target.skill_point = 0;
	target.option = 0;
	target.karma = 0;
	target.manner = 0;
	target.party_id = 0;
	target.guild_id = 0;
	target.hair = hair_style;
	target.hair_color = hair_color;
	target.clothes_color = 0;
	target.inventory[0].nameid = start_weapon; // Knife
	target.inventory[0].amount = 1;
	target.inventory[0].equip = 0x02;
	target.inventory[0].identify = 1;
	target.inventory[1].nameid = start_armor; // Cotton Shirt
	target.inventory[1].amount = 1;
	target.inventory[1].equip = 0x10;
	target.inventory[1].identify = 1;
	target.weapon = 1;
	target.shield = 0;
	target.head_top = 0;
	target.head_mid = 0;
	target.head_bottom = 0;
	target.last_point = start_point;
	target.save_point = start_point;

	account.charlist[slot] = target.char_id;
	cCharList.insert(target);

	return true;
}

CCharDB_mem::CCharDB_mem(const char *configfile) :
	cPos(0)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CCharDB_mem::~CCharDB_mem()
{
}



///////////////////////////////////////////////////////////////////////////
// access interface
size_t CCharDB_mem::size() const
{
	return cCharList.size();
}
CCharCharacter& CCharDB_mem::operator[](size_t i)
{
	return cCharList[i];
}

bool CCharDB_mem::existChar(const char* name)
{
	size_t pos;
	return cCharList.find( CCharCharacter(name), pos, 1);
}
bool CCharDB_mem::searchChar(const char* name, CCharCharacter&data)
{
	size_t pos;
	if( cCharList.find( CCharCharacter(name), pos, 1) )
	{
		data = cCharList[pos];
		return true;
	}
	return false;
}
bool CCharDB_mem::searchChar(uint32 charid, CCharCharacter&data)
{
	size_t pos;
	if( cCharList.find( CCharCharacter(charid), pos, 0) )
	{
		data = cCharList[pos];
		return true;
	}
	return false;
}
bool CCharDB_mem::insertChar(CCharAccount &account, const char *name, unsigned char str, unsigned char agi, unsigned char vit, unsigned char int_, unsigned char dex, unsigned char luk, unsigned char slot, unsigned char hair_style, unsigned char hair_color, CCharCharacter&data)
{
	size_t pos;
	if( cAccountList.find(account,0,pos) &&
		make_new_char(cAccountList[pos], name, str, agi, vit, int_, dex, luk, slot, hair_style, hair_color,data) )
	{
		this->do_createChar(data);
		account = cAccountList[pos];
		return searchChar(name, data);
	}
	return false;
}
bool CCharDB_mem::removeChar(uint32 charid)
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
		this->do_removeChar(cCharList[posc]);
		return cCharList.removeindex(posc, 0);
	}
	return false;
}
bool CCharDB_mem::saveChar(const CCharCharacter& data)
{
	size_t pos;
	if( cCharList.find( data, pos, 0) )
	{
		cCharList[pos] = data;
		this->do_saveChar(data);
		return true;
	}
	return false;
}
bool CCharDB_mem::searchAccount(uint32 accid, CCharCharAccount& account)
{
	size_t pos;
	if( cAccountList.find(CCharCharAccount(accid),0,pos) )
	{
		account = cAccountList[pos];
		return true;
	}
	return false;
}
bool CCharDB_mem::saveAccount(CCharAccount& account)
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
bool CCharDB_mem::removeAccount(uint32 accid)
{
	size_t pos;
	if( cAccountList.find(CCharAccount(accid),0,pos) )
	{	// exist -> update list entry
		cAccountList.removeindex(pos);
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////
// alternative interface
bool CCharDB_mem::aquire()
{
	cMx.lock();
	return this->first();
}
bool CCharDB_mem::release()
{
	cPos=0;
	cMx.unlock();
	return true;
}
bool CCharDB_mem::first()
{
	basics::ScopeLock sl(cMx);
	cPos=0;
	return this->operator bool();
}
CCharDB_mem::operator bool()					
{
	basics::ScopeLock sl(cMx);
	return cCharList.size() > cPos;
}
bool CCharDB_mem::operator++(int)
{
	basics::ScopeLock sl(cMx);
	cPos++;
	return this->operator bool();
}
bool CCharDB_mem::save()
{
	if( cPos < cCharList.size() )
	{
		this->do_removeChar(cCharList[cPos]);
		return true;
	}
	return false;
}
bool CCharDB_mem::find(const char* name)
{
	basics::ScopeLock sl(cMx);
	size_t pos;
	// search in index 1
	if( cCharList.find( CCharCharacter(name), pos, 1) )
	{	// set position based to index 0 
		return cCharList.find( cCharList(pos,1), cPos, 0);
	}
	return false; 

}
bool CCharDB_mem::find(uint32 charid)
{
	basics::ScopeLock sl(cMx);
	return cCharList.find( CCharCharacter(charid), cPos, 0);
}
CCharCharacter& CCharDB_mem::operator()()
{
	return cCharList[cPos]; 
}

void CCharDB_mem::loadfamelist()
{
	basics::ScopeLock sl(cMx);
	size_t pos = 0, k, i;
	fame_t fametype;
	static const char* regnames[4] = 
	{"PC_PK_FAME","PC_SMITH_FAME","PC_CHEM_FAME","PC_TEAK_FAME"};

	CFameList *fl = this->famelists;
	fl[0].clear();
	fl[1].clear();
	fl[2].clear();
	fl[3].clear();

	while( pos<cCharList.size() )
	{
		CCharCharacter& cc = cCharList[pos];
		++pos;

		for(i=0; i<4; ++i)
		{
			// grab through variables
			for(k=0; k<cc.global_reg_num; k++)
			{
				if( 0==strcmp(cc.global_reg[k].str, regnames[i]) )
				{	// found variable
					if( cc.global_reg[k].value < 0 )
						cc.global_reg[k].value = 0;

					if( cc.global_reg[k].value > 0 )
					{
						const uint32 points = cc.global_reg[k].value;

						if(i==0)
						{
							fametype = FAME_PK;
						}
						else if( i==1 && (
							cc.class_ == 10 ||
							cc.class_ == 4011 ||
							cc.class_ == 4033) )
						{	// blacksmiths
							fametype = FAME_SMITH;
						}
						else if( i==2 && (
								 cc.class_ == 18 ||
								 cc.class_ == 4019 ||
								 cc.class_ == 4041) )
						{
							fametype = FAME_CHEM;
						}
						else if( i==3 && (
								 cc.class_ == 4046) )
						{
							fametype = FAME_TEAK;
						}
						else
							break;  // inner for loop

						// look if fame has to be inserted
						for(k=0; k<fl[fametype].cCount; ++k)
						{
							if(cc.fame_points > fl[fametype].cEntry[k].fame_points)
								break;
						}
						if(k<MAX_FAMELIST+1)
						{	// insert at position k
							if(k<fl[fametype].cCount)
								memmove(fl[fametype].cEntry+k, fl[fametype].cEntry+k+1, (fl[fametype].cCount-1-k)*sizeof(fl[fametype].cEntry[0]));
							fl[fametype].cEntry[k] = CFameList::fameentry(cc.char_id, cc.name, points);
							++fl[fametype].cCount;
						}
					}
					break; // inner for loop
				}
			}
		}
	}
}











///////////////////////////////////////////////////////////////////////////
// construct/destruct
CGuildDB_mem::CGuildDB_mem(const char *configfile) :
	cPosGuild(0), cPosCastle(0)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
	// always load guildexp from file
	basics::CParam< basics::string<> > guildexp_filename("guildexp_filename", "db/exp_guild.txt");
	CGuild::cGuildExp.init(guildexp_filename());
}
CGuildDB_mem::~CGuildDB_mem()
{
}


///////////////////////////////////////////////////////////////////////////
// access interface
size_t CGuildDB_mem::size() const
{
	return cGuilds.size();
}
CGuild& CGuildDB_mem::operator[](size_t i)
{
	return cGuilds[i];
}

size_t CGuildDB_mem::castlesize() const
{
	return cCastles.size();
}
CCastle& CGuildDB_mem::castle(size_t i)
{
	return cCastles[i];
}

bool CGuildDB_mem::searchGuild(const char* name, CGuild& guild)
{
	size_t pos;
	if( cGuilds.find( CGuild(name), pos, 1) )
	{
		guild = cGuilds[pos];
		return true;
	}
	return false;
}
bool CGuildDB_mem::searchGuild(uint32 guildid, CGuild& guild)
{
	size_t pos;
	if( cGuilds.find( CGuild(guildid), pos, 0) )
	{
		guild = cGuilds[pos];
		return true;
	}
	return false;
}
bool CGuildDB_mem::insertGuild(const struct guild_member &member, const char *name, CGuild &guild)
{
	CGuild tmp(name);
	size_t pos, i;
	if( !cGuilds.find( tmp, pos, 1) )
	{
		///////////////////////////////////////////////////////////////////////
		basics::CParam<uint32> start_guild_num("start_guild_num", 30000000);
		uint32 guild_id=start_guild_num;
		const size_t p=this->cGuilds.size();
		if( 0==p || cGuilds[0].guild_id > guild_id)
		{	// take the start value
		}
		else if( cGuilds[0].guild_id+p-1 == cGuilds[p-1].guild_id )
		{	// increment last id when range is fully used
			guild_id = cGuilds[p-1].guild_id++;
		}
		else
		{	// find some unused key within
			size_t a=0, b=p, c;
			while( b > a+1 )
			{
				c=(a+b)/2;
				if( cGuilds[c].guild_id > guild_id && 
					(cGuilds[c].guild_id-cGuilds[a].guild_id) > (c-a) )
					b=c;
				else
					a=c;
			}
			guild_id = cGuilds[a].guild_id + 1;
			if( guild_id < start_guild_num() )
				guild_id = start_guild_num;
		}
		///////////////////////////////////////////////////////////////////////

		tmp.guild_id = guild_id;
		tmp.member[0] = member;
		safestrcpy(tmp.master, sizeof(tmp.master), member.name);
		tmp.position[0].mode=0x11;
		safestrcpy(tmp.position[0].name, sizeof(tmp.position[0].name),"GuildMaster");
		safestrcpy(tmp.position[MAX_GUILDPOSITION-1].name, sizeof(tmp.position[0].name),"Newbie");
		for(i=1; i<MAX_GUILDPOSITION-1; ++i)
			snprintf(tmp.position[i].name,sizeof(tmp.position[0].name),"Position %ld",(unsigned long)(i+1));

		tmp.max_member=(16>MAX_GUILD)?MAX_GUILD:16;
		tmp.average_lv=tmp.member[0].lv;
		for(i=0; i<MAX_GUILDSKILL; ++i)
		{
			tmp.skill[i].id = i+GD_SKILLBASE;
			tmp.skill[i].lv = 0;
		}
		guild = tmp;
		this->do_createGuild(tmp);
		return cGuilds.insert(tmp);
	}
	return false;
}
bool CGuildDB_mem::removeGuild(uint32 guildid)
{
	size_t pos,i,k;
	if( cGuilds.find( CGuild(guildid), pos, 0) )
	{
		// clear alliances
		for(i=0; i<cGuilds.size(); ++i)
		for(k=0; k<MAX_GUILDALLIANCE; ++k)
		{
			if( cGuilds[i].alliance[k].guild_id == guildid )
			{
				cGuilds[i].alliance[k].guild_id = 0;
				this->saveGuild( cGuilds[i] );
			}
		}
		// clear castles
		for(i=0; i<cCastles.size(); ++i)
		{
			if( cCastles[i].guild_id == guildid )
			{
				cCastles[i].guild_id = 0;
				this->saveCastle( cCastles[i] );
			}
		}
		this->do_removeGuild( cGuilds[pos] );
		return cGuilds.removeindex(pos,0);
	}
	return false;
}
bool CGuildDB_mem::saveGuild(const CGuild& guild)
{
	size_t pos;
	if( cGuilds.find( guild, pos, 0) )
	{
		cGuilds[pos] = guild;
		this->do_saveGuild( guild );
		
		return true;
	}
	return false;
}

bool CGuildDB_mem::searchCastle(ushort cid, CCastle& castle)
{
	size_t pos;
	if( cCastles.find( CCastle(cid), 0, pos) )
	{
		castle = cCastles[pos];
		return true;
	}
	return false;
}
bool CGuildDB_mem::saveCastle(const CCastle& castle)
{
	size_t pos;
	if( cCastles.find( castle, 0, pos) )
	{
		cCastles[pos] = castle;
		do_saveCastle(castle);
		return true;
	}
	return false;
}





///////////////////////////////////////////////////////////////////////////
// alternative interface
bool CGuildDB_mem::aquireGuild()
{
	cMxGuild.lock();
	return this->firstGuild(); 
}
bool CGuildDB_mem::aquireCastle()
{
	cMxCastle.lock();
	return this->firstCastle(); 
}
bool CGuildDB_mem::releaseGuild()
{
	cMxGuild.unlock();
	return true; 
}
bool CGuildDB_mem::releaseCastle()
{
	cMxCastle.unlock();
	return true; 
}
bool CGuildDB_mem::firstGuild()
{
	cPosGuild=0;
	return this->isGuildOk();
}
bool CGuildDB_mem::firstCastle()
{
	cPosCastle=0;
	return this->isCastleOk();
}
bool CGuildDB_mem::isGuildOk()
{
	return (cGuilds.size() < cPosGuild); 
}
bool CGuildDB_mem::isCastleOk()
{
	return (cCastles.size() < cPosCastle); 
}
bool CGuildDB_mem::nextGuild()
{
	cPosGuild++;
	return this->isGuildOk();
}
bool CGuildDB_mem::nextCastle()
{
	cPosGuild++;
	return this->isGuildOk();
}
bool CGuildDB_mem::saveGuild()
{
	if(cGuilds.size() < cPosGuild)
	{
		this->do_saveGuild(cGuilds[cPosGuild]);
		return true;
	}
	return false;}
bool CGuildDB_mem::saveCastle()
{
	if(cCastles.size() < cPosCastle)
	{
		this->do_saveCastle(cCastles[cPosCastle]);
		return true;
	}
	return false;}

bool CGuildDB_mem::findGuild(const char* name)
{
	size_t pos;
	if( cGuilds.find( CGuild(name), pos, 1) )
	{	// need index base 0
		return cGuilds.find( cGuilds(pos,1), cPosGuild, 0);
	}
	return false;
}
bool CGuildDB_mem::findGuild(uint32 guildid)
{
	return cGuilds.find( CGuild(guildid), cPosCastle, 0);
}
bool CGuildDB_mem::findCastle(ushort cid)
{
	return cCastles.find( CCastle(cid), 0, cPosCastle);
}
CGuild& CGuildDB_mem::getGuild()
{
	return cGuilds[cPosGuild];
}
CCastle& CGuildDB_mem::getCastle()
{
	return cCastles[cPosCastle];
}







///////////////////////////////////////////////////////////////////////////////
// Party Database
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// construct/destruct
CPartyDB_mem::CPartyDB_mem(const char *configfile)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CPartyDB_mem::~CPartyDB_mem()
{
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CPartyDB_mem::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readParties();
}

///////////////////////////////////////////////////////////////////////////
// access interface
size_t CPartyDB_mem::size() const
{
	return cParties.size();
}
CParty& CPartyDB_mem::operator[](size_t i)
{
	return cParties[i];
}

bool CPartyDB_mem::searchParty(const char* name, CParty& party)
{
	size_t pos;
	if( cParties.find( CParty(name), pos, 1) )
	{
		party = cParties[pos];
		return true;
	}
	return false;
}
bool CPartyDB_mem::searchParty(uint32 pid, CParty& party)
{
	size_t pos;
	if( cParties.find( CParty(pid), pos, 0) )
	{
		party = cParties[pos];
		return true;
	}
	return false;
}
bool CPartyDB_mem::insertParty(uint32 accid, const char *nick, const char *mapname, ushort lv, const char *name, CParty &party)
{
	size_t pos;
	CParty temp(name);
	if( !cParties.find( temp, pos, 0) )
	{
		///////////////////////////////////////////////////////////////////////
		basics::CParam<uint32> start_party_num("start_party_num", 40000000);
		uint32 party_id=start_party_num;
		const size_t p=this->cParties.size();
		if( 0==p || cParties[0].party_id > party_id)
		{	// take the start value
		}
		else if( cParties[0].party_id+p-1 == cParties[p-1].party_id )
		{	// increment last id when range is fully used
			party_id = cParties[p-1].party_id++;
		}
		else
		{	// find some unused key within
			size_t a=0, b=p, c;
			while( b > a+1 )
			{
				c=(a+b)/2;
				if( cParties[c].party_id > party_id && 
					(cParties[c].party_id-cParties[a].party_id) > (c-a) )
					b=c;
				else
					a=c;
			}
			party_id = cParties[a].party_id + 1;
			if( party_id < start_party_num() )
				party_id = start_party_num;
		}
		///////////////////////////////////////////////////////////////////////

		temp.party_id = party_id;
		temp.expshare = 0;
		temp.itemshare = 0;
		temp.member[0].account_id = accid;
		safestrcpy(temp.member[0].name, sizeof(temp.member[0].name), nick);
		safestrcpy(temp.member[0].mapname, sizeof(temp.member[0].mapname), mapname);
		char*ip = strchr(temp.member[0].mapname,'.');
		if(ip) *ip=0;

		temp.member[0].leader = 1;
		temp.member[0].online = 1;
		temp.member[0].lv = lv;

		cParties.insert(temp);
		party = temp;
		do_createParty(temp);
		return true;
	}
	return false;
}
bool CPartyDB_mem::removeParty(uint32 pid)
{
	size_t pos;
	if( cParties.find( CParty(pid), pos, 0) )
	{
		do_removeParty(cParties(pos,0));
		cParties.removeindex(pos,0);
		return true;
	}
	return false;
}
bool CPartyDB_mem::saveParty(const CParty& party)
{
	size_t pos;
	if( cParties.find( party, pos, 0) )
	{
		cParties[pos] = party;
		do_saveParty(party);
		return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////
// construct/destruct
CPCStorageDB_mem::CPCStorageDB_mem(const char *configfile)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CPCStorageDB_mem::~CPCStorageDB_mem()
{
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CPCStorageDB_mem::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return do_readPCStorage();
}


///////////////////////////////////////////////////////////////////////////
// access interface
size_t CPCStorageDB_mem::size() const
{
	return cPCStorList.size();
}
CPCStorage& CPCStorageDB_mem::operator[](size_t i)
{
	return cPCStorList[i];
}

bool CPCStorageDB_mem::searchStorage(uint32 accid, CPCStorage& stor)
{
	size_t pos;
	if( cPCStorList.find( CPCStorage(accid), 0, pos) )
	{
		stor = cPCStorList[pos];
		return true;
	}
	return false;
}
bool CPCStorageDB_mem::removeStorage(uint32 accid)
{
	size_t pos;
	if( cPCStorList.find( CPCStorage(accid), 0, pos) )
	{
		this->do_removePCStorage(cPCStorList[pos]);
		cPCStorList.removeindex(pos);
		return true;
	}
	return false;
}
bool CPCStorageDB_mem::saveStorage(const CPCStorage& stor)
{
	size_t pos;
	if( cPCStorList.find( stor, 0, pos) )
	{
		this->do_savePCStorage(stor);
		cPCStorList[pos] = stor;
		return true;
	}
	else
	{
		this->do_cratePCStorage(stor);
		return cPCStorList.insert(stor);
	}
}






///////////////////////////////////////////////////////////////////////////
// construct/destruct
CGuildStorageDB_mem::CGuildStorageDB_mem(const char *configfile)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CGuildStorageDB_mem::~CGuildStorageDB_mem()
{
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CGuildStorageDB_mem::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return do_readGuildStorage();
}


///////////////////////////////////////////////////////////////////////////
// access interface
size_t CGuildStorageDB_mem::size() const
{
	return cGuildStorList.size();
}
CGuildStorage& CGuildStorageDB_mem::operator[](size_t i)
{
	return cGuildStorList[i];
}

bool CGuildStorageDB_mem::searchStorage(uint32 gid, CGuildStorage& stor)
{
	size_t pos;
	if( cGuildStorList.find( CGuildStorage(gid), 0, pos) )
	{
		stor = cGuildStorList[pos];
		return true;
	}
	return false;
}
bool CGuildStorageDB_mem::removeStorage(uint32 gid)
{
	size_t pos;
	if( cGuildStorList.find( CGuildStorage(gid), 0, pos) )
	{
		this->do_removeGuildStorage(cGuildStorList[pos]);
		cGuildStorList.removeindex(pos);
		return true;
	}
	return false;
}
bool CGuildStorageDB_mem::saveStorage(const CGuildStorage& stor)
{
	size_t pos;
	if( cGuildStorList.find( stor, 0, pos) )
	{
		this->do_saveGuildStorage(stor);
		cGuildStorList[pos] = stor;
		return true;
	}
	else
	{
		this->do_crateGuildStorage(stor);
		return cGuildStorList.insert(stor);
	}
}











///////////////////////////////////////////////////////////////////////////
// construct/destruct
CPetDB_mem::CPetDB_mem(const char *configfile)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CPetDB_mem::~CPetDB_mem()
{
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CPetDB_mem::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readPets();
}

///////////////////////////////////////////////////////////////////////////
// access interface
size_t CPetDB_mem::size() const
{
	return this->cPetList.size();
}
CPet& CPetDB_mem::operator[](size_t i)
{
	return this->cPetList[i];
}

bool CPetDB_mem::searchPet(uint32 pid, CPet& pet)
{
	size_t pos;
	if( this->cPetList.find( CPet(pid), 0, pos) )
	{
		pet = this->cPetList[pos];
		return true;
	}
	return false;
}
bool CPetDB_mem::insertPet(uint32 accid, uint32 cid, short pet_class, short pet_lv, short pet_egg_id, ushort pet_equip, short intimate, short hungry, char renameflag, char incuvat, char *pet_name, CPet& pet)
{
	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_pet_num("start_pet_num", 50000000);
	size_t pet_id=start_pet_num;
	const size_t p=this->cPetList.size();
	if( 0==p || this->cPetList[0].pet_id > pet_id)
	{	// take the start value
	}
	else if( this->cPetList[0].pet_id+p-1 == this->cPetList[p-1].pet_id )
	{	// increment last id when range is fully used
		pet_id = this->cPetList[p-1].pet_id++;
	}
	else
	{	// find some unused key within
		size_t a=0, b=p, c;
		while( b > a+1 )
		{
			c=(a+b)/2;
			if( this->cPetList[c].pet_id > pet_id && 
				(this->cPetList[c].pet_id-this->cPetList[a].pet_id) > (c-a) )
				b=c;
			else
				a=c;
		}
		pet_id = this->cPetList[a].pet_id + 1;
		if( pet_id < start_pet_num() )
			pet_id = start_pet_num;
	}
	///////////////////////////////////////////////////////////////////////////
	
	CPet tmppet(pet_id, accid, cid, pet_class, pet_lv, pet_egg_id, pet_equip, intimate, hungry, renameflag, incuvat, pet_name);
	if( this->cPetList.insert(tmppet) && searchPet(pet_id, tmppet) )
	{
		this->do_createPet(tmppet);
		return true;
	}
	return false;
}
bool CPetDB_mem::removePet(uint32 pid)
{
	size_t pos;
	if( this->cPetList.find( CPet(pid), 0, pos) )
	{
		this->do_removePet(cPetList[pos]);
		this->cPetList.removeindex(pos);
		return true;
	}
	return false;
}
bool CPetDB_mem::savePet(const CPet& pet)
{
	size_t pos;
	if( this->cPetList.find( pet, 0, pos) )
	{
		this->cPetList[pos] = pet;
		this->do_savePet(pet);
		return true;
	}
	return false;
}





///////////////////////////////////////////////////////////////////////////
// construct/destruct
CHomunculusDB_mem::CHomunculusDB_mem(const char *configfile)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CHomunculusDB_mem::~CHomunculusDB_mem()
{
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CHomunculusDB_mem::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readHomunculus();
}
///////////////////////////////////////////////////////////////////////////
// access interface

size_t CHomunculusDB_mem::size() const
{
	return this->cHomunculusList.size();
}
CHomunculus& CHomunculusDB_mem::operator[](size_t i)
{
	return this->cHomunculusList[i];
}

bool CHomunculusDB_mem::searchHomunculus(uint32 hid, CHomunculus& hom)
{
	size_t pos;
	if( this->cHomunculusList.find( CHomunculus(hid), 0, pos) )
	{
		hom = this->cHomunculusList[pos];
		return true;
	}
	return false;
}

bool CHomunculusDB_mem::insertHomunculus(CHomunculus& hom)
{
	///////////////////////////////////////////////////////////////////////////
	basics::CParam<uint32> start_homun_num("start_homun_num", 60000000);
	size_t homun_id=start_homun_num;
	const size_t p=this->cHomunculusList.size();
	if( 0==p || this->cHomunculusList[0].homun_id > homun_id)
	{	// take the start value
	}
	else if( cHomunculusList[0].homun_id+p-1 == cHomunculusList[p-1].homun_id )
	{	// increment last id when range is fully used
		homun_id = this->cHomunculusList[p-1].homun_id++;
	}
	else
	{	// find some unused key within
		size_t a=0, b=p, c;
		while( b > a+1 )
		{
			c=(a+b)/2;
			if( this->cHomunculusList[c].homun_id > homun_id && 
				(this->cHomunculusList[c].homun_id-this->cHomunculusList[a].homun_id) > (c-a) )
				b=c;
			else
				a=c;
		}
		homun_id = this->cHomunculusList[a].homun_id + 1;
		if( homun_id < start_homun_num() )
			homun_id = start_homun_num;
	}
	///////////////////////////////////////////////////////////////////////////
	
	hom.homun_id = homun_id;
	if( this->cHomunculusList.insert(hom) && searchHomunculus(homun_id, hom) )
	{
		this->do_createHomunculus(hom);
		return true;
	}
	return false;
}
bool CHomunculusDB_mem::removeHomunculus(uint32 hid)
{
	size_t pos;
	if( this->cHomunculusList.find( CHomunculus(hid), 0, pos) )
	{
		this->do_removeHomunculus(cHomunculusList[pos]);
		this->cHomunculusList.removeindex(pos);
		return true;
	}
	return false;
}
bool CHomunculusDB_mem::saveHomunculus(const CHomunculus& hom)
{
	size_t pos;
	if( this->cHomunculusList.find( hom, 0, pos) )
	{
		this->cHomunculusList[pos] = hom;
		this->do_saveHomunculus(hom);
		return true;
	}
	return false;
}




///////////////////////////////////////////////////////////////////////////
// construct/destruct
CVarDB_mem::CVarDB_mem(const char *configfile)
{
	if(configfile) basics::CParamBase::loadFile(configfile);
}
CVarDB_mem::~CVarDB_mem()
{
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CVarDB_mem::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readVars();
}
///////////////////////////////////////////////////////////////////////////
// access interface

size_t CVarDB_mem::size() const
{
	return this->cVarList.size();
}
CVar& CVarDB_mem::operator[](size_t i)
{
	return this->cVarList[i];
}

bool CVarDB_mem::searchVar(const char* name, CVar& var)
{
	size_t pos;
	if( this->cVarList.find( CVar(name), 0, pos) )
	{
		var = this->cVarList[pos];
		return true;
	}
	return false;
}

bool CVarDB_mem::insertVar(const char* name, const char* value)
{
	CVar var(name, value);
	if( this->cVarList.insert(var) && searchVar(name, var) )
	{
		this->do_createVar(var);
		return true;
	}
	return false;
}

bool CVarDB_mem::removeVar(const char* name)
{
	size_t pos;
	if( this->cVarList.find( CVar(name), 0, pos) )
	{
		this->do_removeVar(cVarList[pos]);
		this->cVarList.removeindex(pos);
		return true;
	}
	return false;
}
bool CVarDB_mem::saveVar(const CVar& var)
{
	size_t pos;

	if( this->cVarList.find( var, 0, pos) )
	{
		if( this->cVarList[pos].value() != var.value() )
		{
			this->cVarList[pos] = var;
			this->do_saveVar(var);
			return true;
		}
		return false;
	}
	else
	{
		return this->cVarList.insert(var);
	}
}





///////////////////////////////////////////////////////////////////////////////
// an implementation of the virtual database read/write functions using fileio
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
#if defined(WITH_TEXT)
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
/// read accounts.
/// reads all accounts from file or sql into memory
bool CAccountDB_txt::do_readAccounts()
{
	int GM_count = 0;
	int server_count = 0;
	const char* filename;
	basics::CParam< basics::string<> > account_filename("account_filename", "save/account.txt");
	FILE *fp;
	filename = account_filename();
	if ((fp = basics::safefopen(filename, "r")) == NULL)
	{
		// no account file -> no account -> no login, including char-server (ERROR)
		ShowError(CL_BT_RED"Accounts file [%s] not found.\n"CL_RESET, filename);
		return false;
	}
	else
	{
		size_t pos;
		CLoginAccount data;
		unsigned long account_id;
		int logincount, state, n, i, j, v, gm;
		char line[2048], *p, userid[2048], pass[2048], lastlogin[2048], sex, email[2048], error_message[2048], last_ip[2048], memo[2048];
		unsigned long ban_until_time;
		unsigned long connect_until_time;

		///////////////////////////////////
		basics::slist<CMapGM> gmlist;
		this->readGMAccount(gmlist);

		while( fgets(line, sizeof(line), fp) )
		{
			if( !is_valid_line(line) )
				continue;
			*userid=0;
			*pass=0;
			*lastlogin=0;
			gm=0;
			

			if( 11 == (i=sscanf(line, 
							"%ld\t"
							"%[^\t]\t%[^\t]\t"
							"%d\t%c\t%[^\t]\t"
							"%d\t%[^\t]\t%[^\t]\t"
							"%ld\t%ld%n",
							&account_id, 
							userid, pass, 
							&gm, &sex, email, 
							&logincount, lastlogin, last_ip, 
							&connect_until_time, &ban_until_time, &n)) && (line[n] == '\t') )
			{	// added gm_level, removed useless stuff and reordered the rest
				;
			}
			else if( 13 == (i=sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]\t%ld%n",
							&account_id, userid, pass, lastlogin, &sex, &logincount, &state, email, error_message, &connect_until_time, last_ip, memo, &ban_until_time, &n)) && (line[n] == '\t') )
			{	// added ban_time
				gm=0;
			}
			else if( 12 == (i=sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%[^\t]\t%[^\t]\t%ld\t%[^\t]\t%[^\t]%n",
							&account_id, userid, pass, lastlogin, &sex, &logincount, &state, email, error_message, &connect_until_time, last_ip, memo, &n)) && (line[n] == '\t') )
			{	// without ban_time
				ban_until_time=0;
				gm=0;
			}
			// Old athena database version reading (v1)
			else if( 5 <= (i=sscanf(line, "%ld\t%[^\t]\t%[^\t]\t%[^\t]\t%c\t%d\t%d\t%n",
							&account_id, userid, pass, lastlogin, &sex, &logincount, &state, &n)) )
			{
				gm=0;
				*email=0;
				*last_ip=0;
				ban_until_time=0;
				connect_until_time=0;
				if (i < 6)
					logincount=0;
			}
			//else if(sscanf(line, "%ld\t%%newid%%\n%n", &account_id, &i) == 1 && i > 0 && account_id > next_account_id)
			else if(sscanf(line, "%ld\t%%newid%%\n%n", &account_id, &i) == 1)
			{
				// ignore
				//next_account_id = account_id;
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

			if( this->cList.find( CLoginAccount(account_id), pos, 0) )
			{
				ShowWarning(CL_BT_RED"An account has an identical id to another.\n"CL_RESET);
				ShowMessage("           Account id #%d -> not read (saved in log file).\nCL_RESET", account_id);

				//!! log account line to file as promised
			}
			else if( this->cList.find( CLoginAccount(userid), pos, 1) )
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
				safestrcpy(temp.userid, sizeof(temp.userid), userid);

				pass[23] = '\0';
				remove_control_chars(pass);
				safestrcpy(temp.passwd, sizeof(temp.passwd), pass);
				temp.sex = (basics::upcase(sex) == 'S') ? 2 : (basics::upcase(sex) == 'M');
				if (temp.sex == 2)
					server_count++;

				remove_control_chars(email);
				if( !email_check(email) )
				{
					ShowWarning("Account %s (%d): invalid e-mail (replaced with a@a.com).\n", userid, account_id);
					safestrcpy(temp.email, sizeof(temp.email), "a@a.com");
				}
				else
					safestrcpy(temp.email, sizeof(temp.email), email);

				if( gmlist.find( CMapGM(account_id),0,pos) )
				{	// gm list value overwrites stored values from db
					temp.gm_level = gmlist[pos].gm_level;
					GM_count++;
				}
				else
				{	// otherwise take the value from db
					temp.gm_level=gm;
					if(gm) GM_count++;
				}

				temp.login_count = (logincount>0)?logincount:0;

				lastlogin[23] = '\0';
				if(lastlogin[0]=='-' && lastlogin[1]==0)
					lastlogin[0] = 0;// remove defaults
				else
					remove_control_chars(lastlogin);
				safestrcpy(temp.last_login, sizeof(temp.last_login), lastlogin);

				last_ip[15] = '\0';
				remove_control_chars(last_ip);

				if(*last_ip && *last_ip!='-')
				{
					basics::ipaddress ip(last_ip);
					temp.client_ip = ip;
					ip.tostring(temp.last_ip, sizeof(temp.last_ip));
				}
				else
				{
					temp.client_ip  = (uint32)INADDR_ANY;
					temp.last_ip[0] = 0;
				}

				temp.ban_until = ban_until_time;
				temp.valid_until = connect_until_time;

				p = line;
				for(j = 0; j < ACCOUNT_REG2_NUM; ++j)
				{
					p += n;
					if(sscanf(p, "%[^\t,],%d %n", memo, &v, &n) != 2)
					{	// We must check if memo is void. If it's, we can continue to read other REG2.
						// Account line will have something like: str2,9 ,9 str3,1 (here, ,9 is not good)
						if (p[0] == ',' && sscanf(p, ",%d %n", &v, &n) == 1)
						{
							j--;
							continue;
						} else
							break;
					}
					memo[31] = '\0';
					remove_control_chars(memo);
					safestrcpy(temp.account_reg2[j].str, sizeof(temp.account_reg2[0].str), memo);
					temp.account_reg2[j].value = v;
				}
				temp.account_reg2_num = j;

				// ignore
				//if (next_account_id <= account_id)
				//	next_account_id = account_id + 1;

				this->cList.insert(temp);

			}
		}
		fclose(fp);
	}

	char str[2048], *ptr = str;
	if( cList.size() == 0 )
	{
		ShowError("No account found in %s.\n", filename);
		ptr+=snprintf(ptr, sizeof(str)-(ptr-str), "No account found in %s.", filename);
	}
	else
	{
		if( this->cList.size() == 1)
		{
			ShowStatus("1 account read in %s,\n", filename);
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), "1 account read in %s,", filename);
		}
		else
		{
			ShowStatus("%d accounts read in %s,\n", this->cList.size(), filename);
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), "%ld accounts read in %s,", (unsigned long)cList.size(), filename);
		}
		if (GM_count == 0)
		{
			ShowMessage("           of which is no GM account, and ");
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), " of which is no GM account and");
		}
		else if (GM_count == 1)
		{
			ShowMessage("           of which is 1 GM account, and ");
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), " of which is 1 GM account and");
		}
		else
		{
			ShowMessage("           of which is %d GM accounts, and ", GM_count);
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), " of which is %d GM accounts and", GM_count);
		}
		if (server_count == 0)
		{
			ShowMessage("no server account ('S').\n");
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), " no server account.");
		}
		else if (server_count == 1)
		{
			ShowMessage("1 server account ('S').\n");
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), " 1 server account.");
		}
		else
		{
			ShowMessage("%d server accounts ('S').\n", server_count);
			ptr+=snprintf(ptr, sizeof(str)-(ptr-str), " %d server accounts.", server_count);
		}
	}
//	login_log("%s" RETCODE, line);
	return true;
}

///////////////////////////////////////////////////////////////////////////
// write all accounts
bool CAccountDB_txt::do_saveAccounts()
{
	basics::CParam< basics::string<> > account_filename("account_filename", "save/account.txt");
	FILE *fp;
	size_t i, k;
	int lock;

	// Data save
	if ((fp = lock_fopen(account_filename(), lock)) == NULL) {
		return false;
	}
	fprintf(fp, "// Accounts file: here are saved all information about the accounts.\n");
	fprintf(fp, "// Structure: account_id, name, password, gm_level, sex, email, # of logins, last login time, last login ip, validity time, ban timestamp, repeated(variable text, variable value)\n");
	fprintf(fp, "// Some explanations:\n");
	fprintf(fp, "//   account name    : between 4 to 23 char for a normal account (standard client can't send less than 4 char).\n");
	fprintf(fp, "//   account password: between 4 to 23 char\n");
	fprintf(fp, "//   sex             : M or F for normal accounts, S for server accounts\n");
	fprintf(fp, "//   email           : between 3 to 39 char (a@a.com is like no email)\n");
	fprintf(fp, "//   valitidy time   : 0: unlimited account, <other value>: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");
	fprintf(fp, "//   ban time        : 0: no ban, <other value>: banned until the date: date calculated by addition of 1/1/1970 + value (number of seconds since the 1/1/1970)\n");

	for(i = 0; i < this->cList.size(); ++i)
	{
		fprintf(fp, "%ld\t"
					"%s\t%s\t"
					"%d\t"
					"%c\t"
					"%s\t"
					"%ld\t"
					"%s\t"
					"%s\t"
					"%ld\t"
					"%ld\t",
					(unsigned long)this->cList[i].account_id,
					this->cList[i].userid, this->cList[i].passwd,
					(int)this->cList[i].gm_level, 
					(this->cList[i].sex == 2) ? 'S' : (this->cList[i].sex ? 'M' : 'F'),
					(*cList[i].email)?this->cList[i].email:"a@a.com",
					(unsigned long)this->cList[i].login_count,
					(*this->cList[i].last_login)?this->cList[i].last_login:"-",
					(this->cList[i].last_ip[0])?this->cList[i].last_ip:"-",
					(unsigned long)this->cList[i].valid_until,
					(unsigned long)this->cList[i].ban_until );

		for(k = 0; k< cList[i].account_reg2_num; ++k)
			if( this->cList[i].account_reg2[k].str[0] &&
				this->cList[i].account_reg2[k].value )
				fprintf(fp, "%s,%ld ", this->cList[i].account_reg2[k].str, (long)this->cList[i].account_reg2[k].value);
		fprintf(fp, RETCODE);
	}
	// ignore
	//fprintf(fp, "%ld\t%%newid%%"RETCODE, (unsigned long)next_account_id);
	lock_fclose(fp, account_filename(), lock);
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// construct/destruct
CAccountDB_txt::CAccountDB_txt(const char* configfile) :
	CTimerBase(10000),		// 60sec save interval
	CAccountDB_mem(configfile),
	savecount(0)
{
	this->init(NULL);
}
CAccountDB_txt::~CAccountDB_txt()
{
	this->close();
}

bool CAccountDB_txt::init(const char* configfile)
{
	return this->do_readAccounts();
}

bool CAccountDB_txt::close()
{
	return this->do_saveAccounts();
}

///////////////////////////////////////////////////////////////////////////////
// timer function
bool CAccountDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if( this->savecount > 10 )
	{
		this->savecount=0;
		this->do_saveAccounts();
	}

	// check changes in gm file
	basics::CParam< basics::string<> > GM_account_filename("GM_account_filename", "conf/GM_account.txt");

	struct stat file_stat;
	time_t new_time;

	// get last modify time/date
	if( 0!=stat(GM_account_filename(), &file_stat) )
		new_time = 0; // error
	else
		new_time = file_stat.st_mtime;

	if(new_time != creation_time_GM_account_file)
	{
		creation_time_GM_account_file = new_time;
		// re-read GM-File
		basics::slist<CMapGM> gmlist;
		readGMAccount(gmlist);

		// overwrite existing gmlevels with new ones
		basics::slist<CMapGM>::iterator iter(gmlist);
		size_t pos;
		while(iter)
		{
			if( this->cList.find( CLoginAccount(iter->account_id), pos, 0) )
			{
				cList[pos].gm_level = iter->gm_level;
				this->saveAccount( cList[pos] );
			}
			iter++;
		}
	}
	return true;
}








///////////////////////////////////////////////////////////////////////////
// Function to create the character line (for save)
int CCharDB_txt::char_to_str(char *str, size_t sz, const CCharCharacter &p)
{
	size_t i;
	char *str_p = str;

	point last_point = p.last_point;

	if (last_point.mapname[0] == '\0')
	{
		safestrcpy(last_point.mapname, sizeof(last_point.mapname), "prontera");
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
		"\t%ld,%ld,%ld,%ld"
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
		p.option, basics::MakeWord(p.karma, p.chaos), p.manner,
		(unsigned long)p.party_id, (unsigned long)p.guild_id,(unsigned long)p.pet_id,(unsigned long)p.homun_id,
		p.hair, p.hair_color, p.clothes_color,
		p.weapon, p.shield, p.head_top, p.head_mid, p.head_bottom,
		// store the checked lastpoint
		last_point.mapname, last_point.x, last_point.y,
		p.save_point.mapname, p.save_point.x, p.save_point.y,
		(unsigned long)p.partner_id,(unsigned long)p.father_id,(unsigned long)p.mother_id,(unsigned long)p.child_id,
		(unsigned long)p.fame_points);
	for(i = 0; i < MAX_MEMO; ++i)
		if (p.memo_point[i].mapname[0]) {
			str_p += snprintf(str_p, str+sz-str_p, "%s,%d,%d", p.memo_point[i].mapname, p.memo_point[i].x, p.memo_point[i].y);
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_INVENTORY; ++i)
		if (p.inventory[i].nameid) {
			str_p += snprintf(str_p, str+sz-str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
					 (int)i, p.inventory[i].nameid, p.inventory[i].amount, p.inventory[i].equip,
					 p.inventory[i].identify, p.inventory[i].refine, p.inventory[i].attribute,
					 p.inventory[i].card[0], p.inventory[i].card[1], p.inventory[i].card[2], p.inventory[i].card[3]);
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_CART; ++i)
		if (p.cart[i].nameid) {
			str_p += snprintf(str_p, str+sz-str_p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
					 (int)i, p.cart[i].nameid, p.cart[i].amount, p.cart[i].equip,
					 p.cart[i].identify, p.cart[i].refine, p.cart[i].attribute,
					 p.cart[i].card[0], p.cart[i].card[1], p.cart[i].card[2], p.cart[i].card[3]);
		}
	*(str_p++) = '\t';

	for(i = 0; i < MAX_SKILL; ++i)
		if (p.skill[i].id && p.skill[i].flag != 1) {
			str_p += snprintf(str_p, str+sz-str_p, "%d,%d ", p.skill[i].id, (p.skill[i].flag == 0) ? p.skill[i].lv : p.skill[i].flag-2);
		}
	*(str_p++) = '\t';

	for(i = 0; i < p.global_reg_num; ++i)
		if (p.global_reg[i].str[0])
			str_p += snprintf(str_p, str+sz-str_p, "%s,%ld ", p.global_reg[i].str, (long)p.global_reg[i].value);
	*(str_p++) = '\t';

	*str_p = '\0';
	return 0;
}

///////////////////////////////////////////////////////////////////////////
// Function to set the character from the line (at read of characters file)
bool CCharDB_txt::char_from_str(const char *str)
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
		"\t%d,%d,%d,%d"
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
		&tmp_int[21], &tmp_int[22], &tmp_int[23],
		&tmp_int[24], &tmp_int[25], &tmp_int[26], &tmp_int[44],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		p.last_point.mapname, &tmp_int[35], &tmp_int[36], //
		p.save_point.mapname, &tmp_int[37], &tmp_int[38], &tmp_int[39],
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next) == 48 )
	{
		// my personal reordering
		//ShowMessage("char: new char data ver.6\n");
	}
	else if( sscanf(str,
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
		tmp_int[44] = 0;
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
		tmp_int[44] = 0;
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
		tmp_int[44] = 0;
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
		tmp_int[44] = 0;
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
		tmp_int[44] = 0;
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
	p.karma = basics::GetByte(tmp_int[22],0);
	p.chaos = basics::GetByte(tmp_int[22],1);
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
	p.homun_id = tmp_int[44];

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
		for(i = 0; str[next] && str[next] != '\t'&&i<MAX_MEMO; ++i)
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
		for(i = 0; str[next] && str[next] != '\t'; ++i)
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
				//p.inventory[i].id = tmp_int[0];
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
		for(i = 0; str[next] && str[next] != '\t'; ++i)
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
				//p.cart[i].id = tmp_int[0];
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
		for(i = 0; str[next] && str[next] != '\t'; ++i) {
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
		char valstr[32];
		next++;
		for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; ++i)
		{	// global_reg実装以前のathena.txt互換のため一応'\n'チェック
			if(sscanf(str + next, "%32[^,],%ld%n", valstr, &val, &len) != 2)
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
				safestrcpy(p.global_reg[i].str, sizeof(p.global_reg[i].str), valstr);
				p.global_reg[i].value = val;
			}
			next += len;
			if (str[next] == ' ')
				next++;
		}
		p.global_reg_num = basics::min(i, (size_t)GLOBAL_REG_NUM);
	}

	// insert the character to the list
	this->cCharList.insert(p);

	return ret;
}




///////////////////////////////////////////////////////////////////////////
bool CCharDB_txt::save_friends()
{
	FILE *fp = basics::safefopen(friends_txt(), "wb");
	if(fp)
	{
		size_t i, k;
		for(i=0; i<this->cCharList.size(); ++i)
		{
			for (k=0; k<MAX_FRIENDLIST; ++k)
			{
				if(this->cCharList[i].friendlist[k].friend_id > 0 && this->cCharList[i].friendlist[k].friend_name[0])
					break;
			}
			if(k<MAX_FRIENDLIST)
			{	// at least one friend exist
				fprintf(fp, "%ld", (unsigned long)this->cCharList[i].char_id);
				for (k=0; k<MAX_FRIENDLIST; ++k)
				{
					if (this->cCharList[i].friendlist[k].friend_id > 0 && this->cCharList[i].friendlist[k].friend_name[0])
						fprintf(fp, ",%ld,%s", (unsigned long)this->cCharList[i].friendlist[k].friend_id,cCharList[i].friendlist[k].friend_name);
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
bool CCharDB_txt::read_friends()
{
	char line[1024];

	FILE *fp = basics::safefopen(friends_txt(), "rb");
	if(fp)
	{
		unsigned long cid=0;
		unsigned long fid[20];
		struct friends friendlist[20];
		size_t pos;

		while(fgets(line, sizeof(line), fp))
		{
			if( !is_valid_line(line) )
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

			if( this->cCharList.find( CCharCharacter(cid), pos, 0) )
			{
				CCharCharacter &temp = this->cCharList(pos,0);
				size_t i;
				for (i=0; i<MAX_FRIENDLIST; ++i)
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
// Function to read characters file
bool CCharDB_txt::do_readChars(void)
{
	char line[65536];
	int line_count;
	FILE *fp;


	fp = basics::safefopen(char_txt(), "r");
	if (fp == NULL)
	{
		ShowError("Characters file not found: %s.\n", (const char*)char_txt());
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

		if( !is_valid_line(line) )
			continue;
		line[sizeof(line)-1] = '\0';

		j = 0;
		if(sscanf(line, "%ld\t%%newid%%%n", &i, &j) == 1 && j > 0)
		{	//ignored
			//if(next_char_id < i)
			//	next_char_id = i;
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

	this->read_friends();

	if( cCharList.size() == 0 )
	{
		ShowError("No character found in %s.\n", (const char*)char_txt());
		//char_log("No character found in %s." RETCODE, (const char*)char_txt());
	}
	else if( cCharList.size() == 1 )
	{
		ShowStatus("1 character read in %s.\n", (const char*)char_txt());
		//char_log("1 character read in %s." RETCODE, (const char*)char_txt());
	}
	else
	{
		ShowStatus("mmo_char_init: %d characters read in %s.\n", cCharList.size(), (const char*)char_txt());
		//char_log("mmo_char_init: %d characters read in %s." RETCODE, cList.size(), (const char*)char_txt());
	}
	//char_log("Id for the next created character: %d." RETCODE, char_id_count);
	return true;
}

///////////////////////////////////////////////////////////////////////////
// Function to save characters in files
bool CCharDB_txt::do_saveChars(void)
{
	char line[65536];
	int lockfp;
	FILE *fp;

	// Data save
	fp = lock_fopen(char_txt(), lockfp);
	if (fp == NULL)
	{
		ShowWarning("Server cannot save characters.\n");
		//char_log("Server cannot save characters." RETCODE);
		return false;
	}
	else
	{
		size_t i;
		for(i=0; i<cCharList.size(); ++i)
		{
			char_to_str(line, sizeof(line), cCharList[i]);
			fprintf(fp, line); fprintf(fp, RETCODE);
		}
		//fprintf(fp, "%d\t%%newid%%" RETCODE, next_char_id);
		lock_fclose(fp, char_txt(), lockfp);

		return this->save_friends();
	}
}


CCharDB_txt::CCharDB_txt(const char *dbcfgfile) :
	CTimerBase(300*1000),
	CCharDB_mem(dbcfgfile),
	char_txt("char_txt","save/athena.txt"),
	friends_txt("friends_txt","save/friends.txt"),
	savecount(0)
{
	this->init(NULL);
}
CCharDB_txt::~CCharDB_txt()
{
	this->close();
}

///////////////////////////////////////////////////////////////////////////
// normal function
bool CCharDB_txt::init(const char* configfile)
{
	// read config
	if(configfile) basics::CParamBase::loadFile(configfile);
	// read data
	cMailDB.open("save/mail");
	return this->do_readChars();
}

bool CCharDB_txt::close()
{
	cMailDB.close();
	return this->do_saveChars();
}

///////////////////////////////////////////////////////////////////////////
// timer function
bool CCharDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if( savecount > 100 )
	{
		savecount=0;
		cMailDB.flush(true);
		this->do_saveChars();
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////
size_t CCharDB_txt::getMailCount(uint32 cid, uint32 &all, uint32 &unread)
{
	basics::ScopeLock sl(cMx);
	basics::simple_database::iterator iter(cMailDB);
	
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

size_t CCharDB_txt::listMail(uint32 cid, unsigned char box, unsigned char *buffer)
{
	basics::ScopeLock sl(cMx);
	basics::simple_database::iterator iter(cMailDB);
	char buf[1024]="";
	unsigned long mmid, tid, sid, stime=0, zeny=0;
	size_t count=0;
	char sname[32], tname[32], head[32], body[80];
	int read;
	int itemval[10];
	while(iter)
	{
		if( iter.Key2()==cid &&
			(!iter.Flag() || box) &&
			iter.read(buf, sizeof(buf)) &&
			//!! replace the whole thing with regex
			22==sscanf(buf,
				"%lu\t%d\t%lu\t%24[^\t]\t%lu\t%24[^\t]\t%lu\t%40[^\t]\t%512[^\t]\t"
				"%lu\t(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n",
				&mmid, &read, &sid, sname, &tid, tname, &stime, head, body,
				&zeny, itemval+0,itemval+1,itemval+2,itemval+3,itemval+4,itemval+5,
				itemval+6,itemval+7,itemval+8,itemval+9) 
			)
		{
			CMailHead mh(mmid, iter.Flag(), sname, stime, head);
			mh._tobuffer(buffer);// automatic buffer increment
			count++;
		}
		// next element
		++iter;
	}
	return count;
}

bool CCharDB_txt::readMail(uint32 cid, uint32 mid, CMail& mail)
{
	basics::ScopeLock sl(cMx);
	bool ret = false;
	char buf[2048], sname[32], tname[32];
	unsigned long mmid=0, tid=0, sid=0, stime=0, zeny=0;
	int read;
	int itemval[10];
	basics::simple_database::iterator iter(cMailDB, mid, cid);

	ret = ( iter.isValid() &&
			iter.read(buf, sizeof(buf)) &&
			//!! replace the whole thing with regex
			22==sscanf(buf,
				"%lu\t%d\t%lu\t%24[^\t]\t%lu\t%24[^\t]\t%lu\t%40[^\t]\t%512[^\t]\t"
				"%lu\t(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n",
				&mmid, &read, &sid, sname, &tid, tname, &stime, mail.head, mail.body,
				&zeny, itemval+0,itemval+1,itemval+2,itemval+3,itemval+4,itemval+5,
				itemval+6,itemval+7,itemval+8,itemval+9) &&
			mid==mmid && ( cid==tid || cid==sid) );


	if(ret)
	{	
		safestrcpy(mail.name, sizeof(mail.name), (cid==tid)?sname:tname);
		mail.msgid = mmid;
		mail.read = iter.Flag();
		mail.sendtime = stime;

		mail.zeny = zeny;
		mail.item.nameid	= itemval[0];
		mail.item.amount	= itemval[1];
		mail.item.equip 	= 0;			// mailed items are not equipped
		mail.item.identify	= itemval[3];
		mail.item.refine	= itemval[4];
		mail.item.attribute	= itemval[5];
		mail.item.card[0]	= itemval[6];
		mail.item.card[1]	= itemval[7];
		mail.item.card[2]	= itemval[8];
		mail.item.card[3]	= itemval[9];

		if( !mail.read && cid==tid )
		{	// update readflag of own unread mails and clear appends
			size_t sz = snprintf(buf, sizeof(buf),
				"%lu\t%c\t%lu\t%.24s\t%lu\t%.24s\t%lu\t%.40s\t%.512s\t0\t(0,0,0,0,0,0,0,0,0,0)\n",
				(unsigned long)mmid, '1', 
				(unsigned long)sid, sname,
				(unsigned long)tid, mail.name,
				(unsigned long)mail.sendtime,
				mail.head, mail.body);
			// will update entry and index
			cMailDB.insert(mmid, tid, buf, sz);
			// update readflag
			iter.Flag() = mail.read = 1;
		}
	}
	else
	{
		mail.msgid=mail.read = mail.name[0] = mail.head[0] = mail.body[0] = 0;
	}
	return ret;
}
bool CCharDB_txt::deleteMail(uint32 cid, uint32 mid)
{
	basics::ScopeLock sl(cMx);
	return cMailDB.remove(mid, cid);
}
bool CCharDB_txt::sendMail(uint32 senderid, const char* sendername, const char* targetname, const char *head, const char *body, uint32 zeny, const struct item& item, uint32& msgid, uint32& tid)
{
	basics::ScopeLock sl(cMx);
	bool ret = false;
	size_t pos;
	// search in index 1
	if( cCharList.find( CCharCharacter(targetname), pos, 1) &&
		cCharList[pos].char_id!=senderid )
	{
		char buffer[2048];

		uint32 targetid = cCharList(pos,1).char_id;
		uint32 mid = cMailDB.getfreekey();
		if(mid)
		{

			char sname[24], tname[24], h[32], b[80];
			time_t stime = time(NULL);
			stime = mktime(localtime(&stime));

			replacecpy(sname, 24, sendername);
			replacecpy(tname, 24, targetname);
			replacecpy(h,     32, head);
			replacecpy(b,     80, body);
			
			// sscanf cannot handle empty/whitespaced strings
			// so just put in something harmless
			if(sname[0]==0) strcpy(sname, ".");
			if(tname[0]==0) strcpy(tname, ".");
			if(h[0]==0) strcpy(h, ".");
			if(b[0]==0) strcpy(b, ".");

			size_t sz = snprintf(buffer, sizeof(buffer),
				"%lu\t%c\t%lu\t%.24s\t%lu\t%.24s\t%lu\t%.40s\t%.512s\t%lu\t(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n",
				(unsigned long)mid, '0', 
				(unsigned long)senderid, sname,
				(unsigned long)targetid, tname,
				(unsigned long)stime,
				h, b,
				(unsigned long)zeny,
				item.nameid, item.amount, /*item.equip*/0, item.identify, item.refine, item.attribute, 
				item.card[0], item.card[1], item.card[2], item.card[3] );
			msgid = mid;
			tid   = targetid;
			ret = cMailDB.insert(mid, tid, buffer, sz);
		}
	}
	return ret;
}








bool CGuildDB_txt::string2guild(const char *str, CGuild &g)
{
	unsigned int i, j, c;
	int tmp_int[16];
	char tmp_str[4][256];
	char tmp_str2[4096];
	char *pstr;

	tmp_str[2][0]=0;
	tmp_str[3][0]=0;
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
	safestrcpy(g.name, sizeof(g.name), tmp_str[0]);
	safestrcpy(g.master, sizeof(g.master), tmp_str[1]);
	safestrcpy(g.mes1, sizeof(g.mes1), tmp_str[2]);
	safestrcpy(g.mes2, sizeof(g.mes2), tmp_str[3]);

	// remove the default chars
	for(pstr = g.mes1+strlen(g.mes1)-1; pstr>=g.mes1 && *pstr=='#'; --pstr) *pstr=0;
	for(pstr = g.mes2+strlen(g.mes2)-1; pstr>=g.mes2 && *pstr=='#'; --pstr) *pstr=0;

	for(j=0; j<6 && str!=NULL; ++j)	// 位置スキップ
		str = strchr(str + 1, '\t');

	// メンバー
	if(g.max_member>MAX_GUILD) g.max_member=MAX_GUILD;
	memset(&g.member,0, sizeof(g.member));
	for(i=0; i<g.max_member && i<MAX_GUILD; ++i)
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
		safestrcpy(g.member[i].name, sizeof(g.member[i].name), tmp_str[0]);

		for(j=0; j<2 && str!=NULL; ++j)	// 位置スキップ
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
		safestrcpy(g.position[i].name, sizeof(g.position[i].name), tmp_str[0]);

		for(pstr = g.position[i].name+strlen(g.position[i].name)-1; pstr>=g.position[i].name && *pstr=='#'; --pstr) *pstr=0;

		for(j=0; j<2 && str!=NULL; ++j)	// 位置スキップ
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

	i=0;
	pstr=tmp_str2;
	while(i<g.emblem_len && i<sizeof(g.emblem_data) && pstr[0]!='$' && pstr[1]!='$')
	{
		int c1 = pstr[0], c2 = pstr[1], x1 = 0, x2 = 0;
		if (c1 >= '0' && c1 <= '9')
			x1 = c1 - '0';
		else if (c1 >= 'a' && c1 <= 'f')
			x1 = c1 - 'a' + 10;
		else if (c1 >= 'A' && c1 <= 'F')
			x1 = c1 - 'A' + 10;
		if (c2 >= '0' && c2 <= '9')
			x2 = c2 - '0';
		else if (c2 >= 'a' && c2 <= 'f')
			x2 = c2 - 'a' + 10;
		else if (c2 >= 'A' && c2 <= 'F')
			x2 = c2 - 'A' + 10;
		g.emblem_data[i++] = (x1<<4) | x2;
		pstr+=2;
	}

	str=strchr(str+1, '\t');	// 位置スキップ

	// 同盟リスト
	if (sscanf(str+1, "%d\t", &c) < 1)
		return false;

	str = strchr(str + 1, '\t');	// 位置スキップ
	memset(&g.alliance,0, sizeof(g.alliance));
	for(i = 0; i < c; ++i)
	{
		if( sscanf(str + 1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str[0]) < 3)
			return false;
		g.alliance[i].guild_id		= tmp_int[0];
		g.alliance[i].opposition	= tmp_int[1];
		safestrcpy(g.alliance[i].name, sizeof(g.alliance[i].name), tmp_str[0]);

		for(j=0; j<2 && str!=NULL; ++j)	// 位置スキップ
			str = strchr(str + 1, '\t');
	}

	// 追放リスト
	if (sscanf(str+1, "%d\t", &c) < 1)
		return false;

	str = strchr(str + 1, '\t');	// 位置スキップ
	memset(&g.explusion,0, sizeof(g.explusion));
	for(i=0; i<c; ++i)
	{
		if( sscanf(str + 1, "%d,%d,%d,%d\t%[^\t]\t%[^\t]\t%[^\t]\t",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			tmp_str[0], tmp_str[1], tmp_str[2]) < 6)
			return false;
		g.explusion[i].account_id = tmp_int[0];
		g.explusion[i].rsv1 = tmp_int[1];
		g.explusion[i].rsv2 = tmp_int[2];
		g.explusion[i].rsv3 = tmp_int[3];
		safestrcpy(g.explusion[i].name, sizeof(g.explusion[i].name), tmp_str[0]);
		safestrcpy(g.explusion[i].acc, sizeof(g.explusion[i].acc), tmp_str[1]);
		safestrcpy(g.explusion[i].mes, sizeof(g.explusion[i].mes), tmp_str[2]);

		for(j=0; j<4 && str!=NULL; ++j)	// 位置スキップ
			str = strchr(str+1, '\t');
	}

	// ギルドスキル
	for(i=0; i<MAX_GUILDSKILL; ++i)
	{
		g.skill[i].id = i+GD_SKILLBASE;
		g.skill[i].lv = 0;
	}
	while( 2==sscanf(str+1,"%d,%d ", &tmp_int[0], &tmp_int[1]) )
	{
		i = tmp_int[0]-GD_SKILLBASE;
		if(i<MAX_GUILDSKILL)
			g.skill[i].lv = tmp_int[1];
		str = strchr(str+1, ' ');
	}
	str = strchr(str + 1, '\t');

	return true;
}

ssize_t CGuildDB_txt::guild2string(char *str, size_t maxlen, const CGuild &g)
{
	ssize_t i, c, len;

	// 基本データ
	len = snprintf(str, maxlen, "%ld\t%s\t%s\t%d,%d,%ld,%d,%d\t%s#\t%s#\t",
				  (unsigned long)g.guild_id, g.name, g.master,
				  g.guild_lv, g.max_member, (unsigned long)g.exp, g.skill_point, 0,//g.castle_id,
				  g.mes1, g.mes2);
	// メンバー
	for(i = 0; i<g.max_member && i<MAX_GUILD; ++i) {
		const struct guild_member &m = g.member[i];
		len += snprintf(str + len, maxlen-len, "%ld,%ld,%d,%d,%d,%d,%d,%ld,%ld,%d\t%s\t",
					   (unsigned long)m.account_id, (unsigned long)m.char_id,
					   m.hair, m.hair_color, m.gender,
					   m.class_, m.lv, (unsigned long)m.exp, (unsigned long)m.exp_payper, m.position,
					   ((m.account_id > 0) ? m.name : "-"));
	}
	// 役職
	for(i = 0; i < MAX_GUILDPOSITION; ++i) {
		const struct guild_position &p = g.position[i];
		len += snprintf(str + len, maxlen-len, "%ld,%ld\t%s#\t", (unsigned long)p.mode, (unsigned long)p.exp_mode, p.name);
	}
	// エンブレム
	len += snprintf(str + len, maxlen-len, "%d,%ld,", g.emblem_len, (unsigned long)g.emblem_id);
	for(i = 0; i < g.emblem_len; ++i) {
		len += snprintf(str + len, maxlen-len, "%02x", (unsigned char)(g.emblem_data[i]));
	}
	len += snprintf(str + len, maxlen-len, "$\t");
	// 同盟リスト
	
	for(i=0, c=0; i<MAX_GUILDALLIANCE; ++i)
		if(g.alliance[i].guild_id > 0)
			c++;

	len += snprintf(str + len, maxlen-len, "%ld\t", (unsigned long)c);

	for(i = 0; i < MAX_GUILDALLIANCE; ++i)
	{
		const struct guild_alliance &a = g.alliance[i];
		if (a.guild_id > 0)
			len += snprintf(str + len, maxlen-len, "%ld,%ld\t%s\t", (unsigned long)a.guild_id, (unsigned long)a.opposition, a.name);
	}
	// 追放リスト
	for(i=0,c=0; i<MAX_GUILDEXPLUSION; ++i)
		if (g.explusion[i].account_id > 0)
			c++;

	len += snprintf(str + len, maxlen-len, "%ld\t", (unsigned long)c);
	for(i = 0; i < MAX_GUILDEXPLUSION; ++i)
	{
		const struct guild_explusion &e = g.explusion[i];
		if (e.account_id > 0)
			len += snprintf(str + len, maxlen-len, "%ld,%ld,%ld,%ld\t%s\t%s\t%s#\t",
						   (unsigned long)e.account_id, (unsigned long)e.rsv1, (unsigned long)e.rsv2, (unsigned long)e.rsv3,
						   e.name, e.acc, e.mes );
	}
	// ギルドスキル
	for(i = 0; i < MAX_GUILDSKILL; ++i)
	{
		len += snprintf(str + len, maxlen-len, "%d,%d ", g.skill[i].id, g.skill[i].lv);
	}
	len += snprintf(str + len, maxlen-len, "\t"RETCODE);
	return len;
}

ssize_t CGuildDB_txt::castle2string(char *str, size_t maxlen, const CCastle &gc)
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
bool CGuildDB_txt::string2castle(char *str, CCastle &gc)
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


bool CGuildDB_txt::do_readGuildsCastles()
{
	char line[16384];
	FILE *fp;
	int i, j, c;

	///////////////////////////////////////////////////////////////////////
	fp = basics::safefopen(guild_filename(),"r");
	if(fp == NULL)
	{
		ShowError("can't read %s\n", (const char*)guild_filename());
		return false;
	}

	c=0;
	while(fgets(line, sizeof(line), fp))
	{
		c++;
		if( !is_valid_line(line) )
			continue;

		CGuild g;
		j = 0;
		if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 )//&& next_guild_id <= (uint32)i)
		{
			//next_guild_id = i;
			continue;
		}
		if( string2guild(line, g) && g.guild_id > 0 )
		{
			g.calcInfo();
			if(g.max_member>0 && g.max_member < MAX_GUILD)
			{
				for(i=0; i<g.max_member && i<MAX_GUILD; ++i)
				{
					if(g.member[i].account_id>0)
						break;
				}
				if(i<g.max_member)
				{
					//if( g.guild_id >= next_guild_id )
					//	next_guild_id = g.guild_id + 1;

					cGuilds.insert(g);
				}
			}
		}
		else
		{
			ShowError("Guild: broken data [%s] line %d\n", (const char*)guild_filename(), c);
		}
	}
	fclose(fp);
	ShowStatus("Guild: %s read done (%d guilds)\n", (const char*)guild_filename(), cGuilds.size());
	///////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	fp = basics::safefopen(castle_filename(), "r");
	if( fp==NULL )
	{
		ShowError("GuildCastle: cannot open %s\n", (const char*)castle_filename());
		return false;
	}
	c = 0;
	while(fgets(line, sizeof(line), fp))
	{
		c++;

		if( !is_valid_line(line) )
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
			ShowError("GuildCastle: broken data [%s] line %d\n", (const char*)castle_filename(), c);

	}
	fclose(fp);
	ShowStatus("GuildCastle: %s read done (%d castles)\n", (const char*)castle_filename(), cCastles.size());

	for(i = 0; i < MAX_GUILDCASTLE; ++i)
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
bool CGuildDB_txt::do_saveGuildsCastles()
{
	bool ret=true;
	char line[65536];
	FILE *fp;
	int lock;
	size_t i, sz;
	///////////////////////////////////////////////////////////////////////
	fp = lock_fopen(guild_filename(), lock);
	if( fp == NULL) {
		ShowError("Guild: cannot open [%s]\n", (const char*)guild_filename());
		ret = false;
	}
	else
	{
		for(i=0; i<cGuilds.size(); ++i)
		{
			sz=guild2string(line, sizeof(line), cGuilds[i]);
			if(sz>0)
				fwrite(line, sz,1,fp);
		}
		lock_fclose(fp, guild_filename(), lock);
	}


	///////////////////////////////////////////////////////////////////////
	fp = lock_fopen(castle_filename(), lock);
	if( fp == NULL) {
		ShowError("Guild: cannot open [%s]\n", (const char*)castle_filename());
		ret = false;
	}
	else
	{
		for(i=0; i<cCastles.size(); ++i)
		{
			sz=castle2string(line, sizeof(line), cCastles[i]);
			//fprintf(fp, "%s" RETCODE, line);	// retcode integrated to line generation
			if(sz>0) fwrite(line, sz,1,fp);
		}
		lock_fclose(fp, castle_filename(), lock);
	}
	///////////////////////////////////////////////////////////////////////
	return ret;
}

///////////////////////////////////////////////////////////////////////////
// construct/destruct
CGuildDB_txt::CGuildDB_txt(const char *configfile) :
	CTimerBase(60000),		// 60sec save interval
	CGuildDB_mem(configfile),
	guild_filename("guild_filename","save/guild.txt"),
	castle_filename("castle_filename","save/castle.txt"),
	savecount(0)
{
	this->init(NULL);
}
CGuildDB_txt::~CGuildDB_txt()
{
	this->close();
}

///////////////////////////////////////////////////////////////////////////
// normal function
bool CGuildDB_txt::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readGuildsCastles();
}
bool CGuildDB_txt::close()
{
	return this->do_saveGuildsCastles();
}

///////////////////////////////////////////////////////////////////////////
// timer function
bool CGuildDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if( this->savecount > 10 )
	{
		this->savecount=0;
		this->do_saveGuildsCastles();
	}
	return true;
}









///////////////////////////////////////////////////////////////////////////////
// Party Database
///////////////////////////////////////////////////////////////////////////////

ssize_t CPartyDB_txt::party_to_string(char *str, size_t maxlen, const CParty &p)
{
	ssize_t i, len;
	len = snprintf(str, maxlen, "%ld\t%s\t%d,%d\t", (unsigned long)p.party_id, p.name, p.expshare, p.itemshare);
	for(i = 0; i < MAX_PARTY; ++i)
	{
		const struct party_member &m = p.member[i];
		len += snprintf(str+len, maxlen-len, "%ld,%ld\t%s\t", (unsigned long)m.account_id, (unsigned long)m.leader, ((m.account_id > 0) ? m.name : "NoMember"));
	}
	snprintf(str+len, maxlen-len, RETCODE);
	return len;
}
bool CPartyDB_txt::party_from_string(const char *str, CParty &p)
{
	int i, j;
	int tmp_int[16];
	char tmp_str[256];

	if (sscanf(str, "%d\t%255[^\t]\t%d,%d\t", &tmp_int[0], tmp_str, &tmp_int[1], &tmp_int[2]) != 4)
		return false;

	p.party_id = tmp_int[0];
	safestrcpy(p.name, sizeof(p.name), tmp_str);
	p.expshare = tmp_int[1];
	p.itemshare = tmp_int[2];
	for(j=0; j<3 && str != NULL; ++j)
		str = strchr(str+1, '\t');

	for(i=0; i<MAX_PARTY; ++i)
	{
		struct party_member &m = p.member[i];
		if(str == NULL)
			return false;
		if(sscanf(str + 1, "%d,%d\t%255[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str) != 3)
			return false;

		m.account_id = tmp_int[0];
		m.leader = tmp_int[1];
		safestrcpy(m.name, sizeof(m.name), tmp_str);
		for(j=0; j<2 && str != NULL; ++j)
			str = strchr(str + 1, '\t');
	}
	return true;
}
bool CPartyDB_txt::do_readParties()
{
	char line[8192];
	int c = 0;
	int i, j;
	FILE *fp = basics::safefopen(party_filename(), "r");

	if( fp == NULL )
	{
		ShowError("Party: cannot open %s\n", (const char*)party_filename());
		return false;
	}
	while(fgets(line, sizeof(line), fp))
	{
		c++;
		if( !is_valid_line(line) )
			continue;

		j = 0;
		if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 )//&& next_party_id <= (uint32)i)
		{
			//next_party_id = i;
		}
		else
		{
			CParty p;
			if( party_from_string(line, p) && p.party_id > 0)
			{
				if( !p.isEmpty() )
				{
				//	if(p.party_id >= next_party_id)
				//		next_party_id = p.party_id + 1;

					cParties.insert(p);
				}
			}
			else
			{
				ShowError("int_party: broken data [%s] line %d\n", (const char*)party_filename(), c);
			}
		}
	}
	fclose(fp);
	ShowStatus("Party: %s read done (%d parties)\n", (const char*)party_filename(), c);
	return true;
}
bool CPartyDB_txt::do_saveParties()
{
	char line[65536];
	FILE *fp;
	int lock;
	size_t i;
	ssize_t sz;

	if ((fp = lock_fopen(party_filename(), lock)) == NULL) {
		ShowError("Party: cannot open [%s]\n", (const char*)party_filename());
		return false;
	}
	for(i=0; i<cParties.size(); ++i)
	{
		sz = party_to_string(line, sizeof(line), cParties[i]);
		if(sz>0) fwrite(line, sz,1, fp);
	}
	//fprintf(fp, "%d\t%%newid%%\n", next_party_id);
	lock_fclose(fp,party_filename(), lock);
	return 0;
}


///////////////////////////////////////////////////////////////////////////
// construct/destruct
CPartyDB_txt::CPartyDB_txt(const char *configfile) :
	CTimerBase(60000),		// 60sec save interval
	CPartyDB_mem(configfile),
	party_filename("party_filename", "save/party.txt"),
	savecount(0)
{
	this->init(NULL);
}
CPartyDB_txt::~CPartyDB_txt()
{
	this->close();
}

///////////////////////////////////////////////////////////////////////////
// normal function
bool CPartyDB_txt::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readParties();
}
bool CPartyDB_txt::close()
{
	return this->do_saveParties();
}

///////////////////////////////////////////////////////////////////////////
// timer function
bool CPartyDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if( this->savecount > 10 )
	{
		this->savecount=0;
		this->do_saveParties();
	}
	return true;
}










ssize_t CPCStorageDB_txt::storage_to_string(char *str, size_t maxlen, const CPCStorage &stor)
{
	int i,f=0;
	char *str_p = str;
	str_p += snprintf(str_p,maxlen,"%ld,%d\t",(unsigned long)stor.account_id, stor.storage_amount);
	for(i=0; i<MAX_STORAGE; ++i)
	{
		if( (stor.storage[i].nameid) && (stor.storage[i].amount) )
		{
			str_p += snprintf(str_p,str+maxlen-str_p,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
				i,stor.storage[i].nameid,stor.storage[i].amount,stor.storage[i].equip,
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
int CPCStorageDB_txt::storage_from_string(const char *str, CPCStorage &stor)
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
	for(i=0;str[next] && str[next]!='\t' && i<MAX_STORAGE; ++i)
	{
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			&tmp_int[4], &tmp_int[5], &tmp_int[6],
			&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
		{
			//stor.storage[i].id = tmp_int[0];
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
			//stor.storage[i].id = tmp_int[0];
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
bool CPCStorageDB_txt::do_readPCStorage()
{
	char line[65536];
	int c=0;
	unsigned long tmp;
	CPCStorage s;
	FILE *fp=basics::safefopen(pcstorage_filename(),"r");

	if(fp==NULL)
	{
		ShowError("Storage: cannot open %s\n", (const char*)pcstorage_filename());
		return false;
	}
	while(fgets(line,sizeof(line),fp))
	{
		c++;
		if( !is_valid_line(line) )
			continue;

		sscanf(line,"%ld",&tmp);
		s.account_id=tmp;
		if(s.account_id > 0 && storage_from_string(line,s) )
		{
			cPCStorList.insert(s);
		}
		else
		{
			ShowError("Storage: broken data [%s] line %d\n",(const char*)pcstorage_filename(),c);
		}
	}
	fclose(fp);
	return true;
}
bool CPCStorageDB_txt::do_savePCStorage()
{
	char line[65536];
	int lock;
	size_t i, sz;
	FILE *fp=lock_fopen(pcstorage_filename(), lock);

	if( fp==NULL )
	{
		ShowError("Storage: cannot open [%s]\n",(const char*)pcstorage_filename());
		return false;
	}
	for(i=0; i<cPCStorList.size(); ++i)
	{
		sz = storage_to_string(line, sizeof(line), cPCStorList[i]);
		if(sz>0) fprintf(fp,"%s"RETCODE,line);
	}
	lock_fclose(fp, pcstorage_filename(), lock);
	return true;
}


///////////////////////////////////////////////////////////////////////////
// construct/destruct
CPCStorageDB_txt::CPCStorageDB_txt(const char *dbcfgfile) :
	CTimerBase(60000),		// 60sec save interval
	CPCStorageDB_mem(dbcfgfile),
	pcstorage_filename( "pcstorage_filename", "save/storage.txt"),
	savecount(0)
{
	this->init(NULL);
}
CPCStorageDB_txt::~CPCStorageDB_txt()
{
	this->close();
}

///////////////////////////////////////////////////////////////////////////
// normal function
bool CPCStorageDB_txt::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readPCStorage();
}
bool CPCStorageDB_txt::close()
{
	return this->do_savePCStorage();
}

///////////////////////////////////////////////////////////////////////////
// timer function
bool CPCStorageDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if( this->savecount > 10 )
	{
		this->savecount=0;
		this->do_savePCStorage();
	}
	return true;
}









ssize_t CGuildStorageDB_txt::guild_storage_to_string(char *str, size_t maxlen, const CGuildStorage &stor)
{
	int i,f=0;
	char *str_p = str;
	str_p+=snprintf(str,maxlen,"%ld,%d\t",(unsigned long)stor.guild_id, stor.storage_amount);

	for(i=0;i<MAX_GUILD_STORAGE; ++i)
	{
		if( (stor.storage[i].nameid) && (stor.storage[i].amount) )
		{
			str_p += snprintf(str_p,str+maxlen-str_p,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
				i,stor.storage[i].nameid,stor.storage[i].amount,stor.storage[i].equip,
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
bool CGuildStorageDB_txt::guild_storage_from_string(const char *str, CGuildStorage &stor)
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
	for(i=0; str[next] && str[next]!='\t' && i<MAX_GUILD_STORAGE; ++i)
	{
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			&tmp_int[4], &tmp_int[5], &tmp_int[6],
			&tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12)
		{
			//stor.storage[i].id = tmp_int[0];
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
			//stor.storage[i].id = tmp_int[0];
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

bool CGuildStorageDB_txt::do_readGuildStorage()
{
	char line[65536];
	int c=0;
	unsigned long tmp;
	CGuildStorage gs;
	FILE *fp=basics::safefopen(guildstorage_filename(),"r");
	if(fp==NULL){
		ShowMessage("cant't read : %s\n",(const char*)guildstorage_filename());
		return 1;
	}
	while(fgets(line,sizeof(line),fp))
	{
		c++;
		if( !is_valid_line(line) )
			continue;

		sscanf(line,"%ld",&tmp);
		gs.guild_id=tmp;
		if(gs.guild_id > 0 && guild_storage_from_string(line,gs) )
		{
			cGuildStorList.insert(gs);
		}
		else
		{
			ShowError("Storage: broken data [%s] line %d\n", (const char*)guildstorage_filename(), c);
		}
	}
	fclose(fp);
	return true;
}
bool CGuildStorageDB_txt::do_saveGuildStorage()
{
	char line[65536];
	int lock;
	size_t i, sz;
	FILE *fp=lock_fopen(guildstorage_filename(), lock);

	if( fp==NULL )
	{
		ShowError("Storage: cannot open [%s]\n",(const char*)guildstorage_filename());
		return false;
	}
	for(i=0; i<cGuildStorList.size(); ++i)
	{
		sz = guild_storage_to_string(line, sizeof(line), cGuildStorList[i]);
		if(sz>0) fprintf(fp,"%s"RETCODE,line);
	}
	lock_fclose(fp, guildstorage_filename(), lock);
	return true;
}

///////////////////////////////////////////////////////////////////////////
// construct/destruct
CGuildStorageDB_txt::CGuildStorageDB_txt(const char *dbcfgfile) :
	CTimerBase(60000),		// 60sec save interval
	CGuildStorageDB_mem(dbcfgfile),
	guildstorage_filename("guildstorage_filename","save/g_storage.txt"),
	savecount(0)
{
	this->init(NULL);
}
CGuildStorageDB_txt::~CGuildStorageDB_txt()
{
	this->close();
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CGuildStorageDB_txt::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readGuildStorage();
}
bool CGuildStorageDB_txt::close()
{
	return this->do_saveGuildStorage();
}

///////////////////////////////////////////////////////////////////////////
// timer function
bool CGuildStorageDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if( this->savecount > 10 )
	{
		this->savecount=0;
		this->do_saveGuildStorage();
	}
	return true;
}













int CPetDB_txt::pet_to_string(char *str, size_t sz, CPet &pet)
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

bool CPetDB_txt::pet_from_string(const char *str, CPet &pet)
{
	int tmp_int[16];
	char tmp_str[256];
	int s=sscanf(str,"%d,%d,%[^\t]\t%d,%d,%d,%d,%d,%d,%d,%d,%d",&tmp_int[0],&tmp_int[1],tmp_str,&tmp_int[2],
			&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],&tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10]);

	if(s==12)
	{
		pet.pet_id = tmp_int[0];
		pet.class_ = tmp_int[1];
		safestrcpy(pet.name, sizeof(pet.name), tmp_str);
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

bool CPetDB_txt::do_readPets()
{
	char line[65536];
	int c=0;
	CPet pet;
	FILE *fp=basics::safefopen(pet_filename(),"r");
	if(fp==NULL){
		ShowMessage("cant't read : %s\n",(const char*)pet_filename());
		return 1;
	}
	while(fgets(line,sizeof(line),fp))
	{
		c++;
		if( !is_valid_line(line) )
			continue;

		if( pet_from_string(line,pet) )
		{
			cPetList.insert(pet);
		}
		else
		{
			ShowError("Storage: broken data [%s] line %d\n", (const char*)pet_filename(), c);
		}
	}
	fclose(fp);
	return true;
}
bool CPetDB_txt::do_savePets()
{
	char line[65536];
	int lock;
	size_t i, sz;
	FILE *fp=lock_fopen(pet_filename(), lock);

	if( fp==NULL )
	{
		ShowError("Storage: cannot open [%s]\n",(const char*)pet_filename());
		return false;
	}
	for(i=0; i<cPetList.size(); ++i)
	{
		sz = pet_to_string(line, sizeof(line), cPetList[i]);
		if(sz>0) fprintf(fp,"%s"RETCODE,line);
	}
	lock_fclose(fp, pet_filename(), lock);
	return true;
}

///////////////////////////////////////////////////////////////////////////
// construct/destruct
CPetDB_txt::CPetDB_txt(const char *dbcfgfile) :
	CTimerBase(60000),		// 60sec save interval
	CPetDB_mem(dbcfgfile),
	pet_filename("pet_filename", "save/pet.txt"),
	savecount(0)
{
	this->init(NULL);
}
CPetDB_txt::~CPetDB_txt()
{
	this->close();
}


///////////////////////////////////////////////////////////////////////////
// normal function
bool CPetDB_txt::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readPets();
}
bool CPetDB_txt::close()
{
	return this->do_savePets();
}

///////////////////////////////////////////////////////////////////////////
// timer function
bool CPetDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	// we have do some authentifications without do saving
	if( this->savecount > 10 )
	{
		this->savecount=0;
		this->do_savePets();
	}
	return true;
}















int CHomunculusDB_txt::homunculus_to_string(char *str, size_t sz, CHomunculus &hom)
{
	int len=0, i;

	len=snprintf(str, sz, 
		"%lu,%lu,%lu"
		"\t%.24s\t"
		"%lu,"
		"%lu,%lu,"
		"%lu,%lu,"
		"%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,"
		"%u,%u,"
		"%lu,%u,%u,%u,%u\t",
		(ulong)hom.homun_id, (ulong)hom.account_id, (ulong)hom.char_id,
		(hom.name && hom.name[0])?hom.name:"dummy", 
		(ulong)hom.base_exp,
		(ulong)hom.hp,(ulong)hom.max_hp,
		(ulong)hom.sp,(ulong)hom.max_sp,
		hom.class_,hom.status_point,hom.skill_point,
		hom.str,hom.agi,hom.vit,hom.int_,hom.dex,hom.luk,
		hom.option,hom.equip,
		(ulong)hom.intimate,hom.hungry,hom.base_level,hom.rename_flag,hom.incubate);

	for(i=0; i<MAX_HOMSKILL; ++i)
	{
		if( hom.skill[i].id && hom.skill[i].lv )
		{
			len += snprintf(str+len, sz-len, "%d,%d\t", hom.skill[i].id, hom.skill[i].lv);
		}
	}
	return len;
}

bool CHomunculusDB_txt::homunculus_from_string(const char *str, CHomunculus &hom)
{
	bool ret = false;
	unsigned int tmp_int[32];
	char tmp_str[256];
	int len;
	int s=sscanf(str,
		"%u,%u,%u"
		"\t%256[^\t]\t"
		"%u,"
		"%u,%u,"
		"%u,%u,"
		"%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,"
		"%u,%u,"
		"%u,%u,%u,%u,%u\t"
		"%n",
		&tmp_int[ 0],&tmp_int[ 1],&tmp_int[ 2],
		tmp_str,
		&tmp_int[ 3],
		&tmp_int[ 4],&tmp_int[ 5],
		&tmp_int[ 6],&tmp_int[ 7],
		&tmp_int[ 8],&tmp_int[ 9],&tmp_int[10],
		&tmp_int[11],&tmp_int[12],&tmp_int[13],&tmp_int[14],&tmp_int[15],&tmp_int[16],
		&tmp_int[17],&tmp_int[18],
		&tmp_int[19],&tmp_int[20],&tmp_int[21],&tmp_int[22],&tmp_int[23],
		&len);

	if(s==25)
	{
		hom.homun_id		= tmp_int[ 0];
		hom.account_id		= tmp_int[ 1];
		hom.char_id			= tmp_int[ 2];
		hom.base_exp		= tmp_int[ 3];
		safestrcpy(hom.name, sizeof(hom.name), tmp_str);
		hom.hp				= tmp_int[ 4];
		hom.max_hp			= tmp_int[ 5];
		hom.sp				= tmp_int[ 6];
		hom.max_sp			= tmp_int[ 7];
		hom.class_			= tmp_int[ 8];
		hom.status_point	= tmp_int[ 9];
		hom.skill_point		= tmp_int[10];
		hom.str				= tmp_int[11];
		hom.agi				= tmp_int[12];
		hom.vit				= tmp_int[13];
		hom.int_			= tmp_int[14];
		hom.dex				= tmp_int[15];
		hom.luk				= tmp_int[16];
		hom.option			= tmp_int[17];
		hom.equip			= tmp_int[18];
		hom.intimate		= tmp_int[19];
		hom.hungry			= tmp_int[20];
		hom.base_level		= tmp_int[21];
		hom.rename_flag		= tmp_int[22];
		hom.incubate		= tmp_int[23];

		if(hom.hungry < 0)
			hom.hungry = 0;
		else if(hom.hungry > 100)
			hom.hungry = 100;
		if(hom.intimate > 100000)
			hom.intimate = 100000;

		ret = true;
	}

	if(ret)
	{	
		// start with the next char after the delimiter
		int next = len++;
		size_t i;

		for(i=0; i<MAX_HOMSKILL; ++i)
		{
			hom.skill[i].id = i+HOM_SKILLID;
			hom.skill[i].lv = 0;
			hom.skill[i].flag = 0;
		}
		while( str[next] && 2==sscanf(str+next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len) )
		{
			i = tmp_int[0]-HOM_SKILLID;
			if( tmp_int[1] && i < MAX_HOMSKILL )
			{
				hom.skill[i].lv = tmp_int[1];
			}
			next += len;
			if(str[next] == '\t')
				next++;
		}
	}
	return ret;
}

bool CHomunculusDB_txt::do_readHomunculus()
{
	char line[65536];
	int c=0;
	CHomunculus hom;
	FILE *fp=basics::safefopen(homunculus_filename(),"r");
	if(fp==NULL)
	{
		ShowError("cant't read : %s\n",(const char*)homunculus_filename());
		return false;
	}
	while(fgets(line,sizeof(line),fp))
	{
		c++;
		if( !is_valid_line(line) )
			continue;

		if( homunculus_from_string(line,hom) )
		{
			cHomunculusList.insert(hom);
		}
		else
		{
			ShowError("Homunculus: broken data [%s] line %d\n", (const char*)homunculus_filename(), c);
		}
	}
	fclose(fp);
	return true;
}
bool CHomunculusDB_txt::do_saveHomunculus()
{
	char line[65536];
	int lock;
	size_t i, sz;
	FILE *fp=lock_fopen(homunculus_filename(), lock);

	if( fp==NULL )
	{
		ShowError("Homunculus: cannot open [%s]\n",(const char*)homunculus_filename());
		return false;
	}
	for(i=0; i<cHomunculusList.size(); ++i)
	{
		sz = homunculus_to_string(line, sizeof(line), cHomunculusList[i]);
		if(sz>0) fprintf(fp,"%s"RETCODE,line);
	}
	lock_fclose(fp, homunculus_filename(), lock);
	return true;
}

///////////////////////////////////////////////////////////////////////////
// construct/destruct
CHomunculusDB_txt::CHomunculusDB_txt(const char *dbcfgfile) :
	CTimerBase(60000),		// 60sec save interval
	CHomunculusDB_mem(dbcfgfile),
	homunculus_filename("homunculus_filename", "save/homunculus.txt"),
	savecount(0)
{
	this->init(NULL);
}
CHomunculusDB_txt::~CHomunculusDB_txt()
{
	this->close();
}

///////////////////////////////////////////////////////////////////////////
// normal function
bool CHomunculusDB_txt::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readHomunculus();
}
bool CHomunculusDB_txt::close()
{
	return this->do_saveHomunculus();
}

///////////////////////////////////////////////////////////////////////////
// timer function
bool CHomunculusDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	if( this->savecount )//> 10 )
	{
		this->savecount=0;
		this->do_saveHomunculus();
	}
	return true;
}





bool CVarDB_txt::do_readVars()
{
	char line[65536];
	int c=0;
	CVar var;
	FILE *fp=basics::safefopen(variable_filename(),"r");
	if(fp==NULL)
	{
		ShowError("cant't read : %s\n",(const char*)variable_filename());
		return false;
	}
	while(fgets(line,sizeof(line),fp))
	{
		c++;
		if( !is_valid_line(line) )
			continue;

		if( var.from_string(line) )
		{
			cVarList.insert(var);
		}
		else
		{
			ShowError("Variable: broken data [%s] line %d\n", (const char*)variable_filename(), c);
		}
	}
	fclose(fp);
	return true;
}
bool CVarDB_txt::do_saveVars()
{
	char line[65536];
	int lock;
	size_t i, sz;
	FILE *fp=lock_fopen(variable_filename(), lock);

	if( fp==NULL )
	{
		ShowError("Variable: cannot open [%s]\n",(const char*)variable_filename());
		return false;
	}
	for(i=0; i<cVarList.size(); ++i)
	{
		sz = cVarList[i].to_string(line, sizeof(line));
		if(sz>0) fprintf(fp,"%s"RETCODE, line);
	}
	lock_fclose(fp, variable_filename(), lock);
	return true;
}

CVarDB_txt::CVarDB_txt(const char *dbcfgfile) :
	CTimerBase(60000),		// 60sec save interval
	CVarDB_mem(dbcfgfile),
	variable_filename("variable_filename", "save/variables.txt"),
	savecount(0)
{
	this->init(NULL);
}

CVarDB_txt::~CVarDB_txt()
{
	this->close();
}

bool CVarDB_txt::init(const char* configfile)
{	// init db
	if(configfile) basics::CParamBase::loadFile(configfile);
	return this->do_readVars();
}
bool CVarDB_txt::close()
{
	return this->do_saveVars();
}

bool CVarDB_txt::timeruserfunc(unsigned long tick)
{
	// we only save if necessary:
	if( this->savecount )//> 10 )
	{
		this->savecount=0;
		this->do_saveVars();
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
#endif//WITH_TEXT
///////////////////////////////////////////////////////////////////////////////
