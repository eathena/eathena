function npcmenu(id, ...)
	local i
	local r
	for i = 1, arg["n"], 2 do
		npcmenu_co(id, arg[i], arg[i+1])
	end
	npcmenu_done(id)
	r = npcmenu_getchoice(id)
	return r
end