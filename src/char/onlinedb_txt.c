// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "charserverdb.h"
#include "char.h" // char_config
#include "chardb.h"
#include "online.h"
#include "onlinedb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// internal structure
typedef struct OnlineDB_TXT
{
	// public interface
	OnlineDB vtable;

	// state
	DBMap* onlinedb;
	bool initialized;

	// settings
	char txt_filename[1024];
	char html_filename[1024];
	int sorting_option; // sorting option to display online players in online files
	int display_option; // display options: to know which columns must be displayed
	int refresh_html; // refresh time (in sec) of the html file in the explorer
	int gm_display_min_lv; // minimum GM level to display 'GM' when we want to display it

} OnlineDB_TXT;


// external imports
extern CharServerDB* charserver;


// forward declarations
static void create_online_files(OnlineDB_TXT* db);


static bool online_db_txt_init(OnlineDB* self)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)self;

	if( db->onlinedb == NULL )
		db->onlinedb = idb_alloc(DB_OPT_BASE);
	else
		db_clear(db->onlinedb);

	return true;
}


static void online_db_txt_destroy(OnlineDB* self)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)self;
	
	// delete online players database
	if( db->onlinedb != NULL )
	{
		db_destroy(db->onlinedb);
		db->onlinedb = NULL;
	}

	// delete entire structure
	aFree(db);
}


static bool online_db_txt_sync(OnlineDB* self)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)self;
	create_online_files(db);
	return true;
}


static bool online_db_txt_get_property(OnlineDB* self, const char* key, char* buf, size_t buflen)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)self;

	if( strcmpi(key, "engine.name") == 0 )
		safesnprintf(buf, buflen, "txt");
	else
		return false;

	return true;
}


static bool online_db_txt_set_property(OnlineDB* self, const char* key, const char* value)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)self;

	if( strcmpi(key, "online.txt.txt_filename") == 0 )
		safestrncpy(db->txt_filename, value, sizeof(db->txt_filename));
	else
	if( strcmpi(key, "online.txt.html_filename") == 0 )
		safestrncpy(db->html_filename, value, sizeof(db->html_filename));
	else
	if( strcmpi(key, "online.txt.sorting_option") == 0 )
		db->sorting_option = atoi(value);
	else
	if( strcmpi(key, "online.txt.display_option") == 0 )
		db->display_option = atoi(value);
	else
	if( strcmpi(key, "online.txt.gm_display_min_lv") == 0 )
		db->gm_display_min_lv = atoi(value);
	else
	if( strcmpi(key, "online.txt.refresh_html") == 0 )
	{
		db->refresh_html = atoi(value);
		if( db->refresh_html < 1 )
			db->refresh_html = 1;
	}
	else
		return false;

	return true;
}


bool online_db_txt_set_online(OnlineDB* self, int account_id, int char_id)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)self;
	idb_put(db->onlinedb, account_id, (void*)char_id);
	return true;
}


bool online_db_txt_set_offline(OnlineDB* self, int account_id, int char_id)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)self;

	if( char_id != -1 )
	{
		if( (int)idb_get(db->onlinedb, account_id) == char_id )
			idb_remove(db->onlinedb, account_id);
	}
	else
	if( account_id != -1 )
		idb_remove(db->onlinedb, account_id);
	else
		db_clear(db->onlinedb);

	return true;
}


/// public constructor
OnlineDB* online_db_txt(void)
{
	OnlineDB_TXT* db = (OnlineDB_TXT*)aCalloc(1, sizeof(OnlineDB_TXT));

	// set up the vtable
	db->vtable.init         = &online_db_txt_init;
	db->vtable.destroy      = &online_db_txt_destroy;
	db->vtable.sync         = &online_db_txt_sync;
	db->vtable.get_property = &online_db_txt_get_property;
	db->vtable.set_property = &online_db_txt_set_property;
	db->vtable.set_online   = &online_db_txt_set_online;
	db->vtable.set_offline  = &online_db_txt_set_offline;

	// initial state
	db->onlinedb = NULL;
	db->initialized = false;

	// default settings
	safestrncpy(db->txt_filename, "online.txt", sizeof(db->txt_filename));
	safestrncpy(db->html_filename, "online.html", sizeof(db->html_filename));
	db->sorting_option = 0; // no sorting
	db->display_option = 1; // just names
	db->refresh_html = 20;
	db->gm_display_min_lv = 20;

	return &db->vtable;
}





//----------------------------------------------------
// This function return the name of the job (by [Yor])
//----------------------------------------------------
char * job_name(int class_)
{
	switch (class_) {
	case JOB_NOVICE:    return "Novice";
	case JOB_SWORDMAN:    return "Swordsman";
	case JOB_MAGE:    return "Mage";
	case JOB_ARCHER:    return "Archer";
	case JOB_ACOLYTE:    return "Acolyte";
	case JOB_MERCHANT:    return "Merchant";
	case JOB_THIEF:    return "Thief";
	case JOB_KNIGHT:    return "Knight";
	case JOB_PRIEST:    return "Priest";
	case JOB_WIZARD:    return "Wizard";
	case JOB_BLACKSMITH:   return "Blacksmith";
	case JOB_HUNTER:   return "Hunter";
	case JOB_ASSASSIN:   return "Assassin";
	case JOB_KNIGHT2:   return "Peco-Knight";
	case JOB_CRUSADER:   return "Crusader";
	case JOB_MONK:   return "Monk";
	case JOB_SAGE:   return "Sage";
	case JOB_ROGUE:   return "Rogue";
	case JOB_ALCHEMIST:   return "Alchemist";
	case JOB_BARD:   return "Bard";
	case JOB_DANCER:   return "Dancer";
	case JOB_CRUSADER2:   return "Peco-Crusader";
	case JOB_WEDDING:   return "Wedding";
	case JOB_SUPER_NOVICE:   return "Super Novice";
	case JOB_GUNSLINGER: return "Gunslinger";
	case JOB_NINJA: return "Ninja";
	case JOB_XMAS: return "Christmas";
	case JOB_NOVICE_HIGH: return "Novice High";
	case JOB_SWORDMAN_HIGH: return "Swordsman High";
	case JOB_MAGE_HIGH: return "Mage High";
	case JOB_ARCHER_HIGH: return "Archer High";
	case JOB_ACOLYTE_HIGH: return "Acolyte High";
	case JOB_MERCHANT_HIGH: return "Merchant High";
	case JOB_THIEF_HIGH: return "Thief High";
	case JOB_LORD_KNIGHT: return "Lord Knight";
	case JOB_HIGH_PRIEST: return "High Priest";
	case JOB_HIGH_WIZARD: return "High Wizard";
	case JOB_WHITESMITH: return "Whitesmith";
	case JOB_SNIPER: return "Sniper";
	case JOB_ASSASSIN_CROSS: return "Assassin Cross";
	case JOB_LORD_KNIGHT2: return "Peko Knight";
	case JOB_PALADIN: return "Paladin";
	case JOB_CHAMPION: return "Champion";
	case JOB_PROFESSOR: return "Professor";
	case JOB_STALKER: return "Stalker";
	case JOB_CREATOR: return "Creator";
	case JOB_CLOWN: return "Clown";
	case JOB_GYPSY: return "Gypsy";
	case JOB_PALADIN2: return "Peko Paladin";
	case JOB_BABY: return "Baby Novice";
	case JOB_BABY_SWORDMAN: return "Baby Swordsman";
	case JOB_BABY_MAGE: return "Baby Mage";
	case JOB_BABY_ARCHER: return "Baby Archer";
	case JOB_BABY_ACOLYTE: return "Baby Acolyte";
	case JOB_BABY_MERCHANT: return "Baby Merchant";
	case JOB_BABY_THIEF: return "Baby Thief";
	case JOB_BABY_KNIGHT: return "Baby Knight";
	case JOB_BABY_PRIEST: return "Baby Priest";
	case JOB_BABY_WIZARD: return "Baby Wizard";
	case JOB_BABY_BLACKSMITH: return "Baby Blacksmith";
	case JOB_BABY_HUNTER: return "Baby Hunter";
	case JOB_BABY_ASSASSIN: return "Baby Assassin";
	case JOB_BABY_KNIGHT2: return "Baby Peco Knight";
	case JOB_BABY_CRUSADER: return "Baby Crusader";
	case JOB_BABY_MONK: return "Baby Monk";
	case JOB_BABY_SAGE: return "Baby Sage";
	case JOB_BABY_ROGUE: return "Baby Rogue";
	case JOB_BABY_ALCHEMIST: return "Baby Alchemist";
	case JOB_BABY_BARD: return "Baby Bard";
	case JOB_BABY_DANCER: return "Baby Dancer";
	case JOB_BABY_CRUSADER2: return "Baby Peco Crusader";
	case JOB_SUPER_BABY: return "Super Baby";
	case JOB_TAEKWON: return "Taekwon";
	case JOB_STAR_GLADIATOR: return "Star Gladiator";
	case JOB_STAR_GLADIATOR2: return "Flying Star Gladiator";
	case JOB_SOUL_LINKER: return "Soul Linker";
	}
	return "Unknown Job";
}

//-------------------------------------------------------------
// Function to create the online files (txt and html). by [Yor]
//-------------------------------------------------------------
static void create_online_files(OnlineDB_TXT* db)
{
	unsigned int k, j; // for loop with strlen comparing
	int i, l; // for loops
	int players;    // count the number of players
	FILE *fp;       // for the txt file
	FILE *fp2;      // for the html file
	char temp[256];      // to prepare what we must display
	time_t time_server;  // for number of seconds
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	char datetime_str[64];
	int id[4096]; // sorting array
	DBIterator* iter;
	int char_id;
	CharDB* chars = charserver->chardb(charserver);

	if( db->display_option == 0 )
		return; // we display nothing

	// Get number of online players, id of each online players, and verify if a server is offline
	players = 0;
	iter = db->onlinedb->iterator(db->onlinedb);
	for( char_id = (int)iter->first(iter,NULL); iter->exists(iter); char_id = (int)iter->next(iter,NULL) )
	{
		// sort online characters.
		id[players] = char_id;
/*
		// use sorting option
		switch( online_sorting_option )
		{
		case 1: // by name (without case sensitive)
			for(k = 0; k < players; k++)
				if (stricmp(cd.name, char_dat[id[k]].status.name) < 0 ||
					// if same name, we sort with case sensitive.
					(stricmp(cd.name, char_dat[id[k]].status.name) == 0 &&
					 strcmp(cd.name, char_dat[id[k]].status.name) < 0)) {
					for(l = players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[players]
					break;
				}
			break;
		case 2: // by zeny
			for(k = 0; k < players; k++)
				if (cd.zeny < char_dat[id[k]].status.zeny ||
					// if same number of zenys, we sort by name.
					(cd.zeny == char_dat[id[k]].status.zeny &&
					 stricmp(cd.name, char_dat[id[k]].status.name) < 0)) {
					for(l = players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[players]
					break;
				}
			break;
		case 3: // by base level
			for(k = 0; k < players; k++)
				if (cd.base_level < char_dat[id[k]].status.base_level ||
					// if same base level, we sort by base exp.
					(cd.base_level == char_dat[id[k]].status.base_level &&
					 cd.base_exp < char_dat[id[k]].status.base_exp)) {
					for(l = players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[players]
					break;
				}
			break;
		case 4: // by job (and job level)
			for(k = 0; k < players; k++)
				if (cd.class_ < char_dat[id[k]].status.class_ ||
					// if same job, we sort by job level.
					(cd.class_ == char_dat[id[k]].status.class_ &&
					 cd.job_level < char_dat[id[k]].status.job_level) ||
					// if same job and job level, we sort by job exp.
					(cd.class_ == char_dat[id[k]].status.class_ &&
					 cd.job_level == char_dat[id[k]].status.job_level &&
					 cd.job_exp < char_dat[id[k]].status.job_exp)) {
					for(l = players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[players]
					break;
				}
			break;
		case 5: // by location map name
		{
			const char *map1, *map2;
			map1 = mapindex_id2name(cd.last_point.map);
			
			for(k = 0; k < players; k++) {
				map2 = mapindex_id2name(char_dat[id[k]].status.last_point.map);
				if (!map1 || !map2 || //Avoid sorting if either one failed to resolve.
					stricmp(map1, map2) < 0 ||
					// if same map name, we sort by name.
					(stricmp(map1, map2) == 0 &&
					 stricmp(cd.name, char_dat[id[k]].status.name) < 0)) {
					for(l = players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[players]
					break;
				}
			}
		}
		break;
		default: // 0 or invalid value: no sorting
			break;
		}
*/
		players++;
	}
	iter->destroy(iter);

	// write files
	fp = fopen(db->txt_filename, "w");
	if (fp == NULL)
		return;
	fp2 = fopen(db->html_filename, "w");
	if (fp2 == NULL)
	{
		fclose(fp);
		return;
	}

	// get time
	time(&time_server); // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	strftime(datetime_str, sizeof(datetime_str), "%d %b %Y %X", datetime); // like sprintf, but only for date/time (05 dec 2003 15:12:52)

	// write html header
	fprintf(fp2, "<HTML>\n");
	fprintf(fp2, "  <META http-equiv=\"Refresh\" content=\"%d\">\n", db->refresh_html);
	fprintf(fp2, "  <HEAD>\n");
	fprintf(fp2, "    <TITLE>Online Players on %s</TITLE>\n", char_config.server_name);
	fprintf(fp2, "  </HEAD>\n");
	fprintf(fp2, "  <BODY>\n");
	fprintf(fp2, "    <H3>Online Players on %s (%s):</H3>\n", char_config.server_name, datetime_str);

	// write txt header
	fprintf(fp, "Online Players on %s (%s):\n", char_config.server_name, datetime_str);
	fprintf(fp, "\n");

	// write table header
	if( players > 0 )
	{
		j = 0; // count the number of characters for the txt version and to set the separate line
		fprintf(fp2, "    <table border=\"1\" cellspacing=\"1\">\n");
		fprintf(fp2, "      <tr>\n");

		if ((db->display_option & 1) || (db->display_option & 64))
		{
			fprintf(fp2, "        <td><b>Name</b></td>\n");
			if (db->display_option & 64) {
				fprintf(fp, "Name                          "); // 30
				j += 30;
			} else {
				fprintf(fp, "Name                     "); // 25
				j += 25;
			}
		}

		if ((db->display_option & 6) == 6) {
			fprintf(fp2, "        <td><b>Job (levels)</b></td>\n");
			fprintf(fp, "Job                 Levels "); // 27
			j += 27;
		} else if (db->display_option & 2) {
			fprintf(fp2, "        <td><b>Job</b></td>\n");
			fprintf(fp, "Job                "); // 19
			j += 19;
		} else if (db->display_option & 4) {
			fprintf(fp2, "        <td><b>Levels</b></td>\n");
			fprintf(fp, " Levels "); // 8
			j += 8;
		}

		if (db->display_option & 24) { // 8 or 16
			fprintf(fp2, "        <td><b>Location</b></td>\n");
			if (db->display_option & 16) {
				fprintf(fp, "Location     ( x , y ) "); // 23
				j += 23;
			} else {
				fprintf(fp, "Location     "); // 13
				j += 13;
			}
		}

		if (db->display_option & 32) {
			fprintf(fp2, "        <td ALIGN=CENTER><b>zenys</b></td>\n");
			fprintf(fp, "          Zenys "); // 16
			j += 16;
		}

		fprintf(fp2, "      </tr>\n");
		fprintf(fp, "\n");

		for (k = 0; k < j; k++)
			fprintf(fp, "-");
		fprintf(fp, "\n");
	}

	for (i = 0; i < players; i++)
	{
		struct mmo_charstatus cd;

		if( !chars->load_num(chars, &cd, id[i]) )
			continue; //TODO: error message?

		fprintf(fp2, "      <tr>\n");

		// displaying the character name
		if ((db->display_option & 1) || (db->display_option & 64))
		{ // without/with 'GM' display
			strcpy(temp, cd.name);
			//l = isGM(cd.account_id);
			l = 0; //FIXME: how to get the gm level?
			if (db->display_option & 64) {
				if (l >= db->gm_display_min_lv)
					fprintf(fp, "%-24s (GM) ", temp);
				else
					fprintf(fp, "%-24s      ", temp);
			} else
				fprintf(fp, "%-24s ", temp);
			// name of the character in the html (no < >, because that create problem in html code)
			fprintf(fp2, "        <td>");
			if ((db->display_option & 64) && l >= db->gm_display_min_lv)
				fprintf(fp2, "<b>");
			for (k = 0; k < strlen(temp); k++) {
				switch(temp[k]) {
				case '<': // <
					fprintf(fp2, "&lt;");
					break;
				case '>': // >
					fprintf(fp2, "&gt;");
					break;
				default:
					fprintf(fp2, "%c", temp[k]);
					break;
				};
			}
			if ((db->display_option & 64) && l >= db->gm_display_min_lv)
				fprintf(fp2, "</b> (GM)");
			fprintf(fp2, "</td>\n");
		}

		// displaying of the job
		if (db->display_option & 6) {
			char * jobname = job_name(cd.class_);
			if ((db->display_option & 6) == 6) {
				fprintf(fp2, "        <td>%s %d/%d</td>\n", jobname, cd.base_level, cd.job_level);
				fprintf(fp, "%-18s %3d/%3d ", jobname, cd.base_level, cd.job_level);
			} else if (db->display_option & 2) {
				fprintf(fp2, "        <td>%s</td>\n", jobname);
				fprintf(fp, "%-18s ", jobname);
			} else if (db->display_option & 4) {
				fprintf(fp2, "        <td>%d/%d</td>\n", cd.base_level, cd.job_level);
				fprintf(fp, "%3d/%3d ", cd.base_level, cd.job_level);
			}
		}

		// displaying of the map
		if (db->display_option & 24) { // 8 or 16
			// prepare map name
			memcpy(temp, mapindex_id2name(cd.last_point.map), MAP_NAME_LENGTH);
			// write map name
			if (db->display_option & 16) { // map-name AND coordinates
				fprintf(fp2, "        <td>%s (%d, %d)</td>\n", temp, cd.last_point.x, cd.last_point.y);
				fprintf(fp, "%-12s (%3d,%3d) ", temp, cd.last_point.x, cd.last_point.y);
			} else {
				fprintf(fp2, "        <td>%s</td>\n", temp);
				fprintf(fp, "%-12s ", temp);
			}
		}
		// displaying nimber of zenys
		if (db->display_option & 32) {
			// write number of zenys
			if (cd.zeny == 0) { // if no zeny
				fprintf(fp2, "        <td ALIGN=RIGHT>no zeny</td>\n");
				fprintf(fp, "        no zeny ");
			} else {
				fprintf(fp2, "        <td ALIGN=RIGHT>%d z</td>\n", cd.zeny);
				fprintf(fp, "%13d z ", cd.zeny);
			}
		}

		fprintf(fp, "\n");
		fprintf(fp2, "      </tr>\n");
	}

	// If we display at least 1 player
	if (players > 0) {
		fprintf(fp2, "    </table>\n");
		fprintf(fp, "\n");
	}

	// Displaying number of online players
	if (players == 0) {
		fprintf(fp2, "    <p>No user is online.</p>\n");
		fprintf(fp, "No user is online.\n");
	} else if (players == 1) {
		fprintf(fp2, "    <p>%d user is online.</p>\n", players);
		fprintf(fp, "%d user is online.\n", players);
	} else {
		fprintf(fp2, "    <p>%d users are online.</p>\n", players);
		fprintf(fp, "%d users are online.\n", players);
	}

	fprintf(fp2, "  </BODY>\n");
	fprintf(fp2, "</HTML>\n");

	fclose(fp2);
	fclose(fp);
}
