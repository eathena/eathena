UPDATE item_db SET type=11 WHERE type=2 AND script_use LIKE "%itemskill%" OR (script_use LIKE "%pet%" AND script_use NOT LIKE "%bpet%");
UPDATE item_db2 SET type=11 WHERE type=2 AND script_use LIKE "%itemskill%" OR (script_use LIKE "%pet%" AND script_use NOT LIKE "%bpet%");
