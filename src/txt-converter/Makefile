all sql: char-converter login-converter

char-converter: char-converter.o ../common/obj/minicore.o ../common/obj/malloc.o ../common/obj/showmsg.o ../common/obj/strlib.o
	$(CC) -o ../../tools/$@ $^ $(LIB_S)

login-converter: login-converter.o ../common/obj/minicore.o ../common/obj/db.o ../common/obj/malloc.o ../common/obj/showmsg.o
	$(CC) -o ../../tools/$@ $^ $(LIB_S)

../common/obj/minicore.o: ../common/core.c ../common/core.h
	$(COMPILE.c) -DMINICORE $(OUTPUT_OPTION) $<	

clean:
	rm -f *.o ../../tools/login-converter ../../tools/char-converter

# DO NOT DELETE

char-converter.o: char-converter.c
login-converter.o: login-converter.c