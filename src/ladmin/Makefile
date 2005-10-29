txt sql all: ladmin

##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) -c $<
##########################################################


COMMON_OBJ = ../common/baseparam.o ../common/baseio.o ../common/basesq.o ../common/core.o ../common/db.o ../common/plugins.o ../common/lock.o ../common/malloc.o ../common/md5calc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o
COMMON_H = ../common/base.h ../common/baseparam.h ../common/baseio.h ../common/core.h ../common/db.h ../common/plugins.h ../common/lock.h ../common/malloc.h ../common/md5calc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/version.h

ladmin: ladmin.o $(COMMON_OBJ)
	$(CCC) $> $(LIB_S) -o ../../$@
ladmin.o: ladmin.cpp ladmin.h $(COMMON_H)

clean:
	rm -rf *.o ../../ladmin
