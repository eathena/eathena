addnpc("Healer Dog","healer_ald","aldebaran.gat",135,115,4,81,"npc_healer")
addnpc("Healer Dog","healer_gef","geffen.gat",125,95,4,81,"npc_healer")
addnpc("Healer Dog","healer_moc","morocc.gat",160,100,4,81,"npc_healer")
addnpc("Healer Dog","healer_prt","prontera.gat",155,190,4,81,"npc_healer")

function npc_healer()
	npcmes "[Healer Dog]"
	npcmes "Would you like a heal?"
	npcnext()
	local r = npcmenu("Sure",1,"I prefer potions",2,"eh",3)
	if r == 1 then
		npcmes "[Healer Dog"
		npcmes "How much HP and SP would you like?"
		npcnext() 
		heal(npcinput(),npcinput())
		npcmes "[Healer Dog]"
		npcmes "Here you go!"
		npcclose()
	elseif r == 2 then
		npcmes "[Healer Dog]"
		npcmes "Oh, right, I have some potions to sell!"
		npcclose()
		npcshop(501,50,502,100)
	elseif r == 3 then
		npcmes "[Healer Dog]"
		npcmes "Sorry to hear that..."
		npcclose()
	end
end

addnpc("Healer Dog","healer_nif","niflheim.gat",195,195,4,81,"npc_biter")
addareascript("Bite Area","niflheim.gat",190,190,200,200,"areascript_biter")

function npc_biter()
	npcmes "[Healer Dog]"
	npcmes "Come, come closer so I can heal you!"
	npcnext()
	npcmes "This dog looks ferocious... I should better keep away... How far?"
	npcnext()
	npcmes("Yes, I think I need to be at least "..npcinput().." "..npcinput(1).." away from him to be safe.")
	npcclose()
end

function areascript_biter()
	percentheal(-50,-50)
end


-- Sample of the Stylist, all commands now exist to create it :)

addnpc("Stylist","StylistPront","prontera.gat",170,180,1,122,"stylemain")

function stylemain()
	npcmes "[^FF8000Stylist^000000]"
	npcmes "I'm the greatest stylist in all of Rune-Midgard~~!"
	npcmes "I can change your hair style or color!"
	npcmes "What do you wish to change?"
	npcnext()
	local r = npcmenu("Hair style",1)
--"Hair color",2,"Cloth Color",3,"Nothing",4)

	if r == 1 then
		npcmes "[^FF8000Stylist^000000]"
		npcmes "Do you want to browse through the choices, or do you know what you want?"
		npcnext()
		local r = npcmenu("Browse",1,"I know what I want",2)
		if r == 2 then
			loop = true
			while loop==true do
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
					loop = false
					break
				elseif r == 2 then
					npcnext()
				end
			end
		elseif r == 1 then
			loop = true
			while loop==true do
				npcmes "[^FF8000Stylist^000000]"
				npcmes "Okay, here we go~! Just stop me when you see something you like, okay?"
				npcnext()
				for i=1,23 do

					npcmes("This is Style Number^FF9009 "..i.." ^000000!")
					setlook(1,i)
					if i < 22 then
						local r = npcmenu("Keep going",1,"I like this one",2)
						if r == 2 then 
							npcclose()
							loop = false
							i = 24
						end
					else
						local r = npcmenu("Back To The Begin",1,"I like this one",2)
						if r == 1 then
							npcnext()
						else	
							npcclose()
							loop = false
							i = 24
						end
					end
				

					
				end
			end
		end
	end
	npcclose()
end