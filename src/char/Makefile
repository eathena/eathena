##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) -c $<
##########################################################


##########################################################
RETURN_PATH ?= .
COMMON_PATH ?= ../common
BASICS_PATH ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT
LIBS        ?= -lpthread -ldl -lrt -lz
##########################################################
include $(RETURN_PATH)/$(BASICS_PATH)/Makefile.dep
include $(RETURN_PATH)/$(COMMON_PATH)/Makefile.dep
##########################################################

OBJECTS = char.o inter.o int_party.o int_guild.o int_storage.o int_pet.o irc.o
EXTLIBS = $(COMMON_STATICLIB) $(BASICS_STATICLIB)

all char char-server: $(RETURN_PATH)/char-server

$(EXTLIBS):
	@echo "$(EXTLIBS) not up to date, rebuild them"

$(RETURN_PATH)/char-server: $(OBJECTS) $(EXTLIBS)
	$(CCC) $^ $(LIBS) -o $@


char.o: char.cpp char.h inter.h int_pet.h $(COMMON_H) $(BASICS_H)
inter.o: inter.cpp inter.h int_party.h int_guild.h int_storage.h int_pet.h char.h $(COMMON_H) $(BASICS_H)
int_party.o: int_party.cpp int_party.h inter.h char.h $(COMMON_H) $(BASICS_H)
int_guild.o: int_guild.cpp int_guild.h int_storage.h inter.h char.h $(COMMON_H) $(BASICS_H)
int_storage.o: int_storage.cpp int_storage.h int_guild.h inter.h char.h $(COMMON_H) $(BASICS_H)
int_pet.o: int_pet.cpp int_pet.h inter.h char.h $(COMMON_H) $(BASICS_H)
irc.o: irc.cpp irc.h char.h $(COMMON_H) $(BASICS_H)

clean:
	rm -rf *.o $(RETURN_PATH)/char-server GNUmakefile
