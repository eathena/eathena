txt sql all: core.o db.o plugins.o grfio.o lock.o malloc.o md5calc.o mmo.o nullpo.o showmsg.o socket.o strlib.o timer.o utils.o

core.o: core.c base.h core.h mmo.h showmsg.h socket.h timer.h utils.h version.h
db.o: db.c base.h db.h mmo.h showmsg.h utils.h
grfio.o: grfio.c base.h grfio.h mmo.h showmsg.h malloc.h utils.h
lock.o: lock.h base.h showmsg.h
malloc.o: malloc.c base.h malloc.h showmsg.h
md5calc.o: md5calc.c base.h md5calc.h
mmo.o: mmo.c mmo.h base.h utils.h showmsg.h malloc.h
nullpo.o: nullpo.c base.h nullpo.h showmsg.h
showmsg.o: showmsg.c base.h showmsg.h
socket.o: socket.c base.h socket.h mmo.h showmsg.h
strlib.o: strlib.c base.h strlib.h utils.h
timer.o: timer.c base.h timer.h showmsg.h
utils.o: utils.c base.h utils.h showmsg.h malloc.h mmo.h
plugins.o: plugins.c base.h plugins.h mmo.h core.h utils.h malloc.h version.h showmsg.h

clean:
	rm -rf *.o obj

