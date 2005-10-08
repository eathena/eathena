login sql all: login-server

COMMON_OBJ = ../common/core.o ../common/timer.o ../common/db.o ../common/socket.o ../common/nullpo.o ../common/dbaccess.o
COMMON_H = ../common/core.h ../common/timer.h ../common/db.h ../common/socket.h ../common/nullpo.h ../common/dbaccess.h ../common/mmo.h

login-server: main.o int_login.o int_char.o $(COMMON_OBJ)
	$(CC) -o ../../../$@ $^ $(LIB_S)

main.o: main.c main.h int_login.h $(COMMON_H)
int_login.o: int_login.c int_login.h int_char.h $(COMMON_H)
int_char.o: int_char.c int_char.h $(COMMON_H)
$(COMMON_OBJ): $(COMMON_H)

clean:
	rm -f *.o ../../../login-server GNUmakefile

