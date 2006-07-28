all: txt sql
txt: txtobj map-server
sql: txtobj sqlobj map-server_sql

txtobj:
	mkdir txtobj
sqlobj:
	mkdir sqlobj

##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################


##########################################################
RETURN_PATH ?= .
COMMON_PATH ?= ../common
BASE_PATH   ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
include $(BP)/Makefile.dep

##########################################################
CP = $(RETURN_PATH)/$(COMMON_PATH)
##########################################################
include $(CP)/Makefile.dep

COMMON_OBJ += $(CP)/grfio.o
COMMON_H   += $(CP)/grfio.h

MAP_SQL = sqlobj/itemdb.o sqlobj/log.o sqlobj/mob.o 
MAP_TXT = txtobj/itemdb.o txtobj/log.o txtobj/mob.o 
MAP_OBJ = txtobj/map.o txtobj/chrif.o txtobj/coordinate.o txtobj/fightable.o txtobj/movable.o txtobj/pc.o txtobj/npc.o txtobj/chat.o txtobj/path.o txtobj/script.o txtobj/storage.o txtobj/skill.o txtobj/atcommand.o txtobj/charcommand.o txtobj/battle.o txtobj/intif.o txtobj/trade.o txtobj/party.o txtobj/vending.o txtobj/guild.o txtobj/pet.o txtobj/homun.o txtobj/status.o txtobj/npc_chat.o txtobj/mercenary.o txtobj/config.o txtobj/clif.o

map-server: $(MAP_TXT) $(MAP_OBJ) $(COMMON_TXT) $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> -lm $(LIBS) -o $(RETURN_PATH)/$@

map-server_sql: $(MAP_SQL) $(MAP_OBJ) $(COMMON_SQL) $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> -lm $(LIBS) $(LIBS) -o $(RETURN_PATH)/$@

txtobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
sqlobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<


txtobj/atcommand.o: atcommand.cpp atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h config.h storage.h guild.h pet.h log.h $(COMMON_H) $(BASE_H)
txtobj/battle.o: battle.cpp battle.h config.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H) $(BASE_H)
txtobj/charcommand.o: charcommand.cpp charcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h config.h storage.h guild.h pet.h log.h $(COMMON_H) $(BASE_H)
txtobj/chat.o: chat.cpp map.h clif.h pc.h chat.h $(COMMON_H) $(BASE_H)
txtobj/chrif.o: chrif.cpp map.h battle.h config.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H) $(BASE_H)
txtobj/clif.o: clif.cpp map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h charcommand.h $(COMMON_H) $(BASE_H)
txtobj/config.o: config.cpp config.h $(COMMON_H) $(BASE_H)
txtobj/coordinate.o: coordinate.cpp coordinate.h $(COMMON_H) $(BASE_H)
txtobj/fightable.o: fightable.cpp fightable.h movable.h map.h $(COMMON_H) $(BASE_H)
txtobj/guild.o: guild.cpp guild.h storage.h battle.h config.h clif.h intif.h pc.h npc.h map.h $(COMMON_H) $(BASE_H)
txtobj/homun.o: homun.cpp homun.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h config.h mob.h npc.h script.h $(COMMON_H) $(BASE_H)
txtobj/intif.o: intif.cpp intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h config.h pet.h $(COMMON_H) $(BASE_H)
txtobj/itemdb.o: itemdb.cpp itemdb.h battle.h config.h pc.h $(COMMON_H) $(BASE_H)
txtobj/log.o: log.cpp log.h map.h $(COMMON_H) $(BASE_H)
txtobj/map.o: map.cpp map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h $(COMMON_H) $(BASE_H)
txtobj/mercenary.o: mercenary.cpp mercenary.h $(COMMON_H) $(BASE_H)
txtobj/mob.o: mob.cpp map.h clif.h intif.h pc.h mob.h skill.h battle.h config.h npc.h itemdb.h $(COMMON_H) $(BASE_H)
txtobj/movable.o: movable.cpp movable.h map.h $(COMMON_H) $(BASE_H)
txtobj/npc.o: npc.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h config.h $(COMMON_H) $(BASE_H)
txtobj/npc_chat.o: npc_chat.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h config.h $(COMMON_H) $(BASE_H)
txtobj/party.o: party.cpp party.h clif.h intif.h pc.h map.h battle.h config.h $(COMMON_H) $(BASE_H)
txtobj/path.o: path.cpp map.h battle.h config.h $(COMMON_H) $(BASE_H)
txtobj/pc.o: pc.cpp map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h config.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h $(COMMON_H) $(BASE_H)
txtobj/pet.o: pet.cpp pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h config.h mob.h npc.h script.h $(COMMON_H) $(BASE_H)
txtobj/script.o: script.cpp itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h config.h log.h $(COMMON_H) $(BASE_H)
txtobj/skill.o: skill.cpp skill.h map.h clif.h pc.h mob.h battle.h config.h itemdb.h script.h $(COMMON_H) $(BASE_H)
txtobj/status.o: status.cpp status.h clif.h intif.h pc.h map.h battle.h config.h $(COMMON_H) $(BASE_H)
txtobj/storage.o: storage.cpp itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H) $(BASE_H)
txtobj/trade.o: trade.cpp trade.h clif.h itemdb.h map.h pc.h npc.h $(COMMON_H) $(BASE_H)
txtobj/vending.o: vending.cpp vending.h clif.h itemdb.h map.h pc.h $(COMMON_H) $(BASE_H)

sqlobj/itemdb.o: itemdb.cpp itemdb.h battle.h config.h pc.h $(COMMON_H) $(BASE_H)
sqlobj/log.o: log.cpp log.h map.h $(COMMON_H) $(BASE_H)
sqlobj/mob.o: mob.cpp map.h clif.h intif.h pc.h mob.h skill.h battle.h config.h npc.h itemdb.h $(COMMON_H) $(BASE_H)


clean:
	rm -rf *.o $(RETURN_PATH)/map-server $(RETURN_PATH)/map-server_sql sqlobj txtobj obj GNUmakefile
