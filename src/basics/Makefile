all: basealgo.o basearray.o basecharset.o basefile.o baseheaps.o basememory.o baseobjects.o baseparser.o basestring.o basesync.o basetime.o basetypes.o
#not in this package
#basecomponent.o baseinet.o baselib.o baseparam.o baseregex.o basestreams.o basethreads.o

##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<


##########################################################
RETURN_PATH ?= .
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



basealgo.o: basealgo.cpp basealgo.h basetypes.h
basearray.o: basearray.cpp basearray.h baseexceptions.h basestring.h basememory.h basetime.h basealgo.h basesafeptr.h baseobjects.h basetypes.h
basecharset.o: basecharset.cpp basecharset.h basetypes.h baseobjects.h basealgo.h basetime.h basememory.h basestring.h
basecomponent.o: basecomponent.cpp $(BASE_H)
basefile.o: basefile.cpp basefile.h basetypes.h baseobjects.h
baseheaps.o: baseheaps.cpp baseheaps.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h baseexceptions.h basearray.h
baseinet.o: baseinet.cpp $(BASE_H)
baselib.o: baselib.cpp $(BASE_H)
basememory.o: basememory.cpp basememory.h basetypes.h baseobjects.h basealgo.h basetime.h basestring.h baseexceptions.h 
baseobjects.o: baseobjects.cpp baseobjects.h basetypes.h
baseparam.o: baseparam.cpp $(BASE_H)
baseparser.o: baseparser.cpp baseparser.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h baseexceptions.h basearray.h
baseregex.o: baseregex.cpp $(BASE_H)
basestreams.o: basestreams.cpp $(BASE_H)
basestring.o: basestring.cpp basestring.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h baseexceptions.h basearray.h baseministring.h
basesync.o: basesync.cpp basesync.h basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h baseexceptions.h
basethreads.o: basethreads.cpp $(BASE_H)
basetime.o: basetime.cpp basetime.h basetypes.h
basetypes.o: basetypes.cpp basetypes.h baseobjects.h basesync.h


clean:
	rm -rf *.o GNUmakefile
