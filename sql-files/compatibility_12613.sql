-- The statements below will create a 'view' (virtual table) that mimics
-- the previous `login` table layout. You can use this hack to make your db
-- compatible with older eathena servers, or control panels.
-- Note: for eathena, go adjust the login_db setting in inter_athena.conf.
-- Note: if your RO CP does not have a config setting for the `login` table
--       name, you'll have to either modify the CP or do some table renaming.

-- create dummy columns, needed to make a 1:1 insertable view
ALTER TABLE `login` ADD `error_message` SMALLINT UNSIGNED NOT NULL;
ALTER TABLE `login` ADD `memo` SMALLINT UNSIGNED NOT NULL;

-- create the view
CREATE VIEW `login_view` ( `account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level`, `error_message`, `connect_until`, `last_ip`, `memo`, `ban_until`, `state` ) AS SELECT `account_id`, `userid`, `user_pass`, `lastlogin`, `sex`, `logincount`, `email`, `level`, `error_message`, `expiration_time`, `last_ip`, `memo`, `unban_time`, `state` FROM `login`
