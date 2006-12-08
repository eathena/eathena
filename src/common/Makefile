##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################


##########################################################
RETURN_PATH ?= .
COMMON_PATH ?= ../common
BASICS_PATH ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT -fPIC
##########################################################

include $(RETURN_PATH)/$(BASICS_PATH)/Makefile.dep
include $(RETURN_PATH)/$(COMMON_PATH)/Makefile.dep


##########################################################
OBJECTS = baseio.o basesq.o basetx.o core.o db.o grfio.o plugins.o lock.o malloc.o md5calc.o mmo.o nullpo.o showmsg.o socket.o timer.o utils.o

all: common staticlib

common: $(OBJECTS)

staticlib: common $(COMMON_STATICLIB)

$(COMMON_STATICLIB): $(OBJECTS)
	ar -rs $@ $+

baseio.o: baseio.cpp lock.h timer.h utils.h socket.h malloc.h baseio.h showmsg.h db.h mmo.h basesq.h basetx.h $(BASICS_H)
basesq.o: basesq.cpp basesq.h baseio.h showmsg.h utils.h socket.h malloc.h timer.h db.h mmo.h $(BASICS_H)
basetx.o: basetx.cpp baseio.h showmsg.h utils.h socket.h malloc.h timer.h db.h mmo.h lock.h basetx.h $(BASICS_H)
core.o: core.cpp mmo.h socket.h malloc.h timer.h utils.h core.h version.h db.h showmsg.h plugins.h $(BASICS_H)
db.o: db.cpp db.h malloc.h showmsg.h mmo.h socket.h timer.h utils.h $(BASICS_H)
grfio.o: grfio.cpp utils.h socket.h malloc.h timer.h mmo.h grfio.h showmsg.h $(BASICS_H)
lock.o: lock.cpp lock.h utils.h socket.h malloc.h timer.h showmsg.h $(BASICS_H)
malloc.o: malloc.cpp malloc.h showmsg.h utils.h socket.h timer.h $(BASICS_H)
md5calc.o: md5calc.cpp md5calc.h $(BASICS_H)
mmo.o: mmo.cpp mmo.h socket.h malloc.h timer.h utils.h showmsg.h $(BASICS_H)
nullpo.o: nullpo.cpp nullpo.h showmsg.h $(BASICS_H)
plugins.o: plugins.cpp mmo.h socket.h malloc.h timer.h utils.h core.h version.h showmsg.h plugin.h plugins.h $(BASICS_H)
showmsg.o: showmsg.cpp showmsg.h malloc.h $(BASICS_H)
socket.o: socket.cpp socket.h malloc.h timer.h mmo.h utils.h showmsg.h $(BASICS_H)
timer.o: timer.cpp timer.h utils.h socket.h malloc.h showmsg.h $(BASICS_H)
utils.o: utils.cpp utils.h socket.h malloc.h timer.h showmsg.h mmo.h $(BASICS_H)


clean:
	rm -rf *.o *.a *.so GNUMakefile

