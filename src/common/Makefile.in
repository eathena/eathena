.PHONY: all sql txt obj common common_sql clean

sql all: obj common common_sql

txt: obj common


obj:
	-mkdir obj

common: obj/core.o obj/socket.o obj/timer.o obj/db.o obj/plugins.o obj/lock.o \
	obj/nullpo.o obj/malloc.o obj/showmsg.o  obj/strlib.o obj/utils.o \
	obj/grfio.o obj/minicore.o obj/minisocket.o obj/minimalloc.o \
	obj/mapindex.o obj/ers.o obj/md5calc.o

common_sql: obj/sql.o

obj/%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

obj/mini%.o: %.c
	$(COMPILE.c) -DMINICORE $(OUTPUT_OPTION) $<


clean:
	rm -rf *.o obj GNUmakefile

HAVESVN = $(shell which svnversion)

ifeq ($(findstring /,$(HAVESVN)), /)
svnversion.h:
	@printf "#define SVNVERSION " > svnversion.h
	@svnversion . >> svnversion.h
	@printf "\n" >> svnversion.h
else
svnversion.h:
	@printf "\n" > svnversion.h
endif

obj/minicore.o: core.c core.h
obj/minisocket.o: socket.c socket.h
obj/minimalloc.o: malloc.c malloc.h

# DO NOT DELETE

obj/core.o: core.c core.h showmsg.h svnversion.h
obj/db.o: db.c db.h showmsg.h ers.h
obj/ers.o: ers.c ers.h cbasetypes.h
obj/grfio.o: grfio.c grfio.h
obj/lock.o: lock.c lock.h showmsg.h
obj/malloc.o: malloc.c malloc.h showmsg.h
obj/mapindex.o: mapindex.c mapindex.h
obj/md5calc.o: md5calc.c md5calc.h
obj/nullpo.o: nullpo.c nullpo.h showmsg.h
obj/plugins.o: plugins.c plugins.h plugin.h
obj/showmsg.o: showmsg.c showmsg.h
obj/socket.o: socket.c socket.h mmo.h showmsg.h plugins.h
obj/sql.o: sql.c sql.h cbasetypes.h malloc.h showmsg.h utils.h
obj/strlib.o: strlib.c strlib.h utils.h
obj/timer.o: timer.c timer.h showmsg.h
obj/utils.o: utils.c utils.h malloc.h showmsg.h mmo.h
mmo.h: cbasetypes.h
	@touch mmo.h
