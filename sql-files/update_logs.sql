#types (M)onsters Drop, (P)layers Drop/Take, Mobs Drop (L)oot Drop/Take, Players (T)rade Give/Take, Players (V)ending Sell/Take, (S)hop Sell/Take, (N)PC Give/Take

#Database: log
#Table: picklog
CREATE TABLE `picklog` (
  `id` int(11) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_id` int(11) NOT NULL default '0',
  `type` enum('M','P','L','T','V','S','N') NOT NULL default 'M',
  `nameid` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '1',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `card0` int(11) NOT NULL default '0',
  `card1` int(11) NOT NULL default '0',
  `card2` int(11) NOT NULL default '0',
  `card3` int(11) NOT NULL default '0',
  `map` varchar(20) NOT NULL default '',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
