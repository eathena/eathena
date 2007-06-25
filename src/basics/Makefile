##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################
RETURN_PATH ?= .
BASICS_PATH ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT -Wall -fPIC
##########################################################

include Makefile.dep

all: basics staticlib

basics: $(BASICS_OBJ)

staticlib: basics $(BASICS_STATICLIB)

$(BASICS_STATICLIB): $(BASICS_OBJ)
	ar -rs $@ $+

basealgo.o: basealgo.cpp basetypes.h basealgo.h basearray.h baseobjects.h basesafeptr.h basememory.h basebooltype.h basenew.h baseswap.h basesort.h
basearray.o: basearray.cpp basetypes.h baseobjects.h basesafeptr.h basealgo.h basetime.h basememory.h basebooltype.h basenew.h basestring.h basemathtypes.h basearray.h baseswap.h basesort.h basestrformat.h basestrsearch.h basesync.h baseexceptions.h
basecharset.o: basecharset.cpp basetypes.h baseswap.h basebooltype.h baseobjects.h basecharset.h basealgo.h basetime.h basememory.h basenew.h basestring.h basemathtypes.h basearray.h basesafeptr.h basesort.h basestrformat.h
basefile.o: basefile.cpp basetypes.h baseobjects.h basefile.h basetime.h basearray.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h baseswap.h basesort.h basestring.h basemathtypes.h basestrformat.h basestrsearch.h
baseinet.o: baseinet.cpp basetypes.h basebits.h baseobjects.h basememory.h basebooltype.h basenew.h basealgo.h basetime.h basestring.h basemathtypes.h basearray.h basesafeptr.h baseswap.h basesort.h basestrformat.h baseregex.h baseexceptions.h baseinet.h basesocket.h
baseipfilter.o: baseipfilter.cpp baseipfilter.h baseinet.h basetypes.h baseobjects.h basememory.h basebooltype.h basenew.h basealgo.h basetime.h basestring.h basemathtypes.h basearray.h basesafeptr.h baseswap.h basesort.h basestrformat.h basesync.h baseexceptions.h baseparam.h basetimerhandler.h
basememory.o: basememory.cpp basetypes.h baseobjects.h basememory.h basebooltype.h basenew.h basealgo.h basetime.h basestring.h basemathtypes.h basearray.h basesafeptr.h baseswap.h basesort.h basestrformat.h baseexceptions.h basesync.h
basemysql.o: basemysql.cpp basemysql.h basetypes.h baseobjects.h basestring.h basemathtypes.h basebooltype.h basememory.h basenew.h basealgo.h basetime.h basearray.h basesafeptr.h baseswap.h basesort.h basestrformat.h basestrsearch.h basepool.h basesync.h baseexceptions.h
baseobjects.o: baseobjects.cpp basetypes.h baseobjects.h basesync.h baseexceptions.h basealgo.h
baseparam.o: baseparam.cpp basetypes.h baseobjects.h basesync.h baseexceptions.h basefile.h basetime.h basearray.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h baseswap.h basesort.h basestring.h basemathtypes.h basestrformat.h baseparam.h basetimerhandler.h
baseparser.o: baseparser.cpp basetypes.h baseobjects.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h basetime.h basestring.h basemathtypes.h basearray.h baseswap.h basesort.h basestrformat.h baseexceptions.h baseparser.h
basereentrant.o: basereentrant.cpp basesync.h basetypes.h baseobjects.h baseexceptions.h basereentrant.h
baseregex.o: baseregex.cpp basetypes.h baseregex.h basestring.h basemathtypes.h basebooltype.h baseobjects.h basememory.h basenew.h basealgo.h basetime.h basearray.h basesafeptr.h baseswap.h basesort.h basestrformat.h
basesocket.o: basesocket.cpp basearray.h basetypes.h baseobjects.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h baseswap.h basesort.h baseinet.h basetime.h basestring.h basemathtypes.h basestrformat.h basesync.h baseexceptions.h basesocket.h baseregex.h basebits.h
basesort.o: basesort.cpp basetypes.h basealgo.h basearray.h baseobjects.h basesafeptr.h basememory.h basebooltype.h basenew.h baseswap.h basesort.h basemath.h basebits.h basepair.h basemathtypes.h
basestrformat.o: basestrformat.cpp basestrformat.h basetypes.h basestring.h basemathtypes.h basebooltype.h baseobjects.h basememory.h basenew.h basealgo.h basetime.h basearray.h basesafeptr.h baseswap.h basesort.h
basestring.o: basestring.cpp basetypes.h basebits.h baseobjects.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h basetime.h basestring.h basemathtypes.h basearray.h baseswap.h basesort.h basestrformat.h baseexceptions.h basestrsearch.h
basestrsearch.o: basestrsearch.cpp basestrsearch.h basestring.h basetypes.h basemathtypes.h basebooltype.h baseobjects.h basememory.h basenew.h basealgo.h basetime.h basearray.h basesafeptr.h baseswap.h basesort.h basestrformat.h
basesync.o: basesync.cpp basetypes.h baseobjects.h basememory.h basebooltype.h basenew.h basealgo.h basetime.h basestring.h basemathtypes.h basearray.h basesafeptr.h baseswap.h basesort.h basestrformat.h basesync.h baseexceptions.h
basetime.o: basetime.cpp basetypes.h basetime.h
basetimerhandler.o: basetimerhandler.cpp basetimerhandler.h basetypes.h baseobjects.h basesync.h baseexceptions.h basearray.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h baseswap.h basesort.h
basetxtdb.o: basetxtdb.cpp basetypes.h baseobjects.h basetxtdb.h basesync.h baseexceptions.h basearray.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h baseswap.h basesort.h basestring.h basemathtypes.h basetime.h basestrformat.h basefile.h
basetypes.o: basetypes.cpp basetypes.h baseobjects.h basesync.h baseexceptions.h
basevariant.o: basevariant.cpp basevariant.h basetypes.h baseobjects.h basearray.h basesafeptr.h basememory.h basebooltype.h basenew.h basealgo.h baseswap.h basesort.h basestring.h basemathtypes.h basetime.h basestrformat.h basemath.h basebits.h basepair.h
basezlib.o: basezlib.cpp basetypes.h baseobjects.h basesafeptr.h basefile.h basetime.h basearray.h basememory.h basebooltype.h basenew.h basealgo.h baseswap.h basesort.h basestring.h basemathtypes.h basestrformat.h baselib.h basezlib.h

clean:
	rm -rf *.o *.a *.so GNUMakefile
