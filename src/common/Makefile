
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



##########################################################
depends = core.o db.o plugins.o lock.o malloc.o md5calc.o mmo.o nullpo.o showmsg.o socket.o timer.o utils.o

ifeq ($(findstring -DWITH_MYSQL,$(CPPFLAGS)), -DWITH_MYSQL)
depends += sqlobj sqlobj/baseio.o sqlobj/basesq.o sqlobj/basetx.o 
else
depends += txtobj txtobj/baseio.o txtobj/basesq.o txtobj/basetx.o
endif

depends += grfio.o


all common: $(depends)

txtobj:
	mkdir txtobj
sqlobj:
	mkdir sqlobj

txtobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
sqlobj/%.o: %.cpp
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<


core.o: core.cpp mmo.h socket.h malloc.h timer.h utils.h core.h version.h db.h showmsg.h plugins.h $(BASE_H)
db.o: db.cpp db.h malloc.h showmsg.h mmo.h socket.h timer.h utils.h $(BASE_H)
grfio.o: grfio.cpp utils.h socket.h malloc.h timer.h mmo.h grfio.h showmsg.h $(BASE_H)
lock.o: lock.cpp lock.h utils.h socket.h malloc.h timer.h showmsg.h $(BASE_H)
malloc.o: malloc.cpp malloc.h showmsg.h utils.h socket.h timer.h $(BASE_H)
md5calc.o: md5calc.cpp md5calc.h $(BASE_H)
mmo.o: mmo.cpp mmo.h socket.h malloc.h timer.h utils.h showmsg.h $(BASE_H)
nullpo.o: nullpo.cpp nullpo.h showmsg.h $(BASE_H)
plugins.o: plugins.cpp mmo.h socket.h malloc.h timer.h utils.h core.h version.h showmsg.h plugin.h plugins.h $(BASE_H)
showmsg.o: showmsg.cpp showmsg.h malloc.h $(BASE_H)
socket.o: socket.cpp socket.h malloc.h timer.h mmo.h utils.h showmsg.h $(BASE_H)
timer.o: timer.cpp timer.h utils.h socket.h malloc.h showmsg.h $(BASE_H)
utils.o: utils.cpp utils.h socket.h malloc.h timer.h showmsg.h mmo.h $(BASE_H)

txtobj/baseio.o: baseio.cpp lock.h timer.h utils.h socket.h malloc.h baseio.h showmsg.h db.h mmo.h basesq.h basetx.h $(BASE_H)
txtobj/basesq.o: basesq.cpp basesq.h baseio.h showmsg.h utils.h socket.h malloc.h timer.h db.h mmo.h $(BASE_H)
txtobj/basetx.o: basetx.cpp baseio.h showmsg.h utils.h socket.h malloc.h timer.h db.h mmo.h lock.h basetx.h $(BASE_H)

sqlobj/baseio.o: baseio.cpp lock.h timer.h utils.h socket.h malloc.h baseio.h showmsg.h db.h mmo.h basesq.h basetx.h $(BASE_H)
sqlobj/basesq.o: basesq.cpp basesq.h baseio.h showmsg.h utils.h socket.h malloc.h timer.h db.h mmo.h $(BASE_H)
sqlobj/basetx.o: basetx.cpp baseio.h showmsg.h utils.h socket.h malloc.h timer.h db.h mmo.h lock.h basetx.h $(BASE_H)


clean:
	rm -rf *.o obj txtobj sqlobj GNUmakefile

