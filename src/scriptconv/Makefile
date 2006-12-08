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

OBJECTS = scriptconv.o scriptengine.o scriptprn.o dblookup.o oldeascriptconv.o aegisscriptconv.o
EXTLIBS = $(BASICS_STATICLIB)

all scriptconv: $(RETURN_PATH)/scriptconv


$(RETURN_PATH)/scriptconv: $(OBJECTS)
	$(CCC) $^ $(EXTLIBS) $(LIBS) -o $@


scriptconv.o: scriptconv.cpp scriptengine.h scriptprn.h oldeascriptconv.h aegisscriptconv.h dblookup.h $(BASICS_H)
scriptengine.o: scriptengine.cpp scriptengine.h $(BASICS_H)
scriptprn.o: scriptprn.cpp scriptprn.h $(BASICS_H)
dblookup.o: dblookup.cpp dblookup.h $(BASICS_H)
oldeascriptconv.o: oldeascriptconv.cpp oldeascriptconv.h oldeascript.h oldeascript.engine.txt dblookup.h $(BASICS_H)
oldeascript.engine.txt:
aegisscriptconv.o: aegisscriptconv.cpp aegisscriptconv.h aegisscript.h aegisscript.engine.txt dblookup.h $(BASICS_H)
aegisscript.engine.txt:

clean:
	rm -rf *.o $(RETURN_PATH)/scriptconv GNUmakefile
