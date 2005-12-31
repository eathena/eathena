all sql txt: baseio.o baseparam.o basesq.o basestring.o core.o db.o plugins.o grfio.o lock.o malloc.o md5calc.o mmo.o nullpo.o showmsg.o socket.o strlib.o timer.o utils.o


##########################################################
# stupid make does not know .cpp suffix by default
.SUFFIXES: .cpp

.cpp.o:
	$(CCC) $(CPPFLAGS) $(OUTPUT_OPTION) -c $<
##########################################################

core.o: core.cpp base.h core.h mmo.h showmsg.h socket.h timer.h utils.h version.h
db.o: db.cpp base.h db.h mmo.h showmsg.h utils.h
grfio.o: grfio.cpp base.h grfio.h mmo.h showmsg.h malloc.h utils.h
lock.o: lock.h base.h showmsg.h
malloc.o: malloc.cpp base.h malloc.h showmsg.h
md5calc.o: md5calc.cpp base.h md5calc.h
mmo.o: mmo.cpp mmo.h base.h utils.h showmsg.h malloc.h
nullpo.o: nullpo.cpp base.h nullpo.h showmsg.h
showmsg.o: showmsg.cpp base.h showmsg.h
socket.o: socket.cpp base.h socket.h mmo.h showmsg.h
strlib.o: strlib.cpp base.h strlib.h utils.h
timer.o: timer.cpp base.h timer.h showmsg.h
utils.o: utils.cpp base.h utils.h showmsg.h malloc.h mmo.h
plugins.o: plugins.cpp base.h plugins.h mmo.h core.h utils.h malloc.h version.h showmsg.h
baseio.o: baseio.cpp baseio.h base.h
basesq.o: basesq.cpp basesq.h baseio.h base.h
baseparam.o: baseparam.cpp baseparam.h base.h
basestring.o: basestring.cpp basestring.h base.h
clean:
	rm -rf *.o obj

