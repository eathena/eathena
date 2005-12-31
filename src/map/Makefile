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


COMMON_OBJ = ../common/grfio.o ../common/baseparam.o ../common/basestring.o ../common/baseio.o ../common/basesq.o ../common/core.o ../common/db.o ../common/plugins.o ../common/lock.o ../common/malloc.o ../common/md5calc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o
COMMON_H = ../common/grfio.h ../common/base.h ../common/baseparam.h ../common/basestring.h ../common/baseio.h ../common/core.h ../common/db.h ../common/plugins.h ../common/lock.h ../common/malloc.h ../common/md5calc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/version.h
LIBS = -lz -lm

map-server: txtobj/map.o txtobj/chrif.o txtobj/pc.o txtobj/npc.o txtobj/chat.o txtobj/path.o txtobj/itemdb.o txtobj/mob.o txtobj/script.o txtobj/storage.o txtobj/skill.o txtobj/atcommand.o txtobj/charcommand.o txtobj/battle.o txtobj/intif.o txtobj/trade.o txtobj/party.o txtobj/vending.o txtobj/guild.o txtobj/pet.o txtobj/log.o txtobj/status.o txtobj/mail.o txtobj/npc_chat.o txtobj/mercenary.o txtobj/clif.o $(COMMON_OBJ)
	$(CCC) $> $(LIB_S) $(LIBS) -o ../../$@

map-server_sql: sqlobj/map.o sqlobj/chrif.o sqlobj/pc.o sqlobj/npc.o sqlobj/chat.o sqlobj/path.o sqlobj/itemdb.o sqlobj/mob.o sqlobj/script.o sqlobj/storage.o sqlobj/skill.o sqlobj/atcommand.o sqlobj/charcommand.o sqlobj/battle.o sqlobj/intif.o sqlobj/trade.o sqlobj/party.o sqlobj/vending.o sqlobj/guild.o sqlobj/pet.o sqlobj/log.o sqlobj/status.o sqlobj/mail.o sqlobj/npc_chat.o sqlobj/mercenary.o sqlobj/clif.o $(COMMON_OBJ)
	$(CCC) $> $(LIB_S) $(LIBS) -o ../../$@

txtobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
sqlobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<

txtobj/map.o: map.cpp map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h $(COMMON_H)
txtobj/chrif.o: chrif.cpp map.h battle.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H)
txtobj/clif.o: clif.cpp map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h charcommand.h $(COMMON_H)
txtobj/pc.o: pc.cpp map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h $(COMMON_H)
txtobj/npc.o: npc.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
txtobj/chat.o: chat.cpp map.h clif.h pc.h chat.h $(COMMON_H)
txtobj/path.o: path.cpp map.h battle.h $(COMMON_H)
txtobj/mob.o: mob.cpp map.h clif.h intif.h pc.h mob.h skill.h battle.h npc.h itemdb.h $(COMMON_H)
txtobj/script.o: script.cpp itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h log.h $(COMMON_H)
txtobj/storage.o: storage.cpp itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H)
txtobj/skill.o: skill.cpp skill.h map.h clif.h pc.h mob.h battle.h itemdb.h script.h $(COMMON_H)
txtobj/atcommand.o: atcommand.cpp atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
txtobj/battle.o: battle.cpp battle.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H)
txtobj/intif.o: intif.cpp intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h pet.h $(COMMON_H)
txtobj/trade.o: trade.cpp trade.h clif.h itemdb.h map.h pc.h npc.h $(COMMON_H)
txtobj/status.o: status.cpp status.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
txtobj/party.o: party.cpp party.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
txtobj/vending.o: vending.cpp vending.h clif.h itemdb.h map.h pc.h $(COMMON_H)
txtobj/guild.o: guild.cpp guild.h storage.h battle.h clif.h intif.h pc.h npc.h map.h $(COMMON_H)
txtobj/pet.o: pet.cpp pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h mob.h npc.h script.h $(COMMON_H)
txtobj/log.o: log.cpp log.h map.h $(COMMON_H)
txtobj/charcommand.o: charcommand.cpp charcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
txtobj/mail.o: mail.cpp mail.h $(COMMON_H)
txtobj/npc_chat.o: npc_chat.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
txtobj/mercenary.o: mercenary.cpp mercenary.h $(COMMON_H)

sqlobj/map.o: map.cpp map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h log.h $(COMMON_H)
sqlobj/chrif.o: chrif.cpp map.h battle.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H)
sqlobj/clif.o: clif.cpp map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h charcommand.h $(COMMON_H)
sqlobj/pc.o: pc.cpp map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h log.h $(COMMON_H)
sqlobj/npc.o: npc.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
sqlobj/chat.o: chat.cpp map.h clif.h pc.h chat.h $(COMMON_H)
sqlobj/path.o: path.cpp map.h battle.h $(COMMON_H)
sqlobj/itemdb.o: itemdb.cpp map.h battle.h itemdb.h $(COMMON_H)
sqlobj/mob.o: mob.cpp map.h clif.h intif.h pc.h mob.h skill.h battle.h npc.h itemdb.h log.h $(COMMON_H)
sqlobj/script.o: script.cpp itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h log.h $(COMMON_H)
sqlobj/storage.o: storage.cpp itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H)
sqlobj/skill.o: skill.cpp skill.h map.h clif.h pc.h mob.h battle.h itemdb.h script.h log.h $(COMMON_H)
sqlobj/atcommand.o: atcommand.cpp atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
sqlobj/battle.o: battle.cpp battle.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H)
sqlobj/intif.o: intif.cpp intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h pet.h $(COMMON_H)
sqlobj/trade.o: trade.cpp trade.h clif.h itemdb.h map.h pc.h npc.h log.h $(COMMON_H)
sqlobj/status.o: status.cpp status.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
sqlobj/party.o: party.cpp party.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
sqlobj/vending.o: vending.cpp vending.h clif.h itemdb.h map.h pc.h log.h $(COMMON_H)
sqlobj/guild.o: guild.cpp guild.h storage.h battle.h clif.h intif.h pc.h npc.h map.h $(COMMON_H)
sqlobj/pet.o: pet.cpp pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h mob.h npc.h script.h $(COMMON_H)
sqlobj/log.o: log.cpp log.h map.h $(COMMON_H)
sqlobj/charcommand.o: charcommand.cpp charcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
sqlobj/mail.o: mail.cpp mail.h $(COMMON_H)
sqlobj/npc_chat.o: npc_chat.cpp map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
sqlobj/mercenary.o: mercenary.cpp mercenary.h $(COMMON_H)


clean:
	rm -rf *.o ../../map-server ../../map-server_sql sqlobj txtobj
