-- script moves the `interlog` table to the `log` database.
-- NOTE: change `ragnarok` and `log` to whatever your database names are.

START TRANSACTION;

CREATE TABLE `log`.`interlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `log` varchar(255) NOT NULL default ''
) ENGINE=MyISAM; 

INSERT INTO `log`.`interlog` 
  SELECT * 
  FROM `ragnarok`.`interlog`;

DROP TABLE `ragnarok`.`charlog` ;

COMMIT;
