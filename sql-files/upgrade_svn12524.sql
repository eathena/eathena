ALTER TABLE `loginlog` CHANGE `ip` `ip` VARCHAR( 15 ) NOT NULL default '';
UPDATE `loginlog` SET `ip` = inet_ntoa(`ip`);
