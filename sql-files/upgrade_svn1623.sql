ALTER TABLE `mob_db` MODIFY `STR` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db` MODIFY `AGI` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db` MODIFY `VIT` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db` MODIFY `INT` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db` MODIFY `DEX` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db` MODIFY `LUK` tinyint(4) unsigned NOT NULL;

ALTER TABLE `mob_db2` MODIFY `STR` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db2` MODIFY `AGI` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db2` MODIFY `VIT` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db2` MODIFY `INT` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db2` MODIFY `DEX` tinyint(4) unsigned NOT NULL;
ALTER TABLE `mob_db2` MODIFY `LUK` tinyint(4) unsigned NOT NULL;

ALTER TABLE `item_db2` CHANGE `ID` `id` SMALLINT( 5 ) UNSIGNED NOT NULL DEFAULT '0',
CHANGE `Name` `name_english` VARCHAR( 24 ) NOT NULL ,
CHANGE `Name2` `name_japanese` VARCHAR( 24 ) NOT NULL ,
CHANGE `Type` `type` TINYINT( 2 ) UNSIGNED NOT NULL DEFAULT '0',
CHANGE `Price` `price_buy` INT( 10 ) UNSIGNED NULL ,
CHANGE `Sell` `price_sell` INT( 10 ) UNSIGNED NULL ,
CHANGE `Weight` `weight` INT( 10 ) UNSIGNED NOT NULL DEFAULT '0',
CHANGE `ATK` `attack` MEDIUMINT( 9 ) UNSIGNED NULL ,
CHANGE `DEF` `defence` MEDIUMINT( 9 ) UNSIGNED NULL ,
CHANGE `Range` `range` TINYINT( 2 ) UNSIGNED NULL ,
CHANGE `Slot` `slots` TINYINT( 2 ) UNSIGNED NULL ,
CHANGE `Job` `equip_jobs` MEDIUMINT( 8 ) UNSIGNED NULL ,
CHANGE `Gender` `equip_genders` TINYINT( 2 ) UNSIGNED NULL ,
CHANGE `Loc` `equip_locations` SMALLINT( 4 ) UNSIGNED NULL ,
CHANGE `wLV` `weapon_level` TINYINT( 2 ) UNSIGNED NULL ,
CHANGE `eLV` `equip_level` TINYINT( 3 ) UNSIGNED NULL ,
CHANGE `View` `view` TINYINT( 3 ) UNSIGNED NULL ,
CHANGE `UseScript` `script_use` TEXT NULL ,
CHANGE `EquipScript` `script_equip` TEXT NULL ,
CHANGE `Comment` `comment` TEXT NULL;
ALTER TABLE `item_db2` ADD PRIMARY KEY ( `id` );
ALTER TABLE `item_db2` DROP INDEX `ID`;