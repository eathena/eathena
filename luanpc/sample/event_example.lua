function PCLoginEvent()
	print("Player with ID "..char_id.." logged in !")
end

function PCLogoutEvent()
	print("Player with ID "..char_id.." logged out !")
end

function PCKillEvent()
	print("Player with ID "..char_id.." killed someone !")
end

function PCDieEvent()
	print("Player with ID "..char_id.." died !")
end