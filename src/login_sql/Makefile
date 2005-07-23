all login: login-server_sql


COMMON_OBJ = ../common/core.o ../common/db.o ../common/plugins.o ../common/lock.o ../common/malloc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o ../common/dbaccess.o
COMMON_H =   ../common/core.h ../common/db.h ../common/plugins.h ../common/lock.h ../common/malloc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/dbaccess.h ../common/version.h ../common/base.h

login-server_sql: login.o md5.o $(COMMON_OBJ)
	$(CC) $> $(LIB_S) -o ../../$@


login.o: login.c login.h $(COMMON_H)
md5.o: md5.cc md5.hh
$(COMMON_OBJ): $(COMMON_H)

clean:
	rm -rf *.o ../../login-server_sql
