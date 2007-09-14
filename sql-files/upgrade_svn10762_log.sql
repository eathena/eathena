UPDATE `picklog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `zenylog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `branchlog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `mvplog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `atcommandlog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `npclog` SET `map` = substring_index(`map`, '.', 1);
UPDATE `chatlog` SET `src_map` = substring_index(`src_map`, '.', 1);

ALTER TABLE `picklog` SET `type` `type` ENUM( 'M', 'P', 'L', 'T', 'V', 'S', 'N', 'C', 'A', 'R', 'G' ) NOT NULL DEFAULT 'P';
ALTER TABLE `zenylog` SET `type` `type` ENUM( 'M', 'T', 'V', 'S', 'N', 'A' ) NOT NULL DEFAULT 'S';
ALTER TABLE `chatlog` SET `type` `type` ENUM( 'O', 'W', 'P', 'G', 'M' ) NOT NULL DEFAULT 'O';

ALTER TABLE `picklog` SET `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `zenylog` SET `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `branchlog` SET `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `mvplog` SET `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `atcommandlog` SET `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `npclog` SET `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `chatlog` SET `src_map` `src_map` VARCHAR( 11 ) NOT NULL;
