##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp
.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################


##########################################################
RETURN_PATH ?= .
BASICS_PATH ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT -I$(RETURN_PATH)/$(BASICS_PATH)
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
include $(RETURN_PATH)/$(BASICS_PATH)/Makefile.dep
##########################################################

OBJECTS = eabuildin.o eacompiler.o eaengine.o eaparser.o eaprogram.o eastorage.o scriptchk.o scriptengine.o scriptprn.o
EXTLIBS = $(BASICS_STATICLIB)

all scriptchk: $(RETURN_PATH)/scriptchk

$(EXTLIBS):
	@echo "$(EXTLIBS) not up to date, rebuild them"

$(RETURN_PATH)/scriptchk: $(OBJECTS) $(EXTLIBS)
	$(CCC) $^ $(LIBS) -o $@


eabuildin.o: eabuildin.cpp eabuildin.h eaprogram.h eaengine.h $(BASICS_H)
eacompiler.o: eacompiler.cpp eaparser.h scriptprn.h scriptengine.h eascript.h eacompiler.h eaopcode.h eaprogram.h eadefine.h eainstance.h eabuildin.h eastorage.h $(BASICS_H)
eaengine.o: eaengine.cpp eaengine.h eaprogram.h eacompiler.h scriptprn.h eaopcode.h eadefine.h eainstance.h eascript.h eastorage.h $(BASICS_H)
eaparser.o: eaparser.cpp eaparser.h scriptprn.h scriptengine.h eascript.h $(BASICS_H)
eaprogram.o: eaprogram.cpp eaopcode.h eaprogram.h eabuildin.h $(BASICS_H)
eastorage.o: eastorage.cpp eastorage.h eaprogram.h eainstance.h eadefine.h eacompiler.h scriptprn.h eaopcode.h eascript.h eaparser.h scriptengine.h eaengine.h $(BASICS_H)
scriptchk.o: scriptchk.cpp eaparser.h scriptprn.h scriptengine.h eascript.h eacompiler.h eaopcode.h eaprogram.h eadefine.h eainstance.h eastorage.h $(BASICS_H)
scriptengine.o: scriptengine.cpp scriptengine.h eascript.h eascript.engine.txt $(BASICS_H)
scriptprn.o: scriptprn.cpp scriptprn.h $(BASICS_H)
eascript.engine.txt:

clean:
	rm -rf *.o $(RETURN_PATH)/scriptchk GNUmakefile
