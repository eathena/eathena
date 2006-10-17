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
CPPFLAGS    ?= -g -O3 -ffast-math -Wall -D_REENTRANT -I$(RETURN_PATH)/$(BASE_PATH)
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
include $(BP)/Makefile.dep



all scriptconv: $(RETURN_PATH)/eascriptconv

$(RETURN_PATH)/eascriptconv: scriptengine.o scriptconv.o $(BASE_OBJ)
	$(CCC) $^ $(LIBS) -o $@

scriptengine.o: scriptengine.cpp eascript.engine.txt scriptengine.h $(BASE_H)
scriptconv.o: scriptconv.cpp scriptengine.h $(BASE_H)
eascript.engine.txt:

clean:
	rm -rf *.o $(RETURN_PATH)/eascriptconv GNUmakefile
