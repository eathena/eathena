all: char-converter login-converter
sql: char-converter login-converter

COMMON_OBJ = core.o ../common/obj/db.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/strlib.o


char-converter: char-converter.o $(COMMON_OBJ)
	$(CC) -o ../../tools/$@ $^ $(LIB_S)
login-converter: login-converter.o $(COMMON_OBJ)
	$(CC) -o ../../tools/$@ $^ $(LIB_S)

char-converter.o: char-converter.c
login-converter.o: login-converter.c
core.o: ../common/core.c ../common/core.h
	$(COMPILE.c) -DMINICORE $(OUTPUT_OPTION) $<	

clean:
	rm -f *.o ../../tools/login-converter ../../tools/char-converter
