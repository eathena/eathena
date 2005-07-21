txt sql all: ladmin

COMMON_OBJ = ../common/core.o ../common/db.o ../common/plugins.o ../common/lock.o ../common/malloc.o ../common/mmo.o ../common/nullpo.o ../common/showmsg.o ../common/socket.o ../common/strlib.o ../common/timer.o ../common/utils.o
COMMON_H =   ../common/core.h ../common/db.h ../common/plugins.h ../common/lock.h ../common/malloc.h ../common/mmo.h ../common/nullpo.h ../common/showmsg.h ../common/socket.h ../common/strlib.h ../common/timer.h ../common/utils.h ../common/version.h ../common/base.h 


ladmin: ladmin.o md5.o$(COMMON_OBJ)
	$(CC) $(LIB_S) -o ../../$@ $>
ladmin.o: ladmin.c ladmin.h $(COMMON_H)
md5.o: md5.cc md5.hh 
$(COMMON_OBJ): $(COMMON_H)

../common/obj/minicore.o: ../common/core.c ../common/core.h
	$(COMPILE.c) -DMINICORE $(OUTPUT_OPTION) $<

clean:
	rm -rf *.o ../../ladmin
