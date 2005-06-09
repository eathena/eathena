all: login-server
txt: login-server

COMMON_OBJ = ../common/core.o ../common/db.o ../common/dll.o ../common/lock.o ../common/malloc.o ../common/md5calc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o
COMMON_H = ../common/base.h ../common/core.h ../common/db.h ../common/dll.h ../common/lock.h ../common/malloc.h ../common/md5calc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/version.h

login-server: login.o $(COMMON_OBJ)
	$(CC) $(LIB_S)  -o ../../$@ $>
login.o: login.c login.h $(COMMON_H)
$(COMMON_OBJ): $(COMMON_H)

clean:
	rm -rf *.o ../../login-server
