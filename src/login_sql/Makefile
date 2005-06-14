all sql: login-server_sql

COMMON_OBJ = ../common/obj/core.o ../common/obj/socket.o ../common/obj/timer.o ../common/obj/db.o ../common/obj/plugins.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/utils.o ../common/obj/strlib.o
COMMON_H = ../common/core.h ../common/socket.h ../common/timer.h ../common/mmo.h ../common/version.h ../common/db.h ../common/plugins.h ../common/malloc.h ../common/showmsg.h ../common/utils.h ../common/strlib.h

login-server_sql: login.o md5calc.o $(COMMON_OBJ)
	$(CC) -o ../../$@ $^ $(LIB_S)

login.o: login.c login.h md5calc.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h

clean:
	rm -f *.o ../../login-server_sql

