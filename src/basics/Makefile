all basics: basealgo.o basearray.o basecharset.o basefile.o baseheaps.o baseinet.o baselists.o basememory.o baseobjects.o baseparam.o baseparser.o baseregex.o basestring.o basestrsearch.o basesync.o basethreadpool.o basethreads.o basetime.o basetypes.o basezlib.o


##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<


##########################################################
RETURN_PATH ?= .
BASE_PATH   ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT -Wall
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
BASE_OBJ = $(BP)/basealgo.o $(BP)/basearray.o $(BP)/basecharset.o  $(BP)/basefile.o $(BP)/baseheaps.o $(BP)/baseinet.o $(BP)/baselists.o $(BP)/basememory.o $(BP)/baseobjects.o $(BP)/baseparam.o $(BP)/baseparser.o $(BP)/baseregex.o $(BP)/basestring.o $(BP)/basestrsearch.o $(BP)/basesync.o $(BP)/basethreadpool.o $(BP)/basethreads.o $(BP)/basetime.o $(BP)/basetypes.o $(BP)/basezlib.o
##########################################################
BASE_H = $(BP)/basealgo.h $(BP)/basearray.h $(BP)/basecharset.h  $(BP)/baseexceptions.h $(BP)/basefile.h $(BP)/baseheaps.h $(BP)/baseinet.h $(BP)/baselib.h $(BP)/baselists.h $(BP)/basememory.h $(BP)/baseministring.h $(BP)/baseobjects.h $(BP)/baseparam.h $(BP)/baseparser.h $(BP)/basepool.h  $(BP)/baseregex.h $(BP)/basesafeptr.h $(BP)/basestring.h $(BP)/basestrsearch.h $(BP)/basesync.h $(BP)/basethreadpool.h $(BP)/basethreads.h $(BP)/basetime.h $(BP)/basetypes.h $(BP)/basezlib.h
##########################################################



basealgo.o: basealgo.cpp basealgo.h basetypes.h
basearray.o: basearray.cpp basearray.h baseexceptions.h basestring.h basememory.h basetime.h basealgo.h basesafeptr.h baseobjects.h basetypes.h
basecharset.o: basecharset.cpp basecharset.h basetypes.h baseobjects.h basealgo.h basetime.h basememory.h basestring.h
basefile.o: basefile.cpp basefile.h basetypes.h baseobjects.h
baseheaps.o: baseheaps.cpp baseheaps.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h baseexceptions.h basearray.h
baseinet.o: baseinet.cpp baseinet.h baseexceptions.h basestring.h basetime.h basealgo.h basememory.h baseobjects.h basetypes.h
baselists.o: baselists.cpp baselists.h baseobjects.h basetypes.h
basememory.o: basememory.cpp basememory.h basetypes.h baseobjects.h basealgo.h basetime.h basestring.h baseexceptions.h 
baseobjects.o: baseobjects.cpp baseobjects.h basetypes.h
baseparam.o: baseparam.cpp $(BASE_H)
baseparser.o: baseparser.cpp baseparser.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h baseexceptions.h basearray.h
baseregex.o: baseregex.cpp baseregex.h basearray.h baseexceptions.h basetime.h basestring.h basealgo.h basememory.h basesafeptr.h baseobjects.h basetypes.h
basestring.o: basestring.cpp basestring.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h baseexceptions.h basearray.h baseministring.h
basestrsearch.o: basestrsearch.cpp  $(BASE_H)
basesync.o: basesync.cpp basesync.h basetypes.h basememory.h basealgo.h basetime.h basestring.h baseexceptions.h
basethreads.o: basethreads.cpp basethreads.h basesync.h baseexceptions.h baseobjects.h basetypes.h
basethreadpool.o: basethreadpool.cpp $(BASE_H)
basetime.o: basetime.cpp basetime.h basetypes.h
basetypes.o: basetypes.cpp basetypes.h baseobjects.h basesync.h
basezlib.o: basezlib.cpp basezlib.h baselib.h basesafeptr.h baseobjects.h basetypes.h

clean:
	rm -rf *.o
