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
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
include $(BP)/Makefile.dep



all scriptchk: $(RETURN_PATH)/eascriptchk

$(RETURN_PATH)/eascriptchk: eacompiler.o scriptchk.o scriptengine.o scriptprn.o $(BASE_OBJ)
	$(CCC) $^ $(LIBS) -o $@

eacompiler.o: eacompiler.cpp $(BASE_H)
scriptchk.o: scriptchk.cpp $(BASE_H)
scriptengine.o: scriptengine.cpp scriptengine.h eascript.engine.txt scriptprn.h $(BASE_H)
scriptprn.o: scriptprn.cpp scriptprn.h $(BASE_H)
eascript.engine.txt:

clean:
	rm -rf *.o $(RETURN_PATH)/eascriptchk GNUmakefile
