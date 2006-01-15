all eascript: eascriptchk

##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################

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


eascriptchk: eacompiler.o scriptchk.o $(BASE_OBJ)
	$(CCC) $> -lz $(LIBS) -o $(RETURN_PATH)/$@

eacompiler.o: eacompiler.cpp $(BASE_H)
scriptchk.o: scriptchk.cpp $(BASE_H)

clean:
	rm -rf *.o $(RETURN_PATH)/eascriptchk GNUmakefile
