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
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
include $(RETURN_PATH)/$(BASICS_PATH)/Makefile.dep
##########################################################

OBJECTS = eacompiler.o scriptchk.o scriptengine.o scriptprn.o
EXTLIBS = $(BASICS_STATICLIB)

all scriptchk: $(RETURN_PATH)/scriptchk

$(EXTLIBS):
	@echo "$(EXTLIBS) not up to date, rebuild them"

$(RETURN_PATH)/scriptchk: $(OBJECTS) $(EXTLIBS)
	$(CCC) $^ $(LIBS) -o $@

eacompiler.o: eacompiler.cpp $(BASICS_H)
scriptchk.o: scriptchk.cpp $(BASICS_H)
scriptengine.o: scriptengine.cpp scriptengine.h eascript.engine.txt scriptprn.h $(BASICS_H)
scriptprn.o: scriptprn.cpp scriptprn.h $(BASICS_H)
eascript.engine.txt:

clean:
	rm -rf *.o $(RETURN_PATH)/scriptchk GNUmakefile
