txt sql all: obj common

obj:
	mkdir obj

common: obj/core.o obj/socket.o obj/timer.o obj/db.o obj/dll.o obj/lock.o obj/nullpo.o obj/malloc.o obj/showmsg.o  obj/strlib.o obj/utils.o

obj/%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

obj/core.o: core.c core.h showmsg.h svnversion.h
obj/socket.o: socket.c socket.h mmo.h showmsg.h dll.h
obj/timer.o: timer.c timer.h showmsg.h
obj/db.o: db.c db.h showmsg.h
obj/dll.o: dll.c dll.h
obj/lock.o: lock.h showmsg.h
obj/nullpo.o: nullpo.c nullpo.h showmsg.h
obj/malloc.o: malloc.c malloc.h showmsg.h
obj/showmsg.o: showmsg.c showmsg.h
obj/strlib.o: strlib.c strlib.h utils.h

clean:
	rm -rf *.o obj


HAVESVN = $(shell which svnversion)

ifeq ($(findstring /,$(HAVESVN)), /)
svnversion.h: ../../Changelog-SVN.txt
	echo -n "#define SVNVERSION " > svnversion.h
	svnversion . >> svnversion.h
else
svnversion.h:
	echo -n "" > svnversion.h
endif
