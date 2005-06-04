--[[
	addnpc("Healer Dog","prontera.gat",100,100,4,81,"npc_healer")
	addnpc("Healer Dog","payon.gat",100,100,4,81,"npc_healer")
	addnpc("Merchant Dog","prontera.gat",101,100,4,81,"npc_merchant")
	addnpc("Merchant Dog","payon.gat",101,100,4,81,"npc_merchant")
]]

function npc_healer()
	npcmes "[Healer Dog]"
	heal()
	npcmes "Here you are!"
	close()
end

function npc_merchant()
	npcshop(501,-1,502,-1)
end

print "NPC example successfully loaded !"