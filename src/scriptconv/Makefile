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

$(RETURN_PATH)/eascriptconv: scriptconv.o scriptengine.o oldeascriptconv.o aegisscriptconv.o $(BASE_OBJ)
	$(CCC) $^ $(LIBS) -o $@

scriptconv.o: scriptconv.cpp scriptengine.h oldeascriptconv.h aegisscriptconv.h $(BASE_H)
scriptengine.o: scriptengine.cpp scriptengine.h $(BASE_H)
oldeascriptconv.o: oldeascriptconv.cpp oldeascriptconv.h oldeascript.h oldeascript.engine.txt $(BASE_H)
oldeascript.engine.txt:
aegisscriptconv.o: aegisscriptconv.cpp aegisscriptconv.h aegisscript.h aegisscript.engine.txt $(BASE_H)
aegisscript.engine.txt:

clean:
	rm -rf *.o $(RETURN_PATH)/eascriptconv GNUmakefile
