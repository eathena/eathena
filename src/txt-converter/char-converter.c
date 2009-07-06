// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
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
#define INTER_CONF_NAME "conf/inter_athena.conf"

CharServerDB* txtdb = NULL;
CharServerDB* sqldb = NULL;

//--------------------------------------------------------

int convert_char(void)
{
	CharServerDB* srcdb = txtdb;
	CharServerDB* dstdb = sqldb;

	if( !txtdb->init(txtdb) || !sqldb->init(sqldb) )
	{
		ShowFatalError("Initialization failed, unable to start conversion.\n");
		return 0;
	}

	ShowStatus("Conversion started...\n");
	//TODO: do some counting & statistics

	{// convert chardb
		CharDB* txt = srcdb->chardb(srcdb);
		CharDB* sql = dstdb->chardb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct mmo_charstatus data;
		int key;

		ShowStatus("Converting Character Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load_num(txt, &data, key);
			sql->create(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert accregs
		AccRegDB* txt = srcdb->accregdb(srcdb);
		AccRegDB* sql = dstdb->accregdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct regs data;
		int key;

		ShowStatus("Converting Account variables Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert charregs
		CharRegDB* txt = srcdb->charregdb(srcdb);
		CharRegDB* sql = dstdb->charregdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct regs data;
		int key;

		ShowStatus("Converting Character variables Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert storage
		StorageDB* txt = srcdb->storagedb(srcdb);
		StorageDB* sql = dstdb->storagedb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct storage_data data;
		int key;

		ShowStatus("Converting Storage Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert status data
		StatusDB* txt = srcdb->statusdb(srcdb);
		StatusDB* sql = dstdb->statusdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct scdata data;
		int key;

		ShowStatus("Converting Status Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			data.account_id = -1;
			sql->save(sql, &data, key);
			if( data.data != NULL )
				aFree(data.data);
		}

		iter->destroy(iter);
	}

	{// convert pets
		PetDB* txt = srcdb->petdb(srcdb);
		PetDB* sql = dstdb->petdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct s_pet data;
		int key;

		ShowStatus("Converting Pet Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->create(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert homunculi
		HomunDB* txt = srcdb->homundb(srcdb);
		HomunDB* sql = dstdb->homundb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct s_homunculus data;
		int key;

		ShowStatus("Converting Homunculus Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->create(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert friends
		FriendDB* txt = srcdb->frienddb(srcdb);
		FriendDB* sql = dstdb->frienddb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		friendlist data;
		int key;

		ShowStatus("Converting Friend Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	//FIXME: partydb isn't able to save multiple members at once
	{// convert parties
		PartyDB* txt = srcdb->partydb(srcdb);
		PartyDB* sql = dstdb->partydb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct party_data data;
		int key;

		ShowStatus("Converting Party Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->create(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert guilds
		GuildDB* txt = srcdb->guilddb(srcdb);
		GuildDB* sql = dstdb->guilddb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct guild data;
		int key;
		int i;

		ShowStatus("Converting Guild Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			for( i = 0; i < data.max_member; ++i )
				data.member[i].modified = GS_MEMBER_MODIFIED;
			for( i = 0; i < MAX_GUILDPOSITION; ++i )
				data.position[i].modified = GS_POSITION_MODIFIED;
			sql->create(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert castles
		CastleDB* txt = srcdb->castledb(srcdb);
		CastleDB* sql = dstdb->castledb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct guild_castle data;
		int key;

		ShowStatus("Converting Castle Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert guild storages
		GuildStorageDB* txt = srcdb->guildstoragedb(srcdb);
		GuildStorageDB* sql = dstdb->guildstoragedb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct guild_storage data;
		int key;

		ShowStatus("Converting Guild Storage Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert hotkeys
		HotkeyDB* txt = srcdb->hotkeydb(srcdb);
		HotkeyDB* sql = dstdb->hotkeydb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		hotkeylist data;
		int key;

		ShowStatus("Converting Hotkey Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert mails
		MailDB* txt = srcdb->maildb(srcdb);
		MailDB* sql = dstdb->maildb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct mail_message data;
		int key;

		ShowStatus("Converting Mail Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->create(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert auctions
		AuctionDB* txt = srcdb->auctiondb(srcdb);
		AuctionDB* sql = dstdb->auctiondb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct auction_data data;
		int key;

		ShowStatus("Converting Auction Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}
/*
	{// convert quests
		QuestDB* txt = srcdb->questdb(srcdb);
		QuestDB* sql = dstdb->questdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		questlog data;
		int key;

		ShowStatus("Converting Quest Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}
*//*
	{// convert rankings
		RankDB* txt = srcdb->rankdb(srcdb);
		RankDB* sql = dstdb->rankdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct questlog data;
		int key;

		ShowStatus("Converting Ranking Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}
*/

	ShowStatus("Everything's been converted!\n");

	return 0;
}

void config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");

	if (fp == NULL) {
		ShowError("Configuration file not found: %s.\n", cfgName);
		return;
	}

	ShowInfo("Reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "import") == 0)
			config_read(w2);
		else
		{
			txtdb->set_property(txtdb, w1, w2);
			sqldb->set_property(sqldb, w1, w2);
		}
	}

	fclose(fp);
	
	ShowInfo("Done reading %s.\n", cfgName);
}


int do_init(int argc, char** argv)
{
	int input;

	txtdb = charserver_db_txt();
	sqldb = charserver_db_sql();

	config_read( (argc > 1) ? argv[1] : CHAR_CONF_NAME);
	config_read( (argc > 2) ? argv[2] : INTER_CONF_NAME);
	mapindex_init();

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
