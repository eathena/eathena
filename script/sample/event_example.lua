function PCLoginEvent(id)
	print("Player with ID "..id.." logged in !")
end

function PCLogoutEvent(id)
	print("Player with ID "..id.." logged out !")
end

function PCKillEvent(id)
	print("Player with ID "..id.." killed someone !")
end

function PCDieEvent(id)
	print("Player with ID "..id.." died !")
end