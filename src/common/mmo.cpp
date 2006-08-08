#include "mmo.h"
#include "showmsg.h"



////////////////////////////////////////////////////
// CAuth Class
////////////////////////////////////////////////////
void CAuth::_tobuffer(unsigned char* &buf) const
{
	if(buf)
	{
		_L_tobuffer( account_id,	buf);
		_L_tobuffer( login_id1, buf);
		_L_tobuffer( login_id2, buf);
		_L_tobuffer( client_ip, buf);
	}
}

void CAuth::_frombuffer(const unsigned char* &buf)
{
	if(buf)
	{
		_L_frombuffer( account_id,	buf);
		_L_frombuffer( login_id1, buf);
		_L_frombuffer( login_id2, buf);
		_L_frombuffer( client_ip, buf);
	}
}

////////////////////////////////////////////////////
// CAccountReg Class
////////////////////////////////////////////////////
void CAccountReg::_tobuffer(unsigned char* &buf) const
{
	if(buf)
	{
		size_t i;
		_W_tobuffer( (account_reg2_num),	buf);

		for(i=0; i<account_reg2_num && i<ACCOUNT_REG2_NUM; ++i)
			_global_reg_tobuffer(account_reg2[i],buf);
	}
}

void CAccountReg::_frombuffer(const unsigned char* &buf)
{
	if(buf)
	{
		size_t i;
		_W_frombuffer( (account_reg2_num),	buf);

		for(i=0; i<account_reg2_num && i<ACCOUNT_REG2_NUM; ++i)
			_global_reg_frombuffer(account_reg2[i],buf);
	}
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
{
	if(buf)
	{
		// only take 32bits of the timer
		uint32 time;
		_B_tobuffer( sex,			buf);
		_B_tobuffer( gm_level,		buf);
		time = ban_until;	_L_tobuffer( time, buf);
		time = valid_until;	_L_tobuffer( time, buf);
		CAuth::_tobuffer(buf);
		CAccountReg::_tobuffer(buf);
	}
}

void CMapAccount::_frombuffer(const unsigned char* &buf)
{
	if(buf)
	{
		// only take 32bits of the timer
		uint32 time;
		_B_frombuffer( sex,			buf);
		_B_frombuffer( gm_level,	buf);
		_L_frombuffer( time, buf);	ban_until=time;
		_L_frombuffer( time, buf);	valid_until=time;
		CAuth::_frombuffer(buf);
		CAccountReg::_frombuffer(buf);
	}
}

////////////////////////////////////////////////////
// CCharAccount Class
////////////////////////////////////////////////////
void CCharAccount::_tobuffer(unsigned char* &buf) const
{
	if(buf)
	{
		_S_tobuffer( email,			buf, sizeof(email));
		CMapAccount::_tobuffer(buf);
	}
}

void CCharAccount::_frombuffer(const unsigned char* &buf)
{
	if(buf)
	{
		_S_frombuffer( email,		buf, sizeof(email));
		CMapAccount::_frombuffer(buf);
	}
}






////////////////////////////////////////////////////
// struct item compare
////////////////////////////////////////////////////
bool operator==(const struct item& a, const struct item& b)
{
	return ( (a.nameid == b.nameid) &&
			 (a.amount == b.amount) &&
			 (a.equip == b.equip) &&
			 (a.identify == b.identify) &&
			 (a.refine == b.refine) &&
			 (a.attribute == b.attribute) &&
			 (a.card[0] == b.card[0]) &&
			 (a.card[1] == b.card[1]) &&
			 (a.card[2] == b.card[2]) &&
			 (a.card[3] == b.card[3]) );
}





///////////////////////////////////////////////////////////////////////////////
// Guild Experience
void CGuildExp::init(const char* filename)
{
	FILE* fp=basics::safefopen(filename,"r");
	memset(exp,0,sizeof(exp));
	if(fp==NULL)
	{
		ShowError("can't read %s\n", filename);
	}
	else
	{
		char line[1024];
		size_t c=0;
		while(fgets(line,sizeof(line),fp) && c<sizeof(exp)/sizeof(exp[0]))
		{
			if( !is_valid_line(line) )
				continue;
			exp[c]=atoi(line);
			c++;
		}
		fclose(fp);
	}
}
///////////////////////////////////////////////////////////////////////////////
// Guild Class Definition
///////////////////////////////////////////////////////////////////////////////

// static member of CGuild
CGuildExp CGuild::cGuildExp;

int CGuild::compare(const CGuild& c, size_t i) const	
{
	if(i==0)
		return (this->guild_id - c.guild_id);
	else
		return strcmp(this->name, c.name); 
}

unsigned short CGuild::checkSkill(unsigned short id)
{
	unsigned short idx = id - GD_SKILLBASE;
	if(idx < MAX_GUILDSKILL)
		return skill[idx].lv;
	return 0;
}

bool CGuild::calcInfo()
{
	size_t i,c;
	uint32 nextexp;
	unsigned short before_max_member = this->max_member;
	unsigned short before_guild_lv = this->guild_lv;
	unsigned short before_skill_point = this->skill_point;

	// スキルIDの設定
	for(i=0;i<MAX_GUILDSKILL; ++i)
		this->skill[i].id = i+GD_SKILLBASE;

	// ギルドレベル
	if(this->guild_lv<=0) this->guild_lv=1;
	nextexp = cGuildExp[this->guild_lv];
	while(this->exp >= nextexp && nextexp > 0)
	{
		this->exp-=nextexp;
		this->guild_lv++;
		this->skill_point++;
		nextexp = cGuildExp[this->guild_lv];
	}

	// ギルドの次の経験値
	this->next_exp = cGuildExp[this->guild_lv];

	// メンバ上限（ギルド拡張適用）
	this->max_member = 16 + this->checkSkill(GD_EXTENSION) * 6; //  Guild Extention skill - adds by 6 people per level to Max Member [Lupus]

	// 平均レベルとオンライン人数
	this->average_lv=0;
	this->connect_member=0;
	for(i=0,c=0; i<this->max_member; ++i){
		if(this->member[i].account_id>0){
			this->average_lv+=this->member[i].lv;
			c++;

			if(this->member[i].online>0)
				this->connect_member++;
		}
	}
	if(c) this->average_lv/=c;

	// return true on changing a value
	return ( before_max_member != this->max_member ||
			 before_guild_lv != this->guild_lv ||
			 before_skill_point != this->skill_point );
}
int CGuild::is_empty()
{
	size_t i;
	for(i=0; i<this->max_member; ++i)
	{
		if (this->member[i].account_id > 0)
			return true;
	}
	return false;
}

