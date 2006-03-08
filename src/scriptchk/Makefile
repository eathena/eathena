all scriptchk: eascriptchk

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



eascriptchk: eacompiler.o scriptchk.o $(BASE_OBJ)
	$(CCC) $> $(LIBS) -o $(RETURN_PATH)/$@

eacompiler.o: eacompiler.cpp $(BASE_H)
scriptchk.o: scriptchk.cpp $(BASE_H)

clean:
	rm -rf *.o $(RETURN_PATH)/eascriptchk GNUmakefile
