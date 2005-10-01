
STATICLIB = libz.a
SHAREDLIB = zlib.dll
UNZLIB = libunz.a

Z_OBJS = adler32.o compress.o crc32.o deflate.o gzio.o infback.o \
	 inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o

UNZ_OBJS = unzip.o ioapi.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: $(SHAREDLIB) $(STATICLIB) $(UNZLIB)

$(SHAREDLIB): zlib.def $(Z_OBJS)
	dllwrap --driver-name $(CC) --def zlib.def -o ../../$@ $(Z_OBJS)
	strip ../../$@

$(STATICLIB): $(Z_OBJS)
	ar rcs ../common/$@ $(Z_OBJS)

$(UNZLIB): $(UNZ_OBJS)
	ar rcs ../common/$@ $(UNZ_OBJS)

clean:
	rm -f *.o ../../$(SHAREDLIB) ../common/$(STATICLIB) ../common/$(UNZLIB)

# DO NOT DELETE

adler32.o: zlib.h zconf.h
compress.o: zlib.h zconf.h
crc32.o: zutil.h zlib.h zconf.h crc32.h
deflate.o: deflate.h zutil.h zlib.h zconf.h
gzio.o: zutil.h zlib.h zconf.h
infback.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
inflate.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
trees.o: deflate.h zutil.h zlib.h zconf.h trees.h
uncompr.o: zlib.h zconf.h
zutil.o: zutil.h zlib.h zconf.h
