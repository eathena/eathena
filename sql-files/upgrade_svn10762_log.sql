UPDATE `picklog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `zenylog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `branchlog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `mvplog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `atcommandlog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `npclog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `chatlog` SET `src_map` = substring_index(`src_map`, '.', 1);

ALTER TABLE `picklog` CHANGE `type` `type` ENUM( 'M', 'P', 'L', 'T', 'V', 'S', 'N', 'C', 'A', 'R', 'G' ) NOT NULL DEFAULT 'P';
ALTER TABLE `zenylog` CHANGE `type` `type` ENUM( 'M', 'T', 'V', 'S', 'N', 'A' ) NOT NULL DEFAULT 'S';
ALTER TABLE `chatlog` CHANGE `type` `type` ENUM( 'O', 'W', 'P', 'G', 'M' ) NOT NULL DEFAULT 'O';

ALTER TABLE `picklog` CHANGE `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `zenylog` CHANGE `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `branchlog` CHANGE `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `mvplog` CHANGE `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `atcommandlog` CHANGE `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `npclog` CHANGE `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `chatlog` CHANGE `src_map` `src_map` VARCHAR( 11 ) NOT NULL;
