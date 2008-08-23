// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// online players by [Yor]
char online_txt_filename[1024] = "online.txt";
char online_html_filename[1024] = "online.html";
int online_sorting_option = 0; // sorting option to display online players in online files
int online_display_option = 1; // display options: to know which columns must be displayed
int online_refresh_html = 20; // refresh time (in sec) of the html file in the explorer
int online_gm_display_min_level = 20; // minimum GM level to display 'GM' when we want to display it

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

static int create_online_files_sub(DBKey key, void* data, va_list va)
{
	struct online_char_data *character;
	int* players;
	int *id;
	int j,k,l;
	character = (struct online_char_data*) data;
	players = va_arg(va, int*);
	id = va_arg(va, int*);
	
	// check if map-server is online
	if (character->server == -1 || character->char_id == -1) { //Character not currently online.
		return -1;
	}
	
	j = character->server;
	if (server[j].fd < 0) {
		server[j].users = 0;
		return -1;
	}
	// search position of character in char_dat and sort online characters.
	for(j = 0; j < char_num; j++) {
		if (char_dat[j].status.char_id != character->char_id)
			continue;
		id[*players] = j;
		// use sorting option
		switch (online_sorting_option) {
		case 1: // by name (without case sensitive)
			for(k = 0; k < *players; k++)
				if (stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0 ||
					// if same name, we sort with case sensitive.
					(stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) == 0 &&
					 strcmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 2: // by zeny
			for(k = 0; k < *players; k++)
				if (char_dat[j].status.zeny < char_dat[id[k]].status.zeny ||
					// if same number of zenys, we sort by name.
					(char_dat[j].status.zeny == char_dat[id[k]].status.zeny &&
					 stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 3: // by base level
			for(k = 0; k < *players; k++)
				if (char_dat[j].status.base_level < char_dat[id[k]].status.base_level ||
					// if same base level, we sort by base exp.
					(char_dat[j].status.base_level == char_dat[id[k]].status.base_level &&
					 char_dat[j].status.base_exp < char_dat[id[k]].status.base_exp)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 4: // by job (and job level)
			for(k = 0; k < *players; k++)
				if (char_dat[j].status.class_ < char_dat[id[k]].status.class_ ||
					// if same job, we sort by job level.
					(char_dat[j].status.class_ == char_dat[id[k]].status.class_ &&
					 char_dat[j].status.job_level < char_dat[id[k]].status.job_level) ||
					// if same job and job level, we sort by job exp.
					(char_dat[j].status.class_ == char_dat[id[k]].status.class_ &&
					 char_dat[j].status.job_level == char_dat[id[k]].status.job_level &&
					 char_dat[j].status.job_exp < char_dat[id[k]].status.job_exp)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			break;
		case 5: // by location map name
		{
			const char *map1, *map2;
			map1 = mapindex_id2name(char_dat[j].status.last_point.map);
			
			for(k = 0; k < *players; k++) {
				map2 = mapindex_id2name(char_dat[id[k]].status.last_point.map);
				if (!map1 || !map2 || //Avoid sorting if either one failed to resolve.
					stricmp(map1, map2) < 0 ||
					// if same map name, we sort by name.
					(stricmp(map1, map2) == 0 &&
					 stricmp(char_dat[j].status.name, char_dat[id[k]].status.name) < 0)) {
					for(l = *players; l > k; l--)
						id[l] = id[l-1];
					id[k] = j; // id[*players]
					break;
				}
			}
		}
		break;
		default: // 0 or invalid value: no sorting
			break;
		}
	(*players)++;
	break;
	}
	return 0;
}
//-------------------------------------------------------------
// Function to create the online files (txt and html). by [Yor]
//-------------------------------------------------------------
void create_online_files(void)
{
	unsigned int k, j; // for loop with strlen comparing
	int i, l; // for loops
	int players;    // count the number of players
	FILE *fp;       // for the txt file
	FILE *fp2;      // for the html file
	char temp[256];      // to prepare what we must display
	time_t time_server;  // for number of seconds
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	int id[4096];

	if (online_display_option == 0) // we display nothing, so return
		return;

	// Get number of online players, id of each online players, and verify if a server is offline
	players = 0;
	online_char_db->foreach(online_char_db, create_online_files_sub, &players, &id);

	// write files
	fp = fopen(online_txt_filename, "w");
	if (fp != NULL) {
		fp2 = fopen(online_html_filename, "w");
		if (fp2 != NULL) {
			// get time
			time(&time_server); // get time in seconds since 1/1/1970
			datetime = localtime(&time_server); // convert seconds in structure
			strftime(temp, sizeof(temp), "%d %b %Y %X", datetime); // like sprintf, but only for date/time (05 dec 2003 15:12:52)
			// write heading
			fprintf(fp2, "<HTML>\n");
			fprintf(fp2, "  <META http-equiv=\"Refresh\" content=\"%d\">\n", online_refresh_html); // update on client explorer every x seconds
			fprintf(fp2, "  <HEAD>\n");
			fprintf(fp2, "    <TITLE>Online Players on %s</TITLE>\n", server_name);
			fprintf(fp2, "  </HEAD>\n");
			fprintf(fp2, "  <BODY>\n");
			fprintf(fp2, "    <H3>Online Players on %s (%s):</H3>\n", server_name, temp);
			fprintf(fp, "Online Players on %s (%s):\n", server_name, temp);
			fprintf(fp, "\n");

			for (i = 0; i < players; i++) {
				// if it's the first player
				if (i == 0) {
					j = 0; // count the number of characters for the txt version and to set the separate line
					fprintf(fp2, "    <table border=\"1\" cellspacing=\"1\">\n");
					fprintf(fp2, "      <tr>\n");
					if ((online_display_option & 1) || (online_display_option & 64)) {
						fprintf(fp2, "        <td><b>Name</b></td>\n");
						if (online_display_option & 64) {
							fprintf(fp, "Name                          "); // 30
							j += 30;
						} else {
							fprintf(fp, "Name                     "); // 25
							j += 25;
						}
					}
					if ((online_display_option & 6) == 6) {
						fprintf(fp2, "        <td><b>Job (levels)</b></td>\n");
						fprintf(fp, "Job                 Levels "); // 27
						j += 27;
					} else if (online_display_option & 2) {
						fprintf(fp2, "        <td><b>Job</b></td>\n");
						fprintf(fp, "Job                "); // 19
						j += 19;
					} else if (online_display_option & 4) {
						fprintf(fp2, "        <td><b>Levels</b></td>\n");
						fprintf(fp, " Levels "); // 8
						j += 8;
					}
					if (online_display_option & 24) { // 8 or 16
						fprintf(fp2, "        <td><b>Location</b></td>\n");
						if (online_display_option & 16) {
							fprintf(fp, "Location     ( x , y ) "); // 23
							j += 23;
						} else {
							fprintf(fp, "Location     "); // 13
							j += 13;
						}
					}
					if (online_display_option & 32) {
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
				fprintf(fp2, "      <tr>\n");
				// get id of the character (more speed)
				j = id[i];
				// displaying the character name
				if ((online_display_option & 1) || (online_display_option & 64)) { // without/with 'GM' display
					strcpy(temp, char_dat[j].status.name);
					//l = isGM(char_dat[j].status.account_id);
					l = 0; //FIXME: how to get the gm level?
					if (online_display_option & 64) {
						if (l >= online_gm_display_min_level)
							fprintf(fp, "%-24s (GM) ", temp);
						else
							fprintf(fp, "%-24s      ", temp);
					} else
						fprintf(fp, "%-24s ", temp);
					// name of the character in the html (no < >, because that create problem in html code)
					fprintf(fp2, "        <td>");
					if ((online_display_option & 64) && l >= online_gm_display_min_level)
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
					if ((online_display_option & 64) && l >= online_gm_display_min_level)
						fprintf(fp2, "</b> (GM)");
					fprintf(fp2, "</td>\n");
				}
				// displaying of the job
				if (online_display_option & 6) {
					char * jobname = job_name(char_dat[j].status.class_);
					if ((online_display_option & 6) == 6) {
						fprintf(fp2, "        <td>%s %d/%d</td>\n", jobname, char_dat[j].status.base_level, char_dat[j].status.job_level);
						fprintf(fp, "%-18s %3d/%3d ", jobname, char_dat[j].status.base_level, char_dat[j].status.job_level);
					} else if (online_display_option & 2) {
						fprintf(fp2, "        <td>%s</td>\n", jobname);
						fprintf(fp, "%-18s ", jobname);
					} else if (online_display_option & 4) {
						fprintf(fp2, "        <td>%d/%d</td>\n", char_dat[j].status.base_level, char_dat[j].status.job_level);
						fprintf(fp, "%3d/%3d ", char_dat[j].status.base_level, char_dat[j].status.job_level);
					}
				}
				// displaying of the map
				if (online_display_option & 24) { // 8 or 16
					// prepare map name
					memcpy(temp, mapindex_id2name(char_dat[j].status.last_point.map), MAP_NAME_LENGTH);
					// write map name
					if (online_display_option & 16) { // map-name AND coordinates
						fprintf(fp2, "        <td>%s (%d, %d)</td>\n", temp, char_dat[j].status.last_point.x, char_dat[j].status.last_point.y);
						fprintf(fp, "%-12s (%3d,%3d) ", temp, char_dat[j].status.last_point.x, char_dat[j].status.last_point.y);
					} else {
						fprintf(fp2, "        <td>%s</td>\n", temp);
						fprintf(fp, "%-12s ", temp);
					}
				}
				// displaying nimber of zenys
				if (online_display_option & 32) {
					// write number of zenys
					if (char_dat[j].status.zeny == 0) { // if no zeny
						fprintf(fp2, "        <td ALIGN=RIGHT>no zeny</td>\n");
						fprintf(fp, "        no zeny ");
					} else {
						fprintf(fp2, "        <td ALIGN=RIGHT>%d z</td>\n", char_dat[j].status.zeny);
						fprintf(fp, "%13d z ", char_dat[j].status.zeny);
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
		}
		fclose(fp);
	}

	return;
}


/*
#ifdef TXT_ONLY
// online files options
		} else if (strcmpi(w1, "online_txt_filename") == 0) {
			strcpy(online_txt_filename, w2);
		} else if (strcmpi(w1, "online_html_filename") == 0) {
			strcpy(online_html_filename, w2);
		} else if (strcmpi(w1, "online_sorting_option") == 0) {
			online_sorting_option = atoi(w2);
		} else if (strcmpi(w1, "online_display_option") == 0) {
			online_display_option = atoi(w2);
		} else if (strcmpi(w1, "online_gm_display_min_level") == 0) { // minimum GM level to display 'GM' when we want to display it
			online_gm_display_min_level = atoi(w2);
			if (online_gm_display_min_level < 5) // send online file every 5 seconds to player is enough
				online_gm_display_min_level = 5;
		} else if (strcmpi(w1, "online_refresh_html") == 0) {
			online_refresh_html = atoi(w2);
			if (online_refresh_html < 1)
				online_refresh_html = 1;
#endif
*/