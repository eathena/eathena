// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mmo.h"
#include "showmsg.h"




///////////////////////////////////////////////////////////////////////////////
/// class_id 2 jobname conversion
const char * job_name(unsigned short class_)
{
	switch (class_) {
	case 0:    return "Novice";
	case 1:    return "Swordsman";
	case 2:    return "Mage";
	case 3:    return "Archer";
	case 4:    return "Acolyte";
	case 5:    return "Merchant";
	case 6:    return "Thief";
	case 7:    return "Knight";
	case 8:    return "Priest";
	case 9:    return "Wizard";
	case 10:   return "Blacksmith";
	case 11:   return "Hunter";
	case 12:   return "Assassin";
	case 13:   return "Peco Knight";
	case 14:   return "Crusader";
	case 15:   return "Monk";
	case 16:   return "Sage";
	case 17:   return "Rogue";
	case 18:   return "Alchemist";
	case 19:   return "Bard";
	case 20:   return "Dancer";
	case 21:   return "Peco Crusader";
	case 22:   return "Wedding";
	case 23:   return "Super Novice";
	case 4001: return "Novice High";
	case 4002: return "Swordsman High";
	case 4003: return "Mage High";
	case 4004: return "Archer High";
	case 4005: return "Acolyte High";
	case 4006: return "Merchant High";
	case 4007: return "Thief High";
	case 4008: return "Lord Knight";
	case 4009: return "High Priest";
	case 4010: return "High Wizard";
	case 4011: return "Whitesmith";
	case 4012: return "Sniper";
	case 4013: return "Assassin Cross";
	case 4014: return "Peko Knight";
	case 4015: return "Paladin";
	case 4016: return "Champion";
	case 4017: return "Professor";
	case 4018: return "Stalker";
	case 4019: return "Creator";
	case 4020: return "Clown";
	case 4021: return "Gypsy";
	case 4022: return "Peko Paladin";
	case 4023: return "Baby Novice";
	case 4024: return "Baby Swordsman";
	case 4025: return "Baby Mage";
	case 4026: return "Baby Archer";
	case 4027: return "Baby Acolyte";
	case 4028: return "Baby Merchant";
	case 4029: return "Baby Thief";
	case 4030: return "Baby Knight";
	case 4031: return "Baby Priest";
	case 4032: return "Baby Wizard";
	case 4033: return "Baby Blacksmith";
	case 4034: return "Baby Hunter";
	case 4035: return "Baby Assassin";
	case 4036: return "Baby Peco Knight";
	case 4037: return "Baby Crusader";
	case 4038: return "Baby Monk";
	case 4039: return "Baby Sage";
	case 4040: return "Baby Rogue";
	case 4041: return "Baby Alchemist";
	case 4042: return "Baby Bard";
	case 4043: return "Baby Dancer";
	case 4044: return "Baby Peco Crusader";
	case 4045: return "Super Baby";
	case 4046: return "Taekwon";
	case 4047: return "Star Gladiator";
	case 4048: return "Flying Star Gladiator";
	case 4049: return "Soul Linker";
	}
	return "Unknown Job";
}
///////////////////////////////////////////////////////////////////////////////
/// jobname to class_id conversion
unsigned short job_id(const char *jobname)
{
	const struct 
	{
		const char *name;
		int id; 
	} jobs[] =
	{
		{ "novice",				0 },
		{ "swordsman",			1 },
		{ "mage",				2 },
		{ "archer",				3 },
		{ "acolyte",			4 },
		{ "merchant",			5 },
		{ "thief",				6 },
		{ "knight",				7 },
		{ "priest",				8 },
		{ "priestess",			8 },
		{ "wizard",				9 },
		{ "blacksmith",			10 },
		{ "hunter",				11 },
		{ "assassin",			12 },
		{ "peco knight",		13 },
		{ "crusader",			14 },
		{ "monk",				15 },
		{ "sage",				16 },
		{ "rogue",				17 },
		{ "alchemist",			18 },
		{ "bard",				19 },
		{ "dancer",				20 },
		{ "peco crusader",		21 },
		{ "wedding",			22 },
		{ "super novice",		23 },
		{ "supernovice",		23 },
		{ "high novice",		4001 },
		{ "swordsman high",		4002 },
		{ "mage high",			4003 },
		{ "archer high",		4004 },
		{ "acolyte high",		4005 },
		{ "merchant high",		4006 },
		{ "thief high",			4007 },
		{ "lord knight",		4008 },
		{ "high priest",		4009 },
		{ "high priestess",		4009 },
		{ "high wizard",		4010 },
		{ "whitesmith",			4011 },
		{ "sniper",				4012 },
		{ "assassin cross",		4013 },
		{ "high peko knight",	4014 },
		{ "paladin",			4015 },
		{ "champion",			4016 },
		{ "professor",			4017 },
		{ "stalker",			4018 },
		{ "creator",			4019 },
		{ "clown",				4020 },
		{ "gypsy",				4021 },
		{ "peko paladin",		4022 },
		{ "baby novice",		4023 },
		{ "baby swordsman",		4024 },
		{ "baby mage",			4025 },
		{ "baby archer",		4026 },
		{ "baby acolyte",		4027 },
		{ "baby merchant",		4028 },
		{ "baby thief",			4029 },
		{ "baby knight",		4030 },
		{ "baby priest",		4031 },
		{ "baby priestess",		4031 },
		{ "baby wizard",		4032 },
		{ "baby blacksmith",	4033 },
		{ "baby hunter",		4034 },
		{ "baby assassin",		4035 },
		{ "baby peco knight",	4036 },
		{ "baby crusader",		4037 },
		{ "baby monk",			4038 },
		{ "baby sage",			4039 },
		{ "baby rogue",			4040 },
		{ "baby alchemist",		4041 },
		{ "baby bard",			4042 },
		{ "baby dancer",		4043 },
		{ "baby peco crusader",	4044 },
		{ "super baby",			4045 },
		{ "taekwon",			4046 },
		{ "taekwon boy",		4046 },
		{ "taekwon girl",		4046 },
		{ "star gladiator",		4047 },
		{ "flying star gladiator",4048 },
		{ "soul linker",		4049 },
	};

	if(jobname && *jobname)
	{
		size_t i;
		size_t len = strlen(jobname);
		for (i=0; i<(sizeof(jobs)/sizeof(*jobs)); ++i)
		{
			if( 0==strncasecmp(jobname, jobs[i].name, len) )
				return jobs[i].id;
		}
	}
	return 0xFFFF;
}

///////////////////////////////////////////////////////////////////////////////
/// is valid jobid
bool job_isvalid(unsigned short class_)
{
	return class_<=23 || (class_>=4001 && class_<=4049);
}


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
// CCharCharacter Class
////////////////////////////////////////////////////
void CCharCharacter::_tobuffer(unsigned char* &buf, bool new_charscreen) const
{// Used to build packet 0x6b (existing chars) and packet 0x6d (new char)
	if(buf)
	{
		_L_tobuffer( char_id, buf );
		_L_tobuffer( base_exp, buf );
		_L_tobuffer( zeny, buf );
		_L_tobuffer( job_exp, buf );
		_L_tobuffer( job_level, buf );

		_L_tobuffer( 0, buf);// probably opt1
		_L_tobuffer( 0, buf);// probably opt2
		_L_tobuffer( option, buf);

		_L_tobuffer( (uint32)karma, buf);
		_L_tobuffer( (sint32)manner, buf);

		_W_tobuffer( status_point, buf );
		_W_tobuffer( ushort((hp > 0x7fff) ? 0x7fff : hp), buf );
		_W_tobuffer( ushort((max_hp > 0x7fff) ? 0x7fff : max_hp), buf );
		_W_tobuffer( ushort((sp > 0x7fff) ? 0x7fff : sp), buf );
		_W_tobuffer( ushort((max_sp > 0x7fff) ? 0x7fff : max_sp), buf );
		_W_tobuffer( ushort(DEFAULT_WALK_SPEED), buf );
		_W_tobuffer( class_, buf );
		_W_tobuffer( hair, buf );

		// pecopeco knights/crusaders crash fix
		if (class_ == 13 || class_ == 21 ||
			class_ == 4014 || class_ == 4022 ||
			class_ == 4036 || class_ == 4044)
			_W_tobuffer( ushort(0), buf );
		else 
			_W_tobuffer( weapon, buf );

		_W_tobuffer( base_level, buf );
		_W_tobuffer( skill_point, buf );
		_W_tobuffer( head_bottom, buf );
		_W_tobuffer( shield, buf );
		_W_tobuffer( head_top, buf );
		_W_tobuffer( head_mid, buf );
		_W_tobuffer( hair_color, buf );
		_W_tobuffer( clothes_color, buf );

		_S_tobuffer( name, buf, 24 );

		_B_tobuffer( uchar((str > 255) ? 255 : str), buf );
		_B_tobuffer( uchar((agi > 255) ? 255 : agi), buf );
		_B_tobuffer( uchar((vit > 255) ? 255 : vit), buf );
		_B_tobuffer( uchar((int_ > 255) ? 255 : int_), buf );
		_B_tobuffer( uchar((dex > 255) ? 255 : dex), buf );
		_B_tobuffer( uchar((luk > 255) ? 255 : luk), buf );

		_W_tobuffer( ushort(slot), buf );
		if( new_charscreen )
			_W_tobuffer( ushort(1), buf );
	}
}


////////////////////////////////////////////////////
// struct item compare
////////////////////////////////////////////////////
bool operator==(const struct item& a, const struct item& b)
{
	return ( (a.nameid == b.nameid) &&
			 (a.equip == b.equip) &&
			 (a.identify == b.identify) &&
			 (a.refine == b.refine) &&
			 (a.attribute == b.attribute) &&
			 (a.card[0] == b.card[0]) &&
			 (a.card[1] == b.card[1]) &&
			 (a.card[2] == b.card[2]) &&
			 (a.card[3] == b.card[3]) );
}
bool operator< (const struct item& a, const struct item& b)
{
	return	(a.nameid < b.nameid);
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

