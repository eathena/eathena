all: char-server_sql
sql: char-server_sql

COMMON_OBJ = ../common/core.o ../common/db.o ../common/dll.o ../common/lock.o ../common/malloc.o ../common/md5calc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o

COMMON_H = ../common/base.h ../common/core.h ../common/db.h ../common/dll.h ../common/lock.h ../common/malloc.h ../common/md5calc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/version.h

char-server_sql: char.o inter.o int_party.o int_guild.o int_storage.o int_pet.o itemdb.o $(COMMON_OBJ)
	$(CC) $(LIB_S) -o ../../$@ $>

char.o: char.c char.h $(COMMON_H)
inter.o: inter.c inter.h int_party.h int_guild.h int_storage.h int_pet.h $(COMMON_H)
int_party.o: int_party.c int_party.h inter.h $(COMMON_H)
int_guild.o: int_guild.c int_guild.h inter.h $(COMMON_H)
int_storage.o: int_storage.c int_storage.h char.h itemdb.h $(COMMON_H)
int_pet.o: int_pet.c int_pet.h inter.h char.h $(COMMON_H)
itemdb.o: itemdb.c itemdb.h $(COMMON_H)
$(COMMON_OBJ): $(COMMON_H)

clean:
	rm -rf *.o ../../char-server_sql
