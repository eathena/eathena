# [Lupus] On reading some MySQL optimization articles...
# ENUM -> SET, because it works faster, uses less space.
# 'SET' type has only one limit: It can contain up to 64 values only.

ALTER TABLE `picklog` CHANGE `type` `type` SET( 'M', 'P', 'L', 'T', 'V', 'S', 'N', 'C', 'A' ) DEFAULT 'P' NOT NULL;
ALTER TABLE `picklog` CHANGE `map` `map` VARCHAR( 20 ) DEFAULT 'prontera.gat' NOT NULL ;

ALTER TABLE `chatlog` CHANGE `type` `type` SET( 'W', 'P', 'G' ) DEFAULT 'W' NOT NULL ;