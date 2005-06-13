--
-- Table structure for table `guild`
--

CREATE TABLE guild (
  guild_id int(11) NOT NULL auto_increment,
  name varchar(24) NOT NULL default '',
  char_id int(11) NOT NULL default '10000',
  master varchar(24) NOT NULL default '',
  guild_lv smallint(6) NOT NULL default '0',
  connect_member smallint(6) NOT NULL default '0',
  max_member smallint(6) NOT NULL default '0',
  average_lv smallint(6) NOT NULL default '0',
  exp bigint(20) unsigned NOT NULL default '0',
  next_exp bigint(20) unsigned NOT NULL default '0',
  skill_point int(11) NOT NULL default '0',
  castle_id int(11) NOT NULL default '-1',
  mes1 varchar(60) NOT NULL default '',
  mes2 varchar(120) NOT NULL default '',
  emblem_len int(11) NOT NULL default '0',
  emblem_id int(11) NOT NULL default '0',
  emblem_data blob NOT NULL,
  PRIMARY KEY  (guild_id,char_id),
  UNIQUE KEY guild_id (guild_id),
  KEY char_id (char_id),
  CONSTRAINT `guild_ibfk_1` FOREIGN KEY (`char_id`) REFERENCES `char` (`char_id`) ON DELETE CASCADE
) TYPE=InnoDB;

--
-- Dumping data for table `guild`
--


--
-- Table structure for table `guild_alliance`
--

CREATE TABLE guild_alliance (
  guild_id int(11) NOT NULL default '0',
  opposition int(11) NOT NULL default '0',
  alliance_id int(11) NOT NULL default '0',
  name varchar(24) NOT NULL default '',
  PRIMARY KEY  (guild_id,alliance_id),
  KEY alliance_id (alliance_id),
  CONSTRAINT `guild_alliance_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE,
  CONSTRAINT `guild_alliance_ibfk_2` FOREIGN KEY (`alliance_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=InnoDB;

--
-- Dumping data for table `guild_alliance`
--


--
-- Table structure for table `guild_castle`
--

CREATE TABLE guild_castle (
  castle_id int(11) NOT NULL default '0',
  guild_id int(11) NOT NULL default '0',
  economy int(11) NOT NULL default '0',
  defense int(11) NOT NULL default '0',
  triggerE int(11) NOT NULL default '0',
  triggerD int(11) NOT NULL default '0',
  nextTime int(11) NOT NULL default '0',
  payTime int(11) NOT NULL default '0',
  createTime int(11) NOT NULL default '0',
  visibleC int(11) NOT NULL default '0',
  visibleG0 int(11) NOT NULL default '0',
  visibleG1 int(11) NOT NULL default '0',
  visibleG2 int(11) NOT NULL default '0',
  visibleG3 int(11) NOT NULL default '0',
  visibleG4 int(11) NOT NULL default '0',
  visibleG5 int(11) NOT NULL default '0',
  visibleG6 int(11) NOT NULL default '0',
  visibleG7 int(11) NOT NULL default '0',
  gHP0 int(11) NOT NULL default '0',
  ghP1 int(11) NOT NULL default '0',
  gHP2 int(11) NOT NULL default '0',
  gHP3 int(11) NOT NULL default '0',
  gHP4 int(11) NOT NULL default '0',
  gHP5 int(11) NOT NULL default '0',
  gHP6 int(11) NOT NULL default '0',
  gHP7 int(11) NOT NULL default '0',
  PRIMARY KEY  (castle_id)
) TYPE=InnoDB;

--
-- Dumping data for table `guild_castle`
--


--
-- Table structure for table `guild_expulsion`
--

CREATE TABLE guild_expulsion (
  guild_id int(11) NOT NULL default '0',
  name varchar(24) NOT NULL default '',
  mes varchar(40) NOT NULL default '',
  acc varchar(40) NOT NULL default '',
  account_id int(11) NOT NULL default '0',
  rsv1 int(11) NOT NULL default '0',
  rsv2 int(11) NOT NULL default '0',
  rsv3 int(11) NOT NULL default '0',
  PRIMARY KEY  (guild_id,name),
  CONSTRAINT `guild_expulsion_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=InnoDB;

--
-- Dumping data for table `guild_expulsion`
--


--
-- Table structure for table `guild_member`
--

CREATE TABLE guild_member (
  guild_id int(11) NOT NULL default '0',
  account_id int(11) NOT NULL default '0',
  char_id int(11) NOT NULL default '0',
  hair smallint(6) NOT NULL default '0',
  hair_color smallint(6) NOT NULL default '0',
  gender smallint(6) NOT NULL default '0',
  class smallint(6) NOT NULL default '0',
  lv smallint(6) NOT NULL default '0',
  exp bigint(20) NOT NULL default '0',
  exp_payper int(11) NOT NULL default '0',
  online tinyint(4) NOT NULL default '0',
  position smallint(6) NOT NULL default '0',
  rsv1 int(11) NOT NULL default '0',
  rsv2 int(11) NOT NULL default '0',
  name varchar(24) NOT NULL default '',
  PRIMARY KEY  (guild_id,char_id),
  KEY char_id (char_id),
  CONSTRAINT `guild_member_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE,
  CONSTRAINT `guild_member_ibfk_2` FOREIGN KEY (`char_id`) REFERENCES `lege_char` (`char_id`) ON DELETE CASCADE
) TYPE=InnoDB;

--
-- Dumping data for table `guild_member`
--


--
-- Table structure for table `guild_position`
--

CREATE TABLE guild_position (
  guild_id int(11) NOT NULL default '0',
  position smallint(6) NOT NULL default '0',
  name varchar(24) NOT NULL default '',
  mode int(11) NOT NULL default '0',
  exp_mode int(11) NOT NULL default '0',
  PRIMARY KEY  (guild_id,position),
  KEY guild_id (guild_id)
) TYPE=InnoDB;

--
-- Dumping data for table `guild_position`
--


--
-- Table structure for table `guild_skill`
--

CREATE TABLE guild_skill (
  guild_id int(11) NOT NULL default '0',
  id int(11) NOT NULL default '0',
  lv int(11) NOT NULL default '0',
  PRIMARY KEY  (guild_id,id),
  CONSTRAINT `guild_skill_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=InnoDB;

--
-- Dumping data for table `guild_skill`
--


--
-- Table structure for table `guild_storage`
--

CREATE TABLE guild_storage (
  id int(11) NOT NULL auto_increment,
  guild_id int(11) NOT NULL default '0',
  nameid int(11) NOT NULL default '0',
  amount int(11) NOT NULL default '0',
  equip mediumint(8) unsigned NOT NULL default '0',
  identify smallint(6) NOT NULL default '0',
  refine tinyint(3) unsigned NOT NULL default '0',
  attribute tinyint(4) NOT NULL default '0',
  card0 int(11) NOT NULL default '0',
  card1 int(11) NOT NULL default '0',
  card2 int(11) NOT NULL default '0',
  card3 int(11) NOT NULL default '0',
  broken int(11) NOT NULL default '0',
  PRIMARY KEY  (id),
  KEY guild_id (guild_id),
  CONSTRAINT `guild_storage_ibfk_1` FOREIGN KEY (`guild_id`) REFERENCES `guild` (`guild_id`) ON DELETE CASCADE
) TYPE=InnoDB;

--
-- Dumping data for table `guild_storage`
--