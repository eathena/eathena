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
CPPFLAGS    ?= -g -O3 -ffast-math -D_REENTRANT -I$(RETURN_PATH)/$(BASICS_PATH)  -I$(RETURN_PATH)/$(COMMON_PATH)
LIBS        ?= -lpthread -lz -ldl
##########################################################
include $(RETURN_PATH)/$(BASICS_PATH)/Makefile.dep
include $(RETURN_PATH)/$(COMMON_PATH)/Makefile.dep
##########################################################

OBJECTS = login.o admin.o
EXTLIBS = $(COMMON_STATICLIB) $(BASICS_STATICLIB)

all login login-server: $(RETURN_PATH)/login-server

$(EXTLIBS):
	@echo "$(EXTLIBS) not up to date, rebuild them"

$(RETURN_PATH)/login-server: $(OBJECTS) $(EXTLIBS)
	$(CCC) $^ $(LIBS) -o $@

login.o: login.cpp login.h admin.h $(COMMON_H) $(BASICS_H)
admin.o: admin.cpp admin.h login.h $(COMMON_H) $(BASICS_H)

clean:
	rm -rf *.o $(RETURN_PATH)/login-server GNUmakefile
