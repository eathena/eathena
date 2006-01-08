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

basealgo.o: basealgo.cpp $(BASE_H)
basearray.o: basearray.cpp $(BASE_H)
basecharset.o: basecharset.cpp $(BASE_H)
basecomponent.o: basecomponent.cpp $(BASE_H)
basefile.o: basefile.cpp $(BASE_H)
baseheaps.o: baseheaps.cpp $(BASE_H)
baseinet.o: baseinet.cpp $(BASE_H)
baselib.o: baselib.cpp $(BASE_H)
basememory.o: basememory.cpp $(BASE_H)
baseobjects.o: baseobjects.cpp $(BASE_H)
baseparam.o: baseparam.cpp $(BASE_H)
baseparser.o: baseparser.cpp $(BASE_H)
baseregex.o: baseregex.cpp $(BASE_H)
basestreams.o: basestreams.cpp $(BASE_H)
basestring.o: basestring.cpp $(BASE_H)
basesync.o: basesync.cpp $(BASE_H)
basethreads.o: basethreads.cpp $(BASE_H)
basetime.o: basetime.cpp $(BASE_H)
basetypes.o: basetypes.cpp $(BASE_H)


clean:
	rm -rf *.o
