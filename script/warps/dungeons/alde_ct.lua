--===== Athena Script ========================================
--= Al de Baran Clock Tower Warp Script
--===== By: ==================================================
--= Nana (1.0)
--===== Current Version: =====================================
--= 1.1
--===== Compatible With: =====================================
--= Any Athena Version; RO Episode 2+
--===== Description: =========================================
--= Warp Points for Clock Tower
--===== Additional Comments: =================================
--= Split off Aldebaran.txt
--= 1.1 fixed clt007 warp
--============================================================

--= Al De Baran Clock Tower
addwarp("ald002","aldebaran.gat",139,135,"c_tower1.gat",199,159,1,1)
addwarp("ald003","c_tower1.gat",200,157,"aldebaran.gat",139,131,1,1)
addwarp("clt001","c_tower1.gat",235,226,"c_tower2.gat",268,26,1,1)
addwarp("clt002","c_tower1.gat",123,22,"alde_dun01.gat",297,25,1,1)
addwarp("clt003","c_tower2.gat",142,283,"c_tower3.gat",65,147,1,1)
addwarp("clt004","c_tower2.gat",24,24,"alde_dun03.gat",277,178,1,1)
addwarp("clt005","c_tower2.gat",273,26,"c_tower1.gat",235,223,1,1)
--= Level 3 ==================================================
addwarp("clt009","c_tower3.gat",60,147,"c_tower2.gat",148,283,1,1)
addwarp("clt010","c_tower3.gat",212,159,"alde_dun03.gat",276,53,1,1)
addwarp("clt011","c_tower3.gat",7,39,"alde_dun01.gat",171,158,1,1)
addwarp("clt012","c_tower3.gat",42,41,"alde_dun02.gat",127,169,1,1)
addwarp("clt013","c_tower3.gat",146,8,"c_tower1.gat",235,223,1,1)
--= Level 4 ==================================================
addwarp("clt017","c_tower4.gat",37,70,"alde_dun03.gat",277,54,1,1)
addwarp("clt018","c_tower4.gat",51,156,"alde_dun01.gat",171,158,1,1)
addwarp("clt019","c_tower4.gat",68,46,"c_tower4.gat",73,154,1,1)
addwarp("clt020","c_tower4.gat",70,19,"c_tower3.gat",151,8,2,2)
addwarp("clt021","c_tower4.gat",79,49,"c_tower4.gat",204,60,2,2)
addwarp("clt022","c_tower4.gat",133,202,"c_tower4.gat",140,149,1,1)
addwarp("clt023","c_tower4.gat",153,107,"c_tower2.gat",228,267,1,1)
addwarp("clt024","c_tower4.gat",171,179,"alde_dun03.gat",276,53,1,1)
addwarp("clt025","c_tower4.gat",198,59,"c_tower4.gat",152,98,1,1)
addwarp("clt026","c_tower4.gat",204,57,"c_tower4.gat",65,77,1,1)
--============================================================
addwarp("aldd01","alde_dun01.gat",292,306,"alde_dun02.gat",43,24,2,1)
addwarp("ald002","alde_dun01.gat",167,158,"c_tower2.gat",148,283,2,2)
addwarp("ald003","alde_dun01.gat",302,25,"c_tower1.gat",125,22,2,2)
addwarp("aldd04","alde_dun02.gat",43,20,"alde_dun01.gat",292,300,1,1)
addwarp("aldd05","alde_dun02.gat",279,250,"alde_dun03.gat",18,267,2,2)
addwarp("ald006","alde_dun02.gat",122,169,"c_tower3.gat",47,41,2,2)
addwarp("ald007","alde_dun02.gat",187,234,"c_tower3.gat",65,147,2,2)
--= B2 ================================================
addwarp("aldd09","alde_dun03.gat",12,267,"alde_dun02.gat",273,250,2,2)
addwarp("ald010","alde_dun03.gat",277,183,"c_tower2.gat",27,27,2,2)
addwarp("ald011","alde_dun03.gat",191,31,"c_tower3.gat",217,159,2,2)
addwarp("ald012","alde_dun03.gat",276,48,"c_tower1.gat",235,223,2,2)
--= 3-2 ===============================================
addwarp("aldd016","alde_dun04.gat",80,273,"alde_dun03.gat",263,26,2,2)
addwarp("ald017","alde_dun04.gat",207,225,"c_tower3.gat",7,34,1,1)
addwarp("ald018","alde_dun04.gat",215,192,"c_tower2.gat",148,283,1,1)
addwarp("aldd19","alde_dun04.gat",32,74,"alde_dun02.gat",187,239,1,1)
addwarp("aldd20","alde_dun04.gat",208,58,"alde_dun04.gat",268,74,2,2)
addwarp("aldd021","alde_dun04.gat",272,74,"alde_dun04.gat",204,62,2,2)

--= Special Warps ============================================
--============================================================
--= Level 2 ==================================================
--= Random 2-1 ===============================================
addnpc("clt006r","clt006r","c_tower2.gat",13,288,4,45)
addareascript("clt006rArea","c_tower2.gat",11,286,15,290,"warp_Clockt01")

function warp_Clockt01()
	local r = math.random(3)
	if r==1 then warp("c_tower2.gat",13,282)
	elseif r==2 then warp("alde_dun03.gat",175,131)
	elseif r==3 then warp("c_tower3.gat",235,7) end
end
--============================================================
--= Random 2-2 ===============================================
addnpc("clt007r","clt007r","c_tower2.gat",223,267,4,45)
addareascript("clt007rArea","c_tower2.gat",222,266,224,268,"warp_Clockt02")

function warp_Clockt02()
	local r = math.random(3)
	if r==1 then warp("c_tower2.gat",288,267)
	elseif r==2 then warp("alde_dun03.gat",130,130)
	elseif r==3 then warp("c_tower3.gat",252,29) end
end
--============================================================
--= Random 3-1 ===============================================
addnpc("clt014r","clt014r","c_tower3.gat",163,252,4,45)
addareascript("clt014rArea","c_tower3.gat",162,251,164,253,"warp_Clockt03")

function warp_Clockt03()
	local r = math.random(2)
	if r==1 then warp("c_tower3.gat",168,252)
	elseif r==2 then warp("alde_dun02.gat",262,41) end
end
--============================================================
--= Random 3-2 ===============================================
addnpc("clt015r","clt015r","c_tower3.gat",240,7,4,45)
addareascript("clt015rArea","c_tower3.gat",239,6,241,8,"warp_Clockt04")

function warp_Clockt04()
	local r = math.random(3)
	if r==1 then warp("c_tower2.gat",13,282)
	elseif r==2 then warp("alde_dun03.gat",175,131)
	elseif r==3 then warp("c_tower3.gat",235,7) end
end
--============================================================
--= Random 3-3 ===============================================
addnpc("clt016r","clt016r","c_tower3.gat",252,24,4,45)
addareascript("clt016rArea","c_tower3.gat",251,23,253,25,"warp_Clockt05")

function warp_Clockt05()
	local r = math.random(3)
	if r==1 then warp("c_tower2.gat",228,267)
	elseif r==2 then warp("alde_dun03.gat",130,130)
	elseif r==3 then warp("c_tower3.gat",252,29) end
end
--============================================================
--= Random 4-1 ===============================================
addnpc("clt027r","clt027r","c_tower4.gat",75,156,4,45)
addareascript("clt027rArea","c_tower4.gat",74,155,76,157,"warp_Clockt06")

function warp_Clockt06()
	local r = math.random(4)
	if r==1 then warp("c_tower3.gat",168,252)
	elseif r==2 then warp("alde_dun02.gat",262,41)
	elseif r==3 then warp("c_tower4.gat",73,154)
	elseif r==4 then warp("c_tower4.gat",140,149) end
end
--============================================================
--= Random 4-2 ===============================================
addnpc("clt028r","clt028r","c_tower4.gat",68,79,4,45)
addareascript("clt028rArea","c_tower4.gat",67,78,69,80,"warp_Clockt07")

function warp_Clockt07()
	local r = math.random(4)
	if r==1 then warp("c_tower2.gat",13,282)
	elseif r==2 then warp("alde_dun03.gat",175,131)
	elseif r==3 then warp("c_tower3.gat",235,7)
	elseif r==4 then warp("c_tower4.gat",65,77) end
end
--============================================================
--= Random 4-3 ===============================================
addnpc("clt029r","clt029r","c_tower4.gat",142,151,4,45)
addareascript("clt029rArea","c_tower4.gat",141,150,143,152,"warp_Clockt08")

function warp_Clockt08()
	local r = math.random(4)
	if r==1 then warp("c_tower3.gat",168,252)
	elseif r==2 then warp("alde_dun02.gat",262,41)
	elseif r==3 then warp("c_tower4.gat",73,154)
	elseif r==4 then warp("c_tower4.gat",140,149) end
end
--============================================================
--= Random 4-4 ===============================================
addnpc("clt030r","clt030r","c_tower4.gat",151,96,4,45)
addareascript("clt030rArea","c_tower4.gat",150,95,152,97,"warp_Clockt09")

function warp_Clockt09()
	local r = math.random(4)
	if r==1 then warp("c_tower2.gat",228,267)
	elseif r==2 then warp("alde_dun03.gat",130,130)
	elseif r==3 then warp("c_tower3.gat",252,29)
	elseif r==4 then warp("c_tower4.gat",152,95) end
end
--============================================================
--= Random 4-5 ===============================================
addnpc("clt031r","clt031r","c_tower4.gat",189,40,4,45)
addareascript("clt031rArea","c_tower4.gat",187,38,191,42,"warp_Clockt10")

function warp_Clockt10()
	local r = math.random(4)
	if r==1 then warp("c_tower2.gat",228,267)
	elseif r==2 then warp("alde_dun03.gat",130,130)
	elseif r==3 then warp("c_tower3.gat",252,29)
	elseif r==4 then warp("c_tower4.gat",152,95) end	
end
--= Random B2 ================================================
addnpc("clt008r","clt008r","alde_dun02.gat",267,41,4,45)
addareascript("clt008rArea","alde_dun02.gat",266,40,268,42,"warp_Clockt11")

function warp_Clockt11()
	local r = math.random(2)
	if r==1 then warp("c_tower3.gat",168,252)
	elseif r==2 then warp("alde_dun02.gat",262,141) end
end
--============================================================
--7(npc)
--= Random B3-1 ================================================
addnpc("clt014r","clt014r","alde_dun03.gat",130,125,4,45)
addareascript("clt014rArea","alde_dun03.gat",129,124,131,126,"warp_Clockt12")

function warp_Clockt12()
	local r = math.random(3)
	if r==1 then warp("c_tower2.gat",228,267)
	elseif r==2 then warp("alde_dun03.gat",130,130)
	elseif r==3 then warp("c_tower3.gat",252,29) end	
end
--============================================================
--= Random 3-2 ===============================================
addnpc("clt015r","clt015r","alde_dun03.gat",171,127,4,45)
addareascript("clt015rArea","alde_dun03.gat",170,126,172,128,"warp_Clockt13")
function warp_Clockt13()
	local r = math.random(3)
	if r==1 then warp("c_tower2.gat",13,282)
	elseif r==2 then warp("alde_dun03.gat",175,131)
	elseif r==3 then warp("c_tower3.gat",235,7) end	
end

addnpc("clt022r","clt022r","alde_dun04.gat",80,34,4,45)
addareascript("clt022rArea","alde_dun04.gat",79,33,81,35,"warp_Clockt14")
function warp_Clockt14()
	local r = math.random(4)
	if r==1 then warp("c_tower2.gat",13,282)
	elseif r==2 then warp("alde_dun03.gat",175,131)
	elseif r==3 then warp("c_tower3.gat",235,7)
	elseif r==4 then warp("alde_dun04.gat",84,36) end	
end