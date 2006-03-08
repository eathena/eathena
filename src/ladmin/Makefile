all txt sql: ladmin

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



ladmin: ladmin.o $(COMMON_OBJ) $(BASE_OBJ)
	$(CCC) $> $(LIBS) -o $(RETURN_PATH)/$@

ladmin.o: ladmin.cpp ladmin.h $(COMMON_H)  $(BASE_H)

clean:
	rm -rf *.o $(RETURN_PATH)/ladmin GNUmakefile
