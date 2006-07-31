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


all login-server : $(RETURN_PATH)/login-server

$(RETURN_PATH)/login-server: login.o $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> $(LIBS) -o $@

login.o: login.cpp login.h $(COMMON_H) $(BASE_H)

clean:
	rm -rf *.o $(RETURN_PATH)/login-server GNUmakefile
