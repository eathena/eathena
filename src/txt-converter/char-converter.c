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
			sql->create(sql, &data);
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

	{// convert inventory
		StorageDB* txt = srcdb->storagedb(srcdb);
		StorageDB* sql = dstdb->storagedb(dstdb);
		CSDBIterator* iter = txt->iterator(txt, STORAGE_INVENTORY);
		struct item data[MAX_INVENTORY];
		int key;

		ShowStatus("Converting Inventory Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, data, MAX_INVENTORY, STORAGE_INVENTORY, key);
			sql->save(sql, data, MAX_INVENTORY, STORAGE_INVENTORY, key);
		}

		iter->destroy(iter);
	}

	{// convert cart
		StorageDB* txt = srcdb->storagedb(srcdb);
		StorageDB* sql = dstdb->storagedb(dstdb);
		CSDBIterator* iter = txt->iterator(txt, STORAGE_CART);
		struct item data[MAX_CART];
		int key;

		ShowStatus("Converting Cart Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, data, MAX_CART, STORAGE_CART, key);
			sql->save(sql, data, MAX_CART, STORAGE_CART, key);
		}

		iter->destroy(iter);
	}

	{// convert storage
		StorageDB* txt = srcdb->storagedb(srcdb);
		StorageDB* sql = dstdb->storagedb(dstdb);
		CSDBIterator* iter = txt->iterator(txt, STORAGE_KAFRA);
		struct item data[MAX_STORAGE];
		int key;

		ShowStatus("Converting Storage Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, data, MAX_STORAGE, STORAGE_KAFRA, key);
			sql->save(sql, data, MAX_STORAGE, STORAGE_KAFRA, key);
		}

		iter->destroy(iter);
	}

	{// convert skill data
		SkillDB* txt = srcdb->skilldb(srcdb);
		SkillDB* sql = dstdb->skilldb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		skilllist data;
		int key;

		ShowStatus("Converting Skill Data...\n");

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
		struct status_change_data* data;
		int key;

		ShowStatus("Converting Status Data...\n");

		while( iter->next(iter, &key) )
		{
			int count = txt->count(txt, key);
			data = (struct status_change_data*)aMalloc(count * sizeof(*data));

			txt->load(txt, data, count, key);
			sql->save(sql, data, count, key);

			aFree(data);
		}

		iter->destroy(iter);
	}

	{// convert memo data
		MemoDB* txt = srcdb->memodb(srcdb);
		MemoDB* sql = dstdb->memodb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		memolist data;
		int key;

		ShowStatus("Converting Memo Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->save(sql, &data, key);
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
			sql->create(sql, &data);
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
			sql->create(sql, &data);
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

	{// convert parties
		PartyDB* txt = srcdb->partydb(srcdb);
		PartyDB* sql = dstdb->partydb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct party data;
		int key;

		ShowStatus("Converting Party Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->create(sql, &data);
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
			sql->create(sql, &data);
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
			sql->save(sql, &data);
		}

		iter->destroy(iter);
	}

	{// convert guild storages
		StorageDB* txt = srcdb->storagedb(srcdb);
		StorageDB* sql = dstdb->storagedb(dstdb);
		CSDBIterator* iter = txt->iterator(txt, STORAGE_GUILD);
		struct item data[MAX_GUILD_STORAGE];
		int key;

		ShowStatus("Converting Guild Storage Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, data, MAX_GUILD_STORAGE, STORAGE_GUILD, key);
			sql->save(sql, data, MAX_GUILD_STORAGE, STORAGE_GUILD, key);
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
			sql->create(sql, &data);
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
			sql->save(sql, &data);
		}

		iter->destroy(iter);
	}

	{// convert quests
		QuestDB* txt = srcdb->questdb(srcdb);
		QuestDB* sql = dstdb->questdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		questlog data;
		int key;
		int count;

		ShowStatus("Converting Quest Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key, &count);
			sql->save(sql, &data, key);
		}

		iter->destroy(iter);
	}

	{// convert mercenaries
		MercDB* txt = srcdb->mercdb(srcdb);
		MercDB* sql = dstdb->mercdb(dstdb);
		CSDBIterator* iter = txt->iterator(txt);
		struct s_mercenary data;
		int key;

		ShowStatus("Converting Mercenary Data...\n");

		while( iter->next(iter, &key) )
		{
			txt->load(txt, &data, key);
			sql->create(sql, &data);
		}

		iter->destroy(iter);
	}

	{// convert rankings
		RankDB* txt = srcdb->rankdb(srcdb);
		RankDB* sql = dstdb->rankdb(dstdb);
		CSDBIterator* iter;
		int data;
		int key;

		ShowStatus("Converting Ranking Data...\n");

		// convert blacksmith ranking
		iter = txt->iterator(txt, RANK_BLACKSMITH);
		while( iter->next(iter, &key) )
		{
			data = txt->get_points(txt, RANK_BLACKSMITH, key);
			sql->set_points(sql, RANK_BLACKSMITH, key, data);
		}
		iter->destroy(iter);

		// convert alchemist ranking
		iter = txt->iterator(txt, RANK_ALCHEMIST);
		while( iter->next(iter, &key) )
		{
			data = txt->get_points(txt, RANK_ALCHEMIST, key);
			sql->set_points(sql, RANK_ALCHEMIST, key, data);
		}
		iter->destroy(iter);

		// convert taekwon ranking
		iter = txt->iterator(txt, RANK_TAEKWON);
		while( iter->next(iter, &key) )
		{
			data = txt->get_points(txt, RANK_TAEKWON, key);
			sql->set_points(sql, RANK_TAEKWON, key, data);
		}
		iter->destroy(iter);
	}

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
