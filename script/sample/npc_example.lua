addnpc("Healer Dog","","prontera.gat",100,100,4,81,"npc_healer")

function npc_healer(id)
	npcmes(id,"[Healer Dog]")
	heal(id, 25, 25)
	npcmes(id,"Here you are!")
	close(id)
end

print "NPC example successfully loaded !"