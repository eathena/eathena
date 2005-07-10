--===== eAthena Script ======================================= 
--= Payon Cave Warps
--===== By: ================================================== 
--= Nana (1.0)
--===== Current Version: ===================================== 
--= 1.0
--===== Compatible With: ===================================== 
--= Any eAthena Mod
--===== Description: ========================================= 
--= Payon Dungeon Warps
--===== Additional Comments: ================================= 
--= No Comment
--============================================================ 

--= Payon Cave ===============================================
addwarp("payd01","pay_dun00.gat",184,33,"pay_dun01.gat",19,33,2,7)
addwarp("pay005","pay_dun00.gat",21,186,"pay_arche.gat",39,131,2,2)
addwarp("payd01-1","pay_dun01.gat",15,33,"pay_dun00.gat",181,33,2,6)
addwarp("payd02","pay_dun01.gat",286,25,"pay_dun02.gat",19,63,2,7)
addwarp("payd03","pay_dun02.gat",137,128,"pay_dun03.gat",155,159,4,1)
addwarp("payd02-1","pay_dun02.gat",16,63,"pay_dun01.gat",283,28,2,7)
addwarp("payd03-1","pay_dun03.gat",155,161,"pay_dun02.gat",137,126,2,1)
addwarp("payd04-2","pay_dun04.gat",191,41,"pay_dun03.gat",125,62,1,1)
addwarp("payd04-3","pay_dun04.gat",202,206,"pay_dun03.gat",125,62,1,1)
addwarp("payd04-4","pay_dun04.gat",32,204,"pay_dun03.gat",125,62,2,1)
addwarp("payd04-1","pay_dun04.gat",40,37,"pay_dun03.gat",125,62,2,2)

--= Special Warp =============================================

addnpc("payd04r","payd04r","pay_dun03.gat",127,62,4,45)
addareascript("payd04rArea","pay_dun03.gat",125,60,129,64,"PayonRand")

function PayonRand()
	local r = math.random(4)
	if r==1 then warp("pay_dun04.gat",201,204)
	elseif r==2 then warp("pay_dun04.gat",193,43)
	elseif r==3 then warp("pay_dun04.gat",43,40)
	elseif r==4 then warp("pay_dun04.gat",34,202) 
	end
end