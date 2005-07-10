--===== eAthena Script =======================================
--= Warper Script
--===== By: ==================================================
--= Darkchild
--===== Current Version: =====================================
--= 2.0
--===== Compatible With: =====================================
--= Any eAthena Version
--===== Description: =========================================
--= Generic warper...
--===== Additional Comments: =================================
--= 1.0 by Darkchild
--= 1.1 by jabs
--= 1.2 by Lupus (placement fixed in Amatsu)
--= 1.3 fixed Louyang label typo, added warp and WARPRA into
--= Nifleheim. Also sorted all names in alphabet order [Lupus]
--= 1.4 fixed morroc warp npc overlaying kafra [Aria]
--= 1.5 Added Ayothaya and Einbroch to list, and town Warpra's [Fredzilla]
--= 2.0 Coverted to Lua [Fredzilla]
--============================================================

function Warpra()
	npcmes "[Warp NPC]"
	npcmes "Hello,"
	npcmes "I can warp you to any Town and Dungeon!"
	npcmes "Were do you want to go?"
	npcnext()
	local r = npcmenu("Towns",1,"Dungeons",2)
	if r == 1 then
	r = npcmenu("Alberta",1,"Aldebaran",2,"Amatsu",3,"Ayothaya",4,"Comodo",5,"Einbroch",6,"Geffen",7,"Gonryun",8,"Izlude",9,"Louyang",10,"Lutie",11,"Morroc",12,"Niflheim",13,"Payon",14,"Prontera",15,"Umbala",16,"Yuno",17)
		WarpTown(r)
	else
	r = npcmenu("Amatsu Dungeon",1,"Anthell",2,"Ayothaya Dungeon",3,"Bibilan Dungeon",4,"Coal Mine (Dead Pit)",5,"Culvert",6,"Einbech Dungeon",7,"Glast Heim",8,"Gonryun Dungeon",9,"Magma-Dungeon",10,"Orc-Dungeon",11,"Payon-Dungeon",12,"Pyramids",13,"Sphinx",14,"Sunken Ship",15,"Turtle Dungeon",16)
		WarpDun(r)
	end
end

------------------Towns----------------\\
function WarpTown(r)
if r == 1 or r == "Alberta" then
	warp("alberta.gat",27,236)
elseif r == 2 or r == "Aldebaran" then
	warp("aldebaran.gat",145,120)
elseif r == 3 or r == "Amatsu" then
	warp("amatsu.gat",197,86)
elseif r == 4 or r == "Ayothaya" then
	warp("ayothaya.gat",149,118)
elseif r == 5 or r == "Comodo" then
	warp("comodo.gat",188,161)
elseif r == 6 or r == "Einbroch" then
	warp("einbroch.gat",64,200)
elseif r == 8 or r == "Gonryun" then
	warp("gonryun.gat",150,130)
elseif r == 7 or r == "Geffen" then
	warp("geffen.gat",119,66)
elseif r == 9 or r == "Izlude" then
	warp("izlude.gat",128,111)
elseif r == 10 or r == "Louyang" then
	warp("louyang.gat",210,108)
elseif r == 12 or r == "Morocc" then
	warp("morocc.gat", 159, 93)
elseif r == 13 or r == "Niflheim" then
	warp("niflheim.gat",35,161)
elseif r == 15 or r == "Prontera" then
	warp("prontera.gat",156,187)
elseif r == 14 or r == "Payon" then
	warp("payon.gat",152,75)
elseif r == 16 or r == "Umbala" then
	warp("umbala.gat",130,130)
elseif r == 11 or r == "Lutie" then
	warp("xmas.gat",148,131)
elseif r == 17 or r == "Yuno" then
	warp("yuno.gat",160,168)
end
scriptend()
end

------------------Dungeons----------------\\

function WarpDun(r)
if r == 1 then
	warp("ama_fild01.gat",172,324)
elseif r == 2 then
	warp("moc_fild04.gat",210,328)
elseif r == 3 then
	warp("ayo_fild02.gat",280,149)
elseif r == 4 then
	warp("izlu2dun.gat",106,88)
elseif r == 6 then
	warp("prt_fild05.gat",273,210)
elseif r == 5 then
	warp("mjolnir_02.gat",81,359)
elseif r == 7 then
	warp("einbech.gat",135,249)
elseif r == 8 then
	warp("glast_01.gat",368,303)
elseif r == 9 then
	warp("gonryun.gat",160,195)
elseif r == 10 then
	warp("yuno_fild03.gat",39,140)
elseif r == 11 then
	warp("gef_fild10.gat",70,332)
elseif r == 12 then
	warp("pay_arche.gat",43,132)
elseif r == 13 then
	warp("moc_ruins.gat",62,162)
elseif r == 14 then
	warp("moc_fild19.gat",107,100)
elseif r == 15 then
	warp("alb2trea.gat",75,98)
elseif r == 16 then
	warp("tur_dun01.gat",149,238)
end
scriptend()
end

addnpc("Warp NPC","Warpra1","alb2trea.gat",73,101,4,115,"Warpra")
addnpc("Warp NPC","Warpra2","alberta.gat",31,240,4,115,"Warpra")
addnpc("Warp NPC","Warpra3","aldebaran.gat",145,118,4,115,"Warpra")
addnpc("Warp NPC","Warpra4","ayothaya.gat",147,121,4,115,"Warpra")
addnpc("Warp NPC","Warpra5","ayo_fild02.gat",279,154,4,115,"Warpra")
addnpc("Warp NPC","Warpra6","amatsu.gat",192,81,1,115,"Warpra")
addnpc("Warp NPC","Warpra7","ama_fild01.gat",178,325,1,115,"Warpra")
addnpc("Warp NPC","Warpra8","comodo.gat",194,158,4,115,"Warpra")
addnpc("Warp NPC","Warpra9","comodo.gat",194,158,4,115,"Warpra")
addnpc("Warp NPC","Warpra10","einbroch.gat",59,205,4,115,"Warpra")
addnpc("Warp NPC","Warpra11","einbech.gat",135,249,4,115,"Warpra")
addnpc("Warp NPC","Warpra12","geffen.gat",115,66,4,115,"Warpra")
addnpc("Warp NPC","Warpra13","gef_fild10.gat",71,339,4,115,"Warpra")
addnpc("Warp NPC","Warpra14","glast_01.gat",370,308,4,115,"Warpra")
addnpc("Warp NPC","Warpra15","gonryun.gat",151,130,4,115,"Warpra")
addnpc("Warp NPC","Warpra16","gonryun.gat",164,196,4,115,"Warpra")
addnpc("Warp NPC","Warpra17","izlude.gat",131,116,4,115,"Warpra")
addnpc("Warp NPC","Warpra18","izlu2dun.gat",104,82,4,115,"Warpra")
addnpc("Warp NPC","Warpra19","louyang.gat",210,106,4,115,"Warpra")
addnpc("Warp NPC","Warpra20","mjolnir_02.gat",85,363,4,115,"Warpra")
addnpc("Warp NPC","Warpra21","moc_fild04.gat",207,331,4,115,"Warpra")
addnpc("Warp NPC","Warpra22","moc_fild19.gat",106,97,4,115,"Warpra")
addnpc("Warp NPC","Warpra23","moc_ruins.gat",64,166,4,115,"Warpra")
addnpc("Warp NPC","Warpra24","morocc.gat",156,95,4,115,"Warpra")
addnpc("Warp NPC","Warpra25","niflheim.gat",32,161,4,115,"Warpra")
addnpc("Warp NPC","Warpra26","pay_arche.gat",39,135,4,115,"Warpra")
addnpc("Warp NPC","Warpra27","payon.gat",182,110,4,115,"Warpra")
addnpc("Warp NPC","Warpra28","prontera.gat",161,192,4,115,"Warpra")
addnpc("Warp NPC","Warpra29","prt_fild05.gat",273,215,4,115,"Warpra")
addnpc("Warp NPC","Warpra30","tur_dun01.gat",148,239,4,115,"Warpra")
addnpc("Warp NPC","Warpra31","umbala.gat",132,130,4,115,"Warpra")
addnpc("Warp NPC","Warpra32","valkyrie.gat",48,35,8,115,"Warpra")
addnpc("Warp NPC","Warpra33","xmas.gat",150,136,4,115,"Warpra")
addnpc("Warp NPC","Warpra34","yuno.gat",137,162,4,115,"Warpra")
addnpc("Warp NPC","Warpra35","yuno_fild03.gat",37,135,4,115,"Warpra")