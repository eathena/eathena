// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/socket.h"
#include "../common/showmsg.h"
#include "char.h"
#include "chardb.h"
#include "inter.h"
#include "int_party.h"
#include "online.h"
#include "partydb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// party database
static PartyDB* parties = NULL;

// local data cache for active parties
static DBMap* cache = NULL; // int party_id -> struct party_data*


// external imports
extern CharServerDB* charserver;


// forward declarations
void int_party_calc_state(struct party_data *p);
int party_check_exp_share(struct party_data *p);
void mapif_party_optionchanged(int fd, struct party *p, int account_id, int flag);
void mapif_party_broken(int party_id, int flag);


/// Retrieves entire party data from database and caches it for subsequent use.
/// @param party_id id of the party to load
/// @return pointer to the requested party data in cache, or NULL
static struct party_data* int_party_load(int party_id)
{
	struct party_data* p;
	CharDB* chars;
	struct mmo_charstatus cd;
	struct online_char_data* online;
	int i;

	// check cache for existing data
	p = idb_get(cache, party_id);
	if( p != NULL )
		return p; // already cached

	// allocate memory for result
	p = (struct party_data*)aMalloc(sizeof(struct party_data));
	if( p == NULL )
		return NULL;

	// fetch base data from PartyDB
	if( !parties->load(parties, p, party_id) )
	{
		aFree(p);
		return NULL;
	}

	// fetch party member data from CharDB and onlinedb
	chars = charserver->chardb(charserver);
	for( i = 0; i < MAX_PARTY; ++i )
	{
		struct party_member* m = &p->party.member[i];

		if( m->char_id == 0 || !chars->load_num(chars, &cd, m->char_id) || cd.account_id != m->account_id )
			continue;

		online = onlinedb_get(m->account_id);

		safestrncpy(m->name, cd.name, sizeof(m->name));
		m->class_ = cd.class_;
		m->map = cd.last_point.map;
		m->lv = cd.base_level;
		m->online = ( online && online->char_id == m->char_id ) ? 1 : 0;
	}

	// recalculate derived values
	int_party_calc_state(p);

	// cache the data for followup requests
	idb_put(cache, party_id, p);

	return p;
}


//Calculates the state of a party.
void int_party_calc_state(struct party_data *p)
{
	int i;
	unsigned int lv;
	p->min_lv = UINT_MAX;
	p->max_lv = 0;
	p->party.count =
	p->size =
	p->family = 0;

	//Check party size
	for(i=0;i<MAX_PARTY;i++){
		if (!p->party.member[i].lv) continue;
		p->size++;
		if(p->party.member[i].online)
			p->party.count++;
	}

	if(p->size == 3) {
		//Check Family State.
		p->family = char_family(
			p->party.member[0].char_id,
			p->party.member[1].char_id,
			p->party.member[2].char_id
		);
	}

	//max/min levels.
	for( i = 0; i < MAX_PARTY; i++ )
	{
		lv = p->party.member[i].lv;
		if( !lv ) continue;

		if( !p->party.member[i].online &&
			p->party.member[i].char_id == p->family ) //On families, the kid is not counted towards exp share rules.
			continue;

		if( lv < p->min_lv ) p->min_lv = lv;
		if( p->max_lv < lv ) p->max_lv = lv;
	}

	if (p->party.exp && !party_check_exp_share(p)) {
		p->party.exp = 0; //Set off even share.
		mapif_party_optionchanged(0, &p->party, 0, 0);
	}
}

// Returns whether this party can keep having exp share or not.
int party_check_exp_share(struct party_data *p)
{
	return( p->party.count < 2 || p->max_lv - p->min_lv <= party_share_level );
}

// Is there any member in the party?
bool party_check_empty(struct party_data *p)
{
	int i;

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id > 0 );
	return ( i == MAX_PARTY );
}


//-------------------------------------------------------------------
// map serverへの通信

/// Party creation notification.
/// @param result 0 on success, 1 on failure
static void mapif_party_created(int fd, int account_id, int char_id, int result, int party_id, const char* name)
{
	WFIFOHEAD(fd, 39);
	WFIFOW(fd,0) = 0x3820;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = char_id;
	WFIFOB(fd,10) = result;
	WFIFOL(fd,11) = party_id;
	safestrncpy((char*)WFIFOP(fd,15), name, NAME_LENGTH);
	WFIFOSET(fd,39);
}

// パーティ情報見つからず
void mapif_party_noinfo(int fd, int party_id)
{
	WFIFOHEAD(fd, 8);
	WFIFOW(fd,0) = 0x3821;
	WFIFOW(fd,2) = 8;
	WFIFOL(fd,4) = party_id;
	WFIFOSET(fd,8);
	ShowWarning("int_party: info not found %d\n", party_id);
}

// パーティ情報まとめ送り
void mapif_party_info(int fd, struct party *p)
{
	unsigned char buf[4+sizeof(struct party)];
	WBUFW(buf,0) = 0x3821;
	WBUFW(buf,2) = 4 + sizeof(struct party);
	memcpy(buf + 4, p, sizeof(struct party));

	if( fd < 0 )
		mapif_sendall(buf, WBUFW(buf,2));
	else
		mapif_send(fd, buf, WBUFW(buf,2));
}

// パーティメンバ追加可否
void mapif_party_memberadded(int fd, int party_id, int account_id, int char_id, int flag)
{
	WFIFOHEAD(fd, 15);
	WFIFOW(fd,0) = 0x3822;
	WFIFOL(fd,2) = party_id;
	WFIFOL(fd,6) = account_id;
	WFIFOL(fd,10) = char_id;
	WFIFOB(fd,14) = flag;
	WFIFOSET(fd,15);
}

// パーティ設定変更通知
void mapif_party_optionchanged(int fd, struct party *p, int account_id, int flag)
{
	unsigned char buf[15];
	WBUFW(buf, 0) = 0x3823;
	WBUFL(buf, 2) = p->party_id;
	WBUFL(buf, 6) = account_id;
	WBUFW(buf,10) = p->exp;
	WBUFW(buf,12) = p->item;
	WBUFB(buf,14) = flag;
	if( flag == 0 )
		mapif_sendall(buf, 15);
	else
		mapif_send(fd, buf, 15);
}

// パーティ脱退通知
void mapif_party_leaved(int party_id,int account_id, int char_id)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x3824;
	WBUFL(buf,2) = party_id;
	WBUFL(buf,6) = account_id;
	WBUFL(buf,10) = char_id;
	mapif_sendall(buf, 14);
}

// パーティマップ更新通知
static void mapif_party_membermoved(int party_id, int account_id, int char_id, unsigned short map, bool online, unsigned short lv)
{
	unsigned char buf[20];

	WBUFW(buf, 0) = 0x3825;
	WBUFL(buf, 2) = party_id;
	WBUFL(buf, 6) = account_id;
	WBUFL(buf,10) = char_id;
	WBUFW(buf,14) = map;
	WBUFB(buf,16) = online;
	WBUFW(buf,17) = lv;
	mapif_sendall(buf, 19);
}

// パーティ解散通知
void mapif_party_broken(int party_id, int flag)
{
	unsigned char buf[7];
	WBUFW(buf,0) = 0x3826;
	WBUFL(buf,2) = party_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
}

// パーティ内発言
void mapif_party_message(int party_id, int account_id, char* mes, int len, int sfd)
{
	unsigned char buf[512]; //FIXME: use appropriate size
	len = min(len, sizeof(buf)-12);

	WBUFW(buf,0) = 0x3827;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = party_id;
	WBUFL(buf,8) = account_id;
	safestrncpy((char*)WBUFP(buf,12), mes, len);
	mapif_sendallwos(sfd, buf, len + 12);
}


//-------------------------------------------------------------------
// map serverからの通信


// Create Party
static void mapif_parse_CreateParty(int fd, char *name, int item, int item2, struct party_member *leader)
{
	struct party_data* p;
	int i;

#ifdef TXT_ONLY
	//FIXME: this should be removed once the savefiles can handle all symbols
	for(i = 0; i < NAME_LENGTH && name[i]; i++) {
		if (!(name[i] & 0xe0) || name[i] == 0x7f) {
			ShowInfo("int_party: illegal party name [%s]\n", name);
			mapif_party_created(fd, leader->account_id, leader->char_id, 1, 0, "");
			return;
		}
	}
#endif

	// Check Authorised letters/symbols in the name of the party
	if (char_config.char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_config.char_name_letters, name[i]) == NULL) {
				mapif_party_created(fd, leader->account_id, leader->char_id, 1, 0, "");
				return;
			}
	} else if (char_config.char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_config.char_name_letters, name[i]) != NULL) {
				mapif_party_created(fd, leader->account_id, leader->char_id, 1, 0, "");
				return;
			}
	}

	// check the availability of this party name
	if( parties->name2id(parties, NULL, name) )
	{
		mapif_party_created(fd, leader->account_id, leader->char_id, 1, 0, "");
		return;
	}

	// prepare new party data
	CREATE(p, struct party_data, 1);
	p->party.party_id = -1; // automatic id generation
	safestrncpy(p->party.name, name, NAME_LENGTH);
	p->party.exp = 0;
	p->party.item =( item?1:0)|(item2?2:0);
	memcpy(&p->party.member[0], leader, sizeof(struct party_member));
	p->party.member[0].leader = 1;

	if( !parties->create(parties, p) )
	{// failed to create party
		mapif_party_created(fd, leader->account_id, leader->char_id, 1, 0, "");
		return;
	}

	int_party_calc_state(p);
	idb_put(cache, p->party.party_id, p);

	mapif_party_created(fd, leader->account_id, leader->char_id, 0, p->party.party_id, p->party.name);
	mapif_party_info(fd, &p->party);
}


// パーティ情報要求
static void mapif_parse_PartyInfo(int fd, int party_id)
{
	struct party_data* p = int_party_load(party_id);

	if( p != NULL )
		mapif_party_info(fd, &p->party);
	else
		mapif_party_noinfo(fd, party_id);
}


// パーティ追加要求	
static void mapif_parse_PartyAddMember(int fd, int party_id, struct party_member *member)
{
	struct party_data* p = int_party_load(party_id);
	int i;

	if( p == NULL )
	{// party doesn't exist
		mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 1);
		return;
	}

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == 0 );
	if( i == MAX_PARTY )
	{// Party full
		mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 1);
		return;
	}

	memcpy(&p->party.member[i], member, sizeof(struct party_member));
	p->party.member[i].leader = 0;
	parties->save(parties, p, PS_ADDMEMBER, i);

	int_party_calc_state(p);
	mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 0);
	mapif_party_info(-1, &p->party);
}


// パーティー設定変更要求
static void mapif_parse_PartyChangeOption(int fd, int party_id, int account_id, int exp, int item)
{
	struct party_data* p = int_party_load(party_id);
	int flag = 0;

	if( p == NULL )
		return;

	p->party.exp = exp;
	if( exp > 0 && !party_check_exp_share(p) )
	{
		flag |= 0x01;
		p->party.exp = 0;
	}

	p->party.item = item&0x3; //Filter out invalid values.

	parties->save(parties, p, PS_BASIC, -1);

	mapif_party_optionchanged(fd, &p->party, account_id, flag);
}


// パーティ脱退要求
static void mapif_parse_PartyLeave(int fd, int party_id, int account_id, int char_id)
{
	struct party_data* p = int_party_load(party_id);
	int i,j=-1;
	bool leader;

	if( p == NULL )
	{// Party does not exist
		mapif_party_noinfo(fd, party_id); //TODO: check if this is right
		return;
	}

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == account_id && p->party.member[i].char_id == char_id );
	if( i == MAX_PARTY )
		return; //Member not found?

	leader = p->party.member[i].leader;

	if( leader && party_break_without_leader )
	{// kick all members from party
		for( j = 0; j < MAX_PARTY; j++ )
		{
			if( p->party.member[j].account_id != 0 )
			{
				mapif_party_leaved(party_id, p->party.member[j].account_id, p->party.member[j].char_id);
				p->party.member[j].account_id = 0;
			}
		}
	}
	else
	{// only remove self
		mapif_party_leaved(party_id, account_id, char_id);
		memset(&p->party.member[i], 0, sizeof(struct party_member));
	}

	if( leader && party_auto_reassign_leader )
	{// grant leadership to another party member
		ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id != 0 && p->party.member[i].online );
		if( i == MAX_PARTY )
			ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id != 0 );

		if( i < MAX_PARTY )
			p->party.member[i].leader = 1;
	}

	if( !party_check_empty(p) )
	{
		parties->save(parties, p, PS_DELMEMBER, i);
		int_party_calc_state(p);
		mapif_party_info(-1, &p->party);
	}
	else
	{// no members, break the party
		parties->remove(parties, party_id);
		idb_remove(cache, party_id);
		mapif_party_broken(party_id, 0);
	}
}


// When member goes to other map, logs in/out, or changes base level.
static void mapif_parse_PartyChangeMap(int fd, int party_id, int account_id, int char_id, unsigned short map, bool online, unsigned short lv)
{
	struct party_data* p = int_party_load(party_id);
	int i;

	if( p == NULL )
		return;

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == account_id && p->party.member[i].char_id == char_id );
	if( i == MAX_PARTY )
		return;

	if( p->party.member[i].map != map )
	{
		p->party.member[i].map = map;
	}
	if( p->party.member[i].online != online )
	{
		p->party.member[i].online = online;
		int_party_calc_state(p);
	}
	if( p->party.member[i].lv != lv )
	{
		p->party.member[i].lv = lv;
		int_party_calc_state(p);
	}

	if( p->party.count == 0 ) // all members offline
		idb_remove(cache, party_id);

	mapif_party_membermoved(party_id, account_id, char_id, map, online, lv);
}


// パーティ解散要求
static void mapif_parse_BreakParty(int fd, int party_id)
{
	struct party_data* p = int_party_load(party_id);

	if( p == NULL )
		return;

	parties->remove(parties, party_id);
	idb_remove(cache, party_id);
	mapif_party_broken(fd, party_id);
}


// パーティメッセージ送信
static void mapif_parse_PartyMessage(int fd, int party_id, int account_id, char *mes, int len)
{
	mapif_party_message(party_id, account_id, mes, len, fd);
}


static void mapif_parse_PartyLeaderChange(int fd, int party_id, int account_id, int char_id)
{
	struct party_data* p = int_party_load(party_id);
	int i;

	if( p == NULL )
		return;

	// remove old leader flag
	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].leader == 1 );
	if( i < MAX_PARTY )
		p->party.member[i].leader = 0;

	// set new leader flag
	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == account_id && p->party.member[i].char_id == char_id );
	if( i < MAX_PARTY )
		p->party.member[i].leader = 1;

	parties->save(parties, p, PS_LEADER, i);
}


// communication with map-server
int inter_party_parse_frommap(int fd)
{
	switch( RFIFOW(fd,0) )
	{
	case 0x3020: mapif_parse_CreateParty(fd, (char*)RFIFOP(fd,4), RFIFOB(fd,28), RFIFOB(fd,29), (struct party_member*)RFIFOP(fd,30)); break;
	case 0x3021: mapif_parse_PartyInfo(fd, RFIFOL(fd,2)); break;
	case 0x3022: mapif_parse_PartyAddMember(fd, RFIFOL(fd,4), (struct party_member*)RFIFOP(fd,8)); break;
	case 0x3023: mapif_parse_PartyChangeOption(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOW(fd,10), RFIFOW(fd,12)); break;
	case 0x3024: mapif_parse_PartyLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x3025: mapif_parse_PartyChangeMap(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOW(fd,14), (bool)RFIFOB(fd,16), RFIFOW(fd,17)); break;
	case 0x3026: mapif_parse_BreakParty(fd, RFIFOL(fd,2)); break;
	case 0x3027: mapif_parse_PartyMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3029: mapif_parse_PartyLeaderChange(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	default:
		return 0;
	}

	return 1;
}

void inter_party_leave(int party_id, int account_id, int char_id)
{
	mapif_parse_PartyLeave(-1, party_id, account_id, char_id);
}

void inter_party_init(PartyDB* db)
{
	parties = db;
	cache = idb_alloc(DB_OPT_RELEASE_DATA);
}

void inter_party_final(void)
{
	parties = NULL;
	db_destroy(cache);
}
