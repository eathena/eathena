txt sql all: ladmin

##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) -c $<
##########################################################


##########################################################
RETURN_PATH ?= .
COMMON_PATH ?= ../common
BASE_PATH   ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
BASE_OBJ = $(BP)/basealgo.o $(BP)/basearray.o $(BP)/basecharset.o  $(BP)/basefile.o $(BP)/baseheaps.o $(BP)/baseinet.o $(BP)/baselists.o $(BP)/basememory.o $(BP)/baseobjects.o $(BP)/baseparam.o $(BP)/baseparser.o $(BP)/baseregex.o $(BP)/basestring.o $(BP)/basestrsearch.o $(BP)/basesync.o $(BP)/basethreadpool.o $(BP)/basethreads.o $(BP)/basetime.o $(BP)/basetypes.o $(BP)/basezlib.o
##########################################################
BASE_H = $(BP)/basealgo.h $(BP)/basearray.h $(BP)/basecharset.h  $(BP)/baseexceptions.h $(BP)/basefile.h $(BP)/baseheaps.h $(BP)/baseinet.h $(BP)/baselib.h $(BP)/baselists.h $(BP)/basememory.h $(BP)/baseministring.h $(BP)/baseobjects.h $(BP)/baseparam.h $(BP)/baseparser.h $(BP)/basepool.h  $(BP)/baseregex.h $(BP)/basesafeptr.h $(BP)/basestring.h $(BP)/basestrsearch.h $(BP)/basesync.h $(BP)/basethreadpool.h $(BP)/basethreads.h $(BP)/basetime.h $(BP)/basetypes.h $(BP)/basezlib.h
##########################################################

##########################################################
CP = $(RETURN_PATH)/$(COMMON_PATH)
##########################################################
COMMON_OBJ = $(CP)/baseio.o $(CP)/basesq.o $(CP)/core.o $(CP)/db.o $(CP)/plugins.o $(CP)/lock.o $(CP)/malloc.o $(CP)/md5calc.o $(CP)/mmo.o $(CP)/nullpo.o $(CP)/showmsg.o $(CP)/socket.o $(CP)/strlib.o $(CP)/timer.o $(CP)/utils.o
##########################################################
COMMON_H = $(CP)/base.h $(CP)/baseio.h $(CP)/basesq.h $(CP)/core.h $(CP)/db.h $(CP)/plugins.h $(CP)/lock.h $(CP)/malloc.h $(CP)/md5calc.h $(CP)/mmo.h $(CP)/nullpo.h $(CP)/showmsg.h $(CP)/socket.h $(CP)/strlib.h $(CP)/timer.h $(CP)/utils.h $(CP)/version.h
##########################################################



ladmin: ladmin.o $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> $(LIBS) -o $(RETURN_PATH)/$@

ladmin.o: ladmin.cpp ladmin.h $(COMMON_H)  $(BASE_H)

clean:
	rm -rf *.o $(RETURN_PATH)/ladmin GNUmakefile
