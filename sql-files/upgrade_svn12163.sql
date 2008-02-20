ALTER TABLE `login` MODIFY COLUMN `userid` VARCHAR(23) NOT NULL default '';

ALTER TABLE `loginlog` MODIFY COLUMN `user` VARCHAR(23) NOT NULL default '';
ALTER TABLE `charlog` MODIFY COLUMN `name` VARCHAR(23) NOT NULL default '';
