--===== eAthena Script ======================================= 
--= Shops
--===== By: ================================================== 
--= eAthena Dev Team
--===== Current Version: ===================================== 
--= 1.8
--===== Compatible With: ===================================== 
--= eAthena 7.15 +
--===== Description: ========================================= 
--=
--===== Additional Comments: ================================= 
--= swapped shop titles in GONRYUN, thanks to Kashy
--= 1.3a Fixed Louyang map name. Added additional shops in Yuno. [kobra_k88]
--= 1.4 Added Niflheim Shops
--= 1.5 New Payon Locations [Darkchild]
--= Moved shops in Umbala.txt here. Commented out the duplicate ones.[kobra_k88]
--= 1.6 Removed GRAPE ID 514 from shops (caused exploits)
--= 1.7 Removed juices from custom amatsu shops (it's a quest item + levelup exploit)
--= 1.8 Corrected Niflheim Shops and Morroc Jewel Merchant [celest]
--= Func Info ================================================
--= Highest func uses of each type of shop ===================
--= shop_tool20
--= shop_wep14
--= shop_equip14
--= shop_trad6
--= shop_meat3
--= shop_gift2
--= shop_item2
--= show_jewel2
--= shop_flower2
--= shop_fruit2
--= shop_church
--= shop_blak
--= shop_souv
--= shop_wed
--= shop_banana
--= shop_veg
--===========================================================



--=======================================================
--ALBERTA
--=======================================================
addnpc("Tool Dealer","shop_69","alb2trea.gat",87,65,1,83,"shop_tool17")
addnpc("Tool Dealer","shop_1","alberta_in.gat",94,56,4,83,"shop_tool1")
addnpc("Tool Dealer","shop_28","alberta_in.gat",182,96,1,73,"shop_tool5")
addnpc("Item Collector","shop_47","alberta_in.gat",165,96,1,74,"shop_item2")
addnpc("Weapon Dealer","shop_70","alberta_in.gat",188,21,1,49,"shop_wep7")
addnpc("Armor Dealer","shop_82","alberta_in.gat",180,15,1,101,"shop_equip1")
--alberta_in.gat,18,59,5	shop	Miner	48,1010:-1,1011:-1

--=======================================================
--AL DE BARAN
--=======================================================
addnpc("Tool Dealer","shop_2","aldeba_in.gat",94,56,5,83,"shop_tool1")
addnpc("Weapon Dealer","shop_54","aldeba_in.gat",28,54,5,85,"shop_wep1")
addnpc("Armor Dealer","shop_92","aldeba_in.gat",20,60,5,101,"shop_equip10")

--=======================================================
--AMATSU
--=======================================================
--Fruit merchant opens a zeny exploit (due to Juice Maker NPC)
--amatsu.gat,169,111,6	shop	Tool Dealer	83,611:-1,1750:-1,501:-1,502:-1,503:-1,504:-1,506:-1,525:-1,601:-1,602:-1,1065:-1,2239:-1
addnpc("Fruit Merchant","shop_4","amatsu.gat",176,126,4,53,"shop_fruit1")
addnpc("Butcher","shop_11","amatsu.gat",189,99,7,49,"shop_meat1")
addnpc("Milk Merchant","shop_16","amatsu.gat",183,127,5,90,"shop_milk1")
addnpc("Flower Girl","shop_33","amatsu.gat",180,102,3,96,"shop_flower1")
addnpc("Gift Merchant","shop_40","amatsu.gat",169,119,6,91,"shop_gift1")
--1st set
--ama_in01.gat,96,28,4	shop	Weapon Dealer	54,1750:-1,1751:-1,1701:-1,1201:-1,1204:-1,1207:-1,1601:-1,1101:-1,1104:-1,1107:-1,1110:-1,1113:-1,1122:-1,1119:-1,1123:-1,1126:-1,1157:-1,1129:-1,1116:-1,1301:-1
--ama_in01.gat,103,24,1	shop	Armor Dealer	48,2101:-1,2103:-1,2401:-1,2403:-1,2501:-1,2503:-1,2220:-1,2226:-1,2301:-1,2303:-1,2305:-1,2328:-1,2307:-1,2309:-1,2312:-1,2314:-1
addnpc("Weapon Dealer","shop_60","ama_in01.gat",101,18,8,47,"shop_wep5")
--2nd set
addnpc("Weapon Dealer","shop_57","ama_in01.gat",102,28,4,766,"shop_wep3")
addnpc("Tool Dealer","shop_78","ama_in01.gat",24,30,4,763,"shop_tool19")
addnpc("Armor Dealer","shop_88","ama_in01.gat",89,28,4,757,"shop_equip6")

--=======================================================
--COMODO
--=======================================================
addnpc("Tool Dealer","shop_79","cmd_in01.gat",79,182,4,83,"shop_tool20")
addnpc("Weapon Dealer","shop_81","cmd_in01.gat",131,165,4,49,"shop_wep13")
addnpc("Armor Dealer","shop_91","cmd_in01.gat",117,165,4,101,"shop_equip9")
addnpc("Item Collector","shop_46","comodo.gat",244,296,7,74,"shop_item2")
addnpc("Souvenir Merchant","shop_52","comodo.gat",296,125,4,101,"shop_souv")

--=======================================================
--GEFFEN
--=======================================================
addnpc("Pet Merchant","shop_21","geffen.gat",193,152,3,125,"shop_pet1")
addnpc("Magical Tool Trade","shop_36","geffen_in.gat",77,173,1,64,"shop_magic1")
addnpc("Trader","shop_50","geffen_in.gat",74,144,1,66,"shop_trad3")
addnpc("Tool Dealer","shop_53","geffen_in.gat",171,123,1,64,"shop_tool10")
addnpc("Tool Dealer","shop_64","geffen_in.gat",77,167,1,68,"shop_tool12")
addnpc("Weapon Dealer","shop_71","geffen_in.gat",29,177,1,47,"shop_wep8")
addnpc("Armor Dealer","shop_85","geffen_in.gat",25,177,1,66,"shop_equip4")

--=======================================================
--GONRYUN
--=======================================================
addnpc("Weapon Dealer","shop_58","gonryun.gat",174,101,4,774,"shop_wep3")
addnpc("Tool Dealer","shop_72","gonryun.gat",147,84,4,777,"shop_tool17")
addnpc("Armor Dealer","shop_89","gonryun.gat",173,84,4,770,"shop_equip7")

--=======================================================
--IZLUDE
--=======================================================
addnpc("Fruit Merchant","shop_5","izlude.gat",127,120,1,72,"shop_fruit1")
addnpc("Butcher","shop_14","izlude.gat",121,138,1,54,"shop_meat3")
addnpc("Milk Merchant","shop_17","izlude.gat",137,126,3,701,"shop_milk1")
addnpc("Pet Merchant","shop_22","izlude.gat",164,138,4,125,"shop_pet1")
addnpc("Tool Dealer","shop_29","izlude_in.gat",115,61,1,47,"shop_tool6")
addnpc("Weapon Dealer","shop_73","izlude_in.gat",60,127,4,98,"shop_wep9")
addnpc("Armor Dealer","shop_86","izlude_in.gat",70,127,4,101,"shop_equip5")

--=======================================================
--LOUYANG
--=======================================================
addnpc("Weapon Dealer","shop_56","lou_in02.gat",130,182,5,774,"shop_wep2")
addnpc("Armor Dealer","shop_90","lou_in02.gat",121,181,5,770,"shop_equip8")

--=======================================================
--LUTIE
--=======================================================
addnpc("Tool Dealer","shop_27","xmas.gat",144,207,4,83,"shop_tool4")
addnpc("Tool Dealer","shop_3","xmas_in.gat",39,37,4,83,"shop_tool2")
addnpc("Weapon Dealer","shop_55","xmas_in.gat",174,98,2,49,"shop_wep1")
addnpc("Armor Dealer","shop_93","xmas_in.gat",168,104,4,101,"shop_equip10")
addnpc("Gift Merchant","shop_99","xmas_in.gat",169,34,2,702,"shop_gift2")

--=======================================================
--MORROC
--=======================================================
addnpc("Trader","shop_9","morocc.gat",139,92,1,99,"shop_banana")
addnpc("Butcher","shop_15","morocc.gat",157,72,6,49,"shop_meat3")
addnpc("Pet Merchant","shop_23","morocc.gat",269,167,4,125,"shop_pet1")
addnpc("Jewel Seller","shop_38","morocc.gat",166,51,1,102,"show_jewel1")
addnpc("Jewel Seller","shop_39","morocc.gat",145,44,1,99,"show_jewel2")
addnpc("Trader","shop_44","morocc.gat",259,193,1,99,"shop_trad1")
addnpc("Trader","shop_45","morocc.gat",268,193,1,93,"shop_trad2")
addnpc("Tool Dealer","shop_48","morocc.gat",170,101,3,85,"shop_tool9")
addnpc("Tool Dealer","shop_49","morocc.gat",206,250,1,85,"shop_tool9")
addnpc("Tool Dealer","shop_65","morocc.gat",147,102,5,93,"shop_tool13")
addnpc("Tool Dealer","shop_66","morocc.gat",151,243,1,99,"shop_tool14")
addnpc("Trader","shop_98","morocc.gat",35,69,1,89,"shop_trad5")
addnpc("Trader","shop_100","morocc.gat",244,134,1,93,"shop_trad6")
addnpc("Armor Dealer","shop_83","morocc_in.gat",141,60,1,58,"shop_equip2")
addnpc("Weapon Dealer","shop_102","morocc_in.gat",141,67,1,58,"shop_wep14")
--morocc_in.gat,65,37,4	shop	Miner	48,1010:-1,1011:-1

--=======================================================
--NIFLHEIM
--=======================================================
addnpc("Tool Dealer","shop_20","nif_in.gat",154,21,3,798,"shop_tool3")
addnpc("Weapon Dealer","shop_59","nif_in.gat",35,84,3,795,"shop_wep4")
addnpc("Armor Dealer","shop_96","nif_in.gat",35,91,3,796,"shop_equip13")
--nif_in.gat,35,84,3	shop	Weapon Dealer	795,1750:-1,1751:-1,1101:-1,1701:-1,1201:-1,1204:-1,1207:-1,1210:-1,1213:-1,1216:-1,1601:-1,1604:-1,1607:-1,1610:-1
--nif_in.gat,35,91,3	shop	Armor Dealer	796,2101:-1,2107:-1,2401:-1,2501:-1,2230:-1,2301:-1,2303:-1,2305:-1,2321:-1,2332:-1
--nif_in.gat,154,21,3	shop	Tool Dealer	798,611:-1,1750:-1,501:-1,502:-1,503:-1,504:-1,506:-1,525:-1,601:-1,602:-1,1065:-1,2239:-1,645:-1,656:-1,657:-1
--niflheim.gat,201,209,3	shop	Milk Merchant	794,519:-1 
--niflheim.gat,224,185,3	shop	Fruit Merchant	795,512:-1,513:-1 
--niflheim.gat,209,161,3	shop	Butcher	794,517:-1,528:-1 
--niflheim.gat,205,152,3	shop	Gift Merchant	795,734:-1,735:-1,736:-1,737:-1,746:-1

--=======================================================
--PAYON
--=======================================================
addnpc("Vegetable Merchant","shop_10","pay_arche.gat",132,101,1,102,"shop_veg")
addnpc("Butcher","shop_12","pay_arche.gat",140,124,1,87,"shop_meat2")
addnpc("Milk Merchant","shop_18","pay_arche.gat",125,108,1,90,"shop_milk1")
--Not Sure About Pet Shop
addnpc("Pet Merchant","shop_24","payon.gat",104,63,4,125,"shop_pet1")
addnpc("Tool Dealer","shop_67","payon.gat",160,97,4,88,"shop_tool15")
addnpc("Weapon Dealer","shop_74","payon_in01.gat",7,119,4,86,"shop_wep10")
addnpc("Armor Dealer","shop_95","payon_in01.gat",15,119,4,87,"shop_equip12")
addnpc("Tool Dealer","shop_75","payon_in02.gat",87,34,1,98,"shop_tool18")

--=======================================================
--PRONTERA
--=======================================================
addnpc("Fruit Merchant","shop_6","prontera.gat",104,49,1,102,"shop_fruit2")
addnpc("Butcher","shop_13","prontera.gat",64,125,1,87,"shop_meat2")
addnpc("Milk Merchant","shop_19","prontera.gat",73,134,1,90,"shop_milk1")
addnpc("Pet Merchant","shop_25","prontera.gat",218,211,4,125,"shop_pet1")
addnpc("Flower Girl","shop_34","prontera.gat",58,182,1,96,"shop_flower2")
addnpc("Flower Lady","shop_35","prontera.gat",113,42,1,90,"shop_flower2")
addnpc("Gift Merchant","shop_41","prontera.gat",105,87,1,91,"shop_gift1")
addnpc("Doll Merchant","shop_42","prontera.gat",248,153,1,85,"shop_doll1")
addnpc("Nun","shop_97","prt_church.gat",108,124,1,79,"shop_church")
addnpc("Tool Dealer","shop_68","prt_fild05.gat",290,221,1,83,"shop_tool16")
addnpc("Tool Dealer","shop_30","prt_in.gat",126,76,1,53,"shop_tool7")
addnpc("Wedding Goods Merchant","shop_43","prt_in.gat",211,169,1,71,"shop_wed")
addnpc("Weapon Dealer","shop_61","prt_in.gat",171,140,1,47,"shop_wep5")
addnpc("Weapon Dealer","shop_76","prt_in.gat",172,130,1,54,"shop_wep11")
addnpc("Armor Dealer","shop_84","prt_in.gat",172,132,1,48,"shop_equip3")
--prt_in.gat,56,69,4	shop	Miner	48,1010:-1,1011:-1

--=======================================================
--TURTLE ISLAND
--=======================================================
addnpc("Tool Dealer","shop_63","tur_dun01.gat",158,54,6,99,"shop_tool11")
-- Spectacles: missing as second item in list
-- Luxury Sunglases: not sure of item/price.

--=======================================================
--UMBALA
--=======================================================
addnpc("Item Merchant","shop_8","um_in.gat",104,124,3,788,"shop_item1")
addnpc("Weapon Merchant","shop_62","um_in.gat",160,125,3,789,"shop_wep6")
addnpc("Armor Merchant","shop_87","um_in.gat",151,125,4,49,"shop_equip5")
--um_in.gat,159,123,3	shop	Weapon Merchant	52,1750:-1,1751:-1,1701:-1,1601:-1,1201:-1,1204:-1,1207:-1,1101:-1,1104:-1,1107:-1,1116:-1,1151:-1,1154:-1,1157:-1,1160:-1,1301:-1
--um_in.gat,98,124,3	shop	Tool Merchant	47,611:-1,501:-1,502:-1,503:-1,504:-1,506:-1,525:-1,601:-1,602:-1,1750:-1,1065:-1
--um_in.gat,103,124,4	shop	Food Merchant	91,515:-1,516:-1,535:-1,519:-1,517:-1


--=======================================================
--YUNO
--=======================================================
addnpc("Fruit Merchant","shop_7","yuno.gat",65,122,4,93,"shop_fruit1")
addnpc("Pet Merchant","shop_26","yuno.gat",197,115,4,124,"shop_pet1")
addnpc("Tool Dealer","shop_31","yuno.gat",217,97,4,83,"shop_tool8")
addnpc("Magic Dealer","shop_37","yuno.gat",163,187,5,90,"shop_magic1")
addnpc("Trader","shop_51","yuno.gat",226,106,5,97,"shop_trad4")
addnpc("Equip Dealer","shop_94","yuno.gat",205,104,4,84,"shop_equip11")
addnpc("Tool Dealer","shop_32","yuno_in01.gat",25,34,4,83,"shop_tool8")
addnpc("Weapon Dealer","shop_77","yuno_in01.gat",104,35,4,49,"shop_wep12")
addnpc("Armor Dealer","shop_101","yuno_in01.gat",112,25,5,101,"shop_equip14")

--=======================================================
-- ST. CAPITOLINA ABBEY
--======================================
--= Monk shop, sells Knuckle weapons)
--=======================================================
addnpc("Blacksmith","shop_80","prt_monk.gat",135,263,3,726,"shop_blak")


--=======================================================
-- Shop Functions
--=======================================================

function shop_tool1()
npcshop(501,-1,502,-1,503,-1,504,-1,506,-1,601,-1,602,-1,611,-1,610,-1,645,-1,656,-1,657,-1)
end

function shop_tool2()
npcshop(502,-1,503,-1,504,-1,506,-1,611,-1,601,-1,602,-1,610,-1)
end

function shop_fruit1()
npcshop(513,-1,515,-1,516,-1)
end

function shop_fruit2()
npcshop(513,-1)
end

function shop_item1()
npcshop(15,515,15,535,15,516,15,513,15,517,50,528,60,537,1000,601,60,602,300,645,800,656,1500,610,4000)
end

function shop_banana()
npcshop(513,-1,513,-1,513,-1,513,-1,513,-1,513,-1)
end

function shop_veg()
npcshop(515,-1,516,-1)
end

function shop_meat1()
npcshop(517,-1,528,-1,540,-1,541,-1)
end

function shop_meat2()
npcshop(517,-1,528,-1)
end

function shop_meat3()
npcshop(517,-1)
end

function shop_milk1()
npcshop(519,-1)
end

function shop_tool3()
npcshop(535,-1,1062,-1,902,-1,7106,-1,537,-1,7154,-1,1052,-1,934,-1)
end

function shop_pet1()
npcshop(537,-1,643,-1,10013,-1,10014,-1)
end

function shop_tool4()
npcshop(601,-1,602,-1)
end

function shop_tool5()
npcshop(611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,1750,-1,2243,-1,645,-1,656,-1,657,-1)
end

function shop_tool6()
npcshop(611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,1750,-1,1065,-1,645,-1,656,-1,657,-1)
end

function shop_tool7()
npcshop(611,-1,1750,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,1065,-1,2239,-1,645,-1,656,-1,657,-1)
end

function shop_tool8()
npcshop(611,-1,1750,-1,501,-1,502,-1,503,-1,504,-1,505,-1,506,-1,645,-1,656,-1,601,-1,602,-1)
end

function shop_flower1()
npcshop(712,-1,744,-1,2612,-1,2215,-1)
end

function shop_flower2()
npcshop(712,-1,744,-1)
end

function shop_magic1()
npcshop(717,-1,1601,-1,1604,-1,1607,-1,1610,-1,2232,-1,2321,-1,2332,-1)
end

function show_jewel1()
npcshop(721,-1,723,-1,726,-1,728,-1,729,-1)
end

function show_jewel2()
npcshop(730,-1,2613,-1)
end

function shop_gift1()
npcshop(734,-1,735,-1,736,-1,737,-1,746,-1)
end

function shop_doll1()
npcshop(740,-1,741,-1,742,-1)
end

function shop_wed()
npcshop(744,-1,745,-1,2338,-1,2206,-1,7170,-1)
end

function shop_trad1()
npcshop(747,-1)
end

function shop_trad2()
npcshop(748,-1)
end

function shop_item2()
npcshop(909,-1,528,-1)
end

function shop_tool9()
npcshop(909,-1,528,-1,919,-1,925,-1)
end

function shop_trad3()
npcshop(909,-1,911,-1,910,-1,912,-1)
end

function shop_trad4()
npcshop(911,-1,910,-1,912,-1)
end

function shop_souv()
npcshop(965,-1,964,-1)
end

function shop_tool10()
npcshop(1092,-1,1093,-1)
end

function shop_wep1()
npcshop(1201,-1,1204,-1,1207,-1,1210,-1,1213,-1,1216,-1,1219,-1,1222,-1)
end

function shop_wep2()
npcshop(1204,-1,1216,-1,1107,-1,1113,-1,1116,-1,1157,-1,1407,-1,1410,-1,1354,-1,1519,-1)
end

function shop_wep3()
npcshop(1207,-1,1216,-1,1107,-1,1122,-1,1116,-1,1154,-1,1407,-1,1457,-1,1354,-1,1519,-1)
end

function shop_wep4()
npcshop(1301,-1,1351,-1,1354,-1,1357,-1,1360,-1)
end

function shop_wep5()
npcshop(1401,-1,1404,-1,1407,-1,1451,-1,1454,-1,1457,-1,1460,-1,1463,-1,1410,-1)
end

function shop_wep6()
npcshop(1501,120,1504,1600,1507,9000,1510,16000,1513,41000,1519,23000,1807,53000,1811,58000,1809,67000)
end

function shop_tool11()
npcshop(1750,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,645,-1,656,-1,657,-1)
end

function shop_tool12()
npcshop(1750,-1,611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,2241,-1,645,-1,656,-1,657,-1)
end

function shop_tool13()
npcshop(1750,-1,611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,1065,-1,645,-1,656,-1,657,-1)
end

function shop_tool14()
npcshop(1750,-1,611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,2242,-1,645,-1,656,-1,657,-1)
end

function shop_tool15()
npcshop(1750,-1,611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,601,-1,525,-1,602,-1,1065,-1,645,-1,656,-1,657,-1
)
end

function shop_tool16()
npcshop(1750,-1,611,-1,501,-1,502,-1,506,-1,601,-1,602,-1,645,-1,656,-1,657,-1
)
end

function shop_tool17()
npcshop(1750,-1,1751,-1,1752,-1,501,-1,502,-1,503,-1,504,-1,506,-1,645,-1,656,-1,657,-1)
end

function shop_wep7()
npcshop(1750,-1,1751,-1,1101,-1,1104,-1,1107,-1,1201,-1,1204,-1,1207,-1,1601,-1,1701,-1,1301,-1,1351,-1,1354,-1,1357,-1,1360,-1)
end

function shop_wep8()
npcshop(1750,-1,1751,-1,1101,-1,1701,-1,1201,-1,1204,-1,1207,-1,1210,-1,1213,-1,1216,-1,1601,-1,1604,-1,1607,-1,1610,-1)
end

function shop_tool17()
npcshop(1750,-1,1751,-1,611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,645,-1,656,-1,601,-1,602,-1,1065,-1)
end

function shop_wep9()
npcshop(1750,-1,1751,-1,1701,-1,1601,-1,1201,-1,1204,-1,1207,-1,1101,-1,1104,-1,1107,-1,1116,-1,1151,-1,1154,-1,1157,-1,1160,-1,1301,-1)
end

function shop_wep10()
npcshop(1750,-1,1751,-1,1101,-1,1104,-1,1107,-1,1201,-1,1204,-1,1207,-1,1601,-1,1701,-1,1704,-1,1707,-1,1710,-1,1713,-1,1714,-1,1718,-1)
end

function shop_tool18()
npcshop(1750,-1,1751,-1,611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,525,-1,601,-1,602,-1,1065,-1,645,-1,656,-1,657,-1)
end

function shop_wep11()
npcshop(1750,-1,1751,-1,1701,-1,1201,-1,1204,-1,1207,-1,1601,-1,1101,-1,1104,-1,1107,-1,1110,-1,1113,-1,1122,-1,1119,-1,1123,-1,1126,-1,1157,-1,1129,-1,1116,-1,1301,-1)
end

function shop_wep12()
npcshop(1750,-1,1751,-1,1101,-1,1701,-1,1201,-1,1204,-1,1207,-1,1210,-1,1213,-1,1216,-1,1601,-1,1604,-1,1607,-1,1610,-1)
end

function shop_tool19()
npcshop(1750,-1,1770,-1,611,-1,501,-1,502,-1,503,-1,504,-1,506,-1,645,-1,656,-1,601,-1,602,-1,1065,-1)
end

function shop_tool20()
npcshop(1753,-1,501,-1,502,-1,503,-1,504,-1,645,-1,656,-1,657,-1,601,-1,602,-1,611,-1,1065,-1)
end

function shop_blak()
npcshop(1801,8000,1803,25000,1805,32000)
end

function shop_wep13()
npcshop(1903,-1,1905,-1,1907,-1,1950,-1,1952,-1,1954,-1,1956,-1,1401,-1,1404,-1,1407,-1,1451,-1,1454,-1,1457,-1,1460,-1,1463,-1,1410,-1)
end

function shop_equip1()
npcshop(2101,-1,2103,-1,2401,-1,2403,-1,2405,-1,2501,-1,2503,-1,2505,-1,2203,-1,2201,-1,2205,-1,2226,-1,2301,-1,2303,-1,2305,-1,2321,-1,2328,-1,2332,-1,2307,-1,2309,-1,2312,-1,2314,-1)
end

function shop_equip2()
npcshop(2101,-1,2103,-1,2401,-1,2403,-1,2405,-1,2501,-1,2503,-1,2218,-1,2301,-1,2303,-1,2305,-1,2321,-1,2328,-1,2332,-1,2307,-1,2309,-1,2335,-1)
end

function shop_equip3()
npcshop(2101,-1,2103,-1,2401,-1,2403,-1,2501,-1,2503,-1,2220,-1,2226,-1,2301,-1,2303,-1,2305,-1,2328,-1,2307,-1,2309,-1,2312,-1,2314,-1)
end

function shop_equip4()
npcshop(2101,-1,2107,-1,2401,-1,2501,-1,2230,-1,2301,-1,2303,-1,2305,-1,2321,-1,2332,-1)
end

function shop_equip5()
npcshop(2103,-1,2105,-1,2403,-1,2405,-1,2503,-1,2505,-1,2226,-1,2228,-1,2303,-1,2305,-1,2328,-1,2307,-1,2309,-1,2312,-1,2314,-1,2316,-1)
end

function shop_equip6()
npcshop(2211,-1,2401,-1,2403,-1,2501,-1,2503,-1,2101,-1,2103,-1,2305,-1,2321,-1,2332,-1,2314,-1,2627,-1)
end

function shop_equip7()
npcshop(2211,-1,2401,-1,2403,-1,2501,-1,2503,-1,2101,-1,2103,-1,2305,-1,2321,-1,2332,-1,2328,-1,2627,-1)
end

function shop_equip8()
npcshop(2211,-1,2401,-1,2403,-1,2501,-1,2503,-1,2101,-1,2103,-1,2503,-1)
end

function shop_equip9()
npcshop(2226,-1,2228,-1,2103,-1,2105,-1,2405,-1,2503,-1,2505,-1,2305,-1,2321,-1,2307,-1,2309,-1,2335,-1,2312,-1,2314,-1,2316,-1)
end

function shop_equip10()
npcshop(2228,-1,2103,-1,2105,-1,2307,-1,2309,-1,2312,-1,2314,-1,2316,-1,2505,-1,2405,-1)
end

function shop_equip11()
npcshop(2340,-1,2341,-1,2411,-1,2222,-1,2230,-1,1721,-1)
end

function shop_equip12()
npcshop(2401,-1,2403,-1,2405,-1,2501,-1,2503,-1,2505,-1,2208,-1,2211,-1,2212,-1,2301,-1,2303,-1,2305,-1,2321,-1,2328,-1,2332,-1,2309,-1,2330,-1)
end

function shop_equip13()
npcshop(2501,-1,2501,-1,2503,-1,2503,-1,2505,-1,2505,-1)
end

function shop_church()
npcshop(2608,-1,2216,-1,2323,-1,2325,-1,1501,-1,1504,-1,1507,-1,1510,-1,1513,-1,1519,-1)
end

function shop_trad5()
npcshop(2609,-1,1516,-1,1522,-1)
end

function shop_gift2()
npcshop(2612,-1,744,-1,748,-1,736,-1,746,-1,740,-1,2613,-1)
end

function shop_trad6()
npcshop(2612,-1)
end

function shop_equip14()
npcshop(2628,-1,2101,-1,2107,-1,2401,-1,2501,-1,2230,-1,2301,-1,2303,-1,2305,-1,2321,-1,2332,-1)
end

function shop_wep14()
npcshop(1750,-1,1751,-1,1701,-1,1601,-1,1201,-1,1204,-1,1207,-1,1210,-1,1213,-1,1216,-1,1219,-1,1222,-1,1250,-1,1252,-1,1254,-1)
end