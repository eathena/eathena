all sql txt: baseio.o basesq.o baseparam.o core.o db.o plugins.o grfio.o lock.o malloc.o md5calc.o mmo.o nullpo.o showmsg.o socket.o strlib.o timer.o utils.o

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
BASE_OBJ = $(BP)/basealgo.o $(BP)/basearray.o $(BP)/basecharset.o $(BP)/basefile.o $(BP)/baseheaps.o $(BP)/basememory.o $(BP)/baseobjects.o $(BP)/baseparser.o $(BP)/basestring.o $(BP)/basesync.o $(BP)/basetime.o $(BP)/basetypes.o
#not in this package
#$(BP)/basecomponent.o $(BP)/baseinet.o $(BP)/baselib.o $(BP)/baseparam.o $(BP)/baseregex.o $(BP)/basestreams.o $(BP)/basethreads.o
##########################################################
BASE_H = $(BP)/basealgo.h $(BP)/basearray.h $(BP)/basecharset.h $(BP)/baseexceptions.h $(BP)/basefile.h $(BP)/baseheaps.h $(BP)/basememory.h $(BP)/baseministring.h $(BP)/baseobjects.h $(BP)/baseparser.h $(BP)/basepool.h $(BP)/basesafeptr.h $(BP)/basestring.h $(BP)/basesync.h $(BP)/basetime.h $(BP)/basetypes.h 
#not in this package
# $(BP)/basecomponent.h $(BP)/baseinet.h $(BP)/baselib.h $(BP)/baseparam.h $(BP)/baseregex.h $(BP)/basestreams.h $(BP)/basethreads.h
##########################################################

##########################################################
CP = $(RETURN_PATH)/$(COMMON_PATH)
##########################################################
COMMON_OBJ = $(CP)/baseparam.o $(CP)/baseio.o $(CP)/basesq.o $(CP)/core.o $(CP)/db.o $(CP)/plugins.o $(CP)/lock.o $(CP)/malloc.o $(CP)/md5calc.o $(CP)/mmo.o $(CP)/nullpo.o $(CP)/showmsg.o $(CP)/socket.o $(CP)/strlib.o $(CP)/timer.o $(CP)/utils.o
##########################################################
COMMON_H = $(CP)/base.h $(CP)/baseparam.h $(CP)/baseio.h $(CP)/basesq.h $(CP)/core.h $(CP)/db.h $(CP)/plugins.h $(CP)/lock.h $(CP)/malloc.h $(CP)/md5calc.h $(CP)/mmo.h $(CP)/nullpo.h $(CP)/showmsg.h $(CP)/socket.h $(CP)/strlib.h $(CP)/timer.h $(CP)/utils.h $(CP)/version.h
##########################################################



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
strlib.o: strlib.cpp base.h strlib.h utils.h
timer.o: timer.cpp base.h timer.h showmsg.h
utils.o: utils.cpp base.h utils.h showmsg.h malloc.h mmo.h
plugins.o: plugins.cpp base.h plugins.h mmo.h core.h utils.h malloc.h version.h showmsg.h
baseio.o: baseio.cpp baseio.h base.h $(BASE_H)
basesq.o: basesq.cpp basesq.h baseio.h base.h $(BASE_H)
baseparam.o: baseparam.cpp baseparam.h base.h $(BASE_H)
clean:
	rm -rf *.o obj GNUmakefile

