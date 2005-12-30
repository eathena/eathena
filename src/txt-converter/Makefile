all sql: char-converter login-converter

char-converter: char-converter.o ../common/obj/minicore.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/strlib.o ../common/obj/mapindex.o
	$(CC) -o ../../tool/$@ $^ $(LIB_S)

login-converter: login-converter.o ../common/obj/minicore.o ../common/obj/db.o ../common/obj/malloc.o ../common/obj/showmsg.o
	$(CC) -o ../../tool/$@ $^ $(LIB_S)

clean:
	rm -f *.o ../../tool/login-converter ../../tool/char-converter

# DO NOT DELETE

char-converter.o: char-converter.c
login-converter.o: login-converter.c
