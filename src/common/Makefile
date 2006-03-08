all: txt sql
common txt: txtobj  txtobj/baseio.o txtobj/basesq.o obj
common_sql sql: sqlobj sqlobj/baseio.o sqlobj/basesq.o obj

obj: core.o db.o plugins.o grfio.o lock.o malloc.o md5calc.o mmo.o nullpo.o showmsg.o socket.o timer.o utils.o

txtobj:
	mkdir txtobj
sqlobj:
	mkdir sqlobj

txtobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
sqlobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<

##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################





##########################################################
RETURN_PATH ?= .
COMMON_PATH ?= ../common
BASE_PATH   ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
include $(BP)/Makefile.dep

##########################################################
CP = $(RETURN_PATH)/$(COMMON_PATH)
##########################################################
include $(CP)/Makefile.dep


core.o: core.cpp base.h core.h mmo.h showmsg.h socket.h timer.h utils.h version.h
db.o: db.cpp base.h db.h mmo.h showmsg.h utils.h
grfio.o: grfio.cpp base.h grfio.h mmo.h showmsg.h malloc.h utils.h
lock.o: lock.h base.h showmsg.h
malloc.o: malloc.cpp base.h malloc.h showmsg.h
md5calc.o: md5calc.cpp base.h md5calc.h
mmo.o: mmo.cpp mmo.h base.h utils.h showmsg.h malloc.h
nullpo.o: nullpo.cpp base.h nullpo.h showmsg.h
showmsg.o: showmsg.cpp base.h showmsg.h
socket.o: socket.cpp base.h socket.h mmo.h showmsg.h $(BASE_H)
timer.o: timer.cpp base.h timer.h showmsg.h
utils.o: utils.cpp base.h utils.h showmsg.h malloc.h mmo.h
plugins.o: plugins.cpp base.h plugins.h mmo.h core.h utils.h malloc.h version.h showmsg.h
sqlobj/baseio.o: baseio.cpp baseio.h base.h $(BASE_H)
txtobj/baseio.o: baseio.cpp baseio.h base.h $(BASE_H)
sqlobj/basesq.o: basesq.cpp basesq.h baseio.h base.h $(BASE_H)
txtobj/basesq.o: basesq.cpp basesq.h baseio.h base.h $(BASE_H)

clean:
	rm -rf *.o obj txtobj sqlobj GNUmakefile

