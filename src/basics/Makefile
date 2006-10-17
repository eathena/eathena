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

include $(BP)/Makefile.dep

all basics: $(BASE_OBJ)
#basealgo.o basearray.o basebuffer.o basecharset.o basefile.o baseheaps.o baseinet.o baselists.o basememory.o basemysql.o baseobjects.o baseparam.o baseparser.o basereentrant.o baseregex.o basesocket.o basestrformat.o basestring.o basestrsearch.o basesync.o basethreadpool.o basethreads.o basetime.o basetree.o basetxtdb.o basetypes.o basevariant.o basezlib.o


basealgo.o: basealgo.cpp basetypes.h basealgo.h basearray.h baseobjects.h basesafeptr.h basememory.h
basearray.o: basearray.cpp basetypes.h baseobjects.h basesafeptr.h basealgo.h basetime.h basememory.h basestring.h basearray.h basestrformat.h basestrsearch.h basesync.h baseexceptions.h
basebuffer.o: basebuffer.cpp basebuffer.h basememory.h basetypes.h baseobjects.h baseinet.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h basesync.h baseexceptions.h
basecharset.o: basecharset.cpp basetypes.h baseobjects.h basecharset.h basealgo.h basetime.h basememory.h basestring.h basearray.h basesafeptr.h basestrformat.h
basefile.o: basefile.cpp basetypes.h baseobjects.h basefile.h basetime.h basearray.h basesafeptr.h basememory.h basealgo.h basestring.h basestrformat.h basestrsearch.h
baseheaps.o: baseheaps.cpp basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h basestrformat.h baseexceptions.h baseheaps.h
baseinet.o: baseinet.cpp basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h baseregex.h baseexceptions.h baseinet.h basesocket.h
baselists.o: baselists.cpp basetypes.h baselists.h baseobjects.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h
basememory.o: basememory.cpp basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h baseexceptions.h
basemysql.o: basemysql.cpp basemysql.h basetypes.h baseobjects.h basestring.h basememory.h basealgo.h basetime.h basearray.h basesafeptr.h basestrformat.h basestrsearch.h basepool.h basesync.h baseexceptions.h
baseobjects.o: baseobjects.cpp basetypes.h baseobjects.h basesync.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h
baseparam.o: baseparam.cpp basetypes.h baseobjects.h baseministring.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h basestrformat.h baseexceptions.h basesync.h basefile.h baseparam.h
baseparser.o: baseparser.cpp basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h basestrformat.h baseexceptions.h baseparser.h
basereentrant.o: basereentrant.cpp basesync.h basetypes.h baseobjects.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h basereentrant.h
baseregex.o: baseregex.cpp basetypes.h baseregex.h basestring.h baseobjects.h basememory.h basealgo.h basetime.h basearray.h basesafeptr.h basestrformat.h
basesocket.o: basesocket.cpp basearray.h basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h baseinet.h basetime.h basestring.h basestrformat.h basebuffer.h basesync.h baseexceptions.h basethreads.h basesocket.h baseregex.h
basesort.o: basesort.cpp basetypes.h basealgo.h basesort.h basearray.h baseobjects.h basesafeptr.h basememory.h
basestrformat.o: basestrformat.cpp basestrformat.h basetypes.h basestring.h baseobjects.h basememory.h basealgo.h basetime.h basearray.h basesafeptr.h baseinet.h
basestring.o: basestring.cpp basetypes.h baseobjects.h basesafeptr.h basememory.h basealgo.h basetime.h basestring.h basearray.h basestrformat.h baseexceptions.h basestrsearch.h baseministring.h
basestrsearch.o: basestrsearch.cpp basestrsearch.h basestring.h basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basearray.h basesafeptr.h basestrformat.h
basesync.o: basesync.cpp basetypes.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h basesync.h baseexceptions.h
basethreadpool.o: basethreadpool.cpp basethreadpool.h basetypes.h basearray.h baseobjects.h basesafeptr.h basememory.h basealgo.h basethreads.h basesync.h baseexceptions.h basetime.h basestring.h basestrformat.h
basethreads.o: basethreads.cpp basetypes.h baseexceptions.h baseobjects.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h basesync.h basethreads.h
basetime.o: basetime.cpp basetypes.h basetime.h
basetree.o: basetree.cpp basetree.h basetypes.h basesync.h baseobjects.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h
basetxtdb.o: basetxtdb.cpp basetypes.h baseobjects.h basetxtdb.h basesync.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h basefile.h
basetypes.o: basetypes.cpp basetypes.h baseobjects.h basesync.h baseexceptions.h basememory.h basealgo.h basetime.h basestring.h basearray.h basesafeptr.h basestrformat.h
basevariant.o: basevariant.cpp basevariant.h basestring.h basearray.h baseobjects.h basetypes.h
basezlib.o: basezlib.cpp basetypes.h baseobjects.h basesafeptr.h basefile.h basetime.h basearray.h basememory.h basealgo.h basestring.h basestrformat.h baselib.h basezlib.h

clean:
	rm -rf *.o GNUMakefile
