--===== eAthena Script ======================================= 
--= Stylist Script
--===== By: ================================================== 
--= eAthena Dev team
--= Revised by Nekosume [pyRO v3.0] 
--===== Current Version: ===================================== 
--= 5.0
--===== Compatible With: ===================================== 
--= Lua eAthena Version 
--===== Description: ========================================= 
--= Revised dye NPC
--===== Additional Comments: =================================
--= v5.0 - Converted to Lua [Fredzilla]
--= v4.1 - New hairstyles added [Mass Zero]
--= v4.0 - Refined and Combined [Darkchild]
--= v3.0 - Added the 'Browse' options
--= v2.5 - Added more hair colors
--= v2.0 - Changed palette and hair style select
--= v1.5 - Revised script / different dialog
--= v1.0 - Split into two NPCs
--============================================================ 


--Stylist-----------------------------------------------------
addnpc("Stylist","StylistPront","prontera.gat",170,180,1,122,"stylemain")

function stylemain()
	npcmes "[^FF8000Stylist^000000]"
	npcmes "I'm the greatest stylist in all of Rune-Midgard~~!"
	npcmes "I can change your hair style or color!"
	npcmes "What do you wish to change?"
	npcnext()
	local r = npcmenu("Hair style",1,"Hair color",2,"Cloth Color",3,"Nothing",4)
	if r == 1 then 
		stylehairs()
	elseif r == 2 then
		stylehairc()
	elseif r == 3 then
		styleclothc()
	else
		npcmes  "[^FF8000Stylist^000000]"
		npcmes  "Well come again."
		npcclose()
	end
end

function stylehairs()
	npcmes "[^FF8000Stylist^000000]"
	npcmes "Do you want to browse through the choices, or do you know what you want?"
	npcnext()
	local r = npcmenu("Browse",1,"I know what I want",2)
	if r == 2 then
		while true do
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Great! Now just pick a style and I'll get started!"
			npcnext()
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Please pick a style number ^0000FFbetween 0 and 23^000000."
			npcmes "Number 0, by the way, is the default style for your character."
			npcnext()
			local sty = npcinput()
			if sty>23 then npcclose() end
			if sty<0 then npcclose() end
			setlook(1,sty)
			npcnext()
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Is this good, or do you want a different style?"
			npcnext()
			local r = npcmenu("This is good",1,"Different style, please",2)
			if r == 1 then
				npcmes "[^FF8000Stylist^000000]"
				npcmes "You look great~! Come back again, okay?"
				npcclose()
				return
			elseif r == 2 then
				npcnext()
			end
		end
	elseif r == 1 then
		while true do
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Okay, here we go~! Just stop me when you see something you like, okay?"
			npcnext()
			for i=1,23 do
				npcmes("This is Style Number^FF9009 "..i.." ^000000!")
				setlook(1,i)
				if i < 23 then
					local r = npcmenu("Keep going",1,"I like this one",2)
					if r == 2 then 
						npcclose()
						return
					end
				else
					local r = npcmenu("Back To The Begin",1,"I like this one",2)
					if r == 1 then
						i = 0
						npcnext()
					else	
						npcclose()
						return
					end
				end
			end
		end
	end
end

function stylehairc()
	npcmes  "[^FF8000Stylist^000000]"
	npcmes  "Do you want to browse through the choices, or do you know what you want?"
	npcnext()
	local r = npcmenu ("Browse",1,"I know what I want",2)
	if r == 2 then
		while true do
			npcmes  "[^FF8000Stylist^000000]"
			npcmes  "Just pick a color and we can get started."
			npcnext()
			npcmes  "[^FF8000Stylist^000000]"
			npcmes  "Please pick a style number ^0000FFbetween 0 and 20^000000."
			npcmes  "Number 0, by the way, is the default color for your character."
	 		npcnext()
			npcmes  "[^FF8000Stylist^000000]"
			npcmes  "0 is default..."
			npcmes  "1 is blonde..."
			npcmes  "2 is lavender..."
			npcmes  "3 is brown..."
			npcmes  "4 is green..."
			npcmes  "5 is blue..."
			npcmes  "6 is white..."
			npcmes  "7 is black..."
			npcmes  "8 is red..."
			npcmes  "and 9-20 are new colors."
			local color = npcinput()
			if color>20 then npcclose() end
			if color<0 then npcclose() end
			setlook(6,color)
			npcnext()
			npcmes  "[^FF8000Stylist^000000]"
			npcmes  "Is this good, or do you want a different color?"
			npcnext()
			local r = npcmenu("This is good",1,"Different color, please",2)
			if r == 1 then
				npcmes  "[^FF8000Stylist^000000]"
				npcmes  "You look great~!  Come back again, okay?"
				npcclose()
				return
			else
				npcnext()
			end
		end
	elseif r == 1 then
		while true do
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Okay, here we go~! Just stop me when you see something you like, okay?"
			npcnext()
			for i=1,20 do
				npcmes("This is Palette Number^FF9009 "..i.." ^000000!")
				setlook(6,i)
				if i < 20 then
					local r = npcmenu("Keep going",1,"I like this one",2)
					if r == 2 then 
						npcclose()
						return
					end
				else
					local r = npcmenu("Back To The Begin",1,"I like this one",2)
					if r == 1 then
						i = 0
						npcnext()
					else	
						npcclose()
						return
					end
				end
			end
		end
	end
end
function styleclothc()
	npcmes "[^FF8000Stylist^000000]"
	npcmes "Do you want to browse through the choices, or do you know what you want?"
	npcnext()
	local r = npcmenu("Browse",1,"I know what I want",2)
	if r == 2 then
		while true do
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Great! Now just pick a palette and I'll get started!"
			npcnext()
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Please pick a palette number ^0000FFbetween 0 and 77^000000."
			npcmes "Number 0, by the way, is the default palette for your character."
			npcnext()
			local clo = npcinput()
			if clo>77 then npcclose() end
			if clo<0 then npcclose() end
			setlook(7,clo)
			npcnext()
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Is this good, or do you want a different style?"
			npcnext()
			local r = npcmenu("This is good",1,"Different style, please",2)
			if r == 1 then
				npcmes "[^FF8000Stylist^000000]"
				npcmes "You look great~! Come back again, okay?"
				npcclose()
				return
			elseif r == 2 then
				npcnext()
			end
		end
	elseif r == 1 then
		while true do
			npcmes "[^FF8000Stylist^000000]"
			npcmes "Okay, here we go~! Just stop me when you see something you like, okay?"
			npcnext()
			for i=1,77 do
				npcmes("This is Style Number^FF9009 "..i.." ^000000!")
				setlook(7,i)
				if i < 77 then
					local r = npcmenu("Keep going",1,"I like this one",2)
					if r == 2 then 
						npcclose()
						return
					end
				else
					local r = npcmenu("Back To The Begin",1,"I like this one",2)
					if r == 1 then
						i = 0
						npcnext()
					else	
						npcclose()
						return
					end
				end
			end
		end
	end
end