function npcmenu(...)
	local i
	local r
	for i = 1, arg["n"], 2 do
		npcmenu_co(arg[i], arg[i+1])
	end
	npcmenu_done()
	r = npcmenu_getchoice()
	return r
end