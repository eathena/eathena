all: char-server_sql
sql: char-server_sql

##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) -c $<
##########################################################


COMMON_OBJ = ../common/baseparam.o ../common/baseio.o ../common/basesq.o ../common/core.o ../common/db.o ../common/plugins.o ../common/lock.o ../common/malloc.o ../common/md5calc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o
COMMON_H = ../common/base.h ../common/baseparam.h ../common/baseio.h ../common/core.h ../common/db.h ../common/plugins.h ../common/lock.h ../common/malloc.h ../common/md5calc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/version.h

char-server_sql: char.o inter.o int_party.o int_guild.o int_storage.o int_pet.o itemdb.o $(COMMON_OBJ)
	$(CCC) $> $(LIB_S) -o ../../$@

char.o: char.cpp char.h $(COMMON_H)
inter.o: inter.cpp inter.h int_party.h int_guild.h int_storage.h int_pet.h $(COMMON_H)
int_party.o: int_party.cpp int_party.h inter.h $(COMMON_H)
int_guild.o: int_guild.cpp int_guild.h inter.h $(COMMON_H)
int_storage.o: int_storage.cpp int_storage.h char.h itemdb.h $(COMMON_H)
int_pet.o: int_pet.cpp int_pet.h inter.h char.h $(COMMON_H)
itemdb.o: itemdb.cpp itemdb.h $(COMMON_H)

clean:
	rm -rf *.o ../../char-server_sql
