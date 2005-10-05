login all: converter
COMMON_OBJ = ../common/core.o ../common/timer.o ../common/db.o ../common/socket.o ../common/dbaccess.o ../common/nullpo.o
COMMON_H = ../common/core.h ../common/timer.h ../common/db.h ../common/socket.h ../common/dbaccess.h ../common/nullpo.h ../common/mmo.h

converter: converter.o ../common/dbaccess.o ../common/malloc.o strlib.o
	$(CC) -o ../../$@ $^ $(LIB_S)
login-server: main.o md5calc.o strlib.o int_login.o int_char.o $(COMMON_OBJ)
	$(CC) -o ../../$@ $^ $(LIB_S)

converter.o: converter.c $(COMMON_H)

main.o: main.c main.h md5calc.h strlib.h int_login.h $(COMMON_H)
int_login.o: int_login.c int_login.h int_char.h $(COMMON_H)
int_char.o: int_char.c int_char.h $(COMMON_H)
md5calc.o: md5calc.c md5calc.h
strlib.o: strlib.c strlib.h
clean:
	rm -f *.o ../../login-server ../../converter

