UPDATE `char` SET `last_map` = substring_index(`last_map`, '.', 1);
UPDATE `char` SET `save_map` = substring_index(`save_map`, '.', 1);
UPDATE `memo` SET `map` = substring_index(`map`, '.', 1);

ALTER TABLE `login` SET `email` `email` VARCHAR( 39 ) NOT NULL;
ALTER TABLE `login` SET `sex` `sex` ENUM( 'M', 'F', 'S' ) NOT NULL DEFAULT 'M';

ALTER TABLE `char` SET `last_map` `last_map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `char` SET `save_map` `save_map` VARCHAR( 11 ) NOT NULL;

ALTER TABLE `memo` SET `map` `map` VARCHAR( 11 ) NOT NULL;
ALTER TABLE `memo` SET `x` `x` SMALLINT( 4 ) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE `memo` SET `y` `y` SMALLINT( 4 ) UNSIGNED NOT NULL DEFAULT '0';

ALTER TABLE `party` SET `name` `name` VARCHAR( 24 ) NOT NULL;
