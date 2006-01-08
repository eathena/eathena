all: txt sql
txt: txtobj map-server
sql: sqlobj map-server_sql

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

LIBS = -lz -lm

##########################################################
RETURN_PATH ?= .
BASE_PATH   ?= ../basics
COMMON_PATH ?= ../common
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
BASE_OBJ = $(BP)/basealgo.o $(BP)/basearray.o $(BP)/basecharset.o $(BP)/basefile.o $(BP)/baseheaps.o $(BP)/basememory.o $(BP)/baseobjects.o $(BP)/baseparser.o $(BP)/basestring.o $(BP)/basesync.o $(BP)/basetime.o $(BP)/basetypes.o
#not in this package
#$(BP)/basecomponent.o $(BP)/baseinet.o $(BP)/baselib.o $(BP)/baseparam.o $(BP)/baseregex.o $(BP)/basestreams.o $(BP)/basethreads.o
##########################################################
BASE_H = $(BP)/basealgo.h $(BP)/basearray.h $(BP)/basecharset.h $(BP)/baseexceptions.h $(BP)/basefile.h $(BP)/baseheaps.h $(BP)/basememory.h $(BP)/baseministring.h $(BP)/baseobjects.h $(BP)/baseparser.h $(BP)/basepool.h $(BP)/basesafeptr.h $(BP)/basestring.h $(BP)/basesync.h $(BP)/basetime.h $(BP)/basetypes.h 
#not in this package
# $(BP)/basecomponent.h $(BP)/baseinet.h $(BP)/baselib.h $(BP)/baseparam.h $(BP)/baseregex.h $(BP)/basestreams.h $(BP)/basethreads.h
##########################################################

##########################################################
CP = $(RETURN_PATH)/$(COMMON_PATH)
##########################################################
COMMON_OBJ = $(CP)/grfio.o $(CP)/baseparam.o $(CP)/baseio.o $(CP)/basesq.o $(CP)/core.o $(CP)/db.o $(CP)/plugins.o $(CP)/lock.o $(CP)/malloc.o $(CP)/md5calc.o $(CP)/mmo.o $(CP)/nullpo.o $(CP)/showmsg.o $(CP)/socket.o $(CP)/strlib.o $(CP)/timer.o $(CP)/utils.o
##########################################################
COMMON_H = $(CP)/grfio.h $(CP)/base.h $(CP)/baseparam.h $(CP)/baseio.h $(CP)/basesq.h $(CP)/core.h $(CP)/db.h $(CP)/plugins.h $(CP)/lock.h $(CP)/malloc.h $(CP)/md5calc.h $(CP)/mmo.h $(CP)/nullpo.h $(CP)/showmsg.h $(CP)/socket.h $(CP)/strlib.h $(CP)/timer.h $(CP)/utils.h $(CP)/version.h
##########################################################


map-server: txtobj/map.o txtobj/chrif.o txtobj/pc.o txtobj/npc.o txtobj/chat.o txtobj/path.o txtobj/itemdb.o txtobj/mob.o txtobj/script.o txtobj/storage.o txtobj/skill.o txtobj/atcommand.o txtobj/charcommand.o txtobj/battle.o txtobj/intif.o txtobj/trade.o txtobj/party.o txtobj/vending.o txtobj/guild.o txtobj/pet.o txtobj/log.o txtobj/status.o txtobj/mail.o txtobj/npc_chat.o txtobj/mercenary.o txtobj/clif.o $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> $(LIB_S) $(LIBS) -o $(RETURN_PATH)/$@

map-server_sql: sqlobj/map.o sqlobj/chrif.o sqlobj/pc.o sqlobj/npc.o sqlobj/chat.o sqlobj/path.o sqlobj/itemdb.o sqlobj/mob.o sqlobj/script.o sqlobj/storage.o sqlobj/skill.o sqlobj/atcommand.o sqlobj/charcommand.o sqlobj/battle.o sqlobj/intif.o sqlobj/trade.o sqlobj/party.o sqlobj/vending.o sqlobj/guild.o sqlobj/pet.o sqlobj/log.o sqlobj/status.o sqlobj/mail.o sqlobj/npc_chat.o sqlobj/mercenary.o sqlobj/clif.o $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> $(LIB_S) $(LIBS) -o $(RETURN_PATH)/$@

txtobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
sqlobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<


txtobj/map.o: map.cpp map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h $(COMMON_H) $(BASE_H)
txtobj/chrif.o: chrif.cpp map.h battle.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H) $(BASE_H)
txtobj/clif.o: clif.cpp map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h charcommand.h $(COMMON_H) $(BASE_H)
txtobj/pc.o: pc.cpp map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h $(COMMON_H) $(BASE_H)
txtobj/npc.o: npc.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H) $(BASE_H)
txtobj/chat.o: chat.cpp map.h clif.h pc.h chat.h $(COMMON_H) $(BASE_H)
txtobj/path.o: path.cpp map.h battle.h $(COMMON_H) $(BASE_H)
txtobj/mob.o: mob.cpp map.h clif.h intif.h pc.h mob.h skill.h battle.h npc.h itemdb.h $(COMMON_H) $(BASE_H)
txtobj/script.o: script.cpp itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h log.h $(COMMON_H) $(BASE_H)
txtobj/storage.o: storage.cpp itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H) $(BASE_H)
txtobj/skill.o: skill.cpp skill.h map.h clif.h pc.h mob.h battle.h itemdb.h script.h $(COMMON_H) $(BASE_H)
txtobj/atcommand.o: atcommand.cpp atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H) $(BASE_H)
txtobj/battle.o: battle.cpp battle.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H) $(BASE_H)
txtobj/intif.o: intif.cpp intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h pet.h $(COMMON_H) $(BASE_H)
txtobj/trade.o: trade.cpp trade.h clif.h itemdb.h map.h pc.h npc.h $(COMMON_H) $(BASE_H)
txtobj/status.o: status.cpp status.h clif.h intif.h pc.h map.h battle.h $(COMMON_H) $(BASE_H)
txtobj/party.o: party.cpp party.h clif.h intif.h pc.h map.h battle.h $(COMMON_H) $(BASE_H)
txtobj/vending.o: vending.cpp vending.h clif.h itemdb.h map.h pc.h $(COMMON_H) $(BASE_H)
txtobj/guild.o: guild.cpp guild.h storage.h battle.h clif.h intif.h pc.h npc.h map.h $(COMMON_H) $(BASE_H)
txtobj/pet.o: pet.cpp pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h mob.h npc.h script.h $(COMMON_H) $(BASE_H)
txtobj/log.o: log.cpp log.h map.h $(COMMON_H) $(BASE_H)
txtobj/charcommand.o: charcommand.cpp charcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H) $(BASE_H)
txtobj/mail.o: mail.cpp mail.h $(COMMON_H) $(BASE_H)
txtobj/npc_chat.o: npc_chat.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H) $(BASE_H)
txtobj/mercenary.o: mercenary.cpp mercenary.h $(COMMON_H) $(BASE_H)

sqlobj/map.o: map.cpp map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h $(COMMON_H) $(BASE_H)
sqlobj/chrif.o: chrif.cpp map.h battle.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H) $(BASE_H)
sqlobj/clif.o: clif.cpp map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h charcommand.h $(COMMON_H) $(BASE_H)
sqlobj/pc.o: pc.cpp map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h $(COMMON_H) $(BASE_H)
sqlobj/npc.o: npc.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H) $(BASE_H)
sqlobj/chat.o: chat.cpp map.h clif.h pc.h chat.h $(COMMON_H) $(BASE_H)
sqlobj/path.o: path.cpp map.h battle.h $(COMMON_H) $(BASE_H)
sqlobj/mob.o: mob.cpp map.h clif.h intif.h pc.h mob.h skill.h battle.h npc.h itemdb.h $(COMMON_H) $(BASE_H)
sqlobj/script.o: script.cpp itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h log.h $(COMMON_H) $(BASE_H)
sqlobj/storage.o: storage.cpp itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H) $(BASE_H)
sqlobj/skill.o: skill.cpp skill.h map.h clif.h pc.h mob.h battle.h itemdb.h script.h $(COMMON_H) $(BASE_H)
sqlobj/atcommand.o: atcommand.cpp atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H) $(BASE_H)
sqlobj/battle.o: battle.cpp battle.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H) $(BASE_H)
sqlobj/intif.o: intif.cpp intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h pet.h $(COMMON_H) $(BASE_H)
sqlobj/trade.o: trade.cpp trade.h clif.h itemdb.h map.h pc.h npc.h $(COMMON_H) $(BASE_H)
sqlobj/status.o: status.cpp status.h clif.h intif.h pc.h map.h battle.h $(COMMON_H) $(BASE_H)
sqlobj/party.o: party.cpp party.h clif.h intif.h pc.h map.h battle.h $(COMMON_H) $(BASE_H)
sqlobj/vending.o: vending.cpp vending.h clif.h itemdb.h map.h pc.h $(COMMON_H) $(BASE_H)
sqlobj/guild.o: guild.cpp guild.h storage.h battle.h clif.h intif.h pc.h npc.h map.h $(COMMON_H) $(BASE_H)
sqlobj/pet.o: pet.cpp pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h mob.h npc.h script.h $(COMMON_H) $(BASE_H)
sqlobj/log.o: log.cpp log.h map.h $(COMMON_H) $(BASE_H)
sqlobj/charcommand.o: charcommand.cpp charcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H) $(BASE_H)
sqlobj/mail.o: mail.cpp mail.h $(COMMON_H) $(BASE_H)
sqlobj/npc_chat.o: npc_chat.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H) $(BASE_H)
sqlobj/mercenary.o: mercenary.cpp mercenary.h $(COMMON_H) $(BASE_H)

clean:
	rm -rf *.o $(RETURN_PATH)/map-server $(RETURN_PATH)/map-server_sql sqlobj txtobj
