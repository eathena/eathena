txt sql all: core.o db.o dll.o grfio.o lock.o malloc.o md5calc.o mmo.o nullpo.o showmsg.o socket.o strlib.o timer.o utils.o

core.o: core.c core.h mmo.h showmsg.h socket.h timer.h utils.h version.h
db.o: db.c db.h mmo.h showmsg.h utils.h
grfio.o: grfio.c grfio.h mmo.h showmsg.h malloc.h utils.h
lock.o: lock.h showmsg.h
malloc.o: malloc.c malloc.h showmsg.h
md5calc.o: md5calc.c md5calc.h
mmo.o: mmo.c mmo.h utils.h showmsg.h malloc.h
nullpo.o: nullpo.c nullpo.h showmsg.h
showmsg.o: showmsg.c showmsg.h
socket.o: socket.c socket.h mmo.h showmsg.h
strlib.o: strlib.c strlib.h utils.h
timer.o: timer.c timer.h showmsg.h
utils.o: utils.c utils.h showmsg.h malloc.h mmo.h
dll.o: dll.c dll.h mmo.h core.h utils.h malloc.h version.h showmsg.h

clean:
	rm -rf *.o obj

