
##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################


##########################################################
RETURN_PATH ?= .
COMMON_PATH ?= ../common
BASICS_PATH ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT -I$(RETURN_PATH)/$(BASICS_PATH)  -I$(RETURN_PATH)/$(COMMON_PATH)
LIBS        ?= -lpthread -ldl -lrt -lz
##########################################################
include $(RETURN_PATH)/$(BASICS_PATH)/Makefile.dep
include $(RETURN_PATH)/$(COMMON_PATH)/Makefile.dep
##########################################################

OBJECTS = map.o chrif.o coordinate.o fightable.o movable.o pc.o npc.o chat.o path.o script.o storage.o skill.o atcommand.o battle.o intif.o trade.o party.o vending.o guild.o pet.o homun.o status.o npclisten.o mercenary.o config.o clif.o itemdb.o log.o mob.o oldskill.o oldstatus.o skillunit.o
EXTLIBS = $(COMMON_STATICLIB) $(BASICS_STATICLIB)

all map map-server: $(RETURN_PATH)/map-server

$(EXTLIBS):
	@echo "$(EXTLIBS) not up to date, rebuild them"

$(RETURN_PATH)/map-server: $(OBJECTS) $(EXTLIBS)
	$(CCC) $^ -lm $(LIBS) -o $@

atcommand.o: atcommand.cpp atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h config.h storage.h guild.h pet.h log.h $(COMMON_H) $(BASICS_H)
battle.o: battle.cpp battle.h config.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H) $(BASICS_H)
chat.o: chat.cpp map.h clif.h pc.h chat.h $(COMMON_H) $(BASICS_H)
chrif.o: chrif.cpp map.h battle.h config.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H) $(BASICS_H)
clif.o: clif.cpp map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h $(COMMON_H) $(BASICS_H)
config.o: config.cpp config.h $(COMMON_H) $(BASICS_H)
coordinate.o: coordinate.cpp coordinate.h $(COMMON_H) $(BASICS_H)
fightable.o: fightable.cpp fightable.h movable.h map.h $(COMMON_H) $(BASICS_H)
guild.o: guild.cpp guild.h storage.h battle.h config.h clif.h intif.h pc.h npc.h map.h $(COMMON_H) $(BASICS_H)
homun.o: homun.cpp homun.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h config.h mob.h npc.h script.h $(COMMON_H) $(BASICS_H)
intif.o: intif.cpp intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h config.h pet.h $(COMMON_H) $(BASICS_H)
itemdb.o: itemdb.cpp itemdb.h battle.h config.h pc.h $(COMMON_H) $(BASICS_H)
log.o: log.cpp log.h map.h $(COMMON_H) $(BASICS_H)
map.o: map.cpp map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h $(COMMON_H) $(BASICS_H)
mercenary.o: mercenary.cpp mercenary.h $(COMMON_H) $(BASICS_H)
mob.o: mob.cpp map.h clif.h intif.h pc.h mob.h skill.h battle.h config.h npc.h itemdb.h $(COMMON_H) $(BASICS_H)
movable.o: movable.cpp movable.h map.h $(COMMON_H) $(BASICS_H)
npc.o: npc.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h config.h $(COMMON_H) $(BASICS_H)
npclisten.o: npclisten.cpp npclisten.h map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h config.h $(COMMON_H) $(BASICS_H)
party.o: party.cpp party.h clif.h intif.h pc.h map.h battle.h config.h $(COMMON_H) $(BASICS_H)
path.o: path.cpp map.h battle.h config.h $(COMMON_H) $(BASICS_H)
pc.o: pc.cpp map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h config.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h $(COMMON_H) $(BASICS_H)
pet.o: pet.cpp pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h config.h mob.h npc.h script.h $(COMMON_H) $(BASICS_H)
script.o: script.cpp itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h config.h log.h $(COMMON_H) $(BASICS_H)
skill.o: skill.cpp skill.h map.h clif.h pc.h mob.h battle.h config.h itemdb.h script.h $(COMMON_H) $(BASICS_H)
skillunit.o: skillunit.cpp skill.h map.h clif.h pc.h mob.h battle.h config.h itemdb.h script.h $(COMMON_H) $(BASICS_H)
status.o: status.cpp status.h clif.h intif.h pc.h map.h battle.h config.h $(COMMON_H) $(BASICS_H)
storage.o: storage.cpp itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H) $(BASICS_H)
trade.o: trade.cpp trade.h clif.h itemdb.h map.h pc.h npc.h $(COMMON_H) $(BASICS_H)
vending.o: vending.cpp vending.h clif.h itemdb.h map.h pc.h $(COMMON_H) $(BASICS_H)
oldskill.o: oldskill.cpp skill.h map.h clif.h pc.h mob.h battle.h config.h itemdb.h script.h $(COMMON_H) $(BASICS_H)
oldstatus.o: oldstatus.cpp status.h clif.h intif.h pc.h map.h battle.h config.h $(COMMON_H) $(BASICS_H)


clean:
	rm -rf *.o $(RETURN_PATH)/map-server GNUmakefile
