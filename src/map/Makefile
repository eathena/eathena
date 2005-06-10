all: txt sql

txt: txtobj map-server

sql: sqlobj map-server_sql

txtobj:
	mkdir txtobj

sqlobj:
	mkdir sqlobj

COMMON_OBJ = ../common/core.o ../common/db.o ../common/dll.o ../common/grfio.o ../common/lock.o ../common/malloc.o ../common/md5calc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o

COMMON_H = ../common/base.h ../common/core.h ../common/db.h ../common/dll.h ../common/grfio.h ../common/lock.h ../common/malloc.h ../common/md5calc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/version.h
LIBS = -lz -lm

map-server: txtobj/map.o txtobj/chrif.o txtobj/clif.o txtobj/pc.o txtobj/npc.o txtobj/chat.o txtobj/path.o txtobj/itemdb.o txtobj/mob.o txtobj/script.o txtobj/storage.o txtobj/skill.o txtobj/atcommand.o txtobj/charcommand.o txtobj/battle.o txtobj/intif.o txtobj/trade.o txtobj/party.o txtobj/vending.o txtobj/guild.o txtobj/pet.o txtobj/log.o txtobj/status.o txtobj/mail.o txtobj/npc_chat.o txtobj/mercenary.o $(COMMON_OBJ)
	$(CC) $(LIB_S) $(LIBS) -o ../../$@ $> 

map-server_sql: sqlobj/map.o sqlobj/chrif.o sqlobj/clif.o sqlobj/pc.o sqlobj/npc.o sqlobj/chat.o sqlobj/path.o sqlobj/itemdb.o sqlobj/mob.o sqlobj/script.o sqlobj/storage.o sqlobj/skill.o sqlobj/atcommand.o sqlobj/charcommand.o sqlobj/battle.o sqlobj/intif.o sqlobj/trade.o sqlobj/party.o sqlobj/vending.o sqlobj/guild.o sqlobj/pet.o sqlobj/log.o sqlobj/status.o sqlobj/mail.o sqlobj/npc_chat.o sqlobj/mercenary.o $(COMMON_OBJ)
	$(CC) $(LIB_S) $(LIBS) -o ../../$@ $> 

txtobj/%.o: %.c
	$(COMPILE.c) -DTXT_ONLY $(OUTPUT_OPTION) $<

sqlobj/%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

txtobj/map.o: map.c map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h $(COMMON_H)
txtobj/chrif.o: chrif.c map.h battle.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H)
txtobj/clif.o: clif.c map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h charcommand.h $(COMMON_H)
txtobj/pc.o: pc.c map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h $(COMMON_H)
txtobj/npc.o: npc.c map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
txtobj/chat.o: chat.c map.h clif.h pc.h chat.h $(COMMON_H)
txtobj/path.o: path.c map.h battle.h $(COMMON_H)
txtobj/mob.o: mob.c map.h clif.h intif.h pc.h mob.h skill.h battle.h npc.h itemdb.h $(COMMON_H)
txtobj/script.o: script.c itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h log.h $(COMMON_H)
txtobj/storage.o: storage.c itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H)
txtobj/skill.o: skill.c skill.h map.h clif.h pc.h mob.h battle.h itemdb.h script.h $(COMMON_H)
txtobj/atcommand.o: atcommand.c atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
txtobj/battle.o: battle.c battle.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H)
txtobj/intif.o: intif.c intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h pet.h $(COMMON_H)
txtobj/trade.o: trade.c trade.h clif.h itemdb.h map.h pc.h npc.h $(COMMON_H)
txtobj/status.o: status.c status.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
txtobj/party.o: party.c party.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
txtobj/vending.o: vending.c vending.h clif.h itemdb.h map.h pc.h $(COMMON_H)
txtobj/guild.o: guild.c guild.h storage.h battle.h clif.h intif.h pc.h npc.h map.h $(COMMON_H)
txtobj/pet.o: pet.c pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h mob.h npc.h script.h $(COMMON_H)
txtobj/log.o: log.c log.h map.h $(COMMON_H)
txtobj/charcommand.o: charcommand.c charcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
txtobj/mail.o: mail.c mail.h $(COMMON_H)
txtobj/npc_chat.o: npc_chat.c map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
txtobj/mercenary.o: mercenary.c mercenary.h $(COMMON_H)

sqlobj/map.o: map.c map.h chrif.h clif.h npc.h pc.h mob.h chat.h skill.h itemdb.h storage.h party.h pet.h atcommand.h log.h $(COMMON_H)
sqlobj/chrif.o: chrif.c map.h battle.h chrif.h clif.h intif.h pc.h npc.h $(COMMON_H)
sqlobj/clif.o: clif.c map.h chrif.h clif.h mob.h intif.h pc.h npc.h itemdb.h chat.h script.h storage.h party.h guild.h atcommand.h pet.h charcommand.h $(COMMON_H)
sqlobj/pc.o: pc.c map.h clif.h intif.h pc.h npc.h mob.h itemdb.h battle.h skill.h script.h party.h guild.h pet.h trade.h storage.h chat.h vending.h log.h $(COMMON_H)
sqlobj/npc.o: npc.c map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
sqlobj/chat.o: chat.c map.h clif.h pc.h chat.h $(COMMON_H)
sqlobj/path.o: path.c map.h battle.h $(COMMON_H)
sqlobj/itemdb.o: itemdb.c map.h battle.h itemdb.h $(COMMON_H)
sqlobj/mob.o: mob.c map.h clif.h intif.h pc.h mob.h skill.h battle.h npc.h itemdb.h log.h $(COMMON_H)
sqlobj/script.o: script.c itemdb.h map.h pc.h mob.h clif.h intif.h npc.h script.h storage.h skill.h pet.h battle.h log.h $(COMMON_H)
sqlobj/storage.o: storage.c itemdb.h pc.h clif.h intif.h storage.h guild.h $(COMMON_H)
sqlobj/skill.o: skill.c skill.h map.h clif.h pc.h mob.h battle.h itemdb.h script.h log.h $(COMMON_H)
sqlobj/atcommand.o: atcommand.c atcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
sqlobj/battle.o: battle.c battle.h skill.h map.h mob.h pc.h pet.h guild.h $(COMMON_H)
sqlobj/intif.o: intif.c intif.h chrif.h clif.h party.h guild.h storage.h map.h battle.h pet.h $(COMMON_H)
sqlobj/trade.o: trade.c trade.h clif.h itemdb.h map.h pc.h npc.h log.h $(COMMON_H)
sqlobj/status.o: status.c status.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
sqlobj/party.o: party.c party.h clif.h intif.h pc.h map.h battle.h $(COMMON_H)
sqlobj/vending.o: vending.c vending.h clif.h itemdb.h map.h pc.h log.h $(COMMON_H)
sqlobj/guild.o: guild.c guild.h storage.h battle.h clif.h intif.h pc.h npc.h map.h $(COMMON_H)
sqlobj/pet.o: pet.c pet.h map.h clif.h chrif.h intif.h pc.h itemdb.h battle.h mob.h npc.h script.h $(COMMON_H)
sqlobj/log.o: log.c log.h map.h $(COMMON_H)
sqlobj/charcommand.o: charcommand.c charcommand.h itemdb.h pc.h map.h skill.h clif.h mob.h intif.h battle.h storage.h guild.h pet.h log.h $(COMMON_H)
sqlobj/mail.o: mail.c mail.h $(COMMON_H)
sqlobj/npc_chat.o: npc_chat.c map.h npc.h clif.h pc.h script.h mob.h itemdb.h battle.h $(COMMON_H)
sqlobj/mercenary.o: mercenary.c mercenary.h $(COMMON_H)

$(COMMON_OBJ): $(COMMON_H)

clean:
	rm -rf *.o ../../map-server ../../map-server_sql sqlobj txtobj
