-- TODO rename to upgrade_svn<revision>.sql when merged to trunk

--
-- Table structure for table `ranks`
--

CREATE TABLE IF NOT EXISTS `ranks` (
  `rank_id` INT(11) NOT NULL,
  `char_id` INT(11) NOT NULL,
  `points` INT(11) NOT NULL DEFAULT '0',
  UNIQUE KEY (`rank_id`,`char_id`)
) ENGINE=MyISAM;

-- blacksmith ranking
INSERT INTO `ranks` SELECT 1, `char_id`, `fame` FROM `char` WHERE `class`=10 OR `class`=4011 OR `class`=4033;

-- alchemist ranking
INSERT INTO `ranks` SELECT 2, `char_id`, `fame` FROM `char` WHERE `class`=18 OR `class`=4019 OR `class`=4041;

-- taekwon ranking
INSERT INTO `ranks` SELECT 3, `char_id`, `fame` FROM `char` WHERE `class`=4046;
