all eascript: eascriptchk

##########################################################
# stupid make does not know .cpp suffix by default
#.SUFFIXES: .cpp
#.cpp.o:
#	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################


COMMON_OBJ = ../basics/basefile.o ../basics/baseinet.o ../basics/baselib.o ../basics/baseobjects.o ../basics/baseregex.o ../basics/basestreams.o ../basics/basestrings.o ../basics/basesync.o ../basics/basethreads.o ../basics/basetime.o ../basics/basetypes.o ../basics/baseparser.o
COMMON_H = ../basics/baseexceptions.h ../basics/basefile.h ../basics/baseinet.h ../basics/baselib.h ../basics/baseobjects2.h ../basics/baseobjects.h ../basics/baseregex.h ../basics/basesafeptr.h ../basics/basestreams.h ../basics/basestrings.h ../basics/basesync.h ../basics/basethreads.h ../basics/basetime.h ../basics/basetypes.h ../basics/baseparser.h

eascriptchk: eacompiler.o scriptchk.o $(COMMON_OBJ)
	$(CCC) $> -lz $(LIBS) -o ../$@

eacompiler.o: eacompiler.cpp $(COMMON_H)
scriptchk.o: scriptchk.cpp $(COMMON_H)

clean:
	rm -rf *.o ../eascriptchk
