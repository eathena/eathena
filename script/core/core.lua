function npcmenu(...)
	local r
	npcmenu_co(arg)
	r = npcmenu_getchoice()
	return r
end

function npcinput(in_type)
	local r
	npcinput_co(in_type)
	r = npcinput_getvalue(in_type)
	return r
end

function npcshop(...)
	npcshop_start()
	npcshop_co(arg)
end