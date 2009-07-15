-- script converts the `charlog` table and moves it to the `log` database.
-- NOTE: change `ragnarok` and `log` to whatever your database names are.

START TRANSACTION;

CREATE TABLE `log`.`charlog` (
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `char_id` int(11) NOT NULL default '0',
  `account_id` int(11) NOT NULL default '0',
  `slot` tinyint(4) NOT NULL default '0',
  `name` varchar(23) NOT NULL default '',
  `message` varchar(255) NOT NULL default ''
) ENGINE=MyISAM; 

INSERT INTO `log`.`charlog` 
  SELECT `time`, IFNULL(`char_id`, 0) as `char_id`, `account_id`, `char_num` as `slot`, `l`.`name`, `char_msg` as `message`
  FROM `ragnarok`.`charlog` AS `l` LEFT OUTER JOIN `ragnarok`.`char` as `c` USING (`account_id`, `char_num`);

DROP TABLE `ragnarok`.`charlog` ;

COMMIT;
