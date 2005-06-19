addnpc("Healer Dog","healer_ald","aldebaran.gat",135,115,4,81,"npc_healer")
addnpc("Healer Dog","healer_gef","geffen.gat",125,95,4,81,"npc_healer")
addnpc("Healer Dog","healer_moc","morocc.gat",160,100,4,81,"npc_healer")
addnpc("Healer Dog","healer_prt","prontera.gat",155,190,4,81,"npc_healer")

function npc_healer()
	npcmes("[Healer Dog]")
	npcmes("Would you like a heal?")
	npcnext()
	local r = npcmenu("Sure",1,"eh",2)
	if r == 1 then
		npcmes("[Healer Dog")
		npcmes("How much hp and sp would you like?")
		npcnext()
		local amount = npcinput()
		heal(amount,amount)
		npcmes("[Healer Dog")
		npcmes("Hear you go!")
	elseif r ==2 then
		npcmes("[Healer Dog]")
		npcmes("Sorry to hear that")
	end
	npcclose(id)
end

addnpc("Healer Dog","healer_nif","niflheim.gat",195,195,4,81,"npc_biter")
addareascript("Bite Area","niflheim.gat",190,190,200,200,"areascript_biter")

function npc_biter()
	npcmes("[Healer Dog]")
	npcmes("Come, come closer so I can heal you!")
	npcclose()
end

function areascript_biter()
	percentheal(-50, -50)
end