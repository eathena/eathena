all basics: basealgo.o basearray.o basebuffer.o basecharset.o basefile.o baseheaps.o baseinet.o baselists.o basememory.o baseobjects.o baseparam.o baseparser.o baseregex.o basestring.o basestrsearch.o basesync.o basethreadpool.o basethreads.o basetime.o basetypes.o basezlib.o


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
BASE_OBJ = $(BP)/basealgo.o $(BP)/basearray.o $(BP)/basebuffer.o $(BP)/basecharset.o $(BP)/basefile.o $(BP)/baseheaps.o $(BP)/baseinet.o $(BP)/baselists.o $(BP)/basememory.o $(BP)/baseobjects.o $(BP)/baseparam.o $(BP)/baseparser.o $(BP)/baseregex.o $(BP)/basesocket.o $(BP)/basestring.o $(BP)/basestrsearch.o $(BP)/basesync.o $(BP)/basethreadpool.o $(BP)/basethreads.o $(BP)/basetime.o $(BP)/basetypes.o $(BP)/basezlib.o
##########################################################
BASE_H = $(BP)/basealgo.h $(BP)/basearray.h $(BP)/basebuffer.h $(BP)/basecharset.h $(BP)/baseexceptions.h $(BP)/basefile.h $(BP)/baseheaps.h $(BP)/baseinet.h $(BP)/baselib.h $(BP)/baselists.h $(BP)/basememory.h $(BP)/baseministring.h $(BP)/baseobjects.h $(BP)/baseparam.h $(BP)/baseparser.h $(BP)/basepool.h $(BP)/baseregex.h $(BP)/basesocket.h $(BP)/basesafeptr.h $(BP)/basestring.h $(BP)/basestrsearch.h $(BP)/basesync.h $(BP)/basethreadpool.h $(BP)/basethreads.h $(BP)/basetime.h $(BP)/basetypes.h $(BP)/basezlib.h
##########################################################


basealgo.o: basealgo.cpp basetypes.h basealgo.h basearray.h baseobjects.h basesafeptr.h basememory.h
basearray.o: basearray.cpp basetypes.h baseobjects.h basesafeptr.h basealgo.h basetime.h basememory.h basestring.h basearray.h basestrsearch.h baseexceptions.h
basebuffer.o: basebuffer.cpp basebuffer.h basememory.h basetypes.h baseobjects.h baseinet.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basesync.h baseexceptions.h
basecharset.o: basecharset.cpp basetypes.h baseobjects.h basecharset.h basealgo.h basetime.h basememory.h basestring.h basearray.h basesafeptr.h
basefile.o: basefile.cpp basetypes.h baseobjects.h basefile.h basetime.h basearray.h basesafeptr.h basememory.h basealgo.h basestring.h basestrsearch.h
baseheaps.o: baseheaps.cpp basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h baseexceptions.h baseheaps.h
baseinet.o: baseinet.cpp basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h baseexceptions.h basethreads.h basesync.h baseinet.h basesocket.h baseregex.h
baselists.o: baselists.cpp basetypes.h baselists.h baseobjects.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h
basememory.o: basememory.cpp basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h baseexceptions.h
baseobjects.o: baseobjects.cpp basetypes.h baseobjects.h basesync.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h
baseparam.o: baseparam.cpp basetypes.h baseobjects.h baseministring.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h baseexceptions.h basesync.h basefile.h baseparam.h
baseparser.o: baseparser.cpp basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h baseexceptions.h baseparser.h
baserand.o: baserand.cpp baserand.h
baseregex.o: baseregex.cpp basetypes.h baseregex.h basestring.h baseobjects.h basememory.h basealgo.h basetime.h basearray.h basesafeptr.h
basesocket.o: basesocket.cpp basearray.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h baseinet.h basetime.h basestring.h basesync.h baseexceptions.h basethreads.h basesocket.h baseregex.h
basestring.o: basestring.cpp basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h baseexceptions.h basestrsearch.h baseministring.h
basestrsearch.o: basestrsearch.cpp basestrsearch.h basestring.h basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basearray.h basesafeptr.h
basesync.o: basesync.cpp basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basesync.h baseexceptions.h
basethreadpool.o: basethreadpool.cpp basethreadpool.h basetypes.h basearray.h baseobjects.h basesafeptr.h basememory.h basealgo.h basethreads.h basesync.h baseexceptions.h basetime.h basestring.h
basethreads.o: basethreads.cpp basetypes.h baseexceptions.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basesync.h basethreads.h
basetime.o: basetime.cpp basetypes.h basetime.h
basetypes.o: basetypes.cpp basetypes.h baseobjects.h basesync.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h
basezlib.o: basezlib.cpp basetypes.h baseobjects.h basesafeptr.h basefile.h basetime.h basearray.h basememory.h basealgo.h basestring.h baselib.h basezlib.h

clean:
	rm -rf *.o GNUMakefile
