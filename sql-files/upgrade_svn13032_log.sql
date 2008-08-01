ALTER TABLE `chatlog` CHANGE `message` `message` VARCHAR( 255 ) NOT NULL;
ALTER TABLE `zenylog` MODIFY COLUMN `type` enum('M','T','V','S','N','A','E') NOT NULL default 'S';
