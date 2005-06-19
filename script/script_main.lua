--------------------------------------------------------------
--                eAthena Primary Scripts File              --
--------------------------------------------------------------
--  The idea of this new system is to make scripts more organized
-- since the old system was rather messy with all the NPCs in one
-- file. Now scripts are organized in to files arraged by type.
-- Custom scripts are now in script_custom.lua, all other 
-- scripts are deemed as 'official'. You should place your NPCs
-- in to script_custom.lua to follow the trend.
--
-- Thanks,
--  Ancyker and the rest of the eAthena Team

--------------------------------------------------------------
------------------- Core Scripts Functions -------------------
-- Simply do NOT touch this ! 
require("script/core/core.lua")
--------------------------------------------------------------
------------------------ Script Files ------------------------
-- Sample scripts
require("script/sample/script_sample.lua")
-- Warps
require("script/warps/script_warps.lua")
-- Monster Spawn
--require("script/mobs/scripts_spawn.lua")
-- Your NPCs go in this file!
-- require("script/custom/script_custom.lua")
--------------------------------------------------------------