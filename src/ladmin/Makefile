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
LIBS        ?= -lpthread -lsocket -lnsl -ldl -lrt -lz
##########################################################
include $(RETURN_PATH)/$(BASICS_PATH)/Makefile.dep
include $(RETURN_PATH)/$(COMMON_PATH)/Makefile.dep
##########################################################

OBJECTS = ladmin.o
EXTLIBS = $(COMMON_STATICLIB) $(BASICS_STATICLIB)

all ladmin: $(RETURN_PATH)/ladmin

$(EXTLIBS):
	@echo "$(EXTLIBS) not up to date, rebuild them"

$(RETURN_PATH)/ladmin: $(OBJECTS) $(EXTLIBS)
	$(CCC) $^ $(LIBS) -o $@

ladmin.o: ladmin.cpp ladmin.h $(COMMON_H)  $(BASICS_H)

clean:
	rm -rf *.o $(RETURN_PATH)/ladmin GNUmakefile
