// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/mapindex.h"
#include "../common/utils.h"

#include "../char/charserverdb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_CONF_NAME "conf/char_athena.conf"
#define SQL_CONF_NAME "conf/inter_athena.conf"
#define INTER_CONF_NAME "conf/inter_athena.conf"

CharServerDB* txtdb = NULL;
CharServerDB* sqldb = NULL;

//--------------------------------------------------------

int convert_char(void)
{
	if( !txtdb->init(txtdb) || !sqldb->init(sqldb) )
	{
		ShowFatalError("Initialization failed, unable to start conversion.\n");
		return 0;
	}

	ShowStatus("Conversion started...\n");
	//TODO: do some counting & statistics

	{// convert chardb
		CharDB* txt = txtdb->chardb(txtdb);
		CharDB* sql = sqldb->chardb(sqldb);
		CharDBIterator* iter = txt->iterator(txt);
		struct mmo_charstatus data;

		ShowStatus("Converting Character Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert accregs
		AccRegDB* txt = txtdb->accregdb(txtdb);
		AccRegDB* sql = sqldb->accregdb(sqldb);
		AccRegDBIterator* iter = txt->iterator(txt);
		struct regs data;
		int key;

		ShowStatus("Converting Account variables Data...\n");

		while( iter->next(iter, &data, &key) )
			if( !sql->save(sql, &data, key) )
				;

		iter->destroy(iter);
	}

	{// convert charregs
		CharRegDB* txt = txtdb->charregdb(txtdb);
		CharRegDB* sql = sqldb->charregdb(sqldb);
		CharRegDBIterator* iter = txt->iterator(txt);
		struct regs data;
		int key;

		ShowStatus("Converting Character variables Data...\n");

		while( iter->next(iter, &data, &key) )
			if( !sql->save(sql, &data, key) )
				;

		iter->destroy(iter);
	}

	{// convert storage
		StorageDB* txt = txtdb->storagedb(txtdb);
		StorageDB* sql = sqldb->storagedb(sqldb);
		StorageDBIterator* iter = txt->iterator(txt);
		struct storage_data data;
		int key;

		ShowStatus("Converting Storage Data...\n");

		while( iter->next(iter, &data, &key) )
			if( !sql->save(sql, &data, key) )
				;

		iter->destroy(iter);
	}

	{// convert status data
		StatusDB* txt = txtdb->statusdb(txtdb);
		StatusDB* sql = sqldb->statusdb(sqldb);
		StatusDBIterator* iter = txt->iterator(txt);
		struct scdata data;

		ShowStatus("Converting Status Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->save(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert pets
		PetDB* txt = txtdb->petdb(txtdb);
		PetDB* sql = sqldb->petdb(sqldb);
		PetDBIterator* iter = txt->iterator(txt);
		struct s_pet data;

		ShowStatus("Converting Pet Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert homunculi
		HomunDB* txt = txtdb->homundb(txtdb);
		HomunDB* sql = sqldb->homundb(sqldb);
		HomunDBIterator* iter = txt->iterator(txt);
		struct s_homunculus data;

		ShowStatus("Converting Homunculus Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert friends
		FriendDB* txt = txtdb->frienddb(txtdb);
		FriendDB* sql = sqldb->frienddb(sqldb);
		FriendDBIterator* iter = txt->iterator(txt);
		friendlist data;
		int key;

		ShowStatus("Converting Friend Data...\n");

		while( iter->next(iter, &data, &key) )
			if( !sql->save(sql, &data, key) )
				;

		iter->destroy(iter);
	}

	//FIXME: partydb isn't able to save multiple members at once
	{// convert parties
		PartyDB* txt = txtdb->partydb(txtdb);
		PartyDB* sql = sqldb->partydb(sqldb);
		PartyDBIterator* iter = txt->iterator(txt);
		struct party_data data;

		ShowStatus("Converting Party Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	//FIXME: guilddb isn't able to save multiple members at once
	{// convert guilds
		GuildDB* txt = txtdb->guilddb(txtdb);
		GuildDB* sql = sqldb->guilddb(sqldb);
		GuildDBIterator* iter = txt->iterator(txt);
		struct guild data;

		ShowStatus("Converting Guild Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert castles
		CastleDB* txt = txtdb->castledb(txtdb);
		CastleDB* sql = sqldb->castledb(sqldb);
		CastleDBIterator* iter = txt->iterator(txt);
		struct guild_castle data;

		ShowStatus("Converting Castle Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert guild storages
		GuildStorageDB* txt = txtdb->guildstoragedb(txtdb);
		GuildStorageDB* sql = sqldb->guildstoragedb(sqldb);
		GuildStorageDBIterator* iter = txt->iterator(txt);
		struct guild_storage data;
		int key;

		ShowStatus("Converting Guild Storage Data...\n");

		while( iter->next(iter, &data, &key) )
			if( !sql->save(sql, &data, key) )
				;

		iter->destroy(iter);
	}

	{// convert hotkeys
		HotkeyDB* txt = txtdb->hotkeydb(txtdb);
		HotkeyDB* sql = sqldb->hotkeydb(sqldb);
		HotkeyDBIterator* iter = txt->iterator(txt);
		hotkeylist data;
		int key;

		ShowStatus("Converting Hotkey Data...\n");

		while( iter->next(iter, &data, &key) )
			if( !sql->save(sql, &data, key) )
				;

		iter->destroy(iter);
	}
/*
	//FIXME: maildb needs a 'saveall' operation to support conversion
	{// convert mails
		MailDB* txt = txtdb->auctiondb(txtdb);
		MailDB* sql = sqldb->auctiondb(sqldb);
		MailDBIterator* iter = txt->iterator(txt);
		struct mail_data data;

		ShowStatus("Converting Mail Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert auctions
		AuctionDB* txt = txtdb->auctiondb(txtdb);
		AuctionDB* sql = sqldb->auctiondb(sqldb);
		AuctionDBIterator* iter = txt->iterator(txt);
		struct auction_data data;

		ShowStatus("Converting Auction Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert quests
		QuestDB* txt = txtdb->questdb(txtdb);
		QuestDB* sql = sqldb->questdb(sqldb);
		QuestDBIterator* iter = txt->iterator(txt);
		struct questlog data;

		ShowStatus("Converting Quest Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}

	{// convert rankings
		RankDB* txt = txtdb->rankdb(txtdb);
		RankDB* sql = sqldb->rankdb(sqldb);
		RankDBIterator* iter = txt->iterator(txt);
		struct questlog data;

		ShowStatus("Converting Ranking Data...\n");

		while( iter->next(iter, &data) )
			if( !sql->create(sql, &data) )
				;

		iter->destroy(iter);
	}
*/

	ShowStatus("Everything's been converted!\n");

	return 0;
}

int do_init(int argc, char** argv)
{
	int input;

	txtdb = charserver_db_txt();
	sqldb = charserver_db_sql();

//	char_config_read( (argc > 1) ? argv[1] : CHAR_CONF_NAME);
	mapindex_init();
//	sql_config_read( (argc > 2) ? argv[2] : SQL_CONF_NAME);
//	inter_init_txt( (argc > 3) ? argv[3] : INTER_CONF_NAME);
//	inter_init_sql( (argc > 3) ? argv[3] : INTER_CONF_NAME);

	ShowWarning("Make sure you backup your databases before continuing!\n");
	ShowMessage("\n");
	ShowNotice("Do you wish to convert your data to SQL? (y/n) : ");
	input = getchar();

	if(input == 'y' || input == 'Y')
		convert_char();

	return 0;
}

void do_final(void)
{
	mapindex_final();
	txtdb->destroy(txtdb);
	sqldb->destroy(sqldb);
}
