addnpc("Healer Dog","healer_ald","aldebaran.gat",135,115,4,81,"npc_healer")
addnpc("Healer Dog","healer_gef","geffen.gat",125,95,4,81,"npc_healer")
addnpc("Healer Dog","healer_moc","morocc.gat",160,100,4,81,"npc_healer")
addnpc("Healer Dog","healer_prt","prontera.gat",155,190,4,81,"npc_healer")

function npc_healer(id)
	npcmes(id,"[Healer Dog]")
	npcmes(id,"Would you like a heal?")
	npcnext(id)
	local r = npcmenu(id,"Sure",1,"eh",2)
	if r == 1 then
		percentheal(id, 50, 50)
		npcmes(id,"[Healer Dog")
		npcmes(id,"Hear you go!")
	elseif r ==2 then
		npcmes(id,"[Healer Dog]")
		npcmes(id,"Sorry to hear that")
	end
	npcclose(id)
end

addnpc("Healer Dog","healer_nif","niflheim.gat",195,195,4,81,"npc_biter")
addareascript("Bite Area","niflheim.gat",190,190,200,200,"areascript_biter")

function npc_biter(id)
	npcmes(id,"[Healer Dog]")
	npcmes(id,"Come, come closer so I can heal you!")
	npcclose(id)
end

function areascript_biter(id)
	percentheal(id, -50, -50)
end