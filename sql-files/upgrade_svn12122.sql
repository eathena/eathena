ALTER TABLE `guild_expulsion`
  DROP `acc`,
  DROP `rsv1`,
  DROP `rsv2`,
  DROP `rsv3`;

ALTER TABLE `guild_expulsion`
  MODIFY `account_id` int(11) unsigned NOT NULL default '0' AFTER `guild_id`;

ALTER TABLE `guild_member`
  DROP `rsv1`,
  DROP `rsv2`;

ALTER TABLE `guild_castle`
  DROP `gHP0`,
  DROP `gHP1`,
  DROP `gHP2`,
  DROP `gHP3`,
  DROP `gHP4`,
  DROP `gHP5`,
  DROP `gHP6`,
  DROP `gHP7`;
