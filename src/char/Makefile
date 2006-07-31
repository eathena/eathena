##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) -c $<
##########################################################


##########################################################
RETURN_PATH ?= .
COMMON_PATH ?= ../common
BASE_PATH   ?= ../basics
CCC         ?= g++ -pipe
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
BP = $(RETURN_PATH)/$(BASE_PATH)
##########################################################
include $(BP)/Makefile.dep


##########################################################
CP = $(RETURN_PATH)/$(COMMON_PATH)
##########################################################
include $(CP)/Makefile.dep


all char-server: $(RETURN_PATH)/char-server

$(RETURN_PATH)/char-server: char.o inter.o int_party.o int_guild.o int_storage.o int_pet.o irc.o $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> $(LIBS) -o $@


char.o: char.cpp char.h inter.h int_pet.h $(COMMON_H) $(BASE_H)
inter.o: inter.cpp inter.h int_party.h int_guild.h int_storage.h int_pet.h char.h $(COMMON_H) $(BASE_H)
int_party.o: int_party.cpp int_party.h inter.h char.h $(COMMON_H) $(BASE_H)
int_guild.o: int_guild.cpp int_guild.h int_storage.h inter.h char.h $(COMMON_H) $(BASE_H)
int_storage.o: int_storage.cpp int_storage.h int_guild.h inter.h char.h $(COMMON_H) $(BASE_H)
int_pet.o: int_pet.cpp int_pet.h inter.h char.h $(COMMON_H) $(BASE_H)
irc.o: irc.cpp irc.h char.h $(COMMON_H) $(BASE_H)

clean:
	rm -rf *.o $(RETURN_PATH)/char-server GNUmakefile
